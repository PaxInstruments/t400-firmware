/*
# t400-firmware

## Overview
Firmware for the Pax Instruments T400 temperature datalogger

## Setup
1. Install the Arduino IDE from http://arduino.cc/en/Main/Software. Use version 1.6.7
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

*/


// Import libraries
#include "t400.h"             // Board definitions

#include <Wire.h>             // I2C
#include <SPI.h>

#include "PaxInstruments-U8glib.h"  // LCD
#include <MCP3424.h>          // ADC
#include <MCP980X.h>          // Ambient/junction temperature sensor
#include <ds3231.h>           // RTC
#include "power.h"            // Manage board power
#include "buttons.h"          // User buttons
#include "typek_constant.h"   // Thermocouple calibration table
#include "functions.h"        // Misc. functions
#include "sd_log.h"           // SD card utilities

#define BUFF_MAX         80   // Size of the character buffer

char fileName[] =        "LD0001.CSV";

// MCP3424 for thermocouple measurements
MCP3424 thermocoupleAdc(MCP3424_ADDR, MCP342X_GAIN_X8, MCP342X_16_BIT);  // address, gain, resolution

MCP980X ambientSensor(0);      // Ambient temperature sensor

// Map of ADC inputs to thermocouple channels
const uint8_t temperatureChannels[SENSOR_COUNT] = {1, 0, 3, 2};

int16_t temperatures_int[SENSOR_COUNT] = {OUT_OF_RANGE_INT,OUT_OF_RANGE_INT,
                                          OUT_OF_RANGE_INT,OUT_OF_RANGE_INT};

// Ambient temperature
int16_t ambient =  0;

boolean backlightEnabled = true;

// Available log intervals, in seconds
#define LOG_INTERVAL_COUNT 6
const uint8_t logIntervals[LOG_INTERVAL_COUNT] = {1, 2, 5, 10, 30, 60};

// Timer 1 related variables
#if 0
#define TIMER_ISR_MS        10
uint16_t m_sample_interval_ms = TIMER_ISR_MS;
uint16_t m_timer_isr_counter_limit = 1;
bool m_sample_flag2 = false;     // If true, the display should be redrawn
#endif

uint8_t m_logInterval    = 0; // currently selected log interval
boolean logging = false;    // True if we are currently logging to a file


bool m_sample_flag = false;    // If true, the display should be redrawn

uint8_t isrTick = 0;        // Number of 1-second tics that have elapsed since the last sample
uint8_t lastIsrTick = 0;    // Last tick that we redrew the screen
uint32_t logTimeSeconds;    // Number of seconds that have elapsed since logging began

struct ts rtcTime;          // Buffer to read RTC time into

uint8_t temperatureUnit;    // Measurement unit for temperature

uint8_t graphChannel = 4;


void rotateTemperatureUnit() {
  // Rotate the unit
  temperatureUnit = (temperatureUnit + 1) % TEMPERATURE_UNITS_COUNT;

  // Reset the graph so we don't have to worry about scaling it
  //resetGraph();

  // TODO: Convert the current data to new units?
  return;
}

int16_t convertTemperatureInt(int16_t celcius) {
  switch(temperatureUnit){
  case TEMPERATURE_UNITS_F:
      celcius = celcius*18;
      celcius = celcius/10;
      celcius = celcius + 320;
      return celcius;
  case TEMPERATURE_UNITS_K:
    return celcius + 2732;
  default: break;
  }
  return celcius;
}

// This function runs once. Use it for setting up the program state.
void setup(void) {
  uint8_t x;

  Power::setup();
  ChargeStatus::setup();
  #if SERIAL_OUTPUT_ENABLED
  Serial.begin(9600);
  #endif

  Wire.begin(); // Start using the Wire library; does the i2c communication.

  Backlight::setup();
  Backlight::set(backlightEnabled);

  setupDisplay();
  resetGraph();

  thermocoupleAdc.begin();

  ambientSensor.begin();
  ambientSensor.writeConfig(ADC_RES_12BITS);

  // Set up the RTC to generate a 1 Hz signal
  pinMode(RTC_INT, INPUT);
  DS3231_init(0);

  // And configure the atmega to interrupt on falling edge of the 1 Hz signal
  EICRA |= _BV(ISC21);    // Configure INT2 to trigger on falling edge
  EIMSK |= _BV(INT2);    // and enable the INT2 interrupt

  Buttons::setup();

  //config_sample_time_ms(1000);

  // Kick off the ADC sampling loop
  adc_start_next_conversion();

  return;
}

