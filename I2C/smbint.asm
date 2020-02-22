

; Basics of interrupt code.

; Bitmasks for SSPSTAT
SSPSMP  	equ 0x80
SSPCKE		equ 0x40
SSPDA		equ 0x20
SSPP		equ 0x10
SSPS		equ 0x08
SSPRW		equ 0x04
SSPUA		equ 0x02
SSPBF   	equ 0x01

; SMBUS comms.
smbidx 		res 1 	; smbidx of bytes read/received
smbaddr 	res 1	; SMBUS address - don't confuse with I2C address!
smbtmp 		res 1 	;


; Setup
		; Configure SSP control for I2C
		bcf		STATUS,RP0		; bank 0 for SSPCON
		movlw H'36' ; Setup SSP module for 7-bit:
					; SMP = 0 (Not i2c)
					; CKE = 0 (not i2c)
					; SSPEN = 1 - enable SSP module
					; CKP = 1 - allow clock to float so master can drive it.
					; 0110 = I2C Slave mode, 7-bit address
		movwf SSPCON ; address, slave mode

		; Set up slave address previously saved in smbtmp
		bsf		STATUS,RP0		; Bank 1 for SSPADD & SSPSTAT
		movlw	NODE_ADDDR	; hard wire slave address
		movwf SSPADD
		clrf SSPSTAT
	
		bcf		STATUS,RP0		; bank 0 for PIR1
	    bcf PIR1,SSPIF ; Reset any outstanding serial interupt

		; enable SPI interrupts
		bsf		STATUS,RP0		; bank 1 for PIE1
		bsf		PIE1,SSPIE		; Enable SSP interrupts.



;---------------------------------------------------------------------
; I2C interrupt core.
; Uses 5 states dependent on the codes in SSPSTAT
; Bit 7 - ignore in i2C
; Bit 6 - ignore in I2C
; Bit 5 - D_A, 1 = last byte was data, 0 = last byte was address
; Bit 4 - P 1 = stop bit was detected last otherwise 0
; Bit 3 - S 1 = start bit was detected last
; Bit 2 - R_W, 1 = read on last address, 0 = write
; Bit 1 - ignore if using 7 bit addressing
; Bit 0 - Buffer full 1 = receive complete or transmitting, 0 buffer empty.
; So only actually intersted in DA, S, R_W, BF
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
	banksel PIR1
	btfss 	PIR1,SSPIF ; Is this a SSP interrupt?
	goto 	NotI2C 	 ; No - skip SSP handler

	bcf 	PIR1,SSPIF	; clear interrupt flag

SSP_Handler
	banksel SSPSTAT
	movf 	SSPSTAT,W ; Get the value of SSPSTAT
	andlw 	(SSPS+SSPDA+SSPRW+SSPBF) ; Only want Start, D/A, R/W and BF bits
	banksel smbtmp ; Put masked value in smbtmp
	movwf 	smbtmp ; for comparision checking.
State1: 
    ; Write operation, last byte was an address
	; Having just received the address byte for this device we
	; reset our internal byte counter so we know to pick up the
	; first data byte as a SMB internal address.
	movlw 	(SSPS + SSPBF); (address + write) 
	xorwf 	smbtmp,W ;
	btfss 	STATUS,Z ; Are we in State1?
	goto 	State2 ; No, check for next state.....
	;showstate 1
	clrf 	smbidx ; Clear the receive index.
	call 	ReadI2C ; Do a dummy read of the SSPBUF - will have addr value.
    banksel SSPCON
	bsf SSPCON,CKP ; Release the clock  - (shouldn't need this).
	goto 	I2CEnd

State2: 
    ; Write operation, last byte was data.
	; Device selected and smbidx keeps track of the count of 
	; bytes received.  When index is 0 it's an internal address
	; byte so we save it in smbaddr. Otherwise we call SMBWrite
	; with the received byte in W
	movlw 	(SSPS + SSPDA + SSPBF) ; buffer is full.
	xorwf 	smbtmp,W
	btfss 	STATUS,Z ; Are we in State2?
	goto 	State3 ; No, check for next state.....
	;showstate 2
	call 	ReadI2C ; Get the byte from the SSP.
    ;banksel SSPCON ; Now we've read the byte... (shouldn't need this)
	;bsf SSPCON,CKP ; Release the clock 
    banksel smbidx
	movf 	smbidx,F ; move smbidx to itself - test for zero
	btfss 	STATUS,Z ; if zero flag is set - byte is smb address
    goto 	S2wr
    movwf 	smbaddr ; save address byte
    incf 	smbidx,F
	goto 	I2CEnd
