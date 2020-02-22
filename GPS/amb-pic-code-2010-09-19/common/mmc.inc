;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/mmc.inc,v 1.10 2010/09/19 09:53:13 amb Exp $
;;
;; Functions for MMC/SD card access.
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
;; Required Variables / Constants for MMC functions
;;
;; Constants:   
;;
;; Variables:   mmc_temp        - Stores temporary data, same bank as SSPBUF
;;              mmc_addr0       - MSByte of 4 byte address, same bank as SSPBUF
;;              mmc_addr1       - Byte of 4 byte address, same bank as SSPBUF
;;              mmc_addr2       - Byte of 4 byte address, same bank as SSPBUF
;;                              - Note LSByte is always zero
;;              spi_temp        - Stores temporary data, same bank as SSPBUF
;;                                (re-uses the spi_temp variable from SPI functions).
;;
;;--------------------------------------------------------------------------------


;;--------------------------------------------------------------------------------
;;
;; Subroutine to initialise an SD/MMC card
;;
;; Parameters:  w (out)         - zero if OK, non-zero for error
;;
;; RAM Bank:    any (in) SSPBUF (out)
;;
;;--------------------------------------------------------------------------------

mmc_init
        call    spi_disable_chip_select

        ;; Send some clock cycles to wake up the card

        call    spi_dummy_wait_rx_byte
        call    spi_dummy_wait_rx_byte
        call    spi_dummy_wait_rx_byte
        call    spi_dummy_wait_rx_byte
        call    spi_dummy_wait_rx_byte
        call    spi_dummy_wait_rx_byte
        call    spi_dummy_wait_rx_byte
        call    spi_dummy_wait_rx_byte
        call    spi_dummy_wait_rx_byte
        call    spi_dummy_wait_rx_byte
        call    spi_dummy_wait_rx_byte
        call    spi_dummy_wait_rx_byte

        ;; Select the card

        call    spi_enable_chip_select

        ;; Send command 0, exit in case of error

        call    mmc_cmd0

        xorlw   0x01
        btfss   STATUS,Z
        goto    mmc_cmd_error

        ;; Send command 1, exit in case of error

        call    mmc_cmd1

        xorlw   0x00
        btfss   STATUS,Z
        goto    mmc_cmd_error

        ;; De-select the card and return with no error

        call    spi_disable_chip_select
        retlw   0


;;--------------------------------------------------------------------------------
;;
;; Subroutine to wait for an R1 single byte response
;;
;; Parameters:  w (out)         - value of R1 response if OK, 0xff for timeout
;;
;; RAM Bank:    any (in) SSPBUF (out)
;;
;;--------------------------------------------------------------------------------

mmc_wait_r1
        BANKSEL SSPBUF          ; Select RAM bank
        clrf    spi_temp        ; not used by spi_dummy_wait_rx_byte function

mmc_wait_r1_loop
        call    spi_dummy_wait_rx_byte

        addlw   0x80
        addlw   0x80
        btfsc   STATUS,C        ; Return if MSB is zero
        return

        decfsz  spi_temp,F
        goto    mmc_wait_r1_loop

        retlw   0xff


;;--------------------------------------------------------------------------------
;;
;; Subroutine to wait until the card is not busy
;;
;; Parameters:  w (out)         - zero if OK, non-zero for error
;;
;; RAM Bank:    any (in) SSPBUF (out)
;;
;;--------------------------------------------------------------------------------

mmc_wait_not_busy
        BANKSEL SSPBUF          ; Select RAM bank
        clrf    spi_temp        ; not used by spi_dummy_wait_rx_byte function

mmc_wait_not_busy_loop
        call    spi_dummy_wait_rx_byte

        xorlw   0x00
        btfss   STATUS,Z        ; Return if value is non-zero
        retlw   0

        movlw   1
        call    delay_ms

        decfsz  spi_temp,F
        goto    mmc_wait_not_busy_loop

        retlw   0xff


;;--------------------------------------------------------------------------------
;;
;; Subroutine to send command 0 to an SD/MMC card
;;
;; Parameters:  w (out)         - Value of R1 response or 0xff in case of timeout
;;
;; RAM Bank:    any (in) SSPBUF (out)
;;
;;--------------------------------------------------------------------------------

mmc_cmd0
        ;; Send CMD0 to go to MMC/SPI mode

        call    spi_dummy_wait_rx_byte
        movlw   0x40
        call    spi_tx_wait_rx_byte
        movlw   0x00
        call    spi_tx_wait_rx_byte
        movlw   0x00
        call    spi_tx_wait_rx_byte
        movlw   0x00
        call    spi_tx_wait_rx_byte
        movlw   0x00
        call    spi_tx_wait_rx_byte
        movlw   0x95
        call    spi_tx_wait_rx_byte

        ;; Wait for an R1 response and return it

        call    mmc_wait_r1

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine to send command 1 to an SD/MMC card
;;
;; Parameters:  w (out)         - Value of R1 response or 0xff in case of timeout
;;
;; RAM Bank:    any (in) SSPBUF (out)
;;
;;--------------------------------------------------------------------------------

