#include <arduino.h>
#include <util/atomic.h>

#include "buttons.h"
#include "t400.h"

// Button definitions
button buttons[BUTTON_COUNT] = {
  {BUTTON_A_PIN,     0},  // Button A, PD4
  {BUTTON_B_PIN,     0},  // Button B, PD6
  {BUTTON_C_PIN,     0},  // Button C, PD7
  {BUTTON_D_PIN,     0},  // Button D, PD2
  {BUTTON_E_PIN,     0},  // Button E, PD3
  {BUTTON_POWER_PIN, 1},  // Power button, PE6
};

void Buttons::setup() {
  for(uint8_t b = 0; b < BUTTON_COUNT; b++) {
    pinMode(buttons[b].pin, INPUT);
  }
  
  pressedButton = BUTTON_COUNT;
  lastPressed = BUTTON_COUNT;
}


// Scan for new button presses
void Buttons::buttonTask() {
  // If a button is currently pressed, don't bother looking for a new one
  if (lastPressed != BUTTON_COUNT) {
    if(digitalRead(buttons[lastPressed].pin) == buttons[lastPressed].inverted) {
      return;
    }
    lastPressed = BUTTON_COUNT;
  }
  
  for(uint8_t b = 0; b < BUTTON_COUNT; b++) {
    // TODO: Use port access here for speed?
    if (digitalRead(buttons[b].pin) == buttons[b].inverted) {
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

