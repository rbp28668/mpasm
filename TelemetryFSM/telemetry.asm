;semi-duplex 38.4kbps (one-wire) at 3.3V
;Byte structure 10-bit 0xx1, where x = nibble (4 bits), LSB first (i.e. standard 1 start bit, 8 data bits and 1 stop bit)
;RX polls sequentially for addresses 2-F (sends 02h - 0Fh) at 5ms intervals (Cockpit SX supports 
;only up to 7)
;If a sensor is present on the bus, it commences its response 300us after the polling byte. The 
;user has to ensure that there are no address conflicts.; A sensor response consists of 3 bytes (6 nibbles)
; Nibble 0: Unit type, i.e. [mAh] (table below)/
; Nbble 1: Address assigned to parameter (same as RX polled)
; Nibble 2-5: LSbit is the alarm bit (the one first transmitted, as it's LSB first)
; Nibble 2-5: Rest of bits is 15 bit signed
; (nibbles 2-5 I have not verified negative numbers, nor alarm bit, but with positive values it works. 
; Depending on unit selected, the decimal point is placed in a fixed position, i.e. Ampere is always 
;indicated with one decimal)
;I refer to Stoeckli on nibbles 2-5 as well as the unit types. I have verified mAh and A readings on 
;my Cockpit SX. Once I get my own MCU programming done, I'll run through the rest.
;Copied from Stoeckli's blog (unit table, I have indicated precision when known):
;01: V (one decimal)
;02: A (one decimal)
;03: m/s (one decimal)
;04: km/h (one decimal)
;05: rpm  ( x100) 
;06: °C (one decimal)
;07: ° (degrees? one decimal)
;08: m (integer value)
;09: % Fuel (integer percentages)
;10: % LQI (integer percentages)
;11: mAh (integer value)
;12: mL (integer value)
;13: km (one decimal)
;14: no units
;15: no units
;
; At 38.4 kbaud, 1 10 bit byte takes 260.4uS
; Total timing
; 260 + 300 + 3 *260 = 1.34mS
;
; Note that units don't have zero value.  Poll byte 0-15 in LS nibble, 0 in high nibble,  First reply byte, address in high nibble.#
; As we never reply to addresses 0 & 1 (Batt voltage & LQI) a poll byte must have 0 in high nibble. 
#include p16f88.inc
		__CONFIG _CONFIG1, _INTRC_IO & _WDT_OFF & _PWRTE_OFF & _MCLR_ON & _BODEN_OFF & _LVP_OFF & _CPD_OFF & _WRT_PROTECT_OFF & _DEBUG_OFF & _CCP1_RB0 & _CP_OFF
        __CONFIG _CONFIG2, _FCMEN_OFF & _IESO_OFF

		radix decimal 

;=======================================================
; Constants
;=======================================================


; Bits of INTFLAG
TICK        EQU 0   ; Timer 2 tick
TIMEOUT     EQU 1   ; Timter 1 timeout
RX          EQU 2   ; char received
TX          EQU 3   ; char transmitted
ERR         EQU 4   ; error state
SENDING     EQU 5   ; set true if sending

; States
ST_SYNC         EQU 0       ; waiting for sync period
ST_WT_ADDR      EQU 1       ; waiting for address byte
ST_WT_SEND1     EQU 2       ; send byte 1 and wait 
ST_WT_SEND2     EQU 3       ; send byte 2 and wait
ST_WT_SEND3     EQU 4       ; send byte 3 and wait
ST_FIN_SEND     EQU 5       ; wait for complete end of transmission
ST_DISCARD      EQU 6       ; discard unwanted

; Unit values
VOLTS       EQU  01; V (one decimal)
AMPS        EQU  02; A (one decimal)
MPS         EQU  03; m/s (one decimal)
KMH         EQU  04; km/h (one decimal)
RPM         EQU  05; rpm  ( x100) 
DEGREESC    EQU  06; °C (one decimal)
DEGREES     EQU  07; ° (degrees? one decimal)
METERS      EQU  08; m (integer value)
PCT_FUEL    EQU  09; % Fuel (integer percentages)
PERCENT     EQU  10; % LQI (integer percentages)
MAH         EQU  11; mAh (integer value)
ML          EQU  12; mL (integer value)
KM          EQU  13; km (one decimal)
NONE14      EQU  14; no units
NONE15      EQU  15; no units

; Time to wait without character to sync.
SYNC_TIME   EQU 2000

; Note RX is on RB2 and TX on RB5 

;=======================================================
; Macros
;=======================================================

; Configure a response a a given channel with specific units.
setchan macro channel, units
        movlw	units | (channel << 4)
        movwf	BYTES + (3 * channel)
        clrf    BYTES + (3 * channel + 1)
        clrf    BYTES + (3 * channel + 2)
        endm

; Ignore this channel
clrchan macro channel
        clrf	BYTES + (3 * channel)
        clrf    BYTES + (3 * channel + 1)
        clrf    BYTES + (3 * channel + 2)
        endm

; Configure timer one for a given microsecond delay (at 8MHz).
settim1	macro us
		movlw 	low(65536-2*us)
		movwf	TMR1L
		movlw	high(65536-2*us)
		movwf	TMR1H
		endm


; Macros to be used when entering a given state.
; They may set up timers, serial port etc on entry.

TO_SYNC         macro        ; waiting for sync period
	            bcf T1CON,TMR1ON        ; diable timer (bank 0)
		        settim1	SYNC_TIME		; Require Space
		        bsf T1CON,TMR1ON        ; enable timer
                movlw       ST_SYNC
                movwf       STATE
                endm

TO_WT_ADDR      macro        ; waiting for address byte
        		bsf RCSTA,CREN			; re-enable reception for next polling byte.
		        bcf PIR1,RCIF
                movlw       ST_WT_ADDR
                movwf       STATE
                endm

TO_WT_SEND1     macro        ; send byte 1 and wait 
                bsf         INTFLAG,SENDING ; signal transmission starting
                bcf         RCSTA,CREN	    ; diable reception of our reply
                banksel     TXSTA
                bsf		    TXSTA,TXEN      ; enable transmission.
                movfw       RXADDR
                movwf       FSR
                bankisel    BYTES
                movfw       INDF
                banksel     TXREG
                movwf       TXREG
                banksel     0
                movlw       ST_WT_SEND1
                movwf       STATE
                endm

TO_WT_SEND2     macro        ; send byte 2 and wait
                incf        RXADDR,F
                movfw       RXADDR
                movwf       FSR
                bankisel    BYTES
                movfw       INDF
                banksel     TXREG
                movwf       TXREG
                banksel     0
                movlw       ST_WT_SEND2
                movwf       STATE
                endm

TO_WT_SEND3     macro        ; send byte 3 and wait
                incf        RXADDR,F
                movfw       RXADDR
                movwf       FSR
                bankisel    BYTES
                movfw       INDF
                banksel     TXREG
                movwf       TXREG
                bcf         INTFLAG,SENDING ; signal last byte sent
                banksel     0
                movlw       ST_WT_SEND3
                movwf       STATE
                endm

TO_FIN_SEND     macro        ; wait for complete end of transmission
	            bcf         T1CON,TMR1ON        ; diable timer (bank 0)
		        settim1	    30        			; 30uS delay to ensure stop bit transmitted
		        bsf         T1CON,TMR1ON        ; enable timer
                movlw       ST_FIN_SEND
                movwf       STATE
                endm

TO_DISCARD      macro        ; discard unwanted
        		bcf RCSTA,CREN			; diable reception of any reply
	            bcf T1CON,TMR1ON        ; diable timer (bank 0)
		        settim1	2000		    ; Ignore bus for next 2mS
		        bsf T1CON,TMR1ON        ; enable timer

                movlw       ST_DISCARD
                movwf       STATE
                endm



;=======================================================
; Variables
;=======================================================
			

isave		udata_shr
WSAVE		res 1		; for interrupts
STATSAVE	res 1		; for interrupts
PCLHSAVE    res 1       ; for interrupts
FSRSAVE     res 1       ; for interrupts
INTCNT      res 1       ; counting interrupt
INTFLAG     res 1       ; for signalling - T2 interrupt sets 
                        ; bit 0 every second
                        ; bit 1 if char received (into RXCHAR)
                        ; bit 2 if T1 expired
RXCHAR      res 1       ; received character
STATE       res 1       ; for telemetry FSM

vars		udata
BYTES       ; Channel send table. "BYTES" is the generic
            ; label for the table but the individual
            ; channels are picked out below.
CH0         res 3
CH1         res 3
CH2         res 3
CH3         res 3
CH4         res 3
CH5         res 3
CH6         res 3
CH7         res 3
CH8         res 3
CH9         res 3
CH10        res 3
CH11        res 3
CH12        res 3
CH13        res 3
CH14        res 3
CH15        res 3

RXADDR		res 1		; the last received address.
;BTN         res 1       ; button state

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
        MOVWF   STATSAVE        ; Copy STATUS to a temporary register
        movfw   PCLATH          ; Save PCLATH as we use GOTO and multiple pages.
        movwf   PCLHSAVE
        clrf    PCLATH
        movfw   FSR
        movwf   FSRSAVE

        BCF     STATUS, RP0     ; Force Bank 0
        BCF     STATUS, RP1     ; 

        ; Manage timer2 interrupt to count 125Hz ticks.
        btfss   PIR1,TMR2IF     ; Timer 2 interrupt (PIR1 is bank 0)
        goto    CHECK_RX        ; skip if not timer 2
        bcf     PIR1,TMR2IF     ; clear interrupt

        decfsz  INTCNT,F
        goto    CHECK_RX        ; done if not zero.
        
        movlw   125             ; 1 seconds at 125Hz
        movwf   INTCNT          ; re-initialise counter
        bsf     INTFLAG,TICK    ; set the flag.

        ; Check for byte received
CHECK_RX:

        btfss   PIR1,RCIF       ; Received Char interrupt
        goto    CHECK_TX        ; skip if not received
        bcf     PIR1,RCIF       ; clear interrupt
        movfw   RCREG           ; get char from serial port
        movwf   RXCHAR          ; and save it
        bsf     INTFLAG,RX      ; signal received

        ; Check for byte transmitted
CHECK_TX:
        btfss   PIR1,TXIF       ; Transmitted Char interrupt
        goto    CHECK_T1        ; skip if not transmitted
        bcf     PIR1,TXIF       ; clear interrupt
        bsf     INTFLAG,TX      ; signal transmitted

        ; Check for T1 timeout.
CHECK_T1:
        btfss   PIR1,TMR1IF     ; Timer 1 timeout
        goto    FSM             ; skip if not received
        bcf     PIR1,TMR1IF     ; clear interrupt
        bsf     INTFLAG,TIMEOUT ; signal T1 timeout
        bcf     T1CON,TMR1ON    ; stop clock now

        ; Main FSM code
FSM:

WT_SYNC:
        ;State 1 : Sync  
	    ; On entry: start timer 1 with sync period
		; expected address = 0
	    ; RX - discard, restart timer
	    ; T1 timeout - : ->State 2
        movlw   ST_SYNC
        xorwf   STATE,W         ; sets Z if equal
        btfss   STATUS,Z        ; skip if equal
        goto    WT_ADDR         ; not equal? try next state

        ; Sync
        btfss   INTFLAG,RX      ; char received?
        goto    SYNC1
        settim1 SYNC_TIME       ; Restart timer 1 as haven't had a full 2mS of without rx.
        bcf     INTFLAG,RX      ; clear flag, ignore char
        goto    INTR_EXIT       ; exit, staying in same state
SYNC1:  
        btfss   INTFLAG,TIMEOUT ; Timed out?
        TO_WT_ADDR              ; next state ST_WT_ADDR
        goto    INTR_EXIT       ; Done in this state

WT_ADDR:
        ;State 2 : Wait for address
	    ;Next expected > limit : ->State 1
	    ;RX - correct address  : Send byte 1, -> State 3
	    ;RX - incorrect address : discard byte, -> State 6
        movlw   ST_WT_ADDR
        xorwf   STATE,W         ; sets Z if equal
        btfss   STATUS,Z        ; skip if equal
        goto    WT_SEND1        ; not equal? try next state

        ; Wait Address
        btfss   INTFLAG,RX      ; Received a char?
        goto    INTR_EXIT       ; if not, exit ISR
        
        ; Ok, so we've got an address character.  
        bcf     INTFLAG,RX

        ;if not a valid address byte (MS nibble must be 0) then
        ; go back to sync.
        movfw   RXCHAR
        andlw   0xF0
        btfsc   STATUS,Z       ; skip zero clear
        goto    WT_ADRX        ; zero set so valid address

        TO_SYNC
        goto    INTR_EXIT

        ; Convert address into an offset in the data table and store in RXADDR
WT_ADRX:
        movlw   BYTES           ; base address
        movwf   RXADDR
        movfw   RXCHAR
        andlw   0x0F            ; restrict to allowable channels
        addwf   RXADDR,F        ; + 3 * channel
        addwf   RXADDR,F        
        addwf   RXADDR,F        ; RXADDR now has pointer into data table.

        movfw   RXADDR
        movwf   FSR
        bankisel BYTES
        
        movlw   0
        xorwf   INDF,W          ; get units/address byte.  If zero, don't respond
        btfsc   STATUS,Z
        goto    NOSEND          ; Z must be set, i.e. byte was zero
        TO_WT_SEND1             ; go into state WT_SEND1
        goto    INTR_EXIT
                
NOSEND:
        TO_DISCARD              ; got into state DISCARD
        goto    INTR_EXIT


WT_SEND1:
        ;State 3 : Wait Send 1
	    ;End of Transmission : Send byte 2, ->State 4
        movlw   ST_WT_SEND1
        xorwf   STATE,W         ; sets Z if equal
        btfss   STATUS,Z        ; skip if equal
        goto    WT_SEND2        ; not equal? try next state

        btfss   INTFLAG,TX      
        goto    INTR_EXIT
        bcf     INTFLAG,TX
        TO_WT_SEND2
        goto    INTR_EXIT


WT_SEND2:
        ;State 4 : Wait Send 2
	    ;End of Transmission : Send byte 3, ->State 5
        movlw   ST_WT_SEND2
        xorwf   STATE,W         ; sets Z if equal
        btfss   STATUS,Z        ; skip if equal
        goto    WT_SEND3        ; not equal? try next state

        btfss   INTFLAG,TX      
        goto    INTR_EXIT
        bcf     INTFLAG,TX
        TO_WT_SEND3
        goto    INTR_EXIT

WT_SEND3:
        ;State 5 : Send byte 3
	    ; End of Transmission : ->State 2
        movlw   ST_WT_SEND3
        xorwf   STATE,W         ; sets Z if equal
        btfss   STATUS,Z        ; skip if equal
        goto    FIN_SEND        ; not equal? try next state

        btfss   INTFLAG,TX      
        goto    INTR_EXIT
        bcf     INTFLAG,TX
        TO_FIN_SEND
        goto    INTR_EXIT

FIN_SEND:
        ; State FIN_SEND: Allow stop bit to be transmitted before disabling transmission
        ; Entry - start stop bit timer
        ; TIMEOUT   -> disable transmission
	    ;           -> TO WAIT_ADDR
        movlw   ST_FIN_SEND
        xorwf   STATE,W         ; sets Z if equal
        btfss   STATUS,Z        ; skip if equal
        goto    DISCARD         ; not equal? try next state

        btfss   INTFLAG,TIMEOUT      
        goto    INTR_EXIT
        bcf     INTFLAG,TIMEOUT
		banksel TXSTA			; bank 1
		bcf		TXSTA,TXEN		; disable transmission, TX/RB5 high impedance.
        banksel 0
        TO_SYNC                 ; Sync with next poll byte.
        goto    INTR_EXIT

DISCARD:
        ; State DISCARD : Discard any received bytes from another sensor
	    ; Entry - disable reception and start timer.
	    ; TIMEOUT   -> TO WT_ADDR
        movlw   ST_DISCARD
        xorwf   STATE,W         ; sets Z if equal
        btfss   STATUS,Z        ; skip if equal
        goto    ERR_EXIT        ; not equal? run out of states so error
 
        bcf     INTFLAG,RX      ; dump any received characters
       
        btfss   INTFLAG,TIMEOUT      
        goto    INTR_EXIT
        bcf     INTFLAG,TIMEOUT
        TO_SYNC                 ; go and wait for the next address
        goto    INTR_EXIT

ERR_EXIT:

INTR_EXIT:
        movfw   FSRSAVE         ; restore FSR
        movwf   FSR
        movfw   PCLHSAVE        ; restore PCLATH
        movwf   PCLATH
        SWAPF   STATSAVE,W      ; Pull Status back into W
        MOVWF   STATUS          ; Store it in status 
        SWAPF   WSAVE,F         ; Prepare W to be restored
        SWAPF   WSAVE,W         ; Return it, preserving Z bit in STATUS		
		retfie

;=======================================================
; Main entry point
;=======================================================
PGM		code					; wherever the linker wants to put this
Startup:

                banksel OSCCON			; bank 1
                MOVLW	B'01110000'		; Set 8MHz internal clock
                MOVWF	OSCCON

                ; Serial port bits must be inputs.  Set all inputs apart from RB3
                bsf	STATUS,RP0		; Select bank 1 for trisB
                movlw	B'11111111'		; All port A inputs
                movwf	TRISA
                movlw	B'11110111'		; RB3 is output for debug led, all others inputs
                movwf   TRISB         
		

                ; Set up USART
                movlw	12			    ; 38.4 kbaud with 8 MHz and high speed set.
                movwf	SPBRG			; bank 1

                movlw 	B'00000100'		; 8 bit, tx disabled, async, high speed
                movwf	TXSTA			; bank 1
		
                banksel RCSTA			; bank 0
                movlw	B'10010000'		; serial enabled, 8 bit, continuous
                movwf	RCSTA

                ; Set up ADC
                banksel ANSEL			; page 1
                movlw	B'0000001'		; set AN0 as analogue input, others digital
		        movwf	ANSEL			;

                MOVLW   B'11000000'     ; ADCS2 is 1,  output right justified. References from Vdd/Vss
                MOVWF   ADCON1          ; still page 1


                ; Configure all the channels we want this to respond to.
                banksel BYTES
                clrchan 0
                clrchan 1
                setchan 2,VOLTS
                clrchan 3
                clrchan 4
                clrchan 5
                clrchan 6
                clrchan 7
                clrchan 8
                clrchan 9
                clrchan 10
                clrchan 11
                clrchan 12
                clrchan 13
                clrchan 14
                clrchan 15

                ; Set up timer 1. Note, set T1CON.TMR1ON to enable.
                movlw	B'00000000'		; Fosc/4, 1:1 prescale, stopped
                banksel	T1CON			; bank 0
                movwf	T1CON

                ; Setup initial state of FSM
                TO_SYNC


                ; Enable interrupts
                banksel PIE1
                bsf PIE1, TMR1IE ; timer 1 interrupts
                bsf PIE1, TXIE   ; serial transmit interrupt
                bsf PIE1, RCIE   ; serial receive interrupt
                bsf PIE1, TMR2IE ; timer 2 interrupts
                banksel INTCON
                bsf INTCON, GIE  ; global interrupt enable
                

mloop:

                ; Debug - lights LED if input low.
                ;btfsc PORTB,RB2
                ;bcf	  PORTB,RB3
                ;btfss PORTB,RB2
                ;bsf	  PORTB,RB3
                ;goto  mloop

                bcf PORTB,RB3			; turn off debug LED

                movlw	0				; reading from AD0
                call	ADCCORE			; read ADC
                banksel	ADRESL			; bank 1
                movfw	ADRESL
                banksel CH2
                movwf	CH2+1
                banksel ADRESH			; bank 0
                movfw	ADRESH
                banksel CH2
                movwf	CH2+2


                ; Switch to test alarm bit is connected to RB0 and grounds RB0 when pressed.
                bcf		STATUS,C
                btfss	PORTB,RB0		; skip if input high (switch not pressed)
                bsf		STATUS,C 
		        ; Shift left to include alarm bit in LS bit.
                rlf		CH2+1,F
                rlf		CH2+2,F
	
                ; TODO - need a form of macro that loads up the data with interrupts briefly
                ; turned off so that it's cleanly transferred under all circumstances.
		        goto mloop				


;=======================================================
; ADCCORE  Subroutine, takes a channel parameter in W and 
; returns with the ADC value ready to be read in ADRESL 
; and ADRESH.  Note that this takes approximately 130uS
; for a conversion.
;=======================================================
ADCCORE
		banksel temp

		; Work out the configuration byte by combining the
		; channel number with clock divider value.
		movwf	temp	
		bcf		STATUS,C
		rlf 	temp,F		; Move channel number to correct location.
		rlf 	temp,F
		rlf 	temp,F
		movlw 	H'41' ; clock divider / 8 or 16 depending on ADCS<2>,set channel and turn on ADC block.
		iorwf 	temp,W	; include channel number
 
		; Configure and turn on ADC
		banksel ADCON0
		movwf 	ADCON0

		; Need to wait for acquisition time c. 20uS (40 clocks)before starting conversion
		movlw 	13
		banksel temp
		movwf 	temp
                decfsz 	temp,F
		goto 	$-1


		; Now trigger conversion and wait for completion.
		banksel ADCON0
		bsf 	ADCON0,GO_NOT_DONE         ; initiate AD conversion
		btfsc 	ADCON0,GO_NOT_DONE
		goto 	$-1

		banksel PIR1						; Bank 0
		bcf		PIR1,ADIF ; clear interrupt flag

		; results available in ADRESL/ADRESH

		return


		end