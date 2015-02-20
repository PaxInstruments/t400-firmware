#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "t400.h"

// Number of intervals in the graph
#define GRAPH_INTERVALS 5


extern void resetGraph();
extern void updateGraph(double* temperatures);

// Lookup table for converting K-type thermocouple measurements into 
// @param microVolt reading from the ADC
// @return Temperature, in ???
extern double GetTypKTemp(double microVolts);

extern void draw(
  U8GLIB_PI13264& u8g,
  double* temperatures,
  double ambient,
  char* fileName,
  uint8_t logInterval
  );
  
  
inline void setupBacklight() {
  pinMode(LCD_BACKLIGHT_PIN, OUTPUT);
} 
  
inline void enableBacklight() {
  digitalWrite(LCD_BACKLIGHT_PIN, LOW);
}

inline void disableBacklight() {
  digitalWrite(LCD_BACKLIGHT_PIN, HIGH);
}

#endif
