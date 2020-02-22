;
; Test program standard 2 line by 16 char LCD display in 4-bit mode.
;
	LIST P=16F818
#include p16f818.inc
		__CONFIG _BODEN_ON & _CP_OFF & _PWRTE_ON & _WDT_OFF & _LVP_OFF & _INTRC_IO


;=======================================================
; Constants
; Note - these need to be set to configure the hardware configuration
; of the PIC driving the LCD.
;=======================================================



; variables 
            udata
temp        res 1
msgtemp     res 1
mt2         res 1

       		code     0                 ; RESET vector location
RESET       GOTO    START              ;
;
; This is the Periperal Interrupt routine. Should NOT get here
;
isr        	code     4              ; Interrupt vector location
			retfie




; Main start location.
START                               ; POWER_ON Reset (Beginning of program)
            CLRF    STATUS          ; Do initialization (Bank 0)
            CLRF    INTCON
            CLRF    PIR1

			banksel OSCCON
			MOVLW	B'01110000'		; Set 8MHz internal clock (01110000 for 8MHz)
			MOVWF	OSCCON

            BSF     STATUS, RP0     ; Select Bank 1
            CLRF    TRISA           ; RA5 -  0 outputs
            MOVLW   B'11010010'     ; Bits 0, 3 and 5 outputs for controls.
                                    ; Leave bits 1 and 4 inputs for I2C master
            MOVWF   TRISB           ; RB7 - 4 inputs, RB3 - 0 outputs 
            BCF     STATUS, RP0     ; Select Bank 0

            call    SendMessage

    		GOTO	$

SendMessage:
            banksel msgtemp
            clrf    msgtemp         ; start at 0 offset
Sendlp:                
            movlw HIGH Message ;load high 8-bit address of Table
            movwf PCLATH
            
            movfw   msgtemp         ; get current offset
            call    Message         ; convert to char
            iorlw    0x00           ; test W for zero without modifying it    
            btfsc    STATUS,Z       ; 
            return 
            movwf   PORTB           ; send W
            incf    msgtemp,F       ; next char.
            goto    Sendlp

tables      code 0x200               ; top 256 bytes for tables given debug exec at 0x300
Message:
            addwf PCL,F ;add offset to pc to generate a computed goto
            retlw 'H' 
            retlw 'e' 
            retlw 'l' 
            retlw 'l' 
            retlw 'o' 
            retlw ' ' 
            retlw 'W' 
            retlw 'o' 
            retlw 'r' 
            retlw 'l' 
            retlw 'd' 
            retlw 0 




            end




