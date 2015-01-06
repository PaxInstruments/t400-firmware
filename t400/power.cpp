#include "power.h"
#include "t400.h"
#include <Arduino.h>
#include "U8glib.h" // LCD

void powerOn() {
  // Turn the power selector on so the board stays on!
  pinMode(PWR_ONOFF_PIN, OUTPUT);
  digitalWrite(PWR_ONOFF_PIN, HIGH);
}

void powerOff(U8GLIB_PI13264& u8g) {
  digitalWrite(PWR_ONOFF_PIN, LOW);
  
  u8g.firstPage();  
  do {
  } while( u8g.nextPage() );
}
