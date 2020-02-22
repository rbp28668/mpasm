;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/rs232_time.inc,v 1.3 2007/05/07 18:31:41 amb Exp $
;;
;; RS232 timing functions.
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2006 Andrew M. Bishop
;; It may be distributed under the GNU Public License, version 2, or
;; any higher version.  See section COPYING of the GNU Public license
;; for conditions under which this file may be redistributed.
;;
;;--------------------------------------------------------------------------------

;;--------------------------------------------------------------------------------
;;
;; Required Variables / Constants for rs232 functions
;;
;; Constants:   CLOCK           - The PIC clock speed in Hz
;;              RS232_BAUD      - Selects the baud rate
;;
;;--------------------------------------------------------------------------------


;;--------------------------------------------------------------------------------
;;
;; Subroutine to delay for one bit period (different versions for different bit periods )
;;
;; RAM Bank:    any (in) unchanged (out)
;;
;;--------------------------------------------------------------------------------

        IF      ( RS232_BAUD==9600 && CLOCK==20000000 ) || ( RS232_BAUD==4800 && CLOCK==10000000 ) || ( RS232_BAUD==2400 && CLOCK==5000000 )

;; Timing analysis (e.g. 20 MHz clock / 9600 = 520.833 cycles )
;; Static overhead = A = 10 + 6
;; Loop            = B = 101 * 5
;; Total           = 521

rs232_dly_bit
        nop                     ; 1 ins (A)
        movlw   102             ; 1 ins (A)
rs232_dly_bit_loop
        addlw   -1              ; 1 ins (B) / 1 ins (A)
        btfsc   STATUS,Z        ; 1 ins (A) / 2 ins (B)
        return                  ; 2 ins (A)
        goto    rs232_dly_bit_loop ; 2 ins (B)

;; Static overhead = A =  2 + 7
;; Loop            = B = 50 * 5
;; Total           = 259

rs232_dly_hbit
        movlw   51              ; 1 ins (A)
        goto    rs232_dly_bit_loop ; 2 ins (A)

        ENDIF

;;--------------------------------------------------------------------------------

        IF      ( RS232_BAUD==9600 && CLOCK==16000000 ) || ( RS232_BAUD==4800 && CLOCK==8000000 ) || ( RS232_BAUD==2400 && CLOCK==4000000 )

;; Timing analysis (e.g. 16 MHz clock / 9600 = 416.666 cycles )
;; Static overhead = A = 10 + 7
;; Loop            = B = 100 * 4
;; Total           = 417

rs232_dly_bit
        nop                     ; 1 ins (A)
        movlw   101             ; 1 ins (A)
rs232_dly_bit_loop
        addlw   -1              ; 1 ins (B) / 1 ins (A)
        btfss   STATUS,Z        ; 1 ins (B) / 2 ins (A)
        goto    rs232_dly_bit_loop ; 2 ins (B)
        return                  ; 2 ins (A)

;; Static overhead = A =  2 + 8
;; Loop            = B = 50 * 4
;; Total           = 210

rs232_dly_hbit
        movlw   51              ; 1 ins (A)
        goto    rs232_dly_bit_loop ; 2 ins (A)

        ENDIF

;;--------------------------------------------------------------------------------

        IF      ( RS232_BAUD==19200 && CLOCK==16000000 ) || ( RS232_BAUD==9600 && CLOCK==8000000 ) || ( RS232_BAUD==4800 && CLOCK==4000000 ) || ( RS232_BAUD==2400 && CLOCK==2000000 )

;; Timing analysis (e.g. 8 MHz clock / 9600 = 208.333 cycles )
;; Static overhead = A = 10 + 6
;; Loop            = B = 48 * 4
;; Total           = 208

rs232_dly_bit
        movlw   49              ; 1 ins (A)
rs232_dly_bit_loop
        addlw   -1              ; 1 ins (B) / 1 ins (A)
        btfss   STATUS,Z        ; 1 ins (B) / 2 ins (A)
        goto    rs232_dly_bit_loop ; 2 ins (B)
        return                  ; 2 ins (A)

;; Static overhead = A =  2 + 10
;; Loop            = B = 23 * 4
;; Total           = 104

