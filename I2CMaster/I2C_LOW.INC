;********************************************************************************************************
;
;				Low Level I2C Routines
;
;	Single Master Transmitter routines.  Adapted from the Microchip application note for
;   relocatable address, 7 bit addressing only and better configurability of ports.
;   Note i2c.h must be included before this.
;
;********************************************************************************************************



i2cMaster      code

i2cMaster   ; allows pagesel i2cMaster

;********************************************************************************************************
;	I2C Bus Initialization
;	Clears the I2C_PORT bits and releases the bus by setting the pins to input i.e. high-z
;********************************************************************************************************
InitI2CBus_Master:
	
    	banksel I2C_PORT
    	movf	I2C_PORT,W
    	andlw	~((1 << SCL_BIT) | (1 << SDA_BIT))		; do not use BSF & BCF on Port Pins
    	movwf	I2C_PORT		; set SDA & SCL to zero. From Now on, simply play with tris
    	RELEASE_BUS
    	banksel Bus_Status
    	clrf	Bus_Status	; reset status reg
    	clrf	Bus_Control	; clear the Bus_Control Reg, reset to 8 bit addressing	
    	return
;
;*********************************************************************************************************
; Send Start Bit
; Note that if other code has been using read-modify-write instructions such as BCF/BSF on I2C_PORT then
; it's likely that the I2C port bits have changed from 0 to 1 (as they're usually pulled high so will be
; read as high and written back as high).  Hence reset at the start of this routine as this is called
; to initialise every i2c transaction.
;
;*********************************************************************************************************

TxmtStartBit:
        banksel I2C_PORT
	    movf	I2C_PORT,W
	    andlw	~((1 << SCL_BIT) | (1 << SDA_BIT))		; do not use BSF & BCF on Port Pins
        movwf	I2C_PORT		; set SDA & SCL to zero. From Now on, simply play with tris
		bsf	STATUS,RP0		; select page 1 for I2C_TRIS
		bsf	_SDA_TRIS		; set SDA high
		bsf	_SCL_TRIS		; clock is high
; Setup time for a REPEATED START condition (4.7 uS)
        call	Delay40uSec	; only necesry for setup time
		bsf	STATUS,RP0		; select page 1 for I2C_TRIS
		bcf	_SDA_TRIS		; give a falling edge on SDA while clock is high
		call	Delay47uSec	; only necessary for START HOLD time
		banksel Bus_Status
		bsf	_Bus_Busy	; on a start condition bus is busy
		return


;*********************************************************************************************************
;					Send Stop Bit
;
;*********************************************************************************************************

TxmtStopBit:
		banksel I2C_TRIS		; select page 1 for I2C_TRIS
		bcf	_SCL_TRIS		
		bcf	_SDA_TRIS		; set SDA low
		bsf	_SCL_TRIS		; Clock is pulled up
		call	Delay40uSec	; Setup Time For STOP Condition 
		banksel I2C_TRIS		; select page 1 for I2C_TRIS
		bsf	_SDA_TRIS		; give a rising edge on SDA while CLOCK is high
		; delay to make sure a START bit is not sent immediately after a STOP, ensure BUS Free Time tBUF
		call	Delay47uSec	
		banksel Bus_Status
		bcf	_Bus_Busy	; on a stop condition bus is considered Free
;
		return	


;*********************************************************************************************************
;					Abort Transmission
;
;   Send STOP Bit & set Abort Flag
;*********************************************************************************************************

AbortTransmission:

		call	TxmtStopBit
        banksel Bus_Status
		bsf	_Abort
		return	

;*********************************************************************************************************
;				Transmit Address (1st Byte)& Put in Read/Write Operation
;
;  Transmits Slave Addr On the 1st byte.
;  Slave Address is passed in W - ls bit determines Read or ~Write - see READ(addr) and WRITE(addr)
;  macros in i2c.h
;
;  On Success, return TRUE in WREG, else FALSE in WREG
;
;   If desired, the failure may tested by the bits in Bus Status Reg
;
;*********************************************************************************************************

