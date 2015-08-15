/*
# t400-firmware

## Overview
Firmware for the Pax Instruments T400 temperature datalogger

## Setup
1. Install the Arduino IDE from http://arduino.cc/en/Main/Software. Use version 1.6
2. Install the following Arduino libraries.
  - U8Glib graphical LCD https://github.com/PaxInstruments/u8glib
  - MCP3424 ADC https://github.com/PaxInstruments/MCP3424
  - MCP980X temperature sensor https://github.com/PaxInstruments/MCP980X
  - DS3231 RTC https://github.com/PaxInstruments/ds3231
  - SdFat library https://github.com/PaxInstruments/SdFat (use the SdFat directory within this repository)
3. Install the Pax Instruments hardware core (unzip it and move it to the hardware/ directory in your Sketches folder):
  - https://github.com/PaxInstruments/ATmega32U4-bootloader
4. Restart Arduino if it was already running
5. In tools->board, set the Arduino board to "Pax Instruments T400".
6. In tools->port, select the serial port corresponding to the T400 (Arduino should identify it correctly)
7. Hold down the power button on the device, then upload the firmware.

*/

// Import libraries
#include <Wire.h>       // i2c
#include <SPI.h>
#include <SdFat.h>

#include "U8glib.h"     // LCD
#include <MCP3424.h>    // ADC
#include <MCP980X.h>    // Ambient/junction temperature sensor
#include <ds3231.h>     // RTC

#include "power.h"            // Manage board power
#include "buttons.h"          // User buttons
#include "typek_constant.h"   // Thermocouple calibration table
#include "functions.h"        // Misc. functions
#include "sd_log.h"           // SD card utilities
#include "t400.h"             // Board definitions

#define BUFF_MAX         80   // Size of the character buffer

// Uncomment this to generate some fake temperature data, for testing the graphing functions.
//#define FAKE_TEMPERATURES

char fileName[] =        "LD0000.CSV";

// MCP3424 for thermocouple measurements
MCP3424      thermocoupleAdc(MCP3424_ADDR, MCP342X_GAIN_X8, MCP342X_16_BIT);  // address, gain, resolution

MCP980X ambientSensor(0);      // Ambient temperature sensor


const uint8_t temperatureChannels[SENSOR_COUNT] = {1, 0, 3, 2};  // Map of ADC inputs to thermocouple channels
double temperatures[SENSOR_COUNT];  // Current temperature of each thermocouple input
double ambient =  0;        // Ambient temperature

boolean backlightEnabled = true;

#define LOG_INTERVAL_COUNT 7
const uint8_t logIntervals[LOG_INTERVAL_COUNT] = {1, 2, 4, 10, 20, 60, 120};  // Available log intervals, in half seconds
uint8_t logInterval    = 1;       // currently selected log interval
boolean logging = false;          // True if we are currently logging to a file


bool timeToSample = false;      // If true, the display should be redrawn
volatile uint8_t isrTicks = 0;  // Number of 1-second tics that have elapsed since the last sample

struct ts rtcTime;                // Buffer to read RTC time into


uint8_t temperatureUnit;          // Measurement unit for temperature


void rotateTemperatureUnit() {
  // Rotate the unit
  temperatureUnit = (temperatureUnit + 1) % TEMPERATURE_UNITS_COUNT;

  // Reset the graph so we don't have to worry about scaling it
  Display::resetGraph();
  
  // TODO: Convert the current data to new units?
}

// Convert temperature from celcius to the new unit
double convertTemperature(double Celcius) {
  if(temperatureUnit == TEMPERATURE_UNITS_C) {
    return Celcius;
  }
  else if(temperatureUnit == TEMPERATURE_UNITS_K) {
    return Celcius + 273.15;
  }
  else {
    return 9.0/5.0*Celcius+32;
  }
}

// This function runs once. Use it for setting up the program state.
void setup(void) {
  Power::setup();

  Serial.begin(9600);
  
  Wire.begin(); // Start using the Wire library; does the i2c communication.
  
  Backlight::setup();
  Backlight::set(backlightEnabled);
  
  Display::setup();
  Display::resetGraph();
  
  thermocoupleAdc.begin();

  ambientSensor.begin();
  ambientSensor.writeConfig(ADC_RES_12BITS);
  
  // Note that this needs to be called after ambientSensor.begin(), because the working version of
  // that library calls Wire.begin(), which resets this value.
//  TWBR = 12; // TWBR=12 sets the i2c SCK to 200 kHz on an 8 MHz clock. Comment out to run at 100 kHz


  sd::init();

  // Set up the RTC to generate a 1 Hz signal
  pinMode(RTC_INT, INPUT);
  DS3231_init(0);
  
  // And configure the atmega to interrupt on any edge of the 1 Hz signal
  EICRA |= 0x10;    // Configure INT2 to trigger on any edge
  EIMSK |= 0x04;    // and enable the INT2 interrupt

  Buttons::setup();
}

