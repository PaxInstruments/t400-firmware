#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>


namespace Buttons {
  // Button names
  #define BUTTON_A      0 // Start/stop logging
  #define BUTTON_B      1 // Set logging interval
  #define BUTTON_C      2 // Change units
  #define BUTTON_D      3 // Toggle channels
  #define BUTTON_E      4 // Backlight on/off
  #define BUTTON_POWER  5 // Power on/off
  #define BUTTON_COUNT  6  // Number of buttons in the system
  
  extern void setup();
  extern void buttonTask();
  extern bool pending();
  extern uint8_t getPending();
}

#endif // BUTTONARRAY_HH
