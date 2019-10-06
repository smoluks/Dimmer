#include <iostm8l.h>
#include <stdint.h>

volatile uint16_t reset_flags;

int main(void)
{
		reset_flags = RST_SR;
		RST_SR = 0b00111111;
		//
		CLK_PCKENR1 = 0b00000001;
		//PORTS
    PB_DDR = 0b00100000;
    PB_CR1 = 0b10100000;
		PB_CR2 = 0b10000000;
    PB_ODR = 0b00100000;
		//CLK
		CLK_DIVR = 0b00000100; //sysclk F/16 = 1 MHz
		//EINT
		EXTI_CONF = 0b00001100; //B by pins, D by port
		EXTI_CR2 = 0b11000000; //INT7 by rising edge
		//T2 - RF
		TIM2_CR1=0b00001000; //OPM
		TIM2_IER=0b00000001; //UIE
		TIM2_PSCR=0;
		TIM2_ARRH=0x07;
		TIM2_ARRL=0xD0;
		//
		_asm("RIM\n");
		PB_ODR = 0b00000000;
    while(1)
    {
    }
}


