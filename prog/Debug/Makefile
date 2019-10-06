###############################################################################
# Makefile for project 
###############################################################################

## General options
PROJECT = 
MCU = ATtiny13
COMPILER = "AVRASM (Proteus)"
TARGET = Debug
CLOCK = 9600000
SHELL = C:\Windows\system32\cmd.exe

## Tools general options
ASMFLAGS=-i"D:\Program Files\Labcenter Electronics\Proteus 7 Professional\tools\AVRASM\appnotes\tn13def.inc" -fI

## Processing Tools
ASM=avrasm2.exe
DDX=avrddx.exe
#  C:/rf/prog/Debug/Debug.hex: TOOL=ASM
#  Post: $(DDX) $(DDXFLAGS) Debug.lst
#  Post: "C:\Program Files\Labcenter Electronics\VSM Studio for Proteus 7\BIN\mv" "Debug.SDI" "Debug/Debug.SDI"
#     C:/rf/prog/1.asm

# Build tree:
all:	Debug

Debug:	Debug/Debug.hex

Debug/Debug.hex:	1.asm 
	$(ASM) $(ASMFLAGS)  -o "Debug\Debug.hex" -l Debug.lst "1.asm"
	$(DDX) $(DDXFLAGS) Debug.lst
	"C:\Program Files\Labcenter Electronics\VSM Studio for Proteus 7\BIN\mv" "Debug.SDI" "Debug/Debug.SDI"

# tidy - delete all temporary files which are not involved in the target generation
tidy:
	rm -rf Debug/Debug.SDI
	rm -rf Debug.lst

# cleanup - delete all generated files
clean:	tidy
	rm -rf Debug/Debug.hex
