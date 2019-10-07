#define __rim() _asm("RIM\n")
#define __wfi() _asm("WFI\n")
#define __halt() _asm("HALT\n")

#define PINB1_SET() _asm("bset 0x500f, #1")
#define PINB1_RESET() _asm("bres 0x500f, #1")
#define PINB2_SET() _asm("bset 0x500f, #2")
#define PINB2_RESET() _asm("bres 0x500f, #2")
