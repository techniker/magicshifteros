#include "firmware.h"



void ExampleMode()
{
	int frame = 0;

	//v = 128;

	while (1) {
		frame++;
		CheckMMA8452();

		setAll(frame % 300 == 0 ? v : 0, frame % 300 == 100 ? v : 0, frame % 300 == 200 ? v : 0);

		setRGB(15, centerBtnPressed ? 100 : 0, powerBtnPressed ? 100 : 0, 0);
		updateLedsWithBlank();

		if (!MagicShifter_Poll())
			break;

		if (powerBtnClickedTime != 0) {
			setAll(90, 100, 12);
			updateLedsWithBlank();
			delay(10);
			powerBtnClickedTime = 0;
		}
		if (centerBtnClickedTime != 0) {
			setAll(23, 12, 100);
			updateLedsWithBlank();
			delay(10);
			centerBtnClickedTime = 0;
		}
	}
}



void IRPWMMode()
{
	int frame = 0;
	v = 128;

	int mt = 2;
	// init PWM	
	pinMode(IRPWM_PIN, OUTPUT);
	digitalWrite(IRPWM_PIN, LOW);

	#define IR_PWM_TIME 216

	
	//cli();
	// set alternate pin functions for RGB pins (clear on compare match [10])
	//TCCR3A /*TCCR1A*/ = (1 << COM3A1);
	TCCR3A /*TCCR1A*/ = (1 << COM3A1); //toggle
	// setup timer1 for Fast PWM mode 14 (TOP=ICR1)
	TCCR3A /*TCCR1A*/ |= (1 << WGM31);	// WGM bits are split across TCCR1A and TCCR1B
	TCCR3B /*TCCR1B*/ = (1 << WGM32) | (1 << WGM33);
	// select clock source and set divider to 1 (244 Hz 16-bit PWM)
	TCCR3B /*TCCR1B*/ |= (1 << CS30);
	// setup ICR1 for counter TOP value
	ICR3 /*ICR1*/ = IR_PWM_TIME;
	OCR3A /*OCR1A */ = IR_PWM_TIME / 2;
	//sli();

	while (1) {
		frame++;

		//setAll(frame % 300 == 0 ? v : 0, frame % 300 == 100 ? v : 0, frame % 300 == 200 ? v : 0);

		//setRGB(15, centerBtnPressed ? 100 : 0, powerBtnPressed ? 100 : 0, 0);
		//updateLedsWithBlank();

		if (!MagicShifter_Poll())
			break;

		if (powerBtnClickedTime != 0) 
		{
			digitalWrite(IRPWM_PIN, LOW);
			delay(100);
			setAll(90, 100, 12);
			updateLedsWithBlank();
			delay(10);
			powerBtnClickedTime = 0;
		}

		if (centerBtnClickedTime != 0) 
		{
			if (1 == 0 && centerBtnClickedTime > LONG_CLICK_TIME)
			{
				setAll(100, 100, 100);
				updateLedsWithBlank();
				delay(10);
				setAll(0, 0, 0);
				updateLedsWithBlank();
				delay(10);

				for (int ssd= 0; ssd < 10; ssd++)
				{				
				for (int i = 0; i < 100; i++)
				{
					digitalWrite(IRPWM_PIN, HIGH);
					//delay(10);
					delayMicroseconds(10);
					digitalWrite(IRPWM_PIN, LOW);
					delayMicroseconds(10);
					//delay(10);
				}
				delay(2);
				}

				setAll(10, 10, 10);
				updateLedsWithBlank();
				delay(10);
				setAll(0, 0, 0);
				updateLedsWithBlank();
				delay(10);
			}
			else
			{
				setAll(0, 100, 100);
				updateLedsWithBlank();
				delay(10);
				setAll(0, 0, 0);
				updateLedsWithBlank();
				delay(10);


				mt = 2; // => 41.67kHz
				// 3 ~= 31kHz
				// 4 /> 25kHz
				mt++;
				if (mt > 30)
					mt = 2;
				
				Serial.println(TCNT3);
	
				//for (int mt = 10; mt < 15; mt++)
				{
					//ICR3 = mt;
					for (int i = 0; i < 300; i++)
					{
						TCCR3A /*TCCR1A*/ = (1 << COM3A1); //toggle
	// setup timer1 for Fast PWM mode 14 (TOP=ICR1)
	TCCR3A /*TCCR1A*/ |= (1 << WGM31);	// WGM bits are split across TCCR1A and TCCR1B
	TCCR3B /*TCCR1B*/ = (1 << WGM32) | (1 << WGM33);
	// select clock source and set divider to 1 (244 Hz 16-bit PWM)
	TCCR3B /*TCCR1B*/ |= (1 << CS30);

						delay(1);
						TCCR3A = 0;
						delay(2);	
					}
				}

				setAll(0, 10, 10);
				updateLedsWithBlank();
				delay(10);
				setAll(0, 0, 0);
				updateLedsWithBlank();
				delay(10);
			}

			centerBtnClickedTime = 0;
		}
	}

	// cleanup PWM stuff
	TCCR3A /*TCCR1A*/ = 0;
	pinMode(IRPWM_PIN, INPUT);
}


#define RTCMENU_SECONDS 1
#define RTCMENU_MINUTES 2
#define RTCMENU_HOURS   3

#define DISPLAYTYPE_TIME 0
#define DISPLAYTYPE_DATE 1

void RenderBinaryNr(uint8_t number, uint8_t channel, uint8_t baseIdx, uint8_t nrOfBits, uint8_t onColor, uint8_t offColor)
{
	uint8_t bitMask = 0x01;

	for (uint8_t idx = 0; idx < nrOfBits; idx++, bitMask <<= 1)
	{
		setChannel(baseIdx - idx, channel, number & bitMask ? onColor : offColor);
	}
}

