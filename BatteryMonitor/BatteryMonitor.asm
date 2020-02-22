;===========================================================
; BatteryMonitor.asm
; Uses - 
; Timer 0 - provide tick timebase (about 50Hz)
; Timer 2 - provide audio output (PWM)
; AN0 to AN3 - voltage, current and 2 x battery temp
;===========================================================

        TITLE "Buggy Battery Monitor"
        LIST P=PIC16F818
        include "P16F818.inc"
        include "math.inc"
       	
		__CONFIG _CCP1_RB3 & _BODEN_ON & _CP_OFF & _PWRTE_ON & _WDT_OFF & _LVP_OFF & _INTRC_IO        

		radix dec

;Uncomment this define to use dummy ADC readings so that it will work
;with the simulator.
;#define DUMMYADC 1

;====  Hardware bits ===
REDLED		EQU	5		; RB5 - drives red LED directly
GREENLED	EQU 2		; RB2 - drives green LED directly
BLUELED		EQU 6		; RA6 - drives blue LED via TR (inverted)
RELAY		EQU 0		; RB0 - drives relay via TR (inverted)
TEMP1		EQU 0		; AN0 - temperature sensor 1
TEMP2		EQU 1		; AN1 - temperature sensor 2
BATTV		EQU 2		; AN2 - battery voltage
BATTI		EQU 3		; AN3 - battery current
INSW		EQU 4		; RA4 - input control switch via TR (inverted)

; Fixed point offset.
FIXEDPT		EQU 4		; multiplier for fixed point with 4 fractional bits.

;==== Trip Voltages ====
VMAX		EQU (784 << FIXEDPT)
VMIN		EQU (145 << FIXEDPT)
DEGLITCH	EQU	50	; period for out of range before tripping in 50Hz ticks (e.g. 50 is 1s)

;===== Min/Max voltage in normal operation for voltage calculations for display
VFULL		EQU (731 << FIXEDPT)	; 4.2V per cell.
VEMPTY		EQU	(145 << FIXEDPT)	; 3.1V per cell.

;==== Trip Temperatures ====
TMPWARN		EQU (467 << FIXEDPT)	; sound warning above this (40 C)
TMPHIGH		EQU	(584 << FIXEDPT)	; shutdown above this
DEGLITCH	EQU	50	; period for out of range before tripping in 50Hz ticks (e.g. 50 is 1s)

;===== Charge / discharge current ====
; Thresholds to decide whether there is a valid charge or discharge going on.
; If not for a given length of tiem we want to turn off.
CHARGT		EQU (98 << FIXEDPT)		; Below this we're charging
DISCHT		EQU (106 << FIXEDPT) 	; Above this we're discharging


;===== Timer 0 ============
T0LOAD		EQU	 ((256-78)&255)

;---------------------------------------------------------------------
; Bitmasks for SSPSTAT
SSPSMP  equ 0x80
SSPCKE	equ 0x40
SSPDA	equ 0x20
SSPP	equ 0x10
SSPS	equ 0x08
SSPRW	equ 0x04
SSPUA	equ 0x02
SSPBF   equ 0x01


;=======================================================
; Variables.
; Note, whilst many are 24 bit, for most of the time
; they can be treated as 16 bit as MS byte will be zero.
; The time this is not the case is during recursive
; averaging of the ADC values where we would (just) run
; out of bits during the scaling.
;=======================================================
isr_data	udata_shr ; independent of bank seelction
WSAVE		res 1	; ISR save W
STATSAVE 	res 1	; ISR save status
STATFLAG	res 1	; Status flags for signalling ISR

			udata
TMP			res 1   ; Temp workspace
TMP24		res 3	; 24 bit scratch
ADCOUT		res 3	; 24 bit output from ADC as fixed point (top byte zero)
DELAY		res 1	; Delay counter for ADC read
T1TEMP		res 3	; T1 temperature sensor as 24 bit fixed point.
T2TEMP		res 3   ; T2 temperature sensor as 24 bit fixed point.
VOLTS		res 3	; Battery voltage as 24 bit fixed point.
AMPS		res 3	; Current drawn as 24 bit fixed point.
VDEL		res 1	; delay counter for over/under voltage.
IDLCNT		res 4	; idle time counter
FLASHT		res 1	; Counter for flashing blue LED

