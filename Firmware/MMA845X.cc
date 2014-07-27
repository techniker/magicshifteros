// adapted from Sparkfun Example: https://www.sparkfun.com/products/10953

#include "firmware.h"
#include "types.h"
#include "i2c.h"


// adress of MMA
#define SA0 0	
#if SA0
#define MMA8452_ADDRESS 0x1D	// SA0 is high, 0x1C if low
#else
#define MMA8452_ADDRESS 0x1C
#endif


// Adress of magnet sensor FSOX
//#define  MMA8452_ADDRESS 0x1E


#define MCP7941X_ADDRESS 0x6F //b01101111


// interrupt handler variables
// only use allowed combinations (WAITFOR_TAPANDTRANSIENT the only one atm)
// yu cant combine ACCEL and TAP yet
// TODO: generalize this, but this would mean that we read the status register that tells us what type of interrupt we use
// so we really losse some time for that in all cases then.
// a really smart solution would know if it was alone on this interrut or if it was shared with others
#define WAITFOR_ACCEL 1
#define WAITFOR_TAP 2
#define WAITFOR_TRANSIENT 4
#define WAITFOR_TAPANDTRANSIENT (WAITFOR_TAP | WAITFOR_TRANSIENT)
// orientation and motion not implemented yet
#define WAITFOR_ORIENTATION 8
#define WAITFOR_MOTION 16
#define WAITFOR_ORIENTATIONANDMOTION (WAITFOR_ORIENTATION | WAITFOR_MOTION)

#define WAITFOR_RTC 32

#define ORIENT_PORTRAIT_UP 0
#define ORIENT_PORTRAIT_DOWN 1
#define ORIENT_LANDSCAPE_RIGHT 2
#define ORIENT_LANDSCAPE_LEFT 3

#define FACE_FRONT 0
#define FACE_BACK 1


// globale state values for interrupt
uint8_t interruptDeviceAddr;
uint8_t interruptRegisterAddr;
uint8_t interruptNrOfRegisters;
uint8_t interruptBuffer[12];
uint8_t interruptBufferIdx;
uint8_t interruptState = 0;
//byte interruptDump[100];
//byte interruptDumpIdx = 0;

// global accelerometer status
int accelCount[3];		// Stores the 12-bit signed value
uint8_t orientation = 0;
uint8_t orientationFace = 0;
uint8_t zTilt = 0;
int8_t xTaps, yTaps, zTaps;
uint8_t lastTransientStatus = 0;
uint8_t lastTapStatus = 0;

// rtc status
uint8_t initiateRTCReadout = 0;
uint8_t rtcBuffer[7];

uint8_t accelWaitForType = 0;


// stores what interrupts are enabled on what channel for efficient interrupt handling
//uint8_t accelInterruptNr, tapInterruptNr, transientInterruptNr;
/* Set the scale below either 2, 4 or 8*/
//byte accelScale = 2;		// Sets full-scale range to +/-2, 4, or 8g. Used to calc real g values.
/* Set the output data rate below. Value should be between 0 and 7*/
//byte accelDataRate = 0;	// 0=800Hz, 1=400, 2=200, 3=100, 4=50, 5=12.5, 6=6.25, 7=1.56

//uint8_t accelInterruptNrBackup, tapInterruptNrBackup, transientInterruptNrBackup;
//byte accelScaleBackup;		// Sets full-scale range to +/-2, 4, or 8g. Used to calc real g values.
/* Set the output data rate below. Value should be between 0 and 7*/
//byte accelDataRateBackup;	// 0=800Hz, 1=400, 2=200, 3=100, 4=50, 5=12.5, 6=6.25, 7=1.56

AccelConfig accelActiveConfig;
AccelConfig accelBackupConfig;


void i2cDisableInterrupts()
{
		cbi(TWCR, TWIE);
}

void i2cEnableInterrupts()
{
		sbi(TWCR, TWIE);
}

