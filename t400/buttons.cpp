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

  //  SW_A 	Logging start/stop 	INT3 	PD3
  EICRA |= 0x40;    // Configure INT3 to trigger on any edge
  EIMSK |= 0x08;    // and enable the INT3 interrupt
  
  //SW_B 	Logging interval 	INT6 	PD2
  EICRB &= ~0x30;    // Configure INT6 to trigger on low level
  EIMSK |= 0x40;    // and enable the INT6 interrupt

  //SW_C 	Temperature units 	PCINT4 	PB4
  //SW_D 	Toggle channels 	PCINT5 	PB5
  //SW_E 	Backlight               PCINT6 	PB6
  //SW_PWR      Power on/off            PCINT7  PB7
  PCMSK0 |= 0xF0;
  PCICR |= 0x01;
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

// button interrupts
ISR(INT6_vect) { Buttons::buttonTask();}
ISR(INT3_vect) { Buttons::buttonTask();}
ISR(PCINT0_vect) { Buttons::buttonTask();}
