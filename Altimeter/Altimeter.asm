;***********************************************************
; Altimeter
; Altimeter using a BMP085 sensor and LCD display. 
; Based on the basic barometer implmentation
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

;#define SIMULATE 1

_ClkIn			EQU		D'8000000'		; Processor clock frequency.


LCD_DATA         EQU     PORTA          ; Uses LS 4 bits of port - update 
LCD_DATA_TRIS    EQU     TRISA
LCD_CNTL         EQU     PORTB
LCD_DATA_MASK	 EQU	 0x0F			; which bits of the port are used for data bits

; LCD Display Commands and Control Signal names on PORTB
E				EQU		5				; LCD Enable control line
RW				EQU		3				; LCD Read/Write control line
RS				EQU		0				; LCD Register Select control line


; Factor to read multiple times for signal averaging.
; 32 should easily sit within a second.
AVERAGE         EQU     32

; Damping for recursive smoothing of vario  (old * damping + new )/(damping + 1)
DAMPING         EQU     3

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
avgp        res 4       ; used for averaging pressure
qfe         res 4       ; start (airfield) pressure.
altm        res 4       ; altitude in m
prevPa      res 4       ; previous reading in Pa (for climb)
climb       res 4       ; damped rate of climb or descent (in Pa per tick)
climbmps    res 4       ; climb or descent in metres per sec.

; Used for BCD conversion
            udata_shr
COUNT       res 1
FLAG        res 1

ReadBMP     macro
            local rplp
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
            pagesel add32
            call    add32
            MOV32   avgp,REGA
            pagesel $
            
            banksel lp1
            decfsz  lp1,F
            goto    rplp

            ; Accumulated total still in REGA
            MOVL32  REGB,AVERAGE
            pagesel div32
            call    div32
            MOV32   Pa,REGA         ; averaged result.
            pagesel $

            endm


;=======================================================
; Code
;=======================================================

RST		code   0;	
        pagesel Startup			; 
		goto Startup

;=======================================================
; Interrupt Handler
;=======================================================
ISR		code	4
        MOVWF   WSAVE           ; Copy W to a temporary register
        SWAPF   STATUS,W        ; Swap Status Nibbles and move to W 
        MOVWF   STATSAVE        ; Copy STATUS to a temporary register
        movfw   PCLATH          ; Save PCLATH as we use GOTO and multiple pages.
        movwf   PCLHSAVE
        clrf    PCLATH

        BCF     STATUS, RP0     ; Force Bank 0
        BCF     STATUS, RP1     ; 

        ; Manage timer2 interrupt to count 125Hz ticks.
        btfss   PIR1,TMR2IF     ; Timer 2 interrupt (PIR1 is bank 0)
        goto    INTR_EXIT       ; skip if not timer 2
        bcf     PIR1,TMR2IF     ; clear interrupt

        decfsz  INTCNT,F
        goto    INTR_EXIT       ; done if not zero.
        
        movlw   125             ; 1 second at 125Hz
        movwf   INTCNT          ; re-initialise counter
        bsf     INTFLAG,0       ; set the flag.

INTR_EXIT:
        movfw   PCLHSAVE      ; restore PCLATH
        movwf   PCLATH
        SWAPF   STATSAVE,W    ; Pull Status back into W
        MOVWF   STATUS          ; Store it in status 
        SWAPF   WSAVE,F         ; Prepare W to be restored
        SWAPF   WSAVE,W         ; Return it, preserving Z bit in STATUS		
		retfie


        ; BMP085 pressure sensor code.
        include "../BMP085/BMP085.inc"

        ; Bring in LCD driving
        include "../lcd/lcd.inc"

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
            MOVLW   B'11010010'     ; Bits 0, 3 and 5 outputs for controls, bit 2 for lcd
                                    ; Leave bits 1 and 4 inputs for I2C master
		    movwf   TRISB         

            bcf     STATUS, RP0     ; Bank 0
            clrf    PORTA           ; ALL PORT output should output Low.
            clrf    PORTB

            ; DEBUG
            banksel PORTA
            bsf     PORTA,4

 
           ; Initialise the LCD
            movlw   0                ; all of port A to be output
            pagesel LCD_INIT
		    call    LCD_INIT
            pagesel $
 
            call    Hello           ; version message.
            
            banksel PORTA
            clrf    PORTA
            bsf     PORTA,6

             ; Initialise the BMP085
            banksel PORTB
            clrf    PORTB

