;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/ps2.inc,v 1.3 2007/03/19 19:53:40 amb Exp $
;;
;; Main include file for PS/2 functions.
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
;;              PS2_CLOCK       - The bit to use for clock
;;
;; Variables:   ps2_data        - Stores the data byte, same bank as PS2_PORT
;;              ps2_status      - Stores the status, same bank as PS2_PORT
;;              ps2_temp        - Stores temporary data, same bank as PS2_PORT
;;
;;--------------------------------------------------------------------------------


;;
;; PS/2 Basic functions
;;

        include "../common/ps2_basic.inc"

;;
;; PS/2 Host functions (bit-banging)
;;

        include "../common/ps2_host.inc"

;;
;; PS/2 Device functions (bit-banging)
;;

        include "../common/ps2_device.inc"

;;
;; PS/2 Monitor functions (bit-banging)
;;

        include "../common/ps2_monitor.inc"

;;--------------------------------------------------------------------------------
