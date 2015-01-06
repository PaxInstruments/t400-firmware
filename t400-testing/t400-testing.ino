/*
 * Pax Instruments T400 test firmware
 */

#include "U8glib.h"  // Graphics library
#include "t400.h"  // T400 hardware definitions
#include <Fat16.h>  // Fat16 SD card library
#include <Fat16util.h>  // use functions to print strings from flash memory
#include <MCP980X.h>      // For MCP9800 ( http://github.com/JChristensen/MCP980X )
#include <Streaming.h>    // For MCP9800 ( http://arduiniana.org/libraries/streaming )
#include <Wire.h>         // For MCP9800 ( http://arduino.cc/en/Reference/Wire )


U8GLIB_PI13264  u8g(LCD_CS, LCD_A0, LCD_RST); // Define the LCD

SdCard card;
Fat16 file;
#define error(s) error_P(PSTR(s))  // store error strings in flash to save RAM


char* buttonStatus = "    ";
char* flashStatus = "    ";
char* lcdStatus = "    ";
char* rtcStatus = "    ";
char* sdStatus = "    ";
char* adcStatus = "    ";
char* mcp9800Status = "    ";

void draw(void) {
  // graphic commands to redraw the complete screen should be placed here
  u8g.setFont(u8g_font_6x10);
  u8g.drawStr( 0, 9, "    T400 v" pcbVersion " test    ");
  u8g.setFont(u8g_font_5x8);
  u8g.drawStr( 0, 18, "Buttons: ");u8g.drawStr( 45, 18, buttonStatus);u8g.drawStr( 75, 18, "LCD: ");u8g.drawStr( 100, 18, lcdStatus);
  u8g.drawStr( 0, 26, "  Flash: ");u8g.drawStr( 45, 26, flashStatus);u8g.drawStr( 75, 26, "RTC: ");u8g.drawStr( 100, 26, rtcStatus);
  u8g.drawStr( 0, 34, "MCP9800: ");u8g.drawStr( 45, 34, mcp9800Status);u8g.drawStr( 75, 34, "ADC: ");u8g.drawStr( 100, 34, adcStatus);
  u8g.drawStr( 0, 42, "         ");u8g.drawStr( 45, 42, "    ");u8g.drawStr( 75, 42, " SD: ");u8g.drawStr( 100, 42, sdStatus);
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
/* 
 * ToDo
 * - PASS only on button state change.
 * - Use button system from production firmware.
 * - Bug: SW_PWR is not detected. Always PASS.
 */
  u8g.setFont(u8g_font_6x10);
  long nextUpdate = millis() + TIMEOUT;
  Serial.print("Button D... ");
  while( millis() <= nextUpdate){
    // Display question
    u8g.firstPage();  
    do {
      u8g.drawStr( 0, 9, "    T400 v" pcbVersion " test    ");
      u8g.drawStr( 25, 35, "Button D...");
      u8g.drawStr( 0, 12, "<--");
    } while( u8g.nextPage() );
    if (digitalRead(BUTTON_D_PIN) == LOW) {
      Serial.println("PASS");
      break;
    }
  }
  if (millis() > nextUpdate) return "FAIL";
  nextUpdate = millis() + TIMEOUT;
  Serial.print("Button E... ");
  while( millis() <= nextUpdate){
    // Display question
    u8g.firstPage();  
    do {
      u8g.drawStr( 0, 9, "    T400 v" pcbVersion " test    ");
      u8g.drawStr( 25, 35, "Button E...");
      u8g.drawStr( 0, 40, "<--");
    } while( u8g.nextPage() );
    if (digitalRead(BUTTON_E_PIN) == LOW) {
      Serial.println("PASS");
      break;
    }
  }
  if (millis() > nextUpdate) return "FAIL";
  nextUpdate = millis() + TIMEOUT;
  Serial.print("Button PWR... ");
  while( millis() <= nextUpdate){
    // Display question
    u8g.firstPage();  
    do {
      u8g.drawStr( 0, 9, "    T400 v" pcbVersion " test    ");
      u8g.drawStr( 25, 35, "Button PWR...");
      u8g.drawStr( 0, 64, "<--");
    } while( u8g.nextPage() );
    if (digitalRead(BUTTON_POWER_PIN) == HIGH) {
      Serial.println("PASS");
      break;
    }
  }
  if (millis() > nextUpdate) return "FAIL";
  nextUpdate = millis() + TIMEOUT;
  Serial.print("Button A... ");
  while( millis() <= nextUpdate){
    // Display question
    u8g.firstPage();  
    do {
      u8g.drawStr( 0, 9, "    T400 v" pcbVersion " test    ");
      u8g.drawStr( 25, 35, "Button A...");
      u8g.drawStr( 115, 12, "-->");
    } while( u8g.nextPage() );
    if (digitalRead(BUTTON_A_PIN) == LOW) {
      Serial.println("PASS");
      break;
    }
  }
  if (millis() > nextUpdate) return "FAIL";
  nextUpdate = millis() + TIMEOUT;
  Serial.print("Button B... ");
  while( millis() <= nextUpdate){
    // Display question
    u8g.firstPage();  
    do {
      u8g.drawStr( 0, 9, "    T400 v" pcbVersion " test    ");
      u8g.drawStr( 25, 35, "Button B...");
      u8g.drawStr( 115, 40, "-->");
    } while( u8g.nextPage() );
    if (digitalRead(BUTTON_B_PIN) == LOW) {
      Serial.println("PASS");
      break;
    }
  }
  if (millis() > nextUpdate) return "FAIL";
  nextUpdate = millis() + TIMEOUT;
  Serial.print("Button C... ");
  while( millis() <= nextUpdate){
    // Display question
    u8g.firstPage();  
    do {
      u8g.drawStr( 0, 9, "    T400 v" pcbVersion " test    ");
      u8g.drawStr( 25, 35, "Button C...");
      u8g.drawStr( 115, 64, "-->");
    } while( u8g.nextPage() );
    if (digitalRead(BUTTON_C_PIN) == LOW) {
      Serial.println("PASS");
      return "PASS";
    }
  }
  return "FAIL";
}

