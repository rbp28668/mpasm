;******************************************************
;Robot Collision Detection
;Target: PIC16F818
;Link with SMBSlave.o
;******************************************************

#include p16f818.inc
#include "../i2c/smb.inc"

		__CONFIG _BODEN_ON & _CP_OFF & _PWRTE_ON & _WDT_OFF & _LVP_OFF & _INTRC_IO


;=======================================================
; Constants
;=======================================================

NODE_ADDR	EQU (0x62 << 1)


;=======================================================
; Variables
;=======================================================
		udata
sticky	res 1					; sticky bits to latch collisions

;=======================================================
; Code
;=======================================================


;---------------------------------------------------------------------
; Vectors
;---------------------------------------------------------------------
STARTUP	code   				; 
		nop
		goto Startup

PROG	code					; wherever the linker wants to put this

;---------------------------------------------------------------------
; Main Code
;---------------------------------------------------------------------
Startup:
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
		
		; Ports A and B both input
		banksel ADCON1
		movlw	0x06			; All pins as digital inputs
		movwf	ADCON1
		banksel TRISA
		movlw	0xFF			; Inputs:
		movwf	TRISA
     	movwf   TRISB         
		bcf		STATUS,RP0
	
		; clear sticky byte
		banksel sticky
		clrf 	sticky

		movlw 	NODE_ADDR
		call 	SMBInit	
		goto 	SMBHandler

;---------------------------------------------------------------------
; SMBRead - read a byte from device, current addr in smbaddr
;---------------------------------------------------------------------
SMBRead
		banksel smbaddr
		movfw	smbaddr
		xorlw	0
		btfss STATUS,Z
		goto SMBR1
		; Read of location 0
		movlw 0x01		; software revision 0.1		
		goto SMBRET

SMBR1
		movfw	smbaddr
		xorlw	1
		btfss STATUS,Z
		goto SMBR2
		; Read of location 1
		call RdLines ; get current state of lines in W
		goto SMBRET

SMBR2
		movfw	smbaddr
		xorlw	2
		btfss STATUS,Z
		goto SMBERR
		; Read of location 2
		banksel sticky
		movfw sticky
		goto SMBRET

SMBERR  ; Some error such as invalid address.
		movlw 0xff	; error return of FF
SMBRET
		return

;---------------------------------------------------------------------
; SMBWrite - write a byte to device, current addr in smbaddr
;---------------------------------------------------------------------
SMBWrite
		; Ignore address, just reset the sticky bits.
		banksel sticky
		clrf sticky
		return

;---------------------------------------------------------------------
; SMBRdEnd - signal the end of a read 
;---------------------------------------------------------------------
SMBRdEnd
		return

;---------------------------------------------------------------------
; SMBPoll reads the lines
;---------------------------------------------------------------------
SMBPoll
		call RdLines
		banksel sticky
		iorwf sticky,F
		return

; Reads the input lines and returns the results in W
; Note that the result is inverted as collision detect
; bits are active low.
RdLines
		banksel PORTB
		clrw
		; Bit 7
		btfss	PORTB,0 
		iorlw B'10000000'
		; Bit 6
		btfss	PORTA,2  
		iorlw B'01000000'
		; Bit 5
		btfss	PORTA,3  
		iorlw B'00100000'
		; Bit 4
		btfss	PORTA,4  
		iorlw B'00010000'
		; Bit 3
		btfss	PORTA,1  
		iorlw B'00001000'
		; Bit 2
		btfss	PORTA,0  
		iorlw B'00000100'
		; Bit 1
		btfss	PORTA,7  
		iorlw B'00000010'
		; Bit 0
		btfss	PORTA,6  
		iorlw B'00000001'
		return


		end