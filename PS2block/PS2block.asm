	LIST   P=PIC16F627
	#include P16F627.INC
	
		__CONFIG        _BODEN_ON & _CP_OFF & _DATA_CP_OFF & _PWRTE_ON & _WDT_OFF & _LVP_OFF & _MCLRE_ON & _XT_OSC
		
				radix dec
				
		;INPUTS
SW1				EQU     H'00'		;SW1 is triggering RA0
SW2				EQU     H'01'		;SW2 is triggering RA1

		;OUTPUTS
		; Note - LEDS: low is on.
HEART_DISP		EQU		H'00'		; display roughly 1s heartbeat
STATE_DISP0		EQU		H'01'		; display of state
STATE_DISP1		EQU		H'02'		; display of state
ALMOST_EMPTY	EQU		H'04'		; buzzer, 1 is on
RELAY			EQU		H'05'		; switch relay 1 is on

DEBUG_LED		EQU		H'03'		; not connected but this pin is it!

;Variables
		cblock 0x20
			STATE				; 0 is "charging", 1 "run"
			TIMER				; approx to seconds
			CHARGEL				; current state of "charge"
			CHARGEH			
			MAXCHARGEL			; Max amount of "charge"
			MAXCHARGEH			
			CHARGE_DIV			; charge rate divider
		endc

; Constants
		cblock 0
			STATE_EMPTY
			STATE_CHARGE			
			STATE_RUN				
			STATE_FULL
		endc

;	   **********************************
				ORG	0				;Reset vector address
				GOTO	RESET		;goto RESET routine when boot.

;	   **********************************
;	   **********************************
DELAY			btfss 	INTCON,T0IF
				goto 	DELAY
				bcf  	INTCON,T0IF
				retlw 	0

;	   	**********************************
;		HeartBeat
;		Slows down the ticks to rougly 1 per sec
;		Returns 1 once a second else zero.
;	   	**********************************
HeartBeat		decfsz	TIMER,1
				retlw	0
				
				; reload timer file
				movlw	61
				movwf	TIMER

				; get here roughly once per second.
				; Toggle the heartbeat LED to show progress..
				btfsc	PORTB,HEART_DISP
				goto	heart1
				bsf		PORTB,HEART_DISP
				goto	heart2
heart1			bcf		PORTB,HEART_DISP
heart2			retlw	1

;	   	**********************************
;		IncCharge
;	   	**********************************

IncCharge
				bcf		PORTB,DEBUG_LED
				decfsz	CHARGE_DIV,1
				return
				

				; reload charge division
				movlw	48
				movwf	CHARGE_DIV

				movlw	1
				addwf	CHARGEL,1
				btfsc	STATUS,C
				incf	CHARGEH,1

				bsf		PORTB,DEBUG_LED
				return

;	   **********************************
;      **  Dedug :  Toggle the debug LED
;      **********************************
Debug			btfsc	PORTB,DEBUG_LED
				goto	debug1
				bsf		PORTB,DEBUG_LED
				goto	debug2
debug1			bcf		PORTB,DEBUG_LED
debug2			return


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
				MOVLW	B'11000000'
				MOVWF	TRISB		;RB7 & RB6 are inputs.
									;RB5...RB0 are outputs.
				MOVLW	B'11111111'	;all RA ports are inputs
				MOVWF	TRISA
				BCF	STATUS,RP0		;Switch Back to reg. Bank 0
				CLRF	PORTB

				MOVLW	61
				MOVWF	TIMER
				
				movlw	STATE_EMPTY
				movwf	STATE

				movlw	48		; ratio 1 hr in 2 days
				movwf	CHARGE_DIV

				movlw	0
				movwf	CHARGEL
				movwf	CHARGEH

				; Approx 1 hr maximum "on" time.
				movlw	3600/256 	
				movwf	MAXCHARGEH
				movlw	3600%256
				movwf	MAXCHARGEL
