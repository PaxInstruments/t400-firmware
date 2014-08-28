/*
  t400-0.4 Firmware
 
 Notes
 =====
 - Use 'Arduino Lilypad USB' as the Arduino type when compiling
 
 - Including a second smaller font adds an additional 876 bytes. Use only one font.
 - Check all the variables. Reduce their size as much as possible.
 - Determine the best ballance between TC lookup table accurace and memory space. 542 bytes of flash, 106 bytes of RAM.
 - Maybe change the floats to ints and do multiplication/division at the output
 - u8glib files needed: u8g_clip.c, u8g_com_api.c, u8g_com_arduino_common.c, u8g_com_arduino_sw_spi.c,
     u8g_delay.c, u8g_dev_st7565_lm6063.c, u8g_font_data.c, u8g_font.c, u8g_line.c, u8g_ll_api.c,
     u8g_page.c, u8g_pb.c, u8g_pb8v1.c, u8g_rot.c, u8g_state.c, u8g.h, U8glib.cpp, U8glib.h
 - Configurable items
   * Units: C, K, F
   * Backlight: off, momentary on, always on
   * Interval: 1s, 10s, etc. Whatever the RTC can mask
   * Graph display: 1, 2, 3, 4, 1234
   * TC subtraction
   * Hold: continue logging, but do not update screen while hold is enabled.
   * Min/max
 - Truncate graph data that falls beyond the graphing area
 - If value is above of below graph, the arrow points down
 - Have a setup menu and the buttons to navigate the screen or have each button dedicated to one function?
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
#include "sd_log.h"               // SD card utilities
#include "t400.h"             // Board definitions

#define BUFF_MAX         40 // Size of the character buffer


char fileName[] =        "LD0000.CSV";

// Graphical LCD
// t400 v0.4 pins: SCK, MOSI, CS, A0, RST
// open u8g_dev_st7565_lm6063.c and set width to 132. Use "u8g.setContrast(0x018*8);"
U8GLIB_LM6063  u8g(A3, A5, A4); // Use HW-SPI


// User buttons
Buttons        userButtons;

// MCP3424 for thermocouple measurements
MCP3424      ADC1(MCP3424_ADDR, MCP342X_GAIN_X4, MCP342X_16_BIT);  // address, gain, resolution

MCP980X ambientSensor(0);      // Ambient temperature sensor

double temperatures[SENSOR_COUNT];
static uint8_t temperatureChannels[SENSOR_COUNT] = {2, 3, 0, 1};
double ambient =  0;        // Ambient temperature

//// Graph data
uint8_t graph[MAXIMUM_GRAPH_POINTS][SENSOR_COUNT]={}; // define the size of the data array
uint8_t graphPoints = 0;        // Number of valid points to graph

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
    u8g.setContrast(0x018*8); // Set contrast level
  #else
    u8g.setContrast(0x018*6); // Set contrast level
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
  
  // Turn on the high-speed interrupt loop
  // TODO: Adjust interrupt speed.
  noInterrupts();
    TCCR4A = 0;
    TCCR4B = 0;
    TCCR4B |= (1 << CS41);    // 8 prescaler 
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
    
    temperatures[i] = GetTypKTemp(ADC1.getMeasurement())*1000 + ambient;
  }
  
  if(logging) {
    snprintf(buff, BUFF_MAX, "%02d:%02d:%02d", t.hour, t.min, t.sec);
    Serial.print(buff);
  
    Serial.print(", ");
    Serial.print(ambient);
  
    for(uint8_t i = 0; i < SENSOR_COUNT; i++) {
      Serial.print(", ");
      Serial.print(temperatures[i]);
    }
    Serial.print("\n");
  
    logToSd(buff, ambient, temperatures);
  }


  // TODO: Don't shift the data here, rotate it during display.
  // Copy the new temperature data points into the graph array
  for(int i = sizeof(graph)/(4*sizeof(byte))-1; i>0;i--){
    graph[i][0] = graph[i-1][0];
    graph[i][1] = graph[i-1][1];
    graph[i][2] = graph[i-1][2];
    graph[i][3] = graph[i-1][3];
  };
  
  graph[0][0] = 64 - temperatures[0] + 5;
  graph[0][1] = 64 - temperatures[1] + 5;
  graph[0][2] = 64 - temperatures[2] + 5;
  graph[0][3] = 64 - temperatures[3] + 5;
  
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
  
  if(userButtons.isPressed()) {
    uint8_t button = userButtons.getPressed();
    
    if(button == BUTTON_POWER) {
      Serial.print("Powering off!");
      stopLogging();
      powerOff(u8g);
    }
    else if(button == BUTTON_A) {
      if(!logging) {
        startLogging();
      }
      else {
        stopLogging();
      }
      
      needsRefresh = true;
    }
    else if(button == BUTTON_B) {
      if(!logging) {
        logInterval = (logInterval + 1) % LOG_INTERVAL_COUNT;
        needsRefresh = true;
      }
    }
    else if(button == BUTTON_E) {
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
        logIntervals[logInterval]
      );
    }
    else {
      draw(u8g,
        temperatures, 
        ambient,
        "Not logging",
        graph,
        graphPoints,
        logIntervals[logInterval]
      );
    }
  }
}


ISR(TIMER4_OVF_vect)        // interrupt service routine 
{
  userButtons.buttonTask();
}

