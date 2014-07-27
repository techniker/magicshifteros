# MagicShifter Firmware Makefile
# Arduino IDE (at least 1.0.1) is needed:
# http://arduino.cc/en/Main/Software

#ARDUINO_DIR = /opt/arduino-1.0.3
#!J! Don't forget to set this before your first make
ARDUINO_DIR = /Applications/Arduino1.0.5.app/Contents/Resources/Java/
ARDUINO_DIR = /usr/share/arduino
ARDUINO_HW  = arduino
ARDUINO_VAR = leonardo

DFU_PROGRAMMER = sudo dfu-programmer

OBJS  = firmware.o bmpfont.o colors.o MagicShifter.o MMA845X.o MCP7941X.o UnitTest.o Modes.o
OBJS += PowerManagement.o TLC5947.o TCS3771.o i2c.o external_eeprom.o
OBJS += LedWarz.o EEPROM.o SPI.o wiring.o wiring_digital.o
OBJS += CustomModes/BikeLight.o CustomModes/MIDIMode.o CustomModes/Wireless.o
OBJS += wiring_analog.o HardwareSerial.o CDC.o Print.o USBCore.o HID.o new.o
OBJS += WString.o FixedMath.o main.o
LDFLAGS = # -T ldscript.x -Wl,--verbose
CXX = avr-g++
CC = avr-gcc

CXXFLAGS  = -MD -mmcu=atmega32u4 -DF_CPU=8000000L -DARDUINO=101 -DUSB_VID=0x2341 -DUSB_PID=0x8036
CXXFLAGS += -Os -fno-exceptions -ffunction-sections -fdata-sections -Wl,--gc-sections
CXXFLAGS += -I$(ARDUINO_DIR)/hardware/$(ARDUINO_HW)/variants/$(ARDUINO_VAR)
CXXFLAGS += -I$(ARDUINO_DIR)/hardware/$(ARDUINO_HW)/cores/arduino
CXXFLAGS += -I$(ARDUINO_DIR)/libraries/EEPROM
CXXFLAGS += -I$(ARDUINO_DIR)/libraries/Wire
CXXFLAGS += -I$(ARDUINO_DIR)/libraries/SPI
CXXFLAGS += -DMIDI_SHIFTER
CCFLAGS = $(CXXFLAGS)

-include Makefile.conf

firmware.hex: firmware.elf
	avr-objcopy -j .text -j .data -O ihex firmware.elf firmware.hex
	avr-objcopy -j .text -j .data -O binary firmware.elf firmware.bin
	avr-objcopy -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0 --no-change-warnings -O ihex firmware.elf firmware.eep
	avr-size firmware.hex firmware.elf

firmware.elf: $(addprefix objects/,$(OBJS)) # ldscript.x
	$(CC) -o $@ $(LDFLAGS) $(CCFLAGS) $(filter-out ldscript.x,$^) -lm

objects/Wire.o: $(ARDUINO_DIR)/libraries/Wire/Wire.cpp
	@mkdir -p objects objects/CustomModes
	$(CXX) -c -o $@ $(CXXFLAGS) -I$(ARDUINO_DIR)/libraries/Wire/utility $<

objects/Wire_twi.o: $(ARDUINO_DIR)/libraries/Wire/utility/twi.c
	@mkdir -p objects objects/CustomModes
	$(CC) -c -o $@ $(CCFLAGS) -I$(ARDUINO_DIR)/libraries/Wire/utility $<

objects/EEPROM.o: $(ARDUINO_DIR)/libraries/EEPROM/EEPROM.cpp
	@mkdir -p objects objects/CustomModes
	$(CXX) -c -o $@ $(CXXFLAGS) $<

objects/SPI.o: $(ARDUINO_DIR)/libraries/SPI/SPI.cpp
	@mkdir -p objects objects/CustomModes
	$(CXX) -c -o $@ $(CXXFLAGS) $<

objects/%.o: $(ARDUINO_DIR)/hardware/$(ARDUINO_HW)/cores/arduino/%.cpp
	@mkdir -p objects objects/CustomModes
	$(CXX) -c -o $@ $(CXXFLAGS) $<

objects/%.o: $(ARDUINO_DIR)/hardware/$(ARDUINO_HW)/cores/arduino/%.c
	@mkdir -p objects objects/CustomModes
	$(CC) -c -o $@ $(CCFLAGS) $<

objects/%.o: %.cc
	@mkdir -p objects objects/CustomModes
	$(CXX) -fverbose-asm -Wa,-a,-ad,-aln=$@.s -c -o $@ $(CXXFLAGS) $<

objects/CustomModes/%.o: %.cc
	@mkdir -p objects/CustomModes
	$(CXX) -fverbose-asm -Wa,-a,-ad,-aln=$@.s -c -o $@ $(CXXFLAGS) $<

flash: firmware.hex
	-$(DFU_PROGRAMMER) atmega32u4 erase
	-$(DFU_PROGRAMMER) atmega32u4 flash firmware.hex --debug 2
	$(DFU_PROGRAMMER) atmega32u4 start

flash-eeprom: firmware.hex
	-$(DFU_PROGRAMMER) atmega32u4 erase
	$(DFU_PROGRAMMER) atmega32u4 flash-eeprom --suppress-bootloader-mem firmware.eep 
	$(DFU_PROGRAMMER) atmega32u4 flash firmware.hex --debug 2
	$(DFU_PROGRAMMER) atmega32u4 start

verify:
	$(DFU_PROGRAMMER) atmega32u4 dump --debug 2 > firmware.cmp
	cmp -ln $$( find firmware.bin -printf "%s" ) firmware.cmp firmware.bin && echo "OK." || ! echo "ERROR!"

flash-raspberry: firmware.hex
	scp firmware.hex pi@192.168.1.23:magicshifter
	scp firmware.eep pi@192.168.1.23:magicshifter

flash-raspberry-tester: firmware.hex
	scp ../Tester/raspberry/* pi@192.168.1.23:magicshifter

testrun: firmware.hex
	python ../Tester/raspberry/MagicTest.py

indent:
	indent -npro -kr -i8 -ts8 -l500 -lc500 -ss -ncs -cp1 *.cc

clean:
	rm -rf firmware.elf firmware.hex firmware.eep objects

-include objects/*.d *.d

.PHONY: flash flash-eeprom indent clean

