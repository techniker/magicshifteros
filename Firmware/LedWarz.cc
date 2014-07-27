
#include "firmware.h"


void setPixel(byte index, byte r, byte g, byte b)
{
	setRGB(index, b, g, r);
}

static void setAllPixel(uint8_t rgb[][16])
{
	for (int8_t i = 0; i < 16; i++) {
			setPixel(i, rgb[0][i], rgb[1][i], rgb[2][i]);
	
	}
	updateLeds();
}

static void fxZipZap(int8_t center, uint8_t r, uint8_t g, uint8_t b)
{
	for (int8_t i = 0; i < 16; i++) {
		if (center+i < 16)
			setPixel(center+i, r, g, b);
		if (center-i >= 0)
			setPixel(center-i, r, g, b);
		updateLeds();
		delay(30);
	}
}

static void fxZipZap(int8_t center, uint8_t rgb[][16])
{
	for (int8_t i = 0; i < 16; i++) {
		if (center+i < 16)
			setPixel(center+i, rgb[0][center+i], rgb[1][center+i], rgb[2][center+i]);
		if (center-i >= 0)
			setPixel(center-i, rgb[0][center-i], rgb[1][center-i], rgb[2][center-i]);
		updateLeds();
		delay(30);
	}
}



static void genHighscore(uint16_t score, uint8_t rgb[][16])
{
	int sb = score;
	uint8_t idx = 0;
	for (uint8_t digit = 0; idx < 16 && score > 0; digit++)
	{
		uint8_t s = score % 5;
		score /= 5;
		for (uint8_t i = 0; i < s && idx < 16; i++, idx++)
			rgb[0][idx] = 255, rgb[1][idx] = 0, rgb[2][idx] = 0;
		for (uint8_t i = s; i < 4 && idx < 16; i++, idx++)
			rgb[0][idx] = 0, rgb[1][idx] = 0, rgb[2][idx] = 16;
		if (idx < 16)
			rgb[0][idx] = 0, rgb[1][idx] = 0, rgb[2][idx] = 0, idx++;
	}
	while (idx < 16)
		rgb[0][idx] = 0, rgb[1][idx] = 0, rgb[2][idx] = 0, idx++;


	setAllPixel(rgb);

	int i =2;
	char *text = "000\0\0";
	while(sb != 0)
	{
		text[i] = '0' + sb%10;
		sb /= 10;
		i--;
	}
	while (i > -1)
	{
		text[i] = ' ';
		i--;
	}
	displayText(text);
}



static bool waitButton()
{
	powerBtnClickedTime = 0;
	centerBtnClickedTime = 0;
	while (powerBtnClickedTime == 0 && centerBtnClickedTime == 0) {
		if (!MagicShifter_Poll())
			return false;
	}
	powerBtnClickedTime = 0;
	centerBtnClickedTime = 0;
	return true;
}

