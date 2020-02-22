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


LCD_DATA         EQU     PORTA          ; Uses LS 4 bits of port - update 
LCD_DATA_TRIS    EQU     TRISA
LCD_CNTL         EQU     PORTB
LCD_DATA_MASK	 EQU	 0x0F			; which bits of the port are used for data bits

; LCD Display Commands and Control Signal names on PORTB
E				EQU		5				; LCD Enable control line
RW				EQU		3				; LCD Read/Write control line
RS				EQU		0				; LCD Register Select control line

; Air pressure in Pascals (1hPa = 100Pa)
MSL_LOW         EQU     98000
MSL_HIGH        EQU     105000

; Factor to read multiple times for signal averaging.
; Note that 100 pretty much fills the 2s timer tick with measurement.
; 64 if LCD delay used rather than busy
AVERAGE         EQU     64

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

;#define RAMP 1  ; intialise history with a ramp for debugging.

; History of the last 16 or 24 hourse
history     udata
#ifndef     RAMP
history     res 4*16    ; last 16 hours, earliest is at lowest address
#else
history     res 4
h1          res 4
h2          res 4
h3          res 4
h4          res 4
h5          res 4
h6          res 4
h7          res 4
h8          res 4
h9          res 4
h10         res 4
h11         res 4
h12         res 4
h13         res 4
h14         res 4
h15         res 4
#endif



;=======================================================
; Code
;=======================================================

RST		code   0;				; 
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
        
        movlw   250             ; 2 seconds at 125Hz
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
            MOVLW   B'11010010'     ; Bits 0, 3 and 5 outputs for controls.
                                ; Leave bits 1 and 4 inputs for I2C master
		    movwf   TRISB         

            BCF     STATUS, RP0     ; Bank 0
            CLRF    PORTA           ; ALL PORT output should output Low.
            CLRF    PORTB

            ; DEBUG
            banksel PORTA
            bsf     PORTA,4

            ; Initialise the LCD
            bcf     PORTB,2          ; ground contrast signal.
            movlw   0               ; all of port A to be output
		    CALL	LCD_INIT

            movlw   CLR_DISP
            call    SEND_CMD

            movlw   DISP_ON
            call    SEND_CMD

            call    Hello           ; version message.

            banksel PORTA
            movfw   PORTA
            iorlw   B'01010000'
            movwf   PORTA

            call    SetLines        ; define chars for pressure graph.

            ;Initialise history to zero
            banksel lp1
            movlw   16*4            ; 16 x 4 byte history
            movwf   lp1

            bankisel history
            movlw   history
            movwf   FSR
