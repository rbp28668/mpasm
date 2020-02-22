; FLIGHT_LIGHTS.ASM   Control model lights
; Note - servo input must be on RB3 as it goes into CCP1 input.
; Servo pulses are timed by CCP1 and interrupts so that the time
; (since the last pulse) of the leading edge is captured, then the
; time of the trailing edge is captured and the pulse width calculated.
; During the trailing edge interrupt, once there is a valid pulse width,
; then bit 0 of PFLAG is set to signal to the main routine that
; there is another pulse.
; Written by Bruce Porteous
; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;
        TITLE "Controlling flight lights from servo"
        LIST P=PIC16F818
        include "P16F818.inc"
        include "math.inc"
       	
		__CONFIG _CCP1_RB3 & _BODEN_ON & _CP_OFF & _PWRTE_ON & _WDT_OFF & _LVP_OFF & _INTRC_IO        
        
#define TESTBOARD 0

; Macros to set lights on. On test board lights are active low, on production, active high.
UPPERON		macro
			if TESTBOARD
			BCF		PORTB,UPPERLED
			else
			BSF		PORTA,UPPERLED
			endif
			endm

UPPEROFF	macro
			if TESTBOARD
			BSF		PORTB,UPPERLED
			else
			BCF		PORTA,UPPERLED
			endif
			endm

LOWERON		macro
			if TESTBOARD
			BCF		PORTB,LOWERLED
			else
			BSF		PORTA,LOWERLED
			endif
			endm

LOWEROFF	macro
			if TESTBOARD
			BSF		PORTB,LOWERLED
			else
			BCF		PORTA,LOWERLED
			endif
			endm

NAVON		macro
			if TESTBOARD
			BCF		PORTB,NAVLED
			else
			BSF		PORTB,NAVLED
			BSF		PORTB,NAVLED2
			endif
			endm

NAVOFF		macro
			if TESTBOARD
			BSF		PORTB,NAVLED
			else
			BCF		PORTB,NAVLED
			BCF		PORTB,NAVLED2
			endif
			endm


;=======================================================
; Constants
;=======================================================
STOFF		equ 0		; off state of lights between pulses
STON1		equ 1		; on state of lights 

ONP1		equ 3		; on duration for channel 1
OFFP1		equ 30		; off duration for channel 1

ONP2		equ 3		; on duration for channel 2
OFFP2		equ 31		; off duration for channel 2

			if TESTBOARD
UPPERLED	equ	0		; bit 0 for upper LED
LOWERLED	equ 5		; bit 5 for lower LED
NAVLED		equ 2		; Bit 2 for navigation LED
			else
UPPERLED	equ	2		; bit 2 of RA for upper LED
LOWERLED	equ 4		; bit 4 of RA for lower LED
NAVLED		equ 0		; Bit 0 of RB for navigation LED
NAVLED2		equ	2		; Bit 2 of RB for other nav LED
			endif

NAVTHRESH	equ D'1333'	; above this value to turn on nav lights
ACTHRESH	equ D'1666'	; above this value to turn on anti-collision lights.

;=======================================================
; Variables
;=======================================================
		udata_shr

WSAVE		res 1	; ISR save W
STATSAVE 	res 1	; ISR save status

LEAD		res 2		; leading edge time LS/MS
PULSE		res 2		; pulse width and temp trailing edge.
PFLAG		res 1		; signal pulse to main loop - set by ISR, cleared by main loop
ST1			res 1		; state for output channel 1
CT1			res 1		; counter for output channel 1
ST2			res 1		; state for output channel 2
CT2			res 1		; counter for output channel 2

;=======================================================
; Initial startup and ISR vectors
    
        ; Startup entry point
STARTUP	code
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

INTR_POLL:
        ;
        ; Process interrupts from the Input Capture/Compare pin
        ; (CCP1 on the 16F818)
        ;
ISR_1:  
        BTFSS   PIR1, CCP1IF    ; Check to see that CCP1 interrupted
        GOTO    ISR_2           ; If not continue
		BCF     PIR1, CCP1IF    ; Re-enable it
        BTFSS   CCP1CON, CCP1M0 ; Which edge? (clear=falling, set = rising)
        GOTO    FALL_EDGE       ; Go pick up the falling edge

		; Capture on rising edge
        MOVF    CCPR1L,W        ; else store leading edge value
        MOVWF   LEAD			; into 16 bit work LEAD
        MOVF    CCPR1H,W
        MOVWF   LEAD+1

        BCF     CCP1CON, CCP1M0 ; Now capture the trailing edge
        GOTO    ENDCCP1           ; Exit the interrupt service routine
        
FALL_EDGE:
        MOVF    CCPR1L,W        ; Store the captured value into
        MOVWF   PULSE         	; PULSE (low)
        MOVF    CCPR1H,W
        MOVWF   PULSE+1       	; and PULSE (high)
        BSF     CCP1CON, CCP1M0 ; Re-set for trailing edge capture

        SUB16   PULSE, LEAD		; PULSE = PULSE - LEAD
        
        BSF     PFLAG,0       ; Indicate we have a new sample.
		
		; Clear timer 1 so that we restart on 0 again.
		CLRF TMR1L				; low first to prevent rollover into high.
		CLRF TMR1H

ENDCCP1: 


ISR_2:                          ; Process the next interrupt
;
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
; Main code
                
