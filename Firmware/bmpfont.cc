#if 0
// needs the folowing defines
//#define START_CHAR 65
//#define END_CHAR 69
//#define CHAR_LEN 6
//byte font_data

#include "firmware.h"
#include "fontdata.h"

int get_char_data(char c, byte pos)
{
	int p;

	if (c == ' ')
		return 0;

	c -= START_CHAR;
	p = c / FONT_DATA_CHUNK_SIZE;

	
	if (p == 0) {
		return pgm_read_byte(&(font_data1[c * CHAR_LEN + pos]));
	} else if (p == 1) {
		return pgm_read_byte(&(font_data2[(c - FONT_DATA_CHUNK_SIZE) * CHAR_LEN + pos]));
	} else if (p == 2) {
		return pgm_read_byte(&(font_data3[(c - (FONT_DATA_CHUNK_SIZE * 2)) * CHAR_LEN + pos]));
	}
	//else if (p == 3) {
	//   return font_data4[(c-(FONT_DATA_CHUNK_SIZE*3))*CHAR_LEN+pos];
	//}
	else {
		return 0x00;
	}
}


#define PIC_SIZE PIC_CUTOFF*2+7*7
#define PIC_CUTOFF 40

#define TEXTS 1

void HackingAt29C3Mode()
{
	int color = 0;
	int frame = 0;
	int textIdx = 0;
	long avgFrameTime = 100;
	long d;
	long ts = 0;
	byte state = 0;
	int ci = 1000;
	int trigp = 0;

	int lastText = 0;

/// Square
	#define MAXX (((1 << FIXED_SHIFT)*15) -1)

	int colorLock = 2, colorLockValue;

	long xV = 0;
	long xPos = 8 * (1 << FIXED_SHIFT);

	long yV = 0;
	long yPos = 8 * (1 << FIXED_SHIFT);

	int colIdx = 123;
// Square

	AccelConfig shakeConfig;
	shakeConfig.accelInterruptNr = 1;
	shakeConfig.tapInterruptNr = 0;
	shakeConfig.transientInterruptNr = 0;
	shakeConfig.accelScale = 2;
	shakeConfig.accelDataRate = 0;
	AccelSetup(&shakeConfig);


	while (1) {
			int tsC = millis();
			d = tsC - ts;

			AccelPoll();

			if (centerBtnClickedTime != 0) {
				textIdx = (textIdx + 1) % TEXTS;
				centerBtnClickedTime = 0;
			}

			if (state == 0 && accelCount[YAXIS] < -1500) state = 1;
			if (state == 1 && accelCount[YAXIS] > -1400)
			{
				if (d < 850)
				{
					avgFrameTime = d;

					//Serial.println(avgFrameTime);

					trigp = (avgFrameTime)-35;
					if (trigp < 0) trigp = 0;

				}
				ts = tsC;	
				state = 0;			
			}

			if (d > trigp && d < trigp+10 && ci > 20)
			//if (d > 65 && d < 70 && ci > 20)
			{
				ci = 7;
				frame++;
				if  (frame % 9 == 8)
				{	
					textIdx = (textIdx + 1) % TEXTS;
				}
			}

/*
			if ((d > (avgFrameTime/3)) && (d < (avgFrameTime/3 + 4)))
			{
				setAll(0,v,0);
				updateLedsWithBlank();
				delay(1);
				setAll(0, 0, 0);
				updateLedsWithBlank();
			}
*/
/*
			if ((d > (avgFrameTime/2)) && (d < (avgFrameTime/2 + 4)))
			{
				setAll(0,0,v);
				updateLedsWithBlank();
				delay(1);
				setAll(0, 0, 0);
				updateLedsWithBlank();
			}
//*/
/*
			if ((d > (avgFrameTime - 5)))
			{
				setAll(v,v,v);
				updateLedsWithBlank();
				delay(1);
				setAll(0, 0, 0);
				updateLedsWithBlank();
			}
			*/


		

		byte rb, gb, bb;
		int r, g, b;

		
		if (ci < (5*7))
		{
			char *text;
			char *text2;

			switch (textIdx) {
			case 0:
				text2 = " AWS        ";
				text  = " AWS        ";
				break;
/*
			case 1:
				text2 = " AWS        ";
				text  = " AWS        ";
				break;
			
			case 2:
				text2 = " :)          ";
				text  = "MAKE         ";
				break;
			case 3:
				text2 = "MAGIC        ";
				text  = " !!!         ";
				break;

			case 4:
				text2 = " <3          ";
				text  = "MAKE         ";
				break;

			case 5:
				text2 = "MAKE         ";
				text  = " <3          ";
				break;

			case 6:
				text = ";)              ";
				break;
			*/			
			}
			

//#define MIRROR_FONT
#define LARGE_FONT 
//#define TWO_LINES

			#ifdef MIRROR_FONT
				int data = get_char_data(*(text + (4 - ci / 7)), 6 - (ci % 7));
			#else
				int data = get_char_data(*(text + (ci / 7)), (ci % 7));
			#endif
			int data2 = get_char_data(*(text2 + (4 - ci / 7)), 6- (ci % 7));
			//Serial.println(data);
			for (int i = 0; i < 8; i++) {
				Wheel(15 * i + 16 * ci + 20 * frame, rb, gb, bb);

				r = (((long)v) * rb) >> 8;
				g = (((long)v) * gb) >> 8;
				b = (((long)v) * bb) >> 8;
				if ((data & (1 << i))) {
					//setRGB(i, (color & 1) ? v : 0, (color & 2) ?  v : 0, (color & 4) ? v : 0);
					//setRGB(i, r, g, b);

					//setRGB(15-i, ((color+1)&3) == 0 ? v : 0, ((color+1)&3) == 1 ?  v : 0, ((color+1)&3) == 2 ? v : 0);

					//setRGB(2*i, (ci&1) ? 0 : v, 0, (ci&1) ? v : 0);
					//setRGB(2*i+1, (ci&1) ? v : 0, 0, (ci&1) ? 0 : v);

#ifdef LARGE_FONT 
			#ifdef MIRROR_FONT
					setRGB(15 - (2 * i), r, g, b);
					setRGB(15 - (2 * i + 1), r, g, b);
			#else
					setRGB((2 * i), r, g, b);
					setRGB((2 * i + 1), r, g, b);
			#endif
					
#else
					setRGB(i, r, g, b);
#endif

				} else {
					//setRGB(i, 0, 0, 0);
					//setRGB(15-i, 0,0,0);

#ifdef LARGE_FONT 
				#ifdef MIRROR_FONT
					setRGB(15 - (2 * i), 0, 0, 0);
					setRGB(15 - (2 * i + 1), 0, 0, 0);
				#else
					setRGB((2 * i), 0, 0, 0);
					setRGB((2 * i + 1), 0, 0, 0);
				#endif
#else
					setRGB(i, 0, 0, 0);
#endif
				}

#ifdef TWO_LINES
				if ((data2 & (1 << i))) {
					setRGB(i+8, r, g, b);
				} else {
					setRGB(i+8, 0, 0, 0);
				}
#endif
				//setRGB(i, (indexp+i)%2 ? on : off,  ((indexp+i)/2)%2 ? on : off,  ((indexp+i)/4)%2 ? on : off);
				/*
				   int r, g, b;
				   float cutoff = 0.3;
				   r = on*(sin((i)*0.3)-cutoff)/(1-cutoff);
				   g = on*(sin((i+indexp)*0.5)-cutoff)/(1-cutoff);
				   b = on*(sin((indexp)*0.3)-cutoff)/(1-cutoff);
				   setRGB(i, r < 0 ? 0 : r, g < 0 ? 0 : g, b < 0 ? 0 : b);
				 */
			}
			updateLedsWithBlank();
			ci++;
			lastText = 150;
			//delay(1);
			delayMicroseconds(1450);
		}
		else
		{

			if (!MagicShifter_Poll()) {
				return;
			}

			if (lastText != 0)
			{
				setAll(0, 0, 0);
				updateLedsWithBlank();
				lastText--;
			}
			else
			{


	/////////////////////////////////////
	// SquareMode

			

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
	}
}

int calC[LEDS], calR[LEDS], calG[LEDS], calB[LEDS];
byte C[LEDS], R[LEDS], G[LEDS], B[LEDS];

void Calibrate(int r, int g, int b)
{
	for (int i = 0; i < LEDS; i++) {
		setAll(0, 0, 0);
		setRGB(i, r, g, b);
		updateLedsWithBlank();
		CheckTCS3771(0);
		calC[i] = cs;
		calR[i] = rs;
		calG[i] = gs;
		calB[i] = bs;

	}
	setAll(0, 0, 0);
	updateLedsWithBlank();
}

void Measure(int r, int g, int b)
{
	for (int i = 0; i < LEDS; i++) {
		setAll(0, 0, 0);
		setRGB(i, r, g, b);
		updateLedsWithBlank();
		CheckTCS3771(0);
		C[i] = (((long)v) * cs) / calC[i];
		R[i] = (((long)v) * rs) / calR[i];
		G[i] = (((long)v) * gs) / calG[i];
		B[i] = (((long)v) * bs) / calB[i];

	}
	setAll(0, 0, 0);
	updateLedsWithBlank();
}


void HackingAt29C3ScannerMode()
{
	int frame = 0;

	while (1) {
		CheckMMA8452();
		if (!MagicShifter_Poll()) {
			return;
		}

		if (centerBtnClickedTime != 0) {
			if (centerBtnClickedTime > LONG_CLICK_TIME) {
				//debugColor(0, 2000, 0, 0, 16, 20);
				//debugColor(2000, 0, 0, 0, 16, 20);
				debugColor(0, 0, 2000, 0, 16, 100);

				Calibrate(1000, 1000, 1000);
			} else {
				Measure(1000, 1000, 1000);
			}
			centerBtnClickedTime = 0;
		}




		frame++;
		for (int i = 0; i < LEDS; i++) {
			setRGB(i, R[i], G[i], B[i]);
		}
		updateLedsWithBlank();
	}
}
#endif
