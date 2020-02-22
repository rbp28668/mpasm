;******************************************************
;Test code for PIC16F818 to test programming
;with PICkit2 and eventually I2C
;******************************************************

#include p16f818.inc
		__CONFIG _BODEN_ON & _CP_OFF & _PWRTE_ON & _WDT_OFF & _LVP_OFF & _INTRC_IO

; Bit numbers of LEDs numbered left to right.
LED1	EQU	3
LED2	EQU	2
LED3	EQU 0
LED4	EQU 5

		org   0;
		goto Startup
;		
Startup:
		bsf		STATUS,RP0		; Select bank 1 for trisB
     	clrf      TRISB          ; Make PortB all output
		bcf		STATUS,RP0
	
Loop:	
		bcf		PORTB,LED1
		bsf		PORTB,LED2
		bcf		PORTB,LED3
		bsf		PORTB,LED4

		bsf		PORTB,LED1
		bcf		PORTB,LED2
		bsf		PORTB,LED3
		bcf		PORTB,LED4

		goto 	Loop

		end