;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/projects/13_rs232-to-ps2/RCS/mouse_host.inc,v 1.1 2007/06/05 17:37:51 amb Exp $
;;
;; PS/2 host emulating program for mouse
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2007 Andrew M. Bishop
;; It may be distributed under the GNU Public License, version 2, or
;; any higher version.  See section COPYING of the GNU Public license
;; for conditions under which this file may be redistributed.
;;
;;--------------------------------------------------------------------------------

MODE_WRAP       EQU  1
MODE_STREAM     EQU  2
MODE_REMOTE     EQU  3

ps2_mouse_host

ps2_mouse_host_send_reset

        ;; Reset device

        movlw   0xff
        call    send_byte

        call    get_byte
        movlw   0xfa
        xorwf   ps2_data,F
        btfss   STATUS,Z
        goto    ps2_mouse_host_send_reset

        call    get_byte
        movlw   0xaa
        xorwf   ps2_data,F
        btfss   STATUS,Z
        goto    ps2_mouse_host_send_reset

        call    get_byte
        movlw   0x00
        xorwf   ps2_data,F
        btfss   STATUS,Z
        goto    ps2_mouse_host_send_reset

ps2_mouse_host_set_defaults

        movlw   100
        movwf   samplerate

        movlw   4
        movwf   resolution

        movlw   1
        movwf   scaling

        movlw   MODE_STREAM
        movwf   mode

        movlw   1
        movwf   disabled


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

;; F3 => set sample rate
        movlw   0xf3
        call    send_byte

        call    get_byte

        movlw   0x64
        call    send_byte

        call    get_byte

;; E6 => set scaling 1:1
        movlw   0xe6
        call    send_byte

        call    get_byte

;; E7 => set scaling 2:1
        movlw   0xe7
        call    send_byte

        call    get_byte

;; E8 => set resolution
        movlw   0xe8
        call    send_byte

        call    get_byte

        movlw   0x03
        call    send_byte

        call    get_byte

;; E9 => status request
        movlw   0xe9
        call    send_byte

        call    get_byte

        call    get_byte

        call    get_byte

        call    get_byte

;; EB => read data
        movlw   0xeb
        call    send_byte

        call    get_byte

        call    get_byte

        call    get_byte

        call    get_byte

;; F4 => enable
        movlw   0xf4
        call    send_byte

        call    get_byte


ps2_mouse_host_loop
        call    get_byte

        goto    ps2_mouse_host_loop
