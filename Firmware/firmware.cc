////////////////////////////////////////
// MagicShifterOS
// http://magicshifter.net
// GPL by Philipp Tiefenbacher
// http://www.gnu.org/copyleft/gpl.html
// wizards23@gmail.com
/////////////////////////////////////////

#include "firmware.h"


int modeIdx = 0;


void LightMeasureDebug();

void ResetMode()
{
	run_bootloader();
}

void setup()
{
	MagicShifter_Init();

	// while (1)
	// {
	// 	setAll(255, 128, 5);
	// 	updateLedsWithBlank();
	// 	delay(100);
	// }


	// if you want you can show the battery status on startup
	//showBatteryStatus(500);	

	//enable turbo charge
	enableTurboCharge(true);	

#ifdef DEBUGMODE
#else
	#ifdef UNITTEST
	UnitTestMode();
	#else

	#define MODECOUNT 11

	modeIdx = loadMode();
	if (modeIdx < 0 || modeIdx >= MODECOUNT)
	{
		modeIdx = 0; 
	}	

	// modeIdx = 0; // usefull for debugging ;)

	while (1) {
#ifdef EYE_FRIENDLY
		v = 1;
#endif

		switch (modeIdx) {
			case 0:
				MagicShifterMode();
				break;
			case 1:
				ColorPickerMode();
				break;
			case 2:
				DISCOMode();
				break;
			case 3:
				RainbowMove();
				break;
			case 4:
				UnicornShadowMode();
				break;
			case 5:
				LightMeasureDebug();
				break;
			case 6:
				FutureMode();
				break;
			case 7:
				RTCMode();			
				break;
		  	case 8:
				// LedWarzMode();
				LedWarzMode();
				break;
			case 9:
				BikeLight();
				break;
			case 10:
				showBatteryStatus(1000);
				break;

			case 11:
				ResetMode();
				break;
		}

		int vTemp = v;
		v = 20;

		// rewarding 50ms blink
		for (int i = 0; i < 4; i++) {
			setAll(v, v, v);
			updateLedsWithBlank();
			delay(50);
			setAll(0, 0, 0);
			updateLedsWithBlank();
			delay(50);
		}
		blink(50, 50, 2);
		WaitClearButtons();

		setAll(0, 0, 0);
		updateLedsWithBlank();

		//modeIdx = (modeIdx + 1) % 7;
		WaitClearButtons();
		char*Text[MODECOUNT]={"Magic","Color","Beat","RGB","RGB2","Heart","Light","Time","Game", "Bike", "Power"};
		modeIdx = ModeSelector(MODECOUNT, modeIdx, Text);
		WaitClearButtons();

		v = vTemp;
	}
	#endif
#endif
}

void loop()
{
}
