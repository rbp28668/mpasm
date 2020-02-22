;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/delay_us.inc,v 1.4 2007/05/07 18:31:40 amb Exp $
;;
;; Delay for a number of microseconds.
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
;; Required Variables / Constants for microsecond delay function
;;
;; Constants:   CLOCK           - The PIC clock speed in Hz
;;
;; Variables:   delay_temp      - Stores temporary data
;;
;;--------------------------------------------------------------------------------


;;--------------------------------------------------------------------------------
;;
;; Subroutine to delay for a number of microseconds (different for different clocks)
;;
;; Parameters:  w (in)          - Number of microseconds to wait for
;;
;; RAM Bank:    any (in), unchanged (out)
;;
;;--------------------------------------------------------------------------------

        IF      CLOCK==4000000

;; Timing analysis (at 4 MHz clock)
;; Minimum delay = 25 us
;; Total         = delay (including load and call overhead)

delay_us                        ;               [0mod4  1mod4  2mod4  3mod4]
        movwf   delay_temp      ; 1 ins          1      1      1      1
        movlw   b'00000011'     ; 1 ins          2      2      2      2
        andwf   delay_temp,W    ; 1 ins          3      3      3      3
        btfsc   STATUS,Z        ; 1 ins / 2 ins  4      5      5      5
        goto    delay_us_0mod4  ; 2 ins          6
        addlw   -1              ; 1 ins                 6      6      6
        btfsc   STATUS,Z        ; 1 ins / 2 ins         7      8      8
        goto    delay_us_1mod4  ; 2 ins                 9
        addlw   -1              ; 1 ins                        9      9
        btfsc   STATUS,Z        ; 1 ins / 2 ins               10     11
        goto    delay_us_2mod4  ; 2 ins                       12       

delay_us_3mod4
delay_us_1mod4
        nop                     ; 1 ins                10            12
        movlw   5               ; 1 ins                11            13
        goto    delay_4us       ; 2 ins                13            15

delay_us_0mod4
        goto    $+1             ; 2 ins          8
        goto    $+1             ; 2 ins         10
delay_us_2mod4
        nop                     ; 1 ins         11            13
        movlw   5               ; 1 ins         12            14

delay_4us                       ; static delay 4 ins = 4us
        rrf     delay_temp,F    ; 1 ins
        rrf     delay_temp,F    ; 1 ins
        subwf   delay_temp,W    ; 1 ins
        andlw   b'00111111'     ; 1 ins

delay_4us_loop                  ; static delay 5+3 ins = 8us
        addlw   -1              ; 1 ins (static,loop)
        btfss   STATUS,Z        ; 1 ins (loop) / 2 ins (static)
        goto    delay_4us_loop  ; 2 ins (loop)
        return                  ; 2 ins (static)

        ENDIF   ; CLOCK==4000000

;;--------------------------------------------------------------------------------

        IF      CLOCK==8000000

;; Timing analysis (at 8 MHz clock)
;; Minimum delay = 7 us
;; Total         = delay (including load and call overhead)

delay_us                        ;               [0mod2  1mod2]
        movwf   delay_temp      ; 1 ins          1      1
        btfsc   delay_temp,0    ; 1 ins / 2 ins  3      2
        goto    delay_2us       ; 2 ins                 4

delay_2us                       ; static delay 5 ins = 2.5us
        nop                     ; 1 ins
        rrf     delay_temp,F    ; 1 ins
        movlw   3               ; 1 ins
        subwf   delay_temp,W    ; 1 ins
        andlw   b'01111111'     ; 1 ins

delay_2us_loop                  ; static delay 5+3 ins = 4us
        addlw   -1              ; 1 ins (static,loop)
        btfss   STATUS,Z        ; 1 ins (loop) / 2 ins (static)
        goto    delay_2us_loop  ; 2 ins (loop)
        return                  ; 2 ins (static)

        ENDIF   ; CLOCK==8000000

;;--------------------------------------------------------------------------------

        IF      CLOCK==16000000

;; Timing analysis (at 16 MHz clock)
;; Minimum delay = 3 us
;; Total         = delay (including load and call overhead)

delay_us
delay_1us                       ; static delay 4 ins = 1us
        goto    $+1             ; 2 ins
        nop                     ; 1 ins
        addlw   -2              ; 1 ins

delay_1us_loop                  ; static delay 5+3 ins = 2us
        addlw   -1              ; 1 ins (static,loop)
        btfss   STATUS,Z        ; 1 ins (loop) / 2 ins (static)
        goto    delay_1us_loop  ; 2 ins (loop)
        return                  ; 2 ins (static)

        ENDIF   ; CLOCK==16000000

;;--------------------------------------------------------------------------------

        IF      CLOCK==20000000

;; Timing analysis (at 20 MHz clock)
;; Minimum delay = 2 us
;; Total         = delay (including load and call overhead)

delay_us
delay_1us                       ; static delay 3 ins = 0.6us
        goto    $+1             ; 2 ins
        addlw   -1              ; 1 ins

delay_1us_loop                  ; static delay 4+3 ins = 1.4us
        addlw   -1              ; 1 ins (static,loop)
        btfsc   STATUS,Z        ; 1 ins (static) / 2 ins (loop)
        return                  ; 2 ins (static)
        goto    delay_1us_loop  ; 2 ins (loop)

        ENDIF   ; CLOCK==20000000

;;--------------------------------------------------------------------------------
