;===========================================================
; math32.asm
; 
; http://www.piclist.com/techref/microchip/math/32bmath-ph.htm
;===========================================================

        TITLE "32 bit Math routines"
        LIST P=PIC16F818
        include "P16F818.inc"
       	

		radix dec


            udata

;Accumulators
REGA        res 4
REGB        res 4
REGC        res 4

;Digits and sign for BCD <-> binary conversion
DSIGN		res 1		;Digit Sign. 0=positive,1=negative
DIGITS      res 10
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

; Temporary data, can be over-written by other routines.
            udata_ovr
MTEMP       res 1
MCOUNT      res 1
DCOUNT      res 1

;Export data
        global      REGA
        global      REGB
        global      REGC
        global      DSIGN
        global      DIGITS


math32  code

;Export entry points
        global      sub32
        global      add32
        global      mul32
        global      umul32
        global      div32
        global      round32
        global      sqrt32
        global      bin2dec32
        global      dec2bin32
        global      uadd32
        global      abs32
        global      negate32


;*** 32 BIT SIGNED SUTRACT ***
;REGA - REGB -> REGA
;Return carry set if overflow

sub32
	    call	negateb		;Negate REGB
	    skpnc
	    return			    ;Overflow

;*** 32 BIT SIGNED ADD ***
;REGA + REGB -> REGA
;Return carry set if overflow

add32	movf	(REGA)+3,w		;Compare signs
	    xorwf	(REGB)+3,w
	    movwf	MTEMP

	    call	addba		;Add REGB to REGA

	    clrc			    ;Check signs
	    movf	(REGB)+3,w	;If signs are same
	    xorwf	(REGA)+3,w	;so must result sign
	    btfss	MTEMP,7		;else overflow
	    addlw	0x80
	    return

;*** 32 BIT SIGNED MULTIPLY ***
;REGA * REGB -> REGA
;Return carry set if overflow

mul32
	    clrf	MTEMP		;Reset sign flag
	    call	absa		;Make REGA positive
	    skpc
	    call	absb		;Make REGB positive
	    skpnc
	    return			    ;Overflow

	    call	movac		;Move REGA to REGC
	    call	clra		;Clear product

	    movlw	D'31'		;Loop counter
	    movwf	MCOUNT

muloop	call	slac		;Shift left product and multiplicand
	
	    rlf	    (REGC)+3,w	;Test MSB of multiplicand
	    skpnc			    ;If multiplicand bit is a 1 then
	    call	addba		;add multiplier to product

	    skpc			    ;Check for overflow
	    rlf	    (REGA)+3,w
	    skpnc
	    return

	    decfsz	MCOUNT,f	;Next
	    goto	muloop

	    btfsc	MTEMP,0		;Check result sign
	    call	negatea		;Negative
	    return

;*** 32 BIT UNSIGNED MULTIPLY ***
;REGA * REGB -> REGA
;Return carry set if overflow

umul32
	    call	movac		;Move REGA to REGC
	    call	clra		;Clear product

	    movlw	D'32'		;Loop counter 
	    movwf	MCOUNT

umuloop	clrc
        call	slac		;Shift left product and multiplicand
	                        ;also shifts msb into carry
	    skpnc			    ;If multiplicand bit is a 1 then
	    call	addba		;add multiplier to product

	    decfsz	MCOUNT,f	;Next
	    goto	umuloop

	    return


;*** 32 BIT SIGNED DIVIDE ***
;REGA / REGB -> REGA
;Remainder in REGC
;Return carry set if overflow or division by zero

div32	clrf	MTEMP		;Reset sign flag
	    movf	(REGB)+0,w	;Trap division by zero
	    iorwf	(REGB)+1,w
	    iorwf	(REGB)+2,w
	    iorwf	(REGB)+3,w
	    sublw	0
	    skpc
	    call	absa		;Make dividend (REGA) positive
	    skpc
	    call	absb		;Make divisor (REGB) positive
	    skpnc
	    return			;Overflow

	    clrf	(REGC)+0		;Clear remainder
	    clrf	(REGC)+1
	    clrf	(REGC)+2
	    clrf	(REGC)+3
	    call	slac		;Purge sign bit

	    movlw	D'31'		;Loop counter
	    movwf	MCOUNT