Txmt_Slave_Addr:
	banksel Bus_Status
	bcf	_ACK_Error		; reset Acknowledge error bit

EndTxmtAddr:
	call	SendData		; send 8 bits of address, bus is our's
;
; if successfully transmitted, expect an ACK bit
;
	banksel Bus_Status
_AddrSendTest:
	btfss	_Txmt_Success		; skip if successful
	goto	_AddrSendFail
	clrwdt
	retlw	TRUE
;
_AddrSendFail:
	clrwdt
	btfss	_ACK_Error
	retlw	FALSE			; Addr Txmt Unsuccessful, so return 0
;
; Address Not Acknowledged, so send STOP bit
;					
	call	TxmtStopBit
	retlw	FALSE			; Addr Txmt Unsuccessful, so return 0
;
;*********************************************************************************************************
;				Transmit A Byte Of Data
;
; The data to be transmitted is passed in W.
; Clock stretching is allowed by slave. If the slave pulls the clock low, then, the stretch is detected.
;*********************************************************************************************************
SendData:

;
; TXmtByte & Send Data are same, Can check errors here before calling TxmtByte
; For future compatibility, the user MUST call SendData & NOT TxmtByte
;
	goto	TxmtByte
			
;
TxmtByte:
		banksel Bus_Status
        movwf	DataByteCopy	; make copy of DataByte byte to send.
		bsf	_Txmt_Progress		; set Bus status for txmt progress
		bcf	_Txmt_Success		; reset status bit
		movlw	0x08
		movwf	BitCount

TxmtNextBit:
		clrwdt					; clear WDT, set for 18 mSec
		banksel I2C_TRIS		; Manipulating TRIS - bank 1
		bcf	_SCL_TRIS
		banksel Bus_Status
		rlf     DataByteCopy, F	; MSB first, Note DataByte Is Lost
		banksel I2C_TRIS
        bcf	_SDA_TRIS
		btfsc	STATUS,C
		bsf	_SDA_TRIS
		call	Delay47uSec		; guareentee min LOW TIME tLOW & Setup time
		banksel I2C_TRIS		; select page 1 for I2C_TRIS
        bsf	_SCL_TRIS			; set clock high , check if clock is high, else clock being stretched
		call	Delay40uSec		; guareentee min HIGH TIME tHIGH
		banksel Bus_Status
		bcf	_TIME_OUT_	; reset timeout error flag

		;Check for clock stretch on each bit.
Check_SCL_1:
		banksel I2C_PORT
		btfss	_SCL		; if clock not being stretched, it must be high
		goto	Check_SCL_1	; loop until SCL high or TMR0 timeout interrupt
        banksel BitCount
		decfsz	BitCount, F
		goto	TxmtNextBit
;
; Check For Acknowledge
;
		banksel I2C_TRIS
		bcf	_SCL_TRIS		; reset clock
		bsf	_SDA_TRIS		; Release SDA line for Slave to pull down
		call Delay47uSec	; guareentee min LOW TIME tLOW & Setup time
		banksel I2C_TRIS
		bsf	_SCL_TRIS		; clock for slave to ACK
		call Delay40uSec	; guareentee min HIGH TIME tHIGH	
		banksel I2C_PORT	; select PAGE 0 to test I2C_PORT pin SDA
		btfsc	_SDA		; SDA should be pulled low by slave if OK
        goto	_TxmtErrorAck
;
		banksel I2C_TRIS
		bcf	_SCL_TRIS		; reset clock

		banksel Bus_Status
		bcf	_Txmt_Progress	; reset TXMT bit in Bus Status
		bsf	_Txmt_Success	; transmission successful
		bcf	_ACK_Error	; ACK OK
		return
_TxmtErrorAck:
		RELEASE_BUS
		banksel Bus_Status
		bcf	_Txmt_Progress	; reset TXMT bit in Bus Status
		bcf	_Txmt_Success	; transmission NOT successful
		bsf	_ACK_Error	; No ACK From Slave
		return
