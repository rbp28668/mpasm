;**********************************************************************************************************
;				I2C Bus Header File
;**********************************************************************************************************

_ClkOut		equ	(_ClkIn >> 2)

;
; Compute the delay constants for setup & hold times
;
_40uS_Delay	set	(_ClkOut/250000)
_47uS_Delay	set	(_ClkOut/212766)
_50uS_Delay	set	(_ClkOut/200000)

#define	_OPTION_INIT	(0xC0 | 0x03)		; Prescaler to TMR0 for Appox 1 mSec timeout
;
#define	_SCL	I2C_PORT,SCL_BIT
#define	_SDA	I2C_PORT,SDA_BIT

#define	_SCL_TRIS	I2C_TRIS,SCL_BIT
#define	_SDA_TRIS	I2C_TRIS,SDA_BIT

#define READ(addr) (addr | 1)
#define WRITE(addr) (addr & 0xFE)

TRUE	equ	1
FALSE	equ	0

;//		Register File Variables for driving I2C.
i2cdata		udata 
BitCount 		res 1		;// The bit number (0:7) transmitted or received
Bus_Status 		res 1		;// Status Reg of I2C Bus for both TXMT & RCVE
Bus_Control 	res 1		;// control Register of I2C Bus
DelayCount 		res 1
DataByteCopy 	res 1		;// copy of DataByte for Left Shifts (destructive)


;*************************************************************************************
;			I2C Bus Status Reg Bit Definitions
;*************************************************************************************

#define	_Bus_Busy	    Bus_Status,0
#define	_Abort		    Bus_Status,1
#define	_Txmt_Progress	Bus_Status,2
#define	_Rcv_Progress	Bus_Status,3

#define	_Txmt_Success	Bus_Status,4
#define	_Rcv_Success	Bus_Status,5
#define	_Fatal_Error	Bus_Status,6
#define	_ACK_Error	    Bus_Status,7

;*************************************************************************************
;			I2C Bus Contro Register
;*************************************************************************************
#define	_10BitAddr	Bus_Control,0
#define	_Slave_RW	Bus_Control,1
#define	_Last_Byte_Rcv	Bus_Control,2

#define	_SlaveActive	Bus_Control,6
#define	_TIME_OUT_	Bus_Control,7




;**********************************************************************************************************
;				General Purpose Macros
;**********************************************************************************************************

RELEASE_BUS	MACRO
			banksel I2C_TRIS		
			bsf	_SDA_TRIS		; tristate SDA
			bsf	_SCL_TRIS		; tristate SCL
;			banksel Bus_Status
;			bcf	_Bus_Busy		; Bus Not Busy, TEMP ????, set/clear on Start & Stop
		ENDM



