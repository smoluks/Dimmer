#include <iostm8l.h>
#include <stdint.h>
#include <stdbool.h>
#include "eeprom.h"
#include "logic.h"

#define PACKET_COUNT_TO_SUCCESS 3

void process_bit(bool bit);
void process_data(uint32_t data);
void process_command(uint32_t command);

volatile uint32_t current_data = 0;
volatile uint32_t currentdatacount = 0;
volatile uint32_t data = 0;
volatile uint8_t count = 0;

extern bool programming_mode;

//RF interrupt
@svlreg @far @interrupt void Pin7_interrupt(void)
{
	if (!(PB_IDR & 0b10000000) && (TIM2_CR1 & 0b00000001))
	{
		//FALL
		uint16_t width = (TIM2_CNTRH << 8) + TIM2_CNTRL;
		if (width > 350 && width < 450)
			process_bit(false);
		else if (width > 1000 && width < 1200)
			process_bit(true);
		else
		{
			data = 0;
			count = 0;
			currentdatacount = 0;
		}
	}
	//TIM2_CR1 = 0b00001000;

	TIM2_CNTRH = 0;
	TIM2_CNTRL = 0;
	TIM2_CR1 = 0b00001001;

	EXTI_SR1 = 0b10000000;
}

@svlreg @far @interrupt void TIM2_interrupt(void)
{
	data = 0;
	count = 0;
	TIM2_SR1 = 0b00000000;
}

void process_bit(bool bit)
{
	if (count == 24)
	{
		if (!bit && data)
		{
			process_data(data);
		}
		count = 0;
		data = 0;
	}
	else
	{
		data = data << 1;
		if (bit)
			data |= 1;

		count++;
	}
}

void process_data(uint32_t data)
{
	if (current_data != data)
	{
		current_data = data;
		currentdatacount = 0;
	}
	else
	{
		if (currentdatacount == (PACKET_COUNT_TO_SUCCESS + 1))
			return;

		if (currentdatacount++ == PACKET_COUNT_TO_SUCCESS)
			process_command(data);
	}
}

void process_command(uint32_t command)
{
	if (programming_mode)
	{
		save_remote_id(command & 0x00FFFFF0);
		process_endprg();
	}
	else if (get_remote_id() == (command & 0x00FFFFF0))
		process_rf(command & 0x0F);
}
