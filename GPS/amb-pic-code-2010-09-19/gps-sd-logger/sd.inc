;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/projects/11_gps-sd-logger/RCS/sd.inc,v 1.7 2007/07/08 15:27:04 amb Exp $
;;
;; Include file with SD card functions.
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2007 Andrew M. Bishop
;; It may be distributed under the GNU Public License, version 2, or
;; any higher version.  See section COPYING of the GNU Public license
;; for conditions under which this file may be redistributed.
;;
;;--------------------------------------------------------------------------------

;; 
;; SD functions
;; 

;;--------------------------------------------------------------------------------

SD_power_up

        ;; Turn on power

        BANKSEL SD_PORT          ; Select RAM bank
        bcf     SD_PORT,SD_POWER ; Turn on SD power

        ;; Enable SPI master port

        call    spi_init_master_port

        ;; Set the power flag

        bsf     status,SD_PWR

        ;; Finished
        return


;;--------------------------------------------------------------------------------

SD_power_down

        ;; Disable SPI port

        BANKSEL SSPCON          ; Select RAM bank
        bcf     SSPCON,SSPEN    ; Disable the SSP

        ;; Set all SPI outputs low

        BANKSEL SSP_PORT        ; Select RAM bank
        bcf     SSP_PORT,SSP_SS
        bcf     SSP_PORT,SSP_SDO
        bcf     SSP_PORT,SSP_SCK

        ;; Turn off power

        BANKSEL SD_PORT          ; Select RAM bank
        bsf     SD_PORT,SD_POWER ; Turn off SD power

        ;; Clear the power flag

        bcf     status,SD_PWR

        ;; Finished
        return


;;--------------------------------------------------------------------------------

SD_start

        ;; Initialise the card and read the first block (try 3 times)

        call    SD_init_card_and_read

        btfsc   status,SD_PWR   ; skip if power is off
        goto    SD_started

        call    SD_power_up
        call    SD_init_card_and_read

        btfsc   status,SD_PWR   ; skip if power is off
        goto    SD_started

        call    SD_power_up
        call    SD_init_card_and_read
        goto    SD_started

SD_started

        ;;  Get the address from EEPROM

        call    SD_read_eeprom

        ;; Finished
        return


;;--------------------------------------------------------------------------------

SD_init_card_and_read

        ;; Initialise MMC card

        call    mmc_init

        xorlw   0x00
        btfss   STATUS,Z
        goto    SD_power_down

        ;; Read first block from SD card

        clrf    mmc_addr_0
        clrf    mmc_addr_1
        clrf    mmc_addr_2

        call    mmc_read_start

        xorlw   0x00
        btfss   STATUS,Z
        goto    SD_power_down

        ;; Read block from card

        clrf    counter         ; Count to 256, 2 bytes per loop

SD_read_first_block_loop
        call    spi_dummy_wait_rx_byte

        call    spi_dummy_wait_rx_byte

        decfsz  counter,F
        goto    SD_read_first_block_loop

        ;; End the read

        call    mmc_read_end

        xorlw   0x00
        btfss   STATUS,Z        ; skip if OK
        goto    SD_power_down

        ;; Finished
        return

;;--------------------------------------------------------------------------------

SD_finish

        ;; Write the current address to EEPROM (just in case the following fails)

        call    SD_write_eeprom

        ;; Increase the address and write an empty block to indicate the end of the session

        call    SD_incr_address

        ;; Start the write

        btfss   status,SD_PWR   ; skip if power is on
        return

        call    mmc_write_start

        xorlw   0x00
        btfss   STATUS,Z        ; skip if OK
        goto    SD_power_down

        clrf    counter         ; Count to 256, 2 bytes per loop

SD_finish_loop1
        movlw   0
        call    spi_tx_wait_rx_byte
        movlw   0
        call    spi_tx_wait_rx_byte

        decfsz  counter,F
        goto    SD_finish_loop1

        ;; End the write

        call    mmc_write_end

        xorlw   0x00
        btfss   STATUS,Z        ; skip if OK
        goto    SD_power_down

        ;; Write this address to EEPROM to leave a gap after the last valid data

        call    SD_write_eeprom

        ;; Increase the address and write another empty block to indicate the end of the card, will be overwritten next time

        call    SD_incr_address

        ;; Start the write

        call    mmc_write_start

        xorlw   0x00
        btfss   STATUS,Z        ; skip if OK
        goto    SD_power_down

        clrf    counter         ; Count to 256, 2 bytes per loop

SD_finish_loop2
        movlw   0
        call    spi_tx_wait_rx_byte
        movlw   0
        call    spi_tx_wait_rx_byte

        decfsz  counter,F
        goto    SD_finish_loop2

        ;; End the write

        call    mmc_write_end

        xorlw   0x00
        btfss   STATUS,Z        ; skip if OK
        goto    SD_power_down

        ;; Finished
        return


;;--------------------------------------------------------------------------------

SD_read_eeprom

        ;; Read address from EEPROM; stores the last written address (reprogram PIC => 0xfffffe)

        movlw   sd_eeprom_addr_0
        call    eeprom_read
        movwf   mmc_addr_0

        movlw   sd_eeprom_addr_1
        call    eeprom_read
        movwf   mmc_addr_1

        movlw   sd_eeprom_addr_2
        call    eeprom_read
        movwf   mmc_addr_2

        ;; Mask off LSbit of LSByte of address just in case

        movlw   0xfe
        andwf   mmc_addr_2,F

        ;; Finished
        return


;;--------------------------------------------------------------------------------

SD_write_eeprom

        ;; Write address to EEPROM; store the last written address (reprogram PIC => 0xfffffe)

        BANKSEL EEDATA          ; Select RAM bank

        movf    mmc_addr_0,W
        movwf   EEDATA
        movlw   sd_eeprom_addr_0
        call    eeprom_write

        movf    mmc_addr_1,W
        movwf   EEDATA
        movlw   sd_eeprom_addr_1
        call    eeprom_write

        movf    mmc_addr_2,W
        movwf   EEDATA
        movlw   sd_eeprom_addr_2
        call    eeprom_write

        ;; Finished
        return


;;--------------------------------------------------------------------------------

SD_failure_after_incr

        call    SD_write_eeprom

        call    SD_power_down

        call    SD_power_up

        call    SD_start

        return


;;--------------------------------------------------------------------------------

SD_incr_address

        movlw   2
        addwf   mmc_addr_2,F
        btfss   STATUS,Z        ; skip if rolled over
        return

        incf    mmc_addr_1,F
        btfss   STATUS,Z        ; skip if rolled over
        return

        incf    mmc_addr_0,F
        return

;;--------------------------------------------------------------------------------

SD_decr_address

        movlw   -2
        addwf   mmc_addr_2,F
        btfss   STATUS,C        ; skip if rolled over
        return

        decf    mmc_addr_1,F
        btfss   STATUS,C        ; skip if rolled over
        return

        decf    mmc_addr_0,F
        return

;;--------------------------------------------------------------------------------
