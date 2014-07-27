#include "firmware.h"
#include "i2c.h"	

#define TCS3771_ADDRESS (0x29)

#define TCS3771_REG_ENABLE    0x00
#define TCS3771_REG_ATIME     0x01
#define TCS3771_REG_PTIME     0x02
#define TCS3771_REG_WTIME     0x03
#define TCS3771_REG_AILTL     0x04
#define TCS3771_REG_AILTH     0x05
#define TCS3771_REG_AIHTL     0x06
#define TCS3771_REG_AIHTH     0x07
#define TCS3771_REG_PILTL     0x08
#define TCS3771_REG_PILTH     0x09
#define TCS3771_REG_PIHTL     0x0A
#define TCS3771_REG_PIHTH     0x0B
#define TCS3771_REG_PERS      0x0C
#define TCS3771_REG_CONFIG    0x0D
#define TCS3771_REG_PPCOUNT   0x0E
#define TCS3771_REG_CONTROL   0x0F
#define TCS3771_REG_ID        0x12
#define TCS3771_REG_STATUS    0x13
#define TCS3771_REG_CDATA     0x14
#define TCS3771_REG_CDATAH    0x15
#define TCS3771_REG_RDATA     0x16
#define TCS3771_REG_RDATAH    0x17
#define TCS3771_REG_GDATA     0x18
#define TCS3771_REG_GDATAH    0x19
#define TCS3771_REG_BDATA     0x1A
#define TCS3771_REG_BDATAH    0x1B
#define TCS3771_REG_PDATA     0x1C
#define TCS3771_REG_PDATAH    0x1D

int cs, rs, gs, bs, ps;
int lightSensorValues[5];

bool VerifyTCS3771Interrupt()
{
	TCS3771_WriteRegister(0x00, 0x00);
	if (!digitalRead(int3Pin))
		return false;
	
	TCS3771_WriteRegister(0x01, 0xFF - 0);	// one rgb cycle
	TCS3771_WriteRegister(0x02, 0xFF);	// PTIME recommendedn
	//TCS3771_WriteRegister(0x0F, 0x40 + 0x20);	// 50 mA, IR
	//TCS3771_WriteRegister(0x0F, 0x80 + 0x20);	// 25 mA, IR
	TCS3771_WriteRegister(0x0F, 0x80 + 0x40 + 0x20);	// 12.5 mA, IR
	TCS3771_WriteRegister(0x0E, 0x03);	// one count
	//TCS3771_WriteRegister(0x00, 0x07);	// on, pox and rgb
   TCS3771_WriteRegister(0x00, 0x10 + 0x07);	// on, pox and rgb, AIN interrup enabled

	delay(30);
	int val = digitalRead(int3Pin);
	TCS3771_WriteRegister(0x00, 0x00);
	return !val;
}

void CheckTCS3771(int cycles)
{
	TCS3771_WriteRegister(0x01, 0xFF - cycles);	// one rgb cycle
	TCS3771_WriteRegister(0x02, 0xFF);	// PTIME recommendedn
	//TCS3771_WriteRegister(0x0F, 0x40 + 0x20);	// 50 mA, IR
	//TCS3771_WriteRegister(0x0F, 0x80 + 0x20);	// 25 mA, IR
	TCS3771_WriteRegister(0x0F, 0x80 + 0x40 + 0x20);	// 12.5 mA, IR
	TCS3771_WriteRegister(0x0E, 0x03);	// one count
	//TCS3771_WriteRegister(0x00, 0x07);	// on, pox and rgb
   TCS3771_WriteRegister(0x00, 0x10 + 0x07);	// on, pox and rgb, AIN interrup enabled

	while (digitalRead(int3Pin));
	//delay(30);
	//delayMicroseconds(cycles * 2400 + 3000);

	//int status = TCS3771_ReadRegister(0x13);
	//Serial.print("stat: 0x");
	//Serial.println(status, HEX);

	cs = TCS3771_ReadRegister(0x14);
	cs += ((int)TCS3771_ReadRegister(0x15)) << 8;
	//Serial.print("c: ");
	//Serial.println(cs);

	rs = TCS3771_ReadRegister(0x16);
	rs += ((int)TCS3771_ReadRegister(0x17)) << 8;
	//Serial.print("r: ");
	//Serial.println(rs);

	gs = TCS3771_ReadRegister(0x18);
	gs += ((int)TCS3771_ReadRegister(0x19)) << 8;
	//Serial.print("g: ");
	//Serial.println(gs);

	bs = TCS3771_ReadRegister(0x1A);
	bs += ((int)TCS3771_ReadRegister(0x1B)) << 8;
	//Serial.print("b: ");
	//Serial.println(bs);

	ps = TCS3771_ReadRegister(0x1C);
	ps += ((int)TCS3771_ReadRegister(0x1D)) << 8;
	//for (int i = 0; i < ps/5; i++)
	//{ 
	//  Serial.print(".");
	//}
	//Serial.println(ps);

	lightSensorValues[0] = cs;
	lightSensorValues[1] = rs;
	lightSensorValues[2] = gs;
	lightSensorValues[3] = bs;
	lightSensorValues[4] = ps;
	TCS3771_WriteRegister(0x00, 0x00);
}

