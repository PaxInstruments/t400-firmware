#ifndef BUTTONS_H
#define BUTTONS_H

//#include <util/atomic.h>
//#include "Types.hh"
//#include "Timeout.hh"
#include <arduino.h>

// Button names
#define BUTTON_A      0
#define BUTTON_B      1
#define BUTTON_C      2
#define BUTTON_D      3
#define BUTTON_E      4
#define BUTTON_POWER  5
#define BUTTON_COUNT   6  // Number of buttons in the system


struct button {
  uint8_t pin;
  uint8_t inverted;
};


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