void startLogging() {

  if(logging) return;

  sd::init();
  logging = sd::open(fileName);
  return;
}

void stopLogging() {
  if(!logging) return;

  logging = false;
  sd::close();
  return;
}

void fake_data(){
    // DEBUG: Fake some data
    #if 0
    // EXTREME!
    temperatures_int[0] = 30000; // 3270.9 C
    temperatures_int[1] = -2732; // -273.2C (abs zero)
    temperatures_int[2] = OUT_OF_RANGE_INT;
    temperatures_int[3] = OUT_OF_RANGE_INT;
    #endif
    #if 0
    temperatures_int[0] = 1234;
    temperatures_int[1] = -345;
    temperatures_int[2] = OUT_OF_RANGE_INT;
    temperatures_int[3] = OUT_OF_RANGE_INT;
    #endif
    #if 0
    #define OFFSET  30.0
    #define SCALE   10.0
    #define ADD     0.05
    static double val=0.0;
    double tmpdbl;
    tmpdbl = ((SCALE*sin(val))+OFFSET)*10;
    temperatures_int[0] = (int16_t)tmpdbl;
    temperatures_int[1] = (int16_t)tmpdbl+5.0;
    val += ADD;
    #endif

    #if 1
    static int16_t val=300;
    static int16_t step=5;
    temperatures_int[0] = val;
    temperatures_int[1] = val+50;
    temperatures_int[2] = val+100;
    temperatures_int[3] = val+150;
    val+=step;
    if(val>=400 || val<= 200) step=step*-1;
    #endif

#if 0
    temperatures_int[0] = convertTemperatureInt(temperatures_int[0]);
    temperatures_int[1] = convertTemperatureInt(temperatures_int[1]);
    temperatures_int[2] = convertTemperatureInt(temperatures_int[2]);
    temperatures_int[3] = convertTemperatureInt(temperatures_int[3]);
#endif

    return;
}

uint8_t m_channel_index = SENSOR_COUNT;
static void adc_start_next_conversion()
{
    m_channel_index = (m_channel_index+1);
    if(m_channel_index>=SENSOR_COUNT) m_channel_index=0;
    thermocoupleAdc.startMeasurement(temperatureChannels[m_channel_index]);
    return;
}

static int16_t adc_read_ambient()
{
    int32_t tmpint32;
    int16_t tmpint16;
    // This gets the temperature as an integer which is °C times 16.
    tmpint32 = ambientSensor.readTempC16(AMBIENT);
    tmpint32 = tmpint32 >> 4; // This divides the temperature by 16
    tmpint16 = (tmpint32 * 10); // Get into 1/10ths
    return tmpint16;
}

static void readTemperatures() {
    int32_t measuredVoltageUv;
    int32_t compensatedVoltage;
    int32_t tmpint32;
    int16_t tmpint16=0;
    float tmpflt;

    // Skip if we don't have a temperature to measure?
    //if(!thermocoupleAdc.measurementReady()) return;

    // This gets the temperature as an integer
    ambient = adc_read_ambient();

    // This function should be called when there is a measurement ready
    // to be read.  This value is the temperature for channel stored
    // in m_channel_index


    // getMeasurementUv returns an int32_t which is the value in micro volts for this channel
    tmpint32 = thermocoupleAdc.getMeasurementUv();

    /******************* Float Math Start ********************/

    // Now we need to calibrate things.  This is y=mx+b
    // Calibration value: MCP3424_OFFSET_CALIBRATION
    tmpflt =  (((float)tmpint32)* MCP3424_CALIBRATION_MULTIPLY) + MCP3424_CALIBRATION_ADD;
    measuredVoltageUv = (uint32_t)tmpflt;

    // Get the measured voltage, removing the ambient junction temperature
    compensatedVoltage = measuredVoltageUv + celcius_to_microvolts( (((float)(ambient))/10.0) );

    // Given a voltage, get the temperature
    tmpflt = microvolts_to_celcius(compensatedVoltage);

    // Convert from float to int
    tmpint16 = ((int16_t)(tmpflt*10));

    /******************* Float Math End ********************/

#if 0
    // Now convert to C, F, K
    if(tmpint16 != OUT_OF_RANGE_INT)
    {
      //temperature = convertTemperature(temperature + ambient_float);
      tmpint16 = convertTemperatureInt(tmpint16);
    }
#endif

    temperatures_int[m_channel_index] = tmpint16;

    // We are done with this channel, kick off the next one
    adc_start_next_conversion();

    return;
}

