#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <avr/io.h>

#include <Arduino.h>
#include "U8glib.h" // LCD
#include "typek_constant.h"
#include "t400.h"
#include "functions.h"

namespace Display {
  
// Graphical LCD
U8GLIB_PI13264  u8g(LCD_CS, LCD_A0, LCD_RST); // Use HW-SPI
  
//// Graph data
int8_t graph[SENSOR_COUNT][MAXIMUM_GRAPH_POINTS]={}; // Array to hold graph data, in pixels
uint8_t graphCurrentPoint;                           // Index of latest point added to the graph (0,MAXIMUM_GRAPH_POINTS]
uint8_t graphPoints;                                 // Number of valid points to graph

double graphMin;      // Value of the minimum tick mark, in degrees
uint32_t graphScale;    // Number of degrees per pixel in the graph[] array.

uint8_t axisDigits;    // Number of digits to display in the axis labels (ex: '80' -> 2, '1000' -> 4, '-999' -> 4)

double maxTemp = -99999;  // highest temperature ever seen (for setting the scale)
double minTemp = 99999;   // lowest temperature ever seen (for setting the scale)

// Get a reference to the graph point at the specified 
// uint8_t& graphPoint(uint8_t sensor, uint8_t point)
// @param sensor Sensor to use (0-3)
// @param point Index of the graph point to access (0=current point, 1=last point, etc)
// @return reference to the memory address of the specified graph point
#define graphPoint(sensor, point) (graph[sensor][(point + graphCurrentPoint)%MAXIMUM_GRAPH_POINTS])

// const uint8_t yOffset = DISPLAY_HEIGHT - 3;
// Convert a temperature measurement from temperature space to graph space
// uint8_t temperatureToGraphPoint(temperature)
// @param temperature Temperature measurement, in any unit
// @param scale Graph scale, in in degrees per pixel
// @param min Minimum temperature value, in temperature
// @return representation of the temperature in graph space
#define temperatureToGraphPoint(temperature, scale, min) (DISPLAY_HEIGHT - 3 - (temperature-min)/scale*10)
#define graphPointToTemperature(point, scale, min) (((double)(DISPLAY_HEIGHT - 3 - point))*scale/10.0 + min)
//#define rescaleGraphPoint(point, originalScale, originalMin, newScale, newMin) ((point - ))


void resetGraph() {
  graphCurrentPoint = 0;
  graphPoints = 0;
  
  graphMin = 99999;  // TODO: sliding scale?
  graphScale = 1; // in 10ths

  maxTemp = -999999;
  minTemp = 999999;
}

void updateGraph(double* temperatures) {

  // Increment the current graph point (it wraps around)
  if(graphCurrentPoint == 0) {
    graphCurrentPoint = MAXIMUM_GRAPH_POINTS - 1;
  }
  else {
    graphCurrentPoint -= 1;
  }
  
  // Increment the number of stored graph points
  if(graphPoints < MAXIMUM_GRAPH_POINTS) {
    graphPoints++;
  }

  // Test if any of the new temperatures are out of range, and adjust the graph appropriately.

  for(uint8_t sensor = 0; sensor < SENSOR_COUNT; sensor++) {
    if(temperatures[sensor] == OUT_OF_RANGE) {
      continue;
    }

    if(temperatures[sensor] > maxTemp) {
      maxTemp = temperatures[sensor];
    }
  
    if(temperatures[sensor] < minTemp) {
      minTemp = temperatures[sensor];
    }
  }

  double graphMinLast = graphMin;
  uint32_t graphScaleLast = graphScale;

  // Shift the minimum value based on the lowest reading
  if(minTemp < graphMin) {
    graphMin = minTemp;
  }

  // Expand the graph scale based on the current measurements
  if(maxTemp - graphMin > graphScale * 4) {
    graphScale = (maxTemp - graphMin + 3.999) / 4;  // TODO: better rounding strategy
  }

  // TODO: Contract these later

  // If we need to scale or shift the graph, modify the existing readings first
  // TODO: Fix me!
  if(graphMinLast != graphMin || graphScaleLast != graphScale) {
    for(uint8_t sensor = 0; sensor < SENSOR_COUNT; sensor++) {
      for(uint8_t point = 0; point < MAXIMUM_GRAPH_POINTS; point++) {
        if(graph[sensor][point] != GRAPH_INVALID) {
          graph[sensor][point] = temperatureToGraphPoint(
            graphPointToTemperature(graph[sensor][point],graphScaleLast,graphMinLast),
            graphScale,
            graphMin);
        }
      }
    }
  }

  // Record the current readings in the graph
  for(uint8_t sensor = 0; sensor < SENSOR_COUNT; sensor++) {
    graphPoint(sensor, 0) = (temperatures[sensor] ==
      OUT_OF_RANGE) ? GRAPH_INVALID : temperatureToGraphPoint(temperatures[sensor],graphScale,graphMin);
  }

  // Calculate the number of axes digits to display
  if(graphMin + graphScale*4 > 999 || graphMin < -99) {
    axisDigits = 4;
  }
  else if(graphMin + graphScale*4 > 99 || graphMin < -9) {
    axisDigits = 3;
  }
  else {
    axisDigits = 2;
  }
}

void setup() {
  u8g.setContrast(LCD_CONTRAST); // Set contrast level
  
  u8g.setRot180(); // Rotate screen
  u8g.setColorIndex(1); // Set color mode to binary
  u8g.setFont(u8g_font_5x8r); // Select font. See https://code.google.com/p/u8glib/wiki/fontsize
}

void draw(
  double* temperatures,
  double ambient,
  uint8_t temperatureUnit,
  char* fileName,
  uint8_t logInterval,
  ChargeStatus::State bStatus,
  uint8_t batteryLevel
  ) {

  // Graphic commands to redraw the complete screen should be placed here
  static char buf[8];

  uint8_t page = 0;
  u8g.firstPage();  // Update the screen
  do {
    //// Draw temperature graph
    if (page < 6) {
      if(page == 5) {
        u8g.drawLine( 0, 16, 132,  16);    // hline between status bar and graph
      }
      
      // Draw the separator line between axes labels and legend
      u8g.drawLine(CHARACTER_SPACING*axisDigits + 2, DISPLAY_HEIGHT,
                   CHARACTER_SPACING*axisDigits + 2, 18);
    
      // Draw axis labels and marks
      for(uint8_t interval = 0; interval < GRAPH_INTERVALS; interval++) {
        u8g.drawPixel(CHARACTER_SPACING*axisDigits + 1, 61 - interval*10);

        u8g.drawStr(0, DISPLAY_HEIGHT - interval*10,  dtostrf(graphMin + graphScale*interval,axisDigits,0,buf));
      }

      // Draw labels on the right side of graph
      // TODO:scale these correctly?
      for(uint8_t sensor=0; sensor<4; sensor++){
        u8g.drawStr(113+5*sensor, 3 + graphPoint(sensor, 0), dtostrf(sensor+1,1,0,buf));
      };
      
       // Calculate how many graph points to display.
      uint8_t lastPoint = graphPoints;
      
       // If the axis indicies are >2 character length, scale back the graph.
      if(lastPoint > MAXIMUM_GRAPH_POINTS - (axisDigits - 2)*5) {
        lastPoint = MAXIMUM_GRAPH_POINTS - (axisDigits - 2)*5;
      }

      // Draw the temperature graph for each sensor
      for(uint8_t sensor = 0; sensor < 4; sensor++) {
        // if the sensor is out of range, don't show it
        if(temperatures[sensor] == OUT_OF_RANGE) {
          continue;
        }

       int8_t* starting_point = &graph[sensor][graphCurrentPoint];  // Starting address of the graph data
       int8_t* wrap_point = &graph[sensor][MAXIMUM_GRAPH_POINTS];   // If the address pointer reaches this, reset it to graph[sensor][0]

        for(uint8_t point = 0; point < lastPoint; point++) {
          
          u8g.drawPixel(MAXIMUM_GRAPH_POINTS+12-point,
                        *(starting_point++));
          
          if(starting_point == wrap_point) {
            starting_point = &graph[sensor][0];
          }
        }
      }
    }

    //// Draw status bar
    else if(page == 6) {  
      u8g.drawStr(0,  15, dtostrf(ambient,5,1,buf));         // Ambient temperature
      
      if(temperatureUnit == TEMPERATURE_UNITS_C) {
        u8g.drawStr(25, 15, "C");
      }
      else if(temperatureUnit == TEMPERATURE_UNITS_F) {
        u8g.drawStr(25, 15, "F");
      }
      else {
        u8g.drawStr(25, 15, "K");
      }
      
      u8g.drawStr(35, 15,fileName);                          // File name
    
      u8g.drawStr( 95, 15, dtostrf(logInterval,2,0,buf));    // Interval
      u8g.drawStr(105, 15, "s");
    
      // Draw battery
      const uint8_t battX = 128;
      const uint8_t battY = 9;
      if(bStatus == ChargeStatus::DISCHARGING) {
        u8g.drawLine(battX,   14, battX+3, 14);
        u8g.drawLine(battX,   14, battX,   10);
        u8g.drawLine(battX+3, 14, battX+3, 10);
        u8g.drawLine(battX+1,  9, battX+2,  9);
      
        // TODO: charge level
        for(uint8_t i = 0; i < batteryLevel; i++) {
          u8g.drawLine(battX, 13-i, battX+3, 13-i);
        }
      }
      else if(bStatus == ChargeStatus::NO_BATTERY) {
        u8g.drawLine(battX,   battY,   battX,   battY+5);
        u8g.drawLine(battX,   battY,   battX+3, battY);
        u8g.drawLine(battX,   battY+2, battX+2, battY+2);
        u8g.drawLine(battX,   battY+5, battX+3, battY+5);

      }
      else if(bStatus == ChargeStatus::CHARGING) {
        u8g.drawLine(battX,   14, battX+3, 14);
        u8g.drawLine(battX,   14, battX,   10);
        u8g.drawLine(battX+3, 14, battX+3, 10);
        u8g.drawLine(battX+1,  9, battX+2,  9);
      
        static uint8_t batteryState = 0;
        batteryState = (batteryState+1)%5;
        for(uint8_t i = 0; i < batteryState; i++) {
          u8g.drawLine(battX, 13-i, battX+3, 13-i);
        }
      }
      else {  // CHARGED
        u8g.drawLine(battX,   battY+1, battX,   battY+5);
        u8g.drawLine(battX+1, battY,   battX+1, battY+5);
        u8g.drawLine(battX+2, battY,   battX+2, battY+5);
        u8g.drawLine(battX+3, battY+1, battX+3, battY+5);
      }
    }
    
    //// Draw thermocouple readings
    else if (page == 7) {
      #define LINE_COUNT 4
      const uint8_t lines[LINE_COUNT][4] = {
        { 0,  7, 132,   7}, // hline between temperatures and status bar
        {31,  0,  31,   7}, // vline between TC1 and TC2
        {65,  0,  65,   7}, // vline between TC2 and TC3
        {99,  0,  99,   7}, // vline between TC3 and TC4
      };

      for (uint8_t i = 0; i < LINE_COUNT; i++) {
        const uint8_t* pos = lines[i];
        u8g.drawLine(pos[0], pos[1], pos[2], pos[3]);
      }
      
      // Display temperature readings  
      for(uint8_t sensor = 0; sensor < SENSOR_COUNT; sensor++) {
        if(temperatures[sensor] == OUT_OF_RANGE) {
          u8g.drawStr(sensor*34,   6,  " ----");
        }
        else {
          u8g.drawStr(sensor*34,   6,  dtostrf(temperatures[sensor], 5, 1, buf));
        }
      }
    }
    
    page++;
  } 
  
  while( u8g.nextPage() );
}

void clear() {
  // Clear the screen
  u8g.firstPage();  
  do {
  } while( u8g.nextPage() );
}

}

