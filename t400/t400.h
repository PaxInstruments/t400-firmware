// Hardware definitions for the t400

// Compile-time settings. Some of these should be set by the user during operation.
#define LOG_INTERVAL     10      // millseconds between entries
#define SYNC_INTERVAL    1000    // millis between calls to sync()

#define SENSOR_COUNT     4       // Number of sensors on the board (fixed)

#define OUT_OF_RANGE     99999.9 // Double value representing an invalid temp. measurement
#define GRAPH_INVALID    -126    // Invalid graph point

// Graph display settings
#define MAXIMUM_GRAPH_POINTS    100

#define DISPLAY_HEIGHT  64    // Height of the display
#define CHARACTER_SPACING 5     // Width of a character+space to next character

#define TEMPERATURE_UNITS_C  0
#define TEMPERATURE_UNITS_F  1
#define TEMPERATURE_UNITS_K  2
#define TEMPERATURE_UNITS_COUNT 3


/// I2C addresses
#define MCP3424_ADDR    0x69

// Pin definitions for Electronics version 0.9
#define pcbVersion ".10" // Electronics version 0.9 milestone.
// Pin definitions
#define BUTTON_B_PIN         0
#define BUTTON_A_PIN         1
// SDA                       2
// SCL                       3
#define LCD_A0               4
#define BATTERY_STATUS_PIN   5    // Battery status pin
#define LCD_BACKLIGHT_PIN    6    // LCD backlight on pin
// RTC_INT                   7
#define LCD_CS               8
// VBAT_SENSE                9
#define BATTERY_VBAT_PIN    10    // Battery voltage pin (analog input)
// FLASH_CS                 11
#define LCD_RST             12
#define BUTTON_C_PIN        13
// MISO                     14
// SCK                      15
// MOSI                     16
#define SD_CS               17
// DATA2                    A0
#define DATA1               A1
#define BUTTON_D_PIN        A2
#define BUTTON_E_PIN        A3
#define BUTTON_POWER_PIN    A4
#define PWR_ONOFF_PIN       A5    // Power on/off pin turns board power off (active low)


#define LCD_CONTRAST     0x018*7  // Sets the LCD contrast


#define DEBUG            0 // Debugging code: 0 disable, 1 enable
#define DEBUG_LCD        0 // Debugging code: 0 disable, 1 enable


// Creates a funciton for printing to serial during debugging.
#if DEBUG
#define DEBUG_PRINT(x)    Serial.print (x)
#define DEBUG_PRINTLN(x)  Serial.println (x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif
