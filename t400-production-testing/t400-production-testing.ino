/*
 * Pax Instruments T400 test firmware
 */

#include "t400.h"  // T400 hardware definitions
#include <SPI.h>  // Arduino SPI library ( http://arduino.cc/en/Reference/SPI )
#include <Wire.h>  // For i2c ( http://arduino.cc/en/Reference/Wire )
#include "U8glib.h"  // Graphics library ( https://github.com/PaxInstruments/u8glib )
#include <Fat16.h>  // Fat16 SD card library ( https://github.com/PaxInstruments/Fat16 )
#include <Fat16util.h>  // use functions to print strings from flash memory
#include <MCP980X.h>  // For MCP9800 ( https://github.com/PaxInstruments/MCP980X )
#include <Streaming.h>  // For MCP9800 maybe. Does not seem needed ( http://arduiniana.org/libraries/streaming )
#include <SPIFlash.h>  // SPI flash https://github.com/PaxInstruments/SPIFlash )
//#include //  MCP3424 ADC ( https://github.com/PaxInstruments/MCP3424 )
//#include //  DS3231 RTC ( https://github.com/PaxInstruments/ds3231 )

U8GLIB_PI13264  u8g(LCD_CS, LCD_A0, LCD_RST); // Define the LCD

SdCard card;
Fat16 file;
#define error(s) error_P(PSTR(s))  // store error strings in flash to save RAM


char* buttonStatus = "    ";
char* backlightStatus = "    ";
char* flashStatus = "    ";
char* lcdStatus = "    ";
char* rtcStatus = "    ";
char* sdStatus = "    ";
char* adcStatus = "    ";
char* mcp9800Status = "    ";

void draw(void) {
  // graphic commands to redraw the complete screen should be placed here
  u8g.setFont(u8g_font_6x10);
  u8g.drawStr( 0, 9, "T400 v" pcbVersion "   start -->");
  u8g.setFont(u8g_font_5x8);
  u8g.drawStr( 0, 18, "Buttons: ");u8g.drawStr( 45, 18, buttonStatus);u8g.drawStr( 75, 18, "LCD: ");u8g.drawStr( 100, 18, lcdStatus);
  u8g.drawStr( 0, 26, "  Flash: ");u8g.drawStr( 45, 26, flashStatus);u8g.drawStr( 75, 26, "RTC: ");u8g.drawStr( 100, 26, rtcStatus);
  u8g.drawStr( 0, 34, "MCP9800: ");u8g.drawStr( 45, 34, mcp9800Status);u8g.drawStr( 75, 34, "ADC: ");u8g.drawStr( 100, 34, adcStatus);
  u8g.drawStr( 0, 42, "Backlit: ");u8g.drawStr( 45, 42, backlightStatus);u8g.drawStr( 75, 42, " SD: ");u8g.drawStr( 100, 42, sdStatus);
  u8g.drawStr( 0, 50, "         ");u8g.drawStr( 45, 50, "    ");u8g.drawStr( 75, 50, "     ");u8g.drawStr( 100, 50, "    ");
  u8g.drawStr( 0, 58, "         ");u8g.drawStr( 45, 58, "    ");u8g.drawStr( 75, 58, "     ");u8g.drawStr( 100, 58, "    ");
}

// Set up the SD error stuff
void error_P(const char* str) {
  PgmPrint("error: ");
  SerialPrintln_P(str);
  if (card.errorCode) {
    PgmPrint("SD error: ");
    Serial.println(card.errorCode, HEX);
  }
  while(1);
}

/*
 * Write an unsigned number to file on SD card.
 * Normally you would use print to format numbers.
 */
void writeNumber(uint32_t n) {
  uint8_t buf[10];
  uint8_t i = 0;
  do {
    i++;
    buf[sizeof(buf) - i] = n%10 + '0';
    n /= 10;
  } while (n);
  file.write(&buf[sizeof(buf) - i], i); // write the part of buf with the number
}