static void writeOutputs() {

  static char updateBuffer[BUFF_MAX];      // Scratch buffer to write serial/sd output into
  uint8_t index=0;

  index += sprintf(&(updateBuffer[index]),"%d",logTimeSeconds);

  for(uint8_t i = 0; i < SENSOR_COUNT; i++)
  {
    if(temperatures_int[i] == OUT_OF_RANGE_INT)
    {
        index += sprintf(&(updateBuffer[index]), ", -");
    }else {
      index+=sprintf(&(updateBuffer[index]), ", %d.%d",temperatures_int[i]/10,temperatures_int[i]%10);
    }
  }

  #if SERIAL_OUTPUT_ENABLED
  Serial.println(updateBuffer);
  #endif

  if(logging) {
    logging = sd::log(updateBuffer);
  }
  return;
}

// Reset the tick counter, so that a new measurement takes place within 1 second
void resetTicks() {
  noInterrupts();
  isrTick = logIntervals[m_logInterval]-1; // TODO: This is a magic number
  logTimeSeconds = 0;
  interrupts();
  return;
}

// This function is called periodically, and performs slow tasks:
// Taking measurements
// Updating the screen
void loop() {
  bool refresh_display_flag = false;  // If true, the display needs to be updated

  // This will read temperatures as fast as we can, this decouples the
  // slow reading from blocking the rest of the system
  if(thermocoupleAdc.measurementReady())
      readTemperatures();

  // This locks in the samples into the array and does some other stuff. This
  // controls the sample rate of the data
  if(m_sample_flag) {
    m_sample_flag = false;


    // DEBUG, force fake values for testing
    fake_data();

    //DS3231_get(&rtcTime);

    // Write the data to serial AND the SD card
    writeOutputs();

    // Update some graph data.
    updateGraphData(temperatures_int);
    updateGraphScaling();

    // Indicate we want to redraw the display
    refresh_display_flag = true;
  }

  // Check for button presses
  if(Buttons::pending()) {
    uint8_t button = Buttons::getPending();

    switch(button){
    case BUTTON_POWER:
      // Disable power
      if(!logging) {
        clear();
        Backlight::set(0);
        Power::shutdown();
      }
      break;

    case BUTTON_A:
      // Start/stop logging
      #if SD_LOGGING_ENABLED
      // NOTE: Logging takes up 30% of the flash!!!
      if(!logging) {
          // This will block for a bit
          startLogging();
      } else {
          stopLogging();
      }
      resetTicks();
      refresh_display_flag = true;
      #endif
      break;

    case BUTTON_B:
      // Cycle log interval
      if(!logging) {
        m_logInterval = (m_logInterval + 1) % LOG_INTERVAL_COUNT;
        resetTicks();

        resetGraph();  // Reset the graph, to keep the x axis consistent
        resetTicks();
        refresh_display_flag = true;
      }
      break;
    case BUTTON_C:
      // Cycle temperature units
      if(!logging) {
        rotateTemperatureUnit();
        resetTicks();
        refresh_display_flag = true;
      }
      break;
    case BUTTON_D:
      // Sensor display mode
      graphChannel = (graphChannel + 1) % GRAPH_CHANNELS_COUNT;
      while(graphChannel < 4 & temperatures_int[graphChannel] == OUT_OF_RANGE_INT) {
        graphChannel = (graphChannel + 1) % GRAPH_CHANNELS_COUNT;
      }
      refresh_display_flag = true;
      break;
    case BUTTON_E:
      // Toggle backlight
      backlightEnabled = !backlightEnabled;
      Backlight::set(backlightEnabled);
      break;

    default: break;
    } // end button select

  } // end if button pending

  // If we are charging, refresh the display every second to make the charging animation
  if(ChargeStatus::get() == ChargeStatus::CHARGING) {
    if(lastIsrTick != isrTick) {
      refresh_display_flag = true;
      lastIsrTick = isrTick;
    }
  }

  // Draw the display
  if(refresh_display_flag)
  {
    char * ptr = NULL;
    if(logging) ptr = fileName;

    refresh_display_flag = false;

    // Actual draw of display, takes a bit of time
    draw(
      temperatures_int,
      graphChannel,
      temperatureUnit,
      ptr,
      logIntervals[m_logInterval],
      ChargeStatus::get(),
      ChargeStatus::getBatteryLevel()
    );

  }

  // Sleep if we are on battery power
  // Note: Don't sleep if there is power, in case we need to communicate over USB
  if(ChargeStatus::get() == ChargeStatus::DISCHARGING) {
    Power::sleep();
  }

  return;
}


