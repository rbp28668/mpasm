;**********************************************************
; BMP085 routines
;**********************************************************

;**********************************************************

#include p16f819.inc

            radix decimal

_ClkIn			EQU		D'8000000'		; Processor clock frequency.

; Which bits and ports are used for I2C.  Note that the ones shown below correspond to the
; bits and port used for the 16F818 slave I2C bits.  These MUST be defined before including I2C.H
#define I2C_PORT	PORTB
#define I2C_TRIS	TRISB
#define SCL_BIT		4
#define SDA_BIT		1


#include "../I2CMaster/I2C.H"

; Need 32 bit arithmetic
#include "../Math/math32.inc"

; Need 16 bit math for simulation
#ifdef SIMULATE
#include "../Math/math.inc"
#endif

; Export entry points and data.
        global BMP085Init
        global BMP085getUT
        global BMP085getUP
        global BMP085getTemp
        global BMP085getPressure
        global T
        global Pa

; I2C bus address of BMP085
BMP085Addr    equ 0xEE ; write address

; BMP085 register addresses
BMP085CMD       equ 0xF4
BMP085EEPROM    equ 0xAA
BMP085ADC       equ 0xF6

AC1Reg          equ BMP085EEPROM+0
AC2Reg          equ BMP085EEPROM+2
AC3Reg          equ BMP085EEPROM+4
AC4Reg          equ BMP085EEPROM+6
AC5Reg          equ BMP085EEPROM+8
AC6Reg          equ BMP085EEPROM+10
B1Reg           equ BMP085EEPROM+12
B2Reg           equ BMP085EEPROM+14
MBReg           equ BMP085EEPROM+16
MCReg           equ BMP085EEPROM+18
MDReg           equ BMP085EEPROM+20

; BMP085 commands. Note for pressure osrs is top 2 bits
CMDTemp         equ 0x2E    ; 00101110 delay 4.5mS
CMDPressure0    equ 0x34    ; 00110100 osrs=0, delay 4.5mS
CMDPressure1    equ 0x74    ; 01110100 osrs=1, delay 7.5mS
CMDPressure2    equ 0xB4    ; 10110100 osrs=2, delay 13.5mS
CMDPressure3    equ 0xF4    ; 11110100 osrs=3, delay 25.5mS

; Calibration data, read from BMP085 during initialisation
bmp085Cal udata_shr
AC1		res		2   ; short
AC2		res		2   ; short
AC3		res		2   ; short
AC4		res		2   ; unsigned short
AC5		res		2   ; unsigned short
AC6		res		2   ; unsigned short
B1      res     2   ; short
B2      res     2   ; short
MB      res     2   ; short
MC      res     2   ; short
MD      res     2   ; short

; Raw and intermediate values
bmp085Arith udata_shr
UT      res     4   ; long, uncompensated temperature
UP      res     4   ; long, uncompensated pressure
X1      res     4   ; long
X2      res     4   ; long
X3      res     4   ; long
B3      res     4   ; long 
B4      res     4   ; unsigned long
B5      res     4   ; long
B6      res     4   ; long
B7      res     4   ; unsigned long

; Results
bmp085Results udata_shr
T       res     4   ; long Temperature in 0.1 of a degree C
Pa      res     4   ; long, Pressure in Pa

; Temporary data
Temp    udata_ovr
TMP1    res     1
TMP2    res     1

;Mode for bmp085 (oversampling setting). Needs to be in the
; range 0..3
OSS     equ     0

    		; Bring in I2C low level routines.
            include "../I2CMaster/i2c_low.inc"


