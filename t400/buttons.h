#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>

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

namespace Buttons {

  void setup();
  bool pending();
  uint8_t getPending();
}

#endif // BUTTONARRAY_HH
