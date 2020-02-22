;***********************************************************
; Barometer
; Barometer using a BMP085 sensor and LCD display
;***********************************************************

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




; Factor to read multiple times for signal averaging.
AVERAGE         EQU     128

;=======================================================
; Variables
;=======================================================
			

; Used for interrupt
isave		udata_shr
WSAVE		res 1		; for interrupts
STATSAVE	res 1		; for interrupts
PCLHSAVE    res 1       ; for interrupts
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
avgp        res 4       ; used for averaging pressure

; Used for BCD conversion
            udata_shr
COUNT       res 1
FLAG        res 1




;=======================================================
; Code
;=======================================================

RST		code   0;				; 
		goto Startup


        ; MAX7219 LED display
        extern INIT_MAX7219
        extern ADDR_MAX7219
        extern WRITE_MAX7219

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
     	    movlw	B'00000000'		; All port A outputs
		    movwf	TRISA
            MOVLW   B'11010010'     ; Bits 0, 3 and 5 outputs for controls.
                                ; Leave bits 1 and 4 inputs for I2C master
		    movwf   TRISB         

            BCF     STATUS, RP0     ; Bank 0
            CLRF    PORTA           ; ALL PORT output should output Low.
            CLRF    PORTB

            ; Initialise the display
            pagesel INIT_MAX7219
		    CALL	INIT_MAX7219
            

            ; Initialise the BMP085
            pagesel BMP085Init
            call    BMP085Init
            pagesel $


Mainlp:
            banksel PORTA
            bsf     PORTA,6
            
            CLRF32  avgp          ; initialise value for average.
            
            pagesel BMP085getUT
            
            call    BMP085getUT
            call    BMP085getTemp

            banksel lp1
            movlw   AVERAGE  
            movwf    lp1

rplp:       
            pagesel BMP085getUP
            call    BMP085getUP
            call    BMP085getPressure
            MOV32   REGA, avgp
            MOV32   REGB, Pa
            call    add32
            MOV32   avgp,REGA
            pagesel $
            
            banksel lp1
            decfsz  lp1,F
            goto    rplp

            ; Accumulated total still in REGA
            pagesel div32
            MOVL32  REGB,AVERAGE
            call    div32
            MOV32   Pa,REGA         ; averaged result.
            pagesel $

            banksel PORTA
            bcf     PORTA,6

            ; T and Pa should now have the calibrated temp and pressure.
            ; Pressure
            MOV32   REGA,Pa
            pagesel bin2dec32
            call    bin2dec32
            pagesel $

            call    WriteP            

            goto    Mainlp
            



;=======================================================
; Writes the pressure as hPa with decimal point and leading
; zero suppression.
;=======================================================
WriteP      
            pagesel ADDR_MAX7219
            movlw   8           ; Address of ms digit.
            call    ADDR_MAX7219
 
            movlw   DIGIT5      ; MS digit for pressure
            movwf   FSR
            bankisel DIGITS
            ; First digit - may be leading zero             
            movf    INDF,W      ; Get digit and test for zero (sets Z)
            btfsc   STATUS,Z    ; if was not zero, skip and leave digit
            movlw   15          ; space code
            call    WRITE_MAX7219
            incf    FSR,F       ; next digit

            movf    INDF,W      ; get bcd digit
            call    WRITE_MAX7219
            incf    FSR,F       ; next digit

            movf    INDF,W      ; get bcd digit
            call    WRITE_MAX7219
            incf    FSR,F       ; next digit

            movlw   128         ; set decimal point
            addwf   INDF,W      ; get bcd digit
            call    WRITE_MAX7219
            incf    FSR,F       ; next digit

            movf    INDF,W      ; get bcd digit
            call    WRITE_MAX7219
            incf    FSR,F       ; next digit

            movf    INDF,W      ; get bcd digit
            call    WRITE_MAX7219
            incf    FSR,F       ; next digit

            ; Blank last 2 digits
            movlw   0xF
            call    WRITE_MAX7219
            movlw   0xF
            call    WRITE_MAX7219

            pagesel $
            return


            end
