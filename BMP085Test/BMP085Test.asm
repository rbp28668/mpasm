;***********************************************************
; Barometer
; Barometer using a BMP085 sensor and LCD display
;***********************************************************

        LIST p=16F88 ;
#include p16f88.inc
		__CONFIG _CONFIG1, _INTRC_IO & _WDT_OFF & _PWRTE_OFF & _MCLR_ON & _BODEN_OFF & _LVP_OFF & _CPD_OFF & _WRT_PROTECT_OFF & _DEBUG_OFF & _CCP1_RB0 & _CP_OFF
        __CONFIG _CONFIG2, _FCMEN_OFF & _IESO_OFF

		radix decimal 


;=======================================================
; Constants
; Note - these need to be set to configure the hardware configuration
; of the PIC driving the LCD.
;=======================================================

_ClkIn			EQU		D'8000000'		; Processor clock frequency.


LCD_DATA         EQU     PORTA          ; Uses LS 4 bits of port - update 
LCD_DATA_TRIS    EQU     TRISA
LCD_CNTL         EQU     PORTB
LCD_DATA_MASK	 EQU	 0x0F			; which bits of the port are used for data bits

; LCD Display Commands and Control Signal names on PORTB
E				EQU		5				; LCD Enable control line
RW				EQU		3				; LCD Read/Write control line
RS				EQU		0				; LCD Register Select control line


;=======================================================
; Variables
;=======================================================
			

; Used for interrupt
isave		udata_shr
WSAVE		res 1		; for interrupts
STATSAVE	res 1		; for interrupts
INTCNT      res 1       ; counting interrupt
INTFLAG     res 1       ; for signalling - T2 interrupt sets bit 0 every 2 seconds

; Misc variables
            udata
temp        res 1       ; general temp 
msgtemp     res 1       ; writing messages
mt2         res 1       ; writing messages
lp1         res 1       ; General loop counter
lp2         res 1       ; General loop counter
secs        res 1       ; for timing seconds
mins        res 1       ; for timing minutes

; Used for BCD conversion
            udata_shr
COUNT       res 1
FLAG        res 1



;=======================================================
; Code
;=======================================================

RST		code   0;				; 
		goto Startup

;=======================================================
; Interrupt Handler
;=======================================================
ISR		code	4
		retfie


        ; BMP085 pressure sensor code.
        include "../BMP085/BMP085.inc"


        ; 32 bit Math routines also
        include "../math/math32.inc"

;=======================================================
; Main entry point
;=======================================================
PGM		    code					; wherever the linker wants to put this
Startup:

		    banksel OSCCON			; bank 1
		    MOVLW	B'01110000'		; Set 8MHz internal clock
		    MOVWF	OSCCON

		    ; Set inputs
		    bsf		STATUS,RP0		; Select bank 1 for trisB
     	    movlw	B'00000000'		; All port A inputs
		    movwf	TRISA
            MOVLW   B'11010010'     ; Bits 0, 3 and 5 outputs for controls.
                                ; Leave bits 1 and 4 inputs for I2C master
		    movwf   TRISB         

            BCF     STATUS, RP0     ; Bank 0
            CLRF    PORTA           ; ALL PORT output should output Low.
            CLRF    PORTB




            ; Initialise the BMP085
            call    BMP085Init

           NOP 
Mainlp:


            movfw   PORTA
            iorlw   B'01010000'
            movwf   PORTA

            call    BMP085getUT
            call    BMP085getUP

            ; Sim
            ;banksel UT
            ;MOVL32  UT,0x5c92
            ;MOVL32  UP,0x136b5

            call    BMP085getTemp
            call    BMP085getPressure

            banksel PORTA
            movfw   PORTA
            andlw   ~B'01010000'
            movwf   PORTA

            goto $

            end
