#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "t400.h"

// Number of intervals in the graph
#define GRAPH_INTERVALS 5

namespace ChargeStatus {
  
  enum State {
     DISCHARGING = 0,    // VBUS=0
     CHARGING = 1,       // VBUS=1, BATT_STAT=0
     CHARGED = 2,        // VBUS=1, BATT_STAT=1, VBATT_SENSE=?
     NO_BATTERY = 3,     // VBUS=1, BATT_STAT=1, VBATT_SENSE=?
  };
  
  // Get the battery status
  // @return 
  State get();
  
  // Get a measurement of the battery leve
  // @return 0 = empty, 4= full
  uint8_t getBatteryLevel();
}


namespace Backlight {
  
  inline void setup() {
    pinMode(LCD_BACKLIGHT_PIN, OUTPUT);
  } 
    
  inline void set(uint8_t level) {  
    if(level > 0) {
      digitalWrite(LCD_BACKLIGHT_PIN, LOW);
    }
    else {
      digitalWrite(LCD_BACKLIGHT_PIN, HIGH);
    }
  }

}

namespace Display {

  void resetGraph();
  void updateGraph(double* temperatures);
  
  void setup();
  
  void draw(
    double* temperatures,
    double ambient,
    uint8_t temperatureUnit,
    char* fileName,
    uint8_t logInterval,
    ChargeStatus::State bStatus,
    uint8_t batteryLevel
    );
  
  void clear();
}

// Lookup table for converting K-type thermocouple measurements into 
// @param microVolt reading from the ADC
// @return Temperature, in ???
double GetTypKTemp(int32_t microVolts);


#endif
