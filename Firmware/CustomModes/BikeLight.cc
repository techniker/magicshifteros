/*
 * BikeLight.cpp
 *
 * Created: 21.04.2014 19:35:06
 *  Author: Daniel
 */ 
#include "../firmware.h"
#define FIXEDMUL(x,y,digits) ((x*y)>>digits)
#define BIKEDIGITS 5
#define BIKEUNIT (1 << BIKEDIGITS)
#define DELTA_CUTOFF 1

long CUTOFF = ((long)(BIKEUNIT*0.98));
int HOLDTIME = 1500;

void BikeLight()
{
	int frame = 0;
	int lastTrigger = 0;

//	v = 128;

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

	long avgGrav = 1;
	long xAvg = 0;
	long yAvg = ((long)(BIKEUNIT*1));
	long zAvg = 0;

	
	int	avgC = 100;
	int	avgW = 99 ;
	while (1) {
		
		lastActivity = millis();
		//Accelerometer abfragen
		frame++;
		AccelPoll();
		if (!MagicShifter_Poll())
			break;

		xValue = (avgW*xValue + accelCount[0])/avgC;
		yValue = (avgW*yValue + accelCount[1])/avgC;
		zValue = (avgW*zValue + accelCount[2])/avgC;
		grav = FixedSqrt(FIXEDMUL(xValue,xValue,BIKEDIGITS)+FIXEDMUL(yValue,yValue,BIKEDIGITS)+FIXEDMUL(zValue,zValue,BIKEDIGITS));

		if (avgCount++ % 20 == 0)
		{
			xAvg = (avgW*xAvg + xValue)/avgC;
			yAvg = (avgW*yAvg + yValue)/avgC;
			zAvg = (avgW*zAvg + zValue)/avgC;
			avgGrav = FixedSqrt(FIXEDMUL(xAvg,xAvg,BIKEDIGITS)+FIXEDMUL(yAvg,yAvg,BIKEDIGITS)+FIXEDMUL(zAvg,zAvg,BIKEDIGITS));
		}

		long p = FIXEDMUL(xAvg,xValue,BIKEDIGITS) + FIXEDMUL(yAvg,yValue,BIKEDIGITS) + FIXEDMUL(zAvg,zValue,BIKEDIGITS);
		//p /= grav*avgGrav;
		p = (p<<BIKEDIGITS)/FIXEDMUL(grav,avgGrav,BIKEDIGITS);

		long pR = FIXEDMUL(xAvg,xValue,BIKEDIGITS) + FIXEDMUL(zAvg,yValue,BIKEDIGITS) + FIXEDMUL(-yAvg,zValue,BIKEDIGITS);
		//pR /= grav*avgGrav;
		pR = (pR<<BIKEDIGITS)/FIXEDMUL(grav,avgGrav,BIKEDIGITS);

		

		//debug
		if (frame% 100 == 0 )
		{
			Serial.println(grav);
			Serial.println(xAvg);
			Serial.println(yAvg);
			Serial.println(zAvg);
			Serial.println(p);
			Serial.println(pR);
			Serial.println(CUTOFF);
			Serial.println("---");
			
		}
		
		//state update
		lastTrigger++;
		if (p < CUTOFF && pR < 0)
		{
			lastTrigger = 0;
		}

		//display
		if (lastTrigger < HOLDTIME)
		{
			setAll(255, 0, 0);
		}
		else 
		{
			setAll (10, 0, 0);
		} 
			
		if (centerBtnClickedTime > 0) 
		{
			CUTOFF += DELTA_CUTOFF;			
			centerBtnClickedTime = 0;
			setAll(255, 255, 255);
			updateLedsWithBlank();
			delay(10);
		}
		if (powerBtnClickedTime > 0) 
		{
			CUTOFF -= DELTA_CUTOFF;			
			powerBtnClickedTime = 0;

			setAll(0, 0, 255);
			updateLedsWithBlank();
			delay(10);
		}
		//setRGB(15, centerBtnPressed ? 100 : 0, powerBtnPressed ? 100 : 0, 0);
		updateLedsWithBlank();
	}
}
