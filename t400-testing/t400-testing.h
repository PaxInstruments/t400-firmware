/* Hardware definitions for the T400 board
 *
 *
 * Notes
 * =====
 * - Use "U8GLIB_PI13264  u8g(LCD_CS, LCD_A0, LCD_RST); // Define the LCD"
 */

#define pcbVersion "0.9" // Electronics version 0.9 milestone.

// Pin definitions for Electronics version 0.9
#define BUTTON_B_PIN         0
#define BUTTON_A_PIN         1
// SDA                       2
// SCL                       3
#define LCD_RST              4
#define SD_CS                5
// RTC_INT                   6
#define PWR_ONOFF_PIN        7    // Power on/off pin turns board power off (active low)
#define BATTERY_VBAT_PIN     8    // Battery voltage pin (analog input)
// VBAT_SENSE                9
#define BATTERY_STATUS_PIN  10    // Battery status pin
// FLASH_CS                 11
#define LCD_A0              12
#define BUTTON_C_PIN        13
// MISO                     14
// SCK                      15
// MOSI                     16
#define BUTTON_POWER_PIN    17
#define LCD_BACKLIGHT_PIN   A0    // LCD backlight on pin
#define LCD_CS              A1
// DATA1                    A2
// DATA2                    A3
#define BUTTON_D_PIN        A4
#define BUTTON_E_PIN        A5
