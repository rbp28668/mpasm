;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/i2c_ssp.inc,v 1.3 2010/09/19 09:43:38 amb Exp $
;;
;; Main include file for I2C functions using SSP module.
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2006,07,10 Andrew M. Bishop
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
;;              SSP_PORT        - Selects the port containing the SSP port (automatically set)
;;              SSP_TRIS        - Selects the tristate register for the SSP port (automatically set)
;;              SSP_SDA         - The bit to use for SDA data in/out (automatically set)
;;              SSP_SCL         - The bit to use for SCL clock in/out (automatically set)
;;              SSP_I2C_ADDRESS - The address to use for the slave address
;;
;; Variables:   i2c_temp        - Stores temporary data, same bank(s) as I2C_PORT and I2C_TRIS
;;
;;--------------------------------------------------------------------------------


;;
;; SSP Port hardware configuration
;;

        include "../common/ssp_hw.inc"

;;
;; I2C Basic functions (using SSP)
;;

        include "../common/i2c_ssp_basic.inc"

;;
;; I2C Master functions (using SSP)
;;

        include "../common/i2c_ssp_master.inc"

;;
;; I2C Slave functions (using SSP)
;;

        include "../common/i2c_ssp_slave.inc"

;;--------------------------------------------------------------------------------
