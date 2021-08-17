#include <iostm8l.h>
#include <stdint.h>
#include <asm.h>
#include <logic.h>

//volatile uint16_t reset_flags;

int main(void)
{
	//reset_flags = RST_SR;
	//RST_SR = 0b00111111;
	//TIM2 + TIM3 clk enable
	CLK_PCKENR1 = 0b00000011;
	//PORTS
	PB_ODR = 0b01000000;
	PB_DDR = 0b01000000;
	PB_CR1 = 0b01000001;
	PB_CR2 = 0b11000000;
	
	PC_ODR = 0b00010000;
	PC_DDR = 0b00010000;
	PC_CR1 = 0b00010000;
	PC_CR2 = 0b00010000;
	
	PD_ODR = 0b00000000;
	PD_DDR = 0b00000000;
	PD_CR1 = 0b00000000;
	PD_CR2 = 0b00000001;
	//CLK
	CLK_DIVR = 0b00000100; //sysclk F/16 = 1 MHz
	//EINT
	EXTI_CONF = 0b00001100; //B by pins, D by port
	EXTI_CR1 = 0b00000011;  //INT0 by any edge
	EXTI_CR2 = 0b11000000;  //INT7 by any edge
	EXTI_CR3 = 0b00001100;  //INTD by any edge
	//T2 - RF - 1 MHz
	TIM2_CR1 = 0b00001000; //OPM
	TIM2_IER = 0b00000001; //UIE
	TIM2_PSCR = 0;
	TIM2_ARRH = 0x0B; //3ms max
	TIM2_ARRL = 0xB8;
	//T3 - Triak off - 1 MHz
	TIM3_CR1 = 0b00001000; //OPM
	TIM3_IER = 0b00000001; //UIE
	TIM3_PSCR = 0;
	TIM3_ARRH = 0x13; //5ms
	TIM3_ARRL = 0x88;
	//Interrupt-only activation level
	CFG_GCR = 0b00000011;
	//WDT
	IWDG_KR = 0xcc;         //  Start the independent watchdog.
	IWDG_KR = 0x55;         //  Allow the IWDG registers to be programmed.
	IWDG_PR = 0x06;         //  Above 150 Hz
	IWDG_RLR = 6 - 1;       //  40mS
	IWDG_KR = 0xaa;         //  Reset the counter.
	//
	__rim();
	//__wfi();
	//__halt();
	while(1);
}