stats		udata
CHARGE		res 6	; integrated current

; SMBUS comms.
smbidx 	res 1 	; smbidx of bytes read/received
smbaddr res 1	; SMBUS address - don't confuse with I2C address!
smbtmp 	res 1 	;


; Statflag bit assignment
T0INT		equ 0	; use bit 0 for signalling Timer 0 interrupt.

;=======================================================
; ADCREAD  takes a channel parameter and returns once the
; AD conversion is complete for that channel.
ADCREAD	macro channel, dest
		MOVLW	channel
		CALL ADCCORE			; Read ADC as 16 bit fixed point into ADCOUT

		; Multiply existing value of dest by 15 into TMP24
		banksel TMP24
		MV24  TMP24, (dest)		; *1 -> TMP16
		ASL24 (dest)			; *2	
		ADD24 TMP24,(dest)		;	 
		ASL24 (dest)			; *4
		ADD24 TMP24,(dest)
		ASL24 (dest)			; *8
		ADD24 TMP24,(dest)

		CALL ADCADD
		MV24	(dest),TMP24

		endm   

;=======================================================
; SHUTDOWN  turns off the main power relay and waits for
; the power to go off.
; Note, also lights up all the LEDs for debugging.
;=======================================================
SHUTDOWN	macro 
		goto SHUTDOWNIMP
		endm


;=======================================================
; T0SYNC
; Synchronises with the rollover of Timer 0 by software
; flag in STATFLAG
;=======================================================
T0SYNC	Macro
		CALL T0SYNCIMP
		endm

;=======================================================
; PWMTIM sets the PWM output frequency to the value
; stored in the W register.  CCPR1L is set to maintain
; an approximately 50% duty cycle.
;=======================================================
PWMTIM	Macro	period
		movlw (period)
		CALL 	PWMTIMIMP
		endm

;=======================================================
; PWMON enables the PWM output (i.e. audio on).
; Note, both T2CON and CCP1CON are both bank 0.
;=======================================================
PWMON	Macro
		call PWMONIMP
		endm

;=======================================================
; PWMOFF diables the PWM output (i.e. audio off).
;=======================================================
PWMOFF	Macro
		call PWMOFFIMP
		endm


;========================================================
; PAUSE delays a fixed number of ticks.
;========================================================
PAUSE	Macro ticks
		MOVLW	(ticks)
		call PAUSEIMP
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


		; handle T0 interrupt and reload.
		btfss	INTCON,TMR0IF
		goto	NOTT0INT		; skip if not timer 0 interrupt
		bcf		INTCON,TMR0IF	; clear interrupt
		movlw	T0LOAD
		movwf	TMR0
		bsf		INTCON,TMR0IE	; make sure re-enabled.
		bsf		STATFLAG,T0INT	; signal timer 0 int.
NOTT0INT


