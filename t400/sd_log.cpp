#include "sd_log.h"
#include "t400.h"

SdCard card;     // The SD card
Fat16 file;      // The logging file

uint32_t syncTime      = 0;     // time of last sync(), in millis()

void error_P(const char* str) {
  Serial.print("error: ");
  Serial.print(str);
  
  if (card.errorCode) {
    Serial.print("SD error: ");
    Serial.println(card.errorCode, HEX);
  }
  
  // Stop the SD card
  closeSd();
}

void initSd(char* fileName) {

  // initialize the SD card
  if (!card.init()) {
    error("card.init");
    return;
  }

  // initialize a FAT16 volume
  if (!Fat16::init(&card)) {
    error("Fat16::init");
    return;
  }

  // Create LDxxxx.CSV for the lowest value of x.
  for (uint8_t i = 0; i < 1000; i++) {
    fileName[2] = (i/1000) % 10 + '0';
    fileName[3] = (i/100)  % 10 + '0';
    fileName[4] = (i/10)   % 10 + '0';
    fileName[5] = i        % 10 + '0';
    // O_CREAT - create the file if it does not exist
    // O_EXCL - fail if the file exists
    // O_WRITE - open for write only
    if (file.open(fileName, O_CREAT | O_EXCL | O_WRITE)) {
      break;
    }
  }
  
  if (!file.isOpen()) { 
    error ("create");
  }
  
  Serial.println("Logging to:");
  Serial.println(fileName);


  // write data header
  
  // clear write error
  file.writeError = false;
  file.print("time, ambient");

  for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
    file.print(", sens");
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

void logToSd(char* message) {
  if(!file.isOpen()) {
    return;
  }
  
  // log time to file
  file.println(message);

  if (file.writeError) error("write data");
  syncSd(false);
}

void syncSd(boolean force) {
  
  if(!file.isOpen()) {
    return;
  }
  
  //don't sync too often - requires 2048 bytes of I/O to SD card
  
  if (!force && (millis() - syncTime) <  SYNC_INTERVAL) {
    return;
  }
  
  syncTime = millis();
  if (!file.sync()) error("sync");
}