void RTCMode()
{
	unsigned int frame = 0;
	int menu = 0; // if != 0 we are in time set mode
	uint8_t displayType = 0; // 0 for time 1 for date
	uint8_t s, m, h;
	uint8_t day, month, year;
	
	AccelConfig shakeConfig;
	shakeConfig.accelInterruptNr = 0;
	shakeConfig.tapInterruptNr = 0;
	shakeConfig.transientInterruptNr = 2;
	shakeConfig.accelScale = 2;
	shakeConfig.accelDataRate = 2;

	AccelSetup(&shakeConfig);

	char textTime[9] = "HH:MM:SS";
	char textDate[9] =  "YY-MM-DD";
	

	MSBitmap *font = &font12px;
 
	AccelShakeNrOfFrames = COLUMNMULTIPLY*8*font->header.frameWidth;
	
	while (1) {
		frame++;

		if (!MagicShifter_Poll())
  			break;

		AccelPoll();
		UpdateAccelShakeCurrentColumn();

		if (menu == 0)
		{
			if (frame % 100 == 0)
			{
				initiateRTCReadout = 1;
			}
			else
			{
				s = rtcBuffer[0];
				m = rtcBuffer[1];
				h = rtcBuffer[2];

				day = rtcBuffer[4];
				month = rtcBuffer[5];
				year = rtcBuffer[6];

				textTime[0] = '0' + ((h >> 4) & 0x7);
				textTime[1] = '0' + (h & 0xF);

				textTime[3] = '0' + ((m >> 4) & 0x7);
				textTime[4] = '0' + (m & 0xF);		
		
				textTime[6] = '0' + ((s >> 4) & 0x7);
				textTime[7] = '0' + (s & 0xF);

				textDate[0] = '0' + ((year >> 4) & 0x7);
				textDate[1] = '0' + (year & 0xF);

				textDate[3] = '0' + ((month >> 4) & 0x1);
				textDate[4] = '0' + (month & 0xF);	

				textDate[6] = '0' + ((day >> 4) & 0x3);
				textDate[7] = '0' + (day & 0xF);

				s = (10 * ((s >> 4) & 0x7)) + (s & 0xF);
				m = (10 * ((m >> 4) & 0x7)) + (m & 0xF);
				h = (10 * ((h >> 4) & 0x3)) + (h & 0xF);

				day = (10 * ((day >> 4) & 0x3)) + (day & 0xF);
				month = (10 * ((month >> 4) & 0x3)) + (month & 0xF);
				year = (10 * ((year >> 4) & 0xF)) + (year & 0xF);
			}
		}
		if (AccelShakeFrameNr != 0xFF)
		{
			if (AccelShakeFrameNr < 0xFE)
			{	
				uint8_t realFrame = AccelShakeFrameNr / COLUMNMULTIPLY;
				
				if (realFrame < 2*font->header.frameWidth)
				{
					font->color.rgb.r = 0;
					font->color.rgb.g = 0;
					font->color.rgb.b = 255;
				}		
				else if (realFrame < 3*font->header.frameWidth)
				{
					font->color.rgb.r = 255;
					font->color.rgb.g = 255;
					font->color.rgb.b = 255;
				}
				else if (realFrame < 5*font->header.frameWidth)
				{
					font->color.rgb.r = 0;
					font->color.rgb.g = 255;
					font->color.rgb.b = 0;
				}
				else if (realFrame < 6*font->header.frameWidth)
				{
					font->color.rgb.r = 255;
					font->color.rgb.g = 255;
					font->color.rgb.b = 255;
				}
				else
				{
					font->color.rgb.r = 255;
					font->color.rgb.g = 0;
					font->color.rgb.b = 0;	
				}
				
				PlotText(font, displayType == DISPLAYTYPE_DATE ? textDate : textTime, realFrame, 0);
				updateLedsWithBrightness();
			}
			else
			{	
				setAll(0,0,0);
				updateLedsWithBrightness();
			}
			if (centerBtnClickedTime != 0) 
			{
				displayType = (displayType+1)%2;
				centerBtnClickedTime = 0;
			}
		}
		else
		{
			setAll(0,0,0);

			uint8_t blink = (frame % 300) < 150;

			if (displayType == DISPLAYTYPE_DATE)
			{
				RenderBinaryNr(day, CHANNEL_RED, 15, 5, 255, (blink && (menu == RTCMENU_SECONDS)) ? 32 : 0);
				RenderBinaryNr(month, CHANNEL_GREEN, 15-5, 4, 255, (blink && (menu == RTCMENU_MINUTES)) ? 32 : 0);
				RenderBinaryNr(year, CHANNEL_BLUE, 15-(5+4), 7, 255, (blink && (menu == RTCMENU_HOURS)) ? 32 : 0);
			}
			else
			{
				if (menu != RTCMENU_MINUTES)
					RenderBinaryNr(s, CHANNEL_RED, 15, 6, 255, (blink && (menu == RTCMENU_SECONDS)) ? 32 : 0);
				if (menu != RTCMENU_SECONDS) 
					RenderBinaryNr(m, CHANNEL_GREEN, 15, 6, 255, (blink && (menu == RTCMENU_MINUTES)) ? 32 : 0);
				RenderBinaryNr(h, CHANNEL_BLUE, 15-7, 6, 255, (blink && (menu == RTCMENU_HOURS)) ? 32 : 0);
				if (menu == 0)
				{
					uint8_t sb = (s%2) ? 255:0;
					setRGB(15-6, sb,sb,sb);
				}
				else {
					setRGB(15-6, 255, 255, 255);
				}
				
			}
			updateLedsWithBrightness();

			if (powerBtnClickedTime != 0)
			{
				if (displayType == DISPLAYTYPE_TIME)
				{
					if (menu == RTCMENU_MINUTES)
					{
						m = (m + 59) % 60;
					}
					else if (menu == RTCMENU_SECONDS)
					{
						s = (s + 59) % 60;
					}
					else if (menu == RTCMENU_HOURS)
					{
						h = (h + 23) % 24;
					}
				}
				else
				{
					if (menu == RTCMENU_MINUTES)
					{
						month--;
						if (month == 0) month = 12;
					}
					else if (menu == RTCMENU_SECONDS)
					{
						day--;
						if (day == 0) day = 31;
					}
					else if (menu == RTCMENU_HOURS)
					{
						year = (year + 99) % 100;
					}
				}
				powerBtnClickedTime = 0;
			}

			if (centerBtnClickedTime != 0) 
			{
				if (centerBtnClickedTime > LONG_CLICK_TIME) 
				{
					// set the time to entered values
					if (menu != 0)
					{
						menu--;
						if (menu == 0 || menu == 4)	
						{
							MCP7941X_SetTime(year, month, day, h, m, s);
							setAll(255, 255, 255);
							updateLedsWithBlank();	
							delay(200);
							setAll(0, 0, 0);
							updateLedsWithBlank();				
						}
					}
					else
					{
						setAll(255, 255, 255);
						updateLedsWithBlank();	
						delay(200);
						setAll(0, 0, 0);
						updateLedsWithBlank();				
						menu = 3;
					}
				}
				else 
				{
					if (menu != 0)
					{
						if (displayType == DISPLAYTYPE_TIME)
						{
							if (menu == RTCMENU_MINUTES)
							{
								m = (m + 1) % 60;
							}
							else if (menu == RTCMENU_SECONDS)
							{
								s = (s + 1) % 60;
							}
							else if (menu == RTCMENU_HOURS)
							{
								h = (h + 1) % 24;
							}
						}
						else
						{
							if (menu == RTCMENU_MINUTES)
							{
								month++;
								if (month > 12) month = 1;
							}
							else if (menu == RTCMENU_SECONDS)
							{
								day++;
								if (day > 31) day = 1;
							}
							else if (menu == RTCMENU_HOURS)
							{
								year = (year + 1) % 100;
							}
						}
					}
					else
					{
						displayType = (displayType+1)%2;
					}
				}
				centerBtnClickedTime = 0;
			}
		}
	}
}


void startToEndChannel(uint8_t start, uint8_t end, int d, int channel, int color)
{
	int i;

	i = start;
	do {
		setAllChannel(channel, 0);
		setChannel(i, channel, color);
		updateLedsWithBlank();
		if (d)
			delay(d);
		if (i < end)
			i++;
		else
			i--;
	} while (i != end);
	setAllChannel(channel, 0);
	setChannel(i, channel, color);
	updateLedsWithBlank();
}

void startToEndZigZag(uint8_t start, uint8_t end, int d, uint8_t r, uint8_t g, uint8_t b)
{
	int i;
	uint8_t lastEnd = end;
	uint8_t currentStart = start;
	uint8_t currentEnd = start;

	if (end < start)
		end = start-1;
	else
		end = start+1;

	for (;;)
	{
		i = currentStart;
		do {
			setAll(0,0,0);
			setRGB(i, r, g, b);
			updateLedsWithBlank();
			if (d)
				delay(d);
			if (i < currentEnd)
				i++;
			else
				i--;
		} while (i != currentEnd);

		if (i == end)
		{
			if (end == lastEnd)
			{
				if (start < end)
					start++;
				else
					start--;
				if (start == end)
					break;
			}		
			currentStart = end;
			currentEnd = start;
		}
		else
		{
			if (end != lastEnd)
			{
				if (end < lastEnd)
					end++;
				else
					end--;
			}
			currentStart = start;
			currentEnd = end;		
		}
	}

	// last
	setAll(0,0,0);
	setRGB(i, r, g, b);
	updateLedsWithBlank();
}

void RainbowMove()
{
	uint8_t lookup[6][3] = { {0, 1, 2}, {1, 0, 2}, {2, 0, 1}, {0, 2, 1}, {1, 2, 0}, {2, 1, 0} };
	uint8_t lookupindex = 0;
	int d = 10;
	uint8_t moveMode = 0;
	uint8_t dir = 0;

#ifndef EYE_FRIENDLY
	uint8_t vOld = v;
	v = 255; 
#endif

	AccelConfig toyConfig;
	toyConfig.accelInterruptNr = 0;
	toyConfig.tapInterruptNr = 1;
	toyConfig.transientInterruptNr = 0;
	toyConfig.accelScale = 2;
	toyConfig.accelDataRate = 0;
	AccelSetup(&toyConfig);

		
	uint8_t start, end;
	int xx = 0;

	while (1) {
		if (dir)
		{
			start = 0;
			end = 15;
		}
		else
		{
			start = 15;
			end = 0;
		}

		if (moveMode == 0)
		{
			for (int index = 0; index < 3; index++) {
				startToEndChannel(start, end, d, lookup[lookupindex][index], 255);
			}
		}
		else if (moveMode == 1)
		{
			startToEndZigZag(start, end, 1, 255, 255, 255);
		}

		while (!centerBtnClickedTime) {
			if (!MagicShifter_Poll())
				return;
			
			AccelPoll();
			//if ((lastTapStatus & (0x88 | 0x44)) == (0x88 | 0x40))
	
			if (powerBtnClickedTime != 0) 
			{
				powerBtnClickedTime = 0;

	
				setAll(255, 255, 255);
				updateLedsWithBlank();
				delay(2);
				setAll(0, 0, 0);
				setRGB(end, 255, 255, 255);
				updateLedsWithBlank();
				moveMode = (moveMode + 1) % 2;
				lastTapStatus = 0;
				vChanged = 1;
			}

			if (vChanged) {
				if (moveMode != 2)
				{
					setRGB(end, 255, 255, 255);
					updateLedsWithBlank();
				}
				vChanged = 0;
			}
/*
			if (moveMode == 2)
			{
				setAll(0, 0, 0);
				setRGB(((xx + 0)%6)*3, 255, 0, 0);

				setRGB(((xx + 1)%6)*3, 255, 255, 0);
				setRGB(((xx + 2)%6)*3, 0, 255, 0);

				setRGB(((xx + 3)%6)*3, 0, 255, 255);
				setRGB(((xx + 4)%6)*3, 0, 0, 255);

				setRGB(((xx + 5)%6)*3, 255, 0, 255);
	
				updateLedsWithBlank();				
	
				delay(300);
				xx++;
			}
*/
		}

		if (centerBtnClickedTime != 0) {
			if (centerBtnClickedTime < LONG_CLICK_TIME) {
				if (d < 50)
					d++;
			} else {
				if (d > 0)
					d--;
			}
			centerBtnClickedTime = 0;
		}

		lookupindex = (lookupindex + 1) % 6;
		dir = (dir + 1) % 2;
	}

#ifndef EYE_FRIENDLY
	v = vOld; 
#endif
}


void FutureMode()
{
	int s = 0;
	int s2 = 0;

	vChanged = true;	// dirty hack
#ifndef EYE_FRIENDLY
	uint8_t vOld = v;
	v = 255; 
#endif
	while (1) {
		if (!MagicShifter_Poll()) {
			SetWhiteLED(0);
			SetUVLED(0);
			setAll(0, 0, 0);
			updateLedsWithBlank();
			break;
		}
		if (centerBtnClickedTime != 0) {

			s = (s + 1) % 4;
			centerBtnClickedTime = 0;
			vChanged = true;	// dirty hack


			s2 = (s2 + 1) % 4;
			if (s2 % 2)
				digitalWrite(RXLED, LOW);
			else
				digitalWrite(RXLED, HIGH);

			if ((s2 >> 1) % 2)
				TXLED1;
			else
				TXLED0;

			blink(30, 30, 2);
		}

		if (vChanged) {
			vChanged = 0;
			
			setAll(0, 0, 0);
			updateLedsWithBlank();
			SetWhiteLED(0);
			SetUVLED(0);
			delay(1);

			if (s == 0) {
				SetWhiteLED(v);
			} else if (s == 1) {
				SetUVLED(v);
			} else if (s == 2) {
				setAll(255, 255, 255);
				updateLedsWithBlank();
			}
		}
	}
#ifndef EYE_FRIENDLY
	v = vOld; 
#endif
}

