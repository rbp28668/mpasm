;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/rs232_basic.inc,v 1.3 2007/05/07 18:31:41 amb Exp $
;;
;; Basic (hardware) RS232 functions.
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
;; Constants:   RS232_PORT      - Selects the port containing the RXD/TXD bits
;;              RS232_TRIS      - Selects the tristate register for the RS232_PORT
;;              RS232_TXD       - The bit to use for TX data output
;;              RS232_RXD       - The bit to use for RX data input
;;              RS232_RTS       - The bit to use for RTS data output
;;              RS232_CTS       - The bit to use for CTS data input
;;
;;--------------------------------------------------------------------------------


;;--------------------------------------------------------------------------------

;; Temporarily disable messages and warnings

        ERRORLEVEL      2


;;--------------------------------------------------------------------------------
;;
;; Subroutine to initialise the port for RS232
;;
;; RAM Bank:    any (in) RS232_PORT(out)
;;
;;--------------------------------------------------------------------------------

rs232_init_port
        BANKSEL RS232_TRIS

        IFDEF   RS232_TXD
        bcf     RS232_TRIS,RS232_TXD ; output
        ENDIF

        IFDEF   RS232_RXD
        bsf     RS232_TRIS,RS232_RXD ; input
        ENDIF

        IFDEF   RS232_RTS
        bcf     RS232_TRIS,RS232_RTS ; output
        ENDIF

        IFDEF   RS232_CTS
        bsf     RS232_TRIS,RS232_CTS ; input
        ENDIF

        BANKSEL RS232_PORT

        IFDEF   RS232_TXD
        bsf     RS232_PORT,RS232_TXD ; output (normally high between bytes)
        ENDIF

        IFDEF   RS232_RTS
        bsf     RS232_PORT,RS232_RTS ; output (high to disable reception)
        ENDIF

        return


;;--------------------------------------------------------------------------------

;; Re-enable messages and warnings

        ERRORLEVEL      0


;;--------------------------------------------------------------------------------