ihlp:        
            movlw   0
            movwf   INDF
            incf    FSR,F
            decfsz  lp1,F
            goto    ihlp

            ; DEBUG - initialise history with a ramp.
            #ifdef RAMP
            MOVL32  history, (MSL_LOW + ((MSL_HIGH-MSL_LOW)*0)/16)
            MOVL32  h1, (MSL_LOW + ((MSL_HIGH-MSL_LOW)*1)/16)
            MOVL32  h2, (MSL_LOW + ((MSL_HIGH-MSL_LOW)*2)/16)
            MOVL32  h3, (MSL_LOW + ((MSL_HIGH-MSL_LOW)*3)/16)
            MOVL32  h4, (MSL_LOW + ((MSL_HIGH-MSL_LOW)*4)/16)
            MOVL32  h5, (MSL_LOW + ((MSL_HIGH-MSL_LOW)*5)/16)
            MOVL32  h6, (MSL_LOW + ((MSL_HIGH-MSL_LOW)*6)/16)
            MOVL32  h7, (MSL_LOW + ((MSL_HIGH-MSL_LOW)*7)/16)
            MOVL32  h8, (MSL_LOW + ((MSL_HIGH-MSL_LOW)*8)/16)
            MOVL32  h9, (MSL_LOW + ((MSL_HIGH-MSL_LOW)*9)/16)
            MOVL32  h10, (MSL_LOW + ((MSL_HIGH-MSL_LOW)*10)/16)
            MOVL32  h11, (MSL_LOW + ((MSL_HIGH-MSL_LOW)*11)/16)
            MOVL32  h12, (MSL_LOW + ((MSL_HIGH-MSL_LOW)*12)/16)
            MOVL32  h13, (MSL_LOW + ((MSL_HIGH-MSL_LOW)*13)/16)
            MOVL32  h14, (MSL_LOW + ((MSL_HIGH-MSL_LOW)*14)/16)
            MOVL32  h15, (MSL_LOW + ((MSL_HIGH-MSL_LOW)*15)/16)
            #endif
            
             ; Initialise the BMP085
            pagesel BMP085Init
            call    BMP085Init
            pagesel $

            ; Initialise interrupt variables and timers
            movlw   250             ; 2 seconds at 125Hz
            movwf   INTCNT          ; re-initialise counter
            clrf    INTFLAG         ; set the flag.

            banksel secs
            movlw   30              ; 1 minute of 2 second ticks
            movwf   secs
            movlw   90              ; an hour and a half of minutes
            movwf   mins
        
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
            
            ; Initial wait to allow you to read the logo message
            ; Wait for the 2 second timer to elapse
            bcf     INTFLAG,0
            btfss   INTFLAG,0
            goto    $-1

            ; Wait for the 2 second timer to elapse
            bcf     INTFLAG,0
            btfss   INTFLAG,0
            goto    $-1

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

            banksel PORTA
            bcf     PORTA,6

            ; T and Pa should now have the calibrated temp and pressure.
            ; Pressure
            MOV32   REGA,Pa
            pagesel bin2dec32
            call    bin2dec32
            pagesel $

            movlw   DD_RAM_UL
            call    SEND_CMD
            call    WriteP            

            ; Temperature
            MOV32   REGA,T
            pagesel bin2dec32
            call    bin2dec32
            pagesel $


            movlw   DD_RAM_UL+11 ; Digits 11 to 15: NN.NC
            call    SEND_CMD
            call    WriteT

            call    drawTrend   ; draw trend line

            ; Wait for the 2 second timer to elapse
            bcf     INTFLAG,0
            btfss   INTFLAG,0
            goto    $-1

            banksel secs
            decfsz  secs,F
            goto    Mainlp  ; not timed out, normal loop
            movlw   30      ; reload seconds counter (multiples of 2 seconds)
            movwf   secs

            decfsz  mins,F
            goto    Mainlp
            
            movlw   90      ; time for 90 mins then 16 entries is 24 hours
            movwf   mins
            
            ; Ok, so if we get here then we have reached a 90 minute tick so update history.

            ; Shift history down, starting at low address (oldest)
            banksel lp1
            bankisel history
            movlw   15*4            ; 16 x 4 byte history, shift 15 of these down overwriting oldest
            movwf   lp1
            movlw   history
            movwf   FSR
shiftlp:       
            incf    FSR,F 
            incf    FSR,F 
            incf    FSR,F 
            incf    FSR,F 
            movfw   INDF

            decf    FSR,F
            decf    FSR,F
            decf    FSR,F
            decf    FSR,F
            movwf   INDF

            incf    FSR,F
            decfsz  lp1,F
            goto    shiftlp

            ; Now save the last pressure reading
            MOV32   (history + 4 * 15),Pa

            goto Mainlp

;=======================================================
; Writes a hello message.
;=======================================================
Hello       movlw   DD_RAM_UL
            call    SEND_CMD
            movlw   'B'
            call    SEND_CHAR
            movlw   'a'
            call    SEND_CHAR
            movlw   'r'
            call    SEND_CHAR
            movlw   'o'
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
            return

;=======================================================
; Writes an unsigned BCD number in DIGITS to LCD with leading
; zero suppression.
;=======================================================
#if 0 ; replaced
WriteBCD    movlw   DIGIT1      ; MS digit
            movwf   FSR
            
            movlw   10          ; 10 digits
            movwf   COUNT
            
            clrf    FLAG        ; will set bit 0 when we hit a non-zero digit

