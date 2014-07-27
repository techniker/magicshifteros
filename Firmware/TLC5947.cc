#include "firmware.h"
#include <avr/io.h>
#include <stdint.h>
#include <SPI.h>

#define TLC5947_USE_HW

#define TLCPORT_SPI PORTB
//#define TLCPORT_SPI_DIR DDRB


#define SCLK_PIN 1
#define SIN_PIN 2

#ifdef BABYSHIFTER
#define LEDS_BLANK  10		// c7
#define LEDS_LATCH  9		// c6
#else
#define LEDS_BLANK  4		// PD4/Pin4
#define LEDS_LATCH  12		// PD6/26/Pin12
#define TLCPORT_CTRL PORTD
//#define TLCPORT_CTRL_DIR DDRD

#endif


#define LEDS_CLOCK  SCK		// pb5 according to pins_ardino.h of promicro
#define LEDS_DATA   MOSI	// according to pins_ardino.h of promicro

#define RGB_BUFFERSIZE (3*LEDS)

uint8_t LEDS_VALUES[RGB_BUFFERSIZE];

void initLEDS()
{
/*
  #ifdef TLC5947_USE_HW 
   cli();

  // Set pins to output
  TLCPORT_SPI_DIR |= _BV(SCLK_PIN) | _BV(SIN_PIN);
  TLCPORT_CTRL &= ~_BV(BLANK_PIN);   // blank everything until ready
  TLCPORT_CTRL_DIR |= _BV(XLAT_PIN) | _BV(XLAT_PIN);

  sei();
#else
*/
	pinMode(LEDS_CLOCK, OUTPUT);
	pinMode(LEDS_DATA, OUTPUT);
	digitalWrite(LEDS_BLANK, HIGH);
	pinMode(LEDS_BLANK, OUTPUT);
	digitalWrite(LEDS_BLANK, HIGH);
	pinMode(LEDS_LATCH, OUTPUT);


	SPI.begin();
	SPI.setBitOrder(MSBFIRST);
	SPI.setDataMode(SPI_MODE0);
	SPI.setClockDivider(SPI_CLOCK_DIV2);	// 1 MHz max, else flicker


	/*
	   SPDR = pixels[i];
	   while(!(SPSR & (1<<SPIF)));

	 */
//#endif

	setAll(0, 0, 0);
	updateLeds();
	enableLeds(1);
}

void enableLeds(uint8_t val)
{
	digitalWrite(LEDS_BLANK, val ? LOW : HIGH);
}



#ifdef TLC5947_USE_HW
void send8(byte v)
{
	for (int i = 0x80; i; i >>= 1) {
		if (v & i)
			TLCPORT_SPI |= _BV(SIN_PIN);
		else
			TLCPORT_SPI &= ~_BV(SIN_PIN);
		TLCPORT_SPI |= _BV(SCLK_PIN);
		TLCPORT_SPI &= ~_BV(SCLK_PIN);
	}
}
#else
void send8(byte pixel)
{
	for (int i = 0; i < 8; i++) {
		if (pixel & 1)
			digitalWrite(LEDS_DATA, HIGH);
		else
			digitalWrite(LEDS_DATA, LOW);
		pixel >>= 1;

		digitalWrite(LEDS_CLOCK, HIGH);
		digitalWrite(LEDS_CLOCK, LOW);

	}
}
#endif

#ifdef TLC5947_USE_HW
void send12(int v)
{
	for (int i = 0x800; i; i >>= 1) {
		if (v & i)
			TLCPORT_SPI |= _BV(SIN_PIN);
		else
			TLCPORT_SPI &= ~_BV(SIN_PIN);
		TLCPORT_SPI |= _BV(SCLK_PIN);
		TLCPORT_SPI &= ~_BV(SCLK_PIN);
	}
}
#else
void send12(int pixel)
{
	for (int i = 0; i < 12; i++) {
		if (pixel & 0x800)
			digitalWrite(LEDS_DATA, HIGH);
		else
			digitalWrite(LEDS_DATA, LOW);
		pixel <<= 1;

		digitalWrite(LEDS_CLOCK, HIGH);
		digitalWrite(LEDS_CLOCK, LOW);

	}
}
#endif


void DebugClear()
{
	for (int i = 0; i < RGB_BUFFERSIZE; i++) {
		LEDS_VALUES[i] = 0;
	}
}

