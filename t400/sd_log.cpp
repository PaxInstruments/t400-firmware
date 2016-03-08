#include "Arduino.h"  // for boolean type
#include "t400.h"
#include "sd_log.h"

#if SD_LOGGING_ENABLED
#include <SdFat.h>
#endif

extern uint8_t temperatureUnit;

namespace sd {

#if SD_LOGGING_ENABLED
SdFat sd;
SdFile file;
#endif

uint32_t syncTime      = 0;     // time of last sync(), in millis()

void init() {
  close();
  #if SD_LOGGING_ENABLED
  if (!sd.begin(SD_CS, SPI_FULL_SPEED)) {
//    error_P("card.init");
    return;
  }
  #endif
}

bool open(char* fileName) {
  // Create LDxxxx.CSV for the lowest value of x.

  #if SD_LOGGING_ENABLED
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
//    error_P("file open");
    return false;
  }
  file.clearWriteError();

  // write data header
  file.print("time (s)");

  #endif
  #if SERIAL_OUTPUT_ENABLED
  Serial.print("v");
  Serial.println(FIRMWARE_VERSION);

  Serial.print("File: ");
  Serial.println(fileName);
  Serial.print("time (s)");
  #endif

  for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
    #if SD_LOGGING_ENABLED
    file.print(", temp_");
    file.print(i, DEC);
    #endif
    #if SERIAL_OUTPUT_ENABLED
    Serial.print(", temp_");
    Serial.print(i, DEC);
    #endif

    switch(temperatureUnit) {
    case TEMPERATURE_UNITS_C:
      #if SD_LOGGING_ENABLED
      file.print(" (C)");
      #endif
      #if SERIAL_OUTPUT_ENABLED
      Serial.print(" (C)");
      #endif
      break;
    case TEMPERATURE_UNITS_F:
      #if SD_LOGGING_ENABLED
      file.print(" (F)");
      #endif
      #if SERIAL_OUTPUT_ENABLED
      Serial.print(" (F)");
      #endif
      break;
    case TEMPERATURE_UNITS_K:
      #if SD_LOGGING_ENABLED
      file.print(" (K)");
      #endif
      #if SERIAL_OUTPUT_ENABLED
      Serial.print(" (K)");
      #endif
      break;
    }
  }
  #if SD_LOGGING_ENABLED
  file.println();
  file.flush();
  #endif
  #if SERIAL_OUTPUT_ENABLED
  Serial.println();
  #endif

  #if SD_LOGGING_ENABLED
  return (file.getWriteError() == false);
  #else
    return true;
  #endif
}

void close() {
  #if SD_LOGGING_ENABLED
  file.close();
  #endif
}

bool log(char* message) {
  // TODO: Test if file is open first

  // log time to file
  #if SD_LOGGING_ENABLED
  file.println(message);
  #endif

  sync(false);

  #if SD_LOGGING_ENABLED
  return (file.getWriteError() == false);
  #else
    return true;
  #endif
}

void sync(boolean force) {
  // TODO: Test if file is open first?

  //don't sync too often - requires 2048 bytes of I/O to SD card

  if (!force && (millis() - syncTime) <  SYNC_INTERVAL) {
    return;
  }

  syncTime = millis();
  #if SD_LOGGING_ENABLED
  file.flush();
  #endif
}

} // namespace sd