char* buttonTest(void){
// ToDo
// - PASS only on button state change.
// - Use button system from production firmware.
// - Bug: SW_PWR is not detected. Always PASS.

  u8g.setFont(u8g_font_6x10);
  long nextUpdate = millis() + TIMEOUT;
  Serial.print("Button D... ");
  if (digitalRead(BUTTON_D_PIN) == LOW) {
    Serial.println("FAIL... LOW start");
    return "FAIL";
  }
  while( millis() <= nextUpdate){
    // Display question
    u8g.firstPage();  
    do {
      u8g.drawStr( 25, 35, "Button D...");
      u8g.drawStr( 0, 12, "<--");
    } while( u8g.nextPage() );
    if (digitalRead(BUTTON_D_PIN) == LOW) {
      Serial.println("PASS");
      break;
    }
  }
  if (millis() > nextUpdate){
    Serial.println("FAIL... timeout");
    return "FAIL";
  }
  nextUpdate = millis() + TIMEOUT;
  Serial.print("Button E... ");
  if (digitalRead(BUTTON_E_PIN) == LOW) {
    Serial.println("FAIL... LOW start");
    return "FAIL";
  }
  while( millis() <= nextUpdate){
    // Display question
    u8g.firstPage();  
    do {
      u8g.drawStr( 25, 35, "Button E...");
      u8g.drawStr( 0, 40, "<--");
    } while( u8g.nextPage() );
    if (digitalRead(BUTTON_E_PIN) == LOW) {
      Serial.println("PASS");
      break;
    }
  }
  if (millis() > nextUpdate){
    Serial.println("FAIL... timeout");
    return "FAIL";
  }
  nextUpdate = millis() + TIMEOUT;
  Serial.print("Button PWR... ");
  if (digitalRead(BUTTON_POWER_PIN) == LOW) {
    Serial.println("FAIL... LOW start");
  }
  while( millis() <= nextUpdate){
    // Display question
    u8g.firstPage();  
    do {
      u8g.drawStr( 25, 35, "Button PWR...");
      u8g.drawStr( 0, 64, "<--");
    } while( u8g.nextPage() );
    if (digitalRead(BUTTON_POWER_PIN) == LOW) {
      Serial.println("PASS");
      break;
    }
  }
  if (millis() > nextUpdate){
    Serial.println("FAIL... timeout");
    return "FAIL";
  }
  nextUpdate = millis() + TIMEOUT;
  Serial.print("Button A... ");
  if (digitalRead(BUTTON_A_PIN) == LOW) {
    Serial.println("FAIL... LOW start");
    return "FAIL";
  }
  while( millis() <= nextUpdate){
    // Display question
    u8g.firstPage();  
    do {
      u8g.drawStr( 25, 35, "Button A...");
      u8g.drawStr( 115, 12, "-->");
    } while( u8g.nextPage() );
    if (digitalRead(BUTTON_A_PIN) == LOW) {
      Serial.println("PASS");
      break;
    }
  }
  if (millis() > nextUpdate){
    Serial.println("FAIL... timeout");
    return "FAIL";
  }
  nextUpdate = millis() + TIMEOUT;
  Serial.print("Button B...");
  if (digitalRead(BUTTON_B_PIN) == LOW) {
    Serial.println(" FAIL... LOW start");
    return "FAIL";
  }
  while( millis() <= nextUpdate){
    // Display question
    u8g.firstPage();  
    do {
      u8g.drawStr( 25, 35, "Button B...");
      u8g.drawStr( 115, 40, "-->");
    } while( u8g.nextPage() );
    if (digitalRead(BUTTON_B_PIN) == LOW) {
      Serial.println("PASS");
      break;
    }
  }
  if (millis() > nextUpdate){
    Serial.println("FAIL... timeout");
    return "FAIL";
  }
  nextUpdate = millis() + TIMEOUT;
  Serial.print("Button C... ");
  if (digitalRead(BUTTON_C_PIN) == LOW) {
    Serial.println("FAIL... LOW start");
    return "FAIL";
  }
  while( millis() <= nextUpdate){
    // Display question
    u8g.firstPage();  
    do {
      u8g.drawStr( 25, 35, "Button C...");
      u8g.drawStr( 115, 64, "-->");
    } while( u8g.nextPage() );
    if (digitalRead(BUTTON_C_PIN) == LOW) {
      Serial.println("PASS");
      return "PASS";
    }
  }
  if (millis() > nextUpdate){
    Serial.println("FAIL... timeout");
    return "FAIL";
  }
}


