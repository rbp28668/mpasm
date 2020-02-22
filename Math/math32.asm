;===========================================================
; math32.asm
; 
; http://www.piclist.com/techref/microchip/math/32bmath-ph.htm
;===========================================================

        TITLE "32 bit Math routines"
;        LIST P=PIC16F818
;        include "P16F818.inc"
       	

		radix dec


math32  udata

;Accumulators
REGA        res 4
REGB        res 4
REGC        res 4

; Internal variables, common to most routines
; put here to ensure stay on same page as accumulators
MCOUNT      res 1
MTEMP       res 1
DCOUNT      res 1

;Digits and sign for BCD <-> binary conversion
DSIGN		res 1		;Digit Sign. 0=positive,1=negative
DIGITS      res 10

;Export data
        global      REGA
        global      REGB
        global      REGC

        global      MCOUNT
        global      MTEMP
        global      DCOUNT

        global  DSIGN
        global  DIGITS

        End