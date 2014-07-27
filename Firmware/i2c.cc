#include "firmware.h"
#include "i2c.h"

/*********************
 ****I2C Functions****
 *********************/

void i2cInit(void)
{
	WRITE_sda();
	// set i2c bit rate to 40KHz
	i2cSetBitrate(400);
	// enable TWI (two-wire interface)
	sbi(TWCR, TWEN);	// Enable TWI
}

void i2cSetBitrate(unsigned short bitrateKHz)
{
	unsigned char bitrate_div;
	// set i2c bitrate
	// SCL freq = F_CPU/(16+2*TWBR))
	cbi(TWSR, TWPS0);
	cbi(TWSR, TWPS1);

	//calculate bitrate division    
	bitrate_div = ((F_CPU / 4000l) / bitrateKHz);
	if (bitrate_div >= 16)
		bitrate_div = (bitrate_div - 16) / 2;
	outb(TWBR, bitrate_div);
}

void i2cSendStart(void)
{
	//WRITE_sda();
	// send start condition
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN) | (1<<TWIE);
}

void i2cSendStop(void)
{
	// transmit stop condition
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO) | (1<<TWIE);
}

void i2cWaitForComplete(void)
{
	int i = 0;		//time out variable

	// wait for i2c interface to complete operation
	while ((!(TWCR & (1 << TWINT))) && (i < 90))
		i++;
}

void i2cSendByte(unsigned char data)
{
	//WRITE_sda();
	// save data to the TWDR
	TWDR = data;
	// begin send
	TWCR = (1 << TWINT) | (1 << TWEN) | (1<<TWIE);
}

void i2cReceiveByte(unsigned char ackFlag)
{
	// begin receive over i2c
	if (ackFlag) {
		// ackFlag = TRUE: ACK the recevied data
		TWCR = (TWCR & TWCR_CMD_MASK) | BV(TWINT) | BV(TWEA);
	} else {
		// ackFlag = FALSE: NACK the recevied data
		TWCR =  (TWCR & TWCR_CMD_MASK) | BV(TWINT);
	}
}

unsigned char i2cGetReceivedByte(void)
{
	// retieve received data byte from i2c TWDR
	return (inb(TWDR));
}

unsigned char i2cGetStatus(void)
{
	// retieve current i2c status from i2c TWSR
	return (inb(TWSR));
}