void RestlichtMode()
{
	int frame = 0;
	int s = 0;
	int s2 = 0;

	vChanged = true;	// dirty hack

	while (1) {
		frame++;
		CheckMMA8452();

		if (!MagicShifter_Poll()) {
			setAll(0, 0, 0);
			updateLedsWithBlank();
			digitalWrite(LEDWHITE_PIN, LOW);
			digitalWrite(LEDUV_PIN, LOW);
			break;
		}

		if (powerBtnClickedTime != 0) {
			powerBtnClickedTime = 0;

		}
		if (centerBtnClickedTime != 0) {

			s = (s + 1) % 4;
			centerBtnClickedTime = 0;
			vChanged = true;	// dirty hack


			s2 = (s2 + 1) % 4;
			if (s2 % 2)
				digitalWrite(RXLED, LOW);
			else
				digitalWrite(RXLED, HIGH);

			if ((s2 >> 1) % 2)
				TXLED1;
			else
				TXLED0;

			blink(30, 30, 2);
		}

		if (vChanged) {
			vChanged = 0;

			digitalWrite(LEDWHITE_PIN, LOW);
			digitalWrite(LEDUV_PIN, LOW);
			setAll(0, 0, 0);
			updateLedsWithBlank();
			delay(1);

			if (s == 0) {

				digitalWrite(LEDWHITE_PIN, HIGH);
				delayMicroseconds(30);
				for (int i = 0; i < (31 - ((v + 1) >> 7)); i++) {
					digitalWrite(LEDWHITE_PIN, LOW);
					digitalWrite(LEDWHITE_PIN, HIGH);
				}
			} else if (s == 1) {
				digitalWrite(LEDUV_PIN, HIGH);
				for (int i = 0; i < (31 - ((v + 1) >> 7)); i++) {
					digitalWrite(LEDUV_PIN, LOW);
					digitalWrite(LEDUV_PIN, HIGH);
				}
			} else if (s == 2) {
				setAll(v, v, v);
				updateLedsWithBlank();
			}
		}
	}
}



void GravBallMode(int intLvl)
{
#define LAND_LIMIT 3500
#define BOUNCE_LIMIT 5500
#define BOUNCE_SPEED 7000

#define POVSENSITIVITY 1300



	int poff = 1500;
	int frame = 0;
	long absAvgY = 0;
	long absAvgZ = 0;
	long avgZ = 0;
	long velocity = 0;
	long xPos = 8 * (1 << FIXED_SHIFT);
	int colIdx = EEPROM.read(1);
	int btnCSingleshot = 0;

	int nextProgTime = 400;

	//integrate +=1;

	while (1) {
		//int intLvl = integrate % 4;
		//if (intLvl == 1) intLvl = 2;
		CheckMMA8452();

		absAvgY *= 19;
		if (accelCount[YAXIS] < 0)
			absAvgY -= accelCount[YAXIS];
		else
			absAvgY += accelCount[YAXIS];
		absAvgY /= 20;

		absAvgZ *= 61;
		if (accelCount[ZAXIS] < 0)
			absAvgZ -= accelCount[ZAXIS];
		else
			absAvgZ += accelCount[ZAXIS];
		absAvgZ /= 62;

		avgZ *= 31;
		avgZ += accelCount[2];
		if (avgZ >= 0) {
			avgZ += 0 + 15;
		} else {
			avgZ -= 0 + 15;
		}
		avgZ /= 32;

		velocity += accelCount[1] >> (12 - FIXED_SHIFT);

		if (intLvl == 1) {
			velocity = (2000 * velocity) / 2001;
			xPos += velocity >> (11 - FIXED_SHIFT);
		} else {

			velocity = (300 * velocity) / 301;
			xPos += velocity >> (FIXED_SHIFT - 3);
		}

		frame++;

		if (xPos < 0) {
			xPos = 0;
			if (velocity > -300000) {
				if (velocity > -LAND_LIMIT) {
					velocity = 0;
					if (intLvl == 1) {
						velocity = 0;
						xPos = 8 * (1 << FIXED_SHIFT);
						/*
						   setAll(4063, 4063, 4063);
						   updateLeds();
						   //delay(200);
						   setAll(0, 0, 0);
						   updateLeds();
						 */
					}
				} else {
					if (intLvl == 1) {
						velocity = velocity = (-16 * velocity) / 16;
						if (velocity != 0 && velocity < BOUNCE_LIMIT)
							velocity = BOUNCE_SPEED;
					} else {
						velocity = velocity = (-11 * velocity) / 10;
					}
				}
			} else {
				velocity = 0;

				xPos = 8 * (1 << FIXED_SHIFT);

				int BSCALE = v / 3;
				setAll((colIdx & 0x3) * BSCALE, ((colIdx & 0xC) >> 2) * BSCALE, ((colIdx & 0x30) >> 4) * BSCALE);
				updateLedsWithBlank();
				delay(REWARD_FLASH_LEN);
				setAll(0, 0, 0);
				updateLedsWithBlank();
			}
		}

		int upperLimit = ((1 << FIXED_SHIFT) * 15) - 1;
		if (xPos > upperLimit) {
			xPos = upperLimit;
			//if (velocity < 500000)
			{
				if (velocity < LAND_LIMIT) {
					velocity = 0;
					if (intLvl == 1) {
						velocity = 0;
						/*
						   xPos = 8 * (1 << FIXED_SHIFT);
						   setAll(4063, 4063, 4063);
						   updateLeds();
						   //delay(200);
						   setAll(0, 0, 0);
						   updateLeds();
						 */
					}
				} else {
					if (intLvl == 1) {
						velocity = velocity = (-16 * velocity) / 16;
						if (velocity != 0 && velocity > -BOUNCE_LIMIT)
							velocity = -BOUNCE_SPEED;
					} else {
						velocity = velocity = (-12 * velocity) / 11;
					}
				}
			}
			/*else
			   {
			   velocity = 0;
			   xPos = 8 * (1 << FIXED_SHIFT);

			   /*
			   setAll(4063, 4063, 4063);
			   updateLeds();
			   //delay(200);
			   setAll(0, 0, 0);
			   updateLeds();
			   * /
			   } */
		}

		if (intLvl == 0 || intLvl == 3) {
			xPos = (accelCount[XAXIS] + (1 << 11)) << (2);

			if (intLvl == 3)
				xPos = 7 * (1 << FIXED_SHIFT) + (1 << (FIXED_SHIFT - 1)) + accelCount[XAXIS] * 30;
			else
				xPos = 7 * (1 << FIXED_SHIFT) + (1 << (FIXED_SHIFT - 1)) + (accelCount[XAXIS]) * 5;

			//xPos = 7*(1<<FIXED_SHIFT) + (1<<(FIXED_SHIFT-1)) + (accelCount[2] + ((1<<11)+avgZ))*10;
			//xPos = 7*(1<<FIXED_SHIFT) + (1<<(FIXED_SHIFT-1)) + (accelCount[2] + ((1<<11)-800))*10;
			//if (frame % 4 == 0)


			if (xPos < 0)
				xPos = 0;
			if (xPos > 15 * (1 << FIXED_SHIFT))
				xPos = 15 * (1 << FIXED_SHIFT);
		}

		int xPosInt = xPos >> FIXED_SHIFT;

		/* xPosInt = 7 * accel[1] + 8;
		   if (xPosInt < 1) xPosInt = 1;
		   if (xPosInt > 15) xPosInt = 15;
		 */
		setAll(0, 0, 0);
		//setRGB(xPosInt+1, colIdx & 1 ? v : 0, colIdx & 2 ? 0 : v, colIdx & 4 ? 0 : v);

		int brightCutoff = 2;

		int a1 = xPos & ((1 << FIXED_SHIFT) - 1);
		int bright = (((long)a1) * v) >> FIXED_SHIFT;
		if (bright <= brightCutoff)
			bright = 0;
		if (xPosInt < 15)
			setRGB(xPosInt + 1, (colIdx & 0x3) * (bright >> 2), ((colIdx & 0xC) >> 2) * (bright >> 2), ((colIdx & 0x30) >> 4) * (bright >> 2));
		bright = (((long)(((1 << FIXED_SHIFT) - 1) - a1)) * v) >> FIXED_SHIFT;
		if (bright <= brightCutoff)
			bright = 0;
		setRGB(xPosInt, (colIdx & 0x3) * (bright >> 2), ((colIdx & 0xC) >> 2) * (bright >> 2), ((colIdx & 0x30) >> 4) * (bright >> 2));

		if ((absAvgY > POVSENSITIVITY || absAvgZ > POVSENSITIVITY) && frame % 2 == 0) {
			if (0) {
				bright = v;
				setAll(colIdx & 1 ? bright : 0, colIdx & 2 ? bright : 0, colIdx & 4 ? bright : 0);
			} else {
				int i1 = accelCount[ZAXIS] + (1 << 11);
				int i2;
				int i3;

				if (accelCount[YAXIS] < 0) {
					i2 = -accelCount[YAXIS];
					i3 = 0;
				} else {
					i3 = accelCount[YAXIS];
					i2 = 0;
				}


				//i1 = (1<< 12) - i1;
				//i2 = (1<< 12) - i2;
				//i3 = (1<< 12) - i3;

				i1 /= 2;
				i2 /= 2;
				i3 /= 2;

				//setAll(i2, i1, i3);
				setAll(v * ((float)i2) / 4096, v * ((float)i1) / 4096, v * ((float)i3) / 4096);
			}
		}
		updateLedsWithBlank();




		if (!MagicShifter_Poll()) {
			EEPROM.write(1, colIdx);
			break;
		}

		if (centerBtnClickedTime != 0) {
			colIdx++;
			if (colIdx > 0x3F)
				colIdx = 1;
			centerBtnClickedTime = 0;
		}
	}
}



