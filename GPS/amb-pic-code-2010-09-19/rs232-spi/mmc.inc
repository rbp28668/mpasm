;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/projects/07_rs232-to-spi/RCS/mmc.inc,v 1.5 2007/05/07 18:33:30 amb Exp $
;;
;; Include file with MMC/SD specific RS232 <-> SPI functions.
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
;; MMC SPI commands.
;; * Initialise a card
;; * Read contents of a block to RS232
;; * Write contents of a block from RS232
;; 

;; Initialise the MMC card

spi_mmc_init

        call    mmc_init

        xorlw   0x00
        btfss   STATUS,Z
        goto    spi_mmc_error
        goto    spi_mmc_ok


;; Read the MMC data to RS232

spi_mmc_read

        call    mmc_get_addr

        ;; Start the read

        call    mmc_read_start

        xorlw   0x00
        btfss   STATUS,Z
        goto    spi_mmc_error

        ;; Stream data from RS232 to card

        clrf    counter         ; Count to 256, 2 bytes per loop

read_loop
        call    spi_dummy_wait_rx_byte

        call    rs232_tx_byte

        call    spi_dummy_wait_rx_byte

        call    rs232_tx_byte

        decfsz  counter,F
        goto    read_loop

        ;; End the read

        call    mmc_read_end

        xorlw   0x00
        btfss   STATUS,Z
        goto    spi_mmc_error
        goto    spi_mmc_ok


;; Write the MMC data from RS232

spi_mmc_write

        call    mmc_get_addr

        ;; Start the write

        call    mmc_write_start

        xorlw   0x00
        btfss   STATUS,Z
        goto    spi_mmc_error

        ;; Stream data from RS232 to card

        clrf    counter         ; Count to 256, 2 bytes per loop

write_loop
        call    rs232_rx_byte

        call    spi_tx_wait_rx_byte

        call    rs232_rx_byte

        call    spi_tx_wait_rx_byte

        decfsz  counter,F
        goto    write_loop

        ;; End the write

        call    mmc_write_end

        xorlw   0x00
        btfss   STATUS,Z
        goto    spi_mmc_error
        goto    spi_mmc_ok


;; Get the MMC block address from RS232

mmc_get_addr

        ;; Read three bytes from RS232

        call    rs232_rx_byte
        movwf   mmc_addr_0

        call    rs232_rx_byte
        movwf   mmc_addr_1

        call    rs232_rx_byte
        movwf   mmc_addr_2

        ;; Multiply by 2 to convert block addresses to byte address (MSB is always zero)

        bcf     STATUS,C
        rlf     mmc_addr_2,F
        rlf     mmc_addr_1,F
        rlf     mmc_addr_0,F

        return


;; Print the OK message at the end.

spi_mmc_ok

        movlw   'O'
        call    rs232_tx_byte

        movlw   'K'
        call    rs232_tx_byte

        movlw   10
        call    rs232_tx_byte

        return


;; Print the error message at the end.

spi_mmc_error

        movlw   'E'
        call    rs232_tx_byte

        movlw   '9'
        call    rs232_tx_byte

        movlw   10
        call    rs232_tx_byte

        return