dvloop	call	slac		;Shift dividend (REGA) msb into remainder (REGC)

	    movf	(REGB)+3,w		;Test if remainder (REGC) >= divisor (REGB)
	    subwf	(REGC)+3,w
	    skpz
	    goto	dtstgt
	    movf	(REGB)+2,w
	    subwf	(REGC)+2,w
	    skpz
	    goto	dtstgt
	    movf	(REGB)+1,w
	    subwf	(REGC)+1,w
	    skpz
	    goto	dtstgt
	    movf	(REGB)+0,w
	    subwf	(REGC)+0,w
dtstgt	skpc			;Carry set if remainder >= divisor
	    goto	dremlt

	    movf	(REGB)+0,w		;Subtract divisor (REGB) from remainder (REGC)
	    subwf	(REGC)+0,f
	    movf	(REGB)+1,w
	    skpc
	    incfsz	(REGB)+1,w
	    subwf	(REGC)+1,f
	    movf	(REGB)+2,w
	    skpc
	    incfsz	(REGB)+2,w
	    subwf	(REGC)+2,f
	    movf	(REGB)+3,w
	    skpc
	    incfsz	(REGB)+3,w
	    subwf	(REGC)+3,f
	    clrc
	    bsf	    (REGA)+0,0		;Set quotient bit

dremlt	decfsz	MCOUNT,f	;Next
	    goto	dvloop

	    btfsc	MTEMP,0		;Check result sign
	    call	negatea		;Negative
	    return

;*** ROUND RESULT OF DIVISION TO NEAREST INTEGER ***

round32	clrf	MTEMP		;Reset sign flag
	    call	absa		;Make positive
	    clrc
	    call	slc		;Multiply remainder by 2
	    movf	(REGB)+3,w		;Test if remainder (REGC) >= divisor (REGB)
	    subwf	(REGC)+3,w
	    skpz
	    goto	rtstgt
	    movf	(REGB)+2,w
	    subwf	(REGC)+2,w
	    skpz
	    goto	rtstgt
	    movf	(REGB)+1,w
	    subwf	(REGC)+1,w
	    skpz
	    goto	rtstgt
	    movf	(REGB)+0,w
	    subwf	(REGC)+0,w
rtstgt	skpc			;Carry set if remainder >= divisor
	    goto	rremlt
	    incfsz	(REGA)+0,f		;Add 1 to quotient
	    goto	rremlt
	    incfsz	(REGA)+1,f
	    goto	rremlt
	    incfsz	(REGA)+2,f
	    goto	rremlt
	    incf	(REGA)+3,f
	    skpnz
	    return			;Overflow,return carry set
rremlt	btfsc	MTEMP,0		;Restore sign
	    call	negatea
	    return


;*** 32 BIT SQUARE ROOT ***
;sqrt(REGA) -> REGA
;Return carry set if negative

sqrt32	rlf	(REGA)+3,w		;Trap negative values
	    skpnc
	    return

	    call	movac		;Move REGA to REGC
	    call	clrba		;Clear remainder (REGB) and root (REGA)

	    movlw	D'16'		;Loop counter
	    movwf	MCOUNT

sqloop	rlf	    (REGC)+0,f		;Shift two msb's
	    rlf	    (REGC)+1,f		;into remainder
	    rlf	    (REGC)+2,f
	    rlf	    (REGC)+3,f
	    rlf	    (REGB)+0,f
	    rlf	    (REGB)+1,f
	    rlf	    (REGB)+2,f
	    rlf	    (REGC)+0,f
	    rlf	    (REGC)+1,f
	    rlf	    (REGC)+2,f
	    rlf	    (REGC)+3,f
	    rlf	    (REGB)+0,f
	    rlf	    (REGB)+1,f
	    rlf	    (REGB)+2,f

	    setc			;Add 1 to root
	    rlf	    (REGA)+0,f		;Align root
	    rlf	    (REGA)+1,f
	    rlf	    (REGA)+2,f

	    movf	(REGA)+2,w		;Test if remdr (REGB) >= root (REGA)
	    subwf	(REGB)+2,w
	    skpz
	    goto	ststgt
	    movf	(REGA)+1,w
	    subwf	(REGB)+1,w
	    skpz
	    goto	ststgt
	    movf	(REGA)+0,w
	    subwf	(REGB)+0,w
