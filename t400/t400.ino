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


char fileName[] =        "LD0000.CSV";

// Graphical LCD
U8GLIB_PI13264  u8g(LCD_CS, LCD_A0, LCD_RST); // Use HW-SPI

// User buttons
Buttons        userButtons;

// MCP3424 for thermocouple measurements
MCP3424      ADC1(MCP3424_ADDR, MCP342X_GAIN_X8, MCP342X_16_BIT);  // address, gain, resolution

MCP980X ambientSensor(0);      // Ambient temperature sensor

double temperatures[SENSOR_COUNT];
static uint8_t temperatureChannels[SENSOR_COUNT] = {1, 0, 3, 2};
double ambient =  0;        // Ambient temperature

//// Graph data
int8_t graph[MAXIMUM_GRAPH_POINTS][SENSOR_COUNT]={}; // define the size of the data array
uint8_t graphPoints = 0;        // Number of valid points to graph

int16_t graphMin = 0;
int16_t graphStep = 10;
int8_t graphScale = 1;    // Number of degrees per dot in the graph[] array.

boolean backlightEnabled;

unsigned long lastLogTime = 0;   // time data was logged
#define LOG_INTERVAL_COUNT 6
uint8_t logIntervals[LOG_INTERVAL_COUNT] = {1, 2, 5, 10, 30, 60};  // Available log intervals, in seconds
uint8_t logInterval    = 1;       // currently selected log interval
boolean logging = false;          // True if we are currently logging to a file


// This function runs once. Use it for setting up the program state.
void setup(void) {
  powerOn();
  
  Serial.begin(9600);
  
  Wire.begin(); // Start using the Wire library; does the i2c communication.
  TWBR = 24; // TWBR=12 sets the i2c SCK to 400 kHz on an 8 MHz clock. Comment out to run at 100 kHz
  
  pinMode(BATTERY_STATUS_PIN, INPUT);
  
  setupBacklight();
  backlightEnabled = true;
  enableBacklight();
  
  // Initialize the data array to a known starting value
  memset(graph, -1, sizeof(graph));
  
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
  
  userButtons.setup();
  
//  for(int point = 0; point < MAXIMUM_GRAPH_POINTS; point++) {
//    graph[point][0] = 0;
//    graph[point][1] = 10;
//    graph[point][2] = 20;
//    graph[point][3] = 40;
//  }
//  graphPoints = MAXIMUM_GRAPH_POINTS;
  
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
  initSd(fileName);
}

void stopLogging() {
  if(!logging) {
    return;
  }
  
  logging = false;
  closeSd();
}

