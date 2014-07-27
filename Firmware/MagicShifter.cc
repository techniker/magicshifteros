#include "firmware.h"
//#include <EEPROM.h>
#include "i2c.h"
#include <avr/eeprom.h> 
#include <util/delay.h>

// TODOimplement this in UpdateButtons
int centerButtonDoublePressed = 0;
int centerButtonTripplePressed = 0;
int centerButtonMultiPressed = 0;

long powerBtnPressed = 0;
int powerBtnClickedTime = 0;
int powerBtnReleased = 0;

long centerBtnPressed = 0;
int centerBtnClickedTime = 0;
int centerBtnReleased = 0;

long lastActivity = -0;
bool vChanged = false;

// for measuring power btn press
extern int ad;

#ifdef EYE_FRIENDLY
uint8_t BLevels[16] = { 1,   2,   3, 6,
					 	     12,  24,  35, 50, 
#else
uint8_t BLevels[16] = { 1,   3,   5, 12,
					 	    18,  25,  35, 50, 
#endif
					        65,  80, 102, 128, 
						    145, 170, 200, 255};
uint8_t v = 4;
// cutoff for brightness ctrl
uint8_t MAXMV = 32;


MSBitmap font16px;
MSBitmap font12px;
MSBitmap font8px;
MSBitmap font6px;

byte powerupReason;	

byte xorshift32() {
	static uint32_t x = 314159265;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return x;
}

void debugColor(int r, int g, int b, int iS, int iE, int d)
{
	setAll(0, 0, 0);
	for (int i = iS; i < iE; i++) {

		setRGB(i, r, g, b);

	}
	updateLeds();
	delay(d);
}

void ackFlash(int r, int g, int b, int iS, int iE, int d1, int d2, int d3)
{
	setAll(0, 0, 0);
	updateLedsWithBlank();
	delay(d1);

	for (int i = iS; i < iE; i++) {

		setRGB(i, r, g, b);

	}
	updateLedsWithBlank();
	delay(d2);
	setAll(0, 0, 0);
	updateLedsWithBlank();
	delay(d3);
}

void gravMenu(int choicesX, int choicesY, int &choiceX, int &choiceY)
{
	int scaling = 800;

	while (1) {
		CheckMMA8452();
		choiceX = ((accelCount[1] + scaling) * choicesX) / (2 * scaling);
		//choiceX += ;
		if (choiceX < 0)
			choiceX = 0;
		if (choiceX >= choicesX)
			choiceX = choicesX - 1;

		setAll(0, 0, 0);
		setRGB((choiceX * 15) / (choicesX - 1), 255, 0, 0);
		updateLeds();
	}
}

/* bootloader_code adapted from:
https://github.com/komar007/atmel_bootloader
Jumps to the beginning of bootloader. Address is set to the address of the ATMEL flip bootloader 
2014.07.23: fixed by prom from raumfahrtagentur.org, ktnx!
*/
void run_bootloader()
{
	v = 10;
	setAll(0, 0, 0);
	setRGB(6, 255, 0, 0);
	setRGB(7, 0, 255, 0);
	setRGB(8, 0, 0, 255);
	setRGB(9, 128, 128, 128);
	updateLeds();

	TIMSK0 = 0;
	uint32_t bladdr = 0x3800;

	// disable interrupts
	cli();

	// detach USB
	UDCON  = _BV(DETACH);
	// disable USB clock
	USBCON = _BV(FRZCLK);

	// give host some extra time
	_delay_ms(10);

	// set reset cause to "external reset"
	MCUSR = _BV(EXTRF);

	// jump to loader
	__asm__ __volatile__(
		"mov r30, %0" "\n\t"
		"mov r31, %1" "\n\t"
		"ijmp"
		:
		: "r" ((bladdr >> 0) & 0xff),
		  "r" ((bladdr >> 8) & 0xff)
		: "r30", "r31");
}

void MagicShifter_Init()
{
	CLKPR = (1 << CLKPCE);	// enable a change to CLKPR 
	CLKPR = 0;		// set the CLKDIV to 0 - was 0011b = div by 8 

	/* disable JTAG to allow corresponding pins to be used */
	#if ((defined(__AVR_AT90USB1287__) || defined(__AVR_AT90USB647__) ||  \
		   defined(__AVR_AT90USB1286__) || defined(__AVR_AT90USB646__) ||  \
		   defined(__AVR_ATmega16U4__)  || defined(__AVR_ATmega32U4__) ||  \
		   defined(__AVR_ATmega32U6__)))
	  // note the JTD bit must be written twice within 4 clock cycles to disable JTAG
	  // you must also set the IVSEL bit at the same time, which requires IVCE to be set first
	  // port pull-up resistors are enabled - PUD(Pull Up Disable) = 0
	  MCUCR = (1 << JTD) | (1 << IVCE) | (0 << PUD);
	  MCUCR = (1 << JTD) | (0 << IVSEL) | (0 << IVCE) | (0 << PUD);
	#endif

	// why? because otherwise it's floating
	pinMode(RTCINT_PIN, INPUT);
	digitalWrite(RTCINT_PIN, HIGH);
	
	digitalWrite(IRPWM_PIN, LOW);
	pinMode(IRPWM_PIN, INPUT);

	digitalWrite(LEDWHITE_PIN, LOW);
	digitalWrite(LEDUV_PIN, LOW);

	pinMode(LEDUV_PIN, OUTPUT);
	pinMode(LEDWHITE_PIN, OUTPUT);

	digitalWrite(EEPROM_CS_PIN, HIGH);
	pinMode(EEPROM_CS_PIN, OUTPUT);

	pinMode(ledPin, OUTPUT);
	digitalWrite(RXLED, HIGH);
	pinMode(RXLED, OUTPUT);
	TXLED0;

	pinMode(BTNC, INPUT);
	digitalWrite(BTNC, HIGH);

	pinMode(int1Pin, INPUT);
	pinMode(int2Pin, INPUT);
	pinMode(int3Pin, INPUT);
	// pullup
	digitalWrite(int3Pin, HIGH);

	blink(50, 50, 3);

	initLEDS();

