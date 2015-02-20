#define __STDC_LIMIT_MACROS
#include <stdint.h>

#include <Fat16.h> // FAT16 CD card library
#include <Fat16util.h>
#include "U8glib.h" // LCD
#include "typek_constant.h"
#include "t400.h"
#include "functions.h"



//// Graph data
int8_t graph[MAXIMUM_GRAPH_POINTS][SENSOR_COUNT]={}; // Array to hold graph data
uint8_t graphCurrentPoint;                           // Index of latest point added to the graph (0,MAXIMUM_GRAPH_POINTS]
uint8_t graphPoints;                                 // Number of valid points to graph

#define graphPoint(point, sensor) (graph[(point + graphCurrentPoint)%MAXIMUM_GRAPH_POINTS][sensor])

int16_t graphMin;
int16_t graphStep;
int8_t graphScale;    // Number of degrees per dot in the graph[] array.


void resetGraph() {
  graphCurrentPoint = 0;
  graphPoints = 0;
  
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
}



void draw(
  U8GLIB_PI13264& u8g,
  double* temperatures,
  double ambient,
  char* fileName,
  uint8_t logInterval
  ) {
    
  u8g.setFont(u8g_font_5x8); // Select font. See https://code.google.com/p/u8glib/wiki/fontsize
    
  uint8_t page = 0;
  u8g.firstPage();  // Update the screen
  do {
    
    // Graphic commands to redraw the complete screen should be placed here
    static char buf[8];

    //// Draw temperature graph
    if (page < 6) {
      u8g.drawLine( 0, 16, 132,  16);    // hline between status bar and graph
      
      if((graphMin < 0) | (graphMin + graphStep*5 > 99)) {
        // 3 digit display
        u8g.drawLine(15, 64,  15,  18);    // Vertical axis
      }
      else {
        // 2 digit display
        u8g.drawLine(12, 64,  12,  18);    // Vertical axis        
      }
    
      // TODO: prerender these strings?
      for(uint8_t i = 0; i < GRAPH_INTERVALS; i++) {
        u8g.drawPixel(11,61 - i*10);
        if((graphMin < 0) | (graphMin + graphStep*5 > 99)) {
          // 3 digit display
          u8g.drawStr(0, 64 - i*10, dtostrf(graphMin + graphStep*i,3,0,buf));
        }
        else {
          // 2 digit display
          u8g.drawStr(0, 64 - i*10, dtostrf(graphMin + graphStep*i,2,0,buf));
        }
      }

      // Draw labels on the right side of graph
      for(uint8_t i=0; i<4; i++){
        u8g.drawStr(113+5*i, graph[0][i]+3, dtostrf(i+1,1,0,buf));
      };
      
      // Display data from graph[][] array to the graph on screen
      
      uint8_t lastPoint = graphPoints;
      
      // On a 3 digit display, skip the last n points since they would be under the label
      if((graphMin < 0) | (graphMin + graphStep*5 > 99)) {
        // 3 digit display
        if(lastPoint > MAXIMUM_GRAPH_POINTS - 3) {
          lastPoint = MAXIMUM_GRAPH_POINTS - 3;
        }
      }
      
      for(uint8_t point = 0; point < lastPoint; point++){
        const uint8_t  x = MAXIMUM_GRAPH_POINTS-point+12;
        
        const uint8_t yDisplaySize = 64;
        const uint8_t yOffset = yDisplaySize - 3;
        
        for(uint8_t sensor = 0; sensor < 4; sensor++) {
          if(graphPoint(point, sensor) != GRAPH_INVALID) {
            u8g.drawPixel(x, yOffset - graphPoint(point,sensor));
          }
        }
      };
    }

    //// Draw status bar
    else if(page == 6) {  
      u8g.drawStr(0,  15, dtostrf(ambient,5,1,buf));         // Ambient temperature
      u8g.drawStr(25, 15, "C");
    
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
}


double GetTypKTemp(double microVolts){
  // Converts the thermocouple µV reading into some usable °C
  
  // Check if the sensor was disconnected
  if(microVolts == 255.99) {
    return OUT_OF_RANGE;
  }
  
  // Check if it's out of range
  // TODO: Read this once.
  int32_t maxConversion = pgm_read_dword(tempTypK + TEMP_TYPE_K_LENGTH - 1);
  if(microVolts > maxConversion || microVolts < tempTypK[0]){
    return OUT_OF_RANGE;
  }

  double LookedupValue;
  
  for(uint16_t i = 0; i<TEMP_TYPE_K_LENGTH; i++){
    int32_t valueLow = pgm_read_dword(tempTypK + i);
    int32_t valueHigh = pgm_read_dword(tempTypK + i + 1);
    
    if(microVolts >= valueLow && microVolts <= valueHigh){
      LookedupValue = ((double)-270 + (i)*10) + ((10 *(microVolts - valueLow)) / ((valueHigh - valueLow)));
      break;
    }
  }
  return LookedupValue;
}

