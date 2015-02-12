#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>

// Button names
#define BUTTON_A      0 // Start/stop logging
#define BUTTON_B      1 // Set logging interval
#define BUTTON_C      2 // Change units
#define BUTTON_D      3 // Toggle channels
#define BUTTON_E      4 // Backlight on/off
#define BUTTON_POWER  5 // Power on/off
#define BUTTON_COUNT  6  // Number of buttons in the system

class Buttons {
  private:
    uint8_t pressedButton;  // Stores the button that was pressed
    uint8_t lastPressed;
    
  public:
    // Initialize the buttons class
    void setup();
    
    // Scan for button presses
    void buttonTask();
    
    // Returns true if a button is pressed
    bool isPressed();
    
    // If a button is pressed, get it!
    uint8_t getPressed();
};


#endif // BUTTONARRAY_HH
