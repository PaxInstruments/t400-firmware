/*
 * Function to turn power on and off
 */

#include <avr/sleep.h>
#include "t400.h"

#ifndef POWER_H
#define POWER_H

namespace Power {
  inline void setup() {
    // Turn the power selector on so the board stays on!
    // Note: This is done in the bootloader.
    // TODO: Test the updated bootloader and comment this out
    pinMode(PWR_ONOFF_PIN, OUTPUT);
    digitalWrite(PWR_ONOFF_PIN, LOW);
  }
  
  // Put the processor into sleep mode
  inline void sleep() {
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    cli();
    sleep_enable();
    sei();
    sleep_cpu();
    /* wake up here */
    sleep_disable();
  }
  
  // Turn off the power to the board
  inline void shutdown() {
    // Wait until the power button has been released
    while(digitalRead(BUTTON_POWER_PIN) == HIGH) {};
    
    // Then wait a little longer just to be safe
    delay(200);
    
    //Then turn off the power
    digitalWrite(PWR_ONOFF_PIN, HIGH);
  }
}

#endif