void updateData() {
  // RTC stuff
  char buff[BUFF_MAX];
  struct ts t;
  
  DS3231_get(&t);

  ambient = ambientSensor.readTempC16(AMBIENT) / 16.0;
  
  // ADC read loop: Start a measurement, wait until it is finished, then 
  for(uint8_t i = 0; i < SENSOR_COUNT; i++) {
    ADC1.startMeasurement(temperatureChannels[i]);
    do {
      // Delay a while. At 16-bit resolution, the ADC can do a speed of 1/15 = .066seconds/cycle
      // Let's wait a little longer than that in case there is set up time for changing channels.
      delay(75);
    } while(!ADC1.measurementReady());

    double measuredVoltageMv = ADC1.getMeasurement();

    double temperature = GetTypKTemp(measuredVoltageMv*1000);

    if(temperature == OUT_OF_RANGE) {
      temperatures[i] = OUT_OF_RANGE;
    }
    else {
      temperatures[i] = temperature + ambient;
    }
  }
  
  snprintf(buff, BUFF_MAX, "%02d:%02d:%02d, ", t.hour, t.min, t.sec);
  dtostrf(ambient, 1, 2, buff+strlen(buff));
    
  for(uint8_t i = 0; i < SENSOR_COUNT; i++) {
    if(temperatures[i] == OUT_OF_RANGE) {
      strcpy(buff+strlen(buff), ", -");
    }
    else {
      strcpy(buff+strlen(buff), ", ");
      dtostrf(temperatures[i], 0, 2, buff+strlen(buff));
    }
  }

  Serial.println(buff);

  if(logging) {
    logToSd(buff);
  }

  // TODO: Don't shift the data here, rotate it during display.
  // Copy the new temperature data points into the graph array
  for(uint8_t pos = sizeof(graph)/(4*sizeof(byte))-1; pos>0;pos--){
    for(uint8_t sensor = 0; sensor < SENSOR_COUNT; sensor++) {
      graph[pos][sensor] = graph[pos-1][sensor];
    }
  };

  for(uint8_t sensor = 0; sensor < SENSOR_COUNT; sensor++) {
    if(temperatures[sensor] == OUT_OF_RANGE) {
      graph[0][sensor] == GRAPH_INVALID;
    }
    else {
      graph[0][sensor] = temperatures[sensor]/graphScale;
    }
  }

  
  // Figure out the correct graph interval
  graphMin = 9999;
  int16_t graphMax = -9999;
  for(uint8_t pos = sizeof(graph)/(4*sizeof(byte))-1; pos>0; pos--){
    for(uint8_t sensor = 0; sensor < SENSOR_COUNT; sensor++) {
      if ((graph[pos][sensor] < graphMin) & (graph[pos][sensor] != GRAPH_INVALID)) {
        graphMin = graph[pos][sensor];
      }
      if ((graph[pos][sensor] > graphMax) & (graph[pos][sensor] != GRAPH_INVALID)) {
        graphMax = graph[pos][sensor];
      }
    }
  };
  
  // TODO: Handle the case where 0 is not the bottom measurement.
  graphMin = 0;
  
  int newGraphScale = 1 + (graphMax - graphMin)/50;
  
  graphStep = 10*newGraphScale;

//  Serial.print(graphMin);
//  Serial.print(", ");
//  Serial.print(graphMax);
//  Serial.print(", ");  
//  Serial.print(graphScale);
//  Serial.print(", ");
//  Serial.print(newGraphScale);
//  Serial.print(", ");
//  Serial.println("");
  
  if(graphScale != newGraphScale) {
    for(int i = sizeof(graph)/(4*sizeof(byte))-1; i>0;i--){
      for(int j = 0; j < 4; j++) {
        graph[i][j] = graph[i][j]*((double)graphScale/newGraphScale);
      }
    };
    
    graphScale = newGraphScale;
  }
  
  
  if(graphPoints < MAXIMUM_GRAPH_POINTS) {
    graphPoints++;
  }
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
    
  
//    // TODO: What interval to consider this?
//  #ifdef DEBUG
//    Serial.print(("DEBUG: Available RAM = "));
//    Serial.print(FreeRam());
//    Serial.println(" bytes");
//  #endif
  }
  
  if(userButtons.pending()) {
    uint8_t button = userButtons.getPending();
    
    if(button == BUTTON_POWER) { // Disable power
      Serial.print("Powering off!");
//      stopLogging();
//      powerOff(u8g);
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
        graphPoints = 0; // Reset the graph history (because the scale is different)
        needsRefresh = true;
      }
    }
    else if(button == BUTTON_E) { // Toggle backlight
      if(backlightEnabled) {
        disableBacklight();
        backlightEnabled = false;
      }
      else {
        enableBacklight();
        backlightEnabled = true;
      }
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
        graph,
        graphPoints,
        logIntervals[logInterval],
        graphScale,
        graphMin,
        graphStep
      );
    }
    else {
      draw(u8g,
        temperatures, 
        ambient,
        "Not logging",
        graph,
        graphPoints,
        logIntervals[logInterval],
        graphScale,
        graphMin,
        graphStep
      );
    }
  }
}


ISR(TIMER4_OVF_vect)        // interrupt service routine 
{
  userButtons.buttonTask();
}

