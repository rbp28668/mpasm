;---------------------------------------------------------------------
; File: smbslave.asm
; Wrapper for I2C driven slave device.  Note this is boiler plate code
; which uses the inversion of control pattern to call application
; specific code which implements the calls defined in smb.inc namely
; SMBRead
; SMBWrite
; SMBRdEnd
; To use this module, define the routines above, call SMBInit with the
; node address (shifted left to make an 8 bit addr) in W, then goto
; SMBHandler.  SMBHandler will then call SMBRead, SMBWrite and SMBRdEnd
; appropriately.  Note application code should read smbaddr to see where
; a read or write should come from or go to but should not change it.
; Note that this code is for PIC16 family only - see AN734 for changes
; for PIC18 family.
;--------------------------------------------------------------------
;
;
;---------------------------------------------------------------------
;---------------------------------------------------------------------
; Include Files
;---------------------------------------------------------------------
			LIST   P=PIC16F818
			#include P16F818.INC
			#define _SMBSLAVE
			#include smb.inc
	
;---------------------------------------------------------------------
; Variable declarations
;---------------------------------------------------------------------
		udata
smbidx 	res 1 	; smbidx of bytes read/received
smbaddr res 1	; SMBUS address - don't confuse with I2C address!
smbtmp 	res 1 ;



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

		code

;---------------------------------------------------------------------
; SMBInit
; Initializes program variables and peripheral registers for I2C slave.
; Pass node address (as 8 bit) in W
;---------------------------------------------------------------------
SMBInit
	banksel smbtmp	; save slave address in smbtmp
	movwf smbtmp
 
	;RB1 (SDA) and RB4 (SCL) must be set as input (bits set).
	banksel TRISB
	movf TRISB,W
	iorlw b'00010010' ; Bits 4 and 1 as inputs.
	movwf TRISB

	; Configure SSP control
	banksel SSPCON
	movlw 0x36 ; Setup SSP module for 7-bit:
				; SMP = 0 (Not i2c)
				; CKE = 0 (not i2c)
				; SSPEN = 1 - enable SSP module
				; CKP = 1 - allow clock to float so master can drive it.
				; 0110 = I2C Slave mode, 7-bit address
	movwf SSPCON ; address, slave mode

	; Set up slave address previously saved in smbtmp
	banksel smbtmp
	movfw smbtmp
	banksel SSPADD
	movwf SSPADD
	
	banksel SSPSTAT
	clrf SSPSTAT

	banksel PIE1 ; Disable SSP interrupts as we poll.
	bcf PIE1,SSPIE

	banksel PIR1 ; Reset any outstanding serial interupt
        bcf PIR1,SSPIF

	banksel smbidx ; Clear various program variables
	clrf smbidx
        clrf smbaddr
        clrf smbtmp

	return

;---------------------------------------------------------------------
; SMBHandler
; Main I2C slave handler.
;---------------------------------------------------------------------
SMBHandler
        call SMBPoll            ; polling loop.
        clrwdt                  ; Clear the watchdog timer.
        banksel PIR1
        btfsc   PIR1,SSPIF      ; Is this a SSP interrupt?
        call    SMBHandler      ; Yes, service SSP event
        goto SMBHandler

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
        banksel PIR1            ; bank 0
        bcf     PIR1,SSPIF	; reset interrupt flag.

        bsf     STATUS,RP0	; bank1 for SSPSTAT
        movf    SSPSTAT,W ; Get the value of SSPSTAT
        andlw   (SSPS+SSPDA+SSPRW+SSPBF) ; Only want Start, D/A, R/W and BF bits
        banksel smbtmp          ; Put masked value in smbtmp
        movwf   smbtmp          ; for comparision checking.
State1: 
        ; Write operation, last byte was an address
        ; Having just received the address byte for this device we
        ; reset our internal byte counter so we know to pick up the
        ; first data byte as a SMB internal address.
        movlw (SSPS + SSPBF); (address + write) 
        xorwf smbtmp,W ;
        btfss STATUS,Z ; Are we in State1?
        goto State2 ; No, check for next state.....
        banksel smbidx
        clrf smbidx ; Clear the receive index.
        call ReadI2C ; Do a dummy read of the SSPBUF.
        banksel SSPCON
        bsf SSPCON,CKP ; Release the clock 
        return

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
        banksel smbidx
        movf smbidx,F ; move smbidx to itself - test for zero
        btfss STATUS,Z ; if zero flag is set - byte is smb address
        goto S2wr
        banksel smbaddr
        movwf smbaddr ; save address byte
        banksel smbidx
        incf smbidx,F
        return
S2wr:
        pagesel SMBWrite ; write byte to current SMB address
        call SMBWrite
        banksel smbidx
        incf smbidx,F
	banksel smbaddr
        incf smbaddr,F
        return

State3: 
        ; Read operation, last byte was an address byte
        ; This is putting the device into read mode.  There must
        ; have been a write to set up the address before this
        ; Address is set up in smbaddr, calls SMBRead which 
        ; should return with the data to be sent back in w.
        movf  smbtmp,W    
        andlw (SSPS+SSPDA+SSPRW) ; Mask out BF bit in SSPSTAT
        xorlw (SSPS+SSPRW)
        btfss STATUS,Z ; Are we in State3?
        goto State4 ; No, check for next state.....
        pagesel SMBRead ;get byte to return in W
        call SMBRead
        call WriteI2C ; Write the byte to SSPBUF
        banksel smbidx
        incf smbidx,F
        banksel smbaddr
        incf smbaddr,F
        return

State4: 
        ; Read operation, last byte was data,
        ; Continued read operation and, as we had an ack,
        ; master is expecting another byte.
        ; current read address in smbaddr.
        movlw (SSPS+SSPDA+SSPRW) ; buffer is empty.
        xorwf smbtmp,W
        btfss STATUS,Z ; Are we in State4?
        goto State5 ; No, check for next state....
        pagesel SMBRead ;get byte to return in W
        call SMBRead
        call WriteI2C ; Write to SSPBUF
        banksel smbidx
        incf smbidx,F
        banksel smbaddr
        incf smbaddr,F
        return

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
        pagesel SMBRdEnd
        call SMBRdEnd
	return 


; If we aren’t in State5, then something is
; wrong.
I2CErr 
        nop
        goto $ ; device, if enabled.
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

	end ; End of file

