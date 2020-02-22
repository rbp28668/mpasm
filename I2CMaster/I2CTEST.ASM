	Title		"I2C Master Mode Implemetation"
        SubTitle        "Rev 0.2	:  04 April 1997"

;***********************************************************************************************************
;
;			Software Implementation Of I2C Master Mode
;
;	* Master Transmitter & Master Receiver Implemented in software 
;	* Slave Mode implemented in hardware
;
;	*	Refer to Signetics/Philips I2C-Bus Specification
;
;    The software is implemented using PIC16C71 & thus can be ported to all Enhanced core PIC16CXX products
;
;	RB1 is SDA		(Any I/O Pin May Be used instead)
;	RB0/INT is SCL		(Any I/O Pin May Be used instead)
;	
;
;       Program:          I2CTEST.ASM 
;       Revision Date:    Rev 0.1	:  01 Mar 1993
;                         4-04-97      Compatibility with MPASMWIN 1.40
;
;***********************************************************************************************************

		LIST    p = 16C71
                ERRORLEVEL -302
		Radix	DEC

_ClkIn		equ	16000000	; Input Clock Frequency Of PIC16C71    	

		include		<p16c71.inc>

TRUE	equ	1
FALSE	equ	0

LSB	equ	0
MSB	equ	7

;
#define	_Slave_1_Addr	0xA0		; Serial EEPROM #1
#define	_Slave_2_Addr	0xAC		; Serial EEPROM #2
#define	_Slave_3_Addr	0xD6		; Slave PIC16CXX

#define	_ENABLE_BUS_FREE_TIME	TRUE
#define	_CLOCK_STRETCH_CHECK	TRUE
#define	_INCLUDE_HIGH_LEVEL_I2C	TRUE


		include		"i2c.h"

		CBLOCK	_End_I2C_Ram
			SaveStatus		; copy of STATUS Reg
			SaveWReg		; copy of WREG
			byteCount
			HoldData						
		ENDC

		CBLOCK	0x20
                	DataBegin	; Data to be read or written is stored here
                ENDC


		ORG	0x00

		goto	Start
;
		ORG	0x04
;*********************************************************************************************************
;				Interrupt Service Routine
;
;   For I2C routines, only TMR0 interrupt is used
; TMR0 Interrupts enabled only if Clock Stretching is Used
; On TMR0 timeout interrupt, disable TMR0 Interrupt, clear pending flags,
; MUST set _TIME_OUT_ flag saying possibly a FATAL error ocured
; The user may choose to retry the operation later again
;
;*********************************************************************************************************

Interrupt:
;
; Save Interrupt Status (WREG & STATUS regs)
;
	movwf	SaveWReg		; Save WREG
	swapf	STATUS,W		; affects no STATUS bits : Only way OUT to save STATUS Reg ?????
	movwf	SaveStatus 		; Save STATUS Reg
 if _CLOCK_STRETCH_CHECK		; TMR0 Interrupts enabled only if Clock Stretching is Used
	btfss	INTCON,T0IF
	goto	MayBeOtherInt		; other Interrupts
	bsf	_TIME_OUT_		; MUST set this Flag, can take other desired actions here
        bcf	INTCON,T0IF
 endif
;
;  Check For Other Interrupts Here, This program usesd only TMR0 & INT Interrupt
;
MayBeOtherInt:
	NOP
;
RestoreIntStatus:			; Restore Interrupt Status
	swapf	SaveStatus,W
	movwf	STATUS			; restore STATUS Reg
        swapf	SaveWReg, F
        swapf	SaveWReg,W		; restore WREG
	retfie
;
;*********************************************************************************************************
;				 Include I2C High Level & Low Level Routines
  if _INCLUDE_HIGH_LEVEL_I2C
		include		"i2c_high.inc"
  endif

;*********************************************************************************************************
;
 
ReadSlave1:

;
;  EEPROM (24C04) may be in write mode (busy), check for ACK by sending a control byte
;
		LOAD_ADDR_8  _Slave_1_Addr
wait1:
		I2C_TEST_DEVICE
		btfss	_SlaveActive	; See If slave is responding
		goto	wait1		; if stuck for ever, recover from WDT, can use other schemes
		clrwdt
		I2C_READ_SUB  8, DataBegin+1, 0x50
;
; Read 8 bytes of data from Slave 2 starting from Sub-Address 0x60
;
		LOAD_ADDR_8  _Slave_2_Addr
wait2:
		I2C_TEST_DEVICE
		btfss	_SlaveActive	; See If slave is responding
		goto	wait2		; if stuck for ever, recover from WDT, can use other schemes
		clrwdt	
		I2C_READ_SUB  8, DataBegin+1, 0x60


	return
;
;*********************************************************************************************************

ReadSlave3:

		LOAD_ADDR_8  _Slave_3_Addr
wait3:
		I2C_TEST_DEVICE
		btfss	_SlaveActive	; See If slave is responding
		goto	wait3		; if stuck for ever, recover from WDT, can use other schemes
		clrwdt
		I2C_READ_SUB  8, DataBegin, 0
;

	return

;*********************************************************************************************************
;
;			Fill Data Buffer With Test Data ( 8 bytes of 0x55, 0xAA pattern)
;
;*********************************************************************************************************

FillDataBuf:

	movlw	0x00		; start address location of EEPROM array
        movwf	DataBegin	; 1st byte of data to be sent is start address
	movlw	DataBegin+1	; data starts following address (RAM Pointer)
	movwf	FSR
	movlw	8		; fill RAM with 8 bytes , this data is written to EEPROM (slave)
	movwf	byteCount
	movlw	0x55		; pattern to fill with is 0x55 & 0xAA
	movwf	HoldData
X1:
	comf	HoldData, F
	movf	HoldData,W
	movwf	INDF
        incf	FSR, F		; point to next location
	decfsz	byteCount, F
        goto	X1
	return
;
;*********************************************************************************************************
;
;			   Main Routine  (Test Program)
;
;			SINGLE  MASTER, MULTIPLE SLAVES
;
;*********************************************************************************************************

Start:
	call	InitI2CBus_Master	; initialize I2C Bus
	bsf	INTCON,GIE			; enable global interrupts
;

	call	FillDataBuf		; fill data buffer with 8 bytes of data (0x55, 0xAA)
;
; Use high level Macro to send 9 bytes to Slave (1 & 2 : TWO 24C04) of 8 bit Addr
;
;  Write 9 bytes to Slave 1, starting at RAM addr pointer DataBegin
;

	btfsc	_Bus_Busy	; is Bus Free, ie. has a start & stop  bit been detected (only for multi master system)
	goto	$-1		; a very simple test, unused for now

	LOAD_ADDR_8  _Slave_1_Addr	
	I2C_WR	      0x09, DataBegin
;
; Write 8 bytes of Data to slave 2 starting at slaves memory address 0x30
;

	btfsc	_Bus_Busy	; is Bus Free, ie. has a start & stop bit been detected (only for multi master system)
	goto	$-1		; a very simple test, unused for now

	LOAD_ADDR_8  _Slave_2_Addr
	I2C_WR_SUB    0x08, DataBegin+1, 0x30

	call	ReadSlave1		; read a byte from slave from current address
;
	LOAD_ADDR_8  _Slave_3_Addr
	movlw	0xCC
        movwf	DataBegin
	I2C_WR_SUB	0x01,DataBegin, 0x33 
;
	call	ReadSlave3		; Read From Slave PIC
;

self    clrwdt
	goto	self		       
;
;*********************************************************************************************************

	END


