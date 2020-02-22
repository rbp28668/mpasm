;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/projects/04_rs232-to-i2c/RCS/program.asm,v 1.9 2009/08/01 18:30:04 amb Exp $
;;
;; Program for RS232 to I2C converter.
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2006, 2009 Andrew M. Bishop
;; It may be distributed under the GNU Public License, version 2, or
;; any higher version.  See section COPYING of the GNU Public license
;; for conditions under which this file may be redistributed.
;;
;;--------------------------------------------------------------------------------

;;  Project title

        TITLE   "RS232 to I2C converter"

;; Compilation options

        RADIX   DEC
        EXPAND

;; Processor

        Processor pic12f675

        include "p12f675.inc"

;; Config fuses

        __CONFIG (_CPD_OFF & _CP_OFF & _BODEN_OFF & _MCLRE_ON & _PWRTE_OFF & _WDT_OFF & _INTRC_OSC_NOCLKOUT & b'1111000111111111')

;; Variables and constants

CLOCK           EQU  4000000    ; Required for "delay.inc", "i2c.inc", "rs232.inc"

I2C_PORT        EQU  GPIO       ; Required for "i2c.inc"
I2C_TRIS        EQU  TRISIO     ; Required for "i2c.inc"
I2C_SCL         EQU  4          ; Required for "i2c.inc"
I2C_SDA         EQU  5          ; Required for "i2c.inc"

RS232_PORT      EQU  GPIO       ; Required for "rs232.inc"
RS232_TRIS      EQU  TRISIO     ; Required for "rs232.inc"
RS232_BAUD      EQU  38400      ; Required for "rs232.inc"
RS232_RXD       EQU  0          ; Required for "rs232.inc"
RS232_TXD       EQU  1          ; Required for "rs232.inc"
RS232_RTS       EQU  2          ; Required for "rs232.inc"

        cblock  0x5e
                state
                temp
        endc

delay_temp      EQU  temp       ; Required for "delay.inc"
i2c_temp        EQU  temp       ; Required for "i2c.inc"
rs232_temp      EQU  temp       ; Required for "rs232.inc"

STATE_GENERIC   EQU  'S'        ; Generic read/write (in generic.inc)
STATE_EEPROM_D  EQU  'D'        ; Dump (in eeprom.inc)
STATE_EEPROM_L  EQU  'L'        ; Load (in eeprom.inc)
STATE_EEPROM_E  EQU  'E'        ; Erase (in eeprom.inc)


;; Reset and interrupt vectors

        org     0x00

reset_vector
        goto    program
        nop
        nop
        nop
int_vector
        nop
program

;; Generic reset of device

        call    reset_device

;; Wait 1 second to make ICSP easier

        movlw   250
        call    delay_ms

        movlw   250
        call    delay_ms

        movlw   250
        call    delay_ms

        movlw   250
        call    delay_ms

;; Enable I2C ports

        call    i2c_init_port

;; Enable RS232 ports

        call    rs232_init_port

;; Main program

main
        BANKSEL GPIO

        ;; Read one byte from RS232

        call    rs232_rx_byte
        movwf   state

        ;; Decide which function to call

        movlw   STATE_GENERIC
        subwf   state,W
        btfss   STATUS,Z
        goto    $+3
        call    generic_i2c
        goto    main

        movlw   STATE_EEPROM_L
        subwf   state,W
        btfss   STATUS,Z
        goto    $+3
        call    eeprom_load
        goto    main

        movlw   STATE_EEPROM_D
        subwf   state,W
        btfss   STATUS,Z
        goto    $+3
        call    eeprom_dump
        goto    main

        movlw   STATE_EEPROM_E
        subwf   state,W
        btfss   STATUS,Z
        goto    $+3
        call    eeprom_erase
        goto    main

        ;; Anything else is looped back
        movf    state,W
        call    rs232_tx_byte

        goto    main


;; Subroutines

        include "generic.inc"

        include "eeprom.inc"

;; Subroutines loaded from include files

        include "../devices/12f675/reset_device.inc"

        include "../common/delay.inc"

        include "../common/rs232.inc"

        include "../common/i2c.inc"

;; End

        end