// TODO: put this in i2c
void SetupInterruptI2CReadout(byte deviceAddr, byte registerAddr, byte nrOfRegisters)
{
	interruptDeviceAddr = deviceAddr << 1;
	interruptRegisterAddr = registerAddr;
	interruptNrOfRegisters = nrOfRegisters;
	interruptBufferIdx = 0;
	//interruptDumpIdx = 0;
	interruptState = 1;
	i2cEnableInterrupts();
	i2cSendStart();
}

void ClearInterruptI2CReadout()
{
	while (interruptState)
	{
		delayMicroseconds(1);
	} // wait for i2c to finish
	accelWaitForType = 0;
}

void BackupAccelState()
{
	memcpy(&accelBackupConfig, &accelActiveConfig, sizeof(AccelConfig));
}

void RestoreAccelState()
{
	ClearInterruptI2CReadout();
	AccelSetup(&accelBackupConfig);
}



//ISR(PCINT0_vect) 

ISR(TWI_vect) 
{ 
/*
	void readRegisters(byte address, int i, byte * dest)
	{
		i2cSendStart();
		i2cWaitForComplete();

		i2cSendByte((MMA8452_ADDRESS << 1));	// write 0xB4
		i2cWaitForComplete();

		i2cSendByte(address);	// write register address
		i2cWaitForComplete();

		i2cSendStart();
		i2cSendByte((MMA8452_ADDRESS << 1) | 0x01);	// write 0xB5
		i2cWaitForComplete();
		for (int j = 0; j < i; j++) {
			i2cReceiveByte(TRUE);
			i2cWaitForComplete();
			dest[j] = i2cGetReceivedByte();	// Get MSB result
		}
		i2cWaitForComplete();
		i2cSendStop();

		cbi(TWCR, TWEN);	// Disable TWI
		sbi(TWCR, TWEN);	// Enable TWI
	}
*/
	if (interruptState != 0)
	{
		/*if (interruptDumpIdx < 100)
		{
			interruptDump[interruptDumpIdx++] = TWSR;
		}*/

		switch (interruptState)
		{
			case 1:
				//i2cSendByte(interruptDeviceAddr);	// device write
				// save data to the TWDR
				TWDR = interruptDeviceAddr;
				// begin send
				TWCR = (1 << TWINT) | (1 << TWEN) | (1<<TWIE);
				interruptState++;
				break;
		
			case 2:
				//i2cSendByte(interruptRegisterAddr);	// write register address
				// save data to the TWDR
				TWDR = interruptRegisterAddr;
				// begin send
				TWCR = (1 << TWINT) | (1 << TWEN) | (1<<TWIE);
				interruptState++;
				break;		

			case 3:
				//i2cSendStart();
				// send start condition
				TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN) | (1<<TWIE);
				interruptState++;
				break;

			case 4:
				//i2cSendByte(interruptDeviceAddr | 0x01);	// device read
				// save data to the TWDR
				TWDR = interruptDeviceAddr | 0x01;
				// begin send
				TWCR = (1 << TWINT) | (1 << TWEN) | (1<<TWIE);
				interruptState++;
				break;

			case 5:
				//i2cReceiveByte(interruptNrOfRegisters != 1); // requ byte
				if (interruptNrOfRegisters != 1) {
					// ackFlag = TRUE: ACK the recevied data
					TWCR = (TWCR & TWCR_CMD_MASK) | BV(TWINT) | BV(TWEA);
				} else {
					// ackFlag = FALSE: NACK the recevied data
					TWCR =  (TWCR & TWCR_CMD_MASK) | BV(TWINT);
				}
				interruptState++;
				break;
				
			default:
				interruptBuffer[interruptBufferIdx++] = TWDR;	// Get result byte	
				if (interruptBufferIdx != interruptNrOfRegisters)
				{
					//i2cReceiveByte(interruptBufferIdx != (interruptNrOfRegisters-1));
					if (interruptBufferIdx != (interruptNrOfRegisters-1)) {
						// ackFlag = TRUE: ACK the recevied data
						TWCR = (TWCR & TWCR_CMD_MASK) | BV(TWINT) | BV(TWEA);
					} else {
						// ackFlag = FALSE: NACK the recevied data
						TWCR =  (TWCR & TWCR_CMD_MASK) | BV(TWINT);
					}
				}
				else
				{
					//i2cSendStop();
					// transmit stop condition
					TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO) | (1<<TWIE);
					interruptState = 0;
				}
				break;
		}  
	}
} 

