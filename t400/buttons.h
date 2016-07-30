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

void setupButtons();
bool buttonPending();
uint8_t buttonGetPending();

#endif // BUTTONARRAY_HH
