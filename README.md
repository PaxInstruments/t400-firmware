# t400-firmware

## Overview
Firmware for the Pax Instruments T400 temperature datalogger

## Setup
1. Install the Arduino IDE from http://arduino.cc/en/Main/Software. This code is developed using Arcuino IDE 1.6.7.
2. Install the following Arduino libraries.
  - U8Glib graphical LCD https://github.com/PaxInstruments/u8glib
  - MCP3424 ADC https://github.com/PaxInstruments/MCP3424
  - MCP980X temperature sensor https://github.com/PaxInstruments/MCP980X
  - DS3231 RTC https://github.com/PaxInstruments/ds3231
  - SdFat library https://github.com/PaxInstruments/SdFat (use the SdFat directory within this repository)
  - SPI flash https://github.com/PaxInstruments/SPIFlash
3. Install the Pax Instruments hardware core (unzip it and move it to the hardware/ directory in your Sketches folder):
  - https://github.com/PaxInstruments/ATmega32U4-bootloader
4. Restart Arduino if it was already running
5. In tools->board, set the Arduino board to "Pax Instruments T400".
6. In tools->port, select the serial port corresponding to the T400 (Arduino should identify it correctly)