;---------------------------------------------------------------------
; Uses 5 states dependent on the codes in SSPSTAT
; Bit 7 - ignore in i2C
; Bit 6 - ignore in I2C
; Bit 5 - D_A, 1 = last byte was data, 0 = last byte was address
; Bit 4 - P 1 = stop bit was detected last otherwise 0
; Bit 3 - S 1 = start bit was detected last
; Bit 2 - R_W, 1 = read on last address, 0 = write
; Bit 1 - ignore if using 7 bit addressing
; Bit 0 - Buffer full 1 = receive complete or transmitting, 0 buffer empty.
; So only actually intersted in DA, S, R_W, BF
; The I2C code below checks for 5 states:
;---------------------------------------------------------------------
; State 1: I2C write operation, last byte was an address byte.
;
; SSPSTAT bits: S = 1, D_A = 0, R_W = 0, BF = 1
;
; State 2: I2C write operation, last byte was a data byte.
;
; SSPSTAT bits: S = 1, D_A = 1, R_W = 0, BF = 1
;
; State 3: I2C read operation, last byte was an address byte.
;
; SSPSTAT bits: S = 1, D_A = 0, R_W = 1, BF = 0
;
; State 4: I2C read operation, last byte was a data byte.
;
; SSPSTAT bits: S = 1, D_A = 1, R_W = 1, BF = 0
;
; State 5: Slave I2C logic reset by NACK from master.
;
; SSPSTAT bits: S = 1, D_A = 1, R_W = 0, BF = 0
;
; For convenience, WriteI2C and ReadI2C functions have been used.
;----------------------------------------------------------------------
	banksel PIR1
	btfss PIR1,SSPIF ; Is this a SSP interrupt?
	goto NotI2C 	 ; No - skip SSP handler

	bcf PIR1,SSPIF	; clear interrupt flag

SSP_Handler
	banksel SSPSTAT
	movf SSPSTAT,W ; Get the value of SSPSTAT
	andlw (SSPS+SSPDA+SSPRW+SSPBF) ; Only want Start, D/A, R/W and BF bits
	banksel smbtmp ; Put masked value in smbtmp
	movwf smbtmp ; for comparision checking.
State1: 
    ; Write operation, last byte was an address
	; Having just received the address byte for this device we
	; reset our internal byte counter so we know to pick up the
	; first data byte as a SMB internal address.
	movlw (SSPS + SSPBF); (address + write) 
	xorwf smbtmp,W ;
	btfss STATUS,Z ; Are we in State1?
	goto State2 ; No, check for next state.....
	clrf smbidx ; Clear the receive index.
	call ReadI2C ; Do a dummy read of the SSPBUF - will have addr value.
    ;banksel SSPCON
	;bsf SSPCON,CKP ; Release the clock  - (shouldn't need this).
	goto I2CEnd

State2: 
    ; Write operation, last byte was data.
	; Device selected and smbidx keeps track of the count of 
	; bytes received.  When index is 0 it's an internal address
	; byte so we save it in smbaddr. Otherwise we call SMBWrite
	; with the received byte in W
	movlw (SSPS + SSPDA + SSPBF) ; buffer is full.
	xorwf smbtmp,W
	btfss STATUS,Z ; Are we in State2?
	goto State3 ; No, check for next state.....
	call ReadI2C ; Get the byte from the SSP.
    ;banksel SSPCON ; Now we've read the byte... (shouldn't need this)
	;bsf SSPCON,CKP ; Release the clock 
    banksel smbidx
	movf smbidx,F ; move smbidx to itself - test for zero
	btfss STATUS,Z ; if zero flag is set - byte is smb address
    goto S2wr
    movwf smbaddr ; save address byte
    incf smbidx,F
	goto I2CEnd
S2wr:
     call SMBWrite
	banksel smbidx
    incf smbidx,F
	incf smbaddr,F
    goto I2CEnd

State3: 
    ; Read operation, last byte was an address byte
	; This is putting the device into read mode.  There must
	; have been a write to set up the address before this
	; Address is set up in smbaddr, calls SMBRead which 
	; should return with the data to be sent back in w.
    movf  smbtmp,W    
    andlw (SSPS+SSPDA+SSPRW) ; Mask out BF bit in SSPSTAT
    xorlw (SSPS+SSPRW)  ; Read and not address
	btfss STATUS,Z ; Are we in State3?
	goto State4 ; No, check for next state.....
	call SMBRead
	movlw H'42' ; BEBUG
	call WriteI2C ; Write the byte to SSPBUF & release clock
	banksel smbidx
	incf smbidx,F
	incf smbaddr,F
	goto I2CEnd

