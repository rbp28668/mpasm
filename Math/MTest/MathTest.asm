;===========================================================
; MathTest.asm
; Uses - 
; Timer 0 - provide tick timebase (about 50Hz)
; Timer 2 - provide audio output (PWM)
; AN0 to AN3 - voltage, current and 2 x battery temp
;===========================================================

        TITLE "Math routine test"
        LIST P=PIC16F818
        include "P16F818.inc"
        include "math.inc"
       	
		__CONFIG _CCP1_RB3 & _BODEN_ON & _CP_OFF & _PWRTE_ON & _WDT_OFF & _LVP_OFF & _INTRC_IO        

		radix dec


;====  Hardware bits ===
REDLED		EQU	5		; RB5 - drives red LED directly
GREENLED	EQU 2		; RB2 - drives green LED directly



;=======================================================
; Variables
;=======================================================
	udata 
WSAVE		res 1	; ISR save W
STATSAVE 	res 1	; ISR save status
STATFLAG	res 1	; Status flags for signalling ISR

ACC16		res 2   ; 16 bit accumulator
TMP16		res 2	; 16 bit scratch

temp		res 1	; for BCD conversion
counter		res 1 	; for BCD conversion

bcd         res 3;  ; for 8 and 16 bit BCD
bin8        res 1;  ; 



;=======================================================
; EQUALS Tests a 16 bit register value for equality and
; sets the RED led if not
;=======================================================
EQUALS  Macro Reg16, Value
		MOVF  Reg16,W
		XORLW low(Value)
		BTFSS STATUS,Z
		GOTO FAIL
		MOVF  Reg16+1,W
		XORLW high(Value)
		BTFSS STATUS,Z
		GOTO FAIL
		endm

;=======================================================
; Initial startup and ISR vectors
    
        ; Startup entry point
STARTUP	code 0
        GOTO    MAIN            ; startup
        
		; Interrupt entry point
        code     H'0004'          ; Interrupt service routine     
        GOTO    INTSVC
        

PROG	code

;=======================================================
; Interrupt Service
     
INTSVC:        
        MOVWF   WSAVE           ; Copy W to a temporary register
        SWAPF   STATUS,W        ; Swap Status Nibbles and move to W 
        MOVWF   STATSAVE      ; Copy STATUS to a temporary register
        BCF     STATUS, RP0     ; Force Bank 0
        BCF     STATUS, RP1     ; 



; Exit the interrupt service routine. 
; This involves recovering W and STATUS and then
; returning. Note that putting STATUS back automatically pops the bank
; back as well.
;               This takes 6 Tcy for a total overhead of 12 Tcy for sync
;               interrupts and 13 Tcy for async interrupts.
; 
INTR_EXIT:
        SWAPF   STATSAVE,W    ; Pull Status back into W
        MOVWF   STATUS          ; Store it in status 
        SWAPF   WSAVE,F         ; Prepare W to be restored
        SWAPF   WSAVE,W         ; Return it, preserving Z bit in STATUS
        RETFIE

;=======================================================
; Main code entry point.  
;=======================================================
                
