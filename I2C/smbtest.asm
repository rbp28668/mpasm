;---------------------------------------------------------------------
; File: smbtest.asm
; Test i2c/smbus slave functionality
;--------------------------------------------------------------------
;
;
;---------------------------------------------------------------------
;---------------------------------------------------------------------
; Include Files
;---------------------------------------------------------------------
			LIST   P=PIC16F818
			#include P16F818.INC
			#include smb.inc

; Config - brownout on, code protect off, no code protect, no WDT, no LVP and internal clock.
			__CONFIG        _BODEN_ON & _CP_OFF &  _PWRTE_ON & _WDT_OFF & _LVP_OFF & _INTRC_IO

;---------------------------------------------------------------------
;Constant Definitions
;---------------------------------------------------------------------
#define NODE_ADDR (0x60 << 1) ; I2C address of this node - 0x60 as 7 bit.

;---------------------------------------------------------------------
; Variables
;---------------------------------------------------------------------
	udata
reslo res 1	; low byte of result
reshi res 1 ; high byte of result
cmd res 1 ; command register


; SMB registers as follows:
;-------------------------------------------------------------------+
; Addr	|			Read				|		Write				|		
;-------+-------------------------------+---------------------------+
; 0		|	Software version			|	Command Byte			|
;-------+-------------------------------+---------------------------+
; 1		|	Low byte of result			|	NOP						|
;-------+-------------------------------+---------------------------+
; 2		|	High byte of result			|	NOP						|
;-------+-------------------------------+---------------------------+

;---------------------------------------------------------------------

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
	bcf STATUS,RP0
	bcf STATUS,RP1
	
	call Setup

Main 
	goto SMBHandler

;---------------------------------------------------------------------
; Setup
; Initializes program variables and peripheral registers.
;---------------------------------------------------------------------
Setup
    ;Set reset flags
	banksel PCON
	bsf PCON,NOT_POR
	bsf PCON,NOT_BOR
    
	;Set oscillator to 8MHz
    bsf OSCCON, IRCF2
    bsf OSCCON, IRCF1
    bsf OSCCON, IRCF0

    ; Disable interrupts.
    banksel PIR1
	clrf PIR1		; Clear all peripheral interrupt flags.
	bcf INTCON,PEIE ; Diable all peripheral interrupts
	bcf INTCON,GIE  ; Disable global interrupts

	; Set all port B outputs high (leds off) (DEBUG)
	movlw 0xff
	movwf PORTB

	; Init I2C passing slave address
	movlw NODE_ADDR
	goto SMBInit

;---------------------------------------------------------------------
; SMBRead - reads the byte with address in smbaddr
;---------------------------------------------------------------------
SMBRead
    movfw smbaddr ; debug - just return the address.
    return
	
;---------------------------------------------------------------------
; SMBWrite - writes the byte in W to address in smbaddr
;---------------------------------------------------------------------
SMBWrite
	xorlw 0xff
    banksel PORTB
	movwf PORTB
	return

;---------------------------------------------------------------------
; SMBRdEnd - signals the end of a read from the PIC
;---------------------------------------------------------------------
SMBRdEnd
    return

;---------------------------------------------------------------------
; SMBPoll - called from i2c polling loop.
;---------------------------------------------------------------------
SMBPoll
	return

	end ; of file.
