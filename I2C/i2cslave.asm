;---------------------------------------------------------------------
; File: i2cslave.asm
; Test i2c slave functionality
;--------------------------------------------------------------------
;
;
;---------------------------------------------------------------------
;---------------------------------------------------------------------
; Include Files
;---------------------------------------------------------------------
			LIST   P=PIC16F818
			#include P16F818.INC
	
            ; Config - brownout on, code protect off, no code protect, no WDT, no LVP and internal clock.
			__CONFIG        _BODEN_ON & _CP_OFF &  _PWRTE_ON & _WDT_OFF & _LVP_OFF & _INTRC_IO
;---------------------------------------------------------------------
;Constant Definitions
;---------------------------------------------------------------------
#define NODE_ADDR 0x02 ; I2C address of this node

;---------------------------------------------------------------------
; Variable declarations
;---------------------------------------------------------------------
	udata
Index res 1 ; Internal address	
Temp res 1 ;

;---------------------------------------------------------------------
; Vectors
;---------------------------------------------------------------------
STARTUP code
	nop
	goto Startup ;

PROG code

;---------------------------------------------------------------------
; Main Code
;---------------------------------------------------------------------
Startup
	bcf STATUS,RP1
	bsf STATUS,RP0
	
	call Setup

Main 
	clrwdt ; Clear the watchdog timer.
	btfss PIR1,SSPIF ; Is this a SSP interrupt?
	goto Main;No - go back to main
	bcf PIR1,SSPIF
	call SSP_Handler ; Yes, service SSP event
    goto Main

;---------------------------------------------------------------------
; Setup
; Initializes program variables and peripheral registers.
;---------------------------------------------------------------------
Setup
	banksel PCON
	bsf PCON,NOT_POR
	bsf PCON,NOT_BOR
    
	banksel OSCCON ; Set oscillator to 8MHz
    bsf OSCCON, IRCF2
    bsf OSCCON, IRCF1
    bsf OSCCON, IRCF0

	banksel Index ; Clear various program variables
	clrf Index
	clrf PORTB
	clrf PIR1
	banksel TRISB
	clrf TRISB
	movlw 0x36 ; Setup SSP module for 7-bit
	banksel SSPCON
	movwf SSPCON ; address, slave mode
	movlw NODE_ADDR
	banksel SSPADD
	movwf SSPADD
	clrf SSPSTAT
	;banksel PIE1 ; Enable interrupts
	;bsf PIE1,SSPIE
	;bsf INTCON,PEIE ; Enable all peripheral interrupts
	;bsf INTCON,GIE ; Enable global interrupts
	bcf STATUS,RP0
	return

;---------------------------------------------------------------------
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
SSP_Handler
	banksel SSPSTAT
	movf SSPSTAT,W ; Get the value of SSPSTAT
	andlw b'00101101' ; Mask out unimportant bits in SSPSTAT.
	banksel Temp ; Put masked value in Temp
	movwf Temp ; for comparision checking.

State1: ; Write operation, last byte was an address
	movlw b'00001001' ; 
	xorwf Temp,W ;
	btfss STATUS,Z ; Are we in State1?
	goto State2 ; No, check for next state.....
	; TODO Process start of write
	clrf Index ; Clear the buffer index.
	call ReadI2C ; Do a dummy read of the SSPBUF.
	return

State2: ; Write operation, last byte was data,
	movlw b'00101001' ; buffer is full.
	xorwf Temp,W
	btfss STATUS,Z ; Are we in State2?
	goto State3 ; No, check for next state.....
	call ReadI2C ; Get the byte from the SSP.
    ; TODO - do something with written byte.
	return

State3: ; Read operation, last byte was an
	movlw b'00001100' ; address, buffer is empty.
	xorwf Temp,W
	btfss STATUS,Z ; Are we in State3?
	goto State4 ; No, check for next state.....
    ; TODO - get byte to return in W
	call WriteI2C ; Write the byte to SSPBUF
	return
State4: ; Read operation, last byte was data,
	movlw b'00101100' ; buffer is empty.
	xorwf Temp,W
	btfss STATUS,Z ; Are we in State4?
	goto State5 ; No, check for next state....
    ; TODO - get the next byte to read into W
	call WriteI2C ; Write to SSPBUF
	return
State5:
	movlw b'00101000' ; A NACK was received when transmitting
	xorwf Temp,W ; data back from the master. Slave logic
	btfss STATUS,Z ; is reset in this case. R_W = 0, D_A = 1
	goto I2CErr ; and BF = 0
	return 
; If we aren’t in State5, then something is
; wrong.
I2CErr 
	nop
	banksel PORTB ; Something went wrong! Set LED
	bsf PORTB,7 ; and loop forever. WDT will reset
	goto $ ; device, if enabled.
	return
;---------------------------------------------------------------------
; WriteI2C
;---------------------------------------------------------------------
WriteI2C
	banksel SSPSTAT
	btfsc SSPSTAT,BF ; Is the buffer full?
	goto WriteI2C ; Yes, keep waiting.
	banksel SSPCON ; No, continue.
DoI2CWrite
	bcf SSPCON,WCOL; Clear the WCOL flag.
	movwf SSPBUF ; Write the byte in WREG
	btfsc SSPCON,WCOL; Was there a write collision?
	goto DoI2CWrite
	bsf SSPCON,CKP ; Release the clock.
	return
;---------------------------------------------------------------------
;ReadI2C
;---------------------------------------------------------------------
ReadI2C
	banksel SSPBUF
	movf SSPBUF,W ; Get the byte and put in WREG
	return

	end ; End of file
