/*
 * Pax Instruments T400 test firmware
 */

#include "U8glib.h"
#include "t400-testing.h"

U8GLIB_PI13264  u8g(LCD_CS, LCD_A0, LCD_RST); // Define the LCD
#define TIMEOUT 6000

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
  u8g.drawStr( 0, 9, "*** T400 v" pcbVersion " test ***");
  u8g.setFont(u8g_font_5x8);
  u8g.drawStr( 0, 18, "Buttons: ");u8g.drawStr( 45, 18, buttonStatus);u8g.drawStr( 75, 18, "LCD: ");u8g.drawStr( 100, 18, lcdStatus);
  u8g.drawStr( 0, 26, "  Flash: ");u8g.drawStr( 45, 26, flashStatus);u8g.drawStr( 75, 26, "RTC: ");u8g.drawStr( 100, 26, rtcStatus);
  u8g.drawStr( 0, 34, "MCP9800: ");u8g.drawStr( 45, 34, mcp9800Status);u8g.drawStr( 75, 34, "ADC: ");u8g.drawStr( 100, 34, adcStatus);
  u8g.drawStr( 0, 42, "         ");u8g.drawStr( 45, 42, "    ");u8g.drawStr( 75, 42, " SD: ");u8g.drawStr( 100, 42, sdStatus);
  u8g.drawStr( 0, 50, "         ");u8g.drawStr( 45, 50, "    ");u8g.drawStr( 75, 50, "     ");u8g.drawStr( 100, 50, "    ");
  u8g.drawStr( 0, 58, "         ");u8g.drawStr( 45, 58, "    ");u8g.drawStr( 75, 58, "     ");u8g.drawStr( 100, 58, "    ");
}

char* buttonTest(void){
  u8g.setFont(u8g_font_6x10);
  long nextUpdate = millis() + TIMEOUT;
  while( millis() <= nextUpdate){
    // Display question
    u8g.firstPage();  
    do {
      u8g.drawStr( 0, 12, "<--");
    } while( u8g.nextPage() );
    //buttonState = digitalRead(BUTTON_D_PIN);
    if (digitalRead(BUTTON_D_PIN) == LOW) {
      break;
    }
  }
  nextUpdate = millis() + TIMEOUT;
  while( millis() <= nextUpdate){
    // Display question
    u8g.firstPage();  
    do {
      u8g.drawStr( 0, 40, "<--");
    } while( u8g.nextPage() );
    //buttonState = digitalRead(BUTTON_D_PIN);
    if (digitalRead(BUTTON_E_PIN) == LOW) {
      break;
    }
  }
  nextUpdate = millis() + TIMEOUT;
  while( millis() <= nextUpdate){
    // Display question
    u8g.firstPage();  
    do {
      u8g.drawStr( 0, 64, "<--");
    } while( u8g.nextPage() );
    //buttonState = digitalRead(BUTTON_D_PIN);
    if (digitalRead(BUTTON_POWER_PIN) == HIGH) {
      break;
    }
  }
  nextUpdate = millis() + TIMEOUT;
  while( millis() <= nextUpdate){
    // Display question
    u8g.firstPage();  
    do {
      u8g.drawStr( 115, 12, "-->");
    } while( u8g.nextPage() );
    //buttonState = digitalRead(BUTTON_D_PIN);
    if (digitalRead(BUTTON_A_PIN) == LOW) {
      break;
    }
  }
  nextUpdate = millis() + TIMEOUT;
  while( millis() <= nextUpdate){
    // Display question
    u8g.firstPage();  
    do {
      u8g.drawStr( 115, 40, "-->");
    } while( u8g.nextPage() );
    //buttonState = digitalRead(BUTTON_D_PIN);
    if (digitalRead(BUTTON_B_PIN) == LOW) {
      break;
    }
  }
  nextUpdate = millis() + TIMEOUT;
  while( millis() <= nextUpdate){
    // Display question
    u8g.firstPage();  
    do {
      u8g.drawStr( 115, 64, "-->");
    } while( u8g.nextPage() );
    //buttonState = digitalRead(BUTTON_D_PIN);
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
  return "NONE";
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
    u8g.drawStr( 0, 9, "*** T400 v" pcbVersion " test ***");
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

