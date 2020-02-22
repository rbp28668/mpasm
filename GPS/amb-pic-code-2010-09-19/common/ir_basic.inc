;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/ir_basic.inc,v 1.3 2007/05/07 18:31:40 amb Exp $
;;
;; Basic (hardware) Infra-Red functions.
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
;;              IR_TRIS         - Selects the tristate register for the IR_PORT
;;              IR_TXD          - The bit to use for TX data output
;;              IR_RXD          - The bit to use for RX data input
;;              IR_T            - Selects the basic time period
;;
;;--------------------------------------------------------------------------------


;;--------------------------------------------------------------------------------

;; Temporarily disable messages and warnings

        ERRORLEVEL      2


;;--------------------------------------------------------------------------------
;;
;; Subroutine to initialise the port for Infra-Red
;;
;; RAM Bank:    any (in) IR_PORT(out)
;;
;;--------------------------------------------------------------------------------

ir_init_port
        BANKSEL IR_TRIS

        IFDEF   IR_TXD
        bcf     IR_TRIS,IR_TXD  ; output
        ENDIF

        IFDEF   IR_RXD
        bsf     IR_TRIS,IR_RXD  ; input
        ENDIF

        BANKSEL IR_PORT

        IFDEF   IR_TXD
        bcf     IR_PORT,IR_TXD ; no output
        ENDIF

        return


;;--------------------------------------------------------------------------------

;; Re-enable messages and warnings

        ERRORLEVEL      0


;;--------------------------------------------------------------------------------
