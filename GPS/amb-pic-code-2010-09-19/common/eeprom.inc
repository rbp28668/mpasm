;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/eeprom.inc,v 1.4 2007/05/07 18:31:40 amb Exp $
;;
;; Read from or write to onboard EEPROM.
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
;; Required Variables / Constants for EEPROM read/write function
;;
;; Variables:   eeprom_temp     - Temporary data for EEPROM access
;;
;;--------------------------------------------------------------------------------

        IFDEF   __16F675
EE_VERIFIED     EQU     1
EE_NEED_PGD     EQU     0
        ENDIF

        IFDEF   __16F819
EE_VERIFIED     EQU     1
EE_NEED_PGD     EQU     1
        ENDIF

        IFNDEF  EE_VERIFIED
           ERROR "EEPROM functions not verified for this processor"
        ENDIF

;;--------------------------------------------------------------------------------

;; Temporarily disable messages and warnings

        ERRORLEVEL      2


;;--------------------------------------------------------------------------------
;;
;; Subroutine to read a data byte from EEPROM
;;
;; Parameters:  w (in)          - Address to read from
;;              w (out)         - Data read from address
;;
;; RAM Bank:    any (in), EEDATA (out)
;;
;;--------------------------------------------------------------------------------

eeprom_read
        BANKSEL EEADR           ; Select RAM bank
        movwf   EEADR

        BANKSEL EECON1          ; Select RAM bank

        IF      EE_NEED_PGD
        bcf     EECON1,EEPGD    ; Select bank of EECON1
        ENDIF

        bsf     EECON1,RD       ; Start the read

        BANKSEL EEDATA          ; Select RAM bank
        movf    EEDATA,W

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine to write a data byte to EEPROM
;;
;; Parameters:  w (in)          - Address to write to
;;              EEDATA          - Data to write to address
;;
;; RAM Bank:    any (in), EEDATA (out)
;;
;;--------------------------------------------------------------------------------

eeprom_write
        BANKSEL EEADR           ; Select RAM bank
        movwf   EEADR

        movf    INTCON,W

        bcf     INTCON,GIE      ; Disable INTs

        BANKSEL EECON1          ; Select RAM bank
        movwf   eeprom_temp

        IF      EE_NEED_PGD
        bcf     EECON1,EEPGD    ; Select bank of EECON1
        ENDIF

        bsf     EECON1,WREN     ; Enable EEPROM writes

        movlw   0x55            ; \
        movwf   EECON2          ;  \ Special sequence to
        movlw   0xAA            ;  / write to EEPROM
        movwf   EECON2          ; /
        bsf     EECON1,WR       ; Start the write

        btfsc   EECON1,WR       ; Check if write is complete
        goto    $-1             ; Wait for write to complete

        bcf     EECON1,WREN     ; Disable EEPROM writes

        btfsc   eeprom_temp,GIE
        bsf     INTCON,GIE      ; Enable INTs if enabled to start with

        BANKSEL EEDATA          ; Select RAM bank

        return


;;--------------------------------------------------------------------------------

;; Re-enable messages and warnings

        ERRORLEVEL      0


;;--------------------------------------------------------------------------------
