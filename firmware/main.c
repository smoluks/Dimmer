#include <iostm8l.h>

int main(void)
{
    PB_DDR = 0b00100000;
    PB_CR1 = 0b10100000;
    PB_ODR = 0b00000000;
    while(1)
    {
    }
}