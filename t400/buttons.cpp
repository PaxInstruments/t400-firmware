#include "buttons.h"
#include "t400.h"



const uint8_t buttonPins[BUTTON_COUNT] = {
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

  stuckButtonMask = bitSet(stuckButtonMask,BUTTON_POWER_PIN);
  pendingButtons = 0;
}

uint8_t buttonDebounce = 0;

// Scan for new button presses
void Buttons::buttonTask() {
  
  for(uint8_t b = 0; b < BUTTON_COUNT; b++) {
    if(bitRead(stuckButtonMask, b)) {
      if (digitalRead(buttonPins[b]) == 1) {
        bitClear(stuckButtonMask, b);
      }
    }
    else {
      if (digitalRead(buttonPins[b]) == 0) {
        bitSet(stuckButtonMask, b);
        bitSet(pendingButtons, b);
      }
    }
  }
}


bool Buttons::pending() {
  return (pendingButtons != 0);
}

// If a button was pressed, return it!
uint8_t Buttons::getPending() {
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