void startLogging() {
  if(logging) {
    return;
  }
  
  logging = true;
  sd::open(fileName);
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
  
  ambient = ambientSensor.readTempC16(AMBIENT) / 16.0;  // Read ambient temperature in C
  
  // ADC read loop: Start a measurement, wait until it is finished, then record it
  for(uint8_t channel = 0; channel < SENSOR_COUNT; channel++) {
    thermocoupleAdc.startMeasurement(temperatureChannels[channel]);
    do {
      // Delay a while. At 16-bit resolution, the ADC can do a speed of 1/15 = .066seconds/cycle
      // Let's wait a little longer than that in case there is set up time for changing channels.
      delay(75);
    } while(!thermocoupleAdc.measurementReady());

    measuredVoltageMv = thermocoupleAdc.getMeasurement();
    temperature = GetTypKTemp(measuredVoltageMv*1000);

    if(temperature == OUT_OF_RANGE) {
      temperatures[channel] = OUT_OF_RANGE;
    }
    else {
      temperatures[channel] = convertTemperature(temperature + ambient);
    }
  }
  
  // Finally, convert ambient to display units
  ambient = convertTemperature(ambient);
}

static void writeOutputs() {
  static char updateBuffer[BUFF_MAX];      // Scratch buffer to write serial/sd output into
 
  // Avoid snprintf() to save 1.4k space 
//  snprintf(updateBuffer, BUFF_MAX, "%02d:%02d:%02d, ", rtcTime.hour, rtcTime.min, rtcTime.sec);
  dtostrf(rtcTime.hour, 2, 0, updateBuffer + 0);
  dtostrf(rtcTime.min,  2, 0, updateBuffer + 3);
  dtostrf(rtcTime.sec,  2, 0, updateBuffer + 6);
  dtostrf(ambient,      1, 2, updateBuffer + 10);
  updateBuffer[2] = updateBuffer[5] = ':';
  updateBuffer[8] = ',';
  updateBuffer[9] = ' ';
 
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

// This function is called periodically, and performs slow tasks:
// Taking measurements
// Updating the screen
void loop() {
  bool needsRefresh = false;  // If true, the display needs to be updated
  
  if(timeToSample) {
    timeToSample = false;

    readTemperatures();
    DS3231_get(&rtcTime);
    writeOutputs();
    Display::updateGraph(temperatures);
    
    needsRefresh = true;
  }
  
  if(Buttons::pending()) {
    uint8_t button = Buttons::getPending();
    
    if(button == Buttons::BUTTON_POWER) { // Disable power
      if(!logging) {
        Serial.print("Powering off!\n");
        Display::clear();
        Backlight::set(0);
        Power::shutdown();
      }
    }
    else if(button == Buttons::BUTTON_A) { // Start/stop logging
      if(!logging) {
        startLogging();
      }
      else {
        stopLogging();
      }
      
      needsRefresh = true;
    }
    else if(button == Buttons::BUTTON_B) { // Cycle log interval
      if(!logging) {
        noInterrupts();
        logInterval = (logInterval + 1) % LOG_INTERVAL_COUNT;
        isrTicks = logIntervals[logInterval]-1; // TODO: This is a magic number
        interrupts();
        
        Display::resetGraph();  // Reset the graph, to keep the x axis consistent
        needsRefresh = true;
      }
    }
    else if(button == Buttons::BUTTON_C) { // Cycle temperature units
      if(!logging) {
        rotateTemperatureUnit();
        needsRefresh = true;
      }
    }
    else if(button == Buttons::BUTTON_D) { // Sensor display mode
      if(!logging) {
        // TODO
        needsRefresh = true;
      }
    }
    else if(button == Buttons::BUTTON_E) { // Toggle backlight
      backlightEnabled = !backlightEnabled;
      Backlight::set(backlightEnabled);
    }
  }
  
  if(needsRefresh) {    
    if(logging) {
      Display::draw(
        temperatures,
        ambient,
        temperatureUnit,
        fileName,
        logIntervals[logInterval]/2,
        ChargeStatus::get()
      );
    }
    else {
      Display::draw(
        temperatures, 
        ambient,
        temperatureUnit,
        "Not logging",
        logIntervals[logInterval]/2,
        ChargeStatus::get()
      );
    }
  }
  
  // Sleep if we don't have a USB connection
  if(ChargeStatus::get() == ChargeStatus::DISCHARGING) {
    Power::sleep();
  }
}


// .5 Hz interrupt from RTC
ISR(INT2_vect)
{
  isrTicks = (isrTicks + 1)%(logIntervals[logInterval]);
  
  if(isrTicks == 0) {
    timeToSample = true;
  }
}

