#ifndef FIRMWARE_H
#define FIRMWARE_H

#define ATMELSTUDIO_HACK

#include <Arduino.h>
#include <EEPROM.h>
//#include <Wire.h>

//#define BABYSHIFTER
//#define NOSERIAL

//#define UNITTEST
//#define DEBUGMODE

// TO prevent accidental blindness by 
#define EYE_FRIENDLY

// customize your own name ;)
#define CUSTOMIZED_NAME "Shifter"

#define LEDS 16
// used for fixed point arithmetic
#define FIXED_SHIFT 8
// make columns longer
#define COLUMNMULTIPLY 2
#define ANIMATIONDELAY 2


// chinese LEDS
//#define COLOR_CODING_GRB
// cree LEDS
#define COLOR_CODING_RGB

#ifdef COLOR_CODING_RGB
	#define CHANNEL_RED 		0
	#define CHANNEL_GREEN 	1
	#define CHANNEL_BLUE 	2
#endif
#ifdef COLOR_CODING_GRB
	#define CHANNEL_RED 		1
	#define CHANNEL_GREEN 	0
	#define CHANNEL_BLUE 	2
#endif

/* Pin definitions */
#ifdef BABYSHIFTER
  #define XAXIS 1
  #define YAXIS 0
  #define ZAXIS 2
    
  #define ledPin 11
  
  #define int1Pin 12 // D7  // Baby Shifter  
  #define int2Pin 13 // B4
  
  #define BTNC 8
  
  #define TXLED1 
  #define TXLED0
#else  
  #define RTCINT_PIN A2
  #define IRPWM_PIN 5 // PC6/pin 31 

  #define XAXIS 0
  #define YAXIS 1
  #define ZAXIS 2
  
  #define ledPin  13
  
  #define int1Pin 8  // Magic Shifter  
  #define int2Pin 9

  #define int3Pin 0 // light sensor

  #define EEPROM_CS_PIN 11
  
  #define BTNC 10
#endif

#define LEDWHITE_PIN 6
#define LEDUV_PIN 1


#define RXLED 17
#define PWRMGT A5
#define PWRMGT_ANALOG_PIN 5
#define LEDTESTER_ANALOG_PIN 2

#define CHRGSTAT_PIN A4
#define TURBOCHRG_PIN 7
//#define SHIFT_OUT_PIN 6

#define AUTOSHUTDOWN_TIME (23*60*1000L)  // shuts down automatically after 5 minutes

#define SHUTDOWN_TIME 2000
#define MODE_EXIT_TIME 2000
#define DEBOUNCE_TIME 15
#define LONG_CLICK_TIME 550

#define BrightnessCtrlTime 600

// TODO
#define DOUBLE_CLICK_TIME 250
#define REWARD_FLASH_LEN 350

#define POWERUP_USER 0
#define POWERUP_USB 1
#define POWERUP_ALERT 2

// global Startup state
extern byte powerupReason;

// global ButtonState
extern long powerBtnPressed;
extern int powerBtnClickedTime;
extern int powerBtnReleased;
extern long centerBtnPressed;
extern int centerBtnClickedTime;
extern int centerBtnReleased;

extern long lastActivity;

// define global LED state
extern uint8_t LEDS_VALUES[];

// global Accelerometer state

extern byte data[6];  // x/y/z accel register data store here
extern int accelCount[3];  // Stores the 12-bit signed value

extern byte orientation;
extern byte orientationFace;
extern byte zTilt;
extern int8_t xTaps, yTaps, zTaps;
extern uint8_t lastTapStatus;
extern byte lastTransientStatus;

//#define SHAKEDEBUG
//#define FRAMEDEBUG
#define LASTINTERRUPTTIMEOUT 8

// rtc state
extern uint8_t initiateRTCReadout;
extern uint8_t rtcBuffer[];

// brightness and mode
extern int ad;
extern uint8_t v;
extern uint8_t MAXMV;
extern bool vChanged;