rs232_dly_hbit
        goto    $+1             ; 2 ins (A)
        movlw   24              ; 1 ins (A)
        goto    rs232_dly_bit_loop ; 2 ins (A)

        ENDIF

;;--------------------------------------------------------------------------------

        IF      ( RS232_BAUD==38400 && CLOCK==16000000 ) || ( RS232_BAUD==19200 && CLOCK==8000000 ) || ( RS232_BAUD==9600 && CLOCK==4000000 ) || ( RS232_BAUD==4800 && CLOCK==2000000 ) || ( RS232_BAUD==2400 && CLOCK==1000000 )

;; Timing analysis (e.g. 4 MHz clock / 9600 = 104.167 cycles )
;; Static overhead = A = 10 + 6
;; Loop            = B = 22 * 4
;; Total           = 104

rs232_dly_bit
        movlw   23              ; 1 ins (A)
rs232_dly_bit_loop
        addlw   -1              ; 1 ins (B) / 1 ins (A)
        btfss   STATUS,Z        ; 1 ins (B) / 2 ins (A)
        goto    rs232_dly_bit_loop ; 2 ins (B)
        return                  ; 2 ins (A)

;; Static overhead = A =  2 + 10
;; Loop            = B = 10 * 4
;; Total           = 52

rs232_dly_hbit
        goto    $+1             ; 2 ins (A)
        movlw   11              ; 1 ins (A)
        goto    rs232_dly_bit_loop ; 2 ins (A)

        ENDIF

;;--------------------------------------------------------------------------------

        IF      ( RS232_BAUD==38400 && CLOCK==8000000 ) || ( RS232_BAUD==19200 && CLOCK==4000000 ) || ( RS232_BAUD==9600 && CLOCK==2000000 ) || ( RS232_BAUD==4800 && CLOCK==1000000 ) || ( RS232_BAUD==2400 && CLOCK==500000 )

;; Timing analysis (e.g. 4 MHz clock / 19200 = 52.083 cycles )
;; Static overhead = A = 10 + 6
;; Loop            = B =  9 * 4
;; Total           = 52

rs232_dly_bit
        movlw   10              ; 1 ins (A)
rs232_dly_bit_loop
        addlw   -1              ; 1 ins (B) / 1 ins (A)
        btfss   STATUS,Z        ; 1 ins (B) / 2 ins (A)
        goto    rs232_dly_bit_loop ; 2 ins (B)
        return                  ; 2 ins (A)

;; Static overhead = A = 2 + 8
;; Loop            = B = 4 * 4
;; Total           = 26

rs232_dly_hbit
        movlw   5               ; 1 ins (A)
        goto    rs232_dly_bit_loop ; 2 ins (A)

        ENDIF

;;--------------------------------------------------------------------------------

        IF      ( RS232_BAUD==38400 && CLOCK==4000000 ) || ( RS232_BAUD==19200 && CLOCK==2000000 ) || ( RS232_BAUD==9600 && CLOCK==1000000 ) || ( RS232_BAUD==4800 && CLOCK==500000 ) || ( RS232_BAUD==2400 && CLOCK==250000 )

;; Timing analysis (e.g. 4 MHz clock / 38400 = 26.042 cycles )
;; Static overhead = A = 10 + 6
;; Loop            = B =  2 * 5
;; Total           = 26

rs232_dly_bit
        nop                     ; 1 ins (A)
        movlw   3               ; 1 ins (A)
rs232_dly_bit_loop
        addlw   -1              ; 1 ins (B) / 1 ins (A)
        btfsc   STATUS,Z        ; 1 ins (A) / 2 ins (B)
        return                  ; 2 ins (A)
        goto    rs232_dly_bit_loop ; 2 ins (B)

;; Static overhead = A = 2 + 7
;; Loop            = B = 1 * 5
;; Total           = 14

rs232_dly_hbit
        movlw   2               ; 1 ins (A)
        goto    rs232_dly_bit_loop ; 2 ins (A)

        ENDIF

;;--------------------------------------------------------------------------------

        IF      ( RS232_BAUD==57600 && CLOCK==20000000 ) || ( RS232_BAUD==28800 && CLOCK==5000000 ) || ( RS232_BAUD==14400 && CLOCK==2500000 )

