;===========================================================
; I2CMaster.asm
; Test program for I2CMaster.  Uses Devantech SRF08 sonar
; as slave for testing.
;===========================================================

        TITLE "I2CMaster Test"
        LIST P=PIC16F818
        include "P16F818.inc"
       	

		__CONFIG _CCP1_RB3 & _BODEN_ON & _CP_OFF & _PWRTE_ON & _WDT_OFF & _LVP_OFF & _INTRC_IO        

		radix dec


; Define 8MHz clock to derive i2C timings. This MUST be defined before including I2C.H
_ClkIn		equ	8000000

; Which bits and ports are used for I2C.  Note that the ones shown below correspond to the
; bits and port used for the 16F818 slave I2C bits.  These MUST be defined before including I2C.H
#define I2C_PORT	PORTB
#define I2C_TRIS	TRISB
#define SCL_BIT		4
#define SDA_BIT		1


#define	SlaveAddr	0xE0		; Default address of SRF08

		#include "I2C.H"

;====  Hardware bits ===
COMMAND_REG     EQU     0
GAIN_REG        EQU     1
RANGE_REG       EQU     2

REVISION_REG    EQU     0
LIGHT_REG       EQU     1
ECHO_REG        EQU     2

RANGE_IN        EQU     0x50
RANGE_CM        EQU     0x51
RANGE_US        EQU     0x52

;=======================================================
; Variables.
;=======================================================
	udata 
WSAVE		res 1	; ISR save W
STATSAVE 	res 1	; ISR save status
STATFLAG	res 1	; Status flags for signalling ISR

temp		res 1	; misc temp register.
Revision    res 1   ; software revision.
Light		res 1	; Light reading
Range		res 2	; Range in cm.


; Statflag bit assignment
T0INT		equ 0	; use bit 0 for signalling Timer 0 interrupt.


;=======================================================
; Initial startup and ISR vectors
    
        ; Startup entry point
STARTUP	code 0
        GOTO    MAIN            ; startup
        
		; Interrupt entry point
        code     H'0004'          ; Interrupt service routine     
        GOTO    INTSVC
        

PROG	code

		; Bring in I2C low level routines.
        include "i2c_low.inc"

;=======================================================
; Interrupt Service
     
INTSVC:        
        MOVWF   WSAVE           ; Copy W to a temporary register
        SWAPF   STATUS,W        ; Swap Status Nibbles and move to W 
        MOVWF   STATSAVE      ; Copy STATUS to a temporary register
        BCF     STATUS, RP0     ; Force Bank 0
        BCF     STATUS, RP1     ; 




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
		MOVLW	B'01110000'		; Set 8MHz internal clock
		MOVWF	OSCCON

		banksel PIR1
		clrf PIR1

        bsf     STATUS,RP0      ; Bank 1.

        MOVLW   0xFF            ;
        MOVWF   ADCON1          ; Port A is Digital.

		MOVLW	H'00'			; All outputs
        MOVWF   TRISA          	; 

		;RB1 (SDA) and RB4 (SCL) must be set as input (bits set).
		MOVLW	H'12'			; RB1 and RB4 as inputs for I2C, rest outputs
        MOVWF   TRISB           ; Set direction for B
        bcf     STATUS,RP0
                
		CALL 	InitI2CBus_Master ; call this after initial setup.

        ; Read revision register
		CALL    TxmtStartBit
		MOVLW	WRITE(SlaveAddr)
		CALL	Txmt_Slave_Addr
		MOVLW	REVISION_REG				; Software Version register
		CALL	SendData
		CALL 	TxmtStartBit	; Restart
		MOVLW	READ(SlaveAddr)
		CALL	Txmt_Slave_Addr
		banksel Bus_Status
		BSF		_Last_Byte_Rcv	; Only reading one byte.
		CALL	GetData
        MOVWF   Revision
		CALL	TxmtStopBit

		; Initiate a ranging command.
StartRange:
		CALL    TxmtStartBit
		MOVLW	WRITE(SlaveAddr)
		CALL	Txmt_Slave_Addr
		MOVLW	COMMAND_REG
		CALL	SendData
		MOVLW	RANGE_CM
		CALL	SendData
		CALL    TxmtStopBit


		; Now loop reading software register until we get something
		; sensible (not FF) back which means that the ranger has
		; finished ranging.
WaitResult:
        BSF     PORTA,4
		CALL    TxmtStartBit
		MOVLW	WRITE(SlaveAddr)
		CALL	Txmt_Slave_Addr
		MOVLW	REVISION_REG				; Software Version register
		CALL	SendData
		CALL 	TxmtStartBit	; Restart
		MOVLW	READ(SlaveAddr)
		CALL	Txmt_Slave_Addr
		banksel Bus_Status
		BSF		_Last_Byte_Rcv	; Only reading one byte.
		CALL	GetData
		XORLW	0xFF			; if FF then slave busy and not responding - zero flag is set.
		BTFSC	STATUS,Z		; skip over loop if zero flag is clear (i.e. we didn't get FF).
		GOTO	WaitResult
		CALL	TxmtStopBit
        BCF     PORTA,4

		; Get the various results.
		CALL    TxmtStartBit
		MOVLW	WRITE(SlaveAddr)
		CALL	Txmt_Slave_Addr
		MOVLW	LIGHT_REG				; Light register
		CALL	SendData
		CALL 	TxmtStartBit	; Restart
		MOVLW	READ(SlaveAddr)
		CALL	Txmt_Slave_Addr
		CALL	GetData			; light value
		banksel Light
		MOVWF	Light
		CALL	GetData			; Echo high byte
		banksel Range
		MOVWF	Range+1
		BSF		_Last_Byte_Rcv
		CALL	GetData			; Echo low byte
		banksel Range
		MOVWF	Range				
		CALL	TxmtStopBit

		GOTO	StartRange
		End