S2wr:
    call 	SMBWrite
	banksel smbidx
    incf 	smbidx,F
	incf 	smbaddr,F
    goto 	I2CEnd

State3: 
    ; Read operation, last byte was an address byte
	; This is putting the device into read mode.  There must
	; have been a write to set up the address before this
	; Address is set up in smbaddr, calls SMBRead which 
	; should return with the data to be sent back in w.
    movf  	smbtmp,W    
    andlw 	(SSPS+SSPDA+SSPRW) ; Mask out BF bit in SSPSTAT
    xorlw 	(SSPS+SSPRW)  ; Read and not address
	btfss 	STATUS,Z ; Are we in State3?
	goto 	State4 ; No, check for next state.....
	;showstate 3
	call 	SMBRead
	;movlw 	H'42' ; BEBUG
	call 	WriteI2C ; Write the byte to SSPBUF & release clock
	banksel smbidx
	incf 	smbidx,F
	incf 	smbaddr,F
	goto 	I2CEnd

State4: 
    ; Read operation, last byte was data,
	; Continued read operation and, as we had an ack,
    ; master is expecting another byte.
	; current read address in smbaddr.
	movlw 	(SSPS+SSPDA+SSPRW) ; buffer is empty.
	xorwf 	smbtmp,W
	btfss 	STATUS,Z ; Are we in State4?
	goto 	State5 ; No, check for next state....
	;showstate 4
	call 	SMBRead
	;movlw 	H'69' ; DEBUG
	call 	WriteI2C ; Write to SSPBUF
	banksel smbidx
	incf 	smbidx,F
	incf 	smbaddr,F
	goto 	I2CEnd

State5:
    ; When master has read the last byte it's expecting it
    ; should assert NAK at the end of the transmission. Hence
    ; this should signal end of TX.
	; SSPCON - bank 0
    movlw 	(SSPS+SSPDA) ; A NACK was received when transmitting
	xorwf 	smbtmp,W ; data back from the master. Slave logic
	btfss 	STATUS,Z ; is reset in this case. R_W = 0, D_A = 1
	goto 	I2CErr ; and BF = 0
	;showstate 5
    banksel SSPCON
	bsf 	SSPCON,CKP ; Release the clock 
	;call SMBRdEnd - no need in this application.
	goto 	I2CEnd 


; If we aren’t in State5, then something is
; wrong. Do nothing on error state so just drop off.
I2CErr 
	banksel SSPSTAT
	clrf 	SSPSTAT
	;showstate 7	
I2CEnd:
	;showstate 0

NotI2C:





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
		btfsc 	SSPSTAT,BF ; Is the buffer full?
		goto 	WriteI2C ; Yes, keep waiting.
		banksel SSPCON ; No, continue. (page 0)
DoI2CWrite
		bcf 	SSPCON,WCOL; Clear the WCOL flag.
		movwf 	SSPBUF ; Write the byte in WREG
		btfsc 	SSPCON,WCOL; Was there a write collision?
		goto 	DoI2CWrite
		bsf 	SSPCON,CKP ; Release the clock so master can clock out of SSPCON
		return

;---------------------------------------------------------------------
; ReadI2C
; Reads the byte written to the slave and returns it in W
; SSPBUF - bank 0
;---------------------------------------------------------------------
ReadI2C
		banksel SSPBUF
		movf 	SSPBUF,W ; Get the byte and put in WREG
		return