// 1 Hz interrupt from RTC
// TODO: Why not use a timer?
ISR(INT2_vect)
{
  isrTick = (isrTick + 1)%(logIntervals[m_logInterval]);
  logTimeSeconds++;

  if(isrTick == 0) {
    m_sample_flag = true;
  }
  return;
}

/**********************************************************
 * Timer 1 functions
 *********************************************************/
#if 0
void config_sample_time_ms(uint16_t time_ms)
{
    timer1_stop();

    // This is how many ms is in each sample
    m_sample_interval_ms = time_ms;
    // This is the count limit for the timer
    m_timer_isr_counter_limit = (time_ms/TIMER_ISR_MS);
    // Configure the timer
    timer1_setup(TIMER_ISR_MS);

    // Start timer
    timer1_start();

    return;
}

// Setup timer 1 but don't start it
// _clockTimeRes is the number of milliseconds
void timer1_setup(uint8_t _clockTimeRes)
{
    uint16_t tmpu16;
    cli();

    // Clear timer 1
    TCCR1A = 0;
    TCCR1B = 0;

    // set Compare Match value:
    // ATmega32U crystal is 16MHz
    // With prescale = 64
    // timer resolution = 1/( 16000000 /64) = 250000Hz = 0.000004 seconds. 1ms=250 counts

    // target time = timer resolution * (# timer counts + 1)
    // so timer counts = (target time)/(timer resolution) -1
    // For 1 ms interrupt, timer counts = 1E-3/4E-6 - 1 = 249
    tmpu16 = (uint16_t)(_clockTimeRes * 249);

    // DEBUG, 10ms
    tmpu16 = 2490;

    // Maximum time is 263ms
    OCR1A = tmpu16;

    // Turn on CTC mode:
    TCCR1B |= (1 << WGM12);

    // Enable timer compare interrupt:
    TIMSK1 |= (1 << OCIE1A);

    // Interrupt enable
    sei();

    return;
}

void timer1_start()
{
    cli();

    // Start the timer 1 counting, with prescaler 64
    TCCR1B |= (1 << CS11) | (1 << CS10);
    // Start the timer counting, with prescaler 1024
    //TCCR1B |= (1 << CS12) | (1 << CS10);

    sei();
}

void timer1_stop()
{
    cli();
    //Stop the timer counting
    TCCR1B &= 0B11111000;
    sei();
}

void timer1_reset()
{
    cli();
    TCCR1A = 0;  // set all bits of Timer/Counter 1 Control Register to 0
    TCCR1B = 0;
    // Reset the Counter value to 0:
    TCNT1 = 0;
    OCR1A = 0;

    sei();
}

// This code is triggered every time the global clock ticks
uint8_t isr_counter=0;
ISR(TIMER1_COMPA_vect)
{
    isr_counter+=1;
    if(isr_counter>=m_timer_isr_counter_limit)
    {
        isr_counter = 0;
        m_sample_flag2 = true;
    }
    return;
}
#endif

//eof
