;**********************************************************
; BMP085 routines
; Interfaces to the BMP085 pressure sensor.
; Sensor must be initialised first using BMP085Init
; To read pressure must call BMP085GetUT and BMP085GetUP
; first to get uncalibrated temperature and pressure.
; Then BMPgetTemp and BMPgetPressure can be called (in
; that order) to initialise the T and Pa variables which
; return temperature in units of 0.1C and pressure in Pa.
; Note 1 hPa = 100 Pa
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

; Need 32 bit arithmetic less  square root.
#define NOSQRT32    1
#define NOROUND32   1
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
        global UT ;Export for debugging
        global UP ;Export for debugging
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
bmp085Cal udata
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
bmp085Arith udata
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
bmp085Results udata
T       res     4   ; long Temperature in 0.1 of a degree C
Pa      res     4   ; long, Pressure in Pa

; Temporary data
Temp    udata_ovr
TMP1    res     1
TMP2    res     1

;Mode for bmp085 (oversampling setting). Needs to be in the
; range 0..3
#ifdef SIMULATE
OSS     equ     0
#else  
OSS     equ     1
#endif

    		; Bring in I2C low level routines.
            include "../I2CMaster/i2c_low.inc"


; Macro to read a calibration word from BMP085
; Reg is the I2C register to read from, Addr is where
; to write the result.  Note BMP085 returns results MS byte
; first whereas all our variables are little-endian.
RdWord      Macro   Reg,Addr
            pagesel i2cMaster
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
            pagesel $
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
        pagesel ispositive
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
        bankisel reg
        movlw reg
        movwf FSR
        movlw bits
        pagesel Div2N
        call  Div2N
        endif
        endm

;=======================================================
; Divide a 32 bit value by 2^N (i.e. shift right) - UNSIGNED
;=======================================================
UDIV2N   Macro reg, bits
        if bits > 0
        bankisel reg
        movlw reg
        movwf FSR
        movlw bits
        pagesel UDiv2N
        call  UDiv2N
        endif
        endm

;=======================================================
; Multiply a 32 bit value by 2^N (i.e. shift left)
;=======================================================
MUL2N   Macro reg, bits
        if bits > 0
        bankisel reg
        movlw reg
        movwf FSR
        movlw bits
        pagesel Mul2N
        call  Mul2N
        endif
        endm

BMP085  code

;=======================================================
;=======================================================
BMP085Init
            ; Initialise I2C
            pagesel i2cMaster
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
            ; Should delay at least 10mS after power up.
            pagesel Delay45
            Call Delay45
            Call Delay45
            Call Delay45
            
            ; Read calibration words
            bankisel AC1    ; ensure IRP bit set correctly for indirect address
            movlw   AC1     ; start of parameter list.
            movwf   FSR     ;
            banksel TMP1
            movlw   AC1Reg  ; I2C register
            movwf   TMP1
            movlw   11      ; Number of values to read
            movwf   TMP2    

           
InitLoop
            pagesel i2cMaster
            incf    FSR,F           ; read Ms byte first
    		CALL    TxmtStartBit
    		MOVLW	WRITE(BMP085Addr)
    		CALL	Txmt_Slave_Addr
            banksel TMP1
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
            
            banksel TMP1
            incf    TMP1,F          ; next 16-bit device register
            incf    TMP1,F

            incf    FSR,F           ; next 16-bit variable
            incf    FSR,F

            pagesel InitLoop
            decfsz  TMP2,F
            goto    InitLoop

#endif
            pagesel $
            return

;=======================================================
;=======================================================
BMP085getUT
#ifdef SIMULATE
            banksel UT
            MOVL32  UT,27898  ; 0x6CFA
#else
            pagesel i2cMaster
       		CALL    TxmtStartBit
    		MOVLW	WRITE(BMP085Addr)
    		CALL	Txmt_Slave_Addr
    		MOVLW	BMP085CMD   ; Select command register
    		CALL	SendData
    		MOVLW	CMDTemp     ; Get temperature command
    		CALL	SendData
    		CALL    TxmtStopBit

            pagesel Delay45
            call    Delay45

            RdWord  BMP085ADC,UT
            banksel UT
            clrf    UT+2    ; zero 2 MS bytes
            clrf    UT+3
            pagesel $