/* This function will read the status of the tap source register.
   And print if there's been a single or double tap, and on what
   axis. */
void tapHandler(uint8_t source)
{
 	int8_t taps;

	if (source & 0x08)  // If DPE (double puls) bit is set
      taps = 2;
	else
      taps = 1;    
  
	if (source & 0x10)  // If AxX bit is set
	{
		if (source & 0x01)  // If PoIX is set
			xTaps = taps;
		else
			xTaps = -taps;
	}
	if (source & 0x20)  // If AxY bit is set
	{
		if (source & 0x02)  // If PoIY is set
			yTaps = taps;
		else
			yTaps = -taps;
	}
	if (source & 0x40)  // If AxZ bit is set
	{
		if (source & 0x04)  // If PoIZ is set
			zTaps = taps;
		else
			zTaps = -taps;
	}
}

/* This function will read the p/l source register and
   print what direction the sensor is now facing */
void portraitLandscapeHandler()
{
  byte pl = readRegister(0x10);  // Reads the PL_STATUS register
  orientation=(pl&0x06)>>1;  // Check on the LAPO[1:0] bits
  orientationFace = (pl&0x01);
  zTilt = (pl&0x40) ? 1 : 0;  // Check the LO bit
}

void CheckMMA8452Transient()
{
	if (digitalRead(int1Pin))	// Interrupt pin, should probably attach to interrupt function
		//if (readRegister(0x00)&0x7) // Polling, you can use this instead of the interrupt pins
	{
		//Determine source of interrupt by reading the system interrupt register
		byte Int_SourceSystem = readRegister(0x0C);
		//Set up Case statement here to service all of the possible interrupts
		if ((Int_SourceSystem&0x20)==0x20)
		{
			//Perform an Action since Transient Flag has been set
			//Read the Transient to clear system interrupt and Transient
			lastTransientStatus = readRegister(0x1E);
		}
	}
}


void CheckMMA8452()
{
/*
	// If int1 goes high, all data registers have new data 
	if (digitalRead(int1Pin))	// Interrupt pin, should probably attach to interrupt function
		//if (readRegister(0x00)&0x7) // Polling, you can use this instead of the interrupt pins
	{
		readRegisters(0x01, 6, &data[0]);	// Read the six data registers into data array

		// For loop to calculate 12-bit ADC and g value for each axis
		for (uint8_t i = 0; i < 6; i += 2) {
			accelCount[i / 2] = ((data[i] << 8) | data[i + 1]) >> 4;	// Turn the MSB and LSB into a 12-bit value
			if (data[i] > 0x7F) {	// If the number is negative, we have to make it so manually (no 12-bit data type)
				accelCount[i / 2] = ~accelCount[i / 2] + 1;
				accelCount[i / 2] *= -1;	// Transform into negative 2's complement #
			}
			// floating point is way too expensive!
			//accel[i/2] = (float) accelCount[i/2]/((1<<12)/(2*scale));  // get actual g value, this depends on scale being set
		}
	}
*/
}

/*
byte transferInitiated = 0;
void CheckMMA8452WithInterrupts()
{
	//if (digitalRead(int1Pin) && transferInitiated == 0)	
	if (!transferInitiated)
		//if (readRegister(0x00)&0x7) // Polling, you can use this instead of the interrupt pins
	{
		SetupInterruptI2CReadout(MMA8452_ADDRESS, 0x01, 6);
		transferInitiated = true;
	}

	if (transferInitiated && (interruptState == 0))
	{
		transferInitiated = false;

		for (uint8_t i = 0; i < 6; i += 2) {
			accelCount[i / 2] = ((interruptBuffer[i] << 8) | interruptBuffer[i + 1]) >> 4;	// Turn the MSB and LSB into a 12-bit value
			if (interruptBuffer[i] > 0x7F) {	// If the number is negative, we have to make it so manually (no 12-bit data type)
				accelCount[i / 2] = ~accelCount[i / 2] + 1;
				accelCount[i / 2] *= -1;	// Transform into negative 2's complement #
			}
		}
		//cbi(TWCR, TWEN);	// Disable TWI
		//sbi(TWCR, TWEN);	// Enable TWI
	}
}
*/

