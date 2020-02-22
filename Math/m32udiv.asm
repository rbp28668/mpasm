; Math 32
; 32 bit division routines

        include "m32util.inc"

; External variables
        extern      REGA
        extern      REGB
        extern      REGC

; Temporary data
  
        extern      MCOUNT


; Export entry points
        global      udiv32

math32udiv   code
;*** 32 BIT UNSIGNED DIVIDE ***
;REGA / REGB -> REGA
;Remainder in REGC
;Return carry set if overflow or division by zero

udiv32	
        banksel REGA
        movf	(REGB)+0,w	;Trap division by zero
	    iorwf	(REGB)+1,w
	    iorwf	(REGB)+2,w
	    iorwf	(REGB)+3,w
	    sublw	0
	    skpnc
	    return			;Overflow

	    clrf	(REGC)+0		;Clear remainder
	    clrf	(REGC)+1
	    clrf	(REGC)+2
	    clrf	(REGC)+3

	    movlw	D'32'		;Loop counter
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

	    return

        End
