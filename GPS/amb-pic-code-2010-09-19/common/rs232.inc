;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/rs232.inc,v 1.4 2007/05/07 18:31:41 amb Exp $
;;
;; Main include file for RS232 functions.
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
;;              RS232_PORT      - Selects the port containing the RXD/TXD bits
;;              RS232_TRIS      - Selects the tristate register for the RS232_PORT
;;              RS232_TXD       - The bit to use for TX data output
;;              RS232_RXD       - The bit to use for RX data input
;;              RS232_RTS       - The bit to use for RTS data output
;;              RS232_CTS       - The bit to use for CTS data input
;;
;; Variables:   rs232_temp      - Stores temporary data, same bank as RS232_PORT
;;
;;--------------------------------------------------------------------------------


;;
;; RS232 Basic functions
;;

        include "../common/rs232_basic.inc"

;;
;; RS232 Transmit functions (bit-banging)
;;

        include "../common/rs232_tx.inc"

;;
;; RS232 Receive functions (bit-banging)
;;

        include "../common/rs232_rx.inc"

;;
;; RS232 Timing functions
;;

        include "../common/rs232_time.inc"

;;--------------------------------------------------------------------------------