#if 0
void CheckMMA8452Pulse()
{
	/* If int2 goes high, either p/l has changed or there's been a single/double tap */
	if (digitalRead(int2Pin))
	{
		byte source = readRegister(0x0C);  // Read the interrupt source reg.
		if ((source & 0x10)==0x10) { // If the p/l bit is set, go check those registers
			portraitLandscapeHandler();
		}
		else if ((source & 0x08)==0x08) {  // Otherwise, if tap register is set go check that
			tapHandler();
		}
		SetDebugLeds(1, 0xFF, 0xFF);
	}
	else
	{
		SetDebugLeds(0, 0xFF, 0xFF);
	}
}
#endif

void AccelPoll()
{
	static uint8_t intOrder = 0;

	// finished 
	if (accelWaitForType != 0)
	{
		if (interruptState == 0)
		{
			switch (accelWaitForType)
			{
				case WAITFOR_ACCEL:
					// For loop to calculate 12-bit ADC and g value for each axis 
					for (uint8_t i = 0; i < 6; i += 2) {
						accelCount[i / 2] = ((interruptBuffer[i] << 8) | interruptBuffer[i + 1]) >> 4;	// Turn the MSB and LSB into a 12-bit value
						if (interruptBuffer[i] > 0x7F) {	// If the number is negative, we have to make it so manually (no 12-bit data type)
							accelCount[i / 2] = ~accelCount[i / 2] + 1;
							accelCount[i / 2] *= -1;	// Transform into negative 2's complement #
						}
					}
					break;
			
				case WAITFOR_TAPANDTRANSIENT:
					lastTransientStatus = interruptBuffer[0];
					lastTapStatus = interruptBuffer[4];
					tapHandler(lastTapStatus);
					break;

				case WAITFOR_TRANSIENT:
					lastTransientStatus = interruptBuffer[0];
					break;
			
				case WAITFOR_TAP:
					lastTapStatus = interruptBuffer[0];
					tapHandler(lastTapStatus);
					break;
				
				case WAITFOR_RTC:
					memcpy(rtcBuffer, interruptBuffer, 7);
			}
			accelWaitForType = 0;
		}
	}
	else
	{
		if (initiateRTCReadout)	
		{
			initiateRTCReadout = 0;
			SetupInterruptI2CReadout(MCP7941X_ADDRESS, 0x00, 7);
			accelWaitForType = WAITFOR_RTC;
			return;
		}
		// in2 will starve if int1 takes all polls! use 1 for the sparese interrupts (tap/transient) and 2 for accel polling
		for (uint8_t interruptNr = 1; interruptNr <= 2; interruptNr++)
		{
			if (digitalRead(interruptNr == 1 ? int1Pin : int2Pin))
			{
				if (accelActiveConfig.accelInterruptNr == interruptNr)
				{
					SetupInterruptI2CReadout(MMA8452_ADDRESS, 0x01, 6);
					accelWaitForType = WAITFOR_ACCEL;
				}
				else if (accelActiveConfig.transientInterruptNr == interruptNr)
				{
					if (accelActiveConfig.transientInterruptNr == interruptNr)
					{
						SetupInterruptI2CReadout(MMA8452_ADDRESS, 0x1E, 5);
						accelWaitForType = WAITFOR_TAPANDTRANSIENT;
					}				
					else
					{	
						SetupInterruptI2CReadout(MMA8452_ADDRESS, 0x1E, 1);
						accelWaitForType = WAITFOR_TRANSIENT;
					}
				}
				else if (accelActiveConfig.tapInterruptNr == interruptNr)
				{
					SetupInterruptI2CReadout(MMA8452_ADDRESS, 0x22, 1); // 0x22 PULSE_SRC
					accelWaitForType = WAITFOR_TAP;
				}
				// todo: same for orientation and motion/freefall

				return; // we cannot initiate moe than one transfer
			}
		}
	}
}


