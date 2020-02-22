; Math 32
; 32 bit multiply routines

        include "m32util.inc"

; External variables
        extern      REGA
        extern      REGB
        extern      REGC

; Temporary data
        extern MCOUNT
        extern MTEMP


; Export entry points
        global      mul32
        global      umul32

math32mult code
;*** 32 BIT SIGNED MULTIPLY ***
;REGA * REGB -> REGA
;Return carry set if overflow

mul32   
        banksel REGA
	    clrf	MTEMP		;Reset sign flag
        pagesel absa
	    call	absa		;Make REGA positive
        pagesel absb
	    skpc
	    call	absb		;Make REGB positive
	    skpnc
	    return			    ;Overflow
        pagesel movac
	    call	movac		;Move REGA to REGC
        pagesel clra
	    call	clra		;Clear product

	    movlw	D'31'		;Loop counter
	    movwf	MCOUNT

muloop	
        pagesel slac
        call	slac		;Shift left product and multiplicand
	
	    rlf	    (REGC)+3,w	;Test MSB of multiplicand
        pagesel addba
	    skpnc			    ;If multiplicand bit is a 1 then
	    call	addba		;add multiplier to product

	    skpc			    ;Check for overflow
	    rlf	    (REGA)+3,w
	    skpnc
	    return

        pagesel muloop
        banksel MCOUNT
	    decfsz	MCOUNT,f	;Next
	    goto	muloop

	    btfsc	MTEMP,0		;Check result sign
	    call	negatea		;Negative
	    return

;*** 32 BIT UNSIGNED MULTIPLY ***
;REGA * REGB -> REGA
;No checking of overflow 
umul32  
        banksel REGA
        pagesel movac
	    call	movac		;Move REGA to REGC
        pagesel clra
	    call	clra		;Clear product

	    movlw	D'32'		;Loop counter 
	    movwf	MCOUNT

        ; product in A
        ; multiplicand in C
        ; multiplier in B
umuloop	
        clrc                ;Clear carry before shift in
        pagesel slac
        call	slac		;Shift left product and multiplicand

        pagesel addba
	    skpnc			    ;If multiplicand bit is a 1 then
	    call	addba		;add multiplier to product
 
        pagesel umuloop
	    decfsz	MCOUNT,f	;Next
	    goto	umuloop

	    return

		End