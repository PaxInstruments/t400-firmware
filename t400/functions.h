#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "t400.h"

// Number of intervals in the graph
#define GRAPH_INTERVALS 5


// Lookup table for converting K-type thermocouple measurements into 
// @param microVolt reading from the ADC
// @return Temperature, in ???
extern double GetTypKTemp(double microVolts);

extern void draw(
  U8GLIB_PI13264& u8g,
  double* temperatures,
  double ambient,
  char* fileName,
  int8_t graph[100][4],
  uint8_t graphPoints,
  uint8_t logInterval,
  uint8_t graphScale,
  int16_t graph_min,
  int16_t graph_step
  );
  
  
inline void setupBacklight() {
  pinMode(LCD_BACKLIGHT_PIN, OUTPUT);
} 
  
inline void enableBacklight() {
  digitalWrite(LCD_BACKLIGHT_PIN, HIGH);
}

inline void disableBacklight() {
  digitalWrite(LCD_BACKLIGHT_PIN, LOW);
}

#endif
