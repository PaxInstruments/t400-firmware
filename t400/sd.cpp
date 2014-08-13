#include "sd.h"
#include "t400.h"

SdCard card;     // The SD card
Fat16 file;      // The logging file

uint32_t syncTime      = 0;     // time of last sync()

void error_P(const char* str) {
  // Print error codes stored in flash
  PgmPrint("error: ");
  SerialPrintln_P(str);
  if (card.errorCode) {
    PgmPrint("SD error: ");
    Serial.println(card.errorCode, HEX);
  }
  while(1);
}

void initSd(char* fileName) {

  // initialize the SD card
  if (!card.init()) error("card.init");

  // initialize a FAT16 volume
  if (!Fat16::init(&card)) error("Fat16::init");

  // Create LOGGERxy.CSV for the lowest values of x and y.
  for (uint8_t i = 0; i < 100; i++) {
    fileName[2] = i/10 + '0';
    fileName[3] = i%10 + '0';
    // O_CREAT - create the file if it does not exist
    // O_EXCL - fail if the file exists
    // O_WRITE - open for write only
    if (file.open(fileName, O_CREAT | O_EXCL | O_WRITE)) break;
  }
  if (!file.isOpen()) error ("create");
  PgmPrint("Logging to: ");
  Serial.println(fileName);

  // write data header

  // clear write error
  file.writeError = false;
  file.print("millis");

  for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
    file.print(",sens");
    file.print(i, DEC);    
  }
  file.println();  
#if ECHO_TO_SERIAL
  Serial.println();
#endif  //ECHO_TO_SERIAL

  if (file.writeError || !file.sync()) {
    error("write header");
  }
}

void closeSd() {
  if(file.isOpen()) {
    syncSd(true);
    file.close();
  }
}

void logToSd(uint32_t logTime, float ambient, float* temperatures) {
  
  if(file.isOpen()) {
    return;
  }
  
  // log time to file
  file.print(logTime);
  file.write(',');
  file.print(ambient);
  for(uint8_t i = 0; i < SENSOR_COUNT; i++) {
    file.write(',');    
    file.print(temperatures[i]);
  }
  file.println();

  if (file.writeError) error("write data");
  syncSd(false);
}

void syncSd(boolean force) {
  
  if(file.isOpen()) {
    return;
  }
  
  //don't sync too often - requires 2048 bytes of I/O to SD card
  
  if (!force && (millis() - syncTime) <  SYNC_INTERVAL) {
    return;
  }
  
  syncTime = millis();
  if (!file.sync()) error("sync");
}