;
;*********************************************************************************************************
;
;				Receive  A Byte Of Data From Slave
;
;  assume address is already sent
;  if last byte to be received, then set _Last_Byte_Rcv bit so that the master does not acknowledge slave.
;  Data Received on successful reception is returned in W.
;
;
;*********************************************************************************************************
;

GetData:
		goto	RcvByte
;
RcvByte:
		
		banksel Bus_Status
		bsf	_Rcv_Progress	; set Bus status for txmt progress
		bcf	_Rcv_Success	; reset status bit

		movlw	0x08
		movwf	BitCount

RcvNextBit:
		clrwdt				; clear WDT, set for 18 mSec
		banksel I2C_TRIS	; page 1 for TRIS manipulation
		bcf	_SCL_TRIS
		bsf	_SDA_TRIS		; can be removed from loop
		call	Delay47uSec	; guareentee min LOW TIME tLOW & Setup time
        banksel I2C_TRIS
		bsf	_SCL_TRIS		; clock high, data sent by slave
		call	Delay40uSec	; guareentee min HIGH TIME tHIGH
		banksel Bus_Status
		bcf	_TIME_OUT_		; reset timeout error flag
		banksel I2C_PORT	; ports for reading clock stretch.

Check_SCL_2:
		btfss	_SCL		; if clock not being stretched, it must be high
		goto	Check_SCL_2	; loop until SCL high or TMR0 timeout interrupt
		bcf	STATUS,C
		btfsc	_SDA
		bsf	STATUS,C
		banksel DataByteCopy
        rlf	DataByteCopy, F	; left shift data ( MSB first)
		decfsz	BitCount, F
		goto	RcvNextBit
;
; Generate ACK bit if not last byte to be read,
; if last byte Gennerate NACK ; do not send ACK on last byte, main routine will send a STOP bit
;
		; Move Last_Byte_Rcv flag into C as then readable in bank 1
		banksel Bus_Status
		bcf		STATUS,C
		btfsc	_Last_Byte_Rcv
		bsf		STATUS,C
		
		; Generate ACK/NACK depending on C (Last_Byte_Rcv)
		banksel I2C_TRIS	; for TRIS
		bcf	_SCL_TRIS
		btfss   STATUS,C
		bcf	_SDA_TRIS		; ACK by pulling SDA low
		btfsc	STATUS,C ; 
		bsf	_SDA_TRIS			; if last byte, send NACK by setting SDA high

		call	Delay47uSec	; guareentee min LOW TIME tLOW & Setup time
        banksel I2C_TRIS
		bsf	_SCL_TRIS
		call	Delay40uSec	; guareentee min HIGH TIME tHIGH
RcvEnd:
        banksel I2C_TRIS
		bcf	_SCL_TRIS		; reset clock

		banksel Bus_Status
		bcf	_Last_Byte_Rcv	; reset for next read.
		bcf	_Rcv_Progress	; reset TXMT bit in Bus Status
		bsf	_Rcv_Success	; transmission successful
		bcf	_ACK_Error	; ACK OK

		movfw DataByteCopy
		return



;*********************************************************************************************************
;			General Purpose Delay Routines
;
;  Delay4uS	is wait loop for 4.0 uSec
;  Delay47uS	is wait loop for 4.7 uSec
;  Delay50uS	is wait loop for 5.0 uSec
;
;*********************************************************************************************************
;

Delay50uSec:
	movlw	((_50uS_Delay-5)/3 + 1)
DlyK
    banksel DelayCount
	movwf	DelayCount
	decfsz	DelayCount, F
	goto	$-1
	return		
;
Delay47uSec:
	movlw	((_47uS_Delay-8)/3 + 1)
    goto	DlyK
;
Delay40uSec:
	movlw	((_40uS_Delay-8)/3 + 1)
	goto	DlyK
;
;*********************************************************************************************************
