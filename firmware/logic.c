#include <stdint.h>
#include <stdbool.h>
#include <logic.h>
#include <eeprom.h>

#define BUTTON_A_ADDR 0x1000
#define BUTTON_B_ADDR 0x1004
#define BUTTON_ONOFF_ADDR 0x1008
#define BUTTON_SLEEP_ADDR 0x100C

void animation_sleep(uint16_t step);
void animation_prgmode(uint16_t step);
void animation_prgmodestep(uint16_t step);
void animation_prgmodeend(uint16_t step);
void set_animation(void (*animation)(uint16_t step));

bool programming_mode = false;
uint8_t programming_step = 0;

bool channel1enable = false;
bool channel2enable = false;

void process_rf(uint32_t command)
{
	if(programming_mode)
	{
			switch(programming_step)
			{
				case BUTTON_A:
					eeprom_write32(BUTTON_A_ADDR, command);
					set_animation(animation_prgmodestep);
					programming_step = BUTTON_B;
					break;
					
				case BUTTON_B:
					eeprom_write32(BUTTON_B_ADDR, command);
					set_animation(animation_prgmodestep);
					programming_step = BUTTON_ONOFF;
					break;
					
				case BUTTON_ONOFF:
					eeprom_write32(BUTTON_ONOFF_ADDR, command);
					set_animation(animation_prgmodestep);
					programming_step = BUTTON_SLEEP;
					break;
					
				case BUTTON_SLEEP:
					eeprom_write32(BUTTON_SLEEP_ADDR, command);
					set_animation(animation_prgmodeend);
					programming_mode = false;
					break;
			}
	}
	else if(command == eeprom_read32(BUTTON_A_ADDR))
	{
		channel1enable = !channel1enable;
	}
	else if(command == eeprom_read32(BUTTON_B_ADDR))
	{
		channel2enable = !channel2enable;
	}
	else if(command == eeprom_read32(BUTTON_ONOFF_ADDR))
	{
		if (channel1enable || channel2enable)
		{
			channel1enable = false;
			channel2enable = false;
		}
		else
		{
			channel1enable = true;
			channel2enable = true;
		}
	}
	else if(command == eeprom_read32(BUTTON_SLEEP_ADDR))
	{
		set_animation(animation_sleep);
	}
}

void process_button(void)
{
    if (!channel1enable)
    {
        channel1enable = true;
    }
    else
    {
        channel1enable = false;
        channel2enable = !channel2enable;
    }
}

void process_buttonhold(void)
{
	if(!programming_mode)
	{
		programming_step = BUTTON_A;
    programming_mode = true;
    set_animation(animation_prgmode);
	}
	else
	{
		programming_mode = false;
		set_animation(animation_prgmodeend);
	}
}

void process_endprg(void)
{
    programming_mode = false;
    set_animation(animation_prgmodeend);
}

bool animation_enable = false;
uint16_t animation_step = 0;
void (*animation_func)(uint16_t step);

void set_animation(void (*animation)(uint16_t step))
{
    animation_func = animation;
    animation_step = 0;
    animation_enable = true;
}

void animation_tick(void)
{
    animation_func(animation_step++);
}

void animation_sleep(uint16_t step)
{
    switch (step)
    {
    case 0:
        channel1enable = true;
        channel2enable = false;
        break;
    case 6000:
        channel1enable = false;
        channel2enable = false;
        animation_enable = false;
        break;
    }
}

void animation_prgmode(uint16_t step)
{
    switch (step)
    {
    case 0:
        channel1enable = false;
        channel2enable = false;
        break;
    case 100:
        channel1enable = true;
        channel2enable = true;
        break;
    case 300:
        channel1enable = false;
        channel2enable = false;
        animation_enable = false;
        break;
    }
}

void animation_prgmodestep(uint16_t step)
{
    switch (step)
    {
    case 0:
        channel1enable = true;
        channel2enable = false;
        break;
		case 100:
        channel1enable = false;
        channel2enable = false;
        animation_enable = false;
        break;		
    }
}

void animation_prgmodeend(uint16_t step)
{
    switch (step)
    {
    case 0:
        channel1enable = false;
        channel2enable = false;
        break;
		case 100:
        channel1enable = true;
        channel2enable = true;
        animation_enable = false;
        break;		
    }
}

void animation_start(uint16_t step)
{
    switch (step)
    {
    case 0:
        channel1enable = false;
        channel2enable = false;
        animation_enable = false;
        break;
    }
}

