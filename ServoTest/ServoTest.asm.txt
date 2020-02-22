;============================================================================================
; Servo Tester
; Input Pot is in AN0. 
; Mode switch is in RA1
; Servo out is in RB0
; Mode LEDs in RB2 to RB5.
;============================================================================================

			LIST   P=PIC16F818
			#include P16F818.INC
	
			__CONFIG        _BODEN_ON & _CP_OFF &  _PWRTE_ON & _WDT_OFF & _LVP_OFF &  _XT_OSC
		
			radix dec
				
		;INPUTS
SW1				EQU     H'01'		;SW1 is triggering RA1

		;OUTPUTS
SERVO_OUT		EQU		H'00'		; Servo output bit.
RB2				EQU		H'02'
RB3				EQU		H'03'
RB4				EQU		H'04'
RB5				EQU		H'05'
DEBUG_LED		EQU		H'05'	

;Variables
		cblock 0x20
				SERVOL				; Low byte of servo position
				SERVOH				; High byte of servo position
				MODE				; Basic operation mode
				SLOWL				; Low byte of timer
				SLOWH
				DIRN				; Servo direction/sub-mode for automatic modes.	
				SWITCH				; Switch de-bounce
				SC1L				; first 16 bit scratch location low/high byte
				SC1H
				SC2L				; 2nd 16 bit scratch location low/high byte
				SC2H
		endc

; Constants
		cblock 0
				MODE_DIRECT			; Pot controls servo directly;
				MODE_SQUARE			; Square wave - pot controls speed
				MODE_TRIANGLE		; sawtooth
				MODE_FENCEPOST 		; low, centre or high point dependent on pot posn.
		endc

		constant baseTime = 65535	;

;		Timer 2
;		Pre/post-scalers total 128
;		Division ratio 156
;		Interrupt rate 50.080 Hz
;		
;	   **********************************
				ORG	0				;Reset vector address
				GOTO	RESET		;goto RESET routine when boot.

;	   	**********************************
;		* T0Sync
;		* Synchronises with the rollover of Timer 0 by
;		* waiting for the T0 interrupt flag to be set.
;	   	**********************************
T0Sync			btfss 	INTCON,TMR0IF
				goto 	T0Sync
				bcf  	INTCON,TMR0IF
				return

;	   	**********************************
;		* T1Sync
;		* Synchronises with the rollover of Timer 1 by
;		* waiting for the T1 interrupt flag to be set.
;	   	**********************************
T1Sync			btfss 	PIR1,TMR1IF
				goto 	T1Sync
				bcf  	PIR1,TMR1IF
				return

;	   	**********************************
;		* T2Sync
;		* Synchronises with the rollover of Timer 2 by
;		* waiting for the T2 interrupt flag to be set.
;	   	**********************************
T2Sync			btfss 	PIR1,TMR2IF
				goto 	T2Sync
				bcf  	PIR1,TMR2IF
				return

;	   	**********************************
;		* ReadADC
;		* Reads the current ADC channel
;	   	**********************************
ReadADC 		bsf		ADCON0,GO		; fire off ADC reading
WaitADC			btfsc	ADCON0,NOT_DONE	; 
				goto	WaitADC
				return

;	   **********************************
;      **  Debug :  Toggle the debug LED
;      **********************************
Debug			btfsc	PORTB,DEBUG_LED
				goto	debug1
				bsf		PORTB,DEBUG_LED
				goto	debug2
debug1			bcf		PORTB,DEBUG_LED
debug2			return


;	   	**********************************
;		* TestADResult : tests the value of
;		* the A/D result against the value
;		* in SC1L/SC1H.  Calculate ADC - SC1. 
;		* If carry set then ADC < SC1.
;	   	**********************************
TestADResult	movfw	SC1L			; Low byte of constant
				bsf		STATUS,RP0		; Switch to register bank 1
				subwf	ADRESL,W		; get low byte of result
				bcf		STATUS,RP0		; Switch back to register bank 0
				

				movfw	SC1H			; high byte of constant -> W
				btfss	STATUS,C		; skip if no borrow
				incf	SC1H,W			; inc high byte of constant to include borrow
				subwf	ADRESH,W		; high byte of result
				return


