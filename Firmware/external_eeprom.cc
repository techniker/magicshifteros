#include "firmware.h"
#include <avr/io.h>
#include <stdint.h>
#include <SPI.h>


#define EEPROM_WREN 0x06 //Write Enable 0000 0110 06h 0 0 0
#define EEPROM_WRDI 0x04 //Write Disable 0000 0100 04h 0 0 0
#define EEPROM_RDID 0x90 //Read Identification 
#define EEPROM_RDSR 0x05 //Read Status Register 0000 0101 05h 0 0 1 to Åá
#define EEPROM_WRSR 0x01 //Write Status Register 0000 0001 01h 0 0 1
#define EEPROM_READ 0x03 //Read Data Bytes 0000 0011 03h 3 0 1 to Åá
#define EEPROM_FAST_READ 0x0B //Read Data Bytes at Higher Speed 0000 1011 0Bh 3 1 1 to Åá
#define EEPROM_PP 0x02 //Page Program 0000 0010 02h 3 0 1 to 256
#define EEPROM_SE 0xD8 //Sector Erase 1101 1000 D8h 3 0 0
#define EEPROM_SE_4K 0x20 //Sector Erase 1101 1000 D8h 3 0 0
#define EEPROM_BE 0xC7 //Bulk Erase 1100 0111 C7h 0 0 0
#define EEPROM_DP 0xB9 //Deep Power-down 1011 1001 B9h 0 0 0
#define EEPROM_RES 0xAB // Release from Deep Power-down,and Read Electronic Signature 1010 1011 ABh

#define WIP_MASK 0x01

inline void SPISendOnly(uint8_t dataout)
{
	// Start transmission (MOSI)
  SPDR = dataout;
  // Wait for transmission complete
  //while(!(SPSR & (1<<SPIF)));
  
}

uint8_t SPISendAndReceive(uint8_t dataout)
{
	// Start transmission (MOSI)
  SPDR = dataout;
  // Wait for transmission complete
  while(!(SPSR & (1<<SPIF)));
  // Get return Value;
  uint8_t v = SPDR;
  return v;
}

void EEPROM_Cmd1(uint8_t cmd) 
{
   digitalWrite(EEPROM_CS_PIN, LOW);
   SPISendAndReceive(cmd);
   digitalWrite(EEPROM_CS_PIN, HIGH);
}

uint8_t EEPROM_Cmd2(uint8_t cmd, uint8_t payload) 
{
   uint8_t data;
   digitalWrite(EEPROM_CS_PIN, LOW);
   SPISendAndReceive(cmd);
   data = SPISendAndReceive(payload);
   digitalWrite(EEPROM_CS_PIN, HIGH);
   return data;
}



uint8_t ReadStatus() {
   return EEPROM_Cmd2(EEPROM_RDSR, 0); 
}

void WaitForEndOfWrite() {
   while (ReadStatus() & WIP_MASK) {
		delayMicroseconds(1);
   }
}

void UnprotectEEPROM()
{
	EEPROM_Cmd1(EEPROM_WREN); // enable writes
		WaitForEndOfWrite();
		EEPROM_Cmd1(0x50); // enable writes
		WaitForEndOfWrite();
		EEPROM_Cmd2(EEPROM_WRSR, 0); // unprotect all sectors // p16 und 17
		WaitForEndOfWrite();
		EEPROM_Cmd1(EEPROM_WREN); // enable writes
		WaitForEndOfWrite();
}

void EraseSector(uint32_t adress)
{
   UnprotectEEPROM();
   
   digitalWrite(EEPROM_CS_PIN, LOW);
   SPISendAndReceive(EEPROM_SE_4K);
	SPISendAndReceive((adress >> 16) & 0xFF);
   SPISendAndReceive((adress >> 8) & 0xFF);
   SPISendAndReceive(adress & 0xFF);
   digitalWrite(EEPROM_CS_PIN, HIGH);
   
   WaitForEndOfWrite();
   EEPROM_Cmd1(EEPROM_WRDI); // disable writes
   WaitForEndOfWrite();
}

void BulkErase(void)
{
   UnprotectEEPROM();
   
   digitalWrite(EEPROM_CS_PIN, LOW);
   SPISendAndReceive(EEPROM_BE);
   digitalWrite(EEPROM_CS_PIN, HIGH);
   
   WaitForEndOfWrite();
   EEPROM_Cmd1(EEPROM_WRDI); // disable writes
   WaitForEndOfWrite();
}

void ReadBytes(uint32_t adress, uint8_t *buffer, uint16_t len) 
{
   uint16_t i;
   
   digitalWrite(EEPROM_CS_PIN, LOW);
   SPISendAndReceive(EEPROM_READ);
   
   SPISendAndReceive((adress >> 16) & 0xFF);
   SPISendAndReceive((adress >> 8) & 0xFF);
   SPISendAndReceive(adress & 0xFF);
   
   for (i = 0; i < len; i++)
   {
      buffer[i] = SPISendAndReceive(0);
   }
   
   digitalWrite(EEPROM_CS_PIN, HIGH);
}

void WriteByte(uint32_t adress, uint8_t data) 
{
   uint16_t i;
   
   EEPROM_Cmd1(EEPROM_WREN); // enable writes

   digitalWrite(EEPROM_CS_PIN, LOW);
   
   SPISendAndReceive(EEPROM_PP);
   
   SPISendAndReceive((adress >> 16) & 0xFF);
   SPISendAndReceive((adress >> 8) & 0xFF);
   SPISendAndReceive(adress & 0xFF);
   
   SPISendAndReceive(data);
   
   digitalWrite(EEPROM_CS_PIN, HIGH);
   
   WaitForEndOfWrite();
   EEPROM_Cmd1(EEPROM_WRDI); // disable writes
}


// len must be divisable by two (even)
void WriteBytes(uint32_t adress, uint8_t *buffer, uint16_t len) 
{
   uint16_t i;

	if (len == 0) return;
   
   EEPROM_Cmd1(EEPROM_WREN); // enable writes

   digitalWrite(EEPROM_CS_PIN, LOW);
	SPISendAndReceive(0xAD); 
   SPISendAndReceive((adress >> 16) & 0xFF);
   SPISendAndReceive((adress >> 8) & 0xFF);
   SPISendAndReceive(adress & 0xFF);
	SPISendAndReceive(buffer[0]);
	SPISendAndReceive(buffer[1]);
	digitalWrite(EEPROM_CS_PIN, HIGH);

	//delay(1);
	WaitForEndOfWrite();
   	
   for (i = 2; i < len; i += 2)
   {
		digitalWrite(EEPROM_CS_PIN, LOW);
      SPISendAndReceive(0xAD); 
		SPISendAndReceive(buffer[i]);
		SPISendAndReceive(buffer[i+1]);
		digitalWrite(EEPROM_CS_PIN, HIGH);

		//delay(1);
		WaitForEndOfWrite();
   }
   
   EEPROM_Cmd1(EEPROM_WRDI); // disable writes
}


void ReadEEEPROMID(byte &id, byte &type) 
{
   digitalWrite(EEPROM_CS_PIN, LOW);

   SPISendAndReceive(EEPROM_RDID);
	SPISendAndReceive(0);
	SPISendAndReceive(0);
	SPISendAndReceive(0);
   
   id = SPISendAndReceive(0);
   type = SPISendAndReceive(0);

   digitalWrite(EEPROM_CS_PIN, HIGH);
}