char* mcp9800Test(void){
/* 
 * MCP9800 test
 * 1. Attempt to read temperature
 * 2. If data is good, PASS, else, FAIL
 *
 * ToDo
 * - Determine what errors can be returned
 * - Determine the difference between good data and bad data
 */
 u8g.firstPage();  
    do {
      u8g.drawStr( 0, 9, "    T400 v" pcbVersion " test    ");
      u8g.drawStr( 25, 35, "Testing JTemp...");
    } while( u8g.nextPage() );
  Serial.println("JTemp... NONE");
  return "NONE";
}

char* flashTest(void){
/* 
 * Flash test
 * 1. Write known data to flash
 * 2. Read data from flash
 * 3. Compare read data with known data
 * 4. If same, PASS, else, FAIL
 */
 u8g.firstPage();  
    do {
      u8g.drawStr( 0, 9, "    T400 v" pcbVersion " test    ");
      u8g.drawStr( 25, 35, "Testing flash...");
    } while( u8g.nextPage() );
  Serial.println("Flash... NONE");
  return "NONE";
}

char* lcdTest(void){
/* 
 * ToDo
 * 
 */
 u8g.firstPage();  
    do {
      u8g.drawStr( 0, 9, "    T400 v" pcbVersion " test    ");
      u8g.drawStr( 25, 35, "Testing LCD...");
    } while( u8g.nextPage() );
  Serial.println("LCD... NONE");
  return "NONE";
}

char* rtcTest(void){
/* 
 * ToDo
 * 
 */
 u8g.firstPage();  
    do {
      u8g.drawStr( 0, 9, "    T400 v" pcbVersion " test    ");
      u8g.drawStr( 25, 35, "Testing RTC...");
    } while( u8g.nextPage() );
  Serial.println("RTC... NONE");
  return "NONE";
}

