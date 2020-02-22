;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/pcd8544.inc,v 1.2 2007/06/17 16:30:20 amb Exp $
;;
;; Overall include file for PCD8544 based LCDs.
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
;; Required Variables / Constants for PCD8544 based LCD functions
;;
;; Constants:   CLOCK           - The PIC clock speed in Hz
;;              PCD8544_PORT    - Selects the port for LCD output
;;              PCD8544_TRIS    - Selects the tristate register for PCD8544_PORT
;;              PCD8544_SDOUT   - The bit to use for data output
;;              PCD8544_SCLK    - The bit to use for clock output
;;              PCD8544_D_C     - The bit to use for data or command select
;;              PCD8544_SCE     - The bit to use for chip select
;;
;; Variables:   pcd8544_temp    - Stores temporary data, same bank as PCD8544_PORT
;;              spi_temp        - Stores temporary data, same bank as PCD8544_PORT
;;
;;--------------------------------------------------------------------------------


;;
;; PCD8544LCD Basic functions
;;

        include "../common/pcd8544_basic.inc"

;;
;; PCD8544LCD bit toggling and SPI functions (bit-banging)
;;

        include "../common/pcd8544_spi.inc"

;;
;; PCD8544LCD higher level functions
;;

        include "../common/pcd8544_funcs.inc"


;;--------------------------------------------------------------------------------
