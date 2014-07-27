#!/bin/sh

dfu-programmer atmega32u4 erase
# dfu-programmer atmega32u4 flash-eeprom --suppress-bootloader-mem firmware.eep --debug 2
dfu-programmer atmega32u4 flash $1 --debug 2
dfu-programmer atmega32u4 start
