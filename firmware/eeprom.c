#include <iostm8l.h>
#include <stdint.h>

#define FLASH_RASS_KEY1 0xAE
#define FLASH_RASS_KEY2 0x56

void eeprom_unlock(void);
void write_word(uint16_t address, uint32_t data);

void eeprom_write32(uint16_t address, uint32_t data)
{
	eeprom_unlock();
	write_word(address, data);
}

uint32_t eeprom_read32(uint16_t address)
{
	uint32_t data = *(uint32_t*)(address);
	return data;
}

void eeprom_unlock(void)
{
   if (!(FLASH_IAPSR & 0x08))
   {
      FLASH_DUKR = FLASH_RASS_KEY1;
      FLASH_DUKR = FLASH_RASS_KEY2;

      while (!(FLASH_IAPSR & 0x08))
      {
      }
   }
}

void write_word(uint16_t address, uint32_t data)
{
	FLASH_CR2 |= 0b01000000;
 
  *((@near uint8_t*)address) = *((uint8_t*)(&data));   
  *(((@near uint8_t*)address) + 1) = *((uint8_t*)(&data) + 1);
  *(((@near uint8_t*)address) + 2) = *((uint8_t*)(&data) + 2); 
  *(((@near uint8_t*)address) + 3) = *((uint8_t*)(&data) + 3); 
}

