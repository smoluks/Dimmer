#include <stdint.h>
#include <stdbool.h>

bool programming_mode = false;
bool channel1enable = false;
bool channel2enable = false;
bool sleep_enable = false;
uint16_t sleepcounter = 0;

void process_rf(uint8_t buttonflags)
{
    if (buttonflags & BUTTON_A)
    {
        channel1enable = !channel1enable;
    }

    if (buttonflags & BUTTON_B)
    {
        channel2enable = !channel2enable;
    }

    if (buttonflags & BUTTON_ONOFF)
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

    if (buttonflags & BUTTON_SLEEP)
    {
        sleepcounter = 0;
        sleep_enable = true;
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
    programming_mode = true;
}

void tick(void)
{
    if (sleep_enable && (++sleepcounter = 6000))
    {
        channel1enable = false;
        channel2enable = false;
        sleep_enable = false;
    }
}