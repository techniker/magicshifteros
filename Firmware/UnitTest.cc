#include <avr/eeprom.h>
#include <avr/wdt.h>
#include "firmware.h"

// on first run (checking with internal eeprom) test wakeup with rtc (takes about 2 seconds)
#define DO_TCHRG_CHECK
#define DO_RTC_CHECK
#define DO_PROX_CHECK 
#define DO_ACCEL_CHECK
#define DO_LED_CHECK
#define WRITE_CALIBRATION_VALUES 

//#define RESET_AFTER_RESULT 
#define EEPROM_SET 0xAA

uint8_t eepromNotInitialBoot EEMEM = 0xFF;
uint8_t eepromSuccessAuto EEMEM = 0xFF;
uint8_t eepromSuccessManual EEMEM = 0xFF;

#define ACCEL_ERROR_MARGIN 200

#define COLOR_TEST_DURATION_SCALE 1
#define COLOR_TEST_TIME (20 *COLOR_TEST_DURATION_SCALE)
#define NR_COLOR_TESTS 4 
// #define LIGHTSENSOR_MARGIN  0.22 // both sides 0.18 // only 7 side 0.16 // range 0.2 -   //china export
#define LIGHTSENSOR_MARGIN  0.35 // 2: 0.18 //0.35 // homecandidate 0.35-
// how much is lost on the side
#define LIGHTFALLOFF_SIDE 0.31 //0.305 // range 0.3-0.31
int ColorTestValues[NR_COLOR_TESTS][3] = {{0,0,4095},{0,4095,0},{4095, 0, 0}, {4095,4095,4095}};

//int ExpectedRGBValues[NR_COLOR_TESTS][4] = {{430, 402, 20, 37},   {244, 25, 152, 62},          {398, 8, 79, 311}, {1080, 433, 254, 412}}; // LED7 chinaexport
/*int ExpectedRGBValues[NR_COLOR_TESTS][4] =   
{{1.2*430, 402, 20, 37+1+2}, 
{244-10, 25-1, 152-8, 62-3}, 
{398+7-3, 8+1-1, 79+2-0, 311+7}, 
{1080, 433, 254, 412}}; // tuned chinaexport
*/

int brightMod[16] = {5917,6710,7274,7696,8538,8777,9815,9749,9747,9716,9353,8831,8469,7650,7004,6302};

int ExpectedRGBValues[NR_COLOR_TESTS][4] = {{439,413, 22, 39}, {229,	26, 143,	60}, {431, 11, 91, 331}, {1071, 441, 244, 421}};