MAIN:
        CLRF    STATUS          ; Set Bank 0
        CLRF    PORTA           ; Clear PortA
        CLRF    PORTB           ; and clear PortB

		; Note for ADC operation at 4MHz need to set ADCS2 to 0 and ADCS1:0 to 01
		banksel TRISA
		MOVLW	H'1F'			; bottom 5 bits to inputs
        MOVWF   TRISA          	; Set top 3 bits output, bottom 5 input
		MOVLW	H'12'			; RB1 and RB4 as inputs for I2C, rest outputs
        MOVWF   TRISB           ; Set direction for B
        
		banksel OSCCON
		MOVLW	B'01100000'		; Set 4MHz internal clock
		MOVWF	OSCCON
 
		; Math test routines.
		banksel ACC16
		;=============================================
		; Loads and moves, clears.
		;=============================================
	
		; Simple test of LD16L
		LD16L ACC16, H'1234'
		EQUALS ACC16, H'1234'

		; Move
		LD16L ACC16, H'1234'
		LD16L TMP16, 0
		MV16  TMP16, ACC16
		EQUALS TMP16, H'1234'

		; Clear
		LD16L ACC16, H'1234'
		CLRF16 ACC16
		EQUALS ACC16, 0
		;=============================================
		; ADDS
		;=============================================

		; 16 bit add literal, no carry 
		LD16L ACC16, H'1234'
		ADDL16 ACC16, H'4365'
		EQUALS ACC16, H'5599'

		; 16 bit add literal, carry low to high 
		LD16L ACC16, H'1289'
		ADDL16 ACC16, H'43AB'
		EQUALS ACC16, H'5634'

		; 16 bit add, no carry 
		LD16L ACC16, H'1234'
		LD16L TMP16, H'4365'
		ADD16 ACC16,TMP16
		EQUALS ACC16, H'5599'

		; 16 bit add , carry low to high 
		LD16L ACC16, H'1289'
		LD16L TMP16, H'43AB'
		ADD16 ACC16,TMP16
		EQUALS ACC16, H'5634'

		;=============================================
		; Subtraction.
		;=============================================

		; 16 bit subtract literal, no carry 
		LD16L ACC16, H'4376'
		SUBL16 ACC16, H'1234'
		EQUALS ACC16, H'3142'

		; 16 bit subtract literal, borrow low to high 
		LD16L ACC16, H'5421'
		SUBL16 ACC16, H'21AB'
		EQUALS ACC16, H'3276'

		; 16 bit subtraction, no carry 
		LD16L ACC16, H'4376'
		LD16L TMP16, H'1234'
		SUB16 ACC16,TMP16
		EQUALS ACC16, H'3142'

		; 16 bit subtraction, borrow low to high 
		LD16L ACC16, H'5421'
		LD16L TMP16, H'21AB'
		SUB16 ACC16,TMP16
		EQUALS ACC16, H'3276'

		;=============================================
		; Increment.
		;=============================================
		
		LD16L ACC16, H'1234'
		INC16 ACC16
		EQUALS ACC16, H'1235'

		LD16L ACC16, H'12FF'
		INC16 ACC16
		EQUALS ACC16, H'1300'

		;=============================================
		; Decrement.
		;=============================================

		LD16L ACC16, H'1234'
		DEC16 ACC16
		EQUALS ACC16, H'1233'

		LD16L ACC16, H'1200'
		DEC16 ACC16
		EQUALS ACC16, H'11FF'

		;=============================================
		; Shift left
		;=============================================

		LD16L ACC16, H'1234'
		ASL16 ACC16
		EQUALS ACC16, H'2468'

		LD16L ACC16, H'FFFF'
		ASL16 ACC16
		EQUALS ACC16, H'FFFE'

		LD16L ACC16, H'0080'
		ASL16 ACC16
		EQUALS ACC16, H'0100'

		;=============================================
		; Shift right
		;=============================================

		LD16L ACC16, H'1234'
		ASR16 ACC16
		EQUALS ACC16, H'091A'

		LD16L ACC16, H'FFFF'
		ASR16 ACC16
		EQUALS ACC16, H'7FFF'

		LD16L ACC16, H'0100'
		ASR16 ACC16
		EQUALS ACC16, H'0080'

		;=============================================
		; Compare
		;=============================================

		LD16L ACC16, H'1234'
		CMP16L ACC16, H'1234'
		; C is set
		BTFSS STATUS,C
		GOTO FAIL

		LD16L ACC16, H'1234'
		CMP16L ACC16, H'1233'
		; C is set
		BTFSS STATUS,C
		GOTO FAIL

		LD16L ACC16, H'1234'
		CMP16L ACC16, H'1235'
		;C is clear
		BTFSC STATUS,C
		GOTO FAIL

        banksel bin8
        movlw   D'123'
        movwf   bin8
        call Bin8toBCD

        ;Result should have 0x23 in LSB, 0x01 in MSB


        LD16L   ACC16,D'12345'
        call    Bin16toBCD
        ;0x45 in LSB, 0x23 and 0x01 in MSB



		banksel PORTB
		BSF PORTB,GREENLED
		goto 	$

FAIL
		banksel PORTB
		BSF PORTB,REDLED
		goto	$

		
		; Declare the 8x8 multiply routine
		; TODO

		; Declare the 2 BCD conversion routines
        
		Bin8toBCDDef bcd,bin8
		Bin16toBCDDef bcd,ACC16

		
		End
