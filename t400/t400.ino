/*
# t400-firmware

## Overview
Firmware for the Pax Instruments T400 temperature datalogger

## Setup
1. Install the Arduino IDE from http://arduino.cc/en/Main/Software. Compiles under 1.0.6 and 1.5.8.
2. Install the following Arduino libraries.
  - U8Glib graphical LCD https://github.com/PaxInstruments/u8glib
  - MCP3424 ADC https://github.com/PaxInstruments/MCP3424
  - MCP980X temperature sensor https://github.com/PaxInstruments/MCP980X
  - DS3231 RTC https://github.com/PaxInstruments/ds3231
  - FAT16 SD card library https://github.com/PaxInstruments/Fat16 (use the FAT16 directory within this repository)
3. Set the Arduino board to "LilyPad Arduino USB".
4. After pluggin in your T400 and turning it on ensure you have selected the correct serial port

During compilation on OSX you may receive errors relating to `RobotControl()` in `ArduinoRobot.cpp`. This seems to be a problem in Arduino 1.0.5 and later. To work around this...

1. Go into Applications>Arduino and right-click, "Show package contents"
2. Go to Contents>Resources>Java>libraries
3. Delete the folder "Robot_Control"
 */

// Import libraries
#include "U8glib.h"     // LCD
#include <Wire.h>       // i2c
#include <MCP3424.h>    // ADC
#include <MCP980X.h>    // Ambient/junction temperature sensor
#include <ds3231.h>     // RTC
#include <Fat16.h>      // FAT16 SD card library
#include <Fat16util.h>

#include "power.h"            // Manage board power
#include "buttons.h"          // User buttons
#include "typek_constant.h"   // Thermocouple calibration table
#include "functions.h"        // Misc. functions
#include "sd_log.h"           // SD card utilities
#include "t400.h"             // Board definitions

#define BUFF_MAX         80   // Size of the character buffer


//#define FAKE_TEMPERATURES

char fileName[] =        "LD0000.CSV";

// Graphical LCD
U8GLIB_PI13264  u8g(LCD_CS, LCD_A0, LCD_RST); // Use HW-SPI

// MCP3424 for thermocouple measurements
MCP3424      ADC1(MCP3424_ADDR, MCP342X_GAIN_X8, MCP342X_16_BIT);  // address, gain, resolution

MCP980X ambientSensor(0);      // Ambient temperature sensor

const uint8_t temperatureChannels[SENSOR_COUNT] = {1, 0, 3, 2};  // Map of ADC inputs to thermocouple channels
double temperatures[SENSOR_COUNT];  // Current temperature of each thermocouple input
double ambient =  0;        // Ambient temperature



boolean backlightEnabled;

unsigned long lastLogTime = 0;   // time data was logged
#define LOG_INTERVAL_COUNT 6
const uint8_t logIntervals[LOG_INTERVAL_COUNT] = {1, 2, 5, 10, 30, 60};  // Available log intervals, in seconds
uint8_t logInterval    = 0;       // currently selected log interval
boolean logging = false;          // True if we are currently logging to a file

char updateBuffer[BUFF_MAX];      // Scratch buffer to write serial/sd output into
struct ts rtcTime;                // Buffer to read RTC time into

// This function runs once. Use it for setting up the program state.
void setup(void) {
  powerOn();
  
  Serial.begin(9600);
  
  Wire.begin(); // Start using the Wire library; does the i2c communication.
  TWBR = 24; // TWBR=12 sets the i2c SCK to 400 kHz on an 8 MHz clock. Comment out to run at 100 kHz
  
  pinMode(BATTERY_STATUS_PIN, INPUT);
  
  setupBacklight();
  backlightEnabled = true;
  setBacklight(!backlightEnabled);
  
  resetGraph();
  
  #if DEBUG_LCD
    u8g.setContrast(LCD_CONTRAST); // Set contrast level
  #endif
  
  u8g.setRot180(); // Rotate screen
  u8g.setColorIndex(1); // Set color mode to binary

  ADC1.begin();

  ambientSensor.begin();
  ambientSensor.writeConfig(ADC_RES_12BITS);

  // Set up the RTC
//  DS3231_init(DS3231_INTCN);
//  DS3231_clear_a1f();
//  set_next_alarm();
  
  Buttons::setup();
  
  // Turn on the high-speed interrupt loop
  // TODO: Adjust interrupt speed.
  noInterrupts();
    TCCR4A = 0;
    TCCR4B = 0;
    TCCR4B |= (1 << CS43);    // 8 prescaler 
    TIMSK4 |= (1 << TOIE4);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts
  
  // Schedule the first read for 100ms in the future
  lastLogTime = millis() - logIntervals[logInterval]*1000 + 100;
}

