#include "firmware.h"

#define LIPO_AUTO_POWER_DOW_LIMIT  700	// 3,3*((R1+R2)/R2)*(700/1024) === 3.59V at R1 = 330; R2 = 560

#define LIPO_DISPLAY_LOWER_LIMIT_V         3.4
#define LIPO_DISPLAY_RED_LIMIT_V           3.65
#define LIPO_DISPLAY_ORANGE_LIMIT_V        3.9
#define LIPO_DISPLAY_UPPER_LIMIT_V         4.3

extern int modeIdx;

int ad = 600;
float adv = 3.7;
int oldad = 0;

void enableTurboCharge(int enable)
{
	if (enable) {
		pinMode(TURBOCHRG_PIN, OUTPUT);
		digitalWrite(TURBOCHRG_PIN, HIGH);
	} else {
		pinMode(TURBOCHRG_PIN, INPUT);
		digitalWrite(TURBOCHRG_PIN, LOW);
	}
}

void MeasureBattery(bool doVCalc)
{
	oldad = ad;
	ad = analogRead(PWRMGT_ANALOG_PIN);

#ifdef BABYSHIFTER
	ad = 850;
#endif

	if (doVCalc) {
		adv = 3.3 * ((330.0 + 560.0) / 560.0) * (ad / 1024.0);
	}
}



void DebugPrintBattery()
{
	MeasureBattery(true);
	Serial.print("V AD Measure:");
	Serial.println(ad);
	Serial.print("V Calc:");
	Serial.println(adv);
}


void waitForPowerDown()
{
	digitalWrite(LEDWHITE_PIN, LOW);
	digitalWrite(LEDUV_PIN, LOW);

	// TODO: no reference to modeIdx here
	if (loadMode() != modeIdx)
		saveMode(modeIdx);

	float a = 0;
	UpdateButtons();
	while (powerBtnPressed != 0) {
		int bright = (1.3 + sin(a) * 40);
		if (bright < 0)
			bright = 0;
		debugColor(bright, bright, bright, 4, 12, 10);
		a += 0.04;

		UpdateButtons();
	}

	showBatteryStatus(-500);
	powerDown();
}

void powerDown()
{
	pinMode(PWRMGT, OUTPUT);
	digitalWrite(PWRMGT, LOW);
	while (1) {};
	// we never get here if the button was not pressen
}

void showBatteryStatus(int d)
{
	//v = MAXMV;

	WaitClearButtons();
	delay(50);
	MeasureBattery(false);

	float avg = 0;
	while (avg == 0) {
		while (powerBtnPressed) {
			debugColor(0, 128, 128, 6, 9, 10);
			UpdateButtons();
			debugColor(0, 0, 0, 0, 15, 20);
		}


		for (int i = 0; i < 3; i++) {
			delay(30);
			UpdateButtons();
			MeasureBattery(true);
			if (powerBtnPressed) {
				avg = 0;
				break;
			}
			avg += adv;
		}
		avg /= 3;
	}

	setAll(0, 0, 0);


	for (int i = 0; i >= 0 && i <= 15; i++) {
		float iV = LIPO_DISPLAY_LOWER_LIMIT_V + (LIPO_DISPLAY_UPPER_LIMIT_V - LIPO_DISPLAY_LOWER_LIMIT_V) * (i / 16.0);

		if (avg > iV) {
			int red, green;
			if (iV > LIPO_DISPLAY_RED_LIMIT_V) {
				green = 255 * (iV - LIPO_DISPLAY_RED_LIMIT_V) / (LIPO_DISPLAY_UPPER_LIMIT_V - LIPO_DISPLAY_RED_LIMIT_V);
			} else
				green = 0;

			if (iV < LIPO_DISPLAY_ORANGE_LIMIT_V) {
				red = 255 * (LIPO_DISPLAY_ORANGE_LIMIT_V - iV) / (LIPO_DISPLAY_ORANGE_LIMIT_V - LIPO_DISPLAY_LOWER_LIMIT_V);
			} else
				red = 0;
			setRGB(15 - i, red, green, 0);

			//setRGB(i, 0, iV > LIPO_DISPLAY_RED_LIMIT_V  ? 150 : 0, iV < LIPO_DISPLAY_ORANGE_LIMIT_V ? 150 : 0);
		}

		if (d > 0) {
			updateLeds();
			delay(12);
		}
	}
	updateLeds();

delay(100);
#ifndef BABYSHIFTER
	if (USBSTA & 0x1)
	{
		if (digitalRead(CHRGSTAT_PIN))
			setRGB(15, 0, 255, 0);
		else
			setRGB(15, 0, 0, 255);
	}
	updateLeds();
#endif

	if (d < 0) {
		d = -d;
		for (int i = 0; i < d; i += 20) {
			UpdateButtons();
			if (powerBtnPressed || !digitalRead(BTNC))
				break;
			delay(20);
		}

		for (int i = 0; i >= 0 && i <= 15; i++) {
			setRGB(i, 0, 0, 0);
			updateLeds();
			delay(12);
		}
	} else {
		delay(100);
		for (int i = 0; i < d; i += 20) {
			UpdateButtons();
			if (powerBtnPressed || !digitalRead(BTNC))
				break;
			delay(20);
		}
	}

	setAll(0, 0, 0);
	updateLeds();

	int f = 0;
	while (1) {
		UpdateButtons();
		if ((!powerBtnPressed) && digitalRead(BTNC))
			break;
		if (f > 100) {
			setAll(128, 128, 128);
			updateLeds();
		}
		f++;
		delay(10);
	}
	WaitClearButtons();
}
