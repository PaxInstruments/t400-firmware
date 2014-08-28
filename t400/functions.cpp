#include <Fat16.h> // FAT16 CD card library
#include <Fat16util.h>
#include "U8glib.h" // LCD
#include "typek_constant.h"
#include "t400.h"

void draw(
  U8GLIB_LM6063& u8g,
  float* temperatures,
  float ambient,
  char* fileName,
  byte graph[100][4],
  int length,
  uint8_t logInterval
  ) {
    
  u8g.setFont(u8g_font_5x8); // Select font. See https://code.google.com/p/u8glib/wiki/fontsize
    
  uint8_t page = 0;
  u8g.firstPage();  // Update the screen
  do {
    
    // Graphic commands to redraw the complete screen should be placed here
    static char buf[8];

    //// Draw temperature readings and status bar    
    if(page > 5) {  
      #define LINE_COUNT 6
      const uint8_t lines[LINE_COUNT][4] = {
        { 0,  7, 132,   7}, // hline between temperatures and status bar
        {31,  0,  31,   7}, // vline between TC1 and TC2
        {65,  0,  65,   7}, // vline between TC2 and TC3
        {99,  0,  99,   7}, // vline between TC3 and TC4
      };
      for (uint8_t i = 0; i < LINE_COUNT; i++) {
        u8g.drawLine(lines[i][0], lines[i][1], lines[i][2], lines[i][3]);
      }
      
      // Display temperature readings  
      for(uint8_t i = 0; i < SENSOR_COUNT; i++) {
        u8g.drawStr(i*34,   6,  dtostrf(temperatures[i], 5, 1, buf)); // Display TC1 temperature
      }
      
      // Draw ambient temperature
      u8g.drawStr(0, 15, dtostrf(ambient,5,1,buf));
      u8g.drawStr(25, 15, "C");
    
      // Draw file name
      u8g.drawStr(35,15,fileName);
    
      // Draw interval
      sprintf(buf, "%is", logInterval);
      u8g.drawStr(95, 15, buf);
    
      // Draw battery
      const uint8_t battX = 128;
      u8g.drawLine(battX,   14, battX+3, 14);
      u8g.drawLine(battX,   14, battX,   10);
      u8g.drawLine(battX+3, 14, battX+3, 10);
      u8g.drawLine(battX+1,  9, battX+2,  9);
    
    
      // TODO: Battery state
      //  if(BATT_STAT_state == 1){
      //  }else{
      //    u8g.drawLine(battX,14,battX+3,9);
      //  };
    }
    
    
    //// Draw temperature graph
    if (page < 6) {
      u8g.drawLine( 0, 16, 132,  16);    // hline between status bar and graph
      u8g.drawLine(12, 64,  12,  18);    // Vertical axis
      
#define GRAPH_INTERVALS 5

      //Draw labels on vertical axis1    
      int16_t graph_min = 0;
      int16_t graph_step = 10;
    
      for(uint8_t i = 0; i < GRAPH_INTERVALS; i++) {
        sprintf(buf, "%02i", graph_min + graph_step*i);
        u8g.drawPixel(11,61 - i*10);
        u8g.drawStr(0, 64 - i*10, buf);
      }

      // Draw labels on the right side of graph
      for(byte i=0; i<4; i++){
        u8g.drawStr(113+5*i, graph[0][i]+3, dtostrf(i+1,1,0,buf));
      };
      
      // Display data from graph[][] array to the graph on screen
      for(byte i = 0; i<length/(4*sizeof(byte));i++){
        u8g.drawPixel(length/(4*sizeof(byte))-i+12, graph[i][0]); // TC1
        u8g.drawPixel(length/(4*sizeof(byte))-i+12, graph[i][1]); // TC2
        u8g.drawPixel(length/(4*sizeof(byte))-i+12, graph[i][2]); // TC3
        u8g.drawPixel(length/(4*sizeof(byte))-i+12, graph[i][3]); // TC4
      };
    }
    
    page++;
  } 
  
  while( u8g.nextPage() );
}

float GetTypKTemp(float microVolts){
  // Converts the thermocouple µV reading into some usable °C
  if(microVolts > tempTypK[sizeof(tempTypK)/sizeof(float)]){
    //Serial.println("Too BIG");
    return 300;
  }

  float LookedupValue;
  
  for(int i = 0; i<sizeof(tempTypK)/sizeof(float);i++){
    if(microVolts >= tempTypK[i] && microVolts <= tempTypK[i+1]){
      LookedupValue = (-270 + (i)*10) + ((10 *(microVolts - tempTypK[i])) / ((tempTypK[i+1] - tempTypK[i])));
      break;
    }
  }
  return LookedupValue;
}

