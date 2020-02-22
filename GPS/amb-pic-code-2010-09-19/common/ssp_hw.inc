;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/ssp_hw.inc,v 1.4 2007/04/19 18:43:48 amb Exp $
;;
;; Definitions of the SSP hardware on different devices.
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2006,07 Andrew M. Bishop
;; It may be distributed under the GNU Public License, version 2, or
;; any higher version.  See section COPYING of the GNU Public license
;; for conditions under which this file may be redistributed.
;;
;;--------------------------------------------------------------------------------

;;--------------------------------------------------------------------------------
;;
;; Constants for SSP functions
;;
;; Constants:   SSP_PORT        - Selects the port containing the SSP port (automatically set)
;;              SSP_TRIS        - Selects the tristate register for the SSP port (automatically set)
;;              SSP_SDI         - The bit to use for SDI data input (automatically set)
;;              SSP_SDO         - The bit to use for SDO data output (automatically set)
;;              SSP_SS          - The bit to use for SS chip select (automatically set)
;;              SSP_SCK         - The bit to use for SC data clock (automatically set)
;;              SSP_SDA         - The bit to use for SDA data in/out (automatically set)
;;              SSP_SCL         - The bit to use for SCL clock in/out (automatically set)
;;
;;--------------------------------------------------------------------------------

;;--------------------------------------------------------------------------------
;; 16F818
;;--------------------------------------------------------------------------------

        IFDEF __16F818

SSP_PORT        EQU     PORTB
SSP_TRIS        EQU     TRISB

SSP_SDI         EQU     1
SSP_SDO         EQU     2
SSP_SS          EQU     5
SSP_SCK         EQU     4

SSP_SDA         EQU     1
SSP_SCL         EQU     4

        ENDIF

;;--------------------------------------------------------------------------------
;; 16F819
;;--------------------------------------------------------------------------------

        IFDEF __16F819

SSP_PORT        EQU     PORTB
SSP_TRIS        EQU     TRISB

SSP_SDI         EQU     1
SSP_SDO         EQU     2
SSP_SS          EQU     5
SSP_SCK         EQU     4

SSP_SDA         EQU     1
SSP_SCL         EQU     4

        ENDIF

;;--------------------------------------------------------------------------------