#endif
            return

;=======================================================
;=======================================================
BMP085getUP
#ifdef SIMULATE
            MOVL32  UP,23843 ; 0x9335
#else
            pagesel i2cMaster
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

            pagesel Delay45
            variable i = OSS+1
            while (i > 0)
            call    Delay45
            i -= 1
            endw
            
            if( OSS == 3 ) ; Only have 18 of 25.5mS so...
            call    Delay45         
            call    Delay45         
            endif

            pagesel i2cMaster
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

            banksel UP
            clrf    UP+3    ; zero MS byte

            pagesel $

            ; Now shift right (divide by 2) 8-OSS times.
            DIV2N   UP,(8-OSS)
#endif
            return

;=======================================================
;=======================================================
BMP085getTemp
            ; X1 = (UT - AC6) * AC5 / 2^15
            MOV32 REGA,UT
            UMOV1632 REGB,AC6    ; UNSIGNED 
            pagesel sub32
            call    sub32   ; (UT - AC6) in ACCA
            UMOV1632 REGB,AC5    ; UNSIGNED 
            pagesel mul32
            call    mul32
            DIV2N   REGA,15
            MOV32   X1,REGA
            ; Simulate:  X1 should have 4743 (0x1287)
        
            ; X2 = MC * 2^11 / (X1 + MD)
            MOV1632 REGB,MD     ; Sim: 0B34
            pagesel add32
            call    add32       ;  X1 + MD in REGA (sim 0x1DBB)
            MOV32   REGB,REGA   ; and now in REGB
            MOV1632 REGA,MC     ; Sim -8711 (0xFFFFDDF9)
            MUL2N   REGA,11
            pagesel div32
            call    div32       ; result in REGA
            MOV32   X2,REGA
            ; Simulate: X2 should have -2344 (0xFFFF F6D8)
            
            ; B5 = X1 + X2
            MOV32   REGB,X1
            pagesel add32
            call    add32       ; X1 + X2 in REGA
            MOV32   B5,REGA

            ; T = (B5 + 8) / 2^4
            MOVL32  REGB,8
            pagesel add32
            call    add32
            DIV2N   REGA,4
            MOV32   T,REGA
            pagesel $
            return