ststgt	skpc			;Carry set if remdr >= root
	    goto	sremlt

	    movf	(REGA)+0,w		;Subtract root (REGA) from remdr (REGB)
	    subwf	(REGB)+0,f
	    movf	(REGA)+1,w
	    skpc
	    incfsz	(REGA)+1,w
	    subwf	(REGB)+1,f
	    movf	(REGA)+2,w
	    skpc
	    incfsz	(REGA)+2,w
	    subwf	(REGB)+2,f
	    bsf	    (REGA)+0,1		;Set current root bit

sremlt	bcf	    (REGA)+0,0		;Clear test bit
	    decfsz	MCOUNT,f	;Next
	    goto	sqloop

	    clrc
	    rrf	    (REGA)+2,f		;Adjust root alignment
	    rrf	    (REGA)+1,f
	    rrf	    (REGA)+0,f
	    return


;*** 32 BIT SIGNED BINARY TO DECIMAL ***
;REGA -> DIGITS 1 (MSD) TO 10 (LSD) & DSIGN
;DSIGN = 0 if REGA is positive, 1 if negative
;Return carry set if overflow
;Uses FSR register

bin2dec32
    	clrf	MTEMP		;Reset sign flag
	    call	absa		;Make REGA positive
	    skpnc
	    return			;Overflow

	    call	clrdig		;Clear all digits

	    movlw	D'32'		;Loop counter
	    movwf	MCOUNT

b2dloop	rlf	(REGA)+0,f		;Shift msb into carry
	    rlf	(REGA)+1,f
	    rlf	(REGA)+2,f
	    rlf	(REGA)+3,f

	    movlw	DIGIT10
	    movwf	FSR		;Pointer to digits
	    movlw	D'10'		;10 digits to do
	    movwf	DCOUNT

adjlp	rlf	INDF,f		;Shift digit and carry 1 bit left
	    movlw	D'10'
	    subwf	INDF,w		;Check and adjust for decimal overflow
	    skpnc
	    movwf	INDF

	    decf	FSR,f		;Next digit
	    decfsz	DCOUNT,f
	    goto	adjlp

	    decfsz	MCOUNT,f	;Next bit
	    goto	b2dloop

	    btfsc	MTEMP,0		;Check sign
	    bsf	DSIGN,0		;Negative
	    clrc
	    return


;*** 32 BIT SIGNED DECIMAL TO BINARY ***
;Decimal DIGIT1 thro DIGIT(X) & DSIGN -> REGA
;Set DSIGN = 0 for positive, DSIGN = 1 for negative values
;Most significant digit in DIGIT1
;Enter this routine with digit count in w register
;Return carry set if overflow
;Uses FSR register

dec2bin32
    	movwf	MTEMP		;Save digit count

	    movlw	D'32'		;Outer bit loop counter
	    movwf	MCOUNT

d2blp1	movlw	DIGITS-1	;Set up pointer to MSD
	    movwf	FSR
	    movf	MTEMP,w		;Inner digit loop counter
	    movwf	DCOUNT

	    movlw	D'10'
	    clrc			;Bring in '0' bit into MSD

d2blp2	incf	FSR,f
	    skpnc
	    addwf	INDF,f		;Add 10 if '1' bit from prev digit
	    rrf	    INDF,f		;Shift out LSB of digit

	    decfsz	DCOUNT,f	;Next L.S. Digit
	    goto	d2blp2

	    rrf	    (REGA)+3,f		;Shift in carry from digits
	    rrf	    (REGA)+2,f
	    rrf	    (REGA)+1,f
	    rrf	    (REGA)+0,f

	    decfsz	MCOUNT,f	;Next bit
	    goto	d2blp1

	    movf	INDF,w		;Check for overflow
	    addlw	0xFF
	    skpc
	    rlf	    (REGA)+3,w
	    skpnc
	    return

	    btfsc	DSIGN,0		;Check result sign
	    call	negatea		;Negative
	    return


;UTILITY ROUTINES