void PickupColorEx(uint8_t cycles, uint8_t useLight)
{
	setAll(0,0,0);
	updateLeds();
	ClearInterruptI2CReadout();

	if (useLight)
		SetWhiteLED(255);

	CheckTCS3771(cycles);
	
	if (useLight)
		SetWhiteLED(0);
}

void PickupColor(uint8_t &byte_r, uint8_t &byte_g, uint8_t &byte_b)
{
	setAll(0,0,0);
	updateLeds();
	ClearInterruptI2CReadout();

	SetWhiteLED(255);
	CheckTCS3771(10);
	SetWhiteLED(0);
			
	int maximum = rs;
	if (gs > maximum)
		maximum = gs;
	if (bs > maximum)
		maximum = bs;

	byte_r = (255l * rs) / maximum;
	byte_g = (255l * gs) / maximum;
	byte_b = (255l * bs) / maximum;
	saturateRgb(byte_r, byte_g, byte_b);
}

/*
float colorMatrix[3][3] = {{13457./5057283., -2650./5057283., 40./722469.}, {-2804./15171849., 108034./15171849., -4357./2167407.}, {-496./1685761., -2533./1685761., 947./240823.}};

void PickupColorMatrix(uint8_t &byte_r, uint8_t &byte_g, uint8_t &byte_b)
{
	setAll(0,0,0);
	updateLeds();
	ClearInterruptI2CReadout();

	SetWhiteLED(255);
	CheckTCS3771(10);
	SetWhiteLED(0);

	float result[3] = {0,0,0};

	for (byte x = 0; x < 3; x++)
	{
		for (byte y = 0; y < 3; y++)
		{
			result[y] += lightSensorValues[x+1]*colorMatrix[y][x];
		}
	}

	float maximum = result[0];
	if (result[1] > maximum)
		maximum = result[1];
	if (result[2] > maximum)
		maximum = result[2];


	byte_r = (255l * result[0]) / maximum;
	byte_g = (255l * result[1]) / maximum;
	byte_b = (255l * result[2]) / maximum;


	saturateRgb(byte_r, byte_g, byte_b);
}
*/
void ColorPickerMode()
{
	byte doFlash = true;
	int maxxx = 500;
	int frame = 0;
	int start;
	uint8_t byte_r=255, byte_g=255, byte_b=255;
	float avgPS = 0;
	byte all = 0;
	byte cycle = 0;
	byte colors[] = {255, 0, 0,		0, 255, 0,		0, 0, 255, 		255, 170, 170,		170, 255, 170, 	170, 170, 255, 		255, 255, 255,    };
	char rtext[10] = CUSTOMIZED_NAME;
	uint8_t sampleTime = 100;
	uint8_t useLight = 0;
	uint8_t saturate = 1;

	while (1) {
		frame++;
		
		if (!MagicShifter_Poll())
			break;

		CheckTCS3771(4);
		
		if (ps > maxxx) maxxx = ps;
		
		avgPS = (2*avgPS + ps)/3.0;
		start = 16 - ((16* avgPS) / maxxx);

		if (start >= 16)
		{
			start = 15;	
		}
		if (all)
			start = 0;
		debugColor(byte_r, byte_g, byte_b, start, 16, 1);

		if (all)
		{
			displayText(rtext, false);
		}

		if (centerBtnClickedTime != 0) 
		{
			if (centerBtnClickedTime >LONG_CLICK_TIME)
			{
				saturate = !saturate;
				setAll(255, 255, 255);
				delay(10);
			}
			else
			{
				if (saturate) 
				{
					PickupColor(byte_r, byte_g, byte_b);
				}
				else
				{
					PickupColorEx(sampleTime, useLight);
					byte_r = rs;
					byte_g = gs;
					byte_b = bs;
				}
			
				Int2Str(cs, rtext);
			}
			centerBtnClickedTime = 0;
		}

		if (powerBtnClickedTime != 0) 
		{			
			
			if (cycle == 7)
			{
				cycle = 0;
				all = 0;
			}
			else 
			{
				all = 1;
				byte_r = colors[cycle*3 + 0];
				byte_g = colors[cycle*3 + 1];
				byte_b = colors[cycle*3 + 2];
				cycle = cycle + 1;
			}

			

			powerBtnClickedTime = 0;
		}

		
	}

	SetWhiteLED(0);
	TCS3771_WriteRegister(0x00, 0x00);
}

