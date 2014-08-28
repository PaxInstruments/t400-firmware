#ifndef SD_LOG_H
#define SD_LOG_H


#include <SD.h>

//// TODO: Why are these needed here?
//#include <Fat16.h> // FAT16 CD card library
//#include <Fat16util.h>

#define error(s) error_P(PSTR(s)) // store error strings in flash to save RAM

// Initialize the SD card
// @param fileName File name to save to. If the file already exists, the name will be iterated until
//        an unused file is found.
extern void initSd(char* fileName);

// Close the file on the SD card and disconnect from it
// Call this before powering down the board
extern void closeSd();

// Log a temperature data point to the SD card
// @param timeString (input) String describing the current time
// @param ambient (input) Ambient temperature (units?)
// @param temperatures (input) Array of sensor data measurements. SENSOR_COUNT length. ? units
extern void logToSd(char* timeString, float ambient, float* temperatures);

// Flush the SD card data to disk
// @param force If true, force the data to be synced
extern void syncSd(boolean force);

#endif