#ifndef BABYSHIFTER
	enableTurboCharge(false);
#endif

#ifndef NOSERIAL
	Serial.begin(9600);
#endif

	int s = 1;
	//Wire.begin();
	// Accelerometer init
	i2cInit();

	// clear TC alert, TODO: check if its here and ask for time if power interuption is detected

	if (!digitalRead(RTCINT_PIN))
		powerupReason = POWERUP_ALERT;
	else if ((USBSTA & 0x1))
		powerupReason = POWERUP_USB;
	else
		powerupReason = POWERUP_USER ;

		/*
	MCP7941X_WriteRegister(7, 0x80); // disable all alerts
	MCP7941X_SetAlert0(0x8, 0, 0, 0, 0, 0);
	MCP7941X_SetAlert1(0x8, 0, 0, 0, 0, 0);
	
	if (!(MCP7941X_ReadRegister(0) & 0x80))
	{
		MCP7941X_SetTime(0, 1, 1, 0, 0, 0);
	}

	//MCP7941X_SetTime(13, 1, 26, 22, 6, 5); // for setting the watch...needs better UI but works for nerds ;)
*/
#ifdef EYE_FRIENDLY 
	v=1;
#else

#ifndef MIDI_SHIFTER 

	PickupColorEx(20, 0);
	v = FixedSqrt(cs);
	if (v < 1) v = 1;