#ifndef SIMULATE
            pagesel BMP085Init
            call    BMP085Init
            pagesel $
#endif

            banksel PORTA
            bsf     PORTA,4

            ; Initialise interrupt variables and timers
            movlw   250             ; 2 seconds at 125Hz
            movwf   INTCNT          ; re-initialise counter
            clrf    INTFLAG         ; set the flag.

        
            ; Initialise timer 2 to provide timebase
            ; with /16 & /10 prescaler/postscaler will give 125Hz timebase
            banksel PR2             ; PR2 is bank 1
            movlw   99              ; time from 0 to 99 - 100 cycles.
            movwf   PR2
            banksel TMR2      ; TMR2 is bank 0
            clrf    TMR2
            ; 8Mhz clock, Fosc/4 = 2000000
            ; /16 prescaler = 125000
            ; /10 postscaler & 100 cycle counter = 125 Hz
            movlw   B'01001111'      ; Internal clock, /10 postscaler, /16 prescaler, enabled.
            movwf   T2CON           ; bank 0 also
            ; Enable T2 interrupts
            banksel PIE1     ; PIE1 bank 1 (INTCON all banks)
            bsf     PIE1,TMR2IE
            bsf     INTCON,PEIE     ; set peripheral interrupt enable
            bsf     INTCON,GIE      ; set global interrupt enable
            bcf     STATUS,RP0


            banksel PORTA
            bsf     PORTA,6


            ; Read the initial pressure to establish a nominal QFE.
            call    RdBMP            
            ;ReadBMP
            MOV32   qfe, Pa
            MOV32   prevPa, Pa ; initial pressure for climb/descent

            CLRF32   climb       ; initial rate of climb 0

            ; Initial wait to allow you to read the logo message
            ; Wait for the 2 second timer to elapse
            pagesel $
            #ifndef SIMULATE
            bcf     INTFLAG,0
            btfss   INTFLAG,0
            goto    $-1
            #endif



Mainlp:
            banksel PORTA
            bsf     PORTA,6
            
            ; Read temp and pressure.
            pagesel RdBMP
            call    RdBMP
            ;ReadBMP

            banksel PORTA
            bcf     PORTA,6

            MOV32   REGB, Pa
            MOV32   REGA, qfe

            pagesel sub32
            call    sub32   ; qfe - Pa -> REGA, pressure difference in Pa

            ; Convert difference in Pressure to M.  30 feet per mB 
            ; or 30 feet per 100Pa or 0.3 feet per Pa or roughly 0.1m per Pa
            ; 22/256 is very close at 0.0859375  for conversion to m over 
            ; bottom 2000 feet of the atmosphere.

            MOVL32  REGB, 220 ; use 220 to give 0.lm resolution
            pagesel mul32
            call    mul32
            MOVL32  REGB, 256
            pagesel div32
            call    div32   
            MOV32   altm,REGA

            ; Now look at climb/descent
            MOV32   REGA, climb
            MOVL32  REGB, DAMPING
            pagesel mul32
            call    mul32       ; in REGA
            MOV32   climb, REGA

            MOV32   REGA, prevPa
            MOV32   REGB, Pa
            pagesel sub32
            call    sub32 ; prevPa - Pa,  if +ve pressure decreasing so up!
            
            MOV32   REGB, climb
            pagesel add32
            call    add32   ; REGA now has old * damping + new
            MOVL32  REGB, (DAMPING + 1)
            pagesel div32
            call    div32       
            
            MOV32   climb, REGA ; store.  Climb in Pa per tick.

            MOV32   prevPa, Pa  ; update for next time.

            MOVL32  REGB, 220   ; allow storing to 1 decimal.
            pagesel mul32
            call    mul32
            MOVL32  REGB, 256
            pagesel div32
            call    div32   
            MOV32   climbmps,REGA   ; climb/descent in metres per sec.