mmc_cmd1
        BANKSEL SSPBUF          ; Select RAM bank
        clrf    mmc_temp

        ;; Send CMD1 to start initialisation

mmc_cmd1_loop
        call    spi_dummy_wait_rx_byte
        movlw   0x41
        call    spi_tx_wait_rx_byte
        movlw   0x00
        call    spi_tx_wait_rx_byte
        movlw   0x00
        call    spi_tx_wait_rx_byte
        movlw   0x00
        call    spi_tx_wait_rx_byte
        movlw   0x00
        call    spi_tx_wait_rx_byte
        movlw   0xff
        call    spi_tx_wait_rx_byte

        ;; Check for an R1 response value of 0x00

        call    mmc_wait_r1

        xorlw   0x00
        btfsc   STATUS,Z
        return

        ;; Send command again perhaps

        movlw   1
        call    delay_ms

        decfsz  mmc_temp,F
        goto    mmc_cmd1_loop

        ;; Return with error

        retlw   0xff


;;--------------------------------------------------------------------------------
;;
;; Subroutine to send command 8 to an SD/MMC card
;;
;; Parameters:  w (out)         - zero if OK, non-zero for error
;;
;; RAM Bank:    any (in) SSPBUF (out)
;;
;;--------------------------------------------------------------------------------

mmc_cmd8
        ;; Select the card

        call    spi_enable_chip_select

        ;; Send CMD8 to probe for card type

        call    spi_dummy_wait_rx_byte
        movlw   0x48
        call    spi_tx_wait_rx_byte
        movlw   0x00
        call    spi_tx_wait_rx_byte
        movlw   0x00
        call    spi_tx_wait_rx_byte
        movlw   0x00
        call    spi_tx_wait_rx_byte
        movlw   0x00
        call    spi_tx_wait_rx_byte
        movlw   0xff
        call    spi_tx_wait_rx_byte

        ;; Check for an R1 response value of 0x01

        call    mmc_wait_r1

        xorlw   0x01
        btfsc   STATUS,Z
        goto    mmc_cmd_error

        ;; Return with no error

        retlw   0x00


;;--------------------------------------------------------------------------------
;;
;; Subroutine to send block read command to an SD/MMC card
;;
;; Parameters:  w (out)         - zero if OK, non-zero for error
;;
;; RAM Bank:    any (in) SSPBUF (out)
;;
;;--------------------------------------------------------------------------------

mmc_read_start
mmc_cmd17_start
        ;; Select the card

        call    spi_enable_chip_select

        ;; Send CMD17 to start block read

        call    spi_dummy_wait_rx_byte
        movlw   0x51
        call    spi_tx_wait_rx_byte
        movf    mmc_addr_0,W
        call    spi_tx_wait_rx_byte
        movf    mmc_addr_1,W
        call    spi_tx_wait_rx_byte
        movf    mmc_addr_2,W
        call    spi_tx_wait_rx_byte
        movlw   0x00
        call    spi_tx_wait_rx_byte
        movlw   0xff
        call    spi_tx_wait_rx_byte

        ;; Wait for an R1 response value of 0x00

        call    mmc_wait_r1

        xorlw   0x00
        btfss   STATUS,Z
        goto    mmc_cmd_error

        ;; Wait for a start transmission token of value 0xfe

        clrf    mmc_temp

mmc_cmd17_loop
        call    spi_dummy_wait_rx_byte

        xorlw   0xfe
        btfsc   STATUS,Z
        retlw   0

        decfsz  mmc_temp,F
        goto    mmc_cmd17_loop

        goto    mmc_cmd_error


;;--------------------------------------------------------------------------------
;;
;; Subroutine to send bytes to get status response at end of read command to an SD/MMC card
;;
;; Parameters:  w (out)         - zero if OK, non-zero for error
;;
;; RAM Bank:    any (in) SSPBUF (out)
;;
;;--------------------------------------------------------------------------------

mmc_read_end
mmc_cmd17_end
        ;; Send some dummy bytes to get the CRC

        call    spi_dummy_wait_rx_byte
        call    spi_dummy_wait_rx_byte

        ;; De-select the card and return with no error

        call    spi_disable_chip_select
        retlw   0


;;--------------------------------------------------------------------------------
;;
;; Subroutine to send block write command to an SD/MMC card
;;
;; Parameters:  w (out)         - zero if OK, non-zero for error
;;
;; RAM Bank:    any (in) SSPBUF (out)
;;
;;--------------------------------------------------------------------------------