#endif
#endif

	int c = readRegister(0x0D);	// Read WHO_AM_I register
	if (c == 0x2A  || c == 0xC4 || c== 0xC7 || c==0x39) // WHO_AM_I should always be 0x2A //C4 or C7 for new FXOS87700
	{
		// init the accelerometer if communication is good
	} 
	else 
	{
		// no enless loop here
		int c = readRegister(0x12);

		for (int i = 0; i < 8; i++)
		{
			setRGB(i, 100, c%2 ? 255 : 0, 0);
c	/= 2;
		}
		updateLedsWithBlank();

		while (1) 
		{
			delay(1);
			//debugColor(100, 0, 0, 5, 10, 1000);
			//debugColor(0, 100, 0, 5, 10, 1000);
			//debugColor(0, 0, 100, 5, 10, 1000);
			//debugColor(0, 0, 0, 5, 10, 300);
		}
	}
	LoadBitmap(128, &font16px);
	LoadBitmap(129, &font12px);
	LoadBitmap(130, &font8px);
	LoadBitmap(131, &font6px);
}

void MagicShifter_BrightnessCtrl2()
{
	int newIdx = v;
	long avgV = 0;
	int newV = 0, lastV = -1;
	uint16_t blink = 0;
	int skip = 100;

	if (powerBtnPressed && millis() - powerBtnPressed > BrightnessCtrlTime) 
	{
	    /*
		// sorry we CANT measure the battery level while pressing the power button
		showBatteryStatus(100);
		delay(20);
		UpdateButtons();
		if (!powerBtnPressed) return;
		*/ 

		BackupAccelState();
		AccelConfig shakeConfig;
		shakeConfig.accelInterruptNr = 1;
		shakeConfig.tapInterruptNr = 0;
		shakeConfig.transientInterruptNr = 0;
		shakeConfig.accelScale = 2;
		shakeConfig.accelDataRate = 0;
		AccelSetup(&shakeConfig);

		v = 1;
		while (skip)
		{
			delay(1);
			UpdateButtons();
			if (powerBtnPressed)
				skip = 100;
			else
			{
				if (skip >0) skip--;
			}
				
			AccelPoll();

			avgV = (61 * avgV + (-accelCount[XAXIS] + 500))/62;

			newIdx = (avgV) / 35;

			if (newIdx < -5) {
				newV = 0;			
			}
			else
			{
				blink = 0;
				if (newIdx < 0) newIdx = 0;
				if (newIdx >= 15) newIdx = 15;	
				newV = BLevels[newIdx];
			}
			if (newV == 0)
			{
				setAll(0,0,0);
				blink++;
				uint16_t bb = blink&0x1FF;
				if (bb > 255) bb =511-bb; 
				//bb = (v*bb)/255;

				setRGB(4, bb/8, bb/8, bb/8);
				setRGB(5, bb/4, bb/4, bb/4);
				setRGB(6, bb/2, bb/2, bb/2);
				setRGB(7, bb, bb, bb);
				setRGB(8, bb, bb, bb);
				setRGB(9, bb/2, bb/2, bb/2);
				setRGB(10, bb/4, bb/4, bb/4);
				setRGB(11, bb/8, bb/8, bb/8);
				
			
				updateLeds();
				delayMicroseconds(200);
			}
			else if (lastV != newV)
			{
				for (uint8_t i = 0; i <= 15; i++) {

					uint8_t lBr = BLevels[i];

					uint8_t dB = lBr;

					if (dB > MAXMV) dB = MAXMV;
					if (dB < 16) dB = 16;

					if (newV >= lBr) 
						setRGB(15 - i, dB, dB, dB);
					else
						setRGB(15 - i, 0, 0, 0);
				}
				updateLeds();
				delayMicroseconds(200);
			}
			lastV = newV;
		}
		if (newV == 0)
			waitForPowerDown();
		v = newV;
		vChanged = true;
			
		WaitClearButtons();

		setAll(0,0,0);
		updateLeds();
		RestoreAccelState();
	}
}

