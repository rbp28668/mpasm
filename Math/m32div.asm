; Math 32
; 32 bit division routines

        include "m32util.inc"

; External variables
        extern      REGA
        extern      REGB
        extern      REGC
; Temporary data
        extern MCOUNT
        extern MTEMP 


; Export entry points
        global      div32
        global      round32

math32div   code
;*** 32 BIT SIGNED DIVIDE ***
;REGA / REGB -> REGA
;Remainder in REGC
;Return carry set if overflow or division by zero

div32	
        banksel REGA
        clrf	MTEMP		;Reset sign flag
	    movf	(REGB)+0,w	;Trap division by zero
	    iorwf	(REGB)+1,w
	    iorwf	(REGB)+2,w
	    iorwf	(REGB)+3,w
	    sublw	0
        pagesel absa
	    skpc
	    call	absa		;Make dividend (REGA) positive
        pagesel absb
	    skpc
	    call	absb		;Make divisor (REGB) positive
	    skpnc
	    return			;Overflow

	    clrf	(REGC)+0		;Clear remainder
	    clrf	(REGC)+1
	    clrf	(REGC)+2
	    clrf	(REGC)+3

        pagesel slac
	    call	slac		;Purge sign bit

	    movlw	D'31'		;Loop counter
	    movwf	MCOUNT

dvloop	
        pagesel slac
        call	slac		;Shift dividend (REGA) msb into remainder (REGC)
        pagesel $
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

        pagesel negatea
	    btfsc	MTEMP,0		;Check result sign
	    call	negatea		;Negative
	    return

;*** ROUND RESULT OF DIVISION TO NEAREST INTEGER ***

round32	
        banksel REGA
        clrf	MTEMP		;Reset sign flag
        pagesel absa
	    call	absa		;Make positive
	    clrc
        pagesel slc
	    call	slc		;Multiply remainder by 2
        pagesel $
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
        pagesel negatea
	    call	negatea
	    return


        End