;	   **********************************
;      **  RESET :  main boot routine  **
;      **********************************

RESET			movlw	B'00000111'	;Disable Comparator module's
				movwf	CCP1CON
				
				bsf		STATUS,RP0	;Switch to register bank 1
									;Disable pull-ups
									;INT on rising edge
									;TMR0 to CLKOUT
									;TMR0 Incr low2high trans.
									;Prescaler assign to Timer0
									;Prescaler rate is 1:64
				movlw	B'11010101'	;Set PIC options (See datasheet).
				movwf	OPTION_REG	;Write the OPTION register.
		
				clrf	INTCON		;Disable interrupts
				clrf	PIE1		;Disable peripheral interrupts.
				movlw	B'11000000'
				movwf	TRISB		;RB7 & RB6 are inputs.
									;RB5...RB0 are outputs.
				movlw	B'11111111'	;all RA ports are inputs
				movwf	TRISA
				bcf		STATUS,RP0	;Switch Back to reg. Bank 0
				clrf	PORTB

				; Setup ADC
				bsf		STATUS,RP0	;Switch to register bank 1
				movlw	B'10001110'	; AN0 analogue in, rest digital, right justified result.
				movwf	ADCON1
				bcf		STATUS,RP0	;Switch back register bank 0
				movlw	B'10000001'	; ADC Clock 32TOsc, Chan 0, Enable ADC
				movwf	ADCON0

				; Start setup of Timer 1
				clrf 	T1CON 		; Stop Timer1, Internal Clock Source,
									; T1 oscillator disabled, prescaler = 1:1
				clrf 	TMR1H 		; Clear Timer1 High byte register
				clrf 	TMR1L		; Clear Timer1 Low byte register

				; Set up Timer 2 for roughly 50Hz timebase.
				clrf	T2CON		; 
				bsf		STATUS,RP0	;Switch to register bank 1
				movlw	156			; for 50Hz with 4MHz clock & /128 scaling
				movwf	PR2			; T2 period register
				bcf		STATUS,RP0	;Switch Back to reg. Bank 0
				movlw	B'00111010'	;Timer2: /8 postscale, /16 pre-scale
				movwf	T2CON
				bsf		T2CON,TMR2ON

				; Set initial servo position
				movlw	1500/256 	
				movwf	SERVOH
				movlw	1500%256
				movwf	SERVOL

				; And zero all the other state variables
				movlw	0
				movwf	DIRN
				movwf	SLOWL
				movwf	SLOWH
				movwf	MODE
				movwf	SWITCH


				clrf 	PIR1 		; Clear peripheral interrupts Flags

				;===================================================================
				; Main application loop
Loop
  				call	T2Sync;		; 50Hz timebase
				

				; calculate servo period and stuff in TMR1H & TMR1L
				movf	SERVOL,w		; low byte of servo pos -> W
				sublw	baseTime%256	; literal - w
				movwf	TMR1L			; save low byte

				movf	SERVOH,w		; high byte of servo pos -> W
				btfss	STATUS,C		; skip borrow if ~borrow set
				goto 	Borrow			
				
				; No borrow....
				sublw	baseTime/256
				goto	DoneSub

Borrow			sublw	(baseTime/256)-1
DoneSub			movwf	TMR1H			; Save high byte

				; Timer 1 now loaded with servo pulse width. Now generate pulse.
				bsf 	T1CON, TMR1ON 	; Timer1 starts to increment
				bsf		PORTB,SERVO_OUT	; Set servo out bit.
				call	T1Sync			; wait for servo pulse to finish
				bcf		PORTB,SERVO_OUT	; clear servo bit.
				bcf		T1CON, TMR1ON	; stop timer 1

				; Look at state of button - may want to change state.
				btfss	PORTA,SW1	
				goto	NotPressed
		
				;button pressed - increment switch variable.  If now at 2
				; (i.e. debounced) then increment mode.
				incf	SWITCH,F

				movfw	SWITCH
				xorlw	2
				btfss	STATUS,Z
				goto	Dispatch

				incf	MODE,W
				andlw	3	
				movwf	MODE
				goto	Dispatch

NotPressed		clrw
				movwf	SWITCH

				; dispatch to appropriate mode.