// light sensor state
extern int cs, rs, gs, bs, ps;
extern int lightSensorValues[5];


union MSColor {
#ifdef COLOR_CODING_RGB
  struct { uint8_t r, g, b; } rgb;
#endif
#ifdef COLOR_CODING_GRB
  struct { uint8_t g, r, b; } rgb;
#endif
  struct { uint8_t ch0, ch1, ch2; } ordered;
  uint8_t channels[3];
	
/*
	public MSColor(uint8_t _r, uint8_t _g, uint8_t _b)
	{
		rgb.r = _r;
		rgb.g = _g;
		rgb.b = _b;
	}
*/ 
 };

// MagicShifter.cc
struct MSBitmapHeader
{
	uint32_t eepromAddr;

	uint8_t pixelFormat;
	uint8_t maxFrame;
	uint8_t frameWidth;
	uint8_t frameHeight;

	uint8_t subType;
	uint8_t firstChar;
	uint16_t animationDelay;
} __attribute__((packed));
	
struct MSBitmap
{
	MSBitmapHeader header;
	MSColor color;
};

extern uint8_t AccelShakeNrOfFrames;
extern uint8_t AccelShakeFrameNr;
extern MSBitmap font16px;
extern MSBitmap font12px;
extern MSBitmap font8px;

//extern uint8_t fontWidth;
//extern uint8_t fontHeight;

void run_bootloader();
void MagicShifter_Init();
void blink(int p1, int p2, int n);
void SetDebugLeds(byte debug, byte rx, byte tx);
uint8_t SerialReadByte();
uint16_t SerialReadWord();
bool MagicShifter_Poll_Menue();
bool MagicShifter_Poll();
void USBPoll();
/*void LoadFont(uint8_t sector);
void PlotFont(uint8_t ascii, uint8_t column, uint8_t startLed);
void PlotText(char *text, uint16_t column, uint8_t startLed);
*/
void LoadBitmap(uint8_t sector, MSBitmap *bitmap);
void PlotBitmapColumn1Bit(const MSBitmap *bitmap, uint16_t absColumn, uint8_t ledIdx);
void PlotBitmapColumn24Bit(const MSBitmap *bitmap, uint16_t absColumn, uint8_t startLed);
void PlotBitmapColumn(const MSBitmap *bitmap, uint8_t frame, uint8_t column, uint8_t startLed);
void PlotText(const MSBitmap *bitmap, char *text, uint16_t column, uint8_t startLed);
void UpdateAccelShakeCurrentColumn();

void startToEndIndex2(int start, int end, int d, int index, int color);
void ackFlash(int r, int g, int b, int iS, int iE, int d1, int d2, int d3);
void debugColor(int r, int g, int b, int iS, int iE, int d);
void UpdateButtons();
void saveMode(int idx);
void saturateRgb(uint8_t &r, uint8_t &g, uint8_t &b);
void SetWhiteLED(uint8_t val);
void SetUVLED(uint8_t val);
void enableLeds(uint8_t val);


void CheckTCS3771(int cycles);
bool VerifyTCS3771Interrupt();
void PickupColor(uint8_t &byte_r, uint8_t &byte_g, uint8_t &byte_b);
void PickupColorEx(uint8_t cycles, uint8_t useLight);
byte xorshift32();


// MMA.cc
struct AccelConfig
{
	uint8_t accelInterruptNr, tapInterruptNr, transientInterruptNr;
	uint8_t accelScale;
	uint8_t accelDataRate;
};

void ClearInterruptI2CReadout();
void AccelSetup(AccelConfig *newConfig);
void BackupAccelState();
void RestoreAccelState();
void AccelPoll();
void initMMA8452(byte fsr, byte dataRate); // dericated
void MMA8452Active();
void MMA8452Standby();
void writeRegister(unsigned char address, unsigned char data);
void CheckMMA8452();
void CheckMMA8452WithInterrupts();
void CheckMMA8452Pulse();

