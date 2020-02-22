;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/projects/13_rs232-to-ps2/RCS/keyboard_host.inc,v 1.1 2007/06/05 17:37:42 amb Exp $
;;
;; PS/2 host emulating program for keyboard
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2007 Andrew M. Bishop
;; It may be distributed under the GNU Public License, version 2, or
;; any higher version.  See section COPYING of the GNU Public license
;; for conditions under which this file may be redistributed.
;;
;;--------------------------------------------------------------------------------


ps2_keyboard_host

ps2_keyboard_host_send_reset

        movlw   250
        call    delay_ms

        ;; Reset device

        movlw   0xff
        call    send_byte

        call    get_byte
        movlw   0xfa
        xorwf   ps2_data,F
        btfss   STATUS,Z
        goto    ps2_keyboard_host_send_reset

        call    get_byte
        movlw   0xaa
        xorwf   ps2_data,F
        btfss   STATUS,Z
        goto    ps2_keyboard_host_send_reset

ps2_keyboard_host_set_defaults

        movlw   2
        movwf   scancode

        movlw   0x2b
        movwf   typematic


;; F5 => disable
        movlw   0xf5
        call    send_byte

        call    get_byte

;; F6 => set default
        movlw   0xf6
        call    send_byte

        call    get_byte

;; F2 => id
        movlw   0xf2
        call    send_byte

        call    get_byte

        call    get_byte

        call    get_byte

;; ED => LED
        movlw   0xed
        call    send_byte

        call    get_byte

        movlw   7
        call    send_byte

        call    get_byte

;; EE => echo
        movlw   0xee
        call    send_byte

        call    get_byte

;; F0 => set scan code
        movlw   0xf0
        call    send_byte

        call    get_byte

        movlw   0x00
        call    send_byte

        call    get_byte

;; F3 => set typematic rate
        movlw   0xf3
        call    send_byte

        call    get_byte

        movlw   0xc2
        call    send_byte

        call    get_byte

;; F4 => enable
        movlw   0xf4
        call    send_byte

        call    get_byte


ps2_keyboard_host_loop

        call    get_byte

        goto    ps2_keyboard_host_loop
