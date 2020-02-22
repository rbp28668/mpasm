;******************************************************
;LED Blaster driver.
;Target: PIC16F818
;Link with SMBSlave.o

; PWM calculates - 21 cycles
; Interrupt overhead - 8 cycles
; Total:			29 cycles.
; 1 PWM cycle is 256 steps - 7474 cycles
; 8MHz clock -> 2M cycles per sec, 500nS per cycle.
; 3.712mS processing per PWM cycle.
; Aim for 100Hz pwm (10mS) - 78 clocks per interrupt.
;******************************************************

#include p16f818.inc
#include "../i2c/smb.inc"

		__CONFIG _BODEN_ON & _CP_OFF & _PWRTE_ON & _WDT_OFF & _LVP_OFF & _INTRC_IO


;=======================================================
; Constants
;=======================================================

NODE_ADDR	EQU 0x40	; for I2C

; LED driving bits.
REDLED		EQU	RB0
GREENLED	EQU	RB2
BLUELED		EQU RB5

PWMSYNC		EQU 0	; bit set to signal PWM cycle end.


;=======================================================
; Macros
;=======================================================
PRBSRedW	macro
			movfw	prbs
			xorwf	prbs+1,w
			endm

PRBSGreenW	macro
			movfw	prbs+1
			xorwf	prbs+2,w
			endm

PRBSBlueW	macro
			movfw	prbs
			xorwf	prbs+2,w
			endm

showstate	macro s
			if (s & 0x04) != 0
			bsf PORTB,REDLED
			else
			bcf	PORTB,REDLED
			endif
			if (s & 0x02) != 0
			bsf PORTB,GREENLED
			else
			bcf	PORTB,GREENLED
			endif
			if (s & 0x01) != 0
			bsf PORTB,BLUELED
			else
			bcf	PORTB,BLUELED
			endif
			endm

;=======================================================
; Variables
;=======================================================
			udata_shr	; 0x20 onwards marked as shared so use this rather than udata.
stat		res 1		; status byte - read as I2C 0.
mode		res 1		; current mode.
red			res 1		; current red value
green		res 1		; current green value
blue		res 1		; current blue value
prbs		res 4		; manage state of PRBS here.
ramp		res 1		; ramp counter for PWM.			
statflag	res 1		; set flag on PWM zero crossing.
redt		res 1
greent		res 1
bluet		res 1



isave		udata_shr
WSAVE		res 1
STATSAVE	res 1

			udata_ovr
temp		res 1


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
        MOVWF   STATSAVE      ; Copy STATUS to a temporary register
        BCF     STATUS, RP0     ; Force Bank 0
        BCF     STATUS, RP1     ; 

		ifndef  NOTIMER
		btfss	PIR1,TMR2IF		; Timer interrupt?
		goto	NOTT2INT
		bcf		PIR1,TMR2IF		; clear T2 interrupt flag before enabling interrupts.

		
		; test for PWM - 7 cycles.
		movfw	ramp
		subwf	red,W				; subtract ramp from value note ~borrow
		btfsc	STATUS,C
		bsf		PORTB,REDLED
		btfss	STATUS,C
		bcf		PORTB,REDLED

		movfw	ramp
		subwf	green,W			; subtract ramp from value note ~borrow
		btfsc	STATUS,C
		bsf		PORTB,GREENLED
		btfss	STATUS,C
		bcf		PORTB,GREENLED

		movfw	ramp
		subwf	blue,W			; subtract ramp from value note ~borrow
		btfsc	STATUS,C
		bsf		PORTB,BLUELED
		btfss	STATUS,C
		bcf		PORTB,BLUELED

		incf	ramp,F
		btfsc	STATUS,Z
		bsf		statflag,PWMSYNC

NOTT2INT:
		endif




INTR_EXIT:
        swapf   STATSAVE,W    ; Pull Status back into W
        movwf   STATUS          ; Store it in status 
        swapf   WSAVE,F         ; Prepare W to be restored
        swapf   WSAVE,W         ; Return it, preserving Z bit in STATUS		
		retfie


;=======================================================
; Main entry point
;=======================================================
PGM		code					; wherever the linker wants to put this
Startup:

		bsf		STATUS,RP0		; Bank 1 for OSCCON
		movlw	B'01110000'		; Set 8MHz internal clock
		movwf	OSCCON

		; Set B as outputs except input bits for I2C. SCL is RB4, SDA is RB1
		movlw	B'00010010'	
     	movwf	TRISB         	; Set port B bits direction. (bank 1)

		; Enable I2C
		movlw NODE_ADDR
		call SMBInit
		
		; Turn off LEDs
		bcf		STATUS,RP0		; bank 0 for PORTB
		bcf 	PORTB,REDLED
		bcf		PORTB,GREENLED
		bcf		PORTB,BLUELED

		banksel mode	
		clrf	mode
		clrf	prbs
		clrf	prbs+1
		clrf	prbs+2
		clrf	prbs+3

		clrf	red
		clrf	green
		clrf	blue

		clrf	ramp
		clrf	statflag
		

		; Configure timer 2 to count 78 cycles (0..77)
		bcf		STATUS,RP0		; bank 0 for PIR1
		bcf		PIR1,TMR2IF		; clear T2 interrupt flag before enabling interrupts.
		bsf		STATUS,	RP0		; bank 1 for PR2, PIE1
		movlw	D'77'
		movwf	PR2
		bsf		PIE1,TMR2IE		; Enable timer 2 interrupts.
		bcf		STATUS, RP0		; T2CON is bank 0
		movlw	B'00000100'		; pre and post scalars 1:1 and timer 2 on.
		movwf	T2CON

		; Don't care which bank for INTCON
		bsf		INTCON,PEIE		; enable peripheral interrupts.
		bsf		INTCON,GIE		; global enable interrupt.

		banksel mode
		movlw	1
		movwf	mode