char* mcp9800Test(void){
// MCP9800 test
// 1. Attempt to read temperature
// 2. If data is good, PASS, else, FAIL
//
// ToDo
// - Determine what errors can be returned
// - Determine the difference between good data and bad data
//
 Serial.print("JTemp... ");
 DEBUG_PRINTLN(" debugging");
 u8g.setFont(u8g_font_6x10);
 u8g.firstPage();  
    do {
      u8g.drawStr( 25, 35, "Testing JTemp...");
    } while( u8g.nextPage() );

  MCP980X mySensor(0);

  mySensor.begin();        //initialize the hardware

  mySensor.writeConfig(ADC_RES_12BITS);       //max resolution, 0.0625 °C
  mySensor.writeTempC2(HYSTERESIS, 0 * 2);    //freezing
  mySensor.writeTempC2(LIMITSET, 100 * 2);    //boiling

  float C, F;
   
  uint8_t config = mySensor.readConfig();
  C = mySensor.readTempC16(AMBIENT) / 16.0;
  F = mySensor.readTempF10(AMBIENT) / 10.0;
  
  if (C > 5 ) {
    DEBUG_PRINT("  DEBUG: ");DEBUG_PRINT(C);DEBUG_PRINTLN(" C");
    DEBUG_PRINT("JTemp... ");
    Serial.println("PASS");
    return "PASS";
  } else {
    DEBUG_PRINT("  DEBUG: ");DEBUG_PRINT(C);DEBUG_PRINTLN(" C");
    DEBUG_PRINT("JTemp... ");
    Serial.println("FAIL");
    return "FAIL";
  }
}


char* flashTest(void){
// Flash test
// 1. Initialize the flash chip
// 2. If initialization is okay, PASS, else, FAIL.
// 
// Old flash test (not doign this one)
// 1. Write known data to flash
// 2. Read data from flash
// 3. Compare read data with known data
// 4. If same, PASS, else, FAIL

  SPIFlash flash(FLASH_CS, 0xEF40);  // SPI flash chip
  
  u8g.setFont(u8g_font_6x10);
  Serial.print("Flash... ");
  u8g.firstPage();
    do {
      u8g.drawStr( 25, 35, "Flash...");
    } while( u8g.nextPage() );
    
  long nextUpdate = millis() + TIMEOUT;
  DEBUG_PRINTLN("debugging");
  DEBUG_PRINT("  DEBUG: nextUpdate = ");DEBUG_PRINTLN(nextUpdate);
  DEBUG_PRINT("  DEBUG: AVR = ");DEBUG_PRINTLN(AVR);
  DEBUG_PRINT("  DEBUG: SPI.transfer(0) = ");DEBUG_PRINTLN(SPI.transfer(0));
  DEBUG_PRINT("  DEBUG: (SPI.transfer(0)+1) & 1 = ");DEBUG_PRINTLN((SPI.transfer(0)+1) & 1);
  DEBUG_PRINT("  DEBUG: flash.initialize() = ");DEBUG_PRINTLN(flash.initialize());
  while( millis() <= nextUpdate){
    DEBUG_PRINT("  DEBUG: millis() = ");DEBUG_PRINTLN(millis());
    if (flash.initialize()) {
      DEBUG_PRINT("Flash... ");
      Serial.println("PASS");              // *** the flash fails to start. Without the chip, this
      return "PASS";                       // *** function just hangs. Must modify the library.
    } else {
      DEBUG_PRINTLN("  DEBUG: Flash init failed");
      
    }
  }
  DEBUG_PRINT("  DEBUG: millis() = ");DEBUG_PRINT(millis());DEBUG_PRINTLN("  while loop ended");
  if (millis() > nextUpdate){
    Serial.println("FAIL... timeout");
    return "FAIL";
  }
}

char* lcdTest(void){
 u8g.firstPage();  
    do {
      u8g.drawStr( 25, 35, "Testing LCD...");
    } while( u8g.nextPage() );
  Serial.println("LCD... NONE");
  return "NONE";
}

