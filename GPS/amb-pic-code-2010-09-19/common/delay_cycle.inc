;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/delay_cycle.inc,v 1.4 2007/05/07 18:31:40 amb Exp $
;;
;; Delay for a number of clock cycles.
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
;; Required Variables / Constants for instruction cycle delay functions
;;
;;--------------------------------------------------------------------------------


;;--------------------------------------------------------------------------------
;;
;; Subroutines to delay for a number of instruction cycles
;;
;; Parameters:  none
;;
;; RAM Bank:    any (in), unchanged (out)
;;
;;--------------------------------------------------------------------------------

delay_30_cycles
        goto    $+1             ; 2 ins
delay_28_cycles
        goto    $+1             ; 2 ins
delay_26_cycles
        goto    $+1             ; 2 ins
delay_24_cycles
        goto    $+1             ; 2 ins
delay_22_cycles
        goto    $+1             ; 2 ins
delay_20_cycles
        goto    $+1             ; 2 ins
delay_18_cycles
        goto    $+1             ; 2 ins
delay_16_cycles
        goto    $+1             ; 2 ins
delay_14_cycles
        goto    $+1             ; 2 ins
delay_12_cycles
        goto    $+1             ; 2 ins
delay_10_cycles
        goto    $+1             ; 2 ins
delay_8_cycles
        goto    $+1             ; 2 ins
delay_6_cycles
        goto    $+1             ; 2 ins
delay_4_cycles
        return                  ; 2 ins (+ 2 ins for call)


;;--------------------------------------------------------------------------------
