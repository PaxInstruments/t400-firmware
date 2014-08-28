/*
 * Function to turn power on and off
 */



#ifndef POWER_H
#define POWER_H

#include "U8glib.h" // LCD

// Enable the board power
extern void powerOn();

// Turn off the power to the board
extern void powerOff(U8GLIB_LM6063& u8g);

#endif