State4: 
    ; Read operation, last byte was data,
	; Continued read operation and, as we had an ack,
    ; master is expecting another byte.
	; current read address in smbaddr.
	movlw (SSPS+SSPDA+SSPRW) ; buffer is empty.
	xorwf smbtmp,W
	btfss STATUS,Z ; Are we in State4?
	goto State5 ; No, check for next state....
	call SMBRead
	movlw H'69' ; DEBUG
	call WriteI2C ; Write to SSPBUF
	banksel smbidx
	incf smbidx,F
	incf smbaddr,F
	goto I2CEnd

State5:
    ; When master has read the last byte it's expecting it
    ; should assert NAK at the end of the transmission. Hence
    ; this should signal end of TX.
	; SSPCON - bank 0
    movlw (SSPS+SSPDA) ; A NACK was received when transmitting
	xorwf smbtmp,W ; data back from the master. Slave logic
	btfss STATUS,Z ; is reset in this case. R_W = 0, D_A = 1
	goto I2CErr ; and BF = 0
    banksel SSPCON
	bsf SSPCON,CKP ; Release the clock 
	;call SMBRdEnd - no need in this application.
	goto I2CEnd 


; If we aren’t in State5, then something is
; wrong. Do nothing on error state so just drop off.
I2CErr 
	banksel SSPSTAT
	clrf SSPSTAT
	
I2CEnd:


NotI2C:

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

		banksel OSCCON
		MOVLW	B'01100000'		; Set 4MHz internal clock
		MOVWF	OSCCON

		banksel PIR1
		clrf PIR1

		; Note for ADC operation at 4MHz need to set ADCS2 to 0 and ADCS1:0 to 01
		banksel TRISA
		MOVLW	H'1F'			; bottom 5 bits to inputs
        MOVWF   TRISA          	; Set top 3 bits output, bottom 5 input

		;RB1 (SDA) and RB4 (SCL) must be set as input (bits set).
		MOVLW	H'12'			; RB1 and RB4 as inputs for I2C, rest outputs
        MOVWF   TRISB           ; Set direction for B
        
        banksel	ADCON1			; Set Bank 1
        MOVLW   B'10000000'     ; Enable AN0 - AN4 as analogue, ADCS2 is 0,  output right justified.
        MOVWF   ADCON1          ; 

		; Configure SSP control for I2C
		banksel SSPCON
		movlw H'36' ; Setup SSP module for 7-bit:
					; SMP = 0 (Not i2c)
					; CKE = 0 (not i2c)
					; SSPEN = 1 - enable SSP module
					; CKP = 1 - allow clock to float so master can drive it.
					; 0110 = I2C Slave mode, 7-bit address
		movwf SSPCON ; address, slave mode

		; Set up slave address previously saved in smbtmp
		movlw	H'40'	; hard wire slave address
		banksel SSPADD
		movwf SSPADD
		clrf SSPSTAT
	
	
		banksel PIR1 ; Reset any outstanding serial interupt
	    bcf PIR1,SSPIF
 

		; Clear variables
		banksel WSAVE			; reset to bank 0
		clrf STATFLAG
		clrf VDEL				; delay counter for over/under voltage.
		clrf IDLCNT				; idle time counter
		clrf IDLCNT+1
		clrf IDLCNT+2
		clrf IDLCNT+3
		clrf FLASHT

		clrf smbidx
	    clrf smbaddr
	    clrf smbtmp
	
		; Turn on relay initially.
		banksel PORTB			; reset to bank 0
		BSF		PORTB,RELAY

        clrf TMR0 ; Clear Timer0 register
        clrf INTCON ; Disable interrupts and clear T0IF
      	; Set up Timer 0 for roughly 50Hz timebase.
		; Bit 5 - 0 for timer mode.
		movlw	B'00000111'		; internal clock, ps assigned to T0, 256 prescale
        bsf     STATUS, RP0     ; banksel for option_reg
		movwf	OPTION_REG
		bcf     STATUS, RP0 	; Select Bank of TMR0
		movlw	T0LOAD
		movwf	TMR0
        bcf		INTCON,TMR0IF	; clear any existing flag.
		bsf		INTCON,TMR0IE	; Enable T0 interrupts
		bsf     PIE1,SSPIE      ; Enable SSP interrupts
        bsf     INTCON,PEIE     ; Enable peripheral interrupts.
		bsf		INTCON,GIE		; Enable global interrupts



		; Let things settle down...
		T0SYNC		; Synchronise with 50Hz timebase

		
		; ADC routine uses recursive averaging.  Do initial reads of the 
		; ADC and use these to prime the values.  Note that ADCOUT is left
		; with the last ADC conversion value as a fixed point number.
		MOVLW TEMP1
		CALL ADCCORE
		MV24 T1TEMP,ADCOUT
		MOVLW TEMP2
		CALL ADCCORE
		MV24 T2TEMP,ADCOUT
		MOVLW BATTV
		CALL ADCCORE
		MV24 VOLTS, ADCOUT
		MOVLW BATTI
		CALL ADCCORE
		MV24 AMPS, ADCOUT
		