Loop
  				
				call	DELAY			; sync to timer

				; Show current state - note inverted state of LEDs
				bsf		PORTB,STATE_DISP0
				btfsc	STATE,0
				bcf		PORTB,STATE_DISP0
				bsf		PORTB,STATE_DISP1
				btfsc	STATE,1
				bcf		PORTB,STATE_DISP1

				
				; Branch to state
				movlw	STATE_EMPTY
				xorwf	STATE,0
				btfsc	STATUS,Z
				goto 	StateEmpty

				movlw	STATE_CHARGE
				xorwf	STATE,0
				btfsc	STATUS,Z
				goto 	StateCharging

				movlw	STATE_RUN
				xorwf	STATE,0
				btfsc	STATUS,Z
				goto 	StateRun

				movlw	STATE_FULL
				xorwf	STATE,0
				btfsc	STATUS,Z
				goto 	StateFull
				
				; Panic if we get here.. invalid state
				goto	RESET
				

StateEmpty
				bcf		PORTB,RELAY
				bcf		PORTB,ALMOST_EMPTY

				call	HeartBeat
				xorlw	0	; set z flag 
				btfsc	STATUS,Z
				goto	Loop

				call	IncCharge

				; Charge moved off zero?
				movf	CHARGEL,1
				btfsc	STATUS,Z
				goto	Loop

				; if so, change state to charging
				movlw	STATE_CHARGE
				movwf	STATE

				goto	Loop

StateCharging
				bcf		PORTB,RELAY
				bcf		PORTB,ALMOST_EMPTY

				; Check switches and determine state
				movlw	STATE_RUN
				btfsc	PORTA,SW1
				movwf	STATE

				call	HeartBeat
				xorlw	0	; set z flag 
				btfsc	STATUS,Z
				goto	Loop

				call	IncCharge

				; Fully charged?
				; if CHARGEL == MAXCHARGEL
				; && CHARGEH == MAXCHARGEH
				; then fully charged.
				movfw	CHARGEL
				xorwf	MAXCHARGEL,0
				btfss	STATUS,Z	
				goto	Loop
				; lower is same, check high
				movfw	CHARGEH
				xorwf	MAXCHARGEH,0
				btfss	STATUS,Z
				goto	Loop

				movlw	STATE_FULL
				movwf	STATE

				goto	Loop

StateRun
				bsf		PORTB,RELAY

				; Check "off" switch. move to charging if pressed.
				movlw	STATE_CHARGE
				btfsc	PORTA,SW2
				movwf	STATE

				; But if start switch still pressed stay in run
				; prevents rapid toggling and relay chatter
				; if both swithces pressed.
				movlw	STATE_RUN
				btfsc	PORTA,SW1
				movwf	STATE

				; Show almost empty if high byte of charge is 0
				bcf		PORTB,ALMOST_EMPTY
				movf	CHARGEH,1
				btfsc	STATUS,Z
				bsf		PORTB,ALMOST_EMPTY

				call	HeartBeat
				xorlw	0	; set z flag 
				btfsc	STATUS,Z
				goto	Loop

				call	Debug

				; Get here about once a second - decrement charge
				decf	CHARGEL,1
				movf	CHARGEL,0	; --> W
				xorlw	255			; = 255, Z if so
				btfsc	STATUS,Z
				decf	CHARGEH,1	


				movf	CHARGEL,0	; charge L -> W
				iorwf	CHARGEH,0	; or with high byte
				btfss	STATUS,Z
				goto	Loop

				; Reached zero, reset status to charge and
				; turn off relay
				movlw	STATE_EMPTY
				movwf	STATE

				goto	Loop

StateFull		
				bcf		PORTB,RELAY
				bcf		PORTB,ALMOST_EMPTY

				; Check switches and determine state
				movlw	STATE_RUN
				btfsc	PORTA,SW1
				movwf	STATE

				call	HeartBeat

				goto	Loop


				end