; Macro to read a calibration word from BMP085
; Reg is the I2C register to read from, Addr is where
; to write the result.  Note BMP085 returns results MS byte
; first whereas all our variables are little-endian.
RdWord      Macro   Reg,Addr
    		CALL    TxmtStartBit
    		MOVLW	WRITE(BMP085Addr)
    		CALL	Txmt_Slave_Addr
    		MOVLW	Reg				; Device register addr
    		CALL	SendData
    		CALL 	TxmtStartBit	; Restart
    		MOVLW	READ(BMP085Addr)
    		CALL	Txmt_Slave_Addr
    		CALL	GetData
            banksel Addr
            MOVWF   Addr+1           ; transmits MSB first
    		banksel Bus_Status
    		BSF		_Last_Byte_Rcv	; Only reading one byte.
    		CALL	GetData
            banksel Addr
            MOVWF   Addr
    		CALL	TxmtStopBit
            EndM
            

;=======================================================
; Move a 16 bit value into a 32 bit register - unsigned
;=======================================================
UMOV1632   Macro dest, src
        banksel src
        movfw src
        banksel dest
        movwf dest
        banksel src
        movfw src + 1
        banksel dest
        movwf dest + 1
        clrf dest+2
        clrf dest+3
        endm

;=======================================================
; Move a 16 bit value into a 32 bit register - signed
;=======================================================
MOV1632   Macro dest, src
        local ispositive

        banksel src
        movfw   src
        banksel dest
        movwf   dest
        banksel src
        movfw   src + 1
        banksel dest
        movwf   dest + 1
        clrf    dest+2  ; assume positive
        clrf    dest+3
        ; Bit 7 of src+1 will be the sign bit,
        banksel src
        btfss   src+1,7
        goto    ispositive
        banksel dest
        comf    dest+2,F ; convert 0 to FF
        comf    dest+3,F  
ispositive             
        endm

;=======================================================
; Divide a 32 bit value by 2^N (i.e. shift right) - SIGNED
;=======================================================
DIV2N   Macro reg, bits
        if bits > 0
        movlw reg
        movwf FSR
        movlw bits
        call  Div2N
        endif
        endm

;=======================================================
; Divide a 32 bit value by 2^N (i.e. shift right) - UNSIGNED
;=======================================================
UDIV2N   Macro reg, bits
        if bits > 0
        movlw reg
        movwf FSR
        movlw bits
        call  UDiv2N
        endif
        endm

;=======================================================
; Multiply a 32 bit value by 2^N (i.e. shift left)
;=======================================================
MUL2N   Macro reg, bits
        if bits > 0
        movlw reg
        movwf FSR
        movlw bits
        call  Mul2N
        endif
        endm

BMP085  code

;=======================================================
;=======================================================
BMP085Init
            ; Initialise I2C
    		CALL 	InitI2CBus_Master ; call this after initial setup.

#ifdef SIMULATE
            ; Use the values from the data sheet for testing
            banksel AC1
            LD16L   AC1, 408
            LD16L   AC2, -72
            LD16L   AC3, -14383
            LD16L   AC4, 32741
            LD16L   AC5, 32757
            LD16L   AC6, 23153
            LD16L   B1,  6190
            LD16L   B2,  4
            LD16L   MB,  -32768
            LD16L   MC,  -8711
            LD16L   MD,  2868
#else
            ; Read calibration words
            movlw   AC1     ; start of parameter list.
            movwf   FSR     ;
            movlw   AC1Reg  ; I2C register
            movwf   TMP1
            movlw   11      ; Number of values to read
            movwf   TMP2    

InitLoop
            incf    FSR,F           ; read Ms byte first
    		CALL    TxmtStartBit
    		MOVLW	WRITE(BMP085Addr)
    		CALL	Txmt_Slave_Addr
    		MOVFW	TMP1            ; Device register addr
    		CALL	SendData
    		CALL 	TxmtStartBit	; Restart
    		MOVLW	READ(BMP085Addr)
    		CALL	Txmt_Slave_Addr
    		CALL	GetData
            MOVWF   INDF            ; Save MS byte
            DECF    FSR,F
    		banksel Bus_Status
    		BSF		_Last_Byte_Rcv	; Only reading one byte.
    		CALL	GetData
            MOVWF   INDF            ; save LS byte
    		CALL	TxmtStopBit
            
            incf    TMP1,F          ; next 16-bit device register
            incf    TMP1,F

            incf    FSR,F           ; next 16-bit variable
            incf    FSR,F

            decfsz  TMP2,F
            goto    InitLoop

