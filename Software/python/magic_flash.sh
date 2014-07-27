dfu-programmer atmega32u4 erase
dfu-programmer atmega32u4 flash-eeprom --suppress-bootloader-mem firmware.eep --debug 2
dfu-programmer atmega32u4 flash firmware.hex --debug 2
dfu-programmer atmega32u4 start
