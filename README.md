# t400-firmware

## Overview
Firmware for the Pax Instruments T400 temperature datalogger

## Setup
1. Install the Arduino IDE from http://arduino.cc/en/Main/Software. Compiles under 1.0.6 and 1.5.8.
2. Install the following Arduino libraries.
  - U8Glib graphical LCD https://github.com/PaxInstruments/u8glib
  - MCP3424 ADC https://github.com/PaxInstruments/MCP3424
  - MCP980X temperature sensor https://github.com/PaxInstruments/MCP980X
  - DS3231 RTC https://github.com/PaxInstruments/ds3231
  - FAT16 SD card library https://github.com/PaxInstruments/Fat16 (use the FAT16 directory within this repository)
3. Set the Arduino board to "LilyPad Arduino USB".
4. Flash the T400 with the Pax Instruments bootloader at https://github.com/PaxInstruments/pi32u4_boards (requires extra steps)
4. After pluggin in your T400 and turning it on ensure you have selected the correct serial port

During compilation on OSX you may receive errors relating to `RobotControl()` in `ArduinoRobot.cpp`. This seems to be a problem in Arduino 1.0.5 and later. To work around this...

1. Go into Applications>Arduino and right-click, "Show package contents"
2. Go to Contents>Resources>Java>libraries
3. Delete the folder "Robot_Control"
