#ifndef FUNCTIONS_H
#define FUNCTIONS_H

// Lookup table for converting K-type thermocouple measurements into 
// @param microVolt reading from the ADC
// @return Temperature, in ???
extern float GetTypKTemp(float microVolts);

extern void draw(
  U8GLIB_LM6063& u8g,
  float* temperatures,
  float ambient,
  char* fileName,
  byte graph[100][4],
  int length,
  uint8_t logInterval
  );

#endif
