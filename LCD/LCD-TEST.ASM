;
; Test program standard 2 line by 16 char LCD display in 4-bit mode.
;
	LIST P=16F88
#include p16f88.inc
; 16F818		__CONFIG _BODEN_ON & _CP_OFF & _PWRTE_ON & _WDT_OFF & _LVP_OFF & _INTRC_IO

		__CONFIG _CONFIG1, _INTRC_IO & _WDT_OFF & _PWRTE_OFF & _MCLR_ON & _BODEN_OFF & _LVP_OFF & _CPD_OFF & _WRT_PROTECT_OFF & _DEBUG_OFF & _CCP1_RB0 & _CP_OFF
        __CONFIG _CONFIG2, _FCMEN_OFF & _IESO_OFF


;=======================================================
; Constants
; Note - these need to be set to configure the hardware configuration
; of the PIC driving the LCD.
;=======================================================

_ClkIn			EQU		D'8000000'		; Processor clock frequency.


LCD_DATA         EQU     PORTA          ; Uses LS 3 bits of port - update 
LCD_DATA_TRIS    EQU     TRISA
LCD_CNTL         EQU     PORTB
LCD_DATA_MASK	 EQU	 0x0F			; which bits of the port are used for data bits

; LCD Display Commands and Control Signal names on PORTB
E				EQU		5				; LCD Enable control line
RW				EQU		3				; LCD Read/Write control line
RS				EQU		0				; LCD Register Select control line
;


; variables 
            udata
temp        res 1
msgtemp     res 1
mt2         res 1
lp1         res 1
lp2         res 1

       		code     0                 ; RESET vector location
RESET       GOTO    START              ;
;
; This is the Periperal Interrupt routine. Should NOT get here
;
isr        	code     4              ; Interrupt vector location
			retfie


            ; Bring in LCD driving
            include "../lcd/lcd.inc"

SendLCD     macro Message
            local Sendlp, SendEnd
            
            banksel msgtemp
            clrf    msgtemp         ; start at 0 offset
Sendlp:     
            movlw   HIGH Message ;load high 8-bit address of Table
            movwf   PCLATH
            movfw   msgtemp         ; get current offset
            call    Message         ; convert to char
            iorlw    0x00           ; test W for zero without modifying it    
            btfsc    STATUS,Z       ; 
            goto    SendEnd 
            call    SEND_CHAR       ; send W
            incf    msgtemp,F       ; next char.
            goto    Sendlp
SendEnd:
            endm

; Main start location.
START                               ; POWER_ON Reset (Beginning of program)
            CLRF    STATUS          ; Do initialization (Bank 0)
            CLRF    INTCON
            CLRF    PIR1

			banksel OSCCON
			MOVLW	B'01110000'		; Set 8MHz internal clock (01110000 for 8MHz)
			MOVWF	OSCCON

            BSF     STATUS, RP0     ; Bank 1
            MOVLW   0x00            ; The LCD module does not like to work w/ weak pull-ups
            MOVWF   OPTION_REG      ;
            CLRF    PIE1            ; Disable all peripheral interrupts
            MOVLW   0xFF            ;
            MOVWF   ADCON1          ; Port A is Digital.
;
;
            BCF     STATUS, RP0     ; Bank 0
            CLRF    PORTA           ; ALL PORT output should output Low.
            CLRF    PORTB


            BCF     T1CON, TMR1ON   ; Timer 1 is NOT incrementing
;
            BSF     STATUS, RP0     ; Select Bank 1
            CLRF    TRISA           ; RA5 -  0 outputs
            MOVLW   B'11010010'     ; Bits 0, 3 and 5 outputs for controls.
                                    ; Leave bits 1 and 4 inputs for I2C master
            MOVWF   TRISB           ; RB7 - 4 inputs, RB3 - 0 outputs 
            BCF     STATUS, RP0     ; Select Bank 0


             ; Initialise the LCD
            movlw   0               ; all of port A to be output
			CALL	LCD_INIT

            movlw   CLR_DISP
            call    SEND_CMD

            movlw   DISP_ON
            call    SEND_CMD

            movlw   DD_RAM_UL
            call    SEND_CMD

            movlw   '0'
            call    SEND_CHAR
            movlw   '1'
            call    SEND_CHAR
            movlw   '2'
            call    SEND_CHAR
            movlw   '3'
            call    SEND_CHAR
            movlw   '4'
            call    SEND_CHAR
            movlw   '5'
            call    SEND_CHAR
            movlw   '6'
            call    SEND_CHAR
            movlw   '7'
            call    SEND_CHAR
            movlw   '8'
            call    SEND_CHAR
            movlw   '9'
            call    SEND_CHAR
            movlw   'A'
            call    SEND_CHAR
            movlw   'B'
            call    SEND_CHAR
            movlw   'C'
            call    SEND_CHAR
            movlw   'D'
            call    SEND_CHAR
            movlw   'E'
            call    SEND_CHAR
            movlw   'F'
            call    SEND_CHAR

            GOTO $

            SendLCD Message;

        
            call    InitGraph
            call    SetLines;

            bsf     PORTA, 4        ; light LED.

    		GOTO	$

; SetLines initialises the CG memory to give characters
; 0-7 a bar per character with it moving up as the 
; character code increases. Hence for a simple graph
; use characters 0-7 to plot values in the range 0-7
SetLines:
            movlw   CG_RAM
            call    SEND_CMD

            movlw   8
            movwf   lp1     ; outer loop counter

outer:
            movlw   8
            movwf   lp2
inner:
            movfw   lp1
            sublw   9
            xorwf   lp2,W   ; set zero flag if equal
            
            movlw   0x00
            btfsc   STATUS,Z
            movlw   0x01F

            call SEND_CHAR

            decfsz  lp2
            goto    inner

            decfsz  lp1
            goto    outer
            return

InitGraph:  
            movlw   DD_RAM_LL
            call    SEND_CMD

            clrf    temp
            movlw   8
            movwf   lp1

iglp:       movfw   temp
            call    SEND_CHAR 

            incf    temp,F
            decfsz  lp1
            goto iglp
            return



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

Arrows:
            addwf PCL,F ;add offset to pc to generate a computed goto
            retlw b'00000001'
            retlw b'00000011'
            retlw b'00001111'
            retlw b'00011111'
            retlw b'00011111'
            retlw b'00001111'
            retlw b'00000011'
            retlw b'00000001'

            retlw b'00010000'
            retlw b'00011000'
            retlw b'00011100'
            retlw b'00011111'
            retlw b'00011111'
            retlw b'00011100'
            retlw b'00011000'
            retlw b'00010000'

            retlw b'00000100'
            retlw b'00000100'
            retlw b'00001110'
            retlw b'00001110'
            retlw b'00001110'
            retlw b'00011111'
            retlw b'00011111'
            retlw b'00011111'

            retlw b'00011111'
            retlw b'00011111'
            retlw b'00011111'
            retlw b'00001110'
            retlw b'00001110'
            retlw b'00001110'
            retlw b'00000100'
            retlw b'00000100'

            retlw 0
	        
            end




