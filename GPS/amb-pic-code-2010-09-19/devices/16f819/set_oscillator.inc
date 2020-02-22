;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/16f819/RCS/set_oscillator.inc,v 1.4 2007/05/07 18:30:53 amb Exp $
;;
;; Function to set oscillator speed of 16F818/16F819 devices.
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2006 Andrew M. Bishop
;; It may be distributed under the GNU Public License, version 2, or
;; any higher version.  See section COPYING of the GNU Public license
;; for conditions under which this file may be redistributed.
;;
;;--------------------------------------------------------------------------------

;; Verify Processor Type

        IFNDEF __16F819
        IFNDEF __16F818
           ERROR "Processor-header file mismatch.  Verify selected processor."
        ENDIF
        ENDIF

;; Temporarily disable messages and warnings

        ERRORLEVEL      2

;;
;; Decide on which clock to use
;;

set_oscillator

        IF      CLOCK==31250

INT_OSC_VALUE   EQU     b'00000000'

        ENDIF   ; CLOCK==31250

        IF      CLOCK==125000

INT_OSC_VALUE   EQU     b'00010000'

        ENDIF   ; CLOCK==125000

        IF      CLOCK==250000

INT_OSC_VALUE   EQU     b'00100000'

        ENDIF   ; CLOCK==250000

        IF      CLOCK==500000

INT_OSC_VALUE   EQU     b'00110000'

        ENDIF   ; CLOCK==500000

        IF      CLOCK==1000000

INT_OSC_VALUE   EQU     b'01000000'

        ENDIF   ; CLOCK==1000000

        IF      CLOCK==2000000

INT_OSC_VALUE   EQU     b'01010000'

        ENDIF   ; CLOCK==2000000

        IF      CLOCK==4000000

INT_OSC_VALUE   EQU     b'01100000'

        ENDIF   ; CLOCK==4000000

        IF      CLOCK==8000000

INT_OSC_VALUE   EQU     b'01110000'

        ENDIF   ; CLOCK==8000000


;;
;; Subroutine to set the oscillator to the value of CLOCK
;;

set_internal_oscillator

        BANKSEL OSCCON          ; Select RAM bank

        movlw   b'10001111'     ; Mask for oscillator control
        andwf   OSCCON,F        ; Update oscillator control

        movlw   INT_OSC_VALUE+1 ; Select frequency and set SCS bit
        iorwf   OSCCON,F        ; Update oscillator control

wait_for_oscillator

        btfss   OSCCON,IOFS     ; Check if oscillator is stable
        goto    wait_for_oscillator

;; Finished

        return

;; Re-enable messages and warnings

        ERRORLEVEL      0
