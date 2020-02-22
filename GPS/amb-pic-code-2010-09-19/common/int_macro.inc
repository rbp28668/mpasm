;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/int_macro.inc,v 1.2 2008/05/04 12:57:39 amb Exp $
;;
;; Include file for Interrupt macros.
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2007,2008 Andrew M. Bishop
;; It may be distributed under the GNU Public License, version 2, or
;; any higher version.  See section COPYING of the GNU Public license
;; for conditions under which this file may be redistributed.
;;
;;--------------------------------------------------------------------------------

;;--------------------------------------------------------------------------------
;;
;; Required Variables / Constants for Interrupt macro functions
;;
;; Variables:   int_temp_w       - Stores the W register, defined in all banks
;;                                 or the current bank when calling INT_PUSH
;;              int_temp_status  - Stores the STATUS register, defined in bank 0
;;
;; RAM Bank:    any (in)   BANK0 (out)    for INT_PUSH
;;              BANK0 (in) previous (out) for INT_POP
;;
;; These macros work because:
;;   * The instruction swapf doesn't set any flags in STATUS
;;   * The variable int_temp_w is available in all banks (simple case) or is
;;     available in the active bank when the INT_PUSH macro runs and STATUS is
;;     restored before it is reloaded.
;;   * The STATUS register is always stored in BANK0.
;;
;; The Microchip User Manual (DS31008A) is incorrect in the description of this.
;;   * In Example 8-1 STATUS_TEMP "must be defined in Bank0" but the push code
;;     does not switch to bank 0.  In fact for this code STATUS_TEMP must be
;;     available from all banks.
;;   * In Example 8-1 and Example 8-2 the comment on the first instruction for
;;     the pop code is "Swap original STATUS register value into W (restores
;;     original bank)" but it does not restore the bank, that is the following
;;     instruction.
;;
;;--------------------------------------------------------------------------------

INT_PUSH MACRO

        movwf   int_temp_w      ; Store W register
        swapf   STATUS,W        ; Copy STATUS to W
        bcf     STATUS,RP0      ; Change to bank 0
        movwf   int_temp_status ; Store W (containing original STATUS register)

        ENDM

;;--------------------------------------------------------------------------------

INT_POP MACRO

        swapf   int_temp_status,W ; Load W with original STATUS register
        movwf   STATUS          ; Restore STATUS register and select original bank
        swapf   int_temp_w,F
        swapf   int_temp_w,W    ; Restore original value of W

        ENDM

;;--------------------------------------------------------------------------------
