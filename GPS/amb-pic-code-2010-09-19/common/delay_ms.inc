;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/delay_ms.inc,v 1.6 2007/05/07 18:31:40 amb Exp $
;;
;; Delay for a number of milliseconds.
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
;; Required Variables / Constants for milliscond delay function
;;
;; Constants:   CLOCK           - The PIC clock speed in Hz
;;
;; Variables:   delay_temp      - Stores temporary data
;;
;;--------------------------------------------------------------------------------


;;--------------------------------------------------------------------------------
;;
;; Subroutine to delay for a number of milliseconds (different versions for different clocks)
;;
;; Parameters:  w (in)          - Number of milliseconds to wait for
;;
;; RAM Bank:    any (in), unchanged (out)
;;
;;--------------------------------------------------------------------------------


        IF      CLOCK==1000000

;; Timing analysis (at 1 MHz clock)
;; Load and call       =  3                     =    3
;; Static overhead = A = 11 + 4 * 59            =  247
;; Outer loop      = B = 10 * (delay-1)         = - 10 +  10 * delay
;; Inner loop      = C =  4 * 60 * (delay-1)    = -240 + 240 * delay
;; Total           = 250 * delay

delay_ms
        goto    $+1             ; 2 ins (A)
        movwf   delay_temp      ; 1 ins (A)
        movlw   60              ; 1 ins (A)

delay_ms_loop
        addlw   -1              ; 1 ins (A,B,C)
        btfss   STATUS,Z        ; 1 ins (C) / 2 ins (A,B)
        goto    delay_ms_loop   ; 2 ins (C)

        decf    delay_temp,F    ; 1 ins (A,B)
        btfsc   STATUS,Z        ; 2 ins (B) / 1 ins (A)
        return                  ; 2 ins (A)

        nop                     ; 1 ins (B)
        movlw   61              ; 1 ins (B)
        goto    delay_ms_loop   ; 2 ins (B)


        ENDIF   ; CLOCK==1000000

;;--------------------------------------------------------------------------------

        IF      CLOCK==2000000

;; Timing analysis (at 2 MHz clock)
;; Load and call       =  3                     =    3
;; Static overhead = A =  9 + 4 * 122           =  497
;; Outer loop      = B = 12 * (delay-1)         = - 12 +  12 * delay
;; Inner loop      = C =  4 * 122 * (delay-1)   = -488 + 488 * delay
;; Total           = 500 * delay

delay_ms
        movwf   delay_temp      ; 1 ins (A)
        movlw   123             ; 1 ins (A)

delay_ms_loop
        addlw   -1              ; 1 ins (A,B,C)
        btfss   STATUS,Z        ; 1 ins (C) / 2 ins (A,B)
        goto    delay_ms_loop   ; 2 ins (C)

        decf    delay_temp,F    ; 1 ins (A,B)
        btfsc   STATUS,Z        ; 2 ins (B) / 1 ins (A)
        return                  ; 2 ins (A)

        goto    $+1             ; 2 ins (B)
        nop                     ; 1 ins (B)
        movlw   123             ; 1 ins (B)
        goto    delay_ms_loop   ; 2 ins (B)


        ENDIF   ; CLOCK==2000000

;;--------------------------------------------------------------------------------

        IF      CLOCK==4000000

;; Timing analysis (at 4 MHz clock)
;; Load and call       =  3                     =    3
;; Static overhead = A =  9 + 4 * 247           =  997
;; Outer loop      = B = 12 * (delay-1)         = - 12 +  12 * delay
;; Inner loop      = C =  4 * 247 * (delay-1)   = -988 + 988 * delay
;; Total           = 1000 * delay

delay_ms
        movwf   delay_temp      ; 1 ins (A)
        movlw   248             ; 1 ins (A)

delay_ms_loop
        addlw   -1              ; 1 ins (A,B,C)
        btfss   STATUS,Z        ; 1 ins (C) / 2 ins (A,B)
        goto    delay_ms_loop   ; 2 ins (C)

        decf    delay_temp,F    ; 1 ins (A,B)
        btfsc   STATUS,Z        ; 2 ins (B) / 1 ins (A)
        return                  ; 2 ins (A)

        goto    $+1             ; 2 ins (B)
        nop                     ; 1 ins (B)
        movlw   248             ; 1 ins (B)
        goto    delay_ms_loop   ; 2 ins (B)


        ENDIF   ; CLOCK==4000000

;;--------------------------------------------------------------------------------

        IF      CLOCK==8000000