/*
{{((int)(1.2*430)), ((int)(1.2*402)), ((int)(1.2*20)), ((int)(1.2*37+1+2))}, 
{((int)(1.2*244-10)), ((int)(1.2*25-1)), ((int)(1.2*152-8)), ((int)(1.2*62-3))}, 
{((int)(1.2*398+7-3)), ((int)(1.2*8+1-1)), ((int)(1.2*79+2-0)), ((int)(1.2*311+7))}, 
{((int)(1.2*1080)), ((int)(1.2*433)), ((int)(1.2*254)), ((int)(1.2*412))}}; // tuned chinaexport
*/
/*
//china
{
{{324, 297, 15, 29}, {164, 16, 98, 44}, {264, 6, 52, 204}, {750, 318, 164, 277}},
{{354, 325, 16, 32}, {181, 18, 109, 48}, {279, 6, 54, 216}, {811, 347, 179, 296}}, 
{{363, 333, 17, 32}, {190, 18, 114, 52}, {318, 7, 63, 245}, {868, 356, 194, 328}}, 
{{388, 357, 18, 34}, {213, 20, 130, 57}, {327, 7, 65, 252}, {928, 383, 212, 344}}, 
{{472, 437, 22, 44}, {209, 22, 129, 51}, {400, 8, 81, 309}, {1079, 464, 231, 403}}, 
{{446, 410, 21, 39}, {245, 24, 150, 64}, {402, 8, 79, 312}, {1091, 440, 249, 414}}, 
{{493, 454, 23, 44}, {228, 24, 136, 60}, {459, 9, 97, 350}, {1179, 487, 256, 454}}, 
{{484, 444, 23, 41}, {269, 27, 166, 69}, {441, 9, 87, 339}, {1194, 479, 276, 449}}, 
{{491, 452, 23, 43}, {266, 25, 162, 71}, {461, 9, 90, 356}, {1218, 485, 274, 471}}, 
{{488, 450, 23, 43}, {252, 26, 155, 64}, {456, 9, 95, 350}, {1196, 484, 272, 457}}, {{440, 405, 21, 38}, {242, 26, 150, 59}, {444, 9, 90, 341}, {1125, 438, 260, 439}}, {{423, 393, 19, 39}, {216, 22, 129, 58}, {370, 8, 73, 286}, {1009, 421, 221, 384}}, {{386, 357, 18, 34}, {186, 20, 115, 46}, {373, 8, 76, 287}, {944, 384, 209, 367}}, {{361, 335, 17, 32}, {200, 21, 123, 51}, {320, 7, 63, 247}, {879, 361, 202, 330}}, {{328, 304, 15, 29}, {182, 18, 112, 48}, {302, 7, 59, 233}, {810, 328, 185, 310}}, {{298, 277, 14, 26}, {152, 15, 94, 40}, {264, 6, 53, 204}, {712, 296, 160, 270}}};


DUMPED: ACC={-17,-1,991}; int proxBase=601; XTRA[NR_EXTRA_TESTS][4]={{0, 0, 0, 0}, {248, 103, 79, 67}, {1553, 635, 490, 434}, {22, 3, 3, 15}, {101, 15, 14, 68}, {301, 294, 245, 284}}; RGB[16][NR_COLOR_TESTS][4]={{{324, 297, 15, 29}, {164, 16, 98, 44}, {264, 6, 52, 204}, {750, 318, 164, 277}}, {{354, 325, 16, 32}, {181, 18, 109, 48}, {279, 6, 54, 216}, {811, 347, 179, 296}}, {{363, 333, 17, 32}, {190, 18, 114, 52}, {318, 7, 63, 245}, {868, 356, 194, 328}}, {{388, 357, 18, 34}, {213, 20, 130, 57}, {327, 7, 65, 252}, {928, 383, 212, 344}}, {{472, 437, 22, 44}, {209, 22, 129, 51}, {400, 8, 81, 309}, {1079, 464, 231, 403}}, {{446, 410, 21, 39}, {245, 24, 150, 64}, {402, 8, 79, 312}, {1091, 440, 249, 414}}, {{493, 454, 23, 44}, {228, 24, 136, 60}, {459, 9, 97, 350}, {1179, 487, 256, 454}}, {{484, 444, 23, 41}, {269, 27, 166, 69}, {441, 9, 87, 339}, {1194, 479, 276, 449}}, {{491, 452, 23, 43}, {266, 25, 162, 71}, {461, 9, 90, 356}, {1218, 485, 274, 471}}, {{488, 450, 23, 43}, {252, 26, 155, 64}, {456, 9, 95, 350}, {1196, 484, 272, 457}}, {{440, 405, 21, 38}, {242, 26, 150, 59}, {444, 9, 90, 341}, {1125, 438, 260, 439}}, {{423, 393, 19, 39}, {216, 22, 129, 58}, {370, 8, 73, 286}, {1009, 421, 221, 384}}, {{386, 357, 18, 34}, {186, 20, 115, 46}, {373, 8, 76, 287}, {944, 384, 209, 367}}, {{361, 335, 17, 32}, {200, 21, 123, 51}, {320, 7, 63, 247}, {879, 361, 202, 330}}, {{328, 304, 15, 29}, {182, 18, 112, 48}, {302, 7, 59, 233}, {810, 328, 185, 310}}, {{298, 277, 14, 26}, {152, 15, 94, 40}, {264, 6, 53, 204}, {712, 296, 160, 270}}};


//homedevice
DUMPED: ACC={-7,-7,1011}; int proxBase=650; XTRA[NR_EXTRA_TESTS][4]={{8, 8, 6, 6}, {335, 140, 109, 98}, {2017, 811, 629, 586}, {35, 12, 10, 25}, {127, 26, 21, 87}, {370, 368, 303, 351}}; RGB[16][NR_COLOR_TESTS][4]=
{
{{289, 266, 18, 31}, {159, 22, 96, 46}, {231, 12, 48, 182}, {663, 284, 150, 247}}, 
{{330, 304, 20, 35}, {176, 26, 106, 50}, {308, 14, 64, 242}, {797, 326, 177, 314}}, 
{{342, 315, 21, 35}, {182, 26, 112, 49}, {313, 13, 68, 245}, {821, 338, 188, 316}}, 
{{378, 349, 22, 39}, {201, 26, 123, 57}, {328, 14, 66, 259}, {893, 373, 199, 343}}, 
{{413, 382, 24, 41}, {221, 30, 137, 59}, {411, 15, 85, 323}, {1030, 410, 234, 412}}, 
{{443, 410, 25, 44}, {239, 31, 143, 68}, {386, 15, 79, 303}, {1051, 439, 235, 403}}, 
{{484, 449, 27, 48}, {250, 34, 154, 65}, {476, 17, 99, 372}, {1193, 483, 267, 473}}, 
{{504, 470, 28, 49}, {256, 33, 154, 72}, {453, 16, 91, 355}, {1197, 502, 261, 464}}, 
{{497, 464, 28, 48}, {254, 32, 158, 68}, {469, 16, 95, 368}, {1204, 496, 268, 472}}, {{474, 444, 27, 46}, {254, 31, 156, 72}, {421, 16, 87, 328}, {1133, 475, 257, 434}}, {{461, 436, 26, 47}, {247, 31, 153, 69}, {442, 16, 89, 348}, {1135, 467, 255, 452}}, {{399, 377, 24, 39}, {228, 29, 141, 65}, {388, 15, 79, 304}, {1001, 405, 231, 397}}, {{417, 397, 24, 42}, {202, 29, 125, 57}, {387, 15, 84, 301}, {991, 424, 220, 387}}, {{358, 341, 22, 36}, {207, 27, 130, 58}, {351, 14, 73, 275}, {898, 365, 212, 357}}, {{319, 304, 20, 33}, {197, 26, 122, 56}, {310, 14, 64, 244}, {809, 327, 193, 320}}, {{296, 283, 19, 31}, {171, 25, 104, 50}, {290, 13, 62, 227}, {740, 304, 173, 294}}};



*/



#define NR_EXTRA_TESTS 6 // black, white, UV, IR
//int ExpectedExtraRGBValues[NR_EXTRA_TESTS][4] =   {{0, 0, 0, 0}, {245, 101, 78, 67}, {1542, 629, 486, 434}, {23, 6, 5, 15}, 
//																	 {102, 16, 14, 69}, {284, 278, 233, 268}}; // china export


