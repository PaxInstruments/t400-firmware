#ifndef SD_LOG_H
#define SD_LOG_H

namespace sd {

// Initialize the SD card
extern void init();

// Open a file for logging
// @param fileName File name to save to. If the file already exists, the name will be iterated until
//        an unused file is found.
extern void open(char* fileName);

// Close the file on the SD card and disconnect from it
// Call this before powering down the board
extern void close();

// Log a message to the SD card
extern void log(char* message);

// Flush the SD card data to disk
// @param force If true, force the data to be synced
extern void sync(boolean force);

}

#endif