void AccelSetup(AccelConfig *newConfig) 
{
	ClearInterruptI2CReadout();

	memcpy(&accelActiveConfig, newConfig, sizeof(AccelConfig));

	MMA8452Standby();	// Must be in standby to change registers
	/* Set up the full scale range to 2, 4, or 8g. */
	if ((accelActiveConfig.accelScale == 2) || (accelActiveConfig.accelScale == 4) || (accelActiveConfig.accelScale == 8))
		writeRegister(0x0E, accelActiveConfig.accelScale >> 2);
	else
		writeRegister(0x0E, 0);
	/* Setup the 3 data rate bits, from 0 to 7 */
	writeRegister(0x2A, (readRegister(0x2A) & ~(0x38)) | ((accelActiveConfig.accelDataRate&0x07) << 3) );

	/* Set up portrait/landscap registers */
	writeRegister(0x11, 0x40);	// Enable P/L
	writeRegister(0x13, 0x14);	// 29deg z-lock, 
	writeRegister(0x14, 0x84);	// 45deg thresh, 14deg hyst
	writeRegister(0x12, 0x05);	// debounce counter at 100ms 

	/* Set up single and double tap */
	writeRegister(0x21, 0x7F);	// enable single/double taps on all axes

	//writeRegister(0x23, 0x20);	// x thresh at 2g
	//writeRegister(0x24, 0x20);	// y thresh at 2g
	//writeRegister(0x25, 0x8);	// z thresh at .5g

	writeRegister(0x23, 0x20);	// x thresh at ??g
	writeRegister(0x24, 0x20);	// y thresh at ??g
	writeRegister(0x25, 0x20);	// z thresh at ??g

	writeRegister(0x26, 0x30);	// 60ms time limit, the min/max here is very dependent on output data rate
	writeRegister(0x26, 0x0D);
	writeRegister(0x27, 0x28);	// 200ms between taps min
	writeRegister(0x28, 0xFF);	// 1.275s (max value) between taps max


	writeRegister(0x26, 0x09);	// ?ms time limit, the min/max here is very dependent on output data rate
	writeRegister(0x27, 0x20);	// ?ms between taps min
	writeRegister(0x28, 0x80);	// ?s  between taps max


	/* Set up interrupt 1 and 2 */
	writeRegister(0x2C, 0x02);	// Active high, push-pull
	writeRegister(0x2D, 0x19);	// DRDY int enabled, P/L enabled
	writeRegister(0x2E, 0x01);	// DRDY on INT1, P/L on INT2

	//writeRegister(0x2E, 0x00);	// all on int2



	// 0x0E: XYZ_DATA_CFG Register: gselect
	// oversampling mode is set in: 0x2B
	// 0x2A: CTRL_REG1 System Control 1 Register

	// set highpass cutff
	//writeRegister(0x0F, 0x00); // highest cutoff frequ
	writeRegister(0x0F, 0x03);
	//writeRegister(0x0F, 0x03); // lowest cutoff frequ

	/* Set up transient on X axis */
	// Step 1: Put the device in Standby Mode: Register 0x2A CTRL_REG1

	// Step 2: Enable X and Y Axes and enable the latch: Register 0x1D Configuration Register
	//writeRegister(0x1D, 0x16); // Enable Latch, Enable X and Enable Y 
	writeRegister(0x1D, 0x14); // Enable Latch, Enable Y 

	// Step 3: Set the Threshold: Register 0x1F / Note: Step count is 0.063g per count
	// 0.5g / 0.063g = 7.93. Therefore set the threshold to 8 counts
	writeRegister(0x1F, 0x08);

	writeRegister(0x1F, 0x2F);

	// Step 4: Set the Debounce Counter for 50 ms: Register 0x20 
	// Note: 100 Hz ODR, therefore 10 ms step sizes 
	writeRegister(0x20, 0x05);
	writeRegister(0x20, 0x01);

	//writeRegister(0x20, 0x09);



	byte enableVal = 0;
	byte intRouteVal = 0;

	if (accelActiveConfig.accelInterruptNr!=0)
	{
		enableVal |= 0x01;
		if (accelActiveConfig.accelInterruptNr == 1)
			intRouteVal |= 0x01;
	}
	if (accelActiveConfig.tapInterruptNr!=0)
	{
		enableVal |= 0x08;
		if (accelActiveConfig.tapInterruptNr == 1)
			intRouteVal |= 0x08;
	}
	if (accelActiveConfig.transientInterruptNr!=0)
	{
		enableVal |= 0x20;
		if (accelActiveConfig.transientInterruptNr == 1)
			intRouteVal |= 0x20;
	}

	// Step 5: Enable Transient Detection Interrupt in the System (CTRL_REG4)
	writeRegister(0x2D, enableVal);
	// Step 6: Route the Transient Interrupt to INT 1 hardware pin (CTRL_REG5)
	writeRegister(0x2E, intRouteVal);

	// Step 7: Put the device in Active Mode: Register 0x2A CTRL_REG1
	MMA8452Active();	// Set to active to start reading
}


