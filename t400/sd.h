#ifndef SD_H
#define SD_H

#include <Fat16.h> // FAT16 CD card library
#include <Fat16util.h>

#define error(s) error_P(PSTR(s)) // store error strings in flash to save RAM

// Initialize the SD card
// @param fileName File name to save to. If the file already exists, the name will be iterated until
//        an unused file is found.
extern void initSd(char* fileName);

// Close the file on the SD card and disconnect from it
// Call this before powering down the board
extern void closeSd();

// Log a temperature data point to the SD card
// @param logTime Time that the data point was taken, in millis
// @param ambient Ambient temperature (units?)
// @param temperatures Array of sensor data measurements. SENSOR_COUNT length. ? units
extern void logToSd(uint32_t logTime, float ambient, float* temperatures);

// Flush the SD card data to disk
// @param force If true, force the data to be synced
extern void syncSd(boolean force);

#endif