void shakeStrobo()
{
	setAll(0, 0, 200);
	updateLeds();
	delay(300);
	setAll(0, 200, 0);
	updateLeds();
	delay(300);
	setAll(200, 0, 0);
	updateLeds();
	delay(300);
	setAll(0, 0, 0);
	updateLeds();

	
	int poff = 1500;
	int oldV = v;
	v = (1 << 12) - 1;
	long frame = 0;
	long absAvgY = 0;
	long absAvgZ = 0;
	long avgZ = 0;
	long velocity = 0;
	long xPos = 8 * (1 << FIXED_SHIFT);
	int colIdx = 2;
	int btnCSingleshot = 0;

	int nextProgTime = 400;

	int color = 0;
	int colorState = 0;
	int policeMode = 1;

	AccelConfig toyConfig;
	toyConfig.accelInterruptNr = 1;
	toyConfig.tapInterruptNr = 0;
	toyConfig.transientInterruptNr = 0;
	toyConfig.accelScale = 2;
	toyConfig.accelDataRate = 0;	
	AccelSetup(&toyConfig);

	while (1) {
		AccelPoll();

		absAvgY *= 19;
		if (accelCount[0] < 0)
			absAvgY -= accelCount[1];
		else
			absAvgY += accelCount[0];
		absAvgY /= 20;

		absAvgZ *= 19;
		if (accelCount[2] < 0)
			absAvgZ -= accelCount[2];
		else
			absAvgZ += accelCount[2];
		absAvgZ /= 20;

		avgZ *= 31;
		avgZ += accelCount[2];
		if (avgZ >= 0) {
			avgZ += 0 + 15;
		} else {
			avgZ -= 0 + 15;
		}
		avgZ /= 32;
		frame++;


		if (absAvgY > 1300 || absAvgZ > 1500) {
			if (colorState == 0)
				color = (color + 1) % 3;
			colorState = 30;
		} else if (colorState && absAvgY < 1300 && absAvgZ < 1500) {
			colorState--;
		}

		if (colorState) {
			if (policeMode == 0) {
				int modFactor = 8;
				int modOn = 1;
				int modd = (frame / 35) % modFactor;
				if (modd < modOn)
					setAll(0, 0, 0);
				else if (modd >= (modFactor >> 1) && modd < ((modFactor >> 1) + modOn))
					setAll(0, 0, v);
				else
					setAll(0, 0, 0);
			} else {
				setAll(color == 0 ? v : 0, color == 1 ? v : 0, color == 2 ? v : 0);
			}
		} else {
			setAll(0, 0, 0);
		}
		updateLedsWithBlank();

		if (!MagicShifter_Poll())
			break;
		if (centerBtnClickedTime != 0) {
			policeMode = (policeMode + 1) % 2;
			centerBtnClickedTime = 0;
		}
	}
	v = oldV;
}


void UnicornShadowMode()
{
	int xx = 0;
	int pDelay = 230;
	int t1 = 10;
	int t2 = 50;

	int poff = 1500;
	int nextProgTime = 400;
	int colIdx = 2;
	int moveDir = 0;
	int btnCSingleshot = 0;





	setAll(0, 0, 0);
	setRGB((xx + 4 * 3) & 0xF, 0, 0, 255);
	updateLeds();
	delay(t1);
	setAll(0, 0, 0);
	updateLeds();
	delay(t2);

	setAll(0, 0, 0);
	setRGB((xx + 3 * 3) & 0xF, 0, v, 255);
	updateLeds();
	delay(t1);
	setAll(0, 0, 0);
	updateLeds();
	delay(t2);

	setAll(0, 0, 0);
	setRGB((xx + 2 * 3) & 0xF, 0, 255, 0);
	updateLeds();
	delay(t1);
	setAll(0, 0, 0);
	updateLeds();
	delay(t2);

	setAll(0, 0, 0);
	setRGB((xx + 1 * 3) & 0xF, 255, 255, 0);
	updateLeds();
	delay(t1);
	setAll(0, 0, 0);
	updateLeds();
	delay(t2);

	setAll(0, 0, 0);
	setRGB((xx + 0 * 3) & 0xF, 255, 0, 0);
	updateLeds();
	delay(t1);
	setAll(0, 0, 0);
	updateLeds();
	delay(t2);


	while (1) {
		CheckMMA8452();

		setAll(0, 0, 0);
		setRGB((xx + 0 * 3) & 0xF, 255, 0, 0);

		setRGB((xx + 1 * 3) & 0xF, 255, 255, 0);
		setRGB((xx + 2 * 3) & 0xF, 0, 255, 0);

		setRGB((xx + 3 * 3) & 0xF, 0, 255, 255);
		setRGB((xx + 4 * 3) & 0xF, 0, 0, 255);

		xx++;
		updateLeds();

		for (int i = 0; i < pDelay; i++) {
			if (pDelay != 1)
				delay(1);
			if (!MagicShifter_Poll())
				return;

			if (centerBtnClickedTime != 0) {
				if (centerBtnClickedTime > LONG_CLICK_TIME) {
					centerBtnClickedTime = 0;
					while (!centerBtnClickedTime) {
						if (!MagicShifter_Poll())
							return;
					}
				} else {
					delay(50);
					pDelay += 1 + pDelay / 2;
					if (pDelay > 250 && pDelay < 500) {
						delay(1000);
						pDelay = 1000;
					} else if (pDelay >= 500) {
						pDelay = 1;
					}
				}

				centerBtnClickedTime = 0;
			}
		}

	}
}

void DISCOMode()
{
	int frame = 0;
	long avgZ = 0;
	long xPos;

	int colIdx = 1;
	int discoIdx = 0;
	uint8_t r, g, b;

	r = g = 0;
	b = 255;


	AccelConfig toyConfig;
	toyConfig.accelInterruptNr = 1;
	toyConfig.tapInterruptNr = 0;
	toyConfig.transientInterruptNr = 0;
	toyConfig.accelScale = 2;
	toyConfig.accelDataRate = 0;	
	AccelSetup(&toyConfig);

	while (1) {
		AccelPoll();

		avgZ *= 31;
		avgZ += accelCount[2];
		if (avgZ >= 0) {
			avgZ += 0 + 15;
		} else {
			avgZ -= 0 + 15;
		}
		avgZ /= 32;

		frame++;


		if (!discoIdx) {
			xPos = 7 * (1 << FIXED_SHIFT) + (1 << (FIXED_SHIFT - 1)) + (accelCount[2] - avgZ) * 40;
			if (xPos < 0)
				xPos = 0;
			if (xPos > 15 * (1 << FIXED_SHIFT))
				xPos = 15 * (1 << FIXED_SHIFT);
			
			
			int8_t xPosInt = xPos >> FIXED_SHIFT;			
			uint8_t xPosRemainder = (xPos & (1 << FIXED_SHIFT) - 1);

			setAll(0, 0, 0);
			setRGB(xPosInt + 1, (r * xPosRemainder) >> FIXED_SHIFT, (g * xPosRemainder) >> FIXED_SHIFT, (b * xPosRemainder) >> FIXED_SHIFT);
			xPosRemainder = ((1 << FIXED_SHIFT) - 1) - xPosRemainder;			
			setRGB(xPosInt, (r * xPosRemainder) >> FIXED_SHIFT, (g * xPosRemainder) >> FIXED_SHIFT, (b * xPosRemainder) >> FIXED_SHIFT);
			updateLedsWithBlank();			
		} else {
			xPos = (accelCount[2] - avgZ);
			if (xPos < 0)
				xPos = -xPos;
			xPos >>= (discoIdx - 1);
			xPos -= 5;
			if (xPos > 16)
				xPos = 16;

			setAll(0, 0, 0);
			for (int i = 0; i < xPos; i++)
				setRGB(15 - i, r, g, b);

			updateLedsWithBlank();
		}

		if (!MagicShifter_Poll())
			break;

		if (powerBtnClickedTime != 0)
		{
			discoIdx = (discoIdx + 1) % 6;
			powerBtnClickedTime = 0;
		}
		if (centerBtnClickedTime != 0) {
			colIdx++;
			if (colIdx > 7) colIdx = 1;
			r = (colIdx & 1) ? 255 : 0;
			g = (colIdx & 2) ? 255 : 0;
			b = (colIdx & 4) ? 255 : 0;
			//PickupColor(r, g, b);
			centerBtnClickedTime = 0;
		}
	}
}