void setAll(uint8_t r, uint8_t g, uint8_t b)
{
	for (int i = 0; i < LEDS; i++) {
		setRGB(i, r, g, b);
	}
}

void setAllChannel(uint8_t channel, uint8_t value)
{
	for (int i = 0; i < LEDS; i++) {
		setChannel(i, channel, value);
	}
}

void setChannel(uint8_t index, uint8_t channelIndex, uint8_t value)
{
  if (index >= 0 && index < LEDS && channelIndex >= 0 && channelIndex < 3)
  {
    LEDS_VALUES[index*3+channelIndex]=value;
  }
}

void setRGB(uint8_t index, uint8_t r, uint8_t g, uint8_t b)
{
	uint8_t colIndex = index*3;
	if (index >= 0 && index < LEDS) {
#ifdef COLOR_CODING_RGB
		LEDS_VALUES[colIndex++] = r;
		LEDS_VALUES[colIndex++] = g;
		LEDS_VALUES[colIndex++] = b;
#endif
#ifdef COLOR_CODING_GRB
		LEDS_VALUES[colIndex++] = g;
		LEDS_VALUES[colIndex++] = r;
		LEDS_VALUES[colIndex++] = b;
#endif
	}
}

/*
void setRGB(int index, int r, int g, int b)
{

#ifdef BABYSHIFTER

#else
	if (index < 8)
		index = 7 - index;
	else
		index = (23 - index);
#endif

	if (index >= 0 && index < LEDS) {
		int bitIndex = (3 * 12) * index;
		int byteIndex = bitIndex >> 3;
		byte bitPos = bitIndex & 0x7;

		if (bitPos == 0) {
			LEDS_VALUES[byteIndex++] = r >> 4;
			LEDS_VALUES[byteIndex++] = (r << 4) | (g >> 8);
			LEDS_VALUES[byteIndex++] = g;
			LEDS_VALUES[byteIndex++] = b >> 4;
			LEDS_VALUES[byteIndex] = (LEDS_VALUES[byteIndex] & 0x0F) | (b << 4);
		} else {
			LEDS_VALUES[byteIndex] = (LEDS_VALUES[byteIndex] & 0xF0) | (r >> 8);
			LEDS_VALUES[++byteIndex] = r;
			LEDS_VALUES[++byteIndex] = g >> 4;
			LEDS_VALUES[++byteIndex] = ((g & 0xF) << 4) | (b >> 8);
			LEDS_VALUES[++byteIndex] = b;
		}
	}
}
*/
/*
void updateLeds()
{
	for (int i = 0; i < RGB_BUFFERSIZE; i++) {
		//send8(LEDS_VALUES[i]);
		SPDR = LEDS_VALUES[i];
		while (!(SPSR & (1 << SPIF))) ;
	}
	digitalWrite(LEDS_LATCH, HIGH);
	digitalWrite(LEDS_LATCH, LOW);
}

void updateLedsWithBlank()
{
	for (int i = 0; i < RGB_BUFFERSIZE; i++) {
		//send8(LEDS_VALUES[i]);
		SPDR = LEDS_VALUES[i];
		while (!(SPSR & (1 << SPIF))) ;
	}

	digitalWrite(LEDS_BLANK, HIGH);

	digitalWrite(LEDS_LATCH, HIGH);
	digitalWrite(LEDS_LATCH, LOW);

	digitalWrite(LEDS_BLANK, LOW);

}
*/

void updateLedsWithBlank() // depricated
{
	updateLedsWithBrightness();
}

void updateLeds() // depricated
{
	updateLedsWithBrightness();
}

