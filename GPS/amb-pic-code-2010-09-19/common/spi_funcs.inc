;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/spi_funcs.inc,v 1.5 2007/05/07 18:31:41 amb Exp $
;;
;; Functions to transmit and receive SPI bytes.
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
;; Required Variables / Constants for SPI functions
;;
;; Constants:   SPI_SDI_INV     - Specifies if the SDI port is inverted
;;
;; Variables:   spi_temp        - Stores temporary data, same bank as SSPBUF
;;
;;--------------------------------------------------------------------------------

;;--------------------------------------------------------------------------------

;; Temporarily disable messages and warnings

        ERRORLEVEL      2


;;--------------------------------------------------------------------------------
;;
;; Subroutine to transmit a byte only (does not wait or read SSPBUF)
;;
;; Parameters:  w (in)          - Byte to transmit
;;
;; RAM Bank:    any (in) SSPBUF (out)
;;
;;--------------------------------------------------------------------------------

spi_tx_byte
        BANKSEL SSPBUF          ; Select RAM bank

        movwf   SSPBUF          ; Put the transmit byte

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine to wait for a byte to be transmitted / received (does not read or write SSPBUF)
;;
;; RAM Bank:    any (in) SSPBUF (out)
;;
;;--------------------------------------------------------------------------------

spi_wait_byte
        BANKSEL SSPSTAT         ; Select RAM bank

        btfss   SSPSTAT,BF      ; Check if buffer is full
        goto    $-1             ; Try again until it empties

        BANKSEL SSPBUF          ; Select RAM bank

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine to receive a byte (does not write SSPBUF or wait)
;;
;; Parameters:  w (out)         - Byte received
;;
;; RAM Bank:    any (in) SSPBUF (out)
;;
;;--------------------------------------------------------------------------------

spi_rx_byte
        BANKSEL SSPBUF          ; Select RAM bank

        IFDEF   SPI_SDI_INV
        comf    SSPBUF,W        ; Get the received byte (and invert it)
        ELSE
        movf    SSPBUF,W        ; Get the received byte
        ENDIF

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine to transmit a byte and wait for it to go (does not read SSPBUF)
;;
;; Parameters:  w (in)          - Byte to transmit
;;              w (out)         - Byte to transmit (unchanged)
;;
;; RAM Bank:    any (in) SSPBUF (out)
;;
;;--------------------------------------------------------------------------------

spi_tx_wait_byte
        BANKSEL SSPBUF          ; Select RAM bank

        movwf   SSPBUF          ; Put the transmit byte

        BANKSEL SSPSTAT         ; Select RAM bank

        btfss   SSPSTAT,BF      ; Check if buffer is full
        goto    $-1             ; Try again until it empties

        BANKSEL SSPBUF          ; Select RAM bank

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine to wait for a byte to be received
;;
;; RAM Bank:    any (in) SSPBUF (out)
;;
;;--------------------------------------------------------------------------------

spi_wait_rx_byte
        BANKSEL SSPSTAT         ; Select RAM bank

        btfss   SSPSTAT,BF      ; Check if buffer is full
        goto    $-1             ; Try again until it empties

        BANKSEL SSPBUF          ; Select RAM bank

        IFDEF   SPI_SDI_INV
        comf    SSPBUF,W        ; Get the received byte (and invert it)
        ELSE
        movf    SSPBUF,W        ; Get the received byte
        ENDIF

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine to receive the previous byte and transmit the next (does not wait)
;;
;; Parameters:  w (in)          - Byte to transmit
;;              w (out)         - Byte received
;;
;; RAM Bank:    any (in) SSPBUF (out)
;;
;;--------------------------------------------------------------------------------