;; Timing analysis (e.g. 20 MHz clock / 57600 = 86.806 cycles )
;; Static overhead = A = 10 + 7
;; Loop            = B = 14 * 5
;; Total           = 87

rs232_dly_bit
        goto    $+1             ; 2 ins (A)
        movlw   15              ; 1 ins (A)
rs232_dly_bit_loop
        addlw   -1              ; 1 ins (B) / 1 ins (A)
        btfsc   STATUS,Z        ; 1 ins (A) / 2 ins (B)
        return                  ; 2 ins (A)
        goto    rs232_dly_bit_loop ; 2 ins (B)

;; Static overhead = A = 2 + 7
;; Loop            = B = 7 * 5
;; Total           = 44

rs232_dly_hbit
        movlw   8               ; 1 ins (A)
        goto    rs232_dly_bit_loop ; 2 ins (A)

        ENDIF

;;--------------------------------------------------------------------------------

        IF      ( RS232_BAUD==57600 && CLOCK==16000000 ) || ( RS232_BAUD==28800 && CLOCK==8000000 ) || ( RS232_BAUD==14400 && CLOCK==4000000 )

;; Timing analysis (e.g. 16 MHz clock / 57600 = 69.444 cycles )
;; Static overhead = A = 10 + 7
;; Loop            = B = 13 * 4
;; Total           = 69

rs232_dly_bit
        nop                     ; 1 ins (A)
        movlw   3               ; 1 ins (A)
rs232_dly_bit_loop
        addlw   -1              ; 1 ins (B) / 1 ins (A)
        btfss   STATUS,Z        ; 1 ins (B) / 2 ins (A)
        goto    rs232_dly_bit_loop ; 2 ins (B)
        return                  ; 2 ins (A)

;; Static overhead = A = 2 + 8
;; Loop            = B = 6 * 4
;; Total           = 34

rs232_dly_hbit
        movlw   7               ; 1 ins (A)
        goto    rs232_dly_bit_loop ; 2 ins (A)

        ENDIF

;;--------------------------------------------------------------------------------

        IF      ( RS232_BAUD==57600 && CLOCK==8000000 ) || ( RS232_BAUD==28800 && CLOCK==4000000 ) || ( RS232_BAUD==14400 && CLOCK==2000000 )

;; Timing analysis (e.g. 8 MHz clock / 57600 = 34.722 cycles )
;; Static overhead = A = 10 + 9
;; Loop            = B =  4 * 4
;; Total           = 35

rs232_dly_bit
        goto    $+1             ; 2 ins (A)
        nop                     ; 1 ins (A)
        movlw   5               ; 1 ins (A)
rs232_dly_bit_loop
        addlw   -1              ; 1 ins (B) / 1 ins (A)
        btfss   STATUS,Z        ; 1 ins (B) / 2 ins (A)
        goto    rs232_dly_bit_loop ; 2 ins (B)
        return                  ; 2 ins (A)

;; Static overhead = A = 2 + 8
;; Loop            = B = 2 * 4
;; Total           = 18

rs232_dly_hbit
        movlw   3               ; 1 ins (A)
        goto    rs232_dly_bit_loop ; 2 ins (A)

        ENDIF

;;--------------------------------------------------------------------------------

        IF      ( RS232_BAUD==57600 && CLOCK==4000000 ) || ( RS232_BAUD==28800 && CLOCK==2000000 ) || ( RS232_BAUD==14400 && CLOCK==1000000 )

;; Timing analysis (e.g. 4 MHz clock / 57600 = 17.361 cycles )
;; Static overhead = A = 10 + 7
;; Total           = 17

rs232_dly_bit
        goto    $+1             ; 2 ins (A)
        goto    $+1             ; 2 ins (A)
        nop                     ; 1 ins (A)
        return                  ; 2 ins (A)

;; Static overhead = A = 2 + 6
;; Total           = 8

rs232_dly_hbit
        goto    $+1             ; 2 ins (A)
        goto    $+1             ; 2 ins (A)
        return                  ; 2 ins (A)

        ENDIF


;;--------------------------------------------------------------------------------
