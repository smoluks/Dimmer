#include <stdint.h>
#include <stdbool.h>

void animation_sleep(uint16_t step);
void animation_prgmode(uint16_t step);
void animation_prgmodeend(uint16_t step);

bool programming_mode = false;
bool channel1enable = false;
bool channel2enable = false;

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
    programming_mode = true;
    set_animation(animation_prgmode);
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
        channel1enable = false;
        channel2enable = false;
        break;
    case 100:
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
    case 200:
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
        animation_enable = false;
        break;
    }
}