;        	RdWord AC1Reg, AC1	
;        	RdWord AC2Reg, AC2	
;    	    RdWord AC3Reg, AC3	
;        	RdWord AC4Reg, AC4	
;    	    RdWord AC5Reg, AC5	
;    	    RdWord AC6Reg, AC6	
;    	    RdWord B1Reg, B1	
;    	    RdWord B2Reg, B2	
;    	    RdWord MBReg, MB
;    	    RdWord MCReg, MC	
;    	    RdWord MDReg, MD	
#endif
            return

;=======================================================
;=======================================================
BMP085getUT
#ifdef SIMULATE
            banksel UT
            MOVL32  UT,27898
#else
       		CALL    TxmtStartBit
    		MOVLW	WRITE(BMP085Addr)
    		CALL	Txmt_Slave_Addr
    		MOVLW	BMP085CMD   ; Select command register
    		CALL	SendData
    		MOVLW	CMDTemp     ; Get temperature command
    		CALL	SendData
    		CALL    TxmtStopBit

            call    Delay45

            RdWord  BMP085ADC,UT
            clrf    UT+2    ; zero 2 MS bytes
            clrf    UT+3
#endif
            return

;=======================================================
;=======================================================
BMP085getUP
#ifdef SIMULATE
            MOVL32  UP,23843
#else
       		CALL    TxmtStartBit
    		MOVLW	WRITE(BMP085Addr)
    		CALL	Txmt_Slave_Addr
    		MOVLW	BMP085CMD       ; Select command register
    		CALL	SendData
    		MOVLW	CMDPressure0 | (OSS << 6)    ; Get pressure command
    		CALL	SendData
    		CALL    TxmtStopBit

            ; Delay depends on oversampling:
            ; OSS = 0 : 4.5mS
            ; OSS = 1 : 7.5mS
            ; OSS = 2 : 13.5mS
            ; OSS = 3 : 25.5mS

            variable i = OSS+1
            while (i > 0)
            call    Delay45
            i -= 1
            endw
            
            if( OSS == 3 ) ; Only have 18 of 25.5mS so...
            call    Delay45         
            call    Delay45         
            endif

            ; Get the result - 3 bytes
    		CALL    TxmtStartBit
    		MOVLW	WRITE(BMP085Addr)
    		CALL	Txmt_Slave_Addr
    		MOVLW	BMP085ADC			; Device register addr
    		CALL	SendData
    		CALL 	TxmtStartBit	; Restart
    		MOVLW	READ(BMP085Addr)
    		CALL	Txmt_Slave_Addr
    		CALL	GetData
            banksel UP
            MOVWF   UP+2           ; transmits MSB first
    		CALL	GetData
            banksel UP
            MOVWF   UP+1           ; now LSB
    		banksel Bus_Status
    		BSF		_Last_Byte_Rcv	; Only reading one byte.
    		CALL	GetData        ; and XLSB
            banksel UP
            MOVWF   UP
    		CALL	TxmtStopBit

            clrf    UP+3    ; zero MS byte

            ; Now shift right (divide by 2) 8-OSS times.
            movlw   UP
            movwf   FSR
            movlw   8-OSS
            call    Div2N
#endif
            return

;=======================================================
;=======================================================
BMP085getTemp
            ; X1 = (UT - AC6) * AC5 / 2^15
            MOV32 REGA,UT
            UMOV1632 REGB,AC6    ; UNSIGNED 
            call    sub32   ; (UT - AC6) in ACCA
            UMOV1632 REGB,AC5    ; UNSIGNED 
            call    mul32
            DIV2N   REGA,15
            MOV32   X1,REGA
        
            ; X2 = MC * 2^11 / (X1 + MD)
            MOV1632 REGB,MD
            call    add32       ;  X1 + MD in REGA
            MOV32   REGB,REGA   ; and now in REGB
            MOV1632 REGA,MC
            MUL2N   REGA,11
            call    div32       ; result in REGA
            MOV32   X2,REGA
            
            ; B5 = X1 + X2
            MOV32   REGB,X1
            call    add32       ; X1 + X2 in REGA
            MOV32   B5,REGA

            ; T = (B5 + 8) / 2^4
            MOVL32  REGB,8
            call    add32
            DIV2N   REGA,4
            MOV32   T,REGA

            return

