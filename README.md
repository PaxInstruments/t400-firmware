# t400-firmware

## Overview
Firmware for the Pax Instruments T400 temperature datalogger

## Setup
You need Arduino 1.5.8 and the following Arduino libraries.
- U8Glib graphical LCD https://github.com/PaxInstruments/u8glib
- MCP3424 ADC https://github.com/PaxInstruments/MCP3424
- MCP980X temperature sensor https://github.com/PaxInstruments/MCP980X
- DS3231 RTC https://github.com/PaxInstruments/DS3232RTC

During compilation on OSX you may receive errors relating to `RobotControl()` in `ArduinoRobot.cpp`. To solve this
1. Go into Applications>Arduino and right-click, "Show package contents"  
2. Go to Contents>Resources>Java>libraries  
3. Delete the folder "Robot_Control"
