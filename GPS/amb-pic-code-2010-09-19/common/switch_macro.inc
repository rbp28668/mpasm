;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/switch_macro.inc,v 1.1 2007/06/16 10:48:55 amb Exp $
;;
;; Include file for switch statement macro.
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2007 Andrew M. Bishop
;; It may be distributed under the GNU Public License, version 2, or
;; any higher version.  See section COPYING of the GNU Public license
;; for conditions under which this file may be redistributed.
;;
;;--------------------------------------------------------------------------------

;;--------------------------------------------------------------------------------
;;
;; Required Variables / Constants for switch statement macro function
;;
;; Variables:   value            - Contains the value to switch on
;;
;; RAM Bank:    any (in)   any (out)
;;
;; The code is that which is described in Microchip App note 556 example 5.
;;
;; The macro takes care of all 13 bits which means that the switch statement
;; can span a 256 byte boundary in program memory.
;;
;;--------------------------------------------------------------------------------

SWITCH  MACRO value

        movlw   LOW ($+9)       ; low 8 bits of address
        addwf   value,F         ; 8 bit addition

        movlw   HIGH ($+7)      ; high 5 bits of address
        btfsc   STATUS,C        ; skip if no 8 bit add overflow
        addlw   1
        movwf   PCLATH          ; load high address in latch

        movf    value,W         ; Move modified value into W
        goto    $+1             ; Jump to cause update of PCL high bits
        movwf   PCL             ; set PCL low bits

        ENDM

;;--------------------------------------------------------------------------------