mmc_write_start
mmc_cmd24_start
        ;; Select the card

        call    spi_enable_chip_select

        ;; Send CMD24 to start block write

        call    spi_dummy_wait_rx_byte
        movlw   0x58
        call    spi_tx_wait_rx_byte
        movf    mmc_addr_0,W
        call    spi_tx_wait_rx_byte
        movf    mmc_addr_1,W
        call    spi_tx_wait_rx_byte
        movf    mmc_addr_2,W
        call    spi_tx_wait_rx_byte
        movlw   0x00
        call    spi_tx_wait_rx_byte
        movlw   0xff
        call    spi_tx_wait_rx_byte

        ;; Wait for an R1 response value of 0x00

        call    mmc_wait_r1

        xorlw   0x00
        btfss   STATUS,Z
        goto    mmc_cmd_error

        ;; Wait a few cycles then send start of transfer marker

        call    spi_dummy_wait_rx_byte
        call    spi_dummy_wait_rx_byte

        movlw   0xfe
        call    spi_tx_wait_rx_byte

        ;; Return with no error

        retlw   0


;;--------------------------------------------------------------------------------
;;
;; Subroutine to send bytes to get status response and wait for end of busy signal at end of write command to an SD/MMC card
;;
;; Parameters:  w (out)         - zero if OK, non-zero for error
;;
;; RAM Bank:    any (in) SSPBUF (out)
;;
;;--------------------------------------------------------------------------------

mmc_write_end
mmc_cmd24_end

        ;; Send some dummy bytes for CRC

        call    spi_dummy_wait_rx_byte
        call    spi_dummy_wait_rx_byte

        ;; Wait for a response value of 0x05 (with mask of 0x1f)

        clrf    mmc_temp

mmc_cmd24_loop
        call    spi_dummy_wait_rx_byte

        xorlw   0xe5
        btfsc   STATUS,Z
        goto    mmc_cmd24_wait_not_busy

        movlw   10              ; This can take a long time >256 ms.
        call    delay_ms

        decfsz  mmc_temp,F
        goto    mmc_cmd24_loop

        goto    mmc_cmd_error

        ;; Wait for end of busy signal

mmc_cmd24_wait_not_busy
        call    mmc_wait_not_busy

        ;; De-select the card and return with no error

        call    spi_disable_chip_select
        retlw   0


;;--------------------------------------------------------------------------------
;;
;; Subroutine to send command 55 to an SD/MMC card
;;
;; Parameters:
;;
;; RAM Bank:    any (in) SSPBUF (out)
;;
;;--------------------------------------------------------------------------------

mmc_cmd55
        ;; Send CMD55 to enable an application command

        call    spi_dummy_wait_rx_byte
        movlw   0x77
        call    spi_tx_wait_rx_byte
        movlw   0x00
        call    spi_tx_wait_rx_byte
        movlw   0x00
        call    spi_tx_wait_rx_byte
        movlw   0x00
        call    spi_tx_wait_rx_byte
        movlw   0x00
        call    spi_tx_wait_rx_byte
        movlw   0xff
        call    spi_tx_wait_rx_byte

        ;; Wait for an R1 response

        call    mmc_wait_r1

        ;; Return

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine to send application command 41 to an SD/MMC card
;;
;; Parameters:  w (out)         - Value of R1 response or 0xff in case of timeout
;;
;; RAM Bank:    any (in) SSPBUF (out)
;;
;;--------------------------------------------------------------------------------

mmc_acmd41
        BANKSEL SSPBUF          ; Select RAM bank
        clrf    mmc_temp

mmc_acmd41_loop

        ;; Send CMD55 to allow an application command

        call    mmc_cmd55

        ;; Send ACMD41 to start initialisation

        call    spi_dummy_wait_rx_byte
        movlw   0x69
        call    spi_tx_wait_rx_byte
        movlw   0x00
        call    spi_tx_wait_rx_byte
        movlw   0x00
        call    spi_tx_wait_rx_byte
        movlw   0x00
        call    spi_tx_wait_rx_byte
        movlw   0x00
        call    spi_tx_wait_rx_byte
        movlw   0xff
        call    spi_tx_wait_rx_byte

        ;; Check for an R1 response value of 0x00

        call    mmc_wait_r1

        xorlw   0x00
        btfsc   STATUS,Z
        return

        ;; Send command again perhaps

        movlw   1
        call    delay_ms

        decfsz  mmc_temp,F
        goto    mmc_acmd41_loop

        ;; Return with error

        retlw   0xff


;;--------------------------------------------------------------------------------
;;
;; Subroutine to exit a command in case of an error after deselecting card
;;
;; Parameters:  w (out)         - zero if OK, non-zero for error
;;
;; RAM Bank:    any (in) SSPBUF (out)
;;
;;--------------------------------------------------------------------------------

mmc_cmd_error
        ;; De-select the card and return with error

        call    spi_disable_chip_select
        retlw   1


;;--------------------------------------------------------------------------------
