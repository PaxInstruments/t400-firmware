# t400-firmware

## Overview
Firmware for the Pax Instruments T400 temperature datalogger

## Setup
1. Install the Arduino IDE from http://arduino.cc/en/Main/Software. This code is developed using Arduino IDE 1.6.7.
2. Install the following Arduino libraries. You will have to rename folders to remove '-main' form the end. For example, u8glib-main must be renamed to u8glib.
  - U8Glib graphical LCD https://github.com/PaxInstruments/u8glib. This repository contains Pax Instruments specific code.
  - MCP3424 ADC https://github.com/PaxInstruments/MCP3424
  - MCP980X temperature sensor https://github.com/PaxInstruments/MCP980X
  - DS3231 RTC https://github.com/PaxInstruments/ds3231
  - SdFat library https://github.com/PaxInstruments/SdFat (use the SdFat directory within this repository)
3. Install the Pax Instruments hardware core https://github.com/PaxInstruments/ATmega32U4-bootloader
  - Unzip it and move it to the hardware/ directory in your Sketches folder
4. Restart Arduino if it was already running
5. In tools->board, set the Arduino board to "Pax Instruments T400".
6. In tools->port, select the serial port corresponding to the T400 (Arduino should identify it correctly)

## Flashing new firmware
1. Power down the device
2. Press and hold the graph button and backlight button. This will keep the device in bootloader mode.
3. Press the power button.
4. Upload firmware
