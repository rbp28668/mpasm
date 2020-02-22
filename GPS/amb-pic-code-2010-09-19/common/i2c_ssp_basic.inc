;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/i2c_ssp_basic.inc,v 1.1 2007/04/29 10:34:02 amb Exp $
;;
;; Basic hardware I2C functions using SSP module.
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2006,07 Andrew M. Bishop
;; It may be distributed under the GNU Public License, version 2, or
;; any higher version.  See section COPYING of the GNU Public license
;; for conditions under which this file may be redistributed.
;;
;;--------------------------------------------------------------------------------

;;--------------------------------------------------------------------------------
;;
;; Required Variables / Constants for I2C functions
;;
;; Constants:   SSP_PORT        - Selects the port containing the SSP port (automatically set)
;;              SSP_TRIS        - Selects the tristate register for the SSP port (automatically set)
;;              SSP_SDA         - The bit to use for SDA data in/out (automatically set)
;;              SSP_SCL         - The bit to use for SCL clock in/out (automatically set)
;;              SSP_I2C_ADDRESS - The address to use for the slave address
;;
;;--------------------------------------------------------------------------------


;;--------------------------------------------------------------------------------

;; Temporarily disable messages and warnings

        ERRORLEVEL      2


;;--------------------------------------------------------------------------------
;;
;; Subroutine to initialise the port for I2C
;;
;; RAM Bank:    any (in), SSPADD (out)
;;
;;--------------------------------------------------------------------------------

i2c_ssp_init_port
        BANKSEL SSP_TRIS
        bsf     SSP_TRIS,SSP_SDA; Set SDA tri-state
        bsf     SSP_TRIS,SSP_SCL; Set SCL tri-state

        BANKSEL SSP_PORT
        bcf     SSP_PORT,SSP_SDA; Set SDA low
        bcf     SSP_PORT,SSP_SCL; Set SCL low

        BANKSEL SSPCON
        movlw   B'00110110'     ; Enable I2C data and clock pins
        movwf   SSPCON          ; & select slave mode with 7-bit address

        BANKSEL PIR1
        bcf     PIR1,SSPIF      ; clear interrupt

        IFDEF SSP_I2C_ADDRESS

        BANKSEL SSPADD
        movlw   SSP_I2C_ADDRESS ; Set the slave address
        movwf   SSPADD

        ENDIF

        return


;;--------------------------------------------------------------------------------

;; Re-enable messages and warnings

        ERRORLEVEL      0


;;--------------------------------------------------------------------------------
