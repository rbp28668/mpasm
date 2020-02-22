; Math 32
; BCD/Decimal conversion ROUTINES

; Define FSR and INDF rather than building for specific PIC.
        ;include "P16F818.inc"
FSR         equ 0x04
INDF        equ 0x00

; External variables used by this
            extern REGA
            extern MCOUNT
            extern MTEMP
            extern DCOUNT
            extern DIGITS
            extern DSIGN

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


; Export entry points
            global      bin2dec32
            global      dec2bin32
            global      clrdig

; External entry points used by this
            extern      absa
            extern      negatea
            
math32bcd   code

;*** 32 BIT SIGNED BINARY TO DECIMAL ***
;REGA -> DIGITS 1 (MSD) TO 10 (LSD) & DSIGN
;DSIGN = 0 if REGA is positive, 1 if negative
;Return carry set if overflow
;Uses FSR register

bin2dec32
        banksel REGA
    	clrf	MTEMP		;Reset sign flag
        pagesel absa
	    call	absa		;Make REGA positive
	    skpnc
	    return			;Overflow

        pagesel clrdig      ; on this page
	    call	clrdig		;Clear all digits

	    movlw	D'32'		;Loop counter
	    movwf	MCOUNT

        bankisel DIGIT10    ; as will use indirect addr for this
b2dloop	
        rlf	(REGA)+0,f		;Shift msb into carry
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
	    bsf	    DSIGN,0		;Negative
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
        banksel REGA
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
        pagesel negatea
	    call	negatea		;Negative
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