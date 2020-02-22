; Math 32
; Include file for 32 bit math macros and entry points to
; 32 bit math library.

; 32 bit Accumulators - note LS byte first.
; Sign is therefore bit 7 of (reg+3)
        extern      REGA
        extern      REGB
        extern      REGC

; BCD / Decimal
        extern      DSIGN
        extern      DIGITS

#define DIGIT1 (DIGITS+0)				;MSD
#define DIGIT2 (DIGITS+1)
#define DIGIT3 (DIGITS+2)
#define DIGIT4 (DIGITS+3)
#define DIGIT5 (DIGITS+4)				;Decimal (BCD) digits
#define DIGIT6 (DIGITS+5)
#define DIGIT7 (DIGITS+6)
#define DIGIT8 (DIGITS+7)
#define DIGIT9 (DIGITS+8)
#define DIGIT10 (DIGITS+9)				;LSD


; Entry points.  Note #defines to allow exclusion of various routines if memory tight.
        extern      sub32       ; signed subtract, REGA - REGB -> REGA
        extern      add32       ; signed add, REGA + REGB -> REGA
        extern      mul32       ; signed multiply, REGA * REGB -> REGA
        extern      umul32      ; unsigned multiply, REGA * REGB -> REGA

        #ifndef     NODIV32
        extern      div32       ; signed divide, REGA / REGB -> REGA, remainde in REGC
        extern      udiv32      ; unsigned divide, REGA / REGB -> REGA, remainde in REGC
        #ifndef     NOROUND32
        extern      round32     ; round result of division
        #endif
        #endif

        #ifndef     NOSQRT32
        extern      sqrt32      ; square root, sqrt(REGA) -> REGA
        #endif

        #ifndef     NOCONV32
        extern      bin2dec32   ; signed binary to decimal,REGA -> DIGITS 1 (MSD) TO 10 (LSD) & DSIGN
        extern      dec2bin32   ; signed decimal to binary, DIGIT1 thro DIGIT(X) & DSIGN -> REGA
        #endif

        extern      uadd32      ; unsigned add, REGA + REGB -> REGA
        extern      abs32       ; absolute value |REGA| -> REGA
        extern      negate32    ; negate, -REGA -> REGA



;=======================================================
; Move 32 bit register value from one to another when both
; are on the same page.  Assumes that banksel set 
; appropriately
;=======================================================
MOV32SP Macro dest, src
        movfw src
        movwf dest
        movfw src + 1
        movwf dest + 1
        movfw src + 2
        movwf dest + 2
        movfw src + 3
        movwf dest + 3
        endm

;=======================================================
; Move 32 bit register value from one to another 
; Dest uses indirection and sets IRP appropriately
;=======================================================
MOV32   Macro dest, src
        bankisel dest
        movlw dest
        movwf FSR
        banksel src
        movfw src
        movwf INDF
        incf  FSR,F
        movfw src + 1
        movwf INDF
        incf  FSR,F
        movfw src + 2
        movwf INDF
        incf  FSR,F
        movfw src + 3
        movwf INDF
        endm
;=======================================================
; Test 32 bit register for equality with literal value.
;=======================================================
EQUALS32  Macro r32, Value
        banksel r32
		MOVF  r32,W
		XORLW (Value & 0xFF)
		BTFSS STATUS,Z
		GOTO FAIL
		MOVF  (r32)+1,W
		XORLW ((Value & 0xFF00) >> 8)
		BTFSS STATUS,Z
		GOTO FAIL
		MOVF  (r32)+2,W
		XORLW ((Value & 0xFF0000) >> 16)
		BTFSS STATUS,Z
		GOTO FAIL
		MOVF  (r32)+3,W
		XORLW ((Value & 0xFF000000) >> 24)
		BTFSS STATUS,Z
		bsf STATUS,C
		endm

;=======================================================
; Move literal value to 32 bit register
;=======================================================
MOVL32  Macro reg32, value
        banksel reg32
        movlw (value & 0xFF)
        movwf reg32
        movlw ((value & 0xFF00) >> 8)
        movwf (reg32)+1
        movlw ((value & 0xFF0000) >> 16)
        movwf (reg32)+2
        movlw ((value & 0xFF000000) >> 24)
        movwf (reg32)+3
        endm

;=======================================================
; CLRF32 sets a 32 bit file location to zero
;=======================================================
CLRF32	Macro DST
		banksel DST
		CLRF (DST)
		CLRF (DST+1)
		CLRF (DST+2)
		CLRF (DST+3)
		endm

;=======================================================
; INCF32 increments a 32 bit value in a file
;=======================================================
INCF32	Macro DST
		banksel DST
		INCF (DST),F
		BTFSC STATUS,C
		INCF (DST)+1,F
		BTFSC STATUS,C
		INCF (DST)+2,F
		BTFSC STATUS,C
		INCF (DST)+3,F
		endm