Dispatch		movfw	MODE
				xorlw	MODE_DIRECT
				btfsc	STATUS,Z
				goto	Direct
				
				movfw	MODE
				xorlw	MODE_SQUARE
				btfsc	STATUS,Z
				goto	Square
				
				movfw	MODE
				xorlw	MODE_TRIANGLE
				btfsc	STATUS,Z
				goto	Triangle

				; if none of the  others, must be MODE_FENCEPOST
				goto	Fencepost

				;======================================================
				;Direct read of ADC to drive servo.  ADC is 10 bits so
				; 0 to 1023.  512 is half way -> 1500uS hence add 988 to ADC.
				; MODE_DIRECT			; Pot controls servo directly;

Direct			bsf		PORTB,RB2
				bcf		PORTB,RB3
				bcf		PORTB,RB4
				bcf		PORTB,RB5

				call	ReadADC
				bsf		STATUS,RP0		; Switch to register bank 1
				movfw	ADRESL			; get low byte of result
				bcf		STATUS,RP0		; Switch back to register bank 0
				addlw	988%256			; add in low byte of offset
				movwf	SERVOL			; save low byte for next run.
				
				movfw	ADRESH			; high byte of result
				btfsc	STATUS,C		; skip if carry clear
				incf	ADRESH,w		; get high byte but incrementing as we go as carry set.
				addlw	988/256
				movwf	SERVOH
				
				goto	Loop


				;===========================================================
				;MODE_SQUARE		; Square wave - pot controls speed
				;Make min on/off time 1/10 second - 5 ticks.

Square			bcf		PORTB,RB2
				bsf		PORTB,RB3
				bcf		PORTB,RB4
				bcf		PORTB,RB5

				movlw	1			; Value to increment SLOWL with.
				addwf	SLOWL,F		; increment low byte and store
				btfss	STATUS,C
				clrw				; if C then leave 1 in w, else clear to 0
				addwf	SLOWH,F

				btfsc	DIRN,0		; look at direction bit
				goto	SQHigh

				; Set low position			
				movlw	1000%256
				movwf	SERVOL
				movlw	1000/256
				movwf	SERVOH
				goto	SQCount


				
SQHigh			movlw	2000%256
				movwf	SERVOL
				movlw	2000/256
				movwf	SERVOH
				; Drop through


SQCount			call	ReadADC
				
				; Get ADC result + 5 into SC1L + SC1H
				movlw	5				; Low byte of constant
				bsf		STATUS,RP0		; Switch to register bank 1
				addwf	ADRESL,W		; get low byte of result
				bcf		STATUS,RP0		; Switch back to register bank 0
				movwf	SC1L

				movfw	ADRESH
				btfsc	STATUS,C		; skip if no carry
				incf	ADRESH, W
				movwf	SC1H

				; SC1 should now be the limit for SLOW
				; calculate SC1 - SLOW : borrow if SLOW > SC1
				movfw	SLOWL			; low byte of time -> W
				subwf	SC1L, W			; limit - time -> W (& discard)
				movfw	SLOWH			; high byte of time -> W
				btfss	STATUS,C		; skip if no borrow 
				incf	SLOWH,W			; borrow so time+1 -> W
				subwf	SC1H, W			; limit - time -> W (& discard)
				btfsc	STATUS,C		; C is zero if borrow, borrow if time > limit		
				goto	Loop

				; Got here - timer expired, 
				; toggle direction bit
				movlw	1
				xorwf	DIRN,F
				clrw
				movwf	SLOWL			; and zero timer.
				movwf	SLOWH
				goto 	Loop



				;===========================================================
				;MODE_TRIANGLE		; sawtooth
				;Updates servo position each 50Hz tick.  1000 counts from 
				;end to end - at 50Hz gives 20s end to end with an increment
				;of 1. Increment of 128 gives c. 0.16s from end to end.
				;1024 -> 128: divide by 8.
