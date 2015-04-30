#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>


namespace Buttons {
  // Button names

  enum Button {
    BUTTON_A,
    BUTTON_B,
    BUTTON_C,
    BUTTON_D,
    BUTTON_E,
    BUTTON_POWER,
    BUTTON_COUNT
  };

  extern void setup();
  extern void buttonTask();
  extern bool pending();
  extern uint8_t getPending();
}

#endif // BUTTONARRAY_HH