// cleanup exact same code for 2 buttons but need fnptrs for that
void UpdateButtons()
{
	MeasureBattery(false);
	long timestamp = millis();

	// Power button 
	int _powerBtnClickedTime = timestamp - powerBtnPressed;
	if (ad > 950 && !powerBtnPressed) {
		lastActivity = timestamp;
		powerBtnPressed = timestamp;
	} else if (ad <= 950 && powerBtnPressed) {
		lastActivity = timestamp;
		if (_powerBtnClickedTime > DEBOUNCE_TIME)	// debounce
		{
			powerBtnReleased = timestamp;
			powerBtnClickedTime = _powerBtnClickedTime;
		}
		powerBtnPressed = 0;
	}
	// center button
	int _centerBtnClickedTime = timestamp - centerBtnPressed;
	if (!digitalRead(BTNC) && !centerBtnPressed) {
		lastActivity = timestamp;
		centerBtnPressed = timestamp;
	} else if (digitalRead(BTNC) && centerBtnPressed) {
		lastActivity = timestamp;
		if (_centerBtnClickedTime > DEBOUNCE_TIME)	// debounce
		{
			centerBtnReleased = timestamp;
			centerBtnClickedTime = _centerBtnClickedTime;
		}
		centerBtnPressed = 0;
	}
}

bool MagicShifter_Poll()
{
	UpdateButtons();
	USBPoll();

	long timestamp = millis();

	if (centerBtnPressed && timestamp - centerBtnPressed > MODE_EXIT_TIME) {
		centerBtnPressed = 0;
		return false;
	}

#ifndef BABYSHIFTER
	if (timestamp - lastActivity > AUTOSHUTDOWN_TIME && !(USBSTA & 0x01)) {
		blink(50, 50, 10);
		waitForPowerDown();
	}
#endif

	MagicShifter_BrightnessCtrl2();

	return true;
}

void WaitClearButtons()
{
	while (1) {
		UpdateButtons();
		if (!centerBtnPressed && !powerBtnPressed)
			break;
	}

	powerBtnPressed = 0;
	powerBtnClickedTime = 0;
	powerBtnReleased = 0;

	centerBtnPressed = 0;
	centerBtnClickedTime = 0;
	centerBtnReleased = 0;
}

int ModeSelector(int modes, int startIdx, char*Text[])
{
	int index = startIdx % modes;
	//char*Text[]={"IMG","COL","VIB","RGB","RGB2","DISCO","LIGHT","TIME","GAME"};
	while (1) {
		//int posIdx = (15 * index)/(modes-1);
		int posIdx = index;
		int colIdx = 1 + (index % 7);
		int r = (colIdx >> 0) % 2 ? v : 0;
		int g = (colIdx >> 1) % 2 ? v : 0;
		int b = (colIdx >> 2) % 2 ? v : 0;

		for (byte i = 0; i < 16; i++) {
			if (i == posIdx) {
				setRGB(i, r, g, b);
			} else {
				setRGB(i, 0, 0, 0);
			}
		}
		updateLedsWithBlank();

		MagicShifter_Poll();
		displayText(Text[index],false);
		

		if (powerBtnClickedTime != 0) {
			//if (powerBtnClickedTime < LONG_CLICK_TIME)
			{
				if (index == 0)
					index = modes - 1;
				else
					index = (index - 1) % modes;
			}
			powerBtnClickedTime = 0;
		}

		if (centerBtnClickedTime != 0) {
			//if (centerBtnClickedTime < LONG_CLICK_TIME)
			{
				index = (index + 1) % modes;
			}
			centerBtnClickedTime = 0;
		}

		if (centerBtnPressed != 0 && (millis() - centerBtnPressed) > LONG_CLICK_TIME) {
			for (int i = 0; i < 4; i++) {
				setRGB(posIdx, v, v, v);
				updateLedsWithBlank();
				delay(50);
				setRGB(posIdx, 0, 0, 0);
				updateLedsWithBlank();
				delay(50);
			}
			return index;
		}
	}
}

// independent of glob brightness
void SetWhiteLED(uint8_t val)
{
	if (val) {
		digitalWrite(LEDWHITE_PIN, HIGH);
		delayMicroseconds(30);
		for (int i = 0; i < (31 - ((val) >> 3)); i++) {
			digitalWrite(LEDWHITE_PIN, LOW);
			digitalWrite(LEDWHITE_PIN, HIGH);
		}
	} else
		digitalWrite(LEDWHITE_PIN, LOW);
}