MAIN:
        CLRF    STATUS          ; Set Bank 0
        CLRF    PORTA           ; Clear PortA
        CLRF    PORTB           ; and clear PortB
        MOVLW   H'07'           ; Make PortA Digital I/O
        MOVWF   ADCON1          ; By setting ADCON1<0:3>
        BSF     STATUS,RP0      ; Set Bank 1
        CLRF    TRISA           ; Now A is all outputs
        CLRF    TRISB           ; B all outputs
        BSF     TRISB,3         ; CCP1 is also an input on RB3
        
		MOVLW	B'01100000'		; Set 4MHz internal clock
		MOVWF	OSCCON
        
		BSF     PIE1, CCP1IE    ; Enable interrupts from CCP1        
        CLRF    STATUS          ; Back to BANK 0
        MOVLW   B'00000001'     ; Enable Timer 1 1:1 Prescale, internal clock, Enabled.
        MOVWF   T1CON
        MOVLW   B'00000101'     ; Capture mode rising edge
        MOVWF   CCP1CON
        
         
		; Clear variables
        CLRW
  		MOVWF	LEAD		; low byte of leading edge time
		MOVWF	LEAD+1		; high byte of leading edge time
		MOVWF	PULSE		; low byte of pulse width
		MOVWF	PULSE+1		; high byte of pulse width
		MOVWF	PFLAG		; signal pulse to main loop - set by ISR, cleared by main loop
		MOVWF	ST1			; state for output channel 1
		MOVWF	CT1			; counter for output channel 1
		MOVWF	ST2			; state for output channel 2
		MOVWF	CT1			; counter for output channel 2


		; Setup FSM
		MOVLW	STOFF		; initial off state
		MOVWF	ST1			; into state 1
		MOVWF	ST2			; and state 2
		MOVLW	OFFP1		; state 1 off counter
		MOVWF	CT1
		MOVLW 	OFFP2
		MOVWF	CT2

        ; Enable interrupts
        BSF     INTCON, PEIE
        BSF     INTCON, GIE     ; Enable interrupts

		; Turn off all lights initially
		LOWEROFF
		UPPEROFF
		NAVOFF

		; Main polling loop, waits for flag to be
		; set by Interrupt Service Routine.
POLL:	BTFSS 	PFLAG,0		; wait for something to do.                       
		GOTO 	POLL
		BCF		PFLAG,0		; reset flag now we're doing something

		; See if Nav lights should be on.
		CMP16L	PULSE,NAVTHRESH  ; C clear if threshold > pulse. So want to turn on nav
								 ; if pulse >= threashold or C clear.
		BTFSS	STATUS,C
		GOTO 	NAVSET
		NAVON
		GOTO	NAVDONE
NAVSET:	NAVOFF

NAVDONE:

		; See if anti-col lights should be on.
		CMP16L	PULSE,ACTHRESH
		BTFSC	STATUS,C
		GOTO	ACFSM
		LOWEROFF
		UPPEROFF
		GOTO POLL

ACFSM:	; Anti-collision lights FSM
		MOVLW	STOFF		; initial state?
		XORWF	ST1,W
		BTFSS	STATUS,Z	; skip if set (z flag)		
		GOTO 	POLL2		

		; Initial state.
		DECFSZ	CT1,F
		GOTO	ENDCH1
		; Timed out - turn on LED and change state.
		UPPERON
		MOVLW	ONP1		; reinit counter 1 for on period
		MOVWF	CT1
		MOVLW	STON1		; change state to on.
		MOVWF	ST1
		GOTO 	ENDCH1

POLL2:	; Not initial state.
		MOVLW	STON1		; First on state?
		XORWF	ST1,W
		BTFSS	STATUS,Z	; skip if equal (z flag set)
		GOTO 	ENDCH1		

		; First on state
		DECFSZ	CT1,F
		GOTO	ENDCH1
		; Timed out - turn off LED and change state.
		UPPEROFF
		MOVLW	OFFP1		; reinit counter 1 for off period
		MOVWF	CT1
		MOVLW	STOFF		; change state to on.
		MOVWF	ST1
		GOTO 	ENDCH1



ENDCH1:	; End of channel 1 FSM
		; Second channel FSM to go here once first debugged.
		MOVLW	STOFF		; initial state?
		XORWF	ST2,W
		BTFSS	STATUS,Z	; skip if set (z flag)		
		GOTO 	POLL3		

		; Initial state.
		DECFSZ	CT2,F
		GOTO	ENDCH2
		; Timed out - turn on LED and change state.
		LOWERON
		MOVLW	ONP2		; reinit counter 2 for on period
		MOVWF	CT2
		MOVLW	STON1		; change state to on.
		MOVWF	ST2
		GOTO 	ENDCH2

POLL3:	; Not initial state.
		MOVLW	STON1		; First on state?
		XORWF	ST2,W
		BTFSS	STATUS,Z	; skip if equal (z flag set)
		GOTO 	ENDCH2		

		; First on state
		DECFSZ	CT2,F
		GOTO	ENDCH2
		; Timed out - turn off LED and change state.
		LOWEROFF
		MOVLW	OFFP2		; reinit counter 2 for off period
		MOVWF	CT2
		MOVLW	STOFF		; change state to on.
		MOVWF	ST2
		GOTO 	ENDCH2

ENDCH2:
		GOTO POLL		
        END        

