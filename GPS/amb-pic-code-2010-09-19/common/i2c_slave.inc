;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/i2c_slave.inc,v 1.4 2010/09/19 09:43:38 amb Exp $
;;
;; I2C slave functions.
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2006, 2010 Andrew M. Bishop
;; It may be distributed under the GNU Public License, version 2, or
;; any higher version.  See section COPYING of the GNU Public license
;; for conditions under which this file may be redistributed.
;;
;;--------------------------------------------------------------------------------

;;--------------------------------------------------------------------------------
;;
;; Required Variables / Constants for I2C functions
;;
;; Constants:   CLOCK           - The PIC clock speed in Hz
;;              I2C_PORT        - Selects the port containing the SDA/SCL bits
;;              I2C_TRIS        - Selects the tristate register for the I2C_PORT port
;;              I2C_SDA         - The bit to use for data (SDA)
;;              I2C_SCL         - The bit to use for clock (SCL)
;;
;; Variables:   i2c_temp        - Stores temporary data, same bank(s) as I2C_PORT and I2C_TRIS
;;
;;--------------------------------------------------------------------------------


;;--------------------------------------------------------------------------------
;;
;; Subroutine for I2C slave devices
;;
;; Parameters:  w (in)          - TBD
;;              w (out)         - TBD
;;
;; RAM Bank:    TBD (in) TBD (out)
;;
;;--------------------------------------------------------------------------------