Main
		clrwdt
        btfsc 	PIR1,SSPIF 		; Is this a SSP interrupt?
		call	SSP_Handler		; service it if so
		btfss	statflag,PWMSYNC		; sync with PWM
		goto 	Main
		bcf		statflag,PWMSYNC

		call	calcPRBS		; Update pseudo-random binary sequence.

		; Figure out which mode we're in and jump accordingly.
		movlw	0
		xorwf	mode,W
		btfsc	STATUS,Z
		goto 	mode0

		movlw	1
		xorwf	mode,W
		btfsc	STATUS,Z
		goto 	mode1

		movlw	2
		xorwf	mode,W
		btfsc	STATUS,Z
		goto 	mode2

		movlw	3
		xorwf	mode,W
		btfsc	STATUS,Z
		goto 	mode3

		goto	Main
		
mode0
		; Drive brightness directly off PRBS
		PRBSRedW
		movwf	red
		PRBSGreenW
		movwf	green
		PRBSBlueW
		movwf	blue

		goto	Main			

mode1	
		;Random ramps

		; Compare current value of LED with target value
		movfw	red
		subwf	redt,W		; redt-red.  If redt >= red then no borrow ( C set) and want to increment, else decrement
		btfsc	STATUS,Z	; if equal
		goto m1rstred			; go and reset.
		movlw	H'FF'		; assume decrement (C clear)
		btfsc	STATUS,C	
		movlw	1			; was set so increment
		addwf	red,F		; incrment or decrement as appropriate.
		goto	m1Green	
m1rstred
		PRBSRedW
		movwf	redt		; new random brightness target


m1Green
		movfw	green
		subwf	greent,W	; greent-green.  If greent >= green then no borrow ( C set) and want to increment, else decrement
		btfsc	STATUS,Z	; if equal
		goto m1rstgreen		; go and reset.
		movlw	H'FF'		; assume decrement (C clear)
		btfsc	STATUS,C	
		movlw	1			; was set so increment
		addwf	green,F		; incrment or decrement as appropriate.
		goto	m1Blue	
m1rstgreen
		PRBSGreenW
		movwf	greent		; new random brightness target
		

m1Blue
		movfw	blue
		subwf	bluet,W		; bluet-blue.  If bluet >= blue then no borrow ( C set) and want to increment, else decrement
		btfsc	STATUS,Z	; if equal
		goto m1rstblue		; go and reset.
		movlw	H'FF'		; assume decrement (C clear)
		btfsc	STATUS,C	
		movlw	1			; was set so increment
		addwf	blue,F		; incrment or decrement as appropriate.
		goto	m1Done	
m1rstblue
		PRBSBlueW		
		movwf	bluet		; new random brightness target

m1Done	goto Main

mode2	; White sawtooth
		incf red,F
		incf green,F
		incf blue,F
		goto Main

mode3	; Hold position - direct control from I2C
		goto Main

;=======================================================
; Runs one cycle of a 32 bit PRBS  (linear feedback shift register).
; max 24 cycles
; See http://www.xilinx.com/support/documentation/application_notes/xapp052.pdf
; for tap positions for maximal length PRBS with a 32 bit shift register
; For 32 bits, taps at 32,22,2,1
; So, taking bit 1 of the shift register as bit 0 of the LS byte and shifting left
; Bit 32 = bit 7 of prbs+3
; Bit 22 = bit 5 of prbs+2
; Bit 2 = bit 1 of prbs
; Bit 1 = bit 0 of prbs
; Code
;=======================================================
calcPRBS
		movlw	1	; 1 to set up xnor in feeback so can initialise with zeros.
		btfsc	prbs,0
		addlw	1
		btfsc	prbs,1
		addlw	1
		btfsc	prbs+2,5
		addlw	1
		btfsc	prbs+3,7
		addlw	1

		; right, w now has the total number of bits set on the feedback tap + 1
		; ls bit is xnor of taps so get it into C for shifting into register.
		bcf		STATUS,C
		andlw	1	; only keep the feedback bit.
		btfss	STATUS,Z	; if was zero, don't set C - set C on 1
		bsf		STATUS,C

		; rotate the feedback bit into the shift register.		
		rlf		prbs,F
		rlf		prbs+1,F
		rlf		prbs+2,F
		rlf		prbs+3,F

		return



    ; Called with byte written to device in W.  Current address in smbaddr.
SMBWrite
		movwf	temp
		movfw 	smbaddr
		addlw	stat		; address status reg as 0
		movwf 	FSR	; set indirect register, users all 8 bits.
		movfw	temp
		movwf 	INDF	; indirect write of W
		return
    ; Called to get byte from current SMB address (in smbaddr) and return 
    ; it in W
SMBRead
		movfw 	smbaddr
		addlw	stat		; address status reg as 0
		movwf 	FSR	; set indirect register, users all 8 bits.
		movfw 	INDF	; indirect read -> W
		return

        ; Called once a read from a slave is complete (master has
        ; asserted NACK at the end of the read).
SMBRdEnd
		return

        ; Called in polling loop - not used as we're implementing our own polling loop
SMBPoll
		return



		end