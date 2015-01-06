/*
 * Pax Instruments T400 test firmware
 */

#include "U8glib.h"  // Graphics library
#include "t400-testing.h"  // T400 hardware definitions
#include <Fat16.h>  // Fat16 SD card library
#include <Fat16util.h>  // use functions to print strings from flash memory

U8GLIB_PI13264  u8g(LCD_CS, LCD_A0, LCD_RST); // Define the LCD

#define TIMEOUT 6000  // Define the test timeout interval

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
  u8g.setFont(u8g_font_6x10);
  long nextUpdate = millis() + TIMEOUT;
  while( millis() <= nextUpdate){
    // Display question
    u8g.firstPage();  
    do {
      u8g.drawStr( 0, 9, "    T400 v" pcbVersion " test    ");
      u8g.drawStr( 25, 35, "Testing D...");
      u8g.drawStr( 0, 12, "<--");
    } while( u8g.nextPage() );
    if (digitalRead(BUTTON_D_PIN) == LOW) {
      break;
    }
  }
  if (millis() > nextUpdate) return "FAIL";
  nextUpdate = millis() + TIMEOUT;
  while( millis() <= nextUpdate){
    // Display question
    u8g.firstPage();  
    do {
      u8g.drawStr( 0, 9, "    T400 v" pcbVersion " test    ");
      u8g.drawStr( 25, 35, "Testing E...");
      u8g.drawStr( 0, 40, "<--");
    } while( u8g.nextPage() );
    if (digitalRead(BUTTON_E_PIN) == LOW) {
      break;
    }
  }
  if (millis() > nextUpdate) return "FAIL";
  nextUpdate = millis() + TIMEOUT;
  while( millis() <= nextUpdate){
    // Display question
    u8g.firstPage();  
    do {
      u8g.drawStr( 0, 9, "    T400 v" pcbVersion " test    ");
      u8g.drawStr( 25, 35, "Testing PWR...");
      u8g.drawStr( 0, 64, "<--");
    } while( u8g.nextPage() );
    if (digitalRead(BUTTON_POWER_PIN) == HIGH) {
      break;
    }
  }
  if (millis() > nextUpdate) return "FAIL";
  nextUpdate = millis() + TIMEOUT;
  while( millis() <= nextUpdate){
    // Display question
    u8g.firstPage();  
    do {
      u8g.drawStr( 0, 9, "    T400 v" pcbVersion " test    ");
      u8g.drawStr( 25, 35, "Testing A...");
      u8g.drawStr( 115, 12, "-->");
    } while( u8g.nextPage() );
    if (digitalRead(BUTTON_A_PIN) == LOW) {
      break;
    }
  }
  if (millis() > nextUpdate) return "FAIL";
  nextUpdate = millis() + TIMEOUT;
  while( millis() <= nextUpdate){
    // Display question
    u8g.firstPage();  
    do {
      u8g.drawStr( 0, 9, "    T400 v" pcbVersion " test    ");
      u8g.drawStr( 25, 35, "Testing B...");
      u8g.drawStr( 115, 40, "-->");
    } while( u8g.nextPage() );
    if (digitalRead(BUTTON_B_PIN) == LOW) {
      break;
    }
  }
  if (millis() > nextUpdate) return "FAIL";
  nextUpdate = millis() + TIMEOUT;
  while( millis() <= nextUpdate){
    // Display question
    u8g.firstPage();  
    do {
      u8g.drawStr( 0, 9, "    T400 v" pcbVersion " test    ");
      u8g.drawStr( 25, 35, "Testing C...");
      u8g.drawStr( 115, 64, "-->");
    } while( u8g.nextPage() );
    if (digitalRead(BUTTON_C_PIN) == LOW) {
      return "PASS";
    }
  }
  return "FAIL";
}

char* mcp9800Test(void){
  // Prompt user to start test
  // Display a series of images
  // Ask for PASS or FAIL
  
  // Display question
  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_6x10);
    u8g.drawStr( 0, 9, "Can you see this?");
  } while( u8g.nextPage() );
  delay(1000);
  return "NONE";
}

char* flashTest(void){
  return "NONE";
}

char* lcdTest(void){
  return "NONE";
}

char* rtcTest(void){
  return "NONE";
}

char* sdTest(void){
  if (!card.begin(SD_CS)) return "cabe";
  if (!Fat16::init(&card)) return "fa16";
  
  // create a new file
  randomSeed(analogRead(DATA1));  // Generate a random number seed
  int randNumber = random(1000, 9999);  // Choose a random number
  char name[] = "WRITE00.TXT";
  for (uint8_t i = 0; i < 100; i++) {  // Incriment a new filename if one already exists.
    name[5] = i/10 + '0';
    name[6] = i%10 + '0';
    // O_CREAT - create the file if it does not exist
    // O_EXCL - fail if the file exists
    // O_WRITE - open for write
    if (file.open(name, O_CREAT | O_EXCL | O_WRITE)) break;
  }
  if (!file.isOpen()) return "fiop";
  // write 100 line to file
  writeNumber(randNumber);
  file.write("\r\n"); // file.println() would work also
  // close file and force write of all data to the SD card
  file.close();
  if (!file.open(name, O_READ)) return "nopn";
  // ****** Somewhere I need to compare what is in the file to what it should be.
  // If they are equal, PASS.
  
  return "WORK";
}

char* adcTest(void){
  return "NONE";
}

void setup(void) {
  // Setup the buttons as inputs
  pinMode(BUTTON_A_PIN, INPUT);
  pinMode(BUTTON_B_PIN, INPUT);
  pinMode(BUTTON_C_PIN, INPUT);
  pinMode(BUTTON_D_PIN, INPUT);
  pinMode(BUTTON_E_PIN, INPUT);
  pinMode(BUTTON_POWER_PIN, INPUT);
  // ****** It looks like the pullup resistor is enabled. This is the RXLED ******
  
  pinMode(PWR_ONOFF_PIN, OUTPUT);
  digitalWrite(PWR_ONOFF_PIN, HIGH);  // Enable device power
  
  // Display a splash screen
  u8g.setRot180();  // rotate screen
  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_6x10);
    u8g.drawStr( 0, 9, "    T400 v" pcbVersion " test    ");
  } while( u8g.nextPage() );
  delay(1000);
  
  // Run all the tests.
  // This should probably be moved to the loop() function.
  buttonStatus = buttonTest();  // Do button test. Requires onscreen instructions.
  flashStatus = flashTest();  // Do SPI flash test
  lcdStatus = lcdTest();  // Do LCD test
  rtcStatus = rtcTest();  // Do RTC test
  sdStatus = sdTest();  // Do SD card test
  adcStatus = adcTest();  // Do ADC test
  mcp9800Status = mcp9800Test();  // Do ADC test
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

