;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/projects/13_rs232-to-ps2/RCS/program.asm,v 1.2 2009/08/01 18:30:51 amb Exp $
;;
;; Program for RS232 to PS/2 converter.
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2007, 2009 Andrew M. Bishop
;; It may be distributed under the GNU Public License, version 2, or
;; any higher version.  See section COPYING of the GNU Public license
;; for conditions under which this file may be redistributed.
;;
;;--------------------------------------------------------------------------------

;;  Project title

        TITLE   "RS232 to PS/2 converter"

;; Compilation options

        RADIX   DEC
        EXPAND

;; Processor

        Processor pic12f683

        include "p12f683.inc"

;; Config fuses

        __CONFIG (_CPD_OFF & _CP_OFF & _BOD_OFF & _MCLRE_ON & _PWRTE_OFF & _WDT_OFF & _FCMEN_OFF & _IESO_OFF & _INTOSCIO & b'1111000111111111')

;; Variables and constants

CLOCK           EQU  4000000    ; Required for "delay.inc", "i2c.inc", "rs232.inc"

RS232_PORT      EQU  GPIO       ; Required for "rs232.inc"
RS232_TRIS      EQU  TRISIO     ; Required for "rs232.inc"
RS232_BAUD      EQU  38400      ; Required for "rs232.inc"
RS232_RXD       EQU  0          ; Required for "rs232.inc"
RS232_TXD       EQU  1          ; Required for "rs232.inc"
RS232_RTS       EQU  2          ; Required for "rs232.inc"

PS2_DATA        EQU  4          ; Required for "ps2.inc"
PS2_CLOCK       EQU  5          ; Required for "ps2.inc"
PS2_PORT        EQU  GPIO       ; Required for "ps2.inc"
PS2_TRIS        EQU  TRISIO     ; Required for "ps2.inc"

STATE_MOUSE_HOST        EQU  'M'
STATE_KEYBOARD_HOST     EQU  'K'


        cblock  0x5e
                state
                byte

                ;; Mouse variables
                mode
                samplerate
                resolution
                scaling

                ;; Keyboard variables
                scancode
                typematic
                key_state

                ;; Mouse and keyboard variables
                disabled

                delay_temp      ; Required for "delay.inc"
                rs232_temp      ; Required for "rs232.inc"
                ps2_data        ; Required for "ps2.inc"
                ps2_status      ; Required for "ps2.inc"
                ps2_temp        ; Required for "ps2.inc"
        endc

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

;; Enable PS2 ports

        call    ps2_init_port

;; Enable RS232 ports

        call    rs232_init_port

;; Main program

main
        BANKSEL GPIO

        ;; Read one byte from RS232

        call    rs232_rx_byte
        movwf   state

        ;; Decide which function to call

        movlw   STATE_MOUSE_HOST
        subwf   state,W
        btfss   STATUS,Z
        goto    $+3
        call    ps2_mouse_host
        goto    main

        movlw   STATE_KEYBOARD_HOST
        subwf   state,W
        btfss   STATUS,Z
        goto    $+3
        call    ps2_keyboard_host
        goto    main

        ;; Anything else is looped back
        movf    state,W
        call    rs232_tx_byte

        goto    main


;; Send a byte (from PC to PS/2)
send_byte
        movwf   byte

        bsf     STATUS,Z
        btfss   PS2_PORT,PS2_DATA
        return

        btfss   PS2_PORT,PS2_CLOCK
        goto    send_byte

        movf    byte,W
        call    ps2_host_tx_byte

        return


;; Get a byte (from PS/2 to PC)
get_byte
        call    ps2_host_rx_byte

        movf    ps2_data,W
        movwf   byte

        BANKSEL PS2_TRIS
        bcf     PS2_TRIS,PS2_CLOCK ; Enable clock output; clock goes low
        BANKSEL PS2_PORT

        call    rs232_tx_byte

        BANKSEL PS2_TRIS
        bsf     PS2_TRIS,PS2_CLOCK ; Disable clock output; clock goes high
        BANKSEL PS2_PORT

        return


;; Subroutines loaded from include files

        include "mouse_host.inc"

        include "keyboard_host.inc"

        include "../devices/12f683/reset_device.inc"

        include "../common/delay.inc"

        include "../common/rs232.inc"

        include "../common/ps2.inc"

;; End

        end
