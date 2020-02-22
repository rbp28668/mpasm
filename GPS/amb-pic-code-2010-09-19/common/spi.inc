;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/spi.inc,v 1.4 2007/05/07 18:31:41 amb Exp $
;;
;; Main include file for SPI functions
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
;;              SPI_SDI_INV     - Specifies if the SDI port is inverted
;;              SSP_PORT        - Selects the port containing the SSP port (automatically set)
;;              SSP_TRIS        - Selects the tristate register for the SSP port (automatically set)
;;              SSP_SDI         - The bit to use for SDI data input (automatically set)
;;              SSP_SDO         - The bit to use for SDO data output (automatically set)
;;              SSP_SS          - The bit to use for SS chip select (automatically set)
;;              SSP_SCK         - The bit to use for SC data clock (automatically set)
;;
;; Variables:   spi_temp        - Stores temporary data, same bank as SSPBUF
;;
;;--------------------------------------------------------------------------------


;;
;; SSP Port hardware configuration
;;

        include "../common/ssp_hw.inc"

;;
;; SPI Basic functions
;;

        include "../common/spi_basic.inc"

;;
;; SPI low level functions (SSP)
;;

        include "../common/spi_funcs.inc"

;;--------------------------------------------------------------------------------