void RaceToTheMoon()
{
#define LAND_LIMIT 4000
	//#define BOUNCE_LIMIT 8000
	//#define BOUNCE_SPEED 8000

#define LOUNCH_SPEED 8000

#define MAXX (((1 << FIXED_SHIFT)*15) - 1)

	long velocity = 0;
	long xPos = 8 * (1 << FIXED_SHIFT);
	long lastXPos = 0;
	int colIdx = EEPROM.read(1);
	int btnCSingleshot = 0;

	while (1) {
		CheckMMA8452();
		velocity += accelCount[0];

		if (xPos < lastXPos)
			lastXPos = xPos;

		//velocity = (2000*velocity)/2001;
		xPos += velocity >> 8;



		if (xPos < 0) {
			xPos = 0;
			// velocity must be negative otherwise we would not be here
			velocity = -velocity;

			if (velocity < LAND_LIMIT) {
				velocity = -LOUNCH_SPEED;
				xPos = MAXX;
				lastXPos = MAXX;

				int BSCALE = v / 3;
				setAll((colIdx & 0x3) * BSCALE, ((colIdx & 0xC) >> 2) * BSCALE, ((colIdx & 0x30) >> 4) * BSCALE);
				updateLedsWithBlank();
				delay(REWARD_FLASH_LEN);
				setAll(0, 0, 0);
				updateLedsWithBlank();
			} else {
				velocity = (10 * velocity) / 9;
				//if (velocity < BOUNCE_LIMIT)
				//  velocity = BOUNCE_SPEED;
			}
		}



		if (xPos > MAXX) {

			if (velocity < LAND_LIMIT && lastXPos < (MAXX / 2)) {
				velocity = LOUNCH_SPEED;
				xPos = 0;

				int BSCALE = v / 3;
				setAll((colIdx & 0x3) * BSCALE, ((colIdx & 0xC) >> 2) * BSCALE, ((colIdx & 0x30) >> 4) * BSCALE);
				updateLedsWithBlank();
				delay(REWARD_FLASH_LEN);
				setAll(0, 0, 0);
				updateLedsWithBlank();
			} else {
				xPos = MAXX;
				velocity = 0;
				lastXPos = MAXX;
			}
		}


		int xPosInt = xPos >> FIXED_SHIFT;

		setAll(0, 0, 0);
		int brightCutoff = 2;
		int a1 = xPos & ((1 << FIXED_SHIFT) - 1);
		int bright = (((long)a1) * v) >> FIXED_SHIFT;
		if (bright <= brightCutoff)
			bright = 0;
		if (xPosInt < 15)
			setRGB(xPosInt + 1, (colIdx & 0x3) * (bright >> 2), ((colIdx & 0xC) >> 2) * (bright >> 2), ((colIdx & 0x30) >> 4) * (bright >> 2));
		bright = (((long)(((1 << FIXED_SHIFT) - 1) - a1)) * v) >> FIXED_SHIFT;
		if (bright <= brightCutoff)
			bright = 0;
		setRGB(xPosInt, (colIdx & 0x3) * (bright >> 2), ((colIdx & 0xC) >> 2) * (bright >> 2), ((colIdx & 0x30) >> 4) * (bright >> 2));
		updateLedsWithBlank();




		if (!MagicShifter_Poll()) {
			EEPROM.write(1, colIdx);
			break;
		}

		if (centerBtnClickedTime != 0) {
			colIdx++;
			if (colIdx > 0x3F)
				colIdx = 1;
			centerBtnClickedTime = 0;
		}
	}
}

/*

void RaceToTheMoon()
{     
    #define LAND_LIMIT 4000
    //#define BOUNCE_LIMIT 8000
    //#define BOUNCE_SPEED 8000
    
    #define LOUNCH_SPEED 8000
    
    #define MAXX (((1 << FIXED_SHIFT)*15) - 1)
  
    long velocity = 0;
    long xPos = 8 * (1 << FIXED_SHIFT);
    long lastXPos = 0;
    int colIdx = EEPROM.read(1);
    int btnCSingleshot = 0;
     
    while (1) 
    {
      CheckMMA8452();
      velocity += accelCount[1];
      
      if (xPos < lastXPos)
        lastXPos = xPos;
      
      //velocity = (2000*velocity)/2001;
      xPos += velocity >> 8;
      
      
      
      if (xPos < 0)
      {
        xPos = 0;
        // velocity must be negative otherwise we would not be here
        velocity = -velocity;
       
        if (velocity < LAND_LIMIT) 
        {
          velocity = -LOUNCH_SPEED;
          xPos = MAXX;
          lastXPos = MAXX;
           
          int BSCALE = v/3;
          setAll((colIdx & 0x3) * BSCALE, ((colIdx & 0xC)>>2) * BSCALE,  ((colIdx & 0x30)>>4) * BSCALE);
          updateLedsWithBlank();
          delay(REWARD_FLASH_LEN);
          setAll(0, 0, 0);
          updateLedsWithBlank();
        }
        else
        {
            velocity = (10 * velocity) / 9;
            //if (velocity < BOUNCE_LIMIT)
            //  velocity = BOUNCE_SPEED;
        }
      }
      
      
      
      if (xPos > MAXX)
      {
        
        if (velocity < LAND_LIMIT && lastXPos < (MAXX /2)) 
        {
          velocity = LOUNCH_SPEED;
          xPos = 0;
           
          int BSCALE = v/3;
          setAll((colIdx & 0x3) * BSCALE, ((colIdx & 0xC)>>2) * BSCALE,  ((colIdx & 0x30)>>4) * BSCALE);
          updateLedsWithBlank();
          delay(REWARD_FLASH_LEN);
          setAll(0, 0, 0);
          updateLedsWithBlank();
        }
        else
        {
          xPos = MAXX;
          velocity = 0;
          lastXPos = MAXX;
        }
      }
      
      
      int xPosInt = xPos>> FIXED_SHIFT;
    
      setAll(0, 0, 0);
      int brightCutoff = 2;
      int a1 = xPos&((1<<FIXED_SHIFT)-1);
      int bright = (((long)a1)*v)>>FIXED_SHIFT;
      if (bright <= brightCutoff) bright = 0;
      if (xPosInt < 15) setRGB(xPosInt+1, (colIdx & 0x3) * (bright>>2), ((colIdx & 0xC)>>2) * (bright>>2),  ((colIdx & 0x30)>>4) * (bright>>2));
      bright = (((long)(((1<<FIXED_SHIFT)-1) - a1))*v)>>FIXED_SHIFT;
      if (bright <= brightCutoff) bright = 0;
      setRGB(xPosInt, (colIdx & 0x3) * (bright>>2), ((colIdx & 0xC)>>2) * (bright>>2),  ((colIdx & 0x30)>>4) * (bright>>2));
      updateLedsWithBlank();
      
      
      
      
      if (!MagicShifter_Poll()) 
      {
        EEPROM.write(1, colIdx);
        break;
      }
    
      if (centerBtnClickedTime != 0)
      {
        colIdx++;
          if (colIdx > 0x3F)
            colIdx = 1;
        centerBtnClickedTime = 0;
      }
    }
}



void RaceToTheMoon()
{     
    #define LAND_LIMIT 4000
    //#define BOUNCE_LIMIT 8000
    //#define BOUNCE_SPEED 8000
    
    #define LOUNCH_SPEED 8000
    
    #define MAXX (((1 << FIXED_SHIFT)*15) - 1)
  
    long velocity = 0;
    long xPos = 8 * (1 << FIXED_SHIFT);
    long lastXPos = 0;
    int colIdx = EEPROM.read(1);
     
    while (1) 
    {
      CheckMMA8452();
      velocity += accelCount[1];
      
      if (xPos < lastXPos)
        lastXPos = xPos;
      
      //velocity = (2000*velocity)/2001;
      xPos += velocity >> 8;
      
      
      
      if (xPos < 0)
      {
        xPos = 0;
        // velocity must be negative otherwise we would not be here
        velocity = -velocity;
       
        if (velocity < LAND_LIMIT) 
        {
          velocity = -LOUNCH_SPEED;
          xPos = MAXX;
          lastXPos = MAXX;
           
          int BSCALE = v/3;
          setAll((colIdx & 0x3) * BSCALE, ((colIdx & 0xC)>>2) * BSCALE,  ((colIdx & 0x30)>>4) * BSCALE);
          updateLedsWithBlank();
          delay(REWARD_FLASH_LEN);
          setAll(0, 0, 0);
          updateLedsWithBlank();
        }
        else
        {
            velocity = (10 * velocity) / 9;
            //if (velocity < BOUNCE_LIMIT)
            //  velocity = BOUNCE_SPEED;
        }
      }
      
      
      
      if (xPos > MAXX)
      {
        
        if (velocity < LAND_LIMIT && lastXPos < (MAXX /2)) 
        {
          velocity = LOUNCH_SPEED;
          xPos = 0;
           
          int BSCALE = v/3;
          setAll((colIdx & 0x3) * BSCALE, ((colIdx & 0xC)>>2) * BSCALE,  ((colIdx & 0x30)>>4) * BSCALE);
          updateLedsWithBlank();
          delay(REWARD_FLASH_LEN);
          setAll(0, 0, 0);
          updateLedsWithBlank();
        }
        else
        {
          xPos = MAXX;
          velocity = 0;
          lastXPos = MAXX;
        }
      }
      
      
      int xPosInt = xPos>> FIXED_SHIFT;
    
      setAll(0, 0, 0);
      int brightCutoff = 2;
      int a1 = xPos&((1<<FIXED_SHIFT)-1);
      int bright = (((long)a1)*v)>>FIXED_SHIFT;
      if (bright <= brightCutoff) bright = 0;
      if (xPosInt < 15) setRGB(xPosInt+1, (colIdx & 0x3) * (bright>>2), ((colIdx & 0xC)>>2) * (bright>>2),  ((colIdx & 0x30)>>4) * (bright>>2));
      bright = (((long)(((1<<FIXED_SHIFT)-1) - a1))*v)>>FIXED_SHIFT;
      if (bright <= brightCutoff) bright = 0;
      setRGB(xPosInt, (colIdx & 0x3) * (bright>>2), ((colIdx & 0xC)>>2) * (bright>>2),  ((colIdx & 0x30)>>4) * (bright>>2));
      updateLedsWithBlank();
      
      
      
      
      if (!MagicShifter_Poll()) 
      {
        EEPROM.write(1, colIdx);
        break;
      }
    
      if (centerBtnClickedTime != 0)
      {
        colIdx++;
          if (colIdx > 0x3F)
            colIdx = 1;
        centerBtnClickedTime = 0;
      }
    }
}

*/