draw:
            banksel PORTA
            bsf     PORTA,4

            ; T and Pa should now have the calibrated temp and pressure.
            ; Pressure
 
            MOV32   REGA,Pa
            pagesel bin2dec32
            call    bin2dec32
 
            movlw   DD_RAM_UL
            pagesel SEND_CMD
            call    SEND_CMD
            pagesel WriteP
            call    WriteP            

            ; Temperature
            MOV32   REGA,T
            pagesel bin2dec32
            call    bin2dec32
 
            movlw   DD_RAM_UL+11 ; Digits 11 to 15: NN.NC
            pagesel SEND_CMD
            call    SEND_CMD
            pagesel WriteT
            call    WriteT

            ; Altitude m
            MOV32   REGA,altm
            pagesel bin2dec32
            call    bin2dec32

            movlw   DD_RAM_LL
            pagesel SEND_CMD
            call    SEND_CMD

            pagesel WriteAlt
            call    WriteAlt

            ; Climb/descent, with resolution of 0.1 ms
            banksel DSIGN
            clrf    DSIGN
            MOV32   REGA,climbmps
            pagesel bin2dec32
            call    bin2dec32

            movlw   DD_RAM_LL+11; 
            pagesel SEND_CMD
            call    SEND_CMD

            pagesel WriteClimb
            call    WriteClimb

            banksel PORTA
            bcf     PORTA,4

            
            ; Wait for the 1 second timer to elapse
            pagesel $
            #ifndef SIMULATE    ; don't bother waiting when simulating.
            bcf     INTFLAG,0
            btfss   INTFLAG,0
            goto    $-1
            #endif

            goto    Mainlp
            
;=======================================================
; RdBMP 
; Reads the BMP085 and leaves T and averaged Pa value.
;=======================================================
RdBMP

#ifdef      SIMULATE
            MOVL32   T,215
            MOVL32   Pa,101325
            return
#else
            CLRF32  avgp          ; initialise value for average.

            ; Read temperature once for the complete cycle.            
            pagesel BMP085getUT
            call    BMP085getUT

            banksel lp1
            movlw   AVERAGE  
            movwf    lp1

rplp:       
            pagesel BMP085getUP
            call    BMP085getUP
            pagesel BMP085getTemp
            call    BMP085getTemp
            pagesel BMP085getPressure
            call    BMP085getPressure
            MOV32   REGA, avgp
            MOV32   REGB, Pa
            pagesel add32
            call    add32
            MOV32   avgp,REGA
            
            pagesel rplp
            banksel lp1
            decfsz  lp1,F
            goto    rplp

            ; Accumulated total still in REGA
            MOVL32  REGB,AVERAGE
            pagesel div32
            call    div32
            MOV32   Pa,REGA         ; averaged result.
            pagesel $
            return
#endif
;=======================================================
; Writes a hello message.
;=======================================================
Hello       
            pagesel SEND_CMD
            movlw   DD_RAM_UL
            call    SEND_CMD
            movlw   'A'
            call    SEND_CHAR
            movlw   'l'
            call    SEND_CHAR
            movlw   't'
            call    SEND_CHAR
            movlw   ' '
            call    SEND_CHAR
            movlw   'v'
            call    SEND_CHAR
            movlw   '1'
            call    SEND_CHAR
            movlw   '.'
            call    SEND_CHAR
            movlw   '0'
            call    SEND_CHAR
            pagesel $
            return

