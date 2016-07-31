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


#if 0
// NOTE: This code will make subdirectories to get around a max file
// limit.  the problem is, mkdir takes up around 1k of flash!!! So we
// are not going to use it

#define MAX_DIRECTORIES     100
#define MAX_FILES           100
//#define DEBUG_FILE_SEARCH
bool find_filename(uint8_t * dirid_ptr, uint8_t * fileid_ptr)
{
    char outbuff[30];
    uint8_t dirid,fileid;

    // Start in root
    sd.chdir(1);

    #ifdef DEBUG_FILE_SEARCH
    sprintf(outbuff,"Search dir\n");
    Serial.print(outbuff);
    #endif
    for(dirid=1;dirid<MAX_DIRECTORIES;dirid++)
    {
        // Look for folder DATAx, if it exists see if we can add a file, if
        // it doesn't exist, make it and select this with file id = 1

        #ifdef DEBUG_FILE_SEARCH
        sprintf(outbuff,"Try dir %d\n",dirid);
        Serial.print(outbuff);
        #endif

        sprintf(outbuff,"DATA%d",dirid);
        if(!sd.chdir(outbuff,false))
        {
            // Folder doesn't exist, make it
            if(!sd.mkdir(outbuff,false))
            {
                #ifdef DEBUG_FILE_SEARCH
                sprintf(outbuff,"mkdir failed %d\n",dirid);
                Serial.print(outbuff);
                #endif
                break;
            }
            // We did make the directory
            fileid = 1;
            #ifdef DEBUG_FILE_SEARCH
            sprintf(outbuff,"mkdir good %d\n",dirid);
            Serial.print(outbuff);
            #endif
            break;
        }else{
            // Folder does exist, see how many files are in here

            // Actually go into the folder
            sd.chdir(outbuff,true);

            for(fileid=1;fileid<MAX_FILES;fileid++)
            {
                sprintf(outbuff,"FILE%d",fileid);
                if(!sd.exists(outbuff))
                {
                    // This file doesn't exist, use it
                    //Serial.print(outbuff);
                    #ifdef DEBUG_FILE_SEARCH
                    if(!file.open(outbuff, O_CREAT | O_WRITE | O_EXCL))
                    {
                        Serial.print("write err\n");
                    }
                    file.clearWriteError();
                    file.print("Test");
                    file.println();
                    file.flush();
                    file.close();

                    sprintf(outbuff,"Use file %d\n",fileid);
                    Serial.print(outbuff);
                    #endif

                    break;
                }
            }
            if(fileid<99) break;

            // if we have over 100 files, go to next directory

            #ifdef DEBUG_FILE_SEARCH
            Serial.print("Over 100 files\n");
            #endif

            // got back to root
            sd.chdir(1);

        }
    }

    *dirid_ptr = dirid;
    *fileid_ptr = fileid;

    // got back to root
    sd.chdir(1);

    if(dirid>=99) return false;

    #ifdef DEBUG_FILE_SEARCH
    sprintf(outbuff,"Dir:%d File:%d\n",dirid,fileid);
    Serial.print(outbuff);
    #endif

    return true;
}
#endif

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
    sprintf(fileName,"LD%04d.CSV",i);
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
