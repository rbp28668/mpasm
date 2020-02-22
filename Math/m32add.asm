; Math 32
; 32 bit add/subtract routines

        include "m32util.inc"

; External variables
        extern      REGA
        extern      REGB
        extern      REGC

; Temporary data
        extern      MTEMP

;Export entry points
        global      sub32
        global      add32


math32add   code

;*** 32 BIT SIGNED SUTRACT ***
;REGA - REGB -> REGA
;Return carry set if overflow

sub32
        banksel REGA
        pagesel negateb
	    call	negateb		;Negate REGB
	    skpnc
	    return			    ;Overflow

;*** 32 BIT SIGNED ADD ***
;REGA + REGB -> REGA
;Return carry set if overflow

add32	
        banksel REGA
        movf	(REGA)+3,w		;Compare signs
	    xorwf	(REGB)+3,w
	    
        movwf	MTEMP

        pagesel addba
	    call	addba		;Add REGB to REGA

	    clrc			    ;Check signs
	    movf	(REGB)+3,w	;If signs are same
	    xorwf	(REGA)+3,w	;so must result sign
	    btfss	MTEMP,7		;else overflow
	    addlw	0x80
	    return

        End