;=======================================================
;=======================================================
BMP085getPressure
            
            ; B6 = B5 - 4000
            MOV32   REGA,B5
            MOVL32  REGB,4000
            call    sub32
            MOV32   B6,REGA
            
            ; X1 = (B2 * (B6 * B6 / 2^12)) / 2^11
            MOV32   REGB, REGA
            call    mul32       ; B6 * B6
            DIV2N   REGA,12     ; (B6 * B6 / 2^12)
            MOV1632   REGB,B2
            call    mul32       ; (B2 * (B6 * B6 / 2^12))
            DIV2N   REGA,11
            MOV32   X1,REGA

            ; X2 = AC2 * B6 / 2^11
            MOV1632 REGA,AC2
            MOV32   REGB,B6
            call    mul32
            DIV2N   REGA,11
            MOV32   X2,REGA

            ; X3 = X1 + X2
            MOV32   REGB,X1
            call    add32
            MOV32   X3,REGA

            ; B3 = ((AC1 * 4 + X3) << OSS + 2)/4   (note, + higher precedence than <<)
            MOV1632 REGA,AC1
            MUL2N   REGA,2          ; AC1 * 4
            MOV32   REGB,X3
            call    add32
            MUL2N   REGA,OSS
            MOVL32  REGB,2
            call    add32
            DIV2N   REGA,2          ; Divide by 2^2 ie 4
            MOV32   B3,REGA

            ; X1 = AC3 * B6 / 2^13
            MOV1632 REGA,AC3
            MOV32   REGB,B6
            call    mul32
            DIV2N   REGA,13
            MOV32   X1,REGA

            ; X2 = (B1 * (B6 * B6 / 2^12)) / 2^16
            MOV32   REGA,B6
            MOV32   REGB,B6
            call    mul32
            DIV2N   REGA,12
            MOV1632 REGB,B1
            call    mul32
            DIV2N   REGA,16
            MOV32   X2,REGA

            ; X3 = ((X1 + X2) + 2) / 2^2
            MOV32   REGB,X1
            call    add32
            MOVL32  REGB,2
            call add32
            DIV2N   REGA,2
            MOV32   X3,REGA            

            ; B4 = AC4 * (unsigned long)(X3 + 32768) / 2^15
            MOVL32  REGB,32768
            call    uadd32
            UMOV1632 REGB,AC4    ; UNSIGNED 
            call    umul32
            DIV2N   REGA,15
            MOV32   B4,REGA

            ; B7 = ((unsigned long)UP - B3) * (50000 >> OSS)
            MOV32   REGA,UP
            MOV32   REGB,B3
            call    sub32
            MOVL32  REGB, (50000 >> OSS)
            ;DIV2N   REGB,OSS
            call    umul32
            MOV32   B7,REGA            

            ; if(B7 < 0x800000000) 
            ;   p = (B7*2) / B4
            ; else
            ;   p = (B7 / B4) * 2
            ; endif
            ;
            ; B7 is unsigned. So < 0x80000000 involves testing high bit to see if
            ; it's clear.
            btfss   B7+3,7
            goto    AltP
            ; B7 is < 0x80000000 as bit as ms bit is clear
            ;   p = (B7*2) / B4
            MOV32   REGA,B7
            MUL2N   REGA,1
            MOV32   REGB,B4
            call    udiv32       
            goto    AltPEnd
AltP        ;   p = (B7 / B4) * 2
            MOV32   REGA,B7
            MOV32   REGB,B4
            call    udiv32       
            MUL2N   REGA,1
AltPEnd
            MOV32   Pa,REGA     ; Note Pa instead of P as otherwise link error.

            ; X1 = (p/2^8) * (p/2^8)
            DIV2N   REGA,8
            MOV32   REGB,REGA
            call    mul32
            ; just leave in REGA, next expression updates X1

            ; X1 = (X1 * 3038) / 2^16
            MOVL32  REGB,3038
            call    mul32
            DIV2N   REGA,16
            MOV32   X1,REGA

            ; X2 = (-7357 * p) / 2^16
            MOV32   REGA,Pa
            MOVL32  REGB,-7357
            call    mul32
            DIV2N   REGA,16
            ;MOV32  X2,REGA

            ; p = p + (X1 + X2 + 3971) / 2^4
            MOV32   REGB,X1
            call    add32
            MOVL32  REGB,3791
            call    add32
            DIV2N   REGA,4
            MOV32   REGB,Pa
            call    add32
            MOV32   Pa,REGA
        
            return
; ************************************************************
; Support routines
; ************************************************************

;=======================================================
; Div2N divides the 4 byte register pointed to by FSR by 2^N
; where N is passed in W and 1 <= N <= 32
; Note that sign is preserved.
;=======================================================
Div2N       
            banksel TMP1
            movwf   TMP1        ; loop counter
Div2NLoop   movlw   3           ; start with MSB 3 bytes up
            addwf   FSR,F       ; so move pointer
            bcf     STATUS,C    ; don't shift anything in!
            btfsc   INDF,7      ; sign bit set?
            bsf     STATUS,C    ; if so, preserve it. 
            rrf     INDF,F
            decf    FSR,F
            rrf     INDF,F
            decf    FSR,F
            rrf     INDF,F
            decf    FSR,F
            rrf     INDF,F
            decfsz  TMP1,F
            goto    Div2NLoop
            return

;=======================================================
; UDiv2N divides the 4 byte unsigned register pointed to by 
; FSR by 2^N where N is passed in W and 1 <= N <= 32
;=======================================================
UDiv2N      
            banksel TMP1
            movwf   TMP1    ; loop counter
UDiv2NLoop  movlw   3       ; start with MSB 3 bytes up
            addwf   FSR,F   ; so move pointer
            bcf     STATUS,C ; don't shift anything in!
            rrf     INDF,F
            decf    FSR,F
            rrf     INDF,F
            decf    FSR,F
            rrf     INDF,F
            decf    FSR,F
            rrf     INDF,F
            decfsz  TMP1,F
            goto    UDiv2NLoop
            return


;=======================================================
; Mul2N multiplies the 4 byte register pointed to by FSR by 2^N
; where N is passed in W and 1 <= N <= 32
;=======================================================
Mul2N       
            banksel TMP1
            movwf   TMP1    ; loop counter
Mul2NLoop   bcf     STATUS,C ; don't shift anything in!
            rlf     INDF,F
            incf    FSR,F
            rlf     INDF,F
            incf    FSR,F
            rlf     INDF,F
            incf    FSR,F
            rlf     INDF,F
            
            movlw   3       ; start with MSB 3 bytes up
            subwf   FSR,F   ; so move pointer
            
            decfsz  TMP1,F
            goto    Mul2NLoop
            return

;=======================================================
; Delay 4.5mS to allow reading to take place.
;=======================================================
Delay45
            banksel TMP1
            MOVLW   (D'45' * (_ClkIn/4) / D'10000') / ( 3*256 + 3) + 1
            MOVWF   TMP2         ; Use MSD and LSD Registers to Initilize LCD
            CLRF    TMP1         ;
LOOP2       DECFSZ  TMP1, F      ; Delay time = MSD * ((3 * 256) + 3) * Tcy
            GOTO    LOOP2           ;
            DECFSZ  TMP2, F      ;
            GOTO    LOOP2           ;
            RETURN
    

        end