char* rtcTest(void){
 u8g.setFont(u8g_font_6x10);
 u8g.firstPage();  
    do {
      u8g.drawStr( 25, 35, "Testing RTC...");
    } while( u8g.nextPage() );
  Serial.println("RTC... NONE");
  return "NONE";
}

char* sdTest(void){ 
// SD card test
// 1. Write known data to flash
// 2. Read data from flash
// 3. Compare read data with known data
// 4. If same, PASS, else, FAIL
//
// ToDo
// - Determine if we should compare data as strings, chars, or numbers
// - Read data from file
// - Compare to known data

 u8g.setFont(u8g_font_6x10);
 u8g.firstPage();  
    do {
      u8g.drawStr( 25, 35, "Testing SD...");
    } while( u8g.nextPage() );
  Serial.print("SD card... ");
  DEBUG_PRINTLN();
  if (!card.begin(SD_CS)){
    Serial.println("FAIL... no SD card found");
    return "FAIL";
  }
  if (!Fat16::init(&card)){
    Serial.println("FAIL... not a FAT16 filesystem");  // Fails here when there is no SD card
    return "FAIL";
  }
  
  // create a new file
  randomSeed(analogRead(DATA1));  // Generate a random number seed
//  long randNumber = random(1001, 9999);  // Choose a random number
  long randNumber = 1999;
  char name[] = "WRITE00.TXT";
  for (uint8_t i = 0; i < 100; i++) {  // Incriment a new filename if one already exists.
    name[5] = i/10 + '0';
    name[6] = i%10 + '0';
    // O_CREAT - create the file if it does not exist
    // O_EXCL - fail if the file exists
    // O_WRITE - open for write
    if (file.open(name, O_CREAT | O_EXCL | O_WRITE)) break;
  }

  DEBUG_PRINT("    File name: ");DEBUG_PRINTLN(name);
  if (!file.isOpen()) error ("file.open");
    
  writeNumber(randNumber);  // write ranNumber to file
//  file.println();
  file.close();  // close file and force write of all data to the SD card
  
  // Start reading
  if (!card.begin(SD_CS)){
    Serial.println("FAIL... no SD card found");
    return "FAIL";
  }
  if (!Fat16::init(&card)){
    Serial.println("FAIL...  not a FAT16 filesystem");  // Fails here when there is no SD card
    return "FAIL";
  }
  
  // open a file
  if (file.open(name, O_READ)) {
  } else{
    Serial.print("    FAIL... could not open ");Serial.println(name);
    return "FAIL";
  }
  
  // ****** Somewhere I need to compare what is in the file to what it should be.
  // If they are equal, PASS.
  int16_t c;
//  char b[4];
  char* readNum = "0000";
  char* randNum = "0000";
  String str;
  str = String(randNumber);
  str.toCharArray(randNum,5);
  int charInt = 0;
  while ((c = file.read()) > 0){
    readNum[charInt] = char(c);
    charInt++;
  }
  DEBUG_PRINT("    Random number: ");DEBUG_PRINTLN(randNumber);
  DEBUG_PRINT("    Write number: ");DEBUG_PRINTLN(randNum);
  DEBUG_PRINT("    Read number: ");DEBUG_PRINTLN(readNum);
  file.close();
  if ( readNum == randNum){
    DEBUG_PRINT("SD card...");
    Serial.println("PASS");
    return "PASS";
  }else{
    DEBUG_PRINT("SD card...");
    Serial.println("FAIL");
    return "FAIL";
  }
}

char* adcTest(void){
// 
// ADC test
// 1. Attempt to read each voltage
// 2. If data is good, PASS, else, FAIL
//
// ToDo
// - Determine what errors can be returned
// - Determine the difference between good data and bad data
// - Note: Without TCs present we should read a full scale reading

 u8g.setFont(u8g_font_6x10);
 u8g.firstPage();  
    do {
      u8g.drawStr( 25, 35, "Testing ADC...");
    } while( u8g.nextPage() );
  Serial.println("ADC... NONE");
  return "NONE";
}

