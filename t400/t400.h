// Hardware definitions for the t400
#define __AVR_ATmega32U4__ 1

// Calibration values
#define MCP3424_CALIBRATION_MULTIPLY 1.00713
#define MCP3424_CALIBRATION_ADD 5.826


// Debugging
#define DEBUG_JUNCTION_TEMPERATURE 0

// Compile-time settings. Some of these should be set by the user during operation.
#define SYNC_INTERVAL    1000    // millis between calls to sync()

#define SENSOR_COUNT     4       // Number of sensors on the board (fixed)

#define OUT_OF_RANGE     99999.9 // Double value representing an invalid temp. measurement
#define GRAPH_INVALID    -127    // Invalid graph point

// Graph display settings
#define MAXIMUM_GRAPH_POINTS    100

#define DISPLAY_HEIGHT  64    // Height of the display
#define CHARACTER_SPACING 5   // Width of a character+space to next character

#define TEMPERATURE_UNITS_C  0
#define TEMPERATURE_UNITS_F  1
#define TEMPERATURE_UNITS_K  2
#define TEMPERATURE_UNITS_COUNT 3
#define GRAPH_CHANNELS_COUNT 5


/// I2C addresses
#define MCP3424_ADDR    0x69

// Pin definitions for Electronics version 0.13
#define pcbVersion ".13" // Electronics version 0.12 milestone.
#define FIRMWARE_VERSION "0.14-beta"

// Pin definitions
#define BUTTON_B_PIN         1  // 0.12-mod 7, 0.13 1
#define BUTTON_A_PIN         7  // 0.12-mod 1, 0.13 7
// SDA                       2
// SCL                       3
#define VBAT_EN              4  // Enable VBAT_SENSE
#define SD_CS                5
#define BATT_STAT            A7    // Battery status pin
#define RTC_INT              0
#define BUTTON_C_PIN         8
#define BUTTON_D_PIN         9
#define BUTTON_E_PIN        10
#define PWR_ONOFF_PIN       30   // Power on/off pin turns board power off (active high)
#define VBAT_SENSE          A11  // Battery voltage /2 (D12)
// #define FLASH_CS            13
// MISO                     14
// SCK                      15
// MOSI                     16
#define BUTTON_POWER_PIN    11

// DATA2                    A0
#define DATA1               A1
#define LCD_A0              A2
#define LCD_RST             A3
#define LCD_CS              A4
#define LCD_BACKLIGHT_PIN   A5   // LCD backlight on pin


#define LCD_CONTRAST     0x018*7  // Sets the LCD contrast
