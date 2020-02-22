
; Accumulators
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


; Entry points
        extern      sub32       ; signed subtract, REGA - REGB -> REGA
        extern      add32       ; signed add, REGA + REGB -> REGA
        extern      mul32       ; signed multiply, REGA * REGB -> REGA
        extern      umul32      ; unsigned multiply, REGA * REGB -> REGA
        extern      div32       ; signed divide, REGA / REGB -> REGA, remainde in REGC
        extern      round32     ; round result of division
        extern      sqrt32      ; square root, sqrt(REGA) -> REGA
        extern      bin2dec32   ; signed binary to decimal,REGA -> DIGITS 1 (MSD) TO 10 (LSD) & DSIGN
        extern      dec2bin32   ; signed decimal to binary, DIGIT1 thro DIGIT(X) & DSIGN -> REGA
        extern      uadd32      ; unsigned add, REGA + REGB -> REGA
        extern      abs32       ; absolute value |REGA| -> REGA
        extern      negate32    ; negate, -REGA -> REGA



; Test 32 bit register for equality with literal value.
EQUALS32  Macro r32, Value
        pagesel FAIL
#ifndef GLOBAL_FAIL
        Local FAIL
        BCF STATUS,C
#endif
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
#ifndef   GLOBAL_FAIL
FAIL	bsf STATUS,C
#else
        GOTO  FAIL
#endif
		endm

; Move literal value to 32 bit register
MOVL32  Macro reg32, value
        movlw (value & 0xFF)
        movwf reg32
        movlw ((value & 0xFF00) >> 8)
        movwf (reg32)+1
        movlw ((value & 0xFF0000) >> 16)
        movwf (reg32)+2
        movlw ((value & 0xFF000000) >> 24)
        movwf (reg32)+3
        endm
