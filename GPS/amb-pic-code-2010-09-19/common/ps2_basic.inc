;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/ps2_basic.inc,v 1.3 2007/03/19 19:53:57 amb Exp $
;;
;; Basic hardware PS/2 functions.
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
;; Required Variables / Constants for PS/2 functions
;;
;; Constants:   CLOCK           - The PIC clock speed in Hz
;;              PS2_PORT        - Selects the port containing the clock/data bits
;;              PS2_TRIS        - Selects the tristate register for the PS2_PORT port
;;              PS2_DATA        - The bit to use for data
;;              PS2_CLK         - The bit to use for clock
;;
;;--------------------------------------------------------------------------------


        IF      CLOCK==4000000
PS2_TIMEOUT EQU     20          ; ~100 us (20 loops of 5 cycles each)
PS2_QPERIOD EQU      3          ;  ~10 us ( 3 loops of 3 cycles each + misc)
PS2_HPERIOD EQU     10          ;  ~30 us (12 loops of 3 cycles each + misc)
        ENDIF

        IF      CLOCK==8000000
PS2_TIMEOUT EQU     40          ; ~100 us (40 loops of 5 cycles each)
PS2_QPERIOD EQU      6          ;  ~10 us ( 6 loops of 3 cycles each + misc)
PS2_HPERIOD EQU     20          ;  ~30 us (20 loops of 3 cycles each + misc)
        ENDIF


;;--------------------------------------------------------------------------------

;; Temporarily disable messages and warnings

        ERRORLEVEL      2


;;--------------------------------------------------------------------------------
;;
;; Subroutine to initialise the port for PS/2
;;
;; RAM Bank:    any (in), PS2_PORT (out)
;;
;;--------------------------------------------------------------------------------

ps2_init_port
        BANKSEL PS2_TRIS
        bsf     PS2_TRIS,PS2_DATA; input
        bsf     PS2_TRIS,PS2_CLOCK; input

        BANKSEL PS2_PORT
        bcf     PS2_PORT,PS2_DATA; zero
        bcf     PS2_PORT,PS2_CLOCK; zero

        return


;;--------------------------------------------------------------------------------

;; Re-enable messages and warnings

        ERRORLEVEL      0


;;--------------------------------------------------------------------------------
