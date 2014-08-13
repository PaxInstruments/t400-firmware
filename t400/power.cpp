#include "power.h"
#include "t400.h"
#include <arduino.h>

void powerOn() {
  // Turn the power selector on so the board stays on!
  pinMode(PWR_ONOFF_PIN, OUTPUT);
  digitalWrite(PWR_ONOFF_PIN, HIGH);
}

void powerOff() {
  digitalWrite(PWR_ONOFF_PIN, LOW);
}
