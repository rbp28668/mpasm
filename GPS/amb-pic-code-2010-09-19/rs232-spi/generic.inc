;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/projects/07_rs232-to-spi/RCS/generic.inc,v 1.5 2007/05/07 18:33:30 amb Exp $
;;
;; Include file with generic RS232 <-> SPI functions.
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2006 Andrew M. Bishop
;; It may be distributed under the GNU Public License, version 2, or
;; any higher version.  See section COPYING of the GNU Public license
;; for conditions under which this file may be redistributed.
;;
;;--------------------------------------------------------------------------------

;; 
;; Generic SPI command.
;; * Read command from RS232
;; * Act on command to:
;;      Enable chip select
;;      Disable chip select
;;      Write some bytes from RS232 to SPI port (no reply to RS232)
;;      Read some bytes from SPI port to RS232 (dummy write of 0xff)
;;      Xchange some bytes from RS2323 to SPI port and SPI port to RS232
;;      Stop and return control
;; 

STATE_STOP      EQU  'S'
STATE_ENABLE    EQU  'E'
STATE_DISABLE   EQU  'D'
STATE_WRITE     EQU  'W'
STATE_READ      EQU  'R'
STATE_XCHANGE   EQU  'X'

generic_spi

        ;; Wait for command on RS232

        call    rs232_rx_byte
        movwf   state

        ;; Decide which function to call

        movlw   STATE_STOP
        xorwf   state,W
        btfss   STATUS,Z
        goto    $+2
        goto    generic_stop

        movlw   STATE_ENABLE
        xorwf   state,W
        btfss   STATUS,Z
        goto    $+2
        goto    generic_enable

        movlw   STATE_DISABLE
        xorwf   state,W
        btfss   STATUS,Z
        goto    $+2
        goto    generic_disable

        movlw   STATE_WRITE
        xorwf   state,W
        btfss   STATUS,Z
        goto    $+2
        goto    generic_write

        movlw   STATE_READ
        xorwf   state,W
        btfss   STATUS,Z
        goto    $+2
        goto    generic_read

        movlw   STATE_XCHANGE
        xorwf   state,W
        btfss   STATUS,Z
        goto    $+2
        goto    generic_xchange

        ;; Anything else is ignored

        goto    generic_spi


        ;; Command to stop

generic_stop

        return


        ;; Command to enable chip select

generic_enable

        call    spi_enable_chip_select

        goto    generic_spi


        ;; Command to disable chip select

generic_disable

        call    spi_disable_chip_select

        goto    generic_spi


        ;; Command to write bytes

generic_write

        ;; Get number of bytes to write

        call    rs232_rx_byte
        movwf   counter

        ;; Get the bytes from RS232 and write to SPI

generic_write_loop
        call    rs232_rx_byte

        call    spi_tx_wait_discard_byte

        decfsz  counter,F
        goto    generic_write_loop

        goto    generic_spi


        ;; Command to read bytes

generic_read

        ;; Get number of bytes to read

        call    rs232_rx_byte
        movwf   counter

        ;; Send dummy bytes and read from SPI

generic_read_loop
        call    spi_dummy_wait_rx_byte

        call    rs232_tx_byte

        decfsz  counter,F
        goto    generic_read_loop

        goto    generic_spi


        ;; Command to xchange bytes

generic_xchange

        ;; Get number of bytes to xchange

        call    rs232_rx_byte
        movwf   counter

        ;; Get the bytes from RS232, write to SPI, read from SPI and write to RS232

generic_xchange_loop
        call    rs232_rx_byte

        call    spi_tx_wait_rx_byte

        call    rs232_tx_byte

        decfsz  counter,F
        goto    generic_xchange_loop

        goto    generic_spi
