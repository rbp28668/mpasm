;====================================================================
; IRTX.ASM
; Author Bruce Porteous
; Infra-red Transmitter code for PIC16F27. 
; Produces a 38kHz square wave to drive an infra-red LED
; Note - timings assume a 12MHz crystal.
;====================================================================
	LIST   P=PIC16F627
	#include P16F627.INC
	
	__CONFIG        _BODEN_ON & _CP_OFF & _DATA_CP_OFF & _PWRTE_ON & _WDT_OFF & _LVP_OFF & _MCLRE_ON & _HS_OSC
		
				radix dec
				
;INPUTS - none

;OUTPUTS
IRLED   		EQU		H'02'		; Infra-red LED on RA2

;Variables
				cblock 0x20
				TICKS				; counter for delay.
				endc

; Constants
				cblock 0
				endc

;	   **********************************
;      Start of program execution
;	   **********************************
				ORG	0				;Reset vector address
;				GOTO	RESET		;goto RESET routine when boot.



;	   **********************************
;      **  RESET :  main boot routine  **
;      **********************************

RESET			MOVLW	B'00000111'	;Disable Comparator module's
				MOVWF	CMCON
				
				BSF	STATUS,RP0	;Switch to register bank 1
					;Disable pull-ups
					;INT on rising edge
					;TMR0 to CLKOUT
					;TMR0 Incr low2high trans.
					;Prescaler assign to Timer0
					;Prescaler rate is 1:64
				MOVLW	B'11010101'	;Set PIC options (See datasheet).
				MOVWF	OPTION_REG	;Write the OPTION register.
		
				CLRF	INTCON		;Disable interrupts
				MOVLW	B'11111111' ;All RB ports are inputs
				MOVWF	TRISB		;RB7 & RB6 are inputs.
									;RB5...RB0 are outputs.
				MOVLW	B'11111011'	;all RA ports are inputs except RA2
				MOVWF	TRISA
				BCF	STATUS,RP0		;Switch Back to reg. Bank 0
				CLRF	PORTB

; Generate 38kHz signal.  Using a 12MHz crystal - that's 3MHz machine cycle. 79 machine cycles
; per output period gives 37.975kHz 0.07% error.  The following code should produce 79 cycles
; 39 ON and 40 OFF
Loop			bsf		PORTA,IRLED ; 1 First 1/2 of cycle	
				
				movlw   11			; 1
                movwf   TICKS       ; 1
hloop			decfsz  TICKS,1     ; 1, 2 if 0
                goto    hloop       ; 2

				nop					; 1
				nop					; 1
                nop					; 1
                nop                 ; 1

				bcf		PORTA,IRLED ; 1


				movlw   11			; 1
                movwf   TICKS       ; 1
lloop			decfsz  TICKS,1     ; 1, 2 if 0 
                goto    lloop       ; 2

				nop					; 1
                nop					; 1
                nop                 ; 1

				goto	Loop		; 2	
	
				

				end