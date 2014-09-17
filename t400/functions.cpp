#include <Fat16.h> // FAT16 CD card library
#include <Fat16util.h>
#include "U8glib.h" // LCD
#include "typek_constant.h"
#include "t400.h"

void draw(
  U8GLIB_LM6063& u8g,
  double* temperatures,
  double ambient,
  char* fileName,
  uint8_t graph[100][4],
  uint8_t graphPoints,
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
      u8g.drawLine(12, 64,  12,  18);    // Vertical axis
      
#define GRAPH_INTERVALS 5

      //Draw labels on vertical axis1    
      const int16_t graph_min = 0;
      const int16_t graph_step = 10;
    
      // TODO: prerender these strings?
      for(uint8_t i = 0; i < GRAPH_INTERVALS; i++) {
        u8g.drawPixel(11,61 - i*10);
        u8g.drawStr(0, 64 - i*10, dtostrf(graph_min + graph_step*i,2,0,buf));
      }

      // Draw labels on the right side of graph
      for(uint8_t i=0; i<4; i++){
        u8g.drawStr(113+5*i, graph[0][i]+3, dtostrf(i+1,1,0,buf));
      };
      
      // Display data from graph[][] array to the graph on screen
      for(uint8_t i = 0; i < graphPoints;i++){
        const uint8_t  x = MAXIMUM_GRAPH_POINTS-i+12;        
        const uint8_t* pos = graph[i];
        
        u8g.drawPixel(x, pos[0]); // TC1
        u8g.drawPixel(x, pos[1]); // TC2
        u8g.drawPixel(x, pos[2]); // TC3
        u8g.drawPixel(x, pos[3]); // TC4
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
  if(microVolts > tempTypK[TEMP_TYPE_K_LENGTH - 1] || microVolts < tempTypK[0]){
    return OUT_OF_RANGE;
  }

  double LookedupValue;
  
  for(uint16_t i = 0; i<TEMP_TYPE_K_LENGTH; i++){
    if(microVolts >= tempTypK[i] && microVolts <= tempTypK[i+1]){
      LookedupValue = ((double)-270 + (i)*10) + ((10 *(microVolts - tempTypK[i])) / ((tempTypK[i+1] - tempTypK[i])));
      break;
    }
  }
  return LookedupValue;
}

