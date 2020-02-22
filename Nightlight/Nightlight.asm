;***********************************************************
; Nightlight.
; PIC16F818 to control a simple LED night light.
; RB3/CCP1 OUTPUT controls LEDS
; RB5   INPUT reads PIR
; RA3/AN3 reads LDR
; TIMER 0 runs an interrupt drivent tick
; TIMER2 drives PWM
;***********************************************************

       TITLE "Nightlight"
        LIST P=PIC16F818
        include "P16F818.inc"
        include "../Math/math.inc"
       	
		__CONFIG _CCP1_RB3 & _BODEN_ON & _CP_OFF & _PWRTE_ON & _WDT_OFF & _LVP_OFF & _INTRC_IO & _MCLR_OFF   

		radix dec

;Uncomment this define to use dummy ADC readings so that it will work
;with the simulator.
;#define DUMMYADC 1


;=======================================================
; Constants
; Note - these need to be set to configure the hardware configuration
; of the PIC driving the LCD.
;=======================================================

_ClkIn			EQU		D'4000000'		; Processor clock frequency.

PIR         EQU 5       ; digital input for PIR sensor
LEDS        EQU 3       ; output bit to turn LEDs on.
LIGHT		EQU 3		; AN3 - light sensor

ISLIGHT     EQU 512     ; light is above this, dark below.

;===== Timer 0 ============
T0LOAD		EQU	 ((256-78)&255)


; Statflag bit assignment
T0INT		equ 0	; use bit 0 for signalling Timer 0 interrupt.
PIRON       equ 1   ; bit 1 for signalling PIR active
;=======================================================
; Variables.
;=======================================================

isr_data	udata_shr ; independent of bank seelction
WSAVE		res 1	; ISR save W
STATSAVE 	res 1	; ISR save status
STATFLAG	res 1	; Status flags for signalling ISR

			udata
TMP			res 1   ; Temp workspace
ADCOUT		res 2	; 16 bit output from ADC 
DELAY		res 1	; Delay counter for ADC read
ONTIME      res 2   ; 16 bit on timer.

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

		;Set RB5 as input (bits set).
		MOVLW	B'00100000'		; RB5 as input, rest outputs
        MOVWF   TRISB           ; Set direction for B
        
        banksel	ADCON1			; Set Bank 1
        MOVLW   B'10000000'     ; Enable AN0 - AN4 as analogue, ADCS2 is 0,  output right justified.
        MOVWF   ADCON1          ; 


		; Clear variables
		banksel WSAVE			; reset to bank 0
		clrf STATFLAG

        banksel ONTIME
        CLRF16  ONTIME
	
		; Turn off LEDs initially.
		banksel PORTB			; reset to bank 0
		BCF		PORTB,LEDS

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
        bsf     INTCON,PEIE     ; Enable peripheral interrupts.
		bsf		INTCON,GIE		; Enable global interrupts


; test code - links LEDS to PIR directly
;        banksel PORTB
;        bcf PORTB,LEDS
;        btfsc PORTB,PIR
;        bsf PORTB,LEDS
;        goto $-3


; test code - links LEDS directly to LDR - light makes them come on.
;DLOOP
;        movlw       LIGHT
;        call        ADCCORE
;        banksel ADCOUT
;		CMP16L	ADCOUT, ISLIGHT ; AMPS,CHARGT ; Carry set if ADCOUT >= LIGHT (i.e. is light)
;		BTFSS	STATUS,C
;		GOTO	NTDEBUG
;       bsf     PORTB,LEDS
;      goto    DLOOP   
;NTDEBUG
;        bcf     PORTB,LEDS
;        goto    DLOOP



		; Let things settle down...
		T0SYNC		; Synchronise with 50Hz timebase

MLOOP	

		T0SYNC		; Synchronise with 50Hz timebase
        
        ; if no PIR then just go to no trigger and drive LED off counters
        banksel     STATFLAG
        btfss       STATFLAG,PIRON
        goto        NOTRIG

        ; Ok so the PIR is active and can potentially trigger the LED.  
        ; If we're already running (ONTIME non zero) then just go and re-trigger
        ; as we want to ignore LDR if the LEDS are already on
        banksel     ONTIME
        movfw       ONTIME
        iorwf       ONTIME+1,W  ; set Z if not already triggered (ontime is zero)
        btfss       STATUS,Z    ; skip if zero, i.e. not triggered 
        goto        TRIGGER     ; skipped if triggered ONTIME is not zero.

        ; ONTIME is zero - initial trigger so check light intensity
        movlw       LIGHT
        call        ADCCORE

        ; Only trigger if it's dark
        banksel ADCOUT
		CMP16L	ADCOUT, ISLIGHT ; Carry set if ADCOUT >= LIGHT (i.e. is light)
		btfsc	STATUS,C  ; skip if carry clear (i.e. no light)
		goto	NOTRIG    ; carry was set - there was light so don't trigger.
      
        ; OK, so PIR is set - trigger delay
TRIGGER 
        banksel     ONTIME
        LD16L       ONTIME, (50 * 60 * 2) ; 2 mins
        ;LD16L       ONTIME,500 ; ten seconds
        
NOTRIG
        banksel     ONTIME
        ; Ok, so look at the timer - if non-zero turn
        ; the LEDs on (and decrement), if zero, off.
        movfw       ONTIME
        iorwf       ONTIME+1, W
        btfsc       STATUS,Z
        goto        IDLE

        ; zero flag is clear i.e. ONTIME is not zero.
        DEC16       ONTIME

        banksel     PORTB
        bsf         PORTB,LEDS
        goto        MLOOP

IDLE      
        banksel PORTB        
        bcf PORTB,LEDS    
		goto MLOOP	; forever




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
; T0SYNC
; Synchronises with the rollover of Timer 0 by software
; flag in STATFLAG.  Also polls the state of the PIR and
; sets PIRON if the PIR goes active at any point.
;=======================================================
T0SYNCIMP
		banksel STATFLAG
        bcf     STATFLAG,PIRON
        
T0WAIT
        banksel PORTB
        btfss   PORTB,PIR
        goto    T0PIROFF
        banksel STATFLAG
        bsf     STATFLAG,PIRON     
T0PIROFF   
		banksel STATFLAG
    	btfss 	STATFLAG,T0INT
		goto 	T0WAIT
		bcf  	STATFLAG,T0INT
		return

		End
