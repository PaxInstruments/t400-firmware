#include "buttons.h"
#include "t400.h"

#include <util/atomic.h>

uint8_t buttonPins[BUTTON_COUNT] = {
  BUTTON_A_PIN,
  BUTTON_B_PIN,
  BUTTON_C_PIN,
  BUTTON_D_PIN,
  BUTTON_E_PIN,
  BUTTON_POWER_PIN,
};

void Buttons::setup() {
  for(uint8_t b = 0; b < BUTTON_COUNT; b++) {
    pinMode(buttonPins[b], INPUT);
  }
  
  pressedButton = BUTTON_COUNT;
  lastPressed = BUTTON_COUNT;
}


// Scan for new button presses
void Buttons::buttonTask() {
  // If a button is currently pressed, don't bother looking for a new one
  if (lastPressed != BUTTON_COUNT) {
    if(digitalRead(buttonPins[lastPressed]) == 0) {
      return;
    }
    lastPressed = BUTTON_COUNT;
  }
  
  for(uint8_t b = 0; b < BUTTON_COUNT; b++) {
    // TODO: Use port access here for speed?
    if (digitalRead(buttonPins[b]) == 0) {
      lastPressed = b;
      pressedButton = b;
      return;
    }
  }
}


bool Buttons::isPressed() {
  return (pressedButton != BUTTON_COUNT);
}

// If a button was pressed, return it!
uint8_t Buttons::getPressed() {
    uint8_t pressed;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      pressed = pressedButton;
      pressedButton = BUTTON_COUNT;
    }

    return pressed;
}

