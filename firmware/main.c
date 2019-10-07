#include <iostm8l.h>
#include <stdint.h>

//volatile uint16_t reset_flags;

int main(void)
{
	//reset_flags = RST_SR;
	//RST_SR = 0b00111111;
	//TIM2 clk enable
	CLK_PCKENR1 = 0b00000001;
	//PORTS
	PB_ODR = 0b00100110;
	PB_DDR = 0b00100110;
	PB_CR1 = 0b10111000;
	PB_CR2 = 0b10000000;

	PD_ODR = 0b00000000;
	PD_DDR = 0b00000000;
	PD_CR1 = 0b00000000;
	PD_CR2 = 0b00000001;

	//CLK
	CLK_DIVR = 0b00000100; //sysclk F/16 = 1 MHz
	//EINT
	EXTI_CONF = 0b00001100; //B by pins, D by port
	EXTI_CR2 = 0b11000000;  //INT7 by any edge
	EXTI_CR3 = 0b00001100;  //INTВ by any edge
	//T2 - RF - 1 MHz
	TIM2_CR1 = 0b00001000; //OPM
	TIM2_IER = 0b00000001; //UIE
	TIM2_PSCR = 0;
	TIM2_ARRH = 0x07; //2ms max
	TIM2_ARRL = 0xD0;
	//T3 - Triak off - 1 MHz
	TIM2_CR1 = 0b00001000; //OPM
	TIM2_IER = 0b00000001; //UIE
	TIM2_PSCR = 0;
	TIM2_ARRH = 0x03; //1ms max
	TIM2_ARRL = 0xE8;
	//Interrupt-only activation level
	CFG_GCR = 0b00000011;
	//
	__rim();
	__wfi();
	__halt();
}