char* sdTest(void){
/* 
 * SD card test
 * 1. Write known data to flash
 * 2. Read data from flash
 * 3. Compare read data with known data
 * 4. If same, PASS, else, FAIL
 *
 * ToDo
 * - Determine if we should compare data as strings, chars, or numbers
 * - Read data from file
 * - Compare to known data
 */
 u8g.firstPage();  
    do {
      u8g.drawStr( 0, 9, "    T400 v" pcbVersion " test    ");
      u8g.drawStr( 25, 35, "Testing SD...");
    } while( u8g.nextPage() );
  Serial.print("SD card... ");
  if (!card.begin(SD_CS)){
    Serial.println("FAIL");
    return "FAIL";
  }
  if (!Fat16::init(&card)){
    Serial.println("FAIL");
    return "FAIL";
  }
  
  // create a new file
  randomSeed(analogRead(DATA1));  // Generate a random number seed
  int randNumber = random(1001, 9999);  // Choose a random number
  char name[] = "WRITE00.TXT";
  for (uint8_t i = 0; i < 100; i++) {  // Incriment a new filename if one already exists.
    name[5] = i/10 + '0';
    name[6] = i%10 + '0';
    // O_CREAT - create the file if it does not exist
    // O_EXCL - fail if the file exists
    // O_WRITE - open for write
    if (file.open(name, O_CREAT | O_EXCL | O_WRITE)) break;
  }
  if (!file.isOpen()){
    Serial.println("FAIL");
    return "FAIL";
  }
//  Serial.print("  Writing ");Serial.println(randNumber);
  writeNumber(randNumber);  // write ranNumber to file
//  file.write("\r\n"); // file.println() would work also
  // close file and force write of all data to the SD card
  file.close();
  if (!file.open(name, O_READ)){
    Serial.println("FAIL");
    return "FAIL";
  }
  // ****** Somewhere I need to compare what is in the file to what it should be.
  // If they are equal, PASS.
  int16_t c;
  char b[4];
  char d[4];
  String str;
  str = String(randNumber);
  str.toCharArray(b,5);
//  Serial.print("rand array = ");Serial.println(b);
  int i = 0;
  while ((c = file.read()) > 0){
//    Serial.print(i);Serial.print(" ");Serial.println((char)c);
    d[i] = char(c);
    i++;
  }
//  Serial.print("b = ");Serial.println(sizeof(b));
//  Serial.print("d = ");Serial.println(sizeof(d));
  
//  Serial.print("read array = ");Serial.println(d);
  if ( strcmp (b,d)){
    Serial.println("PASS");
    return "PASS";
  }else{
    Serial.println("FAIL");
    return "FAIL";
  }
}

char* adcTest(void){
/* 
 * ADC test
 * 1. Attempt to read each voltage
 * 2. If data is good, PASS, else, FAIL
 *
 * ToDo
 * - Determine what errors can be returned
 * - Determine the difference between good data and bad data
 * - Note: Without TCs present we should read a full scale reading
 */
 u8g.firstPage();  
    do {
      u8g.drawStr( 0, 9, "    T400 v" pcbVersion " test    ");
      u8g.drawStr( 25, 35, "Testing ADC...");
    } while( u8g.nextPage() );
  Serial.println("ADC... NONE");
  return "NONE";
}

void setup(void) {
  // Setup pin modes
  pinMode(PWR_ONOFF_PIN, OUTPUT);
  digitalWrite(PWR_ONOFF_PIN, HIGH);  // Enable device power
  pinMode(BUTTON_A_PIN, INPUT);
  pinMode(BUTTON_B_PIN, INPUT);
  pinMode(BUTTON_C_PIN, INPUT);
  pinMode(BUTTON_D_PIN, INPUT);
  pinMode(BUTTON_E_PIN, INPUT);
  pinMode(BUTTON_POWER_PIN, INPUT);
  // ****** It looks like the pullup resistor is enabled. This is the RXLED ******
  
  // Display a splash screen
  u8g.setRot180();  // rotate screen
  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_6x10);
    u8g.drawStr( 0, 9, "    T400 v" pcbVersion " test    ");
  } while( u8g.nextPage() );
  delay(1000);
  
  Serial.begin(9600);
  delay(2000);
  Serial.println();
  Serial.println("    T400 v" pcbVersion " test    ");
  Serial.println("    ==============    ");
  Serial.print("Begin in");
  Serial.print(" 3");
  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_6x10);
    u8g.drawStr( 0, 9, "    T400 v" pcbVersion " test    ");
    u8g.drawStr( 0, 20, "    Begin in 3");
  } while( u8g.nextPage() );
  delay(1000);
  Serial.print(" 2");
  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_6x10);
    u8g.drawStr( 0, 9, "    T400 v" pcbVersion " test    ");
    u8g.drawStr( 0, 20, "    Begin in 2");
  } while( u8g.nextPage() );
  delay(1000);
  Serial.println(" 1");
  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_6x10);
    u8g.drawStr( 0, 9, "    T400 v" pcbVersion " test    ");
    u8g.drawStr( 0, 20, "    Begin in 1");
  } while( u8g.nextPage() );
  delay(1000);
  
  // Run all the tests.
  // This should probably be moved to the loop() function.
  buttonStatus = buttonTest();  // Do button test. Requires onscreen instructions.
  flashStatus = flashTest();  // Do SPI flash test
  delay(2000);
  lcdStatus = lcdTest();  // Do LCD test
  delay(2000);
  rtcStatus = rtcTest();  // Do RTC test
  delay(2000);
  sdStatus = sdTest();  // Do SD card test
  delay(2000);
  adcStatus = adcTest();  // Do ADC test
  delay(2000);
  mcp9800Status = mcp9800Test();  // Do ADC test
  delay(2000);
}

void loop(void) {
  // picture loop
  u8g.firstPage();  
  do {
    draw();
  } while( u8g.nextPage() );
  
  // rebuild the picture after some delay
  delay(50);
}

