#include "power.h"
#include "t400.h"
#include <Arduino.h>
#include "U8glib.h" // LCD

void powerOn() {
  // Turn the power selector on so the board stays on!
  pinMode(PWR_ONOFF_PIN, OUTPUT);
  digitalWrite(PWR_ONOFF_PIN, LOW);
}

void powerOff(U8GLIB_PI13264& u8g) {

  // Clear the screen
  u8g.firstPage();  
  do {
  } while( u8g.nextPage() );

  // Wait until the power button has been released
  while(digitalRead(BUTTON_POWER_PIN) == 0) {};
  
  // Then wait a little longer just to be safe
  delay(200);
  
  //Then turn off the power
  pinMode(PWR_ONOFF_PIN, INPUT);
}