MLOOP	

		T0SYNC		; Synchronise with 50Hz timebase
		; test switch on RA4.  If low, then should switch off (buffered
		; by inverting transistor.
		BTFSS	PORTA,INSW
		GOTO	NOTOFF
		;SHUTDOWN

NOTOFF
		ADCREAD TEMP1,T1TEMP
		ADCREAD TEMP2,T2TEMP
		ADCREAD BATTV,VOLTS
		ADCREAD BATTI,AMPS

		;goto MLOOP	; DEBUG, just read ADC

		banksel PORTB
		BCF		PORTB,REDLED	; Turn current leds off
		BCF		PORTB,GREENLED

		; Set the red / green LED according to whether charging or discharging.
		; if charging or discharging reset the power off counter.
		banksel AMPS
		CMP16L	AMPS,CHARGT ; Carry clear if Amps < charge threshold (if < then charging)
		BTFSC	STATUS,C
		GOTO	NOTCHRG
		; So, if we get here we're charging.
		banksel PORTB
		BSF		PORTB,GREENLED	; so turn on green LED for charging
		banksel IDLCNT
		CLRF16 	IDLCNT			; and clear idle count.
		GOTO PULSBLUE			; Primarily debug.
;		GOTO	CHKVOLT 		; off to check voltages - don't want to flash blue LED or integrate
								; discharge current if charging.

NOTCHRG						; not charging - see if discharging
		CMP16L	AMPS,DISCHT ; Carry set if Amps >= discharge threshold (
		; If power-off counter reached maximum then power off as buggy idle.
		BTFSS   STATUS,C
		GOTO	NOTDISCH

		; if we get here we're discharging
		banksel PORTB
		BSF		PORTB,REDLED	; so turn on red LED for discharging
		banksel IDLCNT
		CLRF16 	IDLCNT			; and clear idle count.
        GOTO	PULSBLUE
		
NOTDISCH					; not discharging (and not charging)
		banksel IDLCNT
		INC16	IDLCNT
		; TODO - check for idle count reaching idle threshold
		; So, 50Hz clock, 60 sec per min = 3000 counts per min.
		; 5 min is therefore 15,000.   
		CMP16L IDLCNT, 15000	; timeout? carry clear if idle < timeout
		BTFSS	STATUS,C
		GOTO	PULSBLUE
		; Ok, timed out so shutdown after a quick descending tone.
		PWMON
		PWMTIM 14
		PAUSE 20
		PWMTIM 18
		PAUSE 20
		PWMTIM 24
		PAUSE 20
		PWMTIM 32
		PAUSE 20
		PWMOFF
		SHUTDOWN

PULSBLUE
		;banksel PORTB ; DEBUG
		;bcf PORTB,GREENLED ;DEBUG
		; Manage flashing of blue LED to show approx battery voltage (max to min)
		; VOLTS - VEMPTY
		; Note that FLASHT cycles counting down to 0.  If volts are high
		; we want blue LED on most of the time - hence led on if VOLTS > FLASHT
		banksel TMP24
		MV16	TMP24,VOLTS
		SUBL16	TMP24,VEMPTY

		; TMP16 should now contain the voltage above the minimum
		; as a fixed point number.  So - Allow for fixed point
		; by shifting right.
		;ASR16	TMP24
		;ASR16	TMP24
		;ASR16	TMP24
		;ASR16	TMP24

		; And divide by 8.
		;ASR16	TMP24
		;ASR16	TMP24
		;ASR16	TMP24
		
		ASL16	TMP24	; Rather than shifting right 7 times we shift
						; left once then take the mid byte below.

		; TMP24 should now contain (VOLTS - VEMPTY) / 8 as an integer.
		; now, if TMP24 >= FLASHT we want to turn on the blue LED.
		MOVFW	TMP24+1		; only need mid byte due to shift.
		SUBWF	FLASHT,W	; C is clear if TMP16 >= FLASHT 
		banksel PORTA
		BTFSC	STATUS,C
		BCF		PORTA,BLUELED ; C set so turn off
		BTFSS	STATUS,C
		BSF		PORTA,BLUELED ; C clear so turn on

		; Decrement flash counter and reload if it hits zero.
		banksel FLASHT
		DECF	FLASHT,W
		BTFSC	STATUS,Z
		MOVLW	((VFULL >> FIXEDPT) - (VEMPTY >> FIXEDPT)) / 8
		MOVWF	FLASHT
		;BTFSS	STATUS,Z	; debug
		;GOTO	CHKVOLT		; debug
		;banksel PORTB		; debug
		;BSF		PORTB,GREENLED ; debug
		; Check for over/under volts.
CHKVOLT	
		banksel VOLTS
		CMP16L	VOLTS,VMIN	; Carry set if V >= Vmin (OK), clear if under-voltage
		BTFSS	STATUS,C
		GOTO	VISLOW
		CMP16L	VOLTS,VMAX  ; Carry set if V >= Vmax (BAD!), clear if OK.
		BTFSC	STATUS,C
		GOTO	VISHIGH

		; Battery is within range - reset VDEL counter to reset de-glitch delay
		MOVLW	DEGLITCH
		MOVWF	VDEL

		; Check for over-temp.
		CMP16L	T1TEMP,TMPHIGH ; Carry set if T1 >= MAX
		BTFSC	STATUS,C
		GOTO	TEMPHI
		CMP16L	T2TEMP,TMPHIGH ; Carry set if T2 >= MAX
		BTFSC	STATUS,C
		GOTO	TEMPHI

		; Ok, so temp is not >= maximum allowable.  Check to see if its above
		; the warning threshold.
		CMP16L	T1TEMP,TMPWARN ; Carry set if T1 >= WARN
		BTFSC	STATUS,C
		GOTO	TEMPWARN
		CMP16L	T2TEMP,TMPWARN ; Carry set if T2 >= WARN
		BTFSC	STATUS,C
		GOTO	TEMPWARN

		; If we get here then everything is just fine!
		PWMOFF 		; disable any alarms.
		GOTO MLOOP

VISLOW	; Voltage is below minimum.  Decrement the counter and if it reaches zero 
		; then turn off the main power relay.
		banksel VDEL
		DECFSZ	VDEL,1
		GOTO 	MLOOP
		PWMON	; just leave low tone
		PAUSE 50
		PWMOFF
		SHUTDOWN

VISHIGH
		; Voltage is above maximum.  Decrement the counter and if it reaches zero 
		; then turn off the main power relay.
		banksel VDEL
		DECFSZ	VDEL,1
		GOTO 	MLOOP
		PWMON
		PWMTIM 14
		PAUSE 50
		PWMOFF
		SHUTDOWN

TEMPHI
		; Temperature is above maximum.
		PWMON
		movlw 10
		movwf DELAY
THILP
		PWMTIM 14
		PAUSE 5
		PWMTIM 18
		PAUSE 5
		DECFSZ DELAY,F
		GOTO THILP
		PWMOFF
		SHUTDOWN

TEMPWARN
		; Temperature is above warning threshold.  Sound an alarm but carry on
		; Alarm is an increasing siren noise from 440 to 1480 Hz
		banksel T2CON
		btfsc T2CON,TMR2ON
		GOTO NOPWMINIT  ; skip initialisation if T2 running.
		PWMON
NOPWMINIT
		movlw	41		; minimum period value
		bsf		STATUS,RP0	;Switch to register bank 1
		banksel PR2
		xorwf 	PR2,0	; compare with PR2, Z set if equal
		btfss 	STATUS,Z
		goto	NOPWM1  ; goto if z clear i.e. not equal
		movlw	142		; max period value +1		
		movwf	PR2		; re-init timer value.
NOPWM1
		decf PR2,W		; current period -1 -> W
		CALL PWMTIMIMP	; set new time - resets RP0 as well.

		GOTO MLOOP

;=======================================================
; ADCCORE  Subroutine, takes a channel parameter in W and 
; returns the ADC value, as a fixed point number in ADCOUT
;=======================================================
ADCCORE
		banksel TMP

		; Work out the configuration byte by combining the
		; channel number with clock divider value.
		MOVWF	TMP	
		BCF	STATUS,C
		RLF TMP,F		; Move channel number to correct location.
		RLF TMP,F
		RLF TMP,F
		MOVLW H'41' ; clock divider / 8 and set channel
		IORWF TMP,W	; include channel number
 
		; Configure and turn on ADC
		banksel ADCON0
		MOVWF ADCON0

		; Need to wait for acquisition time
		MOVLW 20
		banksel DELAY
		MOVWF DELAY
ADDEL	DECFSZ DELAY,1
		GOTO ADDEL

#ifdef DUMMYADC	
		banksel TMP
		movfw TMP
		xorlw (0<<3)
		btfsc STATUS,Z
		goto ADC0
		movfw TMP
		xorlw (1<<3)
		btfsc STATUS,Z
		goto ADC1
		movfw TMP
		xorlw (2<<3)
		btfsc STATUS,Z
		goto ADC2
		movfw TMP
		xorlw (3<<3)
		btfsc STATUS,Z
		goto ADC3
		goto ADC4


ADC0	;Temp1
		LD16L ADCOUT,300
		goto ADCDONE

ADC1	;Temp2
		LD16L ADCOUT,300
		goto ADCDONE

ADC2	;BattV
		LD16L ADCOUT,145 ; 145 to 731 in normal operation 784 to trip
		goto ADCDONE

ADC3	;BattI
		LD16L ADCOUT,102
		goto ADCDONE

ADC4	;Not used.
		LD16L ADCOUT,0
		goto ADCDONE

ADCDONE
#else

		; Now trigger conversion and wait for completion.
		banksel ADCON0
		BSF ADCON0,GO_NOT_DONE         ; initiate AD conversion
ADCWT	BTFSC ADCON0,GO_NOT_DONE
		GOTO ADCWT

		; Write ADC to ADCOUT
		banksel ADRESL
		MOVFW   ADRESL
		banksel ADCOUT
		MOVWF   ADCOUT
		banksel ADRESH
		MOVFW   ADRESH
        banksel ADCOUT
		MOVWF   ADCOUT+1
		banksel PIR1
		BCF		PIR1,ADIF ; clear interrupt flag
#endif
		; Convert ADCOUT to a fixed point number.
		banksel	ADCOUT
		clrf ADCOUT+2			; zero top byte (i.e. treat as 24 bit)
		MUL24x16 ADCOUT

		RETURN

;=======================================================
; Subroutine ADCADD, adds in the ADC result to the 
; temporary accumulator and normalises it by division
; by 16 (with rounding).
;=======================================================
ADCADD
		; Add in ADC result into TMP24
		ADD24	TMP24,ADCOUT
		ROUND24 TMP24
		DIV24x16 TMP24
		RETURN

;=======================================================
; PWMTIMIMP sets the PWM output frequency to the value
; stored in the W register.  CCPR1L is set to maintain
; an approximately 50% duty cycle.
;=======================================================
PWMTIMIMP	
		bsf		STATUS,RP0	;Switch to register bank 1
		movwf	PR2			; Sets period of T2
		bcf		STATUS,RP0	;Switch back to register bank 0
		movwf	TMP		    ; also init duty cycle
		decf	TMP,1	    ; set to (PR2-1)/2
		bcf		STATUS,C	; clear carry for following rotate.
        rrf     TMP,0		; divide by 2 for 50% duty cycle -> W
		movwf	CCPR1L  	; Set period.
		RETURN



;=======================================================
; PWMONIMP enables the PWM output (i.e. audio on).
; Note, both T2CON and CCP1CON are both bank 0.
;=======================================================
PWMONIMP 
		movlw	141			; 440 Hz - A below middle C!
		call    PWMTIMIMP	; set period and duty cycle.
		movlw	B'00000110' ; T2 on, x1 post-scaler, x16 prescaler.
		movwf	T2CON
		movlw	B'00001100'
		movwf	CCP1CON
		return

;=======================================================
; PWMOFFIMP diables the PWM output (i.e. audio off).
;=======================================================
PWMOFFIMP	
		banksel T2CON
		movlw	B'00000010' ; T2 off, x1 post-scaler, x16 prescaler.
		movwf	T2CON
		movlw	B'00000000' ; PWM off
		movwf	CCP1CON
		return


;========================================================
; PAUSEIMP delays a fixed number of ticks.  Number of ticks
; passed in W
;========================================================
PAUSEIMP
		banksel TMP
		MOVWF	TMP
pauseloop
		T0SYNC
		banksel TMP
		DECFSZ	TMP,1
		goto pauseloop
		banksel 0
		return

;=======================================================
; SHUTDOWN  turns off the main power relay and waits for
; the power to go off.
; Note, also lights up all the LEDs for debugging.
;=======================================================
SHUTDOWNIMP	 
		banksel PORTB
		BCF		PORTB,RELAY ; so turn off main power relay
		BSF		PORTA,BLUELED
		BSF		PORTB,REDLED
		BSF		PORTB,GREENLED
		GOTO	$			; and wait for the lights to go out!
		

;=======================================================
; T0SYNC
; Synchronises with the rollover of Timer 0 by software
; flag in STATFLAG
;=======================================================
T0SYNCIMP
		banksel STATFLAG
T0WAIT	btfss 	STATFLAG,T0INT
		goto 	T0WAIT
		bcf  	STATFLAG,T0INT
		return

;---------------------------------------------------------------------
; WriteI2C
; Sends a byte in W back to the master and releases the clock so
; that the master can read the data.
; SSPBUF - bank 0
; SSPCON - bank 0
; SSPSTAT - bank 1
;---------------------------------------------------------------------
WriteI2C
	banksel SSPSTAT
	btfsc SSPSTAT,BF ; Is the buffer full?
	goto WriteI2C ; Yes, keep waiting.
	banksel SSPCON ; No, continue. (page 0)
DoI2CWrite
	bcf SSPCON,WCOL; Clear the WCOL flag.
	movwf SSPBUF ; Write the byte in WREG
	btfsc SSPCON,WCOL; Was there a write collision?
	goto DoI2CWrite
	bsf SSPCON,CKP ; Release the clock so master can clock out of SSPCON
	return

;---------------------------------------------------------------------
; ReadI2C
; Reads the byte written to the slave and returns it in W
; SSPBUF - bank 0
;---------------------------------------------------------------------
ReadI2C
	banksel SSPBUF
	movf SSPBUF,W ; Get the byte and put in WREG
	return


    ; Called with byte written to device in W.  Current address in smbaddr.
SMBWrite
		return
		

    ; Called to get byte from current SMB address (in smbaddr) and return 
    ; it in W
SMBRead
		movfw smbaddr
		movwf FSR	; set indirect register, users all 8 bits.
		movfw INDF	; indirect read -> W
		return

		End
