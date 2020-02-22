;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/pcd8544_spi.inc,v 1.3 2010/09/19 09:54:51 amb Exp $
;;
;; Include file for PCD8544 based LCD SPI functions.
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2007, 2010 Andrew M. Bishop
;; It may be distributed under the GNU Public License, version 2, or
;; any higher version.  See section COPYING of the GNU Public license
;; for conditions under which this file may be redistributed.
;;
;;--------------------------------------------------------------------------------

;;--------------------------------------------------------------------------------
;;
;; Required Variables / Constants for PCD8544 based LCD functions
;;
;; Constants:   CLOCK           - The PIC clock speed in Hz
;;              PCD8544_PORT    - Selects the port for LCD output
;;              PCD8544_SDOUT   - The bit to use for data output
;;              PCD8544_SCLK    - The bit to use for clock output
;;
;; Variables:   spi_temp        - Stores temporary data, same bank as PCD8544_PORT
;;
;;--------------------------------------------------------------------------------


;;--------------------------------------------------------------------------------
;;
;; Subroutine to transmit an SPI byte
;;
;; Parameters:  w (in)          - Byte to transmit
;;
;; RAM Bank:    any (in) PCD8544_PORT (out)
;;
;;--------------------------------------------------------------------------------

;; Timing analysis (change data while clock is low)
;;
;;          |  7   |  6   |  5   |  4   |  3   |  2   |  1   |  0   |
;;
;;            .__.   .__.   .__.   .__.   .__.   .__.   .__.   .__.        
;;    ________|  |___|  |___|  |___|  |___|  |___|  |___|  |___|  |________
;;           ______ ______ ______ ______ ______ ______ ______ ______       
;;    ______X______X______X______X______X______X______X______X______X______
;;
;;          ^      ^      ^      ^      ^      ^      ^      ^      ^
;;         [a]    [b]    [c]    [d]    [e]    [f]    [g]    [h]    [i]
;;
;; Minimum setup, hold and high/low period times are 100 ns.
;;
;; PIC at 20 MHz clock has 5 MHz instruction rate which is 200 ns.
;;
;; Clock cycle period is 5 instructions = clock rate (MHz) / 20.
;;

pcd8544_tx_byte
        BANKSEL PCD8544_PORT

        movwf   spi_temp

        ;; Bit 7

        bcf     PCD8544_PORT,PCD8544_SDOUT ; 1 ins [a]
        btfsc   spi_temp,7                 ; 1 ins / 2 ins
        bsf     PCD8544_PORT,PCD8544_SDOUT ; 1 ins / 0 ins
        bsf     PCD8544_PORT,PCD8544_SCLK  ; 1 ins
        bcf     PCD8544_PORT,PCD8544_SCLK  ; 1 ins

        ;; Bit 6

        bcf     PCD8544_PORT,PCD8544_SDOUT ; 1 ins [b]
        btfsc   spi_temp,6                 ; 1 ins / 2 ins
        bsf     PCD8544_PORT,PCD8544_SDOUT ; 1 ins / 0 ins
        bsf     PCD8544_PORT,PCD8544_SCLK  ; 1 ins
        bcf     PCD8544_PORT,PCD8544_SCLK  ; 1 ins

        ;; Bit 5

        bcf     PCD8544_PORT,PCD8544_SDOUT ; 1 ins [c]
        btfsc   spi_temp,5                 ; 1 ins / 2 ins
        bsf     PCD8544_PORT,PCD8544_SDOUT ; 1 ins / 0 ins
        bsf     PCD8544_PORT,PCD8544_SCLK  ; 1 ins
        bcf     PCD8544_PORT,PCD8544_SCLK  ; 1 ins

        ;; Bit 4

        bcf     PCD8544_PORT,PCD8544_SDOUT ; 1 ins [d]
        btfsc   spi_temp,4                 ; 1 ins / 2 ins
        bsf     PCD8544_PORT,PCD8544_SDOUT ; 1 ins / 0 ins
        bsf     PCD8544_PORT,PCD8544_SCLK  ; 1 ins
        bcf     PCD8544_PORT,PCD8544_SCLK  ; 1 ins

        ;; Bit 3

        bcf     PCD8544_PORT,PCD8544_SDOUT ; 1 ins [e]
        btfsc   spi_temp,3                 ; 1 ins / 2 ins
        bsf     PCD8544_PORT,PCD8544_SDOUT ; 1 ins / 0 ins
        bsf     PCD8544_PORT,PCD8544_SCLK  ; 1 ins
        bcf     PCD8544_PORT,PCD8544_SCLK  ; 1 ins

        ;; Bit 2

        bcf     PCD8544_PORT,PCD8544_SDOUT ; 1 ins [f]
        btfsc   spi_temp,2                 ; 1 ins / 2 ins
        bsf     PCD8544_PORT,PCD8544_SDOUT ; 1 ins / 0 ins
        bsf     PCD8544_PORT,PCD8544_SCLK  ; 1 ins
        bcf     PCD8544_PORT,PCD8544_SCLK  ; 1 ins

        ;; Bit 1

        bcf     PCD8544_PORT,PCD8544_SDOUT ; 1 ins [g]
        btfsc   spi_temp,1                 ; 1 ins / 2 ins
        bsf     PCD8544_PORT,PCD8544_SDOUT ; 1 ins / 0 ins
        bsf     PCD8544_PORT,PCD8544_SCLK  ; 1 ins
        bcf     PCD8544_PORT,PCD8544_SCLK  ; 1 ins

        ;; Bit 0

        bcf     PCD8544_PORT,PCD8544_SDOUT ; 1 ins [h]
        btfsc   spi_temp,0                 ; 1 ins / 2 ins
        bsf     PCD8544_PORT,PCD8544_SDOUT ; 1 ins / 0 ins
        bsf     PCD8544_PORT,PCD8544_SCLK  ; 1 ins
        bcf     PCD8544_PORT,PCD8544_SCLK  ; 1 ins

        ;;  Finished

        bcf     PCD8544_PORT,PCD8544_SDOUT ; 1 ins [i]

        return


;;--------------------------------------------------------------------------------