void SetUVLED(uint8_t val)
{
	if (val) {
		digitalWrite(LEDUV_PIN, HIGH);
		delayMicroseconds(30);
		for (int i = 0; i < (31 - ((val) >> 3)); i++) {
			digitalWrite(LEDUV_PIN, LOW);
			digitalWrite(LEDUV_PIN, HIGH);
		}
	} else
		digitalWrite(LEDUV_PIN, LOW);
}

void SetDebugLeds(byte debug, byte rx, byte tx)
{
	if (debug != 0xFF)
	{
		digitalWrite(ledPin, !debug);
	}
	if (rx != 0xFF)
	{
		digitalWrite(RXLED, !rx);
	}
	if (tx != 0xFF)
	{
		if (tx)
			TXLED1;
		else
			TXLED0;
	}
}

void blink(int p1, int p2, int n)
{
	for (int i = 0; i < n; i++) {
		digitalWrite(ledPin, LOW);	// turn the LED on (HIGH is the voltage level)
		delay(p1);	// wait for a second
		digitalWrite(ledPin, HIGH);	// turn the LED off by making the voltage LOW
		delay(p2);	// wait for a second
	}
}


int loadMode()
{
	return EEPROM.read(0);
}

void saveMode(int idx)
{
	EEPROM.write(0, idx);
}

// blocking USB functions
uint8_t SerialReadByte()
{
	while (!Serial.available());
	return Serial.read();
}

uint16_t SerialReadWord()
{
	uint16_t result = ((uint16_t)SerialReadByte()) << 8;
	result |= SerialReadByte();

	return result;
}

/*
uint32_t fontStartAdress;
uint8_t fontWidth;
uint8_t fontHeight;
uint8_t fontMinChar, fontMaxChar;
*/

void LoadBitmap(uint8_t sector, MSBitmap *bitmap)
{
	uint32_t eepromHeaderAddr = (uint32_t)sector*0x1000l;
	ReadBytes(eepromHeaderAddr, (uint8_t *)(&(bitmap->header)), sizeof(MSBitmapHeader));
	bitmap->header.eepromAddr = eepromHeaderAddr + 16;
	bitmap->color.rgb.r = 255;
	bitmap->color.rgb.g = 255;
	bitmap->color.rgb.b = 255;
}

void PlotBitmapColumn1Bit(const MSBitmap *bitmap, uint16_t absColumn, uint8_t ledIdx)
{
	uint8_t bitBuffer[3];
	uint8_t bitBufferIdx = 0;

	uint16_t bitPos = absColumn * bitmap->header.frameHeight;
	uint32_t eepromAddr = bitmap->header.eepromAddr + (bitPos >> 3);
	ReadBytes(eepromAddr, bitBuffer, 3); // this could be more efficient

	uint8_t bitMask = 1 << (bitPos & 0x07);

	 // this could be more efficient
	uint8_t r = bitmap->color.rgb.r;
	uint8_t g = bitmap->color.rgb.g;
	uint8_t b = bitmap->color.rgb.b;

	uint8_t endIndex = ledIdx + bitmap->header.frameHeight;
	if (endIndex > 16)
		endIndex = 16;

	do
	{
		uint8_t currentByte = bitBuffer[bitBufferIdx++];
		do
		{
			if (bitMask & currentByte)
			{
				setRGB(ledIdx, r, g, b); // this could be more efficient memcopy the structure
			}
			else
			{
				setRGB(ledIdx, 0, 0, 0); // this could be more efficient
			}
			bitMask <<= 1;
		} while (++ledIdx < endIndex && bitMask != 0);
		bitMask = 0x01;
	} while (ledIdx < endIndex); // this could be more efficient
}


void PlotBitmapColumn24Bit(const MSBitmap *bitmap, uint16_t absColumn, uint8_t startLed)
{	
	uint8_t nrOfBytes =  3 * bitmap->header.frameHeight;
	uint32_t addr = bitmap->header.eepromAddr + absColumn * nrOfBytes;
	if (startLed + bitmap->header.frameHeight > 16)
		nrOfBytes = (16 - startLed)*3;
	ReadBytes(addr, LEDS_VALUES + 3*startLed, nrOfBytes); // never make startLed too big or it will corrupt mem
}