// 32 bit quadratic
void updateLedsWithBrightness32q()
{
	uint8_t bufferIdx, nextValue;
	uint32_t bValChA, bValChB;

	uint8_t brigh = v;

	//bufferIdx = (16*3)-1;
	bufferIdx = (8*3)-1;
 	//bufferIdx = 0;
	do
	{
		bValChA = (((uint16_t)LEDS_VALUES[bufferIdx]) * brigh);
		bValChA *= LEDS_VALUES[bufferIdx--];
		nextValue = bValChA >> 16;

		//if (bufferIdx != ((8*3)-2)) while (!(SPSR & (1 << SPIF))) ;
		SPDR = nextValue;

		bValChB = (((uint16_t)LEDS_VALUES[bufferIdx]) * brigh);
		bValChB *= LEDS_VALUES[bufferIdx--];
		nextValue = (((byte)(bValChA >> 8)) & 0xF0) | (bValChB>>20); //nextValue = (((lowByte)bValChA) & 0xF0) | highByte(bValChB)>>4;


		//dummy = SPSR;	// this does not wait but clears the flag, reading dummy takes 1 cycle	
		//while (!(SPSR & (1 << SPIF))) ;	// this really waits								
		SPDR = nextValue;

		nextValue = bValChB>>12;

		if (bufferIdx == 0xFF)
			bufferIdx = (16*3)-1;
	
		while (!(SPSR & (1 << SPIF))) ;
		SPDR = nextValue;

		//while (!(SPSR & (1 << SPIF))) ;
	} while (bufferIdx != ((8*3)-1));

	while (!(SPSR & (1 << SPIF))) ;
	latchLedsWithBlank();
}

// 16bit quadr
void updateLedsWithBrightness16q()
{
	uint8_t bufferIdx, nextValue;
	uint16_t bValChA, bValChB;

	uint8_t brigh = v >> 4;

	//bufferIdx = (16*3)-1;
	bufferIdx = (8*3)-1;
 	//bufferIdx = 0;
	do
	{
		bValChA = (((uint16_t)(LEDS_VALUES[bufferIdx] * LEDS_VALUES[bufferIdx])>>4) * brigh);
		bufferIdx--;
		nextValue = bValChA >> 8;

		if (bufferIdx != ((8*3)-2)) while (!(SPSR & (1 << SPIF))) ;
		SPDR = nextValue;

		bValChB = (((uint16_t)(LEDS_VALUES[bufferIdx] * LEDS_VALUES[bufferIdx])>>4) * brigh);
		bufferIdx--;
		nextValue = (((byte)bValChA) & 0xF0) | (bValChB>>12); //nextValue = (((lowByte)bValChA) & 0xF0) | highByte(bValChB)>>4;


		//dummy = SPSR;	// this does not wait but clears the flag, reading dummy takes 1 cycle	
		while (!(SPSR & (1 << SPIF))) ;	// this really waits								
		SPDR = nextValue;

		nextValue = bValChB>>4;

		if (bufferIdx == 0xFF)
			bufferIdx = (16*3)-1;
	
		while (!(SPSR & (1 << SPIF))) ;
		SPDR = nextValue;

		//while (!(SPSR & (1 << SPIF))) ;
	} while (bufferIdx != ((8*3)-1));

	while (!(SPSR & (1 << SPIF))) ;
	latchLedsWithBlank();
}

// 16 bit linear
void updateLedsWithBrightness()
{
	uint8_t bufferIdx, nextValue;
	uint16_t bValChA, bValChB;

	uint8_t brigh = v;

	//bufferIdx = (16*3)-1;
	bufferIdx = (8*3)-1;
 	//bufferIdx = 0;
	do
	{
		bValChA = (((uint16_t)LEDS_VALUES[bufferIdx--]) * brigh);
		nextValue = bValChA >> 8;

		if (bufferIdx != ((8*3)-2)) while (!(SPSR & (1 << SPIF))) ;
		SPDR = nextValue;

		bValChB = (((uint16_t)LEDS_VALUES[bufferIdx--]) * brigh);
		nextValue = (((byte)bValChA) & 0xF0) | (bValChB>>12); //nextValue = (((lowByte)bValChA) & 0xF0) | highByte(bValChB)>>4;


		//dummy = SPSR;	// this does not wait but clears the flag, reading dummy takes 1 cycle	
		while (!(SPSR & (1 << SPIF))) ;	// this really waits								
		SPDR = nextValue;

		nextValue = bValChB>>4;

		if (bufferIdx == 0xFF)
			bufferIdx = (16*3)-1;
	
		while (!(SPSR & (1 << SPIF))) ;
		SPDR = nextValue;

		//while (!(SPSR & (1 << SPIF))) ;
	} while (bufferIdx != ((8*3)-1));

	while (!(SPSR & (1 << SPIF))) ;
	latchLedsWithBlank();
}

void latchLedsWithBlank()
{
	digitalWrite(LEDS_BLANK, HIGH);

	digitalWrite(LEDS_LATCH, HIGH);
	digitalWrite(LEDS_LATCH, LOW);

	digitalWrite(LEDS_BLANK, LOW);
}


