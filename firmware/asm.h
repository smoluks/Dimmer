#define __rim() _asm("RIM\n")
#define __wfi() _asm("WFI\n")
#define __halt() _asm("HALT\n")

#define CH1_EN() _asm("bres 0x500A, #4")
#define CH1_DIS() _asm("bset 0x500A, #4")
#define CH2_EN() _asm("bres 0x5005, #6")
#define CH2_DIS() _asm("bset 0x5005, #6")