void PlotBitmapColumn(const MSBitmap *bitmap, uint8_t frame, uint8_t column, uint8_t startLed)
{	
	uint8_t frameIdx = frame - bitmap->header.firstChar;
	if (frameIdx <= bitmap->header.maxFrame)
	{	
		uint16_t absColumn = ((uint16_t)frameIdx) * ((uint16_t)bitmap->header.frameWidth) + column;
		switch (bitmap->header.pixelFormat)
		{
			case 24:
				PlotBitmapColumn24Bit(bitmap, absColumn, startLed);
				break;
			case 1:
				PlotBitmapColumn1Bit(bitmap, absColumn, startLed);
				break;
		}
	}
}

void PlotText(const MSBitmap *bitmap, char *text, uint16_t column, uint8_t startLed)
{
	uint8_t ascii = text[column / bitmap->header.frameWidth];
	PlotBitmapColumn(bitmap, ascii, column % bitmap->header.frameWidth, startLed);
}


void Int2Str(int val, char *text) {
	char buffer[8];
	uint8_t len=0;

	//for (len = 0; text[len] != 0; len++);

	uint8_t i = 0;

	do {
		buffer[len] = '0' + (val % 10);
		val /= 10;
		len++;
	} while(val);

	for (i = 0; i < len; i++) {
		*text = buffer[(len-1)-i];
		text++;
	}
	*text = 0;

}



void displayText(char *text,bool wait,MSBitmap *font, int posX)
{
	int n;
	int frame = 0;
	AccelConfig shakeConfig;

	//MSBitmap *font = &font16px;
	if (!font) font = &font16px;

	if (!wait && ( centerBtnClickedTime || powerBtnClickedTime || centerBtnPressed || powerBtnPressed))
  			return;

	#define RGB_BUFFERSIZE (3*16)
	uint8_t backupLedValues[RGB_BUFFERSIZE];

	for (int i = 0; i < RGB_BUFFERSIZE; i++) {
		backupLedValues[i] = LEDS_VALUES[i];
	}

	while (text[n] != 0) n++;

	font->color.rgb.r = 128;
	font->color.rgb.g = 128;
	font->color.rgb.b = 255;

	BackupAccelState();

	shakeConfig.accelInterruptNr = 0;
	shakeConfig.tapInterruptNr = 0;
	shakeConfig.transientInterruptNr = 1;
	shakeConfig.accelScale = 2;
	shakeConfig.accelDataRate = 2;	
	AccelSetup(&shakeConfig);
 
	AccelShakeNrOfFrames = n*COLUMNMULTIPLY*font->header.frameWidth;

	if(wait) 
		WaitClearButtons();

	while (1)
	{
		frame++;

		if (!MagicShifter_Poll() || centerBtnPressed || powerBtnPressed)
  			break;

		AccelPoll();
		UpdateAccelShakeCurrentColumn();
		
		if (AccelShakeFrameNr != 0xFF)
		{
			if (AccelShakeFrameNr < 0xFE)
			{	
				uint8_t realFrame = AccelShakeFrameNr / COLUMNMULTIPLY;

				PlotText(font, text, realFrame, 0);
				updateLedsWithBrightness();
			}
			else
			{	
				setAll(0,0,0);
				updateLedsWithBrightness();
			}
		}
		else {
			for (int i = 0; i < RGB_BUFFERSIZE; i++) {
				LEDS_VALUES[i] = backupLedValues[i];
			}
			updateLedsWithBrightness();
		}
	}

	for (int i = 0; i < RGB_BUFFERSIZE; i++) {
		LEDS_VALUES[i] = backupLedValues[i];
	}
	updateLeds();
	RestoreAccelState();

	if(wait) 
		WaitClearButtons();
}