int ExpectedExtraRGBValues[NR_EXTRA_TESTS][4] =   {{1,1,	1,	1},{245,102,79,66},{1536,633,492,420},{24,	4,	4,	15},{104,17,14,	67},{302,	292,	247,	281}};

//{{8, 8, 6, 6}, {335, 140, 109, 98}, {2017, 811, 629, 586}, {35, 12, 10, 25}, {127, 26, 21, 87}, {370, 368, 303, 351}};


//{{0, 0, 0, 0}, {337, 138, 108, 97}, {2079, 827, 648, 601}, {26, 5, 4, 18}, {121, 22, 18, 83}, {344, 340, 283, 326}}; // china export


//float ExtraMargins[NR_EXTRA_TESTS] = {0, 0.1, 0.1, 0.1, 0.1, 0.1}; // chinaexport
float ExtraMargins[NR_EXTRA_TESTS] = {0, 0.45, 0.45, 0.45, 0.45, 0.45};

int MeasuredAccelValues[3];
//int MeasuredTestValues[NR_COLOR_TESTS][16][4];
int MeasuredExtraTestValues[NR_EXTRA_TESTS][4];


//	['7 c 422 r 391 g 20 b 36 p 551']
//['7 c 429 r 396 g 20 b 37 p 595']
//['7 c 435 r 404 g 21 b 37 p 639']
// ['1# 7 c 244 r 25 g 152 b 62 p 568']
// ['2# 7 c 398 r 8 g 79 b 311 p 578']
// ['3# 7 c 1080 r 433 g 254 b 412 p 588']






// x x x 28
#define PROXIMITY_EXPECTED 600
#define PROXIMITY_SLACK 0.5

#define dprint(x) Serial.print(x)
#define dprintln(x) Serial.println(x)


//#define dprint(x)
//#define dprintln(x)


/*
void SaveEnergyHack()
{
		enableLeds(false); 		

		//CLKPR = (1 << CLKPCE);	// enable a change to CLKPR 
		//CLKPR = 3;		// set the CLKDIV to 0 - was 0011b = div by 8 

		delay(400);

		//CLKPR = (1 << CLKPCE);	// enable a change to CLKPR 
		//CLKPR = 0;		// set the CLKDIV to 0 - was 0011b = div by 8 		

		enableLeds(true);
}
//*/

/*

int16_t temperature_reading() 
{ 
    
   // Set MUX to use on-chip temperature sensor 
   ADMUX |= (1 << MUX0) | (1 << MUX1) | (1 << MUX2); 
   ADCSRB |=  (1 << MUX5);   // MUX 5 bit part of ADCSRB 
    
   // Enable ADC conversions 
   ADCSRA |= (1 << ADEN); 
          
   ADCSRA |= (1 << ADSC); 
    
   while(!(ADCSRA & (1 << ADIF))); 
   ADCSRA |= (1 << ADIF); 
    
   delayMicroseconds(5);

   //disable ADC...now new values can be written in MUX register 
   ADCSRA &= ~(1 << ADEN);    
    
   //delete MUX configuration for changing to another channel
   ADMUX &= ~((1 << MUX0)|(1 << MUX1)|(1 << MUX2)); 
   ADCSRB &= ~(1 << MUX5); 
    
   return ADC; 
} */



// based on: https://bitbucket.org/fmalpartida/new-liquidcrystal/src/1cf625ca9242/examples/i2cLCDextraIO_tempLeonardo/i2cLCDextraIO_tempLeonardo.ino
/*!
    @defined    TEMP_CAL_OFFSET
    @abstract   Temperature calibration offset.
    @discussion This is the offset value that has to be modified to get a
                correct temperature reading from the internal temperature sensor
                of your AVR.
*/

/*
#define TEMP_CAL_OFFSET 282

static int readTemperature()
{
   ADMUX = 0xC7;                          // activate interal temperature sensor, 
                                          // using 2.56V ref. voltage
   ADCSRB |= _BV(MUX5);
   
   ADCSRA |= _BV(ADSC);                   // start the conversion
   while (bit_is_set(ADCSRA, ADSC));      // ADSC is cleared when the conversion 
                                          // finishes
                                          
   // combine bytes & correct for temperature offset (approximate)
   //return ( (ADCL | (ADCH << 8)) - TEMP_CAL_OFFSET);  
	return ( (ADC) - TEMP_CAL_OFFSET);  
}
*/

void updateEEPROMAutoState(bool sucess)
{
	byte currentState = eeprom_read_byte(&eepromSuccessAuto);
	
	if (sucess)
	{
		if (currentState != EEPROM_SET)
			eeprom_write_byte(&eepromSuccessAuto, EEPROM_SET);		
	}
	else
	{
		if (currentState != 0)
			eeprom_write_byte(&eepromSuccessAuto, 0);	
	}
}

void updateEEPROMManualState(bool sucess)
{
	byte currentState = eeprom_read_byte(&eepromSuccessManual);
	
	if (sucess)
	{
		if (currentState != EEPROM_SET)
			eeprom_write_byte(&eepromSuccessManual, EEPROM_SET);		
	}
	else
	{
		if (currentState != 0)
			eeprom_write_byte(&eepromSuccessManual, 0);	
	}
}

