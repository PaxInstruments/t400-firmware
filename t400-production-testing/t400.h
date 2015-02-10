// Hardware definitions for the t400

// Compile-time settings. Some of these should be set by the user during operation.
#define LOG_INTERVAL     10      // millseconds between entries
#define SYNC_INTERVAL    1000    // millis between calls to sync()

#define SENSOR_COUNT     4       // Number of sensors on the board (fixed)

#define AVR "ATmega32U4"  // SPIFlash looked like it wants this

#define OUT_OF_RANGE     99999.9 // Double value representing an invalid temp. measurement
#define GRAPH_INVALID    -126    // Invalid graph point

// Graph display settings
#define MAXIMUM_GRAPH_POINTS    100

// Set timeout for t400-testing
#define TIMEOUT 3000  // Define the test timeout interval

/// I2C addresses
#define MCP3424_ADDR    0x69

// Pin definitions for Electronics version 0.9
#define pcbVersion "0.9" // Electronics version 0.9 milestone.
// Pin definitions
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
#define FLASH_CS            11
#define LCD_A0              12
#define BUTTON_C_PIN        13
// MISO                     14
// SCK                      15
// MOSI                     16
#define BUTTON_POWER_PIN    17
#define LCD_BACKLIGHT_PIN   A0    // LCD backlight on pin
#define LCD_CS              A1
#define DATA1               A2
// DATA2                    A3
#define BUTTON_D_PIN        A4
#define BUTTON_E_PIN        A5


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
