#include "sd.h"

SdCard card; // The SD card

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

void initSd(Fat16& file, char* fileName, int sensorCount) {

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
    if (file.open(fileName, O_CREAT | O_EXCL | O_WRITE))break;
  }
  if (!file.isOpen()) error ("create");
  PgmPrint("Logging to: ");
  Serial.println(fileName);

  // write data header

  // clear write error
  file.writeError = false;
  file.print("millis");

  for (uint8_t i = 0; i < sensorCount; i++) {
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
