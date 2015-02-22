#include <SdFat.h>

#include "t400.h"
#include "sd_log.h"

namespace sd {

SdFat sd;
SdFile file;

uint32_t syncTime      = 0;     // time of last sync(), in millis()

void error_P(const char* str) {
  Serial.print("error: ");
  Serial.print(str);
  
  // Stop the SD card
  close();
}

// TODO: do this every time open() is called, in case the user unplugged the SD card.
void init() {
  
  if (!sd.begin(SD_CS, SPI_HALF_SPEED)) {
    error_P("card.init");
    return;
  }
}

void open(char* fileName) {
  // Create LDxxxx.CSV for the lowest value of x.
  
  uint16_t i = 0;
  do {
    fileName[2] = (i/1000) % 10 + '0';
    fileName[3] = (i/100)  % 10 + '0';
    fileName[4] = (i/10)   % 10 + '0';
    fileName[5] = i        % 10 + '0';
    i++;
  }
  while(sd.exists(fileName));


  if(!file.open(fileName, O_CREAT | O_WRITE | O_EXCL)) {
    error_P("file open");
    return;
  }
  
  Serial.println("Logging to:");
  Serial.println(fileName);

  // write data header
  file.print("time, ambient");

  for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
    file.print(", sens");
    file.print(i, DEC);    
  }
  file.println();
  file.flush();
}

void close() {
  // TODO: Test for error first?
  file.close();
}

void log(char* message) {
  // TODO: Test if file is open first
  
  // log time to file
  file.println(message);

  sync(false);
}

void sync(boolean force) {
  // TODO: Test if file is open first?
  
  //don't sync too often - requires 2048 bytes of I/O to SD card
  
  if (!force && (millis() - syncTime) <  SYNC_INTERVAL) {
    return;
  }
  
  syncTime = millis();
  file.flush();
}

} // namespace sd