;; Timing analysis (at 8 MHz clock)
;; Load and call       =  3                     =     3
;; Static overhead = A = 13 + 8 * 248           =  1997
;; Outer loop      = B = 16 * (delay-1)         = -  16 +   16 * delay
;; Inner loop      = C =  8 * 248 * (delay-1)   = -1984 + 1984 * delay
;; Total           = 2000 * delay

delay_ms
        movwf   delay_temp      ; 1 ins (A)
        movlw   249             ; 1 ins (A)

delay_ms_loop
        goto    $+1             ; 2 ins (A,B,C)
        goto    $+1             ; 2 ins (A,B,C)
        addlw   -1              ; 1 ins (A,B,C)
        btfss   STATUS,Z        ; 1 ins (C) / 2 ins (A,B)
        goto    delay_ms_loop   ; 2 ins (C)

        decf    delay_temp,F    ; 1 ins (A,B)
        btfsc   STATUS,Z        ; 2 ins (B) / 1 ins (A)
        return                  ; 2 ins (A)

        goto    $+1             ; 2 ins (B)
        nop                     ; 1 ins (B)
        movlw   249             ; 1 ins (B)
        goto    delay_ms_loop   ; 2 ins (B)


        ENDIF   ; CLOCK==8000000

;;--------------------------------------------------------------------------------

        IF      CLOCK==16000000

;; Timing analysis (at 16 MHz clock)
;; Load and call       =  3                     =     3
;; Static overhead = A = 13 + 16 * 249          =  3997
;; Outer loop      = B = 16 * (delay-1)         = -  16 +   16 * delay
;; Inner loop      = C = 16 * 249 * (delay-1)   = -3984 + 3984 * delay
;; Total           = 4000 * delay

delay_ms
        movwf   delay_temp      ; 1 ins (A)
        movlw   250             ; 1 ins (A)
        goto    delay_ms_entry_A; 2 ins (A)

delay_ms_loop
        goto    $+1             ; 2 ins (C)
        goto    $+1             ; 2 ins (C)
        goto    $+1             ; 2 ins (C)
delay_ms_entry_B
        goto    $+1             ; 2 ins (B,C)
        goto    $+1             ; 2 ins (B,C)
delay_ms_entry_A
        goto    $+1             ; 2 ins (A,B,C)
        addlw   -1              ; 1 ins (A,B,C)
        btfss   STATUS,Z        ; 1 ins (C) / 2 ins (A,B)
        goto    delay_ms_loop   ; 2 ins (C)

        decf    delay_temp,F    ; 1 ins (A,B)
        btfsc   STATUS,Z        ; 2 ins (B) / 1 ins (A)
        return                  ; 2 ins (A)

        nop                     ; 1 ins (B)
        movlw   250             ; 1 ins (B)
        goto    delay_ms_entry_B; 2 ins (B)


        ENDIF   ; CLOCK==16000000

;;--------------------------------------------------------------------------------

        IF      CLOCK==20000000

;; Timing analysis (at 20 MHz clock)
;; Load and call       =  3                     =     3
;; Static overhead = A = 17 + 20 * 249          =  4997
;; Outer loop      = B = 20 * (delay-1)         = -  20 +   20 * delay
;; Inner loop      = C = 20 * 249 * (delay-1)   = -4980 + 4980 * delay
;; Total           = 5000 * delay

delay_ms
        movwf   delay_temp      ; 1 ins (A)
        movlw   250             ; 1 ins (A)
        goto    delay_ms_entry_A; 2 ins (A)

delay_ms_loop
        goto    $+1             ; 2 ins (C)
        goto    $+1             ; 2 ins (C)
        goto    $+1             ; 2 ins (C)
delay_ms_entry_B
        goto    $+1             ; 2 ins (B,C)
        goto    $+1             ; 2 ins (B,C)
delay_ms_entry_A
        goto    $+1             ; 2 ins (A,B,C)
        goto    $+1             ; 2 ins (A,B,C)
        goto    $+1             ; 2 ins (A,B,C)
        addlw   -1              ; 1 ins (A,B,C)
        btfss   STATUS,Z        ; 1 ins (C) / 2 ins (A,B)
        goto    delay_ms_loop   ; 2 ins (C)

        decf    delay_temp,F    ; 1 ins (A,B)
        btfsc   STATUS,Z        ; 2 ins (B) / 1 ins (A)
        return                  ; 2 ins (A)

        nop                     ; 1 ins (B)
        movlw   250             ; 1 ins (B)
        goto    delay_ms_entry_B; 2 ins (B)


        ENDIF   ; CLOCK==20000000

;;--------------------------------------------------------------------------------