void SquareMode()
{
#define MAXX (((1 << FIXED_SHIFT)*15) -1)

	int colorLock = 2, colorLockValue;

	long xV = 0;
	long xPos = 8 * (1 << FIXED_SHIFT);

	long yV = 0;
	long yPos = 8 * (1 << FIXED_SHIFT);

	int colIdx = 123;

	AccelConfig shakeConfig;
	shakeConfig.accelInterruptNr = 1;
	shakeConfig.tapInterruptNr = 0;
	shakeConfig.transientInterruptNr = 0;
	shakeConfig.accelScale = 2;
	shakeConfig.accelDataRate = 0;
	AccelSetup(&shakeConfig);

	while (1) {
		CheckMMA8452();
		xV += accelCount[0];

		xV = (600 * xV) / 601;
		//yV += accelCount[0];

		xPos += xV >> 11;
		yPos += yV >> 11;

		long bounce = 30;
		//byte yBounce = 16;

		byte r, g, b;
		if (colorLock == 0)
			Wheel(colorLockValue, r, g, b);
		else
			Wheel(colIdx + yPos / (2 * (1 << FIXED_SHIFT - 4) / 3), r, g, b);

		byte hasBounced = false;
		if (xPos < 0) {
			xPos = 0;
			//xPos = -xPos;
			// velocity must be negative otherwise we would not be here
			xV = -xV;
			//xV = (bounce * -xV) / 32;
			if (xV < 500)
				xV = 0;
			hasBounced = true;
		} 
		else if (xPos > MAXX) {
			xPos = MAXX;
			if (xV < 500)
				xV = 0;
			//xV = -((bounce * xV) / 32);
			xV = (xV * 30) / 32;
			if (xV < 500)
				xV = 0;
			xV = -xV;
			hasBounced = true;
			// weird side
			//xV = 0;
		/*
			setAll(0,v,0);
				updateLedsWithBlank();
				delay(1);
				setAll(0, 0, 0);
				updateLedsWithBlank();
		*/
		}

		if (hasBounced && colorLock == 2)
			colIdx += 61;
			

		long bright;
		int s1 = 0;
		int s = (8 - s1);

		if (xV > 250000) {
			xV = 0;

			xPos = 8 * (1 << FIXED_SHIFT);

			for (int i = 0; i < 3; i++) {
				setAll(v, v, v);
				updateLedsWithBlank();
				delay(50);
				setAll(0, 0, 0);
				updateLedsWithBlank();
				delay(50);
			}
		}

		yPos = 7 * (1 << FIXED_SHIFT) + (1 << (FIXED_SHIFT - 1)) + (accelCount[1]);


		int xPosInt = xPos >> FIXED_SHIFT;

		setAll(0, 0, 0);
		int brightCutoff = 2;

		byte s0 = 0;
		int a1 = xPos & ((1 << (FIXED_SHIFT)) - 1);
		bright = (((long)(a1 >> s0)) * v) >> (FIXED_SHIFT - s0);
		//if (bright <= brightCutoff) bright = 0;




		if (xPosInt < 15)
			setRGB(xPosInt + 1, (r * (bright >> s1)) >> s, (g * (bright >> s1)) >> s, (b * (bright >> s1)) >> s);
		//bright = (((long)(((1<<FIXED_SHIFT)-1) - a1))*v)>>FIXED_SHIFT;
		bright = v - bright;
		//if (bright <= brightCutoff) bright = 0;
		setRGB(xPosInt, (r * (bright >> s1)) >> s, (g * (bright >> s1)) >> s, (b * (bright >> s1)) >> s);
		updateLedsWithBlank();




		if (!MagicShifter_Poll()) {
			//EEPROM.write(1, colIdx);
			break;
		}

		if (centerBtnPressed != 0) {
			//xV = 0;
			//colorLockValue = colIdx + yPos/(2*(1<<FIXED_SHIFT-4)/3);
		}
		if (centerBtnClickedTime != 0) {
			if (centerBtnClickedTime < LONG_CLICK_TIME) {
				//colorLock = 0;
				colIdx += 71;
			} else {

				colorLock = (colorLock + 1) % 3;

				colorLockValue = colIdx + yPos / (2 * (1 << FIXED_SHIFT - 4) / 3);

				if (!colorLock)
					setAll((((long)r) * (bright >> s1)) >> s, (((long)g) * (bright >> s1)) >> s, (((long)b) * (bright >> s1)) >> s);
				else {
					int sf = 12;
					if (colorLock == 2)
						sf = 61;
					for (int i = 0; i < 16; i++) {
						Wheel(i * sf, r, g, b);
						setRGB(i, (((long)r) * (bright >> s1)) >> s, (((long)g) * (bright >> s1)) >> s, (((long)b) * (bright >> s1)) >> s);
					}
				}
				updateLedsWithBlank();
				delay(600);
				setAll(0, 0, 0);
				updateLedsWithBlank();

			}
			centerBtnClickedTime = 0;
		}
	}
}



// state of UpdateAccelShakeCurrentColumn
long negTime, posTime, lastIntTime, lastFirstTime;
//long nextZero, nextStart;
int loopTime = 0;
int negDelta, posDelta;
byte shakeState = 0;
byte middleFired = 1;
byte lastFired = 1;
byte AccelFPSFrameNr = 0;
long AccelFPSLastFrameRateTimeStamp = 0;


uint8_t AccelShakeNrOfFrames = 0;
uint8_t AccelShakeFrameNr = 0xFF;
uint8_t AccelShakeFrameChanged = 0;
uint8_t AccelShakeFrameDir = 0;
uint8_t AccelShakeFrameUse = 3;
uint16_t AccelShakeFrameLength = 128; // length of 128 frames in ms

// evaluates last transient // and clears it!
// AccelPoll must be called externaly
// returns columnNr or 
// 0xFF when inactive
// 0xFE when active but blank 
// disable any interrupts that hinder transient interrupts (tap for example) 
// to optimize the algorithm (be carefull not to intrrupt an ongoing interrupt transfer by waiting a little (till its finished)
void UpdateAccelShakeCurrentColumn()
{
	long timeStamp = millis();

	// frame handling
	if (AccelShakeFrameNr < 0xFE)
	{
		AccelShakeFrameNr += AccelShakeFrameDir;
		// under or overflow
		if (AccelShakeFrameNr == 0xFF || AccelShakeFrameNr >= AccelShakeNrOfFrames)
			AccelShakeFrameNr = 0xFE;
#ifdef FRAMEDEBUG
		if (AccelShakeFrameNr == AccelShakeNrOfFrames/2)
			AccelShakeFrameNr = 0xFE;
#endif
	}
	else
	{
		// timeout 
		if (timeStamp > lastIntTime + 500)
		{
			AccelShakeFrameNr = 0xFF;
		}
	}

	// center predictor (center - frames/2)
	if (!middleFired && !shakeState && (AccelShakeFrameUse & 1) &&
		timeStamp > negTime+((posDelta+loopTime)>>1)-(((long)AccelShakeFrameLength * (long)AccelShakeNrOfFrames)>>8))
	{
		middleFired = 1;	
		AccelShakeFrameNr = AccelShakeNrOfFrames-1;
		AccelShakeFrameDir = -1;
#ifdef SHAKEDEBUG
		setRGB(5, 400, 100,400);
		updateLedsWithBlank();
		
#endif
	}
	else if (!middleFired && shakeState && (AccelShakeFrameUse & 2) &&
		timeStamp > posTime+((negDelta+loopTime)>>1)-(((long)AccelShakeFrameLength*(long)AccelShakeNrOfFrames)>>8))
	{
		middleFired = 1;	
		AccelShakeFrameNr = 0;
		AccelShakeFrameDir = 1;
#ifdef SHAKEDEBUG
		setRGB(5, 100, 500, 100);
		updateLedsWithBlank();	
#endif
	}

	if (lastTransientStatus != 0)
	{
		lastIntTime = timeStamp;
		if (lastTransientStatus & 0x08)
		{
			if (lastTransientStatus & 0x04)
			{	
				// negative
				if (shakeState)
				{
#ifdef SHAKEDEBUG
					setRGB(0, 0, 500, 0);
					updateLedsWithBlank();
#endif
					lastFirstTime = timeStamp;
					negTime = timeStamp;
					negDelta = negTime-posTime;
					shakeState = 0;
					middleFired = 0;	
					lastFired = 0;
					AccelShakeFrameChanged = 1;
				}
#ifdef SHAKEDEBUG
				else
				{
					setRGB(0, 0, 10, 0);
					updateLedsWithBlank();	
				}
#endif
			}	
			else
			{
				//positive 
				if (!shakeState)
				{
#ifdef SHAKEDEBUG
					setRGB(0, 500, 0,500);
					updateLedsWithBlank();	
#endif					
					lastFirstTime = timeStamp;
					posTime = timeStamp;
					posDelta = posTime-negTime;
					shakeState = 1;
					middleFired = 0;
					lastFired = 0;
					AccelShakeFrameChanged = 1;
				}
#ifdef SHAKEDEBUG
				else
				{
					setRGB(0, 10, 0, 10);
					updateLedsWithBlank();	
				}
#endif
			}
		}
		lastTransientStatus = 0;
	}		
	else
	{
		// recognize last interrupts
		if (!lastFired && timeStamp > lastIntTime + LASTINTERRUPTTIMEOUT)
		{
			loopTime = lastIntTime - lastFirstTime;
			lastFired = 1;
			/*
			if (shakeState)
			{ 
				nextZero = posTime+loopTime + (negDelta-loopTime)/2;
				//nextZero = posTime+negDelta/2;
			}
			else	
			{
				nextZero = negTime+loopTime + (posDelta-loopTime)/2;
				//nextZero = negTime+posDelta/2;
			}
			nextStart = nextZero - (((long)AccelShakeFrameLength * (long)AccelShakeNrOfFrames)>>8); 
*/
#ifdef SHAKEDEBUG
			setRGB(0, 100, 100, 100);
			updateLedsWithBlank();	
#endif
		}
	}

	// FPS counter used for determining length of complete frame
	AccelFPSFrameNr++;
	if (AccelFPSFrameNr >= 127)
	{
		AccelShakeFrameLength = timeStamp - AccelFPSLastFrameRateTimeStamp;
		AccelFPSLastFrameRateTimeStamp = timeStamp;
		AccelFPSFrameNr = 0;
	}
}