char* backlightTest(void){
// Backlight test
// 1. Blink backlight
// 2. Ask for feedback
//
// ToDo
// - 


 u8g.setFont(u8g_font_6x10);
 u8g.firstPage();  
    do {
      u8g.drawStr( 25, 35, "Backlight...");
    } while( u8g.nextPage() );
 Serial.print("Backlight... ");
 int delayTime = 100;
 digitalWrite(LCD_BACKLIGHT_PIN, HIGH);
 delay(delayTime);
 digitalWrite(LCD_BACKLIGHT_PIN, LOW);
 delay(delayTime);
 digitalWrite(LCD_BACKLIGHT_PIN, HIGH);
 delay(delayTime);
 digitalWrite(LCD_BACKLIGHT_PIN, LOW);
 delay(delayTime);
 digitalWrite(LCD_BACKLIGHT_PIN, HIGH);
 delay(delayTime);
 digitalWrite(LCD_BACKLIGHT_PIN, LOW);
 delay(delayTime);
 digitalWrite(LCD_BACKLIGHT_PIN, HIGH);
 delay(delayTime);
 digitalWrite(LCD_BACKLIGHT_PIN, LOW);
 delay(delayTime);
 long nextUpdate = millis() + TIMEOUT;
 while( millis() <= nextUpdate){
    u8g.firstPage();  
    do {
      u8g.drawStr( 25, 35, "Backlight blink?");
      u8g.drawStr( 85, 45, "PASS -->");
//      u8g.drawStr( 0, 64, "<-- FAIL");
      u8g.drawStr( 0, 45, "<-- FAIL");
    } while( u8g.nextPage() );
    if (digitalRead(BUTTON_B_PIN) == LOW) {
      Serial.println("PASS");
      digitalWrite(LCD_BACKLIGHT_PIN, LOW);
      return "PASS";
    }else if (digitalRead(BUTTON_E_PIN) == LOW) {
      Serial.println("FAIL");
      digitalWrite(LCD_BACKLIGHT_PIN, LOW);
      return "FAIL";
    }
  }
  Serial.println("FAIL");
  digitalWrite(LCD_BACKLIGHT_PIN, LOW);
  return "FAIL";
}


void setup(void) {
  // Setup pin modes
  pinMode(PWR_ONOFF_PIN, OUTPUT);
  digitalWrite(PWR_ONOFF_PIN, HIGH);  // Enable device power
  pinMode(LCD_BACKLIGHT_PIN, OUTPUT);
  digitalWrite(LCD_BACKLIGHT_PIN, LOW);  // Turn on backlight
  pinMode(BUTTON_A_PIN, INPUT);
  pinMode(BUTTON_B_PIN, INPUT);
  pinMode(BUTTON_C_PIN, INPUT);
  pinMode(BUTTON_D_PIN, INPUT);
  pinMode(BUTTON_E_PIN, INPUT);
  pinMode(BUTTON_POWER_PIN, INPUT);
  // ****** It looks like the pullup resistor is enabled. This is the RXLED ******
  
  u8g.setRot180();  // rotate screen
  u8g.setContrast(150);
  
  Serial.begin(9600);
}

int firstTime = 1;
void loop(void) {
  // Display results
  u8g.firstPage();  
  do {
    draw();
  } while( u8g.nextPage() );
  
  if (firstTime == 1){
    long timeout = millis() + TIMEOUT;
    while( millis() <= timeout ){
      if (digitalRead(BUTTON_A_PIN) == LOW) {
        break;
      }
    }
    firstTime = 0;
  } else {
    while ( true ){
      if (digitalRead(BUTTON_A_PIN) == LOW) {
        break;
      }
    }
  }
  
  
  Serial.println();
  Serial.println("T400 v" pcbVersion " test    ");
  Serial.println("==============    ");
  
  // Run all the tests.
  // This should probably be moved to the loop() function.
  buttonStatus = buttonTest();  // Do button test. Requires onscreen instructions.
  backlightStatus = backlightTest();  // Do backlight test
  lcdStatus = lcdTest();  // Do LCD test
  flashStatus = flashTest();  // Do SPI flash test
  rtcStatus = rtcTest();  // Do RTC test
  sdStatus = sdTest();  // Do SD card test
  adcStatus = adcTest();  // Do ADC test
  mcp9800Status = mcp9800Test();  // Do ADC test
  
  Serial.println("Test complete!");
}

