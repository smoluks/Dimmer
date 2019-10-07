#include <stdint.h>
#include <stdbool.h>
#include "logic.h"

extern bool channel1enable;
extern bool channel2enable;

uint16_t buttonimpulse_count = 0;

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

    EXTI_SR2 = 0b00000010;

    tick();
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
            if (buttonimpulse_count == 500)
                return;

            buttonimpulse_count++;
            if (buttonimpulse_count == 10)
                process_button();
            else if (buttonimpulse_count == 500)
                process_buttonhold();
        }
        else
        {
            //button released
            buttonimpulse_count = 0;
        }
    }

    TIM3_SR1 = 0b00000000;
}