#define CENTERX (((1 << FIXED_SHIFT)*7) + (1 << (FIXED_SHIFT-1)))
#define MAXX (((1 << FIXED_SHIFT)*15) -1)
// physic game AND bitmap display
void MagicShifterMode()
{
	// state of physics sim
	int colorLock = 2, colorLockValue;
	long bounce = 30;
	long xV = 0;
	long xPos = CENTERX;
	int colIdx = 123;
	uint8_t physicsMode = 0; // 0 bounce
	byte r, g, b;
	byte hasBounced;

	// state of bitmap shake
	MSBitmap bitmap;
	byte sector = 1;
	uint8_t frameNr = 0;
	


	uint8_t shakeConfigActive = 0;
	AccelConfig shakeConfig;
	shakeConfig.accelInterruptNr = 0;
	shakeConfig.tapInterruptNr = 0;
	shakeConfig.transientInterruptNr = 1;
	shakeConfig.accelScale = 2;
	shakeConfig.accelDataRate = 2;	

	AccelConfig toyConfig;
	toyConfig.accelInterruptNr = 2;
	toyConfig.tapInterruptNr = 1;
	toyConfig.transientInterruptNr = 1;
	toyConfig.accelScale = 2;
	toyConfig.accelDataRate = 0;

	AccelSetup(&toyConfig);

	// load initial image
	LoadBitmap(sector, &bitmap);
	AccelShakeNrOfFrames = bitmap.header.frameWidth*COLUMNMULTIPLY;

	while (1)
	{
		AccelPoll();
		UpdateAccelShakeCurrentColumn();
		if (!MagicShifter_Poll())
		{
			break;					
		}
		if (centerBtnClickedTime != 0) 
		{
			// color change
			colIdx += 61;
			// load next Bitmap
			frameNr = 0;		
			do 
			{
				sector++;
				if (sector >= 128)
					sector = 1;
				LoadBitmap(sector, &bitmap);
				AccelShakeNrOfFrames = bitmap.header.frameWidth*COLUMNMULTIPLY;

			} while (sector != 1 && (AccelShakeNrOfFrames == 0xFF || bitmap.header.subType != 0xBA));	
			centerBtnClickedTime = 0;
		}
		//if ((lastTapStatus & (0x88 | 0x44)) == (0x88 | 0x40))
		if (powerBtnClickedTime != 0) 
		{
				powerBtnClickedTime = 0;	
			physicsMode = (physicsMode + 1) % 3;
			lastTapStatus = 0;
		}

#ifdef SHAKEDEBUG
		delayMicroseconds(400);
		setAll(0,0,0);
		updateLedsWithBrightness();
#else
		// no shake so let's do the physics simulation here
		if (AccelShakeFrameNr == 0xFF)
		{
			if (shakeConfigActive)
			{
				AccelSetup(&toyConfig);
				shakeConfigActive = 0;
			}

			if (zTaps != 0)
				colIdx += 77;
			zTaps = 0;
			
			switch (physicsMode)
			{
				case 0:
					if (xTaps == 2)
						xV += 150000;
					if (xTaps == -2)
						xV -= 150000;
					if (xTaps == 1)
						xV += 25000;
					if (xTaps == -1)
						xV -= 25000;
					xTaps = 0;

					xV += accelCount[0];
					xV = (600 * xV) / 601; // dampening
					xPos += xV / 4000; // inertia?
				
					hasBounced = false;
					if (xPos < 0) {
						xPos = 0;
						// velocity must be negative otherwise we would not be here
						xV = -xV;
						xV = (bounce * xV) / 32;
						if (xV < 500)
							xV = 0;
						hasBounced = true;
					} 
					else if (xPos > MAXX) {
						xPos = MAXX;
						xV = (xV * 30) / 32;
						if (xV < 500)
							xV = 0;
						// velocity must be positive otherwise we would not be here
						xV = -xV;
						hasBounced = true;
					}
					if (hasBounced && colorLock == 2)
						colIdx += 61;
					break;

				case 1:
					if (xTaps == 2)
						xV += 50000;
					if (xTaps == -2)
						xV -= 50000;
					if (xTaps == 1)
						xV += 15000;
					if (xTaps == -1)
						xV -= 15000;

					xTaps = 0;

					xV += (CENTERX - xPos)/8 + accelCount[0]/8;
					xV = (600 * xV) / 601;
					xPos += xV/400;
					//xPos = ((1 << FIXED_SHIFT)*8) + accelCount[0]/10;
					break;
			
				case 2:
					xV = 0;
					xPos = CENTERX + 15*accelCount[0]/2;
					break;
					
			}
			
			if (colorLock == 0)
				Wheel(colorLockValue, r, g, b);
			else
				Wheel(colIdx + accelCount[1]/10, r, g, b);
					

			int8_t xPosInt = xPos >> FIXED_SHIFT;			
			uint8_t xPosRemainder = (xPos & (1 << FIXED_SHIFT) - 1);

			setAll(0, 0, 0);
			setRGB(xPosInt + 1, (r * xPosRemainder) >> FIXED_SHIFT, (g * xPosRemainder) >> FIXED_SHIFT, (b * xPosRemainder) >> FIXED_SHIFT);
			xPosRemainder = ((1 << FIXED_SHIFT) - 1) - xPosRemainder;			
			setRGB(xPosInt, (r * xPosRemainder) >> FIXED_SHIFT, (g * xPosRemainder) >> FIXED_SHIFT, (b * xPosRemainder) >> FIXED_SHIFT);
			updateLedsWithBlank();			
		}
		else	
		{
			if (!shakeConfigActive)
			{
				AccelSetup(&shakeConfig);
				shakeConfigActive = 1;
			}
			if (AccelShakeFrameChanged)
			{
				frameNr = (frameNr + 1) % (ANIMATIONDELAY*(bitmap.header.maxFrame+1));
				AccelShakeFrameChanged = 0;
			}

			if (AccelShakeFrameNr < 0xFE)
			{
				setAll(0,0,0);
				PlotBitmapColumn(&bitmap, frameNr/ANIMATIONDELAY, AccelShakeFrameNr/COLUMNMULTIPLY, 0);
			}
			else
			{
				PlotBitmapColumn(&bitmap, 0, AccelShakeFrameNr/COLUMNMULTIPLY, 0);
				setAll(0,0,0);
			}
			updateLedsWithBrightness();
		}	
#endif	
	}
}





#define TAPSTATUS(axisNr, sign, double) ((double ? 0x88 : 0x80) | (sign ? (1 << axisNr) : 0) | (1 << (4+axisNr))) 

/*
struct ModeLinearChooser
{
	int16_t minVal;
	int16_t maxVal;
	uint8_t stepSize;
	int16_t value;
}

uint8_t *modeData;
*/

/*
void TapValueHandler()
{
	if (xTaps == 2)
	{
		if (!tapActive)
			xTaps = 0;
		tapActive = 1;
	}
	if (tapActive)
	{
		if (xTaps > 0)
		{
			if (tapValue[tapIndex] < 15)
					tapValue[tapIndx]++;
		}
		if (xTaps < 0)
		{
			if (tapValue[tapIndex] > 0)
					tapValue[tapIndx]--;
		}
	}
			
}*/

