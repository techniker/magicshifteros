#include "firmware.h"
// Function adapted from:
// Example sketch for driving Adafruit WS2801 pixels!
// https://github.com/adafruit/Adafruit-WS2801-Library
//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g -b - back to r
void Wheel(byte WheelPos, byte & r, byte & g, byte & b)
{
	if (WheelPos < 85) {
		r = WheelPos * 3, g = 255 - WheelPos * 3, b = 0;
	} else if (WheelPos < 170) {
		WheelPos -= 85;
		r = 255 - WheelPos * 3, g = 0, b = WheelPos * 3;
	} else {
		WheelPos -= 170;
		r = 0, g = WheelPos * 3, b = 255 - WheelPos * 3;
	}
}


// TODO impement
void Wheel12Bit(int WheelPos, int &r, int &g, int &b)
{
	const int STEP = 1000;


	// 85 

	if (WheelPos < 85) {
		r = WheelPos * 3, g = 255 - WheelPos * 3, b = 0;
	} else if (WheelPos < 170) {
		WheelPos -= 85;
		r = 255 - WheelPos * 3, g = 0, b = WheelPos * 3;
	} else {
		WheelPos -= 170;
		r = 0, g = WheelPos * 3, b = 255 - WheelPos * 3;
	}
}

static uint8_t rgb2h(int r, int g, int b)
{
	int min, max, delta, h;

	min = r < g ? r : g;
	min = min < b ? min : b;

	max = r > g ? r : g;
	max = max > b ? max : b;

	delta = max - min;

	if (max <= 0.0)
		h = 0;
	else if (r >= max)
		h = 32 * (g - b) / delta;
	else if (g >= max)
		h = 64 + 32 * (b - r) / delta;
	else
		h = 128 + 32 * (r - g) / delta;

	if (h < 0)
		h += 192;
	return h;
}

static void h2rgb(uint8_t h, uint8_t & r, uint8_t & g, uint8_t & b)
{
	uint8_t hh = h / 32, ff = (h % 32) << 3;
	switch (hh) {
	case 0:
		r = 255;
		g = ff;
		b = 0;
		break;
	case 1:
		r = 255 - ff;
		g = 255;
		b = 0;
		break;
	case 2:
		r = 0;
		g = 255;
		b = ff;
		break;
	case 3:
		r = 0;
		g = 255 - ff;
		b = 255;
		break;
	case 4:
		r = ff;
		g = 0;
		b = 255;
		break;
	default:
		r = 255;
		g = 0;
		b = 255 - ff;
		break;
	}
}

void saturateRgb(uint8_t & r, uint8_t & g, uint8_t & b)
{
	uint8_t h = rgb2h(r, g, b);
	h2rgb(h, r, g, b);
}