WBCDLoop    movf    INDF,W      ; Get digit and test z
            btfss   STATUS,Z
            goto    NotZero
            movlw   ' '         ; replace leading zeros with space.
            btfsc   FLAG,0
            movlw   '0'
            goto    WBCDPt
NotZero
            bsf     FLAG,0      ; set flag we've seen a non-zero digit
            movlw   '0'
            addwf   INDF,W      ; convert bcd to ascii digit
WBCDPt      call    SEND_CHAR

            incf    FSR,F       ; next digit
            decfsz  COUNT,F
            goto    WBCDLoop 
            return
#endif
;=======================================================
; Writes the pressure as hPa with decimal point and leading
; zero suppression.
;=======================================================
WriteP       
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

            return

;=======================================================
; Writes the temp in C with decimal point
;=======================================================
WriteT       
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

            return


; SetLines initialises the CG memory to give characters
; 0-7 a bar per character with it moving up as the 
; character code increases. Hence for a simple graph
; use characters 0-7 to plot values in the range 0-7
SetLines:
            movlw   CG_RAM
            call    SEND_CMD

            banksel lp1
            movlw   8
            movwf   lp1     ; outer loop counter - do for 8 characters.

slouter:
            movlw   8
            movwf   lp2     ; inner loop counter - 8 bytes per char.
slinner:
            movfw   lp1
            xorwf   lp2,W   ; set zero flag if equal
            
            movlw   0x00
            btfsc   STATUS,Z
            movlw   0x01F

            call SEND_CHAR

            banksel lp2
            decfsz  lp2,F
            goto    slinner

            decfsz  lp1,F
            goto    slouter
            return

; Draws a trend line for the last 16 hours
; Normal atmospheric range 980 to 1050 hPa at mean sea level
drawTrend:
            movlw   DD_RAM_LL   ; Init LCD to start drawing in lower left corner.
            call    SEND_CMD

            banksel lp1
            movlw   16      ; initialise loop counter
            movwf   lp1

            ; For each pressure reading we want to calculate (Pa - MSL_LOW) * 8 / (MSL_HIGH - MSL_LOW)
            ; to get value in range 0 to 7 for normal pressure range.
trendLoop:
            MOVL32 REGB,MSL_LOW
            banksel lp1
            movfw   lp1         ; Invert sense of loop counter so starts at 0 and increments
            sublw   16
            movwf   temp    

            bcf     STATUS,C    ; now convert temp to a 4 byte offset into integer array.
            rlf     temp,F
            bcf     STATUS,C
            rlf     temp,W      ; into W

            addlw   history     ; now has byte address
            movwf   FSR
            bankisel history
            banksel REGA        ; move 32 bit history value to REGA
            movfw   INDF
            movwf   REGA+0
            incf    FSR,F
            movfw   INDF
            movwf   REGA+1
            incf    FSR,F
            movfw   INDF
            movwf   REGA+2
            incf    FSR,F
            movfw   INDF
            movwf   REGA+3

            pagesel sub32
            call    sub32       ; pressure - MSL_LOW -> REGA
            pagesel $

            banksel REGA
            btfss   REGA+3,7    ; sign bit set (negative if so)
            goto    trendPos
            MOVL32  REGA,0      ; below lower limit so set to zero.
trendPos:
            pagesel mul32
            MOVL32  REGB,8
            call    mul32
            MOVL32  REGB,(MSL_HIGH-MSL_LOW)
            call    div32
            pagesel $
            
            ; Ok should be in range 0-7 but may be above this.
            banksel REGA
            movfw   REGA        ; ls byte of result
            sublw   7           ; 7 - result, carry clear if negative (result > 7)
            btfss   STATUS,C    ; so skip limiting if it's set.
            movlw   7

            ; W should now have the result in the range 0-7
            call    SEND_CHAR   ; So write to current position.

            banksel lp1
            decfsz  lp1,F
            goto    trendLoop
            return

            end