#if 0
void ColorSensorTest()
{
	int s = 0;
	// join i2c bus (address optional for master)
	Wire.begin();

	while (1) {
		if (s == 0) {
			debugColor(100, 0, 0, 0, 16, 1);
			debugColor(0, 100, 0, 0, 16, 1);
			debugColor(0, 0, 100, 0, 16, 1);
			debugColor(0, 0, 0, 0, 16, 1);
		} else {
			delay(20);
		}
		Serial.println("---");

		Wire.beginTransmission(TCS3771_ADDRESS);
		Wire.write(0x80);
		Wire.write(0x01);	// Turn the device on and enable ADC
		Wire.endTransmission();

		Wire.beginTransmission(TCS3771_ADDRESS);	// Request confirmation
		Wire.requestFrom(TCS3771_ADDRESS, 3);
		int x = Wire.read();
		Wire.endTransmission();

		Wire.beginTransmission(TCS3771_ADDRESS);
		Wire.write(0x80 + 0x12);
		Wire.endTransmission();

		Wire.requestFrom(TCS3771_ADDRESS, 1);
		while (Wire.available() < 1) ;
		int y = Wire.read();

		Serial.print("rx00:");
		Serial.print(x);
		Serial.print(" rx12:");
		Serial.println(y);

		if (!MagicShifter_Poll()) {
			setAll(20, 20, 20);
			updateLedsWithBlank();
			WaitClearButtons();
		}
		if (powerBtnClickedTime != 0) {
			powerBtnClickedTime = 0;
		}
		if (centerBtnClickedTime != 0) {
			s = (s + 1) % 3;
			digitalWrite(LEDWHITE_PIN, s & 1);
			digitalWrite(LEDUV_PIN, s & 2);
			centerBtnClickedTime = 0;
		}
	}
}
#endif

/*
void ColorSensorTest()
{
  int s = 0;
  // join i2c bus (address optional for master)
  Wire.begin();
  
  while (1)
  {
    if (s == 0)
    {
    debugColor(100, 0, 0, 0, 16, 1);
    debugColor(0, 100, 0, 0, 16, 1);
    debugColor(0, 0, 100, 0, 16, 1);
    debugColor(0, 0, 0, 0, 16, 1);
    }
    else
    {
      delay(20);
    }
    Serial.println("---");
    
    Wire.beginTransmission(TCS3771_ADDRESS);
    Wire.write(0x80);
    Wire.write(0x01); // Turn the device on and enable ADC
    Wire.endTransmission();
    
    Wire.beginTransmission(TCS3771_ADDRESS); // Request confirmation
    Wire.requestFrom(TCS3771_ADDRESS,3);
    int x = Wire.read();
    Wire.endTransmission();   
    
    Wire.beginTransmission(TCS3771_ADDRESS);
    Wire.write(0x80 + 0x12);
    Wire.endTransmission();  
    
    Wire.requestFrom(TCS3771_ADDRESS, 1);        
    while(Wire.available() < 1);               
    int y = Wire.read(); 
    
    Serial.print("rx00:");
    Serial.print(x);
    Serial.print(" rx12:");
    Serial.println(y);
    
    
    if (!MagicShifter_Poll())
    {
      setAll(20, 20, 20);
      updateLedsWithBlank();
      WaitClearButtons();
    }
    
    if (powerBtnClickedTime != 0)
    {
      
       
      powerBtnClickedTime = 0;
    }
    if (centerBtnClickedTime != 0)
    {
      s = (s+1)%3;
      digitalWrite(LEDWHITE_PIN, s&1);
      digitalWrite(LEDUV_PIN, s&2);
      centerBtnClickedTime = 0;
    }
  }
}
*/



void TCS3771_WriteRegister(byte address, byte data)
{
  i2cSendStart();
  i2cWaitForComplete();
  
  i2cSendByte((TCS3771_ADDRESS<<1));
  i2cWaitForComplete();
  
  i2cSendByte(0x80 | address);	// write register address
  i2cWaitForComplete();
  
  i2cSendByte(data);
  i2cWaitForComplete();
  
  i2cSendStop();
  
  cbi(TWCR, TWEN);	// Disable TWI
  sbi(TWCR, TWEN);	// Enable TWI
}

byte TCS3771_ReadRegister(byte address)
{
  byte data;
  
  i2cSendStart();
  i2cWaitForComplete();
  
  i2cSendByte((TCS3771_ADDRESS<<1));	// write 0xB4
  i2cWaitForComplete();
  
  i2cSendByte(0x80 | address);	// write register address
  i2cWaitForComplete();
  i2cSendStop();

  i2cSendStart();
  i2cWaitForComplete();

  i2cSendByte((TCS3771_ADDRESS<<1)|0x01);	// write 0xB5
  i2cWaitForComplete();

  i2cReceiveByte(FALSE); // or true??
  i2cWaitForComplete();

  data = i2cGetReceivedByte();	// Get MSB result

  i2cSendStop();

  delay(1);
  cbi(TWCR, TWEN);	// Disable TWI
  sbi(TWCR, TWEN);	// Enable TWI
  
  return data;
}