;=======================================================
; Get corrected pressure.
; Note that for this to work the BMP085 must have been
; initialised, raw temp and pressure read and 
; BMP085getTemperature called as getTemperature sets up
; the temperature calibration factor in B5
;=======================================================
BMP085getPressure
            ; B6 = B5 - 4000
            MOV32   REGA,B5         ; Sim 0x096F
            MOVL32  REGB,4000
            pagesel sub32
            call    sub32
            MOV32   B6,REGA         ; Sim 0xFFFFF9C0
            
            ; X1 = (B2 * (B6 * B6 / 2^12)) / 2^11
            MOV32   REGB, REGA
            pagesel mul32
            call    mul32       ; B6 * B6
            DIV2N   REGA,12     ; (B6 * B6 / 2^12)
            MOV1632   REGB,B2
            pagesel mul32
            call    mul32       ; (B2 * (B6 * B6 / 2^12))
            DIV2N   REGA,11
            MOV32   X1,REGA     ; Sim: 1

            ; X2 = AC2 * B6 / 2^11
            MOV1632 REGA,AC2
            MOV32   REGB,B6
            pagesel mul32
            call    mul32
            DIV2N   REGA,11
            MOV32   X2,REGA     ; Sim 56

            ; X3 = X1 + X2
            MOV32   REGB,X1
            pagesel add32
            call    add32
            MOV32   X3,REGA     ; Sim 57

            ; B3 = ((AC1 * 4 + X3) << OSS + 2)/4   (note, + higher precedence than <<)
            MOV1632 REGA,AC1
            MUL2N   REGA,2          ; AC1 * 4
            MOV32   REGB,X3
            pagesel add32
            call    add32
            MUL2N   REGA,OSS
            MOVL32  REGB,2
            pagesel add32
            call    add32
            DIV2N   REGA,2          ; Divide by 2^2 ie 4
            MOV32   B3,REGA         ; Sim 422 (0x01A6)

            ; X1 = AC3 * B6 / 2^13
            MOV1632 REGA,AC3
            MOV32   REGB,B6
            pagesel mul32
            call    mul32
            DIV2N   REGA,13
            MOV32   X1,REGA         ; Sim 2809 (0x0AF9)

            ; X2 = (B1 * (B6 * B6 / 2^12)) / 2^16
            MOV32   REGA,B6
            MOV32   REGB,B6
            pagesel mul32
            call    mul32
            DIV2N   REGA,12
            MOV1632 REGB,B1
            pagesel mul32
            call    mul32
            DIV2N   REGA,16
            MOV32   X2,REGA         ; Sim 59

            ; X3 = ((X1 + X2) + 2) / 2^2
            MOV32   REGB,X1
            pagesel add32
            call    add32
            MOVL32  REGB,2
            pagesel add32
            call    add32
            DIV2N   REGA,2
            MOV32   X3,REGA         ; Sim 717 (0x02CD)          

            ; B4 = AC4 * (unsigned long)(X3 + 32768) / 2^15
            MOVL32  REGB,32768
            pagesel uadd32
            call    uadd32
            UMOV1632 REGB,AC4    ; UNSIGNED 
            pagesel umul32
            call    umul32
            DIV2N   REGA,15
            MOV32   B4,REGA         ; Sim 33457 (0x82B1)

            ; B7 = ((unsigned long)UP - B3) * (50000 >> OSS)
            MOV32   REGA,UP
            MOV32   REGB,B3
            pagesel sub32
            call    sub32
            MOVL32  REGB, (50000 >> OSS)
            ;DIV2N   REGB,OSS
            pagesel umul32
            call    umul32
            MOV32   B7,REGA         ;Sim 1171050000 (0x45CCCE10)           

            ; if(B7 < 0x800000000) 
            ;   p = (B7*2) / B4
            ; else
            ;   p = (B7 / B4) * 2
            ; endif
            ;
            ; B7 is unsigned. So < 0x80000000 involves testing high bit to see if
            ; it's clear.
            pagesel AltP        
            btfss   B7+3,7
            goto    AltP
            ; B7 is < 0x80000000 as bit as ms bit is clear
            ;   p = (B7*2) / B4
            MOV32   REGA,B7
            MUL2N   REGA,1
            MOV32   REGB,B4
            pagesel udiv32
            call    udiv32
            pagesel AltPEnd       
            goto    AltPEnd
AltP        ;   p = (B7 / B4) * 2
            MOV32   REGA,B7
            MOV32   REGB,B4
            pagesel udiv32
            call    udiv32       
            MUL2N   REGA,1
AltPEnd
            MOV32   Pa,REGA     ; Note Pa instead of P as otherwise link error.
            ; Sim 70002 (0x011172)

            ; X1 = (p/2^8) * (p/2^8)
            DIV2N   REGA,8
            MOV32   REGB,REGA
            pagesel mul32
            call    mul32
            ; just leave in REGA, next expression updates X1
            ; Sim  74529 (0x012321)

            ; X1 = (X1 * 3038) / 2^16
            MOVL32  REGB,3038
            pagesel mul32
            call    mul32
            DIV2N   REGA,16
            MOV32   X1,REGA     ; Sim 3454 (0x0D7E)

            ; X2 = (-7357 * p) / 2^16
            MOV32   REGA,Pa
            MOVL32  REGB,-7357
            pagesel mul32
            call    mul32
            DIV2N   REGA,16
            ;MOV32  X2,REGA     ; Sim -7859

            ; p = p + (X1 + X2 + 3971) / 2^4
            MOV32   REGB,X1
            pagesel add32
            call    add32
            MOVL32  REGB,3791
            pagesel add32
            call    add32
            DIV2N   REGA,4
            MOV32   REGB,Pa
            pagesel add32
            call    add32
            MOV32   Pa,REGA     ; Sim 69963 (0x01114B)
            
            pagesel $
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
