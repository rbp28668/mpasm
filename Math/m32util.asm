; Math 32
; UTILITY ROUTINES

; Export External entry points
        global      uadd32
        global      abs32
        global      negate32

; Export Internal math32 package entry points
        global      addba
        global      movac
        global      clrba
        global      clra
        global      absa
        global      absb
        global      negatea
        global      negateb
        global      slac
        global      slc

; External variables
        extern      REGA
        extern      REGB
        extern      REGC

; Temporary data
        extern MTEMP
        extern MCOUNT

math32util  code

;Add REGB to REGA (Unsigned)
;Used by add, multiply,
uadd32 
addba	
        banksel REGA
        movf	(REGB)+0,w		;Add lo byte
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

movac	
        banksel REGA
        movf	(REGA)+0,w
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

clrba	
        clrf	(REGB)+0
	    clrf	(REGB)+1
	    clrf	(REGB)+2
	    clrf	(REGB)+3

;Clear REGA
;Used by multiply, sqrt

clra	
        clrf	(REGA)+0
	    clrf	(REGA)+1
	    clrf	(REGA)+2
	    clrf	(REGA)+3
	    return


;Check sign of REGA and convert negative to positive
;Used by multiply, divide, bin2dec, round

abs32
absa	
        banksel REGA
        rlf	(REGA)+3,w
	    skpc
	    return			;Positive

;Negate REGA
;Used by absa, multiply, divide, bin2dec, dec2bin, round
negate32
negatea	
        banksel REGA
        movf	(REGA)+3,w		;Save sign in w
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



        End