;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/pcd8544_funcs.inc,v 1.5 2010/09/19 09:54:51 amb Exp $
;;
;; Include file for PCD8544 based LCD high level functions.
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
;;
;; Variables:   pcd8544_temp    - Stores temporary data, same bank as PCD8544_PORT
;;
;;--------------------------------------------------------------------------------


;;--------------------------------------------------------------------------------
;;
;; Subroutine to initialise the display registers
;; (Uses macros from pcd8544_basic.inc instead of subroutines for speed)
;;
;; Parameters:  none
;;
;; RAM Bank:    any (in) PCD8544_PORT (out)
;;
;;--------------------------------------------------------------------------------

pcd8544_init_display
        BANKSEL PCD8544_PORT

        PCD8544_CHIP_ENABLE     ; Enable the chip

        PCD8544_COMMAND_MODE    ; Enter command mode

        ;; PD=0 => not power down; V=0 => horizontal addressing; H=1 => extended instructions

        movlw   0x20 + 0x01
        call    pcd8544_tx_byte

        ;; Temperature compensation=1

        movlw   0x04 + 0x01
        call    pcd8544_tx_byte

        ;; Bias=4 => 011 (default value from Philips PCD8544 datasheet)

        movlw   0x10 + 0x03
        call    pcd8544_tx_byte

        ;; Voutput=0x45  (value from Goldentek datasheet)

        movlw   0x80 + 0x45
        call    pcd8544_tx_byte

        ;; PD=0 => not power down; V=0 => horizontal addressing; H=0 => normal instructions

        movlw   0x20 + 0x00
        call    pcd8544_tx_byte

        goto    pcd8544_clear


;;--------------------------------------------------------------------------------
;;
;; Subroutine to clear the display
;; (Uses macros from pcd8544_basic.inc instead of subroutines for speed)
;;
;; Parameters:  none
;;
;; RAM Bank:    any (in) PCD8544_PORT (out)
;;
;;--------------------------------------------------------------------------------

pcd8544_clear
        BANKSEL PCD8544_PORT

        PCD8544_COMMAND_MODE    ; Enter command mode

        ;; D=0, E=0 => Blank mode

        movlw   0x08
        call    pcd8544_tx_byte

        ;; X=0; auto-incremented first for horizontal addressing

        movlw   0x80 + 0x00
        call    pcd8544_tx_byte

        ;; Y=0; auto-incremented second for horizontal addressing

        movlw   0x40 + 0x00
        call    pcd8544_tx_byte

        ;; Loop to clear out the RAM

        PCD8544_DATA_MODE       ; Enter data mode

        movlw   252             ; (84*48/8)/2
        movwf   pcd8544_temp
        movlw   0

pcd8544_clear_loop
        call    pcd8544_tx_byte
        call    pcd8544_tx_byte

        decfsz  pcd8544_temp,F
        goto    pcd8544_clear_loop

        PCD8544_COMMAND_MODE    ; Enter command mode

        ;; D=1, E=0 => Normal mode

        movlw   0x08 + 0x04
        call    pcd8544_tx_byte

        goto    pcd8544_reset_xy


;;--------------------------------------------------------------------------------
;;
;; Subroutine to reset the X,Y coordinates to the top
;; (Uses macros from pcd8544_basic.inc instead of subroutines for speed)
;;
;; Parameters:  none
;;
;; RAM Bank:    any (in) PCD8544_PORT (out)
;;
;;--------------------------------------------------------------------------------

pcd8544_reset_xy
        BANKSEL PCD8544_PORT

        PCD8544_COMMAND_MODE    ; Enter command mode

        ;; X=0; auto-incremented first for horizontal addressing

        movlw   0x80 + 0x00
        call    pcd8544_tx_byte

        ;; Y=0; auto-incremented second for horizontal addressing

        movlw   0x40 + 0x00
        call    pcd8544_tx_byte

        PCD8544_DATA_MODE       ; Enter data mode

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine to set normal mode
;; (Uses macros from pcd8544_basic.inc instead of subroutines for speed)
;;
;; Parameters:  none
;;
;; RAM Bank:    any (in) PCD8544_PORT (out)
;;
;;--------------------------------------------------------------------------------

pcd8544_mode_normal
        BANKSEL PCD8544_PORT

        PCD8544_COMMAND_MODE    ; Enter command mode

        ;; D=1, E=0 => Normal mode

        movlw   0x08 + 0x04
        call    pcd8544_tx_byte

        PCD8544_DATA_MODE       ; Enter data mode

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine to set inverse mode
;; (Uses macros from pcd8544_basic.inc instead of subroutines for speed)
;;
;; Parameters:  none
;;
;; RAM Bank:    any (in) PCD8544_PORT (out)
;;
;;--------------------------------------------------------------------------------

pcd8544_mode_inverse
        BANKSEL PCD8544_PORT

        PCD8544_COMMAND_MODE    ; Enter command mode

        ;; D=1, E=1 => Inverse mode

        movlw   0x08 + 0x04 + 0x01
        call    pcd8544_tx_byte

        PCD8544_DATA_MODE       ; Enter data mode

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine to flash the display
;; (Uses macros from pcd8544_basic.inc instead of subroutines for speed)
;;
;; Parameters:  none
;;
;; RAM Bank:    any (in) PCD8544_PORT (out)
;;
;;--------------------------------------------------------------------------------

pcd8544_flash_all_pixels
        BANKSEL PCD8544_PORT

        PCD8544_COMMAND_MODE    ; Enter command mode

        ;; D=0, E=0 => All pixels off mode

        movlw   0x08
        call    pcd8544_tx_byte

        movlw   200
        call    delay_ms

        ;; D=0, E=1 => All pixels on mode

        movlw   0x08 + 0x01
        call    pcd8544_tx_byte

        movlw   200
        call    delay_ms

        ;; D=1, E=0 => Normal mode

        movlw   0x08 + 0x04
        call    pcd8544_tx_byte

        PCD8544_DATA_MODE       ; Enter data mode

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine to send a NOP (seems to be needed if SCE line is not being used)
;; (Uses macros from pcd8544_basic.inc instead of subroutines for speed)
;;
;; Parameters:  none
;;
;; RAM Bank:    any (in) PCD8544_PORT (out)
;;
;;--------------------------------------------------------------------------------

pcd8544_nop
        BANKSEL PCD8544_PORT

        PCD8544_COMMAND_MODE    ; Enter command mode

        movlw   0
        call    pcd8544_tx_byte

        PCD8544_DATA_MODE       ; Enter data mode

        return


;;--------------------------------------------------------------------------------