void LedWarzMode()
{
	static uint16_t highscore;
	highscore = EEPROM.read(1);
	highscore += 256*EEPROM.read(2);
	if (highscore < 0) highscore = 23;

	uint16_t score;
	uint8_t playerPos;
	uint8_t npcPos[3];
	int8_t npcVelocity[3];
	int8_t hysterisis = 0;
	bool zone0to1, zone0to2;
	uint8_t frame;
	uint8_t life;

	AccelConfig toyConfig;
	toyConfig.accelInterruptNr = 1;
	toyConfig.tapInterruptNr = 0;
	toyConfig.transientInterruptNr = 0;
	toyConfig.accelScale = 2;
	toyConfig.accelDataRate = 0;	
	AccelSetup(&toyConfig);

reset:
	frame = 0;
	score = 1;
	hysterisis = 0;
	zone0to1 = false;
	zone0to2 = false;
	playerPos = 128;
	for (int i = 0; i < 3; i++) {
		npcPos[i] = 128;
		npcVelocity[i] = 0;
	}
	life = 2;

	while (1) {
		frame++;
		AccelPoll();

		powerBtnClickedTime = 0;
		centerBtnClickedTime = 0;
		if (!MagicShifter_Poll())
			return;

		uint8_t saveZone = 0;
		int16_t yaccel = accelCount[YAXIS] + hysterisis;
		if (yaccel < -100) {
			saveZone = 1;
			hysterisis = -50;
		}
		else if (yaccel > +100) {
			saveZone = 2;
			hysterisis = +50;
		}
		else {
			hysterisis = 0;
		}

		if (-130 < yaccel && yaccel < -70)
			zone0to1 = true;
		if (yaccel < -140 || -60 < yaccel)
			zone0to1 = false;
		if (70 < yaccel && yaccel < 130)
			zone0to2 = true;
		if (yaccel < 60 || 140 < yaccel)
			zone0to2 = false;

		uint8_t rgb[3][16] = { /* zeros */ };
		int b =  2 * life * life * life + 2;
		rgb[0][playerPos/16] = b; 
		if (saveZone == 0) {
			rgb[1][playerPos/16] = b;
			rgb[2][playerPos/16] = b;
		}

		for (int i = 0; i < 3; i++) {
			if (npcVelocity[i] == 0)
				continue;
			// safe layers
			if (i == saveZone || (score < 5 && (i == saveZone+1)) || (score < 1) ) 
			{
				rgb[1][npcPos[i]/16] = 255;
				if (rgb[2][npcPos[i]/16] == 0) {
					if (zone0to1 && (i == 0 || i == 1))
						rgb[2][npcPos[i]/16] = 128;
					if (zone0to2 && (i == 0 || i == 2))
						rgb[2][npcPos[i]/16] = 128;
				}
				if (npcPos[i]/16 == playerPos/16) {
					//fxZipZap(playerPos/16, 0, 255, 0);
					npcVelocity[i] = 0;
					if (life < 3)
					{
						life++;
					}
					score++;
				}
			} else {
				if (npcPos[i]/16 == playerPos/16) {
					npcVelocity[i] = 0;
					life--;
					if (life != 0)
					{
						setAll(255,0,0);
						updateLeds();
						delay(60);
					}
					else
					{
						fxZipZap(playerPos/16, 0, 0, 255);
						if (score > highscore) {
							fxZipZap(7, 255, 0, 255);
							fxZipZap(7, 0, 255, 255);
							fxZipZap(7, 0, 255, 0);
							highscore = score;
							EEPROM.write(1,highscore);
							EEPROM.write(2,highscore/256);
						}
						uint8_t tmp_rgb[3][16];
						genHighscore(score, tmp_rgb);
						//fxZipZap(7, tmp_rgb);
						//delay(2000);
						fxZipZap(7, 0, 0, 0);
						goto reset;
					}
				}
				rgb[2][npcPos[i]/16] = 255;
				if (rgb[1][npcPos[i]/16] == 0) {
					if (zone0to1 && (i == 0 || i == 1))
						rgb[1][npcPos[i]/16] = 128;
					if (zone0to2 && (i == 0 || i == 2))
						rgb[1][npcPos[i]/16] = 128;
				}
			}
			if (frame % 4 == 0) {
				if (npcVelocity[i] > 0 && npcPos[i] > 255-npcVelocity[i])
					npcVelocity[i] = 0, score++;
				if (npcVelocity[i] < 0 && npcPos[i] < uint8_t(-npcVelocity[i]))
					npcVelocity[i] = 0, score++;
				npcPos[i] += npcVelocity[i];
			}
		}

		if (powerBtnClickedTime != 0 || centerBtnClickedTime != 0) {
			uint8_t tmp_rgb[3][16];
			fxZipZap(playerPos/16, 0, 255, 0);
			genHighscore(score, tmp_rgb);
			//fxZipZap(7, tmp_rgb);
			//if (!waitButton())
			//	return;
			//fxZipZap(playerPos/16, 0, 255, 0);
			//genHighscore(highscore, tmp_rgb);
			//fxZipZap(7, tmp_rgb);
			//if (!waitButton())
			//	return;
			//fxZipZap(playerPos/16, rgb);
			continue;
		}

		for (int i = 0; i < 16; i++)
			setPixel(i, rgb[0][i], rgb[1][i], rgb[2][i]);
		updateLeds();

		int npcIdx = xorshift32();
		if (npcIdx < 3 && npcVelocity[npcIdx] == 0) {
			npcVelocity[npcIdx] = xorshift32() % 4;
			if (xorshift32() % 2) {
		alt_start_pos_left:
				if (playerPos/16 < 3)
					goto alt_start_pos_right;
				npcPos[npcIdx] = 0;
			} else {
		alt_start_pos_right:
				if (playerPos/16 > 13)
					goto alt_start_pos_left;
				npcPos[npcIdx] = 255;
				npcVelocity[npcIdx] *= -1;
			}
		}

		delay(10);
		int16_t velocity = accelCount[XAXIS] / 16;
		if (velocity > +16)
			playerPos += 16;
		else if (velocity < -16)
			playerPos -= 16;
		else
			playerPos += velocity;
	}
}

