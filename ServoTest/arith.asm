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
				

				; calculate servo period and stuff in TMR1H & TMR1L
				movf	SERVOL,w		; low byte of servo pos -> W
				sublw	baseTime%256	; literal - w
				movwf	SC1L			; save low byte

				movf	SERVOH,w		; high byte of servo pos -> W
				btfss	STATUS,C		; skip borrow if carry (borrow) clear
				goto 	Borrow			
				
				; No borrow....
				sublw	baseTime/256
				goto	DoneSub

Borrow			sublw	(baseTime/256)-1
DoneSub			movwf	SC1H			; Save high byte

				goto 	Loop
				end

