#include <Fat16.h> // FAT16 CD card library
#include <Fat16util.h>
#include "U8glib.h" // LCD
#include "typek_constant.h"

void createDataArray(byte graph[100][4], int length) {
  // Creates an array to hold the most recent thermocouple readings for graphing

  // TODO: Is this length ok?
  for (byte i=0;i<length/(4*sizeof(byte));i++){
    graph[i][0] = -1;  // 2 - TC1
    graph[i][1] = -1;  // 2 - TC2
    graph[i][2] = -1;  // 2 - TC3
    graph[i][3] = -1;  // 2 - TC4
  };
}

void draw(U8GLIB_LM6063& u8g, float* temperatures, float ambient, char* fileName, byte graph[100][4], int length) {
  // Graphic commands to redraw the complete screen should be placed here

  u8g.setFont(u8g_font_5x8); // Select font. See https://code.google.com/p/u8glib/wiki/fontsize
  
  // Display temperature readings
  char buffer[8];
  u8g.drawStr(0,   6,  dtostrf(temperatures[0], 5, 1, buffer)); // Display TC1 temperature
  u8g.drawStr(34,  6,  dtostrf(temperatures[0], 5, 1, buffer)); // Display TC2 temperature
  u8g.drawStr(68,  6,  dtostrf(temperatures[0], 5, 1, buffer)); // Display TC3 temperature
  u8g.drawStr(102, 6,  dtostrf(temperatures[0], 5, 1, buffer)); // Display TC4 temperature

  // Draw brackets around the thermocouple readings
  u8g.drawLine( 0, 7, 132, 7);  // hline across screen
  u8g.drawLine(31, 0,  31, 7);  // vline between TC1 and TC2
  u8g.drawLine(65, 0,  65, 7);  // vline between TC2 and TC3
  u8g.drawLine(99, 0,  99, 7);  // vline between TC3 and TC4
  
 // u8g.drawLine(0, 16, 132, 16); // hline across screen

  // Draw graph axes
  u8g.drawLine(12, 64, 12, 18); // Vertical axis
  //Draw labels on vertical axis1
  u8g.drawPixel(11,61);
  u8g.drawStr(6,64,"0");
  u8g.drawPixel(11,51);
  u8g.drawStr(6,54,"1");
  u8g.drawPixel(11,41);
  u8g.drawStr(0,44,"2");
  u8g.drawStr(6,44,"0");
  u8g.drawPixel(11,31);
  u8g.drawStr(6,34,"3");
  u8g.drawPixel(11,21);
  u8g.drawStr(6,24,"4");
  //  u8g.drawLine(8,50,sizeof(graph)/(4*sizeof(byte))+8,50);

  // Draw ambient temperature
  u8g.drawStr(0, 15, dtostrf(ambient,5,1,buffer));
  u8g.drawStr(25, 15, "C");
  
  // Draw file name
  u8g.drawStr(35,15,fileName);
  
  // Draw interval
  u8g.drawStr(80,15, "10s");
  
  // Draw battery
  byte battX = 128;
  u8g.drawLine(battX,14,battX+3,14);
  u8g.drawLine(battX,14,battX,10);
  u8g.drawLine(battX+3,14,battX+3,10);
  u8g.drawLine(battX+1,9,battX+2,9);
    
    
  // TODO: Battery state
//  if(BATT_STAT_state == 1){
//  }else{
//    u8g.drawLine(battX,14,battX+3,9);
//  };
  
  // Display data from graph[][] array to the graph on screen
  for(byte i = 0; i<length/(4*sizeof(byte));i++){
    u8g.drawPixel(length/(4*sizeof(byte))-i+12,graph[i][0]); // TC1
    u8g.drawPixel(length/(4*sizeof(byte))-i+12,graph[i][1]); // TC2
    u8g.drawPixel(length/(4*sizeof(byte))-i+12,graph[i][2]); // TC3
    u8g.drawPixel(length/(4*sizeof(byte))-i+12,graph[i][3]); // TC4
  };

  // Draw labels on the right side of graph
  for(byte i=0;i<4;i++){
    u8g.drawStr(113+5*i,graph[0][i]+3,dtostrf(i+1,1,0,buffer));
  };
}

float GetTypKTemp( float microVolts){
  // Converts the thermocouple µV reading into some usable °C

  float LookedupValue = -9999; // TODO: actual float minimum?
  for(unsigned int i = 0; i<sizeof(tempTypK)/sizeof(float);i++){
    if(microVolts >= tempTypK[i] && microVolts <= tempTypK[i+1]){
      LookedupValue = (-270 + (i)*10) + ((10 *(microVolts - tempTypK[i])) / ((tempTypK[i+1] - tempTypK[i])));
      break;
    }
  }
  return LookedupValue;
}

void writeNumber(Fat16& file, uint32_t n) {
  // Not sure. Probably from SD example code. Will probably delete.
  uint8_t buf[10];
  uint8_t i = 0;
  do {
    i++;
    buf[sizeof(buf) - i] = n%10 + '0';
    n /= 10;
  } 
  while (n);
  file.write(&buf[sizeof(buf) - i], i); // write the part of buf with the number
}