void updateLedsWithBlank(); // depricated
void updateLeds(); // depricated

void setRGB(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
void setAll(uint8_t r, uint8_t g, uint8_t b);
void setChannel(uint8_t index, uint8_t channelIndex, uint8_t value);
void setAllChannel(uint8_t channel, uint8_t value);

//void setPixel(byte index, byte r, byte g, byte b); // now only used in ledwarz
void Wheel(byte WheelPos, byte &r, byte &g, byte &b);

// TLC5947
void initLEDS();
void updateLeds(); // depricated
void updateLedsWithBlank(); // depricated

void updateLedsWithBrightness();
void latchLedsWithBlank();

byte readRegister(uint8_t address);
byte TCS3771_ReadRegister(byte address);
int loadMode();
int ModeSelector(int modes, int startIdx, char*text[]);

static bool waitButton();
void displayText(char *text, bool wait=true, MSBitmap *font = NULL, int posX=0);
void Int2Str(int val, char *text);

// Modes.cc
void ColorPickerMode();
void DebugPrintBattery();
void DISCOMode();
void enableTurboCharge(int enable);
void FutureMode();
void GravBallMode(int intLvl);
void HackingAt29C3Mode();
void HackingAt29C3ScannerMode();
void RaceToTheMoon();
void RainbowMove();
void readRegisters(byte address, int i, byte * dest);
void shakeStrobo();
void showBatteryStatus(int d);
void SquareMode();
void TCS3771_WriteRegister(byte address, byte data);
void UnicornShadowMode();
void WaitClearButtons();
void MeasureBattery(bool doVCalc);
void waitForPowerDown();
void powerDown();
void LedWarzMode();
void BikeLight();
void RTCMode();
void IRPWMMode();
void UnitTestMode();
void MagicShifterMode();
void UIDemoMode();
void MIDIMode();

unsigned char MCP7941X_ReadRegister(uint8_t address);
void MCP7941X_WriteRegister(unsigned char address, unsigned char data);
void MCP7941X_SetTime(int year, int month, int day, int hour, int minute, int second);
void MCP7941X_SetAlert0(byte alarmType, int month, int day, int hour, int minute, int second);
void MCP7941X_SetAlert1(byte alarmType, int month, int day, int hour, int minute, int second);

void ReadEEEPROMID(byte &id, byte &type);
void WriteByte(uint32_t adress, uint8_t data);
void ReadBytes(uint32_t adress, uint8_t *buffer, uint16_t len);
void WriteBytes(uint32_t adress, uint8_t *buffer, uint16_t len);
uint8_t ReadStatus();
void UnprotectEEPROM();
void EraseSector(uint32_t adress);

//FixedMath.cc
long FixedSqrt(long);


void setup();

#ifdef ATMELSTUDIO_HACK

#define F_CPU 8000000L
#define ARDUINO 101
#define USB_VID 0x2341
#define USB_PID 0x8036

#include "pins_arduino.h"
#include <Arduino.h>

#include "main.cpp"

#include "wiring.c"
#include "wiring_digital.c"
#include "wiring_analog.c"
//#include "HardwareSerial.cpp"
#include "CDC.cpp"
#include "Print.cpp"
#include "HID.cpp"
#include "new.cpp"
#include "EEPROM.cpp"
#include "USBCore.cpp"
#include "SPI.cpp"
#include "WString.cpp"




#include "CustomModes/BikeLight.cc"
#include "CustomModes/Wireless.cc"
#include "bmpfont.cc"
#include "colors.cc"
#include "external_eeprom.cc"
#include "firmware.cc" 
#include "i2c.cc"
#include "LedWarz.cc"
#include "MagicShifter.cc"
#include "MCP7941X.cc"
#include "MMA845X.cc"
#include "PowerManagement.cc"
#include "pwm.cc"
#include "TCS3771.cc"
#include "TLC5947.cc"
#include "Modes.cc"
#include "FixedMath.cc"

//#include "UnitTest.cc"
#endif
#endif