Triangle		bcf		PORTB,RB2
				bcf		PORTB,RB3
				bsf		PORTB,RB4
				bcf		PORTB,RB5

				call	ReadADC

				; Get ADC value to scratch 1
				bsf		STATUS,RP0		; Switch to register bank 1
				movfw	ADRESL  		; get low byte of result
				bcf		STATUS,RP0		; Switch back to register bank 0
				movwf	SC1L
				movfw	ADRESH
				movwf	SC1H

				bcf		STATUS,C
				rrf		SC1H,F
				rrf		SC1L,F

				bcf		STATUS,C
				rrf		SC1H,F
				rrf		SC1L,F

				bcf		STATUS,C
				rrf		SC1H,F
				rrf		SC1L,W
				addlw	1				; inc.
				; Save add value in SC1 as unsigned.
				movwf	SC1L
				clrf	SC1H			; must be zero.

				; if dirn = 0 then add w to servo, else subtract.
				clrw
				xorwf	DIRN,W			; test dirn for zero.
				btfsc	STATUS,Z		; Zero so add
				goto	TriAdd			
				
				;negate SC1 to make subtract
				comf 	SC1L, F ; negate ACCa ( -ACCa -> ACCa )
				incf 	SC1L, F
				btfsc 	STATUS,Z
				decf 	SC1H, F
				comf 	SC1H, F

TriAdd			movf 	SC1L,W
				addwf 	SERVOL, F ; add lsb
				btfsc 	STATUS,C ; add in carry
				incf 	SERVOH, F
				movf 	SC1H,W
				addwf 	SERVOH, F ; add msb
				
				; Now check bounds......., limit and change direction as needed
				; TODO.

				; Servo - 1000 - borrow if servo < 1000
				movlw	1000%256		; Low byte of constant -> W
				subwf	SERVOL, W		; get low byte of result 
				movlw	1000/256		; high byte of constant -> W
				btfss	STATUS,C		; skip if no ~borrow
				movlw	1+(1000/256)	; inc high byte of constant to include borrow
				subwf	SERVOH, W		; high byte of result
				btfss	STATUS,C		; if servo < 1000 then borrow (C is clear)
				goto	TriGoUp

				; Servo - 2000 - borrow if servo < 2000
				movlw	2000%256		; Low byte of constant
				subwf	SERVOL, W		; get low byte of result
				movlw	2000/256		; high byte of constant -> W
				btfss	STATUS,C		; skip if no borrow
				movlw	1+(2000/256)	; inc high byte of constant to include borrow
				subwf	SERVOH, W		; high byte of result
				btfss	STATUS,C		; No borrow so skip goto if servo >= 2000
				goto	Loop

				; go down...
				movlw	1				; dirn = down
				movwf	DIRN
				movlw	2000%256		; set servo pos to 2000
				movwf	SERVOL
				movlw	2000/256
				movwf	SERVOH
				goto	Loop

TriGoUp			movlw	0				; dirn = up
				movwf	DIRN
				movlw	1000%256		; set servo pos to 1000
				movwf	SERVOL
				movlw	1000/256
				movwf	SERVOH
				goto	Loop


				;===========================================================
				;MODE_FENCEPOST		; low/med/high dependent on pot position
				; if ADC < 341 (1024/3) then low position
  				; else if ADC > 682 then high position
				; else centred.
Fencepost		bcf		PORTB,RB2
				bcf		PORTB,RB3
				bcf		PORTB,RB4
				bsf		PORTB,RB5

				call	ReadADC
				
				; calculate ADC - 341. If carry set then ADC < 341
				movlw	341%256
				movwf	SC1L
				movlw	341/256
				movwf	SC1H
				call	TestADResult
				btfss	STATUS,C		; 
				goto	LowPos

				; calculate ADC - 682. If carry set then ADC < 682
				movlw	682%256
				movwf	SC1L
				movlw	682/256
				movwf	SC1H
				call	TestADResult
				btfsc	STATUS,C		; 
				goto	HighPos

				; Default if not at either end.
				goto 	Centre

				;MODE_LOW			; Low point
LowPos			
				movlw	1000%256
				movwf	SERVOL
				movlw	1000/256
				movwf	SERVOH
				goto	Loop

				;MODE_CENTRE		; Centred
Centre			
				movlw	1500%256
				movwf	SERVOL
				movlw	1500/256
				movwf	SERVOH
				goto	Loop

				;MODE_HIGH			; High point
HighPos			
				movlw	2000%256
				movwf	SERVOL
				movlw	2000/256
				movwf	SERVOH
				goto	Loop

				end

