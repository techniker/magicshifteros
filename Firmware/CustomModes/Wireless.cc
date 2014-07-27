/*
 * Wireless.cc
 *
 * Created: 17.07.2014 16:57
 *  Author: Wizard23
 */ 
#include "../firmware.h"
#define SENSE_DURATION 1

void LightMeasureDebug()
{
	int frame = 0;
	int lastTrigger = 0;

	//v = 128;

	AccelConfig toyConfig;
	toyConfig.accelInterruptNr = 2;
	toyConfig.tapInterruptNr = 1;
	toyConfig.transientInterruptNr = 1;
	toyConfig.accelScale = 4;
	toyConfig.accelDataRate = 0;

	AccelSetup(&toyConfig);
	
	unsigned int avgCount = 0;

	long grav;
	long xValue = 0;
	long yValue = 0;
	long zValue = 0;

	long avgLight = 0;

	long avgGrav = 1;
	long xAvg = 0;
	long yAvg = ((long)(BIKEUNIT*1));
	long zAvg = 0;

	
	int	avgC = 100;
	int	avgW = 99 ;

	while (1) {
		//delay(3);
		setAll(0,0,0);
		
		
		CheckTCS3771(10);

		avgLight = (112 * avgLight + 16 * rs) >> 7;

		for (int i = avgLight - 10; i < avgLight + 10; i++)
		{
			if (i < rs) Serial.print('.');
			else Serial.print(' ');
		} 

		int offset = 0;
		if (rs < avgLight - offset) {
			for (int j = 0; j < (avgLight-offset) - rs; j++)
			setRGB(j, 0, 0, 1);
		}
		setRGB(7, 255, 0, 0);
		setRGB(8, 255, 0, 0);

		Serial.println(' ');
	
		frame++;
		AccelPoll();
		if (!MagicShifter_Poll())
			break;
			
		if (centerBtnClickedTime > 0) 
		{	
			centerBtnClickedTime = 0;
			setAll(255, 255, 255);
			updateLedsWithBlank();
			delay(10);
		}
		if (powerBtnClickedTime > 0) 
		{
			powerBtnClickedTime = 0;
			setAll(0, 0, 255);
			updateLedsWithBlank();
			delay(10);
		}
		updateLedsWithBlank();
	}
}