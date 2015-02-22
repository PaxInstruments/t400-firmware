#define __STDC_LIMIT_MACROS
#include <stdint.h>

#include <Arduino.h>
#include "U8glib.h" // LCD
#include "typek_constant.h"
#include "t400.h"
#include "functions.h"

//// Graph data
int8_t graph[MAXIMUM_GRAPH_POINTS][SENSOR_COUNT]={}; // Array to hold graph data
uint8_t graphCurrentPoint;                           // Index of latest point added to the graph (0,MAXIMUM_GRAPH_POINTS]
uint8_t graphPoints;                                 // Number of valid points to graph

#define graphPoint(point, sensor) (graph[(point + graphCurrentPoint)%MAXIMUM_GRAPH_POINTS][sensor])

int16_t graphMin;      // Value of the minimum tick mark
int16_t graphStep;     // Number of degrees per tick mark in the graph
int8_t graphScale;     // Number of degrees per dot in the graph[] array.
uint8_t axisDigits;    // Number of digits to display in the axis labels (ex: '80' -> 2, '1000' -> 4, '-999' -> 4)

void resetGraph() {
  graphCurrentPoint = 0;
  graphPoints = 0;
//  graphPoints = MAXIMUM_GRAPH_POINTS;  // TODO: just for testing the graph display
  
  graphMin = 0;
  graphStep = 20;
  graphScale = 2;
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

  // Record the current readings in the graph
  for(uint8_t sensor = 0; sensor < SENSOR_COUNT; sensor++) {
    graphPoint(0, sensor) = (temperatures[sensor] == OUT_OF_RANGE) ? GRAPH_INVALID : temperatures[sensor]/graphScale;
  }
  
//  // Figure out the correct graph interval
//  // TODO: Base this on current temperatures only?
//  graphMin = INT16_MAX;
//  int16_t graphMax = INT16_MIN;
//  for(uint8_t pos = sizeof(graph)/(4*sizeof(byte))-1; pos>0; pos--){
//    for(uint8_t sensor = 0; sensor < SENSOR_COUNT; sensor++) {
//      if ((graph[pos][sensor] < graphMin) & (graph[pos][sensor] != GRAPH_INVALID)) {
//        graphMin = graph[pos][sensor];
//      }
//      if ((graph[pos][sensor] > graphMax) & (graph[pos][sensor] != GRAPH_INVALID)) {
//        graphMax = graph[pos][sensor];
//      }
//    }
//  };
  
//  // TODO: Handle the case where 0 is not the bottom measurement.
//  graphMin = 0;
//
//  int newGraphScale = 1 + (graphMax - graphMin)/50;
//  
//  graphStep = 10*newGraphScale;
//  
//  if(graphScale != newGraphScale) {
//    for(int i = sizeof(graph)/(4*sizeof(byte))-1; i>0;i--){
//      for(int j = 0; j < 4; j++) {
//        graph[i][j] = graph[i][j]*((double)graphScale/newGraphScale);
//      }
//    };
//    
//    graphScale = newGraphScale;
//  }

  // Calculate the number of axes digits to display
  if(graphMin + graphStep*4 > 999) {
    axisDigits = 4;
  }
  else if(graphMin + graphStep*4 > 99) {
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
  uint8_t logInterval
  ) {
//  digitalWrite(LCD_BACKLIGHT_PIN, HIGH);

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

        u8g.drawStr(0, DISPLAY_HEIGHT - interval*10,  dtostrf(graphMin + graphStep*interval,axisDigits,0,buf));
      }

      // Draw labels on the right side of graph
      // TODO:scale these correctly?
      for(uint8_t sensor=0; sensor<4; sensor++){
        
        u8g.drawStr(113+5*sensor, DISPLAY_HEIGHT - graphPoint(0, sensor), dtostrf(sensor+1,1,0,buf));
      };
      
      // Display data from graph[][] array to the graph on screen
      
      // Calculate how many graph points to display.
      uint8_t lastPoint = graphPoints;
      
      // If the axis indicies are >2 character length, scale back the graph.
      if(lastPoint > MAXIMUM_GRAPH_POINTS - (axisDigits - 2)*5) {
        lastPoint = MAXIMUM_GRAPH_POINTS - (axisDigits - 2)*5;
      }

      // Note: Use pointer arithmatic here to improve speed by 25%
      int8_t* starting_point = graph[graphCurrentPoint];    // Starting address of the graph data
      int8_t* wrap_point = graph[MAXIMUM_GRAPH_POINTS];     // If the address pointer reaches this, reset it to graph[0][0]

      const uint8_t yOffset = DISPLAY_HEIGHT - 3;

      for(uint8_t point = 0; point < lastPoint; point++){
        
        for(uint8_t sensor = 0; sensor < 4; sensor++) {
          u8g.drawPixel(MAXIMUM_GRAPH_POINTS+12-point,
                        yOffset - *(starting_point++));
        }
        
        if(starting_point == wrap_point) {
          starting_point = graph[0];
        }
      };      
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
      u8g.drawLine(battX,   14, battX+3, 14);
      u8g.drawLine(battX,   14, battX,   10);
      u8g.drawLine(battX+3, 14, battX+3, 10);
      u8g.drawLine(battX+1,  9, battX+2,  9);
    
    
      uint8_t batteryState = 3;  // Battery state 0-4 (0 = empty, 4=full);
      for(uint8_t i = 0; i < batteryState; i++) {
        u8g.drawLine(battX, 13-i, battX+3, 13-i);
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
      for(uint8_t i = 0; i < SENSOR_COUNT; i++) {
        if(temperatures[i] == OUT_OF_RANGE) {
          u8g.drawStr(i*34,   6,  " ----");
        }
        else {
          u8g.drawStr(i*34,   6,  dtostrf(temperatures[i], 5, 1, buf));
        }
      }
    }
    
    page++;
  } 
  
  while( u8g.nextPage() );
//  digitalWrite(LCD_BACKLIGHT_PIN, LOW);
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

