#include <stdint.h>
#include <stdbool.h>
#include <iostm8l.h>
#include "asm.h"
#include "logic.h"

extern bool channel1enable;
extern bool channel2enable;
extern bool animation_enable;

uint16_t button_impulse_count = 0;

//Crosszero interrupt
@svlreg @far @interrupt void
PortD_interrupt(void)
{
    if (channel1enable)
        PINB2_RESET();
    else
        PINB2_SET();

    if (channel2enable)
        PINB1_RESET();
    else
        PINB1_SET();

    //TIM3 start
    TIM3_CNTRH = 0;
    TIM3_CNTRL = 0;
    TIM3_CR1 = 0b00001001;

    if (animation_enable)
        animation_tick();

    EXTI_SR2 = 0b00000010;
}

@svlreg @far @interrupt void TIM3_interrupt(void)
{
    PINB2_SET();
    PINB1_SET();

    if (PD_IDR & 0b00000001)
    {
        //phase+
        if (PB_IDR & 0b00000001)
        {
            //button pressed
            if (button_impulse_count == 500)
                return;

            button_impulse_count++;
            if (button_impulse_count == 10)
                process_button();
            else if (button_impulse_count == 500)
                process_buttonhold();
        }
        else
        {
            //button released
            button_impulse_count = 0;
        }
    }

    TIM3_SR1 = 0b00000000;
}
