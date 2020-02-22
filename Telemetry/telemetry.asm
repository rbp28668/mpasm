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

#include p16f88.inc
		__CONFIG _CONFIG1, _INTRC_IO & _WDT_OFF & _PWRTE_OFF & _MCLR_ON & _BODEN_OFF & _LVP_OFF & _CPD_OFF & _WRT_PROTECT_OFF & _DEBUG_OFF & _CCP1_RB0 & _CP_OFF
        __CONFIG _CONFIG2, _FCMEN_OFF & _IESO_OFF

		radix decimal 

;=======================================================
; Constants
;=======================================================

UNITS	EQU 6 	; Degrees C
MSBADDR	EQU 2	; listen on this MSB channel

; Note RX is on RB2 and TX on RB5 

;=======================================================
; Macros
;=======================================================

settim1	macro us
		movlw 	low(65536-2*us)
		movwf	TMR1L
		movlw	high(65536-2*us)
		movwf	TMR1H
		endm
;=======================================================
; Variables
;=======================================================
			

isave		udata_shr

WSAVE		res 1		; for interrupts
STATSAVE	res 1		; for interrupts

vars		udata
BYTE0		res 1		; first byte to send, ls nibble is units, ms nibble our address
BYTE1		res 1		; second byte to send, ls bit is alarm, next 7 bits is ls 7 bits of value
BYTE2		res 1		; third byte to send, ms 8 bits of signed value

RXADDR		res 1		; the last received address.

BTN         res 1       ; button state

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


INTR_EXIT:
        SWAPF   STATSAVE,W    ; Pull Status back into W
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
		movlw	12			; 38.4 kbaud with 8 MHz and high speed set.
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

		; Set up the first byte to transmit.  Could be configured in EEPROM
		banksel BYTE0
		movlw	UNITS | (MSBADDR << 4)
		movwf	BYTE0

		; Set up timer 1. Note, set T1CON.TMR1ON to enable.
		movlw	B'00000000'		; Fosc/4, 1:1 prescale, stopped
		banksel	T1CON			; bank 0
		movwf	T1CON


mloop:

		; Debug - lights LED if input low.
		;btfsc PORTB,RB2
		;bcf	  PORTB,RB3
		;btfss PORTB,RB2
		;bsf	  PORTB,RB3
		;goto  mloop

		bcf PORTB,RB3			; turn off debug LED

		; Want to sync - poll bytes are preceeded by typically 2-5mS of idle.
        ; If RX (aka RB2) goes low during a 2ms period then restart the timer
        ; if timer times out then we've had 2ms without serial input
sync:
		bcf T1CON,TMR1ON
		settim1	2000			; Require 2mS Space
		bsf T1CON,TMR1ON
syncw:
		btfss PORTB,RB2			; if RX input goes low in time period, restart clock.
		goto sync

		btfss PIR1,TMR1IF		; keep waiting if TMR1 flag not set.
		goto syncw
		bcf T1CON,TMR1ON		; stop clock now


        ; At this point the sync period has elapsed, the poll requests will start 
        ; soon.

		movlw	0				; reading from AD0
		call	ADCCORE			; read ADC
		banksel	ADRESL			; bank 1
		movfw	ADRESL
		banksel BYTE1
		movwf	BYTE1
		banksel ADRESH			; bank 0
		movfw	ADRESH
		movwf	BYTE2


		; Switch to test alarm bit is connected to RB0 and grounds RB0 when pressed.
		bcf		STATUS,C
		btfss	PORTB,RB0		; skip if input high (switch not pressed)
		bsf		STATUS,C 
		; Shift left to include alarm bit in LS bit.
		rlf		BYTE1,F
		rlf		BYTE2,F


                ; Test button and increment units when released
                btfsc   PORTB,RB0       ; 
                goto    nosw
                ;switch pressed
                incfsz  BTN,F
                goto    endsw
                swapf   BYTE0,W          ; units in LS 4 bits, now MS 4 bits
                addlw   16               ; add 1 to units, ignore carry
                movwf   BYTE0
                swapf   BYTE0,F


nosw:
                clrf    BTN

endsw:

		bsf RCSTA,CREN			; re-enable reception for next polling byte.
		bcf PIR1,RCIF

		; In the meantime, reload the timer value for 300uS delay (with 8MHz clock)
		;settim1 300

		; wait for poll.
		btfss PIR1,RCIF
		goto $-1				; wait until a byte is received.

		; start timer 1 for 300uS delay
		;bsf T1CON,TMR1ON

		; Get address byte and save in RXADDR
		movfw	RCREG
		movwf	RXADDR

		bcf RCSTA,CREN			; diable reception of any reply


		; Now go and see if it's for us.
		movlw	MSBADDR			; get address this should respond to.
		xorwf 	RXADDR,W		; zero if it's our assigned MSB address
		btfss	STATUS,Z		; if zero flag set, skip goto ignore.
		goto	ignore
		bsf 	PORTB,RB3		; turn on debug LED to show we're responding to a poll.

		; Ok, if we get here we've been polled and we've got slightly less
		; than 300uS to figure out what value we're going to return.		
		; read input values

		; wait until 300 uS timer rolls over
		;btfss PIR1,TMR1IF
		;goto $-1
		;bcf T1CON,TMR1ON		; stop clock now

		

		; Transmit 3 bytes response
		banksel TXSTA
		bsf		TXSTA,TXEN		; enable transmission.

		banksel	BYTE0
		movfw	BYTE0
		banksel TXREG
		movwf 	TXREG ;into TXREG
		banksel PIR1
		btfss 	PIR1,TXIF ;test if TXREG empty
		goto 	$-1 ;wait until TXREG empty
		
		movfw 	BYTE1
		banksel TXREG
		movwf 	TXREG ;into TXREG
		banksel PIR1
		btfss 	PIR1,TXIF ;test if TXREG empty - will skip
		goto 	$-1 ;wait until TXREG empty
		
		movfw 	BYTE2
		banksel TXREG
		movwf 	TXREG ;into TXREG
		banksel PIR1
		btfss 	PIR1,TXIF ;test if TXREG empty
		goto 	$-1 ;wait until TXREG empty			
		
		; Wait for all the data to be transmitted back.
		banksel TXSTA
		btfss	TXSTA,TRMT		; will be set when data transmitted (not stop bit)
		goto	$-1

		; stop bit will take another 26 uS (at 38.4kbaud).
		banksel temp
		movlw	20				; roughly 60 cycles
		movwf	temp
		decfsz	temp,F			; usually one cycle
		goto	$-1				; usually 2 cycles

		banksel TXSTA			; bank 1
		bcf		TXSTA,TXEN		; disable transmission, TX/RB5 high impedance.
		banksel BYTE1

		goto mloop				; wait for next polling byte.

ignore:

		; wait until 300 uS timer rolls over
		;btfss PIR1,TMR1IF
		;goto $-1
		;bcf T1CON,TMR1ON
		
		; Another unit on the bus may be replying.  Wait until it responds
		; delay for at least 3 x 10 bits at 38.4kbaud - 782uS, make it 1mS
		;settim1 1000
		;bsf 	T1CON,TMR1ON		; start timer
		;btfss 	PIR1,TMR1IF			; overflowed yet?
		;goto 	$-1
		;bcf 	T1CON,TMR1ON		; stop timer

		; dump any spurious reads from other units
		movfw RCREG
		movfw RCREG
		movfw RCREG
		
		goto mloop				; wait for next polling byte.


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