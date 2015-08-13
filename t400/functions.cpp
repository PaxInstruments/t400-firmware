#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <avr/io.h>

#include <Arduino.h>
#include "U8glib.h" // LCD
#include "typek_constant.h"
#include "t400.h"
#include "functions.h"


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


// Find the maximum value displayed on the graph
//   int16_t newGraphMax = INT16_MIN;

//   for(uint8_t sensor = 0; sensor < 4; sensor++) {
//     // if the sensor is out of range, don't show it
//     if(temperatures[sensor] == OUT_OF_RANGE) {
//       continue;
//     }

//     int8_t* starting_point = &graph[sensor][graphCurrentPoint];  // Starting address of the graph data
//     int8_t* wrap_point = &graph[sensor][MAXIMUM_GRAPH_POINTS];   // If the address pointer reaches this, reset it to graph[sensor][0]

//     for(uint8_t point = 0; point < graphPoints; point++) {
      
//       // u8g.drawPixel(MAXIMUM_GRAPH_POINTS+12-point,
//       //               *(starting_point++));
//       if ((*(starting_point) > newGraphMax) && (*(starting_point) != GRAPH_INVALID)) {
//         newGraphMax = *(starting_point);
//       }

//       starting_point++;
      
//       if(starting_point == wrap_point) {
//         starting_point = &graph[sensor][0];
//       }
//     }
//   }

//    // TODO: Handle the case where 0 is not the bottom measurement.
//    int16_t graphMin = 0;

//    uint8_t newGraphScale = 1 + (graphMax - graphMin)/50;
   
//    graphStep = 10*newGraphScale;
   
//    if(graphScale != newGraphScale) {
//      // for(int i = sizeof(graph)/(4*sizeof(byte))-1; i>0;i--){
//      //   for(int j = 0; j < 4; j++) {
//      //     graph[j][i] = graph[j][i]*((double)graphScale/newGraphScale);
//      //   }
//      // };
     
//      graphScale = newGraphScale;
//    }

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

void draw(
  U8GLIB_PI13264& u8g,
  double* temperatures,
  double ambient,
  uint8_t temperatureUnit,
  char* fileName,
  uint8_t logInterval,
  uint8_t bStatus
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
      if(bStatus == DISCHARGING) {
        const uint8_t battX = 128;
        u8g.drawLine(battX,   14, battX+3, 14);
        u8g.drawLine(battX,   14, battX,   10);
        u8g.drawLine(battX+3, 14, battX+3, 10);
        u8g.drawLine(battX+1,  9, battX+2,  9);
      
        // TODO
        uint8_t batteryState = 3;  // Battery state 0-4 (0 = empty, 4=full);
        for(uint8_t i = 0; i < batteryState; i++) {
          u8g.drawLine(battX, 13-i, battX+3, 13-i);
        }
      }
      else if(bStatus == NO_BATTERY) {
        const uint8_t battX = 128;
        u8g.drawLine(battX,   10, battX+3, 14);
        u8g.drawLine(battX,   14, battX+3, 10);

      }
      else {
        const uint8_t battX = 128;
        u8g.drawLine(battX,   10, battX+3, 10);
        u8g.drawLine(battX,   14, battX+3, 14);
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

extern u8 USBConnected();

batteryStatus getBatteryStatus() {
  // We want to output one of these states:
//   DISCHARGING = 0,    // VBUS=0
//   CHARGING = 1,       // VBUS=1, BATT_STAT=0
//   CHARGED = 2,        // VBUS=1, BATT_STAT=1, VBATT_SENSE=?
//   NO_BATTERY = 3,     // VBUS=1, BATT_STAT=1, VBATT_SENSE=?

  // register USBSTA - bit 0 - VBUS
  USBCON |= (1<<OTGPADE); //enables VBUS pad
  
//  bool usbConnected = USBSTA & _BV(VBUS);
  bool usbConnected = USBConnected();
  
  bool STAT = digitalRead(BATT_STAT) == LOW;
  uint16_t VBATT = analogRead(VBAT_SENSE);

  if(!usbConnected) {
    return DISCHARGING;
  }
  else if(!STAT) {
    return CHARGING;
  }
  else if(VBATT < 2000) {
    return NO_BATTERY;
  }
  else {
    return CHARGED;
  }
}

double GetTypKTemp(double microVolts){
  // Converts the thermocouple µV reading into some usable °C
  
  // Check if the sensor was disconnected
  if(microVolts == 255.99) {
    return OUT_OF_RANGE;
  }
  
  // Check if it's out of range
  // TODO: Read this once.
  // TODO: Why does minConversion not work?
  double maxConversion = lookupThermocouleData(TEMP_TYPE_K_LENGTH - 1);
  double minConversion = lookupThermocouleData(0);
  
  if(microVolts > maxConversion || microVolts < -200){
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