;Add REGB to REGA (Unsigned)
;Used by add, multiply,
uadd32
addba	movf	(REGB)+0,w		;Add lo byte
	    addwf	(REGA)+0,f

	    movf	(REGB)+1,w		;Add mid-lo byte
	    skpnc			;No carry_in, so just add
	    incfsz	(REGB)+1,w		;Add carry_in to REGB
	    addwf	(REGA)+1,f		;Add and propagate carry_out

	    movf	(REGB)+2,w		;Add mid-hi byte
	    skpnc
	    incfsz	(REGB)+2,w
	    addwf	(REGA)+2,f

	    movf	(REGB)+3,w		;Add hi byte
	    skpnc
	    incfsz	(REGB)+3,w
	    addwf	(REGA)+3,f
	    return


;Move REGA to REGC
;Used by multiply, sqrt

movac	movf	(REGA)+0,w
	    movwf	(REGC)+0
	    movf	(REGA)+1,w
	    movwf	(REGC)+1
	    movf	(REGA)+2,w
	    movwf	(REGC)+2
	    movf	(REGA)+3,w
	    movwf	(REGC)+3
	    return


;Clear REGB and REGA
;Used by sqrt

clrba	clrf	(REGB)+0
	    clrf	(REGB)+1
	    clrf	(REGB)+2
	    clrf	(REGB)+3

;Clear REGA
;Used by multiply, sqrt

clra	clrf	(REGA)+0
	    clrf	(REGA)+1
	    clrf	(REGA)+2
	    clrf	(REGA)+3
	    return


;Check sign of REGA and convert negative to positive
;Used by multiply, divide, bin2dec, round

abs32
absa	rlf	(REGA)+3,w
	    skpc
	    return			;Positive

;Negate REGA
;Used by absa, multiply, divide, bin2dec, dec2bin, round
negate32
negatea	movf	(REGA)+3,w		;Save sign in w
	    andlw	0x80

	    comf	(REGA)+0,f		;2's complement
	    comf	(REGA)+1,f
	    comf	(REGA)+2,f
	    comf	(REGA)+3,f
	    incfsz	(REGA)+0,f
	    goto	nega1
	    incfsz	(REGA)+1,f
	    goto	nega1
	    incfsz	(REGA)+2,f
	    goto	nega1
	    incf	(REGA)+3,f
nega1
	    incf	MTEMP,f		;flip sign flag
	    addwf	(REGA)+3,w		;Return carry set if -2147483648
	    return


;Check sign of REGB and convert negative to positive
;Used by multiply, divide

absb	rlf	    (REGB)+3,w
	    skpc
	    return			;Positive

;Negate REGB
;Used by absb, subtract, multiply, divide

negateb	movf	(REGB)+3,w		;Save sign in w
	    andlw	0x80

	    comf	(REGB)+0,f		;2's complement
	    comf	(REGB)+1,f
	    comf	(REGB)+2,f
	    comf	(REGB)+3,f
	    incfsz	(REGB)+0,f
	    goto	negb1
	    incfsz	(REGB)+1,f
	    goto	negb1
	    incfsz	(REGB)+2,f
	    goto	negb1
	    incf	(REGB)+3,f
negb1
	    incf	MTEMP,f		    ;flip sign flag
	    addwf	(REGB)+3,w		;Return carry set if -2147483648
	    return


;Shift left REGA and REGC
;Used by multiply, divide, round

slac	rlf	(REGA)+0,f
	    rlf	(REGA)+1,f
	    rlf	(REGA)+2,f
	    rlf	(REGA)+3,f
slc	    rlf	(REGC)+0,f
	    rlf	(REGC)+1,f
	    rlf	(REGC)+2,f
	    rlf	(REGC)+3,f
	    return


;Set all digits to 0
;Used by bin2dec

clrdig	clrf	DSIGN
	    clrf	DIGIT1
	    clrf	DIGIT2
	    clrf	DIGIT3
	    clrf	DIGIT4
	    clrf	DIGIT5
	    clrf	DIGIT6
	    clrf	DIGIT7
	    clrf	DIGIT8
	    clrf	DIGIT9
	    clrf	DIGIT10
	    return

        End