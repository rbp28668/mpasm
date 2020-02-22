;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/ds1307.inc,v 1.1 2007/08/04 18:29:59 amb Exp $
;;
;; Functions for DS1307 real time clock IC access.
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
;; Required Variables / Constants for DS1307 functions
;;
;; Constants:   
;;
;; Variables:   ds1307_second   - Stores time in seconds read from device
;;              ds1307_minute   - Stores time in minutes read from device
;;              ds1307_hour     - Stores time in minutes read from device
;;              ds1307_day      - Stores time in minutes read from device
;;              ds1307_date     - Stores time in minutes read from device
;;              ds1307_month    - Stores time in minutes read from device
;;              ds1307_year     - Stores time in minutes read from device
;;
;;--------------------------------------------------------------------------------

DS1307_W_ADDR   EQU     0xD0
DS1307_R_ADDR   EQU     0xD1


;;--------------------------------------------------------------------------------
;;
;; Subroutine to start the clock of the DS1307 device (if halted)
;;
;; Parameters:  w (out)         - zero if OK, non-zero for error
;;
;; RAM Bank:    any (in) I2C_TRIS (out)
;;
;;--------------------------------------------------------------------------------

ds1307_start_clock
        call    i2c_master_tx_start ; I2C Start

        movlw   DS1307_W_ADDR
        call    i2c_master_tx_byte ; DS1307 write address

        movlw   0
        call    i2c_master_tx_byte ; First register

        call    i2c_master_tx_restart ; I2C restart

        movlw   DS1307_R_ADDR
        call    i2c_master_tx_byte ; DS1307 read address

        call    i2c_master_rx_last_byte ; receive data byte (seconds)
        movwf   ds1307_second

        call    i2c_master_tx_stop ; I2C Stop

        btfss   ds1307_second,7 ; Skip if clock is halted
        return                  ; return if clock is already running

        bcf     ds1307_second,7

        call    i2c_master_tx_start ; I2C Start

        movlw   DS1307_W_ADDR
        call    i2c_master_tx_byte ; DS1307 write address

        movlw   0
        call    i2c_master_tx_byte ; Seconds register (0)

        movf    ds1307_second,W
        call    i2c_master_tx_byte ; Remove halt bit

        call    i2c_master_tx_stop ; I2C Stop

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine to stop the clock of the DS1307 device (if running)
;;
;; Parameters:  w (out)         - zero if OK, non-zero for error
;;
;; RAM Bank:    any (in) I2C_TRIS (out)
;;
;;--------------------------------------------------------------------------------

ds1307_stop_clock
        call    i2c_master_tx_start ; I2C Start

        movlw   DS1307_W_ADDR
        call    i2c_master_tx_byte ; DS1307 write address

        movlw   0
        call    i2c_master_tx_byte ; First register

        call    i2c_master_tx_restart ; I2C restart

        movlw   DS1307_R_ADDR
        call    i2c_master_tx_byte ; DS1307 read address

        call    i2c_master_rx_last_byte ; receive data byte (seconds)
        movwf   ds1307_second

        call    i2c_master_tx_stop ; I2C Stop

        btfsc   ds1307_second,7 ; Skip if clock is running
        return                  ; return if clock is already halted

        bsf     ds1307_second,7

        call    i2c_master_tx_start ; I2C Start

        movlw   DS1307_W_ADDR
        call    i2c_master_tx_byte ; DS1307 write address

        movlw   0
        call    i2c_master_tx_byte ; Seconds register (0)

        movf    ds1307_second,W
        call    i2c_master_tx_byte ; Remove halt bit

        call    i2c_master_tx_stop ; I2C Stop

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine to enable the PPS output of the DS1307 device
;;
;; Parameters:  w (out)         - zero if OK, non-zero for error
;;
;; RAM Bank:    any (in) I2C_TRIS (out)
;;
;;--------------------------------------------------------------------------------

ds1307_enable_pps
        call    i2c_master_tx_start ; I2C Start

        movlw   DS1307_W_ADDR
        call    i2c_master_tx_byte ; DS1307 write address

        movlw   7
        call    i2c_master_tx_byte ; Control register (7)

        movlw   0x10
        call    i2c_master_tx_byte ; Enable output, set 1 Hz

        call    i2c_master_tx_stop ; I2C Stop

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine to disable the PPS output of the DS1307 device
;;
;; Parameters:  w (out)         - zero if OK, non-zero for error
;;
;; RAM Bank:    any (in) I2C_TRIS (out)
;;
;;--------------------------------------------------------------------------------

ds1307_disable_pps
        call    i2c_master_tx_start ; I2C Start

        movlw   DS1307_W_ADDR
        call    i2c_master_tx_byte ; DS1307 write address

        movlw   7
        call    i2c_master_tx_byte ; Control register (7)

        movlw   0x00
        call    i2c_master_tx_byte ; Disable output

        call    i2c_master_tx_stop ; I2C Stop

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine to read all of the time information from the DS1307 device
;;
;; Parameters:  w (out)         - zero if OK, non-zero for error
;;
;; RAM Bank:    any (in) SSPBUF (out)
;;
;;--------------------------------------------------------------------------------

ds1307_read
        call    i2c_master_tx_start ; I2C Start

        movlw   DS1307_W_ADDR
        call    i2c_master_tx_byte ; DS1307 write address

        movlw   0
        call    i2c_master_tx_byte ; First register

        call    i2c_master_tx_restart ; I2C restart

        movlw   DS1307_R_ADDR
        call    i2c_master_tx_byte ; DS1307 read address

        call    i2c_master_rx_byte ; receive data byte (seconds)
        movwf   ds1307_second

        call    i2c_master_rx_byte ; receive data byte (minutes)
        movwf   ds1307_minute

        call    i2c_master_rx_byte ; receive data byte (hours)
        movwf   ds1307_hour

        call    i2c_master_rx_byte ; receive data byte (day)
        movwf   ds1307_day

        call    i2c_master_rx_byte ; receive data byte (date)
        movwf   ds1307_date

        call    i2c_master_rx_byte ; receive data byte (month)
        movwf   ds1307_month

        call    i2c_master_rx_last_byte ; receive last data byte (year), send NACK
        movwf   ds1307_year

        call    i2c_master_tx_stop ; I2C Stop

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine to write all of the time information to the DS1307 device
;;
;; Parameters:  w (out)         - zero if OK, non-zero for error
;;
;; RAM Bank:    any (in) SSPBUF (out)
;;
;;--------------------------------------------------------------------------------

ds1307_write
        call    i2c_master_tx_start ; I2C Start

        movlw   DS1307_W_ADDR
        call    i2c_master_tx_byte ; DS1307 write address

        movlw   0
        call    i2c_master_tx_byte ; First register

        movf    ds1307_second,W
        call    i2c_master_tx_byte ; transmit data byte (seconds)

        movf    ds1307_minute,W
        call    i2c_master_tx_byte ; transmit data byte (minutes)

        movf    ds1307_hour,W
        call    i2c_master_tx_byte ; transmit data byte (hours)

        movf    ds1307_day,W
        call    i2c_master_tx_byte ; transmit data byte (day)

        movf    ds1307_date,W
        call    i2c_master_tx_byte ; transmit data byte (date)

        movf    ds1307_month,W
        call    i2c_master_tx_byte ; transmit data byte (month)

        movf    ds1307_year,W
        call    i2c_master_tx_byte ; transmit data byte (year)

        call    i2c_master_tx_stop ; I2C Stop

        return


;;--------------------------------------------------------------------------------
