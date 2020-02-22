;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/ir_time.inc,v 1.4 2007/05/07 18:31:41 amb Exp $
;;
;; Infra-Red timing functions.
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
;; Required Variables / Constants for Infra-Red functions
;;
;; Constants:   CLOCK           - The PIC clock speed in Hz
;;              IR_PORT         - Selects the port containing the RXD/TXD bits
;;              IR_TXD          - The bit to use for TX data input
;;
;; Variables:   ir_temp         - Temporary byte
;;
;;--------------------------------------------------------------------------------


;;--------------------------------------------------------------------------------
;;
;; Subroutine to delay for a number of cycles, optionally transmitting a modulated carrier.
;;
;; Parameters:  w (in)          - Number of cycles (of 26 us each)
;;              STATUS,C (in)   - If set then modulate, else don't
;;
;; RAM Bank:    IR_PORT (in) IR_PORT (out)
;;
;;--------------------------------------------------------------------------------

        IF CLOCK==4000000

;; Timing analysis
;; Static (A) = 8 ins + 4 ins   = 12 us
;; Loop   (B) = loop * 26 ins   = loop * 26 us
;; Total      = loop * 26 + 12

ir_cycle_delay
        movwf   ir_temp         ; 1 ins (A)

        btfsc   STATUS,C        ; 1 ins (A) / 2 ins (A)
        goto    ir_cycle_delay_tx_loop ; 2 ins (A)
        nop                     ; 1 ins (A)

ir_cycle_delay_no_tx_loop
        call    delay_22_cycles ;22 ins (B)

        nop                     ; 1 ins (B)
        decfsz  ir_temp,F       ; 2 ins (A) / 1 ins (B)
        goto    ir_cycle_delay_no_tx_loop ; 2 ins (B)
        return                  ; 2 ins (A)

ir_cycle_delay_tx_loop
        call    delay_10_cycles ;10 ins (B)
        IFDEF   IR_TXD
        bsf     IR_PORT,IR_TXD  ; 1 ins (B)
        ELSE
        nop                     ; 1 ins (B)
        ENDIF

        call    delay_10_cycles ; 10 ins (B)
        IFDEF   IR_TXD
        bcf     IR_PORT,IR_TXD  ; 1 ins (B)
        ELSE
        nop                     ; 1 ins (B)
        ENDIF

        nop                     ; 1 ins (B)
        decfsz  ir_temp,F       ; 2 ins (A) / 1 ins (B)
        goto    ir_cycle_delay_tx_loop ; 2 ins (B)
        return                  ; 2 ins (A)

        ENDIF  ; CLOCK==4000000


;;--------------------------------------------------------------------------------

        IF CLOCK==8000000

;; Timing analysis
;; Static (A) = 8 ins + 4 ins   = 12 us
;; Loop   (B) = loop * 52 ins   = loop * 26 us
;; Total      = loop * 26 us + 6 us

ir_cycle_delay
        movwf   ir_temp         ; 1 ins (A)

        btfsc   STATUS,C        ; 1 ins (A) / 2 ins (A)
        goto    ir_cycle_delay_tx_loop ; 2 ins (A)
        nop                     ; 1 ins (A)

ir_cycle_delay_no_tx_loop
        call    delay_20_cycles ;20 ins (B)
        call    delay_28_cycles ;28 ins (B)

        nop                     ; 1 ins (B)
        decfsz  ir_temp,F       ; 2 ins (A) / 1 ins (B)
        goto    ir_cycle_delay_no_tx_loop ; 2 ins (B)
        return                  ; 2 ins (A)

ir_cycle_delay_tx_loop
        call    delay_24_cycles ;24 ins (B)
        IFDEF   IR_TXD
        bsf     IR_PORT,IR_TXD  ; 1 ins (B)
        ELSE
        nop                     ; 1 ins (B)
        ENDIF

        call    delay_22_cycles ;22 ins (B)
        IFDEF   IR_TXD
        bcf     IR_PORT,IR_TXD  ; 1 ins (B)
        ELSE
        nop                     ; 1 ins (B)
        ENDIF

        nop                     ; 1 ins (B)
        decfsz  ir_temp,F       ; 2 ins (A) / 1 ins (B)
        goto    ir_cycle_delay_tx_loop ; 2 ins (B)
        return                  ; 2 ins (A)

        ENDIF  ; CLOCK==8000000


;;--------------------------------------------------------------------------------