void ReportErrorWithResult(String text, int expected, int result, int format)
{
	updateEEPROMManualState(false);
	updateEEPROMAutoState(false);

	Serial.print("MAGIC_ERROR:");
	Serial.print(text);
	Serial.print("; expected ");
	Serial.print(expected, format);
	Serial.print("; result ");
	Serial.println(result, format);

#ifdef RESET_AFTER_RESULT
	for (int i = 0; i < 3; i++) {
#else
	while (1) {
#endif
		setAll(0, 0, 10);
		updateLedsWithBlank();	
		delay(250);
		setAll(0, 0, 0);
		updateLedsWithBlank();	
		delay(250);
	}
	MCP7941X_WriteRegister(7, 0x80); // disable all alerts
	MCP7941X_SetTime(0, 1, 1, 0, 0, 0);
	MCP7941X_SetAlert0(0x0, 0, 0, 0, 0, 1);
	MCP7941X_WriteRegister(7, 0x90); // enable alert 0
	powerDown();
}

void ReportError(String text)
{
	updateEEPROMAutoState(false);
	updateEEPROMManualState(false);

	Serial.print("MAGIC_ERROR:");
	Serial.println(text);
	
#ifdef RESET_AFTER_RESULT
	for (int i = 0; i < 3; i++) {
#else
	while (1) {
#endif
		setAll(0, 0, 10);
		updateLedsWithBlank();	
		delay(250);
		setAll(0, 0, 0);
		updateLedsWithBlank();	
		delay(250);
	}
	MCP7941X_WriteRegister(7, 0x80); // disable all alerts
	MCP7941X_SetTime(0, 1, 1, 0, 0, 0);
	MCP7941X_SetAlert0(0x0, 0, 0, 0, 0, 1);
	MCP7941X_WriteRegister(7, 0x90); // enable alert 0
	powerDown();
}

void ReportOK(String text)
{
	Serial.print("MAGIC_OK:");
	Serial.println(text);
	
	int ledIdx = 0;
#ifdef RESET_AFTER_RESULT
	for (int i = 0; i < 100; i++) {
#else
	while (1) {
#endif
		
		if (ledIdx == 0)
			digitalWrite(RXLED, LOW);
		else
			digitalWrite(RXLED, HIGH);

		if (ledIdx == 1)
			TXLED1;
		else
			TXLED0;

		if (ledIdx == 2)
			digitalWrite(ledPin, LOW);	
		else
			digitalWrite(ledPin, HIGH);	


		ledIdx = (ledIdx + 1) % 3;

		setAll(0, 10, 0);
		updateLedsWithBlank();	
		delay(125);
		setAll(0, 10, 0);
		updateLedsWithBlank();	
		delay(125);
	}
	MCP7941X_WriteRegister(7, 0x80); // disable all alerts
	MCP7941X_SetTime(0, 1, 1, 0, 0, 0);
	MCP7941X_SetAlert0(0x0, 0, 0, 0, 0, 1);
	MCP7941X_WriteRegister(7, 0x90); // enable alert 0
	powerDown();
}

void DoLightSensing()
{
	updateLedsWithBlank();	
	CheckTCS3771(COLOR_TEST_TIME);
	TCS3771_WriteRegister(0x00, 0x00);
}

void eeepromWriteInt(int &idx, int v)
{
	byte h = v>>8;
	byte l = v;
	WriteByte(idx++, h);
	WriteByte(idx++, l);
}

void eeepromWriteColor(int &idx, int c, int r, int g, int b)
{
	eeepromWriteInt(idx, c);
	eeepromWriteInt(idx, r);
	eeepromWriteInt(idx, g);
	eeepromWriteInt(idx, b);
}


int eeepromReadInt(int &idx)
{
	byte h, l;
	ReadBytes(idx++, &h, 1);
	ReadBytes(idx++, &l, 1);
	return (((int)h) << 8) | l;
}

bool eeepromTestInt(int &idx, int v)
{
	/*byte h, l;
	ReadBytes(idx++, &h, 1);
	ReadBytes(idx++, &l, 1);
	return ((v >> 8) == h && ((v & 0xFF) == l));
	*/
	return eeepromReadInt(idx) == v;
}

bool eeepromTestColor(int &idx, int *vals)
{
	bool result = true;
	result &= eeepromTestInt(idx, vals[0]);
	result &= eeepromTestInt(idx, vals[1]);
	result &= eeepromTestInt(idx, vals[2]);
	result &= eeepromTestInt(idx, vals[3]);
	return result;
}


void UnitTestMode()
{
	char commandbuffer[100];
	int bufferIndex;
	int randomNr = 0;
#ifdef WRITE_CALIBRATION_VALUES 
	int eepromIdx;
#endif

	while (0)
	{
	
			digitalWrite(RXLED, LOW);
		
			TXLED1;
		
			digitalWrite(ledPin, LOW);	
		}

	// capcheck
	while (0) {
		enableTurboCharge(false);

		setAll(0, 0, 0);
		for (int i = 0; i < 100; i++) {
			setRGB(i, 4095, 4095, 4095);
			updateLedsWithBlank();	
			delay(1000);
		}
		setAll(0, 0, 0);
		updateLedsWithBlank();	
		delay(1000);
	}

#ifdef DO_RTC_CHECK
	// reed eeprom
	byte notInitialBoot = eeprom_read_byte(&eepromNotInitialBoot);
	
	if (notInitialBoot != EEPROM_SET)
	{
		// Initial Boot 	
		debugColor(20, 20, 0, 5, 11, 100);
			
		eeprom_write_byte(&eepromNotInitialBoot, EEPROM_SET);		

		MCP7941X_WriteRegister(7, 0x80); // disable all alerts
		MCP7941X_SetTime(0, 1, 1, 0, 0, 0);
		MCP7941X_SetAlert0(0x0, 0, 0, 0, 0, 1);
		MCP7941X_WriteRegister(7, 0x90); // enable alert 0
		powerDown();
	}
#endif	

#ifdef NOSERIAL
	Serial.begin(9600);
#endif

	int ledIdx;
	#define COMMANDBUFFERSIZE 100
	char cmdBuffer[100];
	int cmdBufferIdx = 0;



	while (1) 
	{
		byte lastTimeSuccessAuto = eeprom_read_byte(&eepromSuccessAuto);
		byte lastTimeSuccessManual = eeprom_read_byte(&eepromSuccessManual);


		if (lastTimeSuccessAuto == EEPROM_SET && lastTimeSuccessManual == EEPROM_SET)
		{
			setAll(0, 10, 0);
			updateLedsWithBlank();	
			delay(100);
		}
		else
		{
			setAll(0, 0, 10);
			
			if (lastTimeSuccessAuto == EEPROM_SET)
				setRGB(0, 0, 10, 10);
			if (lastTimeSuccessManual == EEPROM_SET)
				setRGB(1, 0, 10, 10);

			updateLedsWithBlank();	
			delay(100);
			
		}
		setAll(0, 0, 0);
		updateLedsWithBlank();	
		delay(100);
		
	
		if(Serial.available())
		{
			debugColor(10, 10, 10, 5, 6 + 5, 100); 
			debugColor(0, 0, 0, 0, 16, 1);
			//delay(100);

			cmdBufferIdx = 0;
			while(Serial.available() && cmdBufferIdx < COMMANDBUFFERSIZE-1) 
			{
				char b = Serial.read();
				if (b == '\n' || b == '\r')
					break;
				cmdBuffer[cmdBufferIdx++] = b;
			}
			cmdBuffer[cmdBufferIdx++]='\0';

			bool executed = false;
		
///*
			if (!strcmp(cmdBuffer, "MAGIC_DUMP"))
			{
				eepromIdx = 0;
				int rdV;
				Serial.print("ACC={");
				for (int axis = 0; axis < 3; axis++)
				{
					rdV = eeepromReadInt(eepromIdx);	
					if (axis != 0) Serial.print(",");
					Serial.print(rdV);

				}
				Serial.print("}; int proxBase=");

				eepromIdx = 16;
				rdV = eeepromReadInt(eepromIdx);	
				Serial.print(rdV);
				Serial.print("; XTRA[NR_EXTRA_TESTS][4]={");

				eepromIdx = 128;

				for (int testIdx = 0; testIdx < NR_EXTRA_TESTS; testIdx++)
				{
					if (testIdx != 0) Serial.print(", ");
					//Serial.print("{");
					for (int lightIdx = 0; lightIdx < 4; lightIdx++)
					{
						rdV = eeepromReadInt(eepromIdx);	
						if (lightIdx != 0) Serial.print(", ");
						Serial.print(rdV);
					}
					//Serial.print("}");
				}
				Serial.print("}; RGB[16][NR_COLOR_TESTS][4]={");
	
				eepromIdx = 256;

				for (ledIdx = 0; ledIdx < 16; ledIdx++)
				{
					if (ledIdx != 0) Serial.print("-"); //Serial.print(", ");
					
					//Serial.print("{");

					for (int testIdx = 0; testIdx < NR_COLOR_TESTS; testIdx++)
					{
						if (testIdx != 0) Serial.print(", ");
						//Serial.print("{");

						for (int lightIdx = 0; lightIdx < 4; lightIdx++)
						{
							rdV = eeepromReadInt(eepromIdx);	
							if (lightIdx != 0) Serial.print(", ");
							Serial.print(rdV);
						}
						//Serial.print("}");
					}
					//Serial.print("}");
				}
				//Serial.print("Temp:");
				//Serial.print(readTemperature());
				Serial.println("EOD");//Serial.println("};");
			}
		   else
			//*/

			if (!strcmp(cmdBuffer, "MAGIC_PING"))
			{
				#ifdef DO_TCHRG_CHECK
					enableTurboCharge(true);
					// we die of too little akku if 
					debugColor(4095, 4095, 4095, 5, 6 + 5, 120); // 3Leds with no tchrg, 8LEDS at tchrg //30ms um ohne tchrg abzuschalten
					debugColor(0, 0, 0, 0, 16, 1);
					delay(50);
				#endif

				//SaveEnergyHack();

				Serial.println("MAGIC_PONG");
				executed = true;
			}
			else if (!strcmp(cmdBuffer, "MAGIC_AUTO"))
			{
				digitalWrite(IRPWM_PIN, LOW);
				SetWhiteLED(0);
				SetUVLED(0);
				setAll(0, 0, 0);
				updateLedsWithBlank();

				if (!(USBSTA & 0x1))
				{
					ReportError("VUSB not 5V!");
				}

#ifdef DO_RTC_CHECK
				if (notInitialBoot != EEPROM_SET && powerupReason != POWERUP_USB)
				{
					ReportError("USB V not working!");
				}
				if (powerupReason != POWERUP_ALERT)
				{
					ReportError("RTC not recognized");
				}
#endif

				pinMode(int1Pin, INPUT);
				digitalWrite(int1Pin, LOW);
				pinMode(int2Pin, INPUT);
				digitalWrite(int2Pin, LOW);
				
			
				for (int puN = 0; puN < 3; puN++)
				{
					MMA8452Standby();
					writeRegister(0x2D, 0); // disable all ints
					writeRegister(0x2C, 0x00); // Active low, push-pull
					MMA8452Active();
					if (!digitalRead(int1Pin))
					{
						ReportError("Accel Int1 not high");
					}
					if (!digitalRead(int2Pin))
					{
						ReportError("Accel Int2 not high");
					}

					MMA8452Standby();
					writeRegister(0x2D, 0); // disable all ints
					writeRegister(0x2C, 0x02);	// Active high, push-pull
					MMA8452Active();
					if (digitalRead(int1Pin))
					{
						ReportError("Accel Int1 not low");
					}
					if (digitalRead(int2Pin))
					{
						ReportError("Accel Int2 not low");
					}
				}
				
				MMA8452Standby();
				enableLeds(false); 
				delay(300);

				bool wasLow = false;
				bool wasHigh = false;
				for (int i = 0; i < 20000 && !(wasLow && wasHigh); i++)
				{
					if (digitalRead(CHRGSTAT_PIN))
						wasHigh = true;
					else
						wasLow = true;
				}
				if (!(wasHigh && wasLow))
				{
					if (wasHigh)
						ReportError("ChargeSt always HIGH");
					else
						ReportError("ChargeSt always LOW");
				}

				enableLeds(true); 

				//ReportError("Who's in charge here?");

				// moved here after power hack				
				initMMA8452(2, 0);

				if (!VerifyTCS3771Interrupt())
				{
					ReportError("Light Inter. does't change.");
				}
				

				// TODO: Check accel interrupt pins

				byte id, type;
				ReadEEEPROMID(id, type);

				/*if (id == 0xBF	&& type == 0x49) // 1Mbit EEPROM
				{
					debugColor(0, 10, 10, 5, 10, 100);
				}					
				else*/ 
				if (id != 0xBF || type != 0x8E) //8Mbit EEPROM
				{
					ReportErrorWithResult("EEPROM not found!", (((int)id) << 8) + type, 0xBF8E, HEX);
				}

				int adVal = analogRead(2);
				if (adVal < 1010) {
					ReportErrorWithResult("V LTEST too low", 1010, adVal, HEX);
				}
				SetUVLED(4095);
				delay(1);
				adVal = analogRead(2);
				SetUVLED(0);
				if (adVal >= 500) {
					ReportErrorWithResult("V LTEST too high", 500, adVal, DEC);
				}


			


// ACCELEROMETER
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11
// !!!!!!!!!!!!!!!! TODO TODO TODO
// TODO!!! TEST BOTH INT PINS!!!!
				UpdateButtons();
				int c = readRegister(0x0D);	// Read WHO_AM_I register
				if (c == 0x2A)		// WHO_AM_I should always be 0x2A
				{
					initMMA8452(2, 0);	// init the accelerometer if communication is good
				}
				else
				{
					ReportError("ACCEL error");
				}
#ifdef DO_ACCEL_CHECK
				accelCount[XAXIS] = 2000;
				accelCount[YAXIS] = 2000;
				accelCount[ZAXIS] = 2000;
				// assert: X, Y ~= 0, Z ~= -1g
				int accelTests = 100;
				float avgX=0, avgY, avgZ;
				for (int i = 0; i < accelTests; i++)
				{
	/*
					if (i%2)
						writeRegister(0x2E, 0x01);
					else
						writeRegister(0x2E, 0x00);
*/
					delay(10);
					CheckMMA8452();

					if (abs(accelCount[XAXIS]) > ACCEL_ERROR_MARGIN) ReportErrorWithResult("ACCELX off", 0, accelCount[XAXIS], DEC);
					if (abs(accelCount[YAXIS]) > ACCEL_ERROR_MARGIN) ReportErrorWithResult("ACCELY off", 0, accelCount[YAXIS], DEC);
					if (abs(accelCount[ZAXIS] - 1024) > ACCEL_ERROR_MARGIN) ReportErrorWithResult("ACCELZ off", 1024, accelCount[ZAXIS], DEC);
					
					avgX += accelCount[XAXIS];
					avgY += accelCount[YAXIS];
					avgZ += accelCount[ZAXIS];	
				}

				MeasuredAccelValues[0] = round(avgX/accelTests);
				MeasuredAccelValues[1] = round(avgY/accelTests);
				MeasuredAccelValues[2] = round(avgZ/accelTests);

				
#endif

// RGB LEDs
				// in the background assert that each button is clicked at least 200ms and not clicked for 1 sec 
				// assert: light sensor is connected
				// assert: R,G,B ~= 0 helligkeit
				// store avg over long time for white balance (will save it in eeprom later)
				// test each led, 16xRGB, IR, UV and white seperately
				// store the RGB sense vslues for each single led


				// ambient light conditions
				int AmbientRGBValues[4];

				pinMode(IRPWM_PIN, OUTPUT);
				digitalWrite(IRPWM_PIN, LOW);

				setAll(0, 0, 0);
					updateLedsWithBlank();

				float AvgProx = 0;

				for (int testIdx = 0; testIdx < NR_EXTRA_TESTS; testIdx++)
				{
					UpdateButtons();
					if (testIdx == 0)
					{
					}
					else if (testIdx == 1)
					{
						SetWhiteLED(512);
					}
					else if (testIdx == 2)
					{
						SetWhiteLED(4095);
					}
					else if (testIdx == 3)
					{
						SetUVLED(1024);
					}
					else if (testIdx == 4)
					{
						SetUVLED(4095);
					}
					else if (testIdx == 5)
					{	
						digitalWrite(IRPWM_PIN, HIGH);
					}
					delay(1);
					DoLightSensing();

					digitalWrite(IRPWM_PIN, LOW);
					SetWhiteLED(0);
					SetUVLED(0);
					setAll(0, 0, 0);
					updateLedsWithBlank();
							
	#ifdef DO_PROX_CHECK 
					if (ps < 100) {
						ReportErrorWithResult("Prox too high!", 300, ps, DEC);
					}
					if (ps > 900)	{
						ReportErrorWithResult("Prox too close!", 900, ps, DEC);
					}
	#endif

					AvgProx += ps;
					for (int lightIdx = 0; lightIdx < 4; lightIdx++)
					{
						if (testIdx == 0) AmbientRGBValues[lightIdx] = lightSensorValues[lightIdx];
						MeasuredExtraTestValues[testIdx][lightIdx] =  lightSensorValues[lightIdx];

						float real = lightSensorValues[lightIdx] - AmbientRGBValues[lightIdx];
						float delta = abs(real - ExpectedExtraRGBValues[testIdx][lightIdx]);
						if (testIdx != 0 && delta > 14 && delta > ExpectedExtraRGBValues[testIdx][lightIdx] * ExtraMargins[testIdx] * COLOR_TEST_DURATION_SCALE)
						{
#ifdef DO_LED_CHECK
							String errorMsg = "LED test";
							errorMsg += testIdx;
				
							errorMsg += " {";
							errorMsg += cs;
							errorMsg += ", ";
							errorMsg += rs;
							errorMsg += ", ";
							errorMsg += gs;
							errorMsg += ", ";
							errorMsg += bs;
							errorMsg += "} p ";
							errorMsg += ps;
						
							ReportErrorWithResult(errorMsg, ExpectedExtraRGBValues[testIdx][lightIdx], real, DEC);
#endif
						}		
					}
				}

#ifdef WRITE_CALIBRATION_VALUES 
				EraseSector(0);
				delay(100);
				eepromIdx = 256;
#endif

				for (ledIdx = 0; ledIdx < 16; ledIdx++)
				{
					UpdateButtons();
					for (int testIdx = 0; testIdx < NR_COLOR_TESTS; testIdx++)
					{
						UpdateButtons();
						setAll(0, 0, 0);
						setRGB(ledIdx, ColorTestValues[testIdx][0], ColorTestValues[testIdx][1], ColorTestValues[testIdx][2]);
						updateLedsWithBlank();	
						DoLightSensing();

#ifdef WRITE_CALIBRATION_VALUES 
						eeepromWriteColor(eepromIdx, 
								lightSensorValues[0], lightSensorValues[1], 
								lightSensorValues[2], lightSensorValues[3]);
#endif

						AvgProx += ps;

						//float posScale = 1-(LIGHTFALLOFF_SIDE*abs(ledIdx - 7.5)/7.5);

						for (int lightIdx = 0; lightIdx < 4; lightIdx++)
						{
							//MeasuredTestValues[testIdx][ledIdx][lightIdx] =  lightSensorValues[lightIdx];

							//float expected = posScale * ExpectedRGBValues[testIdx][lightIdx] * COLOR_TEST_DURATION_SCALE;

							float expected = (brightMod[ledIdx]/10000.0) * ExpectedRGBValues[testIdx][lightIdx] * COLOR_TEST_DURATION_SCALE;

							float real = (lightSensorValues[lightIdx] - AmbientRGBValues[lightIdx]);
							float delta = abs(real - expected);
							if (delta > 12 && delta > LIGHTSENSOR_MARGIN * expected)
							{
#ifdef DO_LED_CHECK
								String errorMsg = "RGB LED nr";
								errorMsg += ledIdx;
							 	errorMsg += "c";	
								errorMsg += lightIdx;
								errorMsg += "t";	
								errorMsg += testIdx;
								errorMsg += " brig err ( ";
						
								errorMsg += " {";
								errorMsg += cs;
								errorMsg += ", ";
								errorMsg += rs;
								errorMsg += ", ";
								errorMsg += gs;
								errorMsg += ", ";
								errorMsg += bs;
								errorMsg += "} p ";
								errorMsg += ps;

							
								ReportErrorWithResult(errorMsg, expected, real, DEC);
#endif
							}
						}
					}
				}
				setAll(0, 0, 0);
				updateLedsWithBlank();	

				int AvgProxInt = round(AvgProx / (NR_COLOR_TESTS*16 + NR_EXTRA_TESTS));


				


#ifdef WRITE_CALIBRATION_VALUES 
				eepromIdx = 0;

				for (int axis = 0; axis < 3; axis++)
				{
					eeepromWriteInt(eepromIdx, MeasuredAccelValues[axis]);
				}
			
				eepromIdx = 16;

				eeepromWriteInt(eepromIdx, AvgProxInt); 

				eepromIdx = 128;

				for (int testIdx = 0; testIdx < NR_EXTRA_TESTS; testIdx++)
				{
					eeepromWriteColor(eepromIdx, 
						MeasuredExtraTestValues[testIdx][0], MeasuredExtraTestValues[testIdx][1], 
						MeasuredExtraTestValues[testIdx][2],MeasuredExtraTestValues[testIdx][3]); 
				}
				/*

				eepromIdx = 256;

				for (ledIdx = 0; ledIdx < 16; ledIdx++)
				{
					for (int testIdx = 0; testIdx < NR_COLOR_TESTS; testIdx++)
					{
						eeepromWriteColor(eepromIdx, 
							MeasuredTestValues[testIdx][ledIdx][0], MeasuredTestValues[testIdx][ledIdx][1], 
							MeasuredTestValues[testIdx][ledIdx][2], MeasuredTestValues[testIdx][ledIdx][3]);
					}
				}
				*/

	// test read back

				eepromIdx = 0;

				for (int axis = 0; axis < 3; axis++)
				{
					if (!eeepromTestInt(eepromIdx, MeasuredAccelValues[axis]))
					{
						int oi = axis*2;
						ReportErrorWithResult("ExEEPROM readback fail at Accel", MeasuredAccelValues[axis], eeepromReadInt(oi), DEC);
					}	
				}

				eepromIdx = 16;

				if (!eeepromTestInt(eepromIdx, AvgProxInt))
				{
					ReportError("ExEEPROM readback fail at Prox");
				}
				eeepromWriteInt(eepromIdx, AvgProxInt); 

				eepromIdx = 128;

				for (int testIdx = 0; testIdx < NR_EXTRA_TESTS; testIdx++)
				{
					if (!eeepromTestColor(eepromIdx,  MeasuredExtraTestValues[testIdx]))
					{
						ReportError("ExEEPROM readback fail at Light");
					}
				}
	/*
				eepromIdx = 256;

				for (ledIdx = 0; ledIdx < 16; ledIdx++)
				{
					for (int testIdx = 0; testIdx < NR_COLOR_TESTS; testIdx++)
					{
						if (!eeepromTestColor(eepromIdx,  MeasuredTestValues[testIdx][ledIdx]))
						{
							ReportError("External EEPROM readback fail at RGB Sensor Data");
						}
					}
				}
	*/
#endif

				UpdateButtons();
				if (centerBtnClickedTime != 0)
				{
					ReportError("Button 1 was pressed");
				}
				if (powerBtnClickedTime != 0)
				{
					ReportError("Button 2 was pressed");
				}
				if (centerBtnPressed != 0)
				{
					ReportError("Button 1 IS pressed");
				}
				if (powerBtnPressed != 0)
				{
					ReportError("Button 2 IS pressed");
				}


				updateEEPROMAutoState(true);

				Serial.println("MAGIC_OK:");
				executed = true;
			}
			else if (!strcmp(cmdBuffer, "MAGIC_MANUAL"))
			{
				UpdateButtons();
				if (centerBtnClickedTime != 0)
				{
					ReportError("Button 1 was pressed before it should.");
				}
				if (powerBtnClickedTime != 0)
				{
					ReportError("Button 2 was pressed before it should.");
				}

				int pressed = 0;
				for (int bb = 0; bb < 6000 && pressed != 3; bb++)
				{
					UpdateButtons();
					if (centerBtnClickedTime != 0)
					{
						pressed |= 1;
					}
					if (powerBtnClickedTime != 0)
					{
						pressed |= 2;
					}
					digitalWrite(ledPin, (bb / 100) % 2 ?  LOW : HIGH);	
					delay(1);
				}
				digitalWrite(ledPin, HIGH);

				if (pressed == 0)
				{
					ReportError("No Button was pressed.");
				}
				else if (pressed == 1)
				{
					ReportError("Button 2 was NOT pressed!");
				}
				else if (pressed == 2)
				{
					ReportError("Button 1 was NOT pressed");
				}

				updateEEPROMManualState(true);
				ReportOK("");
				executed = true;
			}
			
		
			if (!executed)
			{
				Serial.println("MAGIC_UNKNOWN_CMD");
			}
		}	
		continue;	

	// calculate white balance for all R, G and B leds in total
	// store these white balance values

	// assert: eeprom is connected
	// assert: write white balance data 

	
		}

	/*
	while (1) 
	{
		debugColor(10, 00, 0, 0, notInitialBoot, 50);

	   updateLedsWithBlank();


		if (!MagicShifter_Poll()) {
			//EEPROM.write(1, colIdx);
			break;
		}

		randomNr += 23;

		if(Serial.available())
		{
	  		delay(150);
			bufferIndex = 0;
	  		while( Serial.available() && bufferIndex < 99) {
				byte c = Serial.read();
				if (c == '\n' || c == '\r') break;
	     		commandbuffer[bufferIndex++] = c;
	  		}
			if (bufferIndex == 0) continue;

	  		commandbuffer[bufferIndex++]='\0';
			bool executed = false;

			if (!strcmp(commandbuffer, "MAGIC_PING"))
			{
				Serial.println("MAGIC_PONG");
				executed = true;
			}
			else if (!strcmp(commandbuffer, "MAGIC_START_TEST"))
			{
				delay(1000);
				if ((randomNr % 100) < 50)
				{
					Serial.println("MAGIC_OK");
				}
				else
				{
					delay(1000);
					if (randomNr % 4 == 0) Serial.println("MAGIC_ERROR:Accelerometer is not responing.");
					else if (randomNr % 4 == 1) Serial.println("MAGIC_ERROR: LED Red/7 is not lighting up.");
					else if (randomNr % 4 == 2) Serial.println("MAGIC_ERROR: Core Meltdown!!!11");
					else Serial.println("MAGIC_ERROR: Light sensor is not responding.");
				}
				executed = true;
			}
		
			if (!executed)
			{
				Serial.println("MAGIC_UNKNOWN_CMD");
			}
		}		
	}
		*/
}
