; Math 32
; 32 bit square root routines

        include "m32util.inc"

; External variables
        extern      REGA
        extern      REGB
        extern      REGC

; Temporary data
        extern      MCOUNT


; Export entry points
        global      sqrt32

math32sqrt		code

;*** 32 BIT SQUARE ROOT ***
;sqrt(REGA) -> REGA
;Return carry set if negative

		
sqrt32	
        banksel REGA
        rlf	(REGA)+3,w		;Trap negative values
	    skpnc
	    return

	    pagesel movac
        call	movac		;Move REGA to REGC
        pagesel clrba
	    call	clrba		;Clear remainder (REGB) and root (REGA)

        pagesel $

	    movlw	D'16'		;Loop counter
	    movwf	MCOUNT

sqloop  rlf	    (REGC)+0,f		;Shift two msb's
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

		End