/*
  LumberDAQ-0.4 Firmware
 
 Notes
 =====
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
#include "sd.h"               // SD card utilities
#include "t400.h"             // Board definitions

#define BUFF_MAX         128 // Size of the character buffer


char fileName[] =        "LD00.CSV";

// Graphical LCD
// LumberDAQ v0.4 pins: SCK, MOSI, CS, A0, RST
// open u8g_dev_st7565_lm6063.c and set width to 132. Use "u8g.setContrast(0x018*8);"
U8GLIB_LM6063  u8g(A3, A5, A4); // Use HW-SPI

// User buttons
Buttons        userButtons;

// MCP3424 for thermocouple measurements
MCP3424      ADC1(0x69, 0, 2);  // address, gain, resolution

MCP980X ambientSensor(0);      // Ambient temperature sensor

float temperatures[SENSOR_COUNT];
float ambient =  0;        // Ambient temperature

// Graph data
byte graph[100][4]={}; // define the size of the data array

uint32_t logTime       = 0;      // time data was logged


// This function runs once. Use it for setting up the program state.
void setup(void) {
  powerOn();
  
  Serial.begin(9600);
  Wire.begin(); // Start using the Wire library; does the i2c communication.
  
  initSd(fileName);
  
  pinMode(BATTERY_STATUS_PIN, INPUT);

  createDataArray(graph, sizeof(graph)); // Create the data array
  
  #if DEBUG_LCD
    u8g.setContrast(0x018*8); // Set contrast level
  #else
    u8g.setContrast(0x018*6); // Set contrast level
  #endif
  
  u8g.setRot180(); // Rotate screen
  u8g.setColorIndex(1); // Set color mode to binary

  // This will have to be an option later
  pinMode(LCD_BACKLIGHT_PIN, OUTPUT); // Set backlight pin as output
  digitalWrite(LCD_BACKLIGHT_PIN, HIGH); // Turn on backlight

  ambientSensor.begin();
  ambientSensor.writeConfig(ADC_RES_12BITS);

  DS3231_init(DS3231_INTCN);
  
  userButtons.setup();
  
  // Turn on the high-speed interrupt loop
  // TODO: Adjust interrupt speed.
  noInterrupts();
  TCCR4A = 0;
  TCCR4B = 0;
  TCCR4B |= (1 << CS41);    // 8 prescaler 
  TIMSK4 |= (1 << TOIE4);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts
}


// This function is called periodically, and performs slow tasks:
// Taking measurements
// Updating the screen
void loop() {
  // RTC stuff
  char buff[BUFF_MAX];
  struct ts t;
  DS3231_get(&t);
  snprintf(buff, BUFF_MAX, "%02d:%02d:%02d", t.hour, t.min, t.sec);
  Serial.print(buff);
  
  ambient = ambientSensor.readTempC16(AMBIENT) / 16.0;
  temperatures[0] = GetTypKTemp((ADC1.getChannelmV(2))*1000)+ambient;
  temperatures[1] = GetTypKTemp((ADC1.getChannelmV(3))*1000)+ambient;
  temperatures[2] = GetTypKTemp((ADC1.getChannelmV(0))*1000)+ambient;
  temperatures[3] = GetTypKTemp((ADC1.getChannelmV(1))*1000)+ambient;
  
  DEBUG_PRINTLN((ADC1.getChannelmV(2))*1000);
  DEBUG_PRINTLN(GetTypKTemp((ADC1.getChannelmV(2))*1000));
  DEBUG_PRINTLN(temp1);
  
  Serial.print(", ");
  Serial.print(ambient);
  Serial.print(", ");
  for(uint8_t i = 0; i < SENSOR_COUNT; i++) {
    Serial.print(temperatures[i]);
    Serial.print(", ");
  }
  
  
  u8g.firstPage();  // Update the screen
  do {
    draw(u8g,
      temperatures,
      ambient,
      fileName,
      graph,
      sizeof(graph)
    );
  } 
  while( u8g.nextPage() );

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

  // Comment out these debug lines and save 32 bytes
  DEBUG_PRINT("DEBUG: Available RAM = ");
  DEBUG_PRINT(FreeRam());
  DEBUG_PRINTLN(" bytes");
  
#ifdef DEBUG
  Serial.print(("DEBUG: Available RAM = "));
  Serial.print(FreeRam());
  Serial.println(" bytes");
  delay(500);
#endif

  uint32_t m = logTime;
  // wait till time is an exact multiple of LOG_INTERVAL
  do {
    logTime = millis();
  }
  while (m == logTime || logTime % LOG_INTERVAL);
  
  logToSd(logTime, ambient, temperatures);

  //  Read the switches
  uint16_t BATT_STAT_state = digitalRead(BATTERY_STATUS_PIN);
  DEBUG_PRINT("Battery full = ");
  DEBUG_PRINTLN(BATT_STAT_state);
  
  if(userButtons.isPressed()) {
    uint8_t button = userButtons.getPressed();
    Serial.print("Button pressed: ");
    Serial.print(button);
    Serial.print("\n");
    
    if(button == BUTTON_POWER) {
      Serial.print("Powering off!");
      closeSd();
      powerOff();
    }
  }
}


ISR(TIMER4_OVF_vect)        // interrupt service routine 
{
  userButtons.buttonTask();
}