;=======================================================
; Writes the pressure as hPa with decimal point and leading
; zero suppression.
;=======================================================
WriteP       
            pagesel SEND_CHAR
            movlw   DIGIT5      ; MS digit for pressure
            movwf   FSR
            bankisel DIGITS
            ; First digit - may be leading zero             
            movf    INDF,W      ; Get digit and test for zero (sets Z)
            movlw   ' '         ; replace leading zeros with space.
            btfss   STATUS,Z    ; if was zero, skip and leave space
            movlw   '0'
            addwf   INDF,W      ; convert bcd to ascii digit
            call    SEND_CHAR
            incf    FSR,F       ; next digit

            movlw   '0'
            addwf   INDF,W      ; convert bcd to ascii digit
            call    SEND_CHAR
            incf    FSR,F       ; next digit

            movlw   '0'
            addwf   INDF,W      ; convert bcd to ascii digit
            call    SEND_CHAR
            incf    FSR,F       ; next digit

            movlw   '0'
            addwf   INDF,W      ; convert bcd to ascii digit
            call    SEND_CHAR
            incf    FSR,F       ; next digit

            movlw   '.'
            call    SEND_CHAR

            movlw   '0'
            addwf   INDF,W      ; convert bcd to ascii digit
            call    SEND_CHAR
            incf    FSR,F       ; next digit

            movlw   '0'
            addwf   INDF,W      ; convert bcd to ascii digit
            call    SEND_CHAR
            incf    FSR,F       ; next digit

            ; units text (hPa)
            movlw   'h'
            call    SEND_CHAR
            movlw   'P'
            call    SEND_CHAR
            movlw   'a'
            call    SEND_CHAR

            pagesel $
            return

;=======================================================
; Writes the temp in C with decimal point
;=======================================================
WriteT       
            pagesel SEND_CHAR
            movlw   DIGIT8      ; MS digit for pressure
            movwf   FSR
            bankisel DIGITS

            movlw   '0'
            addwf   INDF,W      ; convert bcd to ascii digit
            call    SEND_CHAR
            incf    FSR,F       ; next digit

            movlw   '0'
            addwf   INDF,W      ; convert bcd to ascii digit
            call    SEND_CHAR
            incf    FSR,F       ; next digit

            movlw   '.'
            call    SEND_CHAR

            movlw   '0'
            addwf   INDF,W      ; convert bcd to ascii digit
            call    SEND_CHAR
            incf    FSR,F       ; next digit

            ; units text (C)
            movlw   'C'
            call    SEND_CHAR
            pagesel $
            return

;=======================================================
; WriteAlt
; Writes altitude to nearest meter
;=======================================================
WriteAlt
            pagesel SEND_CHAR
            movlw   DIGIT6      ; MS digit for alt (5 digits)
            movwf   FSR
            bankisel DIGITS

            movlw   '0'
            addwf   INDF,W      ; convert bcd to ascii digit
            call    SEND_CHAR
            incf    FSR,F       ; next digit

            movlw   '0'
            addwf   INDF,W      ; convert bcd to ascii digit
            call    SEND_CHAR
            incf    FSR,F       ; next digit

            movlw   '0'
            addwf   INDF,W      ; convert bcd to ascii digit
            call    SEND_CHAR
            incf    FSR,F       ; next digit

            movlw   '0'
            addwf   INDF,W      ; convert bcd to ascii digit
            call    SEND_CHAR
            incf    FSR,F       ; next digit

            movlw   '.'
            call    SEND_CHAR

            movlw   '0'
            addwf   INDF,W      ; convert bcd to ascii digit
            call    SEND_CHAR
            incf    FSR,F       ; next digit


            pagesel $
            return

;=======================================================
; WriteClimb
; Writes the rate of climb/descent to nearest 0.1ms
;=======================================================
WriteClimb
            pagesel SEND_CHAR
            
            banksel DSIGN
            ; TODO - stuff in sign DSIGN set to 1 if -ve.
            movlw   '+'
            btfsc   DSIGN,0
            movlw   '-'
            call    SEND_CHAR

            movlw   DIGIT8      ; MS digit for alt (3 digits)
            movwf   FSR
            bankisel DIGITS

            movlw   '0'
            addwf   INDF,W      ; convert bcd to ascii digit
            call    SEND_CHAR
            incf    FSR,F       ; next digit

            movlw   '0'
            addwf   INDF,W      ; convert bcd to ascii digit
            call    SEND_CHAR
            incf    FSR,F       ; next digit

            movlw   '.'
            call    SEND_CHAR

            movlw   '0'
            addwf   INDF,W      ; convert bcd to ascii digit
            call    SEND_CHAR
            incf    FSR,F       ; next digit

            pagesel $
            return


            end
