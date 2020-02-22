;
; Test program for SFR08 display on standard 2 line by 16 char LCD display in 4-bit mode.
;
	LIST P=16F818
#include p16f818.inc
		__CONFIG _BODEN_ON & _CP_OFF & _PWRTE_ON & _WDT_OFF & _LVP_OFF & _INTRC_IO


;=======================================================
; Constants
; Note - these need to be set to configure the hardware configuration
; of the PIC driving the LCD.
;=======================================================

_ClkIn			EQU		D'8000000'		; Processor clock frequency.


LCD_DATA         EQU     PORTA          ; Uses LS 3 bits of port - update 
LCD_DATA_TRIS    EQU     TRISA
LCD_CNTL         EQU     PORTB
LCD_DATA_MASK	 EQU	 0x0F			; which bits of the port are used for data bits

; LCD Display Commands and Control Signal names on PORTB
E				EQU		5				; LCD Enable control line
RW				EQU		3				; LCD Read/Write control line
RS				EQU		0				; LCD Register Select control line
;

; Which bits and ports are used for I2C.  Note that the ones shown below correspond to the
; bits and port used for the 16F818 slave I2C bits.  These MUST be defined before including I2C.H
#define I2C_PORT	PORTB
#define I2C_TRIS	TRISB
#define SCL_BIT		4
#define SDA_BIT		1


#define	SlaveAddr	0xE0		; Default address of SRF08

		#include "../I2CMaster/I2C.H"


; I2C registers and commands for SRF08
COMMAND_REG     EQU     0
GAIN_REG        EQU     1
RANGE_REG       EQU     2

REVISION_REG    EQU     0
LIGHT_REG       EQU     1
ECHO_REG        EQU     2

RANGE_IN        EQU     0x50
RANGE_CM        EQU     0x51
RANGE_US        EQU     0x52


            udata
Revision    res 1   ; software revision.
Light		res 1	; Light reading
Range		res 2	; Range in cm.

; variables for math
temp        res 1
counter     res 1
bcd         res 3   ; BCD result of conversion.

       		code     0                 ; RESET vector location
RESET       GOTO    START              ;
;
; This is the Periperal Interrupt routine. Should NOT get here
;
isr        	code     4              ; Interrupt vector location
			retfie


    		; Bring in I2C low level routines.
            include "../I2CMaster/i2c_low.inc"

            ; Bring in LCD driving
            include "../lcd/lcd.inc"

            ; Math routines also
            include "../math/math.inc"

            Bin16toBCDDef bcd,Range ; convert 16 bit range to packed BCD
            Bin8toBCDDef  bcd,Light ; convert 8 bit light to packed BCD.

; Main start location.
START                               ; POWER_ON Reset (Beginning of program)
            CLRF    STATUS          ; Do initialization (Bank 0)
            CLRF    INTCON
            CLRF    PIR1

			banksel OSCCON
			MOVLW	B'01110000'		; Set 8MHz internal clock (01110000 for 8MHz)
			MOVWF	OSCCON

            BSF     STATUS, RP0     ; Bank 1
            MOVLW   0x00            ; The LCD module does not like to work w/ weak pull-ups
            MOVWF   OPTION_REG      ;
            CLRF    PIE1            ; Disable all peripheral interrupts
            MOVLW   0xFF            ;
            MOVWF   ADCON1          ; Port A is Digital.
;
;
            BCF     STATUS, RP0     ; Bank 0
            CLRF    PORTA           ; ALL PORT output should output Low.
            CLRF    PORTB
            BCF     T1CON, TMR1ON   ; Timer 1 is NOT incrementing
;
            BSF     STATUS, RP0     ; Select Bank 1
            CLRF    TRISA           ; RA5 -  0 outputs
            MOVLW   B'11010010'     ; Bits 0, 3 and 5 outputs for controls.
                                    ; Leave bits 1 and 4 inputs for I2C master
            MOVWF   TRISB           ; RB7 - 4 inputs, RB3 - 0 outputs 
            BCF     STATUS, RP0     ; Select Bank 0


            ; Initialise the LCD
            bcf     PORTB,2          ; ground contrast signal.
            movlw   0               ; all of port A to be output
			CALL	LCD_INIT

            movlw   CLR_DISP
            call    SEND_CMD

            movlw   DISP_ON
            call    SEND_CMD

            ; Initialise I2C
            movlw   0 ; zero all of port B
    		CALL 	InitI2CBus_Master ; call this after initial setup.
    
            ; Read SRF08
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

            bcf     STATUS,RP0
            bsf     PORTA,4

    		CALL    TxmtStartBit
    		MOVLW	WRITE(SlaveAddr)
    		CALL	Txmt_Slave_Addr
    		MOVLW	COMMAND_REG
    		CALL	SendData
    		MOVLW	RANGE_CM
    		CALL	SendData
    		CALL    TxmtStopBit
    
            bcf     STATUS,RP0
            bcf     PORTA,4
    
    		; Now loop reading software register until we get something
    		; sensible (not FF) back which means that the ranger has
    		; finished ranging.
            bcf     STATUS,RP0
            bsf     PORTA,6
WaitResult:


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
    
            bcf     STATUS,RP0
            bcf     PORTA,6

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

            ; Note - if we loop here it reads repeatedly.
    		;GOTO	StartRange      ; Read next range.


            movlw   DD_RAM_UL
            call    SEND_CMD
            movlw   'R'
            call    SEND_CHAR

            ; But not here!
    		;GOTO	StartRange      ; Read next range.
           
            banksel bcd
            call    Bin16toBCD      ; Get range as packed BCD

            ;write to LCD MS digit first
            banksel bcd
            movfw   bcd+2
            andlw   0x0F
            addlw   '0'
            call    SEND_CHAR

            banksel bcd
            swapf   bcd+1,W
            andlw   0x0F
            addlw   '0'
            call    SEND_CHAR

            banksel bcd
            movfw   bcd+1
            andlw   0x0F
            addlw   '0'
            call    SEND_CHAR

            banksel bcd
            swapf   bcd,W
            andlw   0x0F
            addlw   '0'
            call    SEND_CHAR

            banksel bcd
            movfw   bcd
            andlw   0x0F
            addlw   '0'
            call    SEND_CHAR

            ; units text (cm)
            movlw   'c'
            call    SEND_CHAR

            movlw   'm'
            call    SEND_CHAR


            ; Write out light to second line.
            movlw   DD_RAM_UL + 0x40
            call    SEND_CMD

            movlw   'L'
            call    SEND_CHAR

            banksel bcd
            call    Bin8toBCD       ; Get light as packed BCD
            ;123 -> 0x23 in LSB, 0x01 in MSB

            movfw   bcd+1
            andlw   0x0F
            addlw   '0'
            call    SEND_CHAR

            banksel bcd
            swapf   bcd,W
            andlw   0x0F
            addlw   '0'
            call    SEND_CHAR

            banksel bcd
            movfw   bcd
            andlw   0x0F
            addlw   '0'
            call    SEND_CHAR


    		GOTO	StartRange      ; Read next range.

            end




