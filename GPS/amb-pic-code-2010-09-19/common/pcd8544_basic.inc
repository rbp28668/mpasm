;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/pcd8544_basic.inc,v 1.3 2007/08/04 10:45:47 amb Exp $
;;
;; Include file for PCD8544 based LCD basic functions.
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
;;--------------------------------------------------------------------------------


;;--------------------------------------------------------------------------------

;; Temporarily disable messages and warnings

        ERRORLEVEL      2


;;--------------------------------------------------------------------------------
;;
;; Subroutine to initialise the port
;;
;; Parameters:  none
;;
;; RAM Bank:    any (in) PCD8544_PORT (out)
;;
;;--------------------------------------------------------------------------------

pcd8544_init_port
        BANKSEL RS232_TRIS

        bcf     PCD8544_TRIS,PCD8544_SDOUT ; output

        bcf     PCD8544_TRIS,PCD8544_SCLK ; output

        bcf     PCD8544_TRIS,PCD8544_D_C ; output

        IFDEF   PCD8544_SCE
        bcf     PCD8544_TRIS,PCD8544_SCE ; output
        ENDIF

        BANKSEL PCD8544_PORT

        bcf     PCD8544_PORT,PCD8544_SDOUT ; output (start low)

        bcf     PCD8544_PORT,PCD8544_SCLK ; output (normally low between bytes)

        bsf     PCD8544_PORT,PCD8544_D_C ; output (high selects data)

        IFDEF   PCD8544_SCE
        bsf     PCD8544_PORT,PCD8544_SCE ; output (high to disable device)
        ENDIF

        return


;;--------------------------------------------------------------------------------

;; Re-enable messages and warnings

        ERRORLEVEL      0


;;--------------------------------------------------------------------------------
;;
;; Subroutine to enable the LCD (chip enable signal)
;;
;; Parameters:  none
;;
;; RAM Bank:    any (in) PCD8544_PORT (out)
;;
;;--------------------------------------------------------------------------------

pcd8544_chip_enable
        BANKSEL PCD8544_PORT

        IFDEF   PCD8544_SCE
        bcf     PCD8544_PORT,PCD8544_SCE
        ENDIF

        return


;;--------------------------------------------------------------------------------
;;
;; Macro to enable the LCD (chip enable signal)
;;
;; Parameters:  none
;;
;; RAM Bank:    PCD8544_PORT (in) PCD8544_PORT (out)
;;
;;--------------------------------------------------------------------------------

PCD8544_CHIP_ENABLE MACRO

        IFDEF   PCD8544_SCE
        bcf     PCD8544_PORT,PCD8544_SCE
        ENDIF

        ENDM


;;--------------------------------------------------------------------------------
;;
;; Subroutine to disable the LCD (chip enable signal)
;;
;; Parameters:  none
;;
;; RAM Bank:    any (in) PCD8544_PORT (out)
;;
;;--------------------------------------------------------------------------------

pcd8544_chip_disable
        BANKSEL PCD8544_PORT

        IFDEF   PCD8544_SCE
        bsf     PCD8544_PORT,PCD8544_SCE
        ENDIF

        return


;;--------------------------------------------------------------------------------
;;
;; Macro to disable the LCD (chip enable signal)
;;
;; Parameters:  none
;;
;; RAM Bank:    PCD8544_PORT (in) PCD8544_PORT (out)
;;
;;--------------------------------------------------------------------------------

PCD8544_CHIP_DISABLE MACRO

        IFDEF   PCD8544_SCE
        bsf     PCD8544_PORT,PCD8544_SCE
        ENDIF

        ENDM


;;--------------------------------------------------------------------------------
;;
;; Subroutine to select command mode
;;
;; Parameters:  none
;;
;; RAM Bank:    any (in) PCD8544_PORT (out)
;;
;;--------------------------------------------------------------------------------

pcd8544_command_mode
        BANKSEL PCD8544_PORT

        bcf     PCD8544_PORT,PCD8544_D_C

        return


;;--------------------------------------------------------------------------------
;;
;; Macro to select command mode
;;
;; Parameters:  none
;;
;; RAM Bank:    PCD8544_PORT (in) PCD8544_PORT (out)
;;
;;--------------------------------------------------------------------------------

PCD8544_COMMAND_MODE MACRO

        bcf     PCD8544_PORT,PCD8544_D_C

        ENDM


;;--------------------------------------------------------------------------------
;;
;; Subroutine to select data mode
;;
;; Parameters:  none
;;
;; RAM Bank:    any (in) PCD8544_PORT (out)
;;
;;--------------------------------------------------------------------------------

pcd8544_data_mode
        BANKSEL PCD8544_PORT

        bsf     PCD8544_PORT,PCD8544_D_C

        return


;;--------------------------------------------------------------------------------
;;
;; Macro to select data mode
;;
;; Parameters:  none
;;
;; RAM Bank:    PCD8544_PORT (in) PCD8544_PORT (out)
;;
;;--------------------------------------------------------------------------------

PCD8544_DATA_MODE MACRO

        bsf     PCD8544_PORT,PCD8544_D_C

        ENDM


;;--------------------------------------------------------------------------------