void startLogging() {
  if(logging) {
    return;
  }
  
  logging = true;
  sd::init(fileName);
}

void stopLogging() {
  if(!logging) {
    return;
  }
  
  logging = false;
  sd::close();
}

static void readTemperatures() {
  double measuredVoltageMv;
  double temperature;
  
#ifdef FAKE_TEMPERATURES
  static double count = 0;
#endif
  
  // ADC read loop: Start a measurement, wait until it is finished, then record it
  for(uint8_t channel = 0; channel < SENSOR_COUNT; channel++) {
    ADC1.startMeasurement(temperatureChannels[channel]);
    do {
      // Delay a while. At 16-bit resolution, the ADC can do a speed of 1/15 = .066seconds/cycle
      // Let's wait a little longer than that in case there is set up time for changing channels.
      delay(75);
    } while(!ADC1.measurementReady());

    measuredVoltageMv = ADC1.getMeasurement();
    temperature = GetTypKTemp(measuredVoltageMv*1000);

#ifdef FAKE_TEMPERATURES
    if(channel == 0) {
      temperature  = 0 + count;
    }
    
    count +=.5;
    if(count > 50) {
      count = 0;
    }
#endif

    if(temperature == OUT_OF_RANGE) {
      temperatures[channel] = OUT_OF_RANGE;
    }
    else {
      temperatures[channel] = temperature + ambient;
    }
  }
}

static void writeOutputs() {
  snprintf(updateBuffer, BUFF_MAX, "%02d:%02d:%02d, ", rtcTime.hour, rtcTime.min, rtcTime.sec);
  dtostrf(ambient, 1, 2, updateBuffer+strlen(updateBuffer));
    
  for(uint8_t i = 0; i < SENSOR_COUNT; i++) {
    if(temperatures[i] == OUT_OF_RANGE) {
      strcpy(updateBuffer+strlen(updateBuffer), ", -");
    }
    else {
      strcpy(updateBuffer+strlen(updateBuffer), ", ");
      dtostrf(temperatures[i], 0, 2, updateBuffer+strlen(updateBuffer));
    }
  }

  Serial.println(updateBuffer);

  if(logging) {
    sd::log(updateBuffer);
  }
}
  
static void updateData() {
  DS3231_get(&rtcTime);

  ambient = ambientSensor.readTempC16(AMBIENT) / 16.0;
  
  readTemperatures();
  
  writeOutputs();

  updateGraph(temperatures);
}

// This function is called periodically, and performs slow tasks:
// Taking measurements
// Updating the screen
void loop() {
  bool needsRefresh = false;
  
  // If enough time has passed, take a new data point
  if(millis() >= lastLogTime + logIntervals[logInterval]*1000) {

    lastLogTime += logIntervals[logInterval];
    
    // If we've gotten out of sync, reset the last log time to now
    if(abs(lastLogTime - millis()) > 100) {
      lastLogTime = millis();
    }

    updateData();
    
    needsRefresh = true;
  }
  
  if(Buttons::pending()) {
    uint8_t button = Buttons::getPending();
    
    if(button == BUTTON_POWER) { // Disable power
      if(!logging) {
        Serial.print("Powering off!\n");
//        powerOff(u8g);
      }
    }
    else if(button == BUTTON_A) { // Start/stop logging
      if(!logging) {
        startLogging();
      }
      else {
        stopLogging();
      }
      
      needsRefresh = true;
    }
    else if(button == BUTTON_B) { // Cycle log interval
      if(!logging) {
        logInterval = (logInterval + 1) % LOG_INTERVAL_COUNT;
        resetGraph();  // Reset the graph, to keep the x axis consistent
        needsRefresh = true;
      }
    }
    else if(button == BUTTON_C) { // Cycle temperature units
      if(!logging) {
        // TODO
        needsRefresh = true;
      }
    }
    else if(button == BUTTON_E) { // Toggle backlight
      backlightEnabled = !backlightEnabled;
      setBacklight(!backlightEnabled);
    }
  }
  
  if(needsRefresh) {
//    uint16_t BATT_STAT_state = digitalRead(BATTERY_STATUS_PIN);
//    DEBUG_PRINT("Battery full = ");
//    DEBUG_PRINTLN(BATT_STAT_state);
    
    if(logging) {
      draw(u8g,
        temperatures, 
        ambient,
        fileName,
        logIntervals[logInterval]
      );
    }
    else {
      draw(u8g,
        temperatures, 
        ambient,
        "Not logging",
        logIntervals[logInterval]
      );
    }
  }
}


ISR(TIMER4_OVF_vect)        // interrupt service routine 
{
  Buttons::buttonTask();
}

