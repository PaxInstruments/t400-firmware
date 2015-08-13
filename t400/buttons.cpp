#include "buttons.h"
#include "t400.h"

namespace Buttons {

uint8_t stuckButtonMask;
uint8_t pendingButtons;


const uint8_t buttonPins[BUTTON_COUNT] = {
  BUTTON_A_PIN,
  BUTTON_B_PIN,
  BUTTON_C_PIN,
  BUTTON_D_PIN,
  BUTTON_E_PIN,
  BUTTON_POWER_PIN,
};

const uint8_t activeState[BUTTON_COUNT] = {
  LOW,
  LOW,
  LOW,
  LOW,
  LOW,
  HIGH,
};

void setup() {

  for(uint8_t b = 0; b < BUTTON_COUNT; b++) {
    pinMode(buttonPins[b], INPUT);
  }

  // If the power button was held down during boot, ignore it.
  // stuckButtonMask = bitSet(stuckButtonMask,BUTTON_POWER_PIN);
  // pendingButtons = 0;
}

uint8_t buttonDebounce = 0;

// Scan for new button presses
void buttonTask() {
  
  for(uint8_t b = 0; b < BUTTON_COUNT; b++) {
    if(bitRead(stuckButtonMask, b)) {
      if (digitalRead(buttonPins[b]) == !activeState[b]) {
        bitClear(stuckButtonMask, b);
      }
    }
    else {
      if (digitalRead(buttonPins[b]) == activeState[b]) {
        bitSet(stuckButtonMask, b);
        bitSet(pendingButtons, b);
      }
    }
  }
}


bool pending() {
  return (pendingButtons != 0);
}

// If a button was pressed, return it!
uint8_t getPending() {
  uint8_t button = BUTTON_COUNT;
  
  noInterrupts();

  for(uint8_t b = 0; b < BUTTON_COUNT; b++) {
    if (bitRead(pendingButtons, b)) {
      bitClear(pendingButtons, b);
      button = b;
      break;
    }
  }
  
  interrupts();
  
  return button;
}


} // namespace buttons
