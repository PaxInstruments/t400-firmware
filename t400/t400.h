// Hardware definitions for the t400

// Compile-time settings. Some of these should be set by the user during operation.
#define LOG_INTERVAL     10      // millseconds between entries
#define SYNC_INTERVAL    1000    // millis between calls to sync()

#define SENSOR_COUNT     4       // Number of sensors on the board (fixed)

#define OUT_OF_RANGE     99999.9 // Double value representing an invalid temp. measurement

// Graph display settings
#define MAXIMUM_GRAPH_POINTS    100

/// I2C addresses
#define MCP3424_ADDR    0x69


// Pin definitions
#define BUTTON_A_PIN      4
#define BUTTON_B_PIN      12
#define BUTTON_C_PIN      6
#define BUTTON_D_PIN      0
#define BUTTON_E_PIN      1
#define BUTTON_POWER_PIN  7

#define BUZZER_PIN            13    // Output pin turns buzzer on (active low)
#define PWR_ONOFF_PIN         11    // Power on/off pin turns board power off (active low)
#define LCD_BACKLIGHT_PIN     A2    // LCD backlight on pin
#define BATTERY_VBAT_PIN      A1    // Battery voltage pin (analog input)
#define BATTERY_STATUS_PIN    A0    // Battery status pin


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
