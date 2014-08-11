#ifndef SD_H
#define SD_H

#include <Fat16.h> // FAT16 CD card library
#include <Fat16util.h>

#define error(s) error_P(PSTR(s)) // store error strings in flash to save RAM

// Functions for interacting with the SD card
extern void initSd(Fat16& file, char* fileName, int sensorCount);

extern void error_P(const char* str);

#endif