void UIDemoMode()
{
	AccelConfig accelConfig;
	accelConfig.accelInterruptNr = 0;
	accelConfig.tapInterruptNr = 1;
	accelConfig.transientInterruptNr = 1;
	accelConfig.accelScale = 2;
	accelConfig.accelDataRate = 2;	

	AccelSetup(&accelConfig);

	while (1)
	{
		AccelPoll();

		// XAXIS
		if (lastTapStatus == TAPSTATUS(0, 0, 0))
		{
			setRGB(15, 255, 0, 0);
			updateLedsWithBrightness();
			
			lastTapStatus = 0;
		}
		if (lastTapStatus == TAPSTATUS(0, 1, 0))
		{
			setRGB(0, 255, 0, 0);
			updateLedsWithBrightness();
		
			lastTapStatus = 0;
		}
		if (lastTapStatus == TAPSTATUS(0, 0, 1))
		{
			setRGB(14, 255, 0, 0);
			setRGB(15, 255, 0, 0);
			updateLedsWithBrightness();
			
			lastTapStatus = 0;
		}
		if (lastTapStatus == TAPSTATUS(0, 1, 1))
		{
			setRGB(0, 255, 0, 0);
			setRGB(1, 255, 0, 0);
			updateLedsWithBrightness();
		
			lastTapStatus = 0;
		}

		// YAXIS
		if (lastTapStatus == TAPSTATUS(1, 0, 0))
		{
			setRGB(0, 0, 255, 0);
			updateLedsWithBrightness();

			lastTapStatus = 0;
		}
		if (lastTapStatus == TAPSTATUS(1, 0, 1))
		{
			setRGB(0, 0, 255, 0);
			setRGB(1, 0, 255, 0);
			updateLedsWithBrightness();

			lastTapStatus = 0;
		}
		if (lastTapStatus == TAPSTATUS(1, 1, 0))
		{
			setRGB(15, 0, 255, 0);
			updateLedsWithBrightness();

			lastTapStatus = 0;
		}
		if (lastTapStatus == TAPSTATUS(1, 1, 1))
		{
			setRGB(14, 0, 255, 0);
			setRGB(15, 0, 255, 0);
			updateLedsWithBrightness();

			lastTapStatus = 0;
		}

		// ZAXIS
		if (lastTapStatus == TAPSTATUS(2, 0, 0))
		{
			setRGB(0, 0, 0, 255);
			updateLedsWithBrightness();

			lastTapStatus = 0;
		}
		if (lastTapStatus == TAPSTATUS(2, 0, 1))
		{
			setRGB(0, 0, 0, 255);
			setRGB(1, 0, 0, 255);
			updateLedsWithBrightness();

			lastTapStatus = 0;
		}
		if (lastTapStatus == TAPSTATUS(2, 1, 0))
		{
			setRGB(15, 0, 0, 255);
			updateLedsWithBrightness();

			lastTapStatus = 0;
		}
		if (lastTapStatus == TAPSTATUS(2, 1, 1))
		{
			setRGB(14, 0, 0, 255);
			setRGB(15, 0, 0, 255);
			updateLedsWithBrightness();

			lastTapStatus = 0;
		}
	
		
		if (!MagicShifter_Poll())
		{
			break;					
		}
		if (centerBtnClickedTime != 0) 
		{
			setAll(0, 0, 20);
			updateLedsWithBrightness();

			centerBtnClickedTime = 0;
		}
		if (powerBtnClickedTime != 0) 
		{
			setAll(0, 0, 200);
			updateLedsWithBrightness();

			powerBtnClickedTime = 0;	
		}

		delay(1);
		setAll(0, 0, 0);
		updateLedsWithBrightness();
	}
}



void TapMode()
{
	while (1) {
		CheckMMA8452();
		CheckMMA8452Pulse();
		
		
		if (!MagicShifter_Poll()) {
			setAll(0, 0, 0);
			updateLedsWithBlank();
			break;
		}

		setAll(0, 0, 0);
		
		if (xTaps)
		{
			setRGB(0, xTaps == 1 ? 100 : 0, xTaps == 2 ? 100 : 0, xTaps < 0 ? 100 : 0);
			xTaps = 0;
		}
		if (yTaps)
		{
			setRGB(1, yTaps == 1 ? 100 : 0, yTaps == 2 ? 100 : 0, yTaps < 0 ? 100 : 0);
			yTaps = 0;
		}
		if (zTaps)
		{
			setRGB(2, zTaps == 1 ? 100 : 0, zTaps == 2 ? 100 : 0, zTaps < 0 ? 100 : 0);
			zTaps = 0;
		}

		setRGB(6, orientation % 2 ? 100 : 0, (orientation/2) % 2 ? 100 : 0, 0);
		setRGB(7, orientationFace ? 100 : 0, 0, 0);
		setRGB(8, zTilt ? 100 : 0, 0, 0);

		updateLedsWithBlank();
		delay(100);
	}
}



void DefenseMode()
{
	v = 0xFFF;

	digitalWrite(LEDWHITE_PIN, HIGH);
	setAll(v, v, v);
	updateLedsWithBlank();

	int frame = 0;
	int state = 0;
	

	while (1) {
		frame++;		
		CheckMMA8452();
		
		if (state == 0)
		{
			digitalWrite(LEDWHITE_PIN, HIGH);
			setAll(v, v, v);
			updateLeds();
		}
		if (state == 1)
		{
			if ((frame % 6) < 3)
			{
				setAll(v, v, v);
			}
			else
				setAll(0, 0, 0);
			updateLeds();
			digitalWrite(LEDWHITE_PIN, LOW);
		}
		if (state == 2)
		{
			setAll(0, 0, 0);
			updateLeds();
		}

		delay(10);

		if (!MagicShifter_Poll()) {
			setAll(0, 0, 0);
			updateLedsWithBlank();
			digitalWrite(LEDWHITE_PIN, LOW);
			digitalWrite(LEDUV_PIN, LOW);
			break;
		}

		if (centerBtnClickedTime != 0) {
			blink(10, 3, 2);
			centerBtnClickedTime = 0;
			state = (state + 1) % 3;
		}

	}
}




//////////////////////////////////////////
// NEW modes: use new interrupt based accel polling
// and 8bit color scheme
////////////////////////////////////////////

#define USBBUFFERSIZE 32

void USBPoll()
{
	static uint8_t pingStatus;

	if(Serial.available())
	{
		uint8_t cmdBufferIdx = 0;
		uint8_t usbBuffer[USBBUFFERSIZE];
		
		setAll(20, 0, 0);
		updateLedsWithBlank();	
		delay(1);

		while(Serial.available() && cmdBufferIdx < USBBUFFERSIZE-1) 
		{
			char b = Serial.read();
			if (b == '\n' || b == '\r')
				break;
			usbBuffer[cmdBufferIdx++] = b;
		}
		usbBuffer[cmdBufferIdx++]='\0';

		bool executed = false;
	
		if (!strcmp((const char *)usbBuffer, "MAGIC_DUMP"))
		{
			Serial.println("TODO: implement DUMP for calibration values");
		}
		if (!strcmp((const char *)usbBuffer, "MAGIC_RESET"))
		{
			run_bootloader();
		}
		else if (!strcmp((const char *)usbBuffer, "MAGIC_PING"))
		{
			pingStatus = !pingStatus;
			SetDebugLeds(pingStatus, 0, 0);


			//Serial.print(BOOTRST);
			Serial.print(":");
			Serial.print(IVSEL);
			Serial.print(":");
			//Serial.print(WDTON);
			Serial.print(":");
			Serial.print(WDE);
			Serial.print(":");
			Serial.print(WDIE);
			/*
			Serial.print(" ea: ");
			Serial.print(font16px.header.eepromAddr, HEX);
			Serial.print(" bit: ");
			Serial.print(font16px.header.pixelFormat);
			Serial.print(" max: ");
			Serial.print(font16px.header.maxFrame);
			Serial.print(" w: ");
			Serial.print(font16px.header.frameWidth);
			Serial.print(" h: ");
			Serial.print(font16px.header.frameHeight);

			Serial.print(" st: ");
			Serial.print(font16px.header.subType, HEX);
			Serial.print(" f: ");
			Serial.print(font16px.header.firstChar);
			Serial.print(" d: ");
			Serial.print(font16px.header.animationDelay);
			*/

			Serial.print("Ping Status now: ");
			Serial.println(pingStatus);
		}
		else if (!strcmp((const char *)usbBuffer, "MAGIC_UPLOAD"))
		{
			byte sector;
			uint32_t dataSize;				
			
			// sector
			sector = SerialReadByte();
			// frames (bytes)
			dataSize = SerialReadWord();

			long sectorAddr = 0x1000l * sector;
			
			// erase all necessary sectors
			uint8_t sectorsToErase = dataSize >> 12;
			if ((dataSize & 0xFFFl) != 0)
				sectorsToErase++;
			
			if (sector == 0 || sector+sectorsToErase >= 0xFF)
			{
				Serial.println("Writing over sector 0 is not allowed!");
			}
			else
			{
				// erase the needed sectors
				for (uint8_t sectorToErase = 0; sectorToErase < sectorsToErase; sectorToErase++)		
				{		
					EraseSector(sectorAddr + 0x1000l * (long)sectorToErase); 
				}


				long time = millis();
				for (uint32_t index = 0; index < dataSize; index += USBBUFFERSIZE)
				{
					byte byteIndex;
					for (byteIndex = 0; (byteIndex < USBBUFFERSIZE) && ((index + byteIndex) < dataSize); byteIndex++)
					{
						usbBuffer[byteIndex] = SerialReadByte();
					}
					// this if takes care of files that have an odd nr of bytes ;)
					// if uneven nr of bytes need to be written (last part of file)
					// then it fill up with 0xFF to even nr ob bytes (which should leave byte untuched)
					if (byteIndex & 0x01)
					{
						usbBuffer[byteIndex] = 0xFF;
						byteIndex++;
					}	
					WriteBytes(sectorAddr + index, usbBuffer, byteIndex);
				}
				time = millis() - time;

				ReadBytes(sectorAddr, usbBuffer, USBBUFFERSIZE);
				Serial.print(time);
				Serial.print("READBACK @sector");
				Serial.print(sector);
				Serial.print(" erase: ");
				Serial.print(sectorsToErase);
				Serial.print(" size: ");
				Serial.print(dataSize);
				Serial.print(":");
				for (int i = 0; i < USBBUFFERSIZE; i++)
				{
					Serial.print(usbBuffer[i], HEX);
					Serial.print(",");
				}				
				Serial.println("END");
			}
		}
		else
		{
			Serial.print("UNKNOWN CMD: \"");
			Serial.print((const char *)usbBuffer);
			Serial.println("\"");
		}
		WaitClearButtons();
		setAll(0, 0, 0);
		updateLedsWithBlank();	
	}
}







