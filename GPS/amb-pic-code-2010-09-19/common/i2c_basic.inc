;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/i2c_basic.inc,v 1.3 2007/05/07 18:31:40 amb Exp $
;;
;; Basic hardware I2C functions.
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
;; Required Variables / Constants for I2C functions
;;
;; Constants:   I2C_PORT        - Selects the port containing the SDA/SCL bits
;;              I2C_TRIS        - Selects the tristate register for the I2C_PORT port
;;              I2C_SDA         - The bit to use for data (SDA)
;;              I2C_SCL         - The bit to use for clock (SCL)
;;
;;--------------------------------------------------------------------------------


;;--------------------------------------------------------------------------------

;; Temporarily disable messages and warnings

        ERRORLEVEL      2


;;--------------------------------------------------------------------------------
;;
;; Subroutine to initialise the port for I2C
;;
;; RAM Bank:    any (in), I2C_PORT (out)
;;
;;--------------------------------------------------------------------------------

i2c_init_port
        BANKSEL I2C_TRIS
        bsf     I2C_TRIS,I2C_SDA
        bsf     I2C_TRIS,I2C_SCL

        BANKSEL I2C_PORT
        bcf     I2C_PORT,I2C_SDA
        bcf     I2C_PORT,I2C_SCL

        return


;;--------------------------------------------------------------------------------

;; Re-enable messages and warnings

        ERRORLEVEL      0


;;--------------------------------------------------------------------------------
