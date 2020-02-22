;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/spi_basic.inc,v 1.4 2007/05/21 15:15:38 amb Exp $
;;
;; Basic (hardware) SPI functions.
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
;; Constants:   SPI_CKE         - Selects the clock edge to transmit data on
;;              SPI_CKP         - Selects the clock idle state
;;              SPI_SMP         - Selects the point to sample the data
;;              SPI_RATE_DIV    - Selects the SPI port rate divide ratio
;;              SSP_PORT        - Selects the port containing the SSP port (automatically set)
;;              SSP_TRIS        - Selects the tristate register for the SSP port (automatically set)
;;              SSP_SDI         - The bit to use for SDI data input (automatically set)
;;              SSP_SDO         - The bit to use for SDO data output (automatically set)
;;              SSP_SS          - The bit to use for SS chip select (automatically set)
;;              SSP_SCK         - The bit to use for SC data clock (automatically set)
;;
;;--------------------------------------------------------------------------------

;;--------------------------------------------------------------------------------

;; Temporarily disable messages and warnings

        ERRORLEVEL      2


;;--------------------------------------------------------------------------------
;;
;; Subroutine to initialise the SSP port
;;
;; RAM Bank:    any (in) SSP_PORT (out)
;;
;;--------------------------------------------------------------------------------

spi_init_master_port
        BANKSEL SSP_TRIS        ; Select RAM bank

        bsf     SSP_TRIS,SSP_SDI; SDI is input
        bcf     SSP_TRIS,SSP_SDO; SDO is output
        bcf     SSP_TRIS,SSP_SCK; SCK is output
        bcf     SSP_TRIS,SSP_SS ; SS is output

        BANKSEL SSP_PORT        ; Select RAM bank

        bsf     SSP_PORT,SSP_SS ; Disable the chip select

        BANKSEL SSPSTAT         ; Select RAM bank

        IF      SPI_SMP
        bsf     SSPSTAT,SMP     ; Sample data at end
        ELSE
        bcf     SSPSTAT,SMP     ; Sample data at middle
        ENDIF

        IF      SPI_CKE
        bsf     SSPSTAT,CKE     ; Output data when clock changes from idle to active
        ELSE
        bcf     SSPSTAT,CKE     ; Output data when clock changes from active to idle
        ENDIF

        BANKSEL SSPCON          ; Select RAM bank

        bcf     SSPCON,WCOL     ; Clear collision flag

        bcf     SSPCON,SSPOV    ; Clear overflow flag

        IF      SPI_CKP
        bsf     SSPCON,CKP      ; Idle state for clock is high
        ELSE
        bcf     SSPCON,CKP      ; Idle state for clock is low
        ENDIF

        movlw   0xf0
        andwf   SSPCON,F        ; Clear mode bits

        IF      SPI_RATE_DIV==4
        movlw   0
        ENDIF
        IF      SPI_RATE_DIV==16
        movlw   1
        ENDIF
        IF      SPI_RATE_DIV==64
        movlw   2
        ENDIF

        iorwf   SSPCON,F        ; Set the mode bits

        bsf     SSPCON,SSPEN    ; Enable the SSP

        return


;;--------------------------------------------------------------------------------

;; Re-enable messages and warnings

        ERRORLEVEL      0


;;--------------------------------------------------------------------------------
