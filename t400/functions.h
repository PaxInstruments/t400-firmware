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

enum batteryStatus {
  NO_BATTERY,     // VBAT=0 and BATT_STAT = 0?
  CHARGING,       // VBAT<?? and BATT_STAT = 0?
  CHARGED,        // VBUS=connected, VBAT>?? and BAT_STAT = 1
  DISCHARGING     // VBUS=disconnected, VBAT<?? and BATT_STAT = 0?
};

// Get the battery status
// @return 
extern batteryStatus getBatteryStatus();

extern void draw(
  U8GLIB_PI13264& u8g,
  double* temperatures,
  double ambient,
  uint8_t temperatureUnit,
  char* fileName,
  uint8_t logInterval
  );
  
  
inline void setupBacklight() {
  pinMode(LCD_BACKLIGHT_PIN, OUTPUT);
} 
  
inline void setBacklight(uint8_t level) {  
  if(level > 0) {
    digitalWrite(LCD_BACKLIGHT_PIN, LOW);
  }
  else {
    digitalWrite(LCD_BACKLIGHT_PIN, HIGH);
  }
}  

#endif
