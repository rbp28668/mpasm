# MPLAB IDE generated this makefile for use with GNU make.
# Project: SRF08.mcp
# Date: Sat Jun 29 13:11:17 2013

AS = MPASMWIN.exe
CC = 
LD = mplink.exe
AR = mplib.exe
RM = rm

SRF08.cof : SRF08.o
	$(CC) /p16F818 "SRF08.o" /z__MPLAB_BUILD=1 /o"SRF08.cof" /M"SRF08.map" /W

SRF08.o : SRF08.ASM ../../../Program\ Files\ (x86)/Microchip/MPASM\ Suite/p16f818.inc ../I2CMaster/I2C.H ../I2CMaster/i2c_low.inc ../lcd/lcd.inc ../math/math.inc
	$(AS) /q /p16F818 "SRF08.ASM" /l"SRF08.lst" /e"SRF08.err" /o"SRF08.o"

clean : 
	$(CC) "SRF08.o" "SRF08.err" "SRF08.lst" "SRF08.cof" "SRF08.hex"