double GetTypKTemp(int32_t microVolts) {
  // Check if it's in range
  if(microVolts > TEMP_TYPE_K_MAX_CONVERSION || microVolts < TEMP_TYPE_K_MIN_CONVERSION){  
    return OUT_OF_RANGE;
  }
  
  double LookedupValue;
  
  // TODO: Binary search here to decrease lookup time
  for(uint16_t i = 0; i<TEMP_TYPE_K_LENGTH; i++){
    int32_t valueLow = lookupThermocouleData(i);
    int32_t valueHigh = lookupThermocouleData(i + 1);
    
    if(microVolts >= valueLow && microVolts <= valueHigh){
      LookedupValue = ((double)-270 + (i)*10) + ((10 *(microVolts - valueLow)) / ((valueHigh - valueLow)));
      break;
    }
  }
  return LookedupValue;
}


namespace ChargeStatus {
  
void setup() {
  // Set VBAT_EN high to enable VBAT_SENSE readings
  pinMode(VBAT_EN, OUTPUT);
  digitalWrite(VBAT_EN, HIGH);

  // Configure the batt stat pin as an input
//  analogReference(DEFAULT);
  pinMode(BATT_STAT, INPUT);
  pinMode(VBAT_SENSE, INPUT);  

  // enable the VBUS pad
  USBCON |= (1<<OTGPADE);
}

State get() {
  // We want to output one of these states:
//   DISCHARGING = 0,    // VBUS=0
//   CHARGING = 1,       // VBUS=1, BATT_STAT<.7V
//   CHARGED = 2,        // VBUS=1, .8V<BATT_STAT<1.2V
//   NO_BATTERY = 3,     // VBUS=1, 3V<BATT_STAT

  bool usbConnected = USBSTA & _BV(VBUS);

  uint16_t battStatCounts = analogRead(BATT_STAT);
  
  #define BATT_CHARGING_COUNTS_MAX  217 // 1024/3.3*.7
  #define BATT_DISCONNECTED_COUNTS_MIN 248 // 1024/3.3*.8
  #define BATT_DISCONNECTED_COUNTS_MAX 372 // 1024/3.3*.1.2

  if(!usbConnected) {
    return DISCHARGING;
  }
  else if(battStatCounts < BATT_CHARGING_COUNTS_MAX) {
    return CHARGING;
  }
  else if(BATT_DISCONNECTED_COUNTS_MIN < battStatCounts
        && battStatCounts < BATT_DISCONNECTED_COUNTS_MAX) {
    return NO_BATTERY;
  }
  else {
    // default to this
    return CHARGED;
  }
}

uint8_t getBatteryLevel() {
  // VBAT_SENSE_V= 34 × VBAT/(34 + 18.7)
  // VBAT_SENSE_COUNTS = VBAT_SENSE_V / 3.3 * 1024
  
  #define VBAT_SENSE_FULL 820   // 4.1V
  #define VBAT_SENSE_EMPTY 720  // 3.6V
  
  // Note: We'll divide this into 5 sections so that the user gets a full battery for a little bit.
  uint16_t vbatSenseCounts = analogRead(VBAT_SENSE);
  uint8_t batteryLevel = ((vbatSenseCounts - VBAT_SENSE_EMPTY)*5)/(VBAT_SENSE_FULL - VBAT_SENSE_EMPTY);
  
  return batteryLevel<5?batteryLevel:4;
}

}