#if 0
/* Initialize the MMA8452 registers 
   See the many application notes for more info on setting 
   all of these registers:
   http://www.freescale.com/webapp/sps/site/prod_summary.jsp?code=MMA8452Q
   
   Feel free to modify any values, these are settings that work well for me.
*/
// this should gep parameters on all the parts and select interrupts for it
void initMMA8452(byte fsr, byte dataRate)
{
	MMA8452Standby();	// Must be in standby to change registers

	/* Set up the full scale range to 2, 4, or 8g. */
	if ((fsr == 2) || (fsr == 4) || (fsr == 8))
		writeRegister(0x0E, fsr >> 2);
	else
		writeRegister(0x0E, 0);
	/* Setup the 3 data rate bits, from 0 to 7 */
	writeRegister(0x2A, readRegister(0x2A) & ~(0x38));
	if (dataRate <= 7)
		writeRegister(0x2A, readRegister(0x2A) | (dataRate << 3));
	/* Set up portrait/landscap registers */
	writeRegister(0x11, 0x40);	// Enable P/L
	writeRegister(0x13, 0x14);	// 29deg z-lock, 
	writeRegister(0x14, 0x84);	// 45deg thresh, 14deg hyst
	writeRegister(0x12, 0x05);	// debounce counter at 100ms 



	/* Set up single and double tap */
	writeRegister(0x21, 0x7F);	// enable single/double taps on all axes

	//writeRegister(0x23, 0x20);	// x thresh at 2g
	//writeRegister(0x24, 0x20);	// y thresh at 2g
	//writeRegister(0x25, 0x8);	// z thresh at .5g

	writeRegister(0x23, 0x40);	// x thresh at ??g
	writeRegister(0x24, 0x40);	// y thresh at ??g
	writeRegister(0x25, 0x40);	// z thresh at ??g

	writeRegister(0x26, 0x30);	// 60ms time limit, the min/max here is very dependent on output data rate
	writeRegister(0x27, 0x28);	// 200ms between taps min
	writeRegister(0x28, 0xFF);	// 1.275s (max value) between taps max


	writeRegister(0x26, 0x0A);	// ?ms time limit, the min/max here is very dependent on output data rate
	writeRegister(0x27, 0x08);	// ?ms between taps min
	writeRegister(0x28, 0x08);	// ?s  between taps max


	/* Set up interrupt 1 and 2 */
	writeRegister(0x2C, 0x02);	// Active high, push-pull
	writeRegister(0x2D, 0x19);	// DRDY int enabled, P/L enabled
	writeRegister(0x2E, 0x01);	// DRDY on INT1, P/L on INT2

	//writeRegister(0x2E, 0x00);	// all on int2



	// 0x0E: XYZ_DATA_CFG Register: gselect
	// oversampling mode is set in: 0x2B
	// 0x2A: CTRL_REG1 System Control 1 Register

	dataRate = 2;
	/* Setup the 3 data rate bits, from 0 to 7 */
	writeRegister(0x2A, (readRegister(0x2A) & ~(0x38)) | ((dataRate&0x07) << 3) );

	// set highpass cutff
	//writeRegister(0x0F, 0x00); // highest cutoff frequ
	writeRegister(0x0F, 0x03);
	//writeRegister(0x0F, 0x03); // lowest cutoff frequ

	/* Set up transient on X axis */
	// Step 1: Put the device in Standby Mode: Register 0x2A CTRL_REG1

	// Step 2: Enable X and Y Axes and enable the latch: Register 0x1D Configuration Register
	//writeRegister(0x1D, 0x16); // Enable Latch, Enable X and Enable Y 
	writeRegister(0x1D, 0x14); // Enable Latch, Enable Y 

	// Step 3: Set the Threshold: Register 0x1F / Note: Step count is 0.063g per count
	// 0.5g / 0.063g = 7.93. Therefore set the threshold to 8 counts
	writeRegister(0x1F, 0x08);

	writeRegister(0x1F, 0x2F);

	// Step 4: Set the Debounce Counter for 50 ms: Register 0x20 
	// Note: 100 Hz ODR, therefore 10 ms step sizes 
	writeRegister(0x20, 0x05);
	writeRegister(0x20, 0x01);

	//writeRegister(0x20, 0x09);

	// Step 5: Enable Transient Detection Interrupt in the System (CTRL_REG4)
	writeRegister(0x2D, 0x20);

	// Step 6: Route the Transient Interrupt to INT 1 hardware pin (CTRL_REG5)
	writeRegister(0x2E, 0x20);

	// Step 7: Put the device in Active Mode: Register 0x2A CTRL_REG1
	MMA8452Active();	// Set to active to start reading
}
#endif

