;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/i2c_ssp_slave.inc,v 1.2 2010/09/19 09:43:38 amb Exp $
;;
;; I2C slave functions using SSP module.
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
;;
;; Variables:   i2c_temp        - Stores temporary data, same bank(s) as I2C_PORT and I2C_TRIS
;;
;;--------------------------------------------------------------------------------


;;--------------------------------------------------------------------------------
;;
;; Subroutine for a slave receiving the address byte (matching address byte with ACK)
;;
;; Parameters:  w (out)         - Address received (LSB=1 for read, 0 for write)
;;
;; RAM Bank:    any (in) SSP_TRIS (out)
;;
;;--------------------------------------------------------------------------------

i2c_ssp_slave_rx_address
        BANKSEL SSPCON
        bcf     SSPCON,SSPOV    ; Clear overflow flag

        BANKSEL PIR1
        btfss   PIR1,SSPIF      ; skip if interrupt flag set
        goto    $-1
        bcf     PIR1,SSPIF      ; clear interrupt

        ;; Check for the right combination of flags

        BANKSEL SSPSTAT

        btfss   SSPSTAT,S       ; skip if start bit was last
        goto    i2c_ssp_slave_rx_address

        btfsc   SSPSTAT,D_A     ; skip if address byte
        goto    i2c_ssp_slave_rx_address

        ;; Correct type of interrupt (S=1, D_A=0)

        BANKSEL SSPBUF
        movf    SSPBUF,W        ; resets BF flag

        BANKSEL SSP_TRIS
        movwf   i2c_temp

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine for a slave receiving the non-last I2C serial byte (with ACK)
;;
;; Parameters:  w (out)         - Data byte received
;;
;; RAM Bank:    any (in) SSP_TRIS (out)
;;
;;--------------------------------------------------------------------------------

i2c_ssp_slave_rx_byte
        BANKSEL SSPCON
        bcf     SSPCON,SSPOV    ; Clear overflow flag

        BANKSEL PIR1
        btfss   PIR1,SSPIF      ; skip if interrupt flag set
        goto    $-1
        bcf     PIR1,SSPIF      ; clear interrupt

        BANKSEL SSPBUF
        movf    SSPBUF,W        ; resets BF flag

        BANKSEL SSP_TRIS
        movwf   i2c_temp

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine for a slave receiving the last I2C serial byte (with NACK)
;;
;; Parameters:  w (out)         - Data byte received
;;
;; RAM Bank:    any (in) SSP_TRIS (out)
;;
;;--------------------------------------------------------------------------------

i2c_ssp_slave_rx_last_byte
        BANKSEL SSPCON
        bsf     SSPCON,SSPOV    ; Set overflow flag

        BANKSEL PIR1
        btfss   PIR1,SSPIF      ; skip if interrupt flag set
        goto    $-3
        bcf     PIR1,SSPIF      ; clear interrupt

        BANKSEL SSPBUF
        movf    SSPBUF,W        ; resets BF flag

        BANKSEL SSPCON
        bcf     SSPCON,SSPOV    ; Clear overflow flag

        BANKSEL SSP_TRIS
        movwf   i2c_temp

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine for a slave transmitting an I2C serial byte
;;
;; Parameters:  w (in)          - Data byte to transmit
;;              w (out)         - Zero for ACK, 1 for NACK
;;
;; RAM Bank:    any (in) SSP_TRIS (out)
;;
;;--------------------------------------------------------------------------------

i2c_ssp_slave_tx_byte
        BANKSEL SSPBUF
        movwf   SSPBUF          ; Load the byte to send

        BANKSEL SSPCON
        bsf     SSPCON,CKP      ; Set the bit to start transmission

        BANKSEL PIR1
        btfss   PIR1,SSPIF      ; skip if interrupt flag set
        goto    $-1
        bcf     PIR1,SSPIF      ; clear interrupt

        BANKSEL SSPSTAT
        btfsc   SSPSTAT,R_W     ; skip if R_W is clear
        retlw   0               ; ACK
        retlw   1               ; NACK


;;--------------------------------------------------------------------------------