spi_rx_tx_byte
        BANKSEL SSPBUF          ; Select RAM bank

        movwf   spi_temp        ; Store the data to be transmitted

        IFDEF   SPI_SDI_INV
        comf    SSPBUF,W        ; Get the received byte (and invert it)
        ELSE
        movf    SSPBUF,W        ; Get the received byte
        ENDIF

        xorwf   spi_temp,F      ; \
        xorwf   spi_temp,W      ;  | Swap W and spi_temp using 3 XOR trick
        xorwf   spi_temp,F      ; /

        movwf   SSPBUF          ; Put the transmit byte

        movf    spi_temp,W      ; Retrieve the received byte

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine to transmit a byte, wait for it to go and receive the result
;;
;; Parameters:  w (in)          - Byte to transmit
;;              w (out)         - Byte received
;;
;; RAM Bank:    any (in) SSPBUF (out)
;;
;;--------------------------------------------------------------------------------

spi_tx_wait_rx_byte
        BANKSEL SSPBUF          ; Select RAM bank

        movwf   SSPBUF          ; Put the transmit byte

        BANKSEL SSPSTAT         ; Select RAM bank

        btfss   SSPSTAT,BF      ; Check if buffer is full
        goto    $-1             ; Try again until it empties

        BANKSEL SSPBUF          ; Select RAM bank

        IFDEF   SPI_SDI_INV
        comf    SSPBUF,W        ; Get the received byte (and invert it)
        ELSE
        movf    SSPBUF,W        ; Get the received byte
        ENDIF

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine to transmit a byte, wait for it to go and receive the result
;;
;; Parameters:  w (out)         - Byte received
;;
;; RAM Bank:    any (in) SSPBUF (out)
;;
;;--------------------------------------------------------------------------------

spi_dummy_wait_rx_byte
        BANKSEL SSPBUF          ; Select RAM bank

        movlw   0xff
        movwf   SSPBUF          ; Put the transmit byte

        BANKSEL SSPSTAT         ; Select RAM bank

        btfss   SSPSTAT,BF      ; Check if buffer is full
        goto    $-1             ; Try again until it empties

        BANKSEL SSPBUF          ; Select RAM bank

        IFDEF   SPI_SDI_INV
        comf    SSPBUF,W        ; Get the received byte (and invert it)
        ELSE
        movf    SSPBUF,W        ; Get the received byte
        ENDIF

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine to transmit a byte, wait for it to go and receive but discard the result
;;
;; Parameters:  w (in)          - Byte to transmit
;;              w (out)         - Byte to transmit (unchanged)
;;
;; RAM Bank:    any (in) SSPBUF (out)
;;
;;--------------------------------------------------------------------------------

spi_tx_wait_discard_byte
        BANKSEL SSPBUF          ; Select RAM bank

        movwf   spi_temp        ; Store the data to be transmitted

        movwf   SSPBUF          ; Put the transmit byte

        BANKSEL SSPSTAT         ; Select RAM bank

        btfss   SSPSTAT,BF      ; Check if buffer is full
        goto    $-1             ; Try again until it empties

        BANKSEL SSPBUF          ; Select RAM bank

        IFDEF   SPI_SDI_INV
        comf    SSPBUF,W        ; Get the received byte (and invert it)
        ELSE
        movf    SSPBUF,W        ; Get the received byte
        ENDIF

        movf    spi_temp,W      ; Restore the transmitted byte

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine to enable the chip select
;;
;; RAM Bank:    any (in) SSPBUF (out)
;;
;;--------------------------------------------------------------------------------

spi_enable_chip_select
        BANKSEL SSP_PORT        ; Select RAM bank

        bcf     SSP_PORT,SSP_SS ; Clear the bit (to enable chip select)

        BANKSEL SSPBUF          ; Select RAM bank

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine to disable the chip select
;;
;; RAM Bank:    any (in) SSPBUF (out)
;;
;;--------------------------------------------------------------------------------

spi_disable_chip_select
        BANKSEL SSP_PORT        ; Select RAM bank

        bsf     SSP_PORT,SSP_SS ; Set the bit (to disable chip select)

        BANKSEL SSPBUF          ; Select RAM bank

        return


;;--------------------------------------------------------------------------------

;; Re-enable messages and warnings

        ERRORLEVEL      0


;;--------------------------------------------------------------------------------