/* Sets the MMA8452 to standby mode.
   It must be in standby to change most register settings */
void MMA8452Standby()
{
	byte c = readRegister(0x2A);
	writeRegister(0x2A, c & ~(0x01));
}

/* Sets the MMA8452 to active mode.
   Needs to be in this mode to output data */
void MMA8452Active()
{
	byte c = readRegister(0x2A);
	writeRegister(0x2A, c | 0x01);
}

/* Read i registers sequentially, starting at address 
   into the dest byte arra */
void readRegisters(byte address, int i, byte * dest)
{
	i2cSendStart();
	i2cWaitForComplete();

	i2cSendByte((MMA8452_ADDRESS << 1));	// write 0xB4
	i2cWaitForComplete();

	i2cSendByte(address);	// write register address
	i2cWaitForComplete();

	i2cSendStart();
	i2cSendByte((MMA8452_ADDRESS << 1) | 0x01);	// write 0xB5
	i2cWaitForComplete();
	for (int j = 0; j < i; j++) {
		i2cReceiveByte(TRUE);
		i2cWaitForComplete();
		dest[j] = i2cGetReceivedByte();	// Get MSB result
	}
	i2cWaitForComplete();
	i2cSendStop();

	cbi(TWCR, TWEN);	// Disable TWI
	sbi(TWCR, TWEN);	// Enable TWI
}

/* read a single byte from address and return it as a byte */
byte readRegister(uint8_t address)
{
	byte data;

	i2cSendStart();
	i2cWaitForComplete();

	i2cSendByte((MMA8452_ADDRESS << 1));	// write 0xB4
	i2cWaitForComplete();

	i2cSendByte(address);	// write register address
	i2cWaitForComplete();

	i2cSendStart();

	i2cSendByte((MMA8452_ADDRESS << 1) | 0x01);	// write 0xB5
	i2cWaitForComplete();
	i2cReceiveByte(TRUE);
	i2cWaitForComplete();

	data = i2cGetReceivedByte();	// Get MSB result
	i2cWaitForComplete();
	i2cSendStop();

	cbi(TWCR, TWEN);	// Disable TWI
	sbi(TWCR, TWEN);	// Enable TWI

	return data;
}

/* Writes a single byte (data) into address */
void writeRegister(unsigned char address, unsigned char data)
{
	i2cSendStart();
	i2cWaitForComplete();

	i2cSendByte((MMA8452_ADDRESS << 1));	// write 0xB4
	i2cWaitForComplete();

	i2cSendByte(address);	// write register address
	i2cWaitForComplete();

	i2cSendByte(data);
	i2cWaitForComplete();

	i2cSendStop();
}

