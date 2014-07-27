#include "firmware.h"
#include "types.h"
#include "i2c.h"

#define MCP7941X_ADDRESS 0x6F //b01101111


/* read a single byte from address and return it as a byte */
unsigned char MCP7941X_ReadRegister(uint8_t address)
{
	byte data;

	i2cSendStart();
	i2cWaitForComplete();

	i2cSendByte((MCP7941X_ADDRESS << 1));	
	i2cWaitForComplete();

	i2cSendByte(address);	// write register address
	i2cWaitForComplete();

	i2cSendStart();

	i2cSendByte((MCP7941X_ADDRESS << 1) | 0x01);	// write 0xB5
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
void MCP7941X_WriteRegister(unsigned char address, unsigned char data)
{
	i2cSendStart();
	i2cWaitForComplete();

	i2cSendByte((MCP7941X_ADDRESS << 1));	// write 0xB4
	i2cWaitForComplete();

	i2cSendByte(address);	// write register address
	i2cWaitForComplete();

	i2cSendByte(data);
	i2cWaitForComplete();

	i2cSendStop();

	cbi(TWCR, TWEN);	// Disable TWI
	sbi(TWCR, TWEN);	// Enable TWI
}

void MCP7941X_SetTime(int year, int month, int day, int hour, int minute, int second)
{
	MCP7941X_WriteRegister(0, 0); // disable clock
	
	MCP7941X_WriteRegister(1, minute % 10 + ((minute / 10) <<4));
	MCP7941X_WriteRegister(2, hour % 10 + ((hour / 10) <<4));
	MCP7941X_WriteRegister(4, day % 10 + ((day / 10) <<4));
	MCP7941X_WriteRegister(5, month % 10 + ((month / 10) <<4));
	MCP7941X_WriteRegister(6, year % 10 + ((year / 10) <<4));

	MCP7941X_WriteRegister(0, 0x80 | ( second % 10 + ((second / 10) <<4)) );
}

void MCP7941X_SetAlert0(byte alarmType, int month, int day, int hour, int minute, int second)
{
	MCP7941X_WriteRegister(0xA, ((second % 10) + ((second / 10) <<4) ) );
	MCP7941X_WriteRegister(0xB, ((minute % 10) + ((minute / 10) <<4) ) );
	MCP7941X_WriteRegister(0xC, ((hour % 10) + ((hour / 10) <<4)) );
	MCP7941X_WriteRegister(0xE, ((day % 10) + ((day / 10) <<4))  );
	MCP7941X_WriteRegister(0xF, ((month % 10) + ((month / 10) <<4)));
	MCP7941X_WriteRegister(0xD, (alarmType << 4) );
}

void MCP7941X_SetAlert1(byte alarmType, int month, int day, int hour, int minute, int second)
{
	int offset = (0x14-0xD);
	MCP7941X_WriteRegister(offset+0xA, ((second % 10) + ((second / 10) <<4) ) );
	MCP7941X_WriteRegister(offset+0xB, ((minute % 10) + ((minute / 10) <<4) ) );
	MCP7941X_WriteRegister(offset+0xC, ((hour % 10) + ((hour / 10) <<4)) );
	MCP7941X_WriteRegister(offset+0xE, ((day % 10) + ((day / 10) <<4))  );
	MCP7941X_WriteRegister(offset+0xF, ((month % 10) + ((month / 10) <<4)));
	MCP7941X_WriteRegister(offset+0xD, (alarmType << 4) );
}

