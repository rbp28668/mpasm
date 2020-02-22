;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/ps2_monitor.inc,v 1.4 2007/03/19 19:50:23 amb Exp $
;;
;; PS/2 device and host monitoring functions.
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
;; Required Variables / Constants for PS/2 functions
;;
;; Constants:   PS2_TIMEOUT     - The PS2 clock edge timeout in loop repetitions
;;              PS2_PORT        - Selects the port containing the clock/data bits
;;              PS2_TRIS        - Selects the tristate register for the PS2_PORT port
;;              PS2_DATA        - The bit to use for data
;;              PS2_CLK         - The bit to use for clock
;;
;; Variables:   ps2_data        - Stores the data byte, same bank as PS2_PORT
;;              ps2_status      - Stores the status, same bank as PS2_PORT
;;              ps2_temp        - Stores temporary data, same bank as PS2_PORT
;;
;;--------------------------------------------------------------------------------


;;--------------------------------------------------------------------------------
;;
;; Subroutine for PS/2 host and device monitoring
;;
;; Parameters:  w (out)         - interrupt position if interrupted, zero otherwise
;;              ps2_data (out)  - data byte
;;              ps2_status(out) - status byte bit 0 = interrupted
;;                                bit 1 = 0 if host to device, 1 if device to host
;;                                bit 2 = 0 if OK, 1 if interrupted
;;                                bit 3 = 0 if parity OK, 1 if parity bad
;;
;; RAM Bank:    any (in) PS2_PORT (out)
;;
;;--------------------------------------------------------------------------------

ps2_monitor_byte

        BANKSEL PS2_PORT        ; Select RAM bank

        clrf    ps2_data
        clrf    ps2_status

        IFDEF   PS2_CLOCK_INV
        btfss   PS2_PORT,PS2_CLOCK ; Wait for clock to go low (data inverted)
        ELSE
        btfsc   PS2_PORT,PS2_CLOCK ; Wait for clock to go low
        ENDIF
        goto    $-1

        IFDEF   PS2_DATA_INV
        btfsc   PS2_PORT,PS2_DATA ; Skip if data is high (data inverted)
        ELSE
        btfss   PS2_PORT,PS2_DATA ; Skip if data is high
        ENDIF
        goto    ps2_monitor_device_to_host_byte ; data is low

        IFDEF   PS2_CLOCK_INV
        btfsc   PS2_PORT,PS2_CLOCK ; Wait for clock to go high (data inverted)
        ELSE
        btfss   PS2_PORT,PS2_CLOCK ; Wait for clock to go high
        ENDIF
        goto    $-1

        IFDEF   PS2_DATA_INV
        btfsc   PS2_PORT,PS2_DATA ; Skip if data is high (data inverted)
        ELSE
        btfss   PS2_PORT,PS2_DATA ; Skip if data is high
        ENDIF
        goto    ps2_monitor_host_to_device_byte ; data is low

        goto    ps2_monitor_byte


ps2_monitor_device_to_host_byte

;; Timing analysis (capture data on falling edge of clock)
;;
;;          |start|  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |parit|stop
;;
;;    ________.  .__.  .__.  .__.  .__.  .__.  .__.  .__.  .__.  .__.  .__.  .__
;;            |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|
;;    ______       _____ _____ _____ _____ _____ _____ _____ _____ _____ _______
;;          \_____X_____X_____X_____X_____X_____X_____X_____X_____X_____X
;;
;;            ^     ^     ^     ^     ^     ^     ^     ^     ^     ^     ^
;;           [a]   [b]   [c]   [d]   [e]   [f]   [g]   [h]   [i]   [j]   [k]
;;

        ;; Start bit
        call    ps2_dh_mon_skip    ; [a]
        btfsc   ps2_status,2
        retlw   1

        ;; Bit 0
        call    ps2_dh_mon_bit     ; [b]
        btfsc   ps2_status,2
        retlw   2

        ;; Bit 1
        call    ps2_dh_mon_bit     ; [c]
        btfsc   ps2_status,2
        retlw   3

        ;; Bit 2
        call    ps2_dh_mon_bit     ; [d]
        btfsc   ps2_status,2
        retlw   4

        ;; Bit 3
        call    ps2_dh_mon_bit     ; [e]
        btfsc   ps2_status,2
        retlw   5

        ;; Bit 4
        call    ps2_dh_mon_bit     ; [f]
        btfsc   ps2_status,2
        retlw   6

        ;; Bit 5
        call    ps2_dh_mon_bit     ; [g]
        btfsc   ps2_status,2
        retlw   7

        ;; Bit 6
        call    ps2_dh_mon_bit     ; [h]
        btfsc   ps2_status,2
        retlw   8

        ;; Bit 7
        call    ps2_dh_mon_bit     ; [i]
        btfsc   ps2_status,2
        retlw   9

        ;; Parity bit
        call    ps2_dh_mon_parity  ; [j]
        btfsc   ps2_status,2
        retlw   10

        ;; Stop bit
        bsf     ps2_status,1       ; [k]
        bcf     ps2_status,2
        retlw   0


;;--------------------------------------------------------------------------------

ps2_dh_mon_parity

        clrw
        IFDEF   PS2_DATA_INV
        btfss   PS2_PORT,PS2_DATA ; Capture data (data inverted)
        ELSE
        btfsc   PS2_PORT,PS2_DATA ; Capture data
        ENDIF
        movlw   b'10000000'
        xorwf   ps2_status,F    ; update parity bit (bit 7 of ps2_status)
        btfss   ps2_status,7    ; Skip if parity bit is 1 (no error)
        bsf     ps2_status,3    ; parity error
        goto    ps2_dh_mon_skip

ps2_dh_mon_bit

        bcf     STATUS,C
        IFDEF   PS2_DATA_INV
        btfss   PS2_PORT,PS2_DATA ; Capture data (data inverted)
        ELSE
        btfsc   PS2_PORT,PS2_DATA ; Capture data
        ENDIF
        bsf     STATUS,C
        rrf     ps2_data,F

        movlw   b'10000000'
        andwf   ps2_data,W
        xorwf   ps2_status,F    ; update parity bit (bit 7 of ps2_status)

ps2_dh_mon_skip

        movlw   PS2_TIMEOUT  ; Wait only until timeout
        movwf   ps2_temp

ps2_dh_mon_skip_loop

        IFDEF   PS2_CLOCK_INV
        btfss   PS2_PORT,PS2_CLOCK ; Wait for clock to go high (data inverted)
        ELSE
        btfsc   PS2_PORT,PS2_CLOCK ; Wait for clock to go high
        ENDIF
        goto    ps2_dh_mon_skip_high
        decfsz  ps2_temp,F
        goto    ps2_dh_mon_skip_loop

        bsf     ps2_status,2    ; clock failed to go high
        return

ps2_dh_mon_skip_high

        IFDEF   PS2_CLOCK_INV
        btfss   PS2_PORT,PS2_CLOCK ; Wait for clock to go low (data inverted)
        ELSE
        btfsc   PS2_PORT,PS2_CLOCK ; Wait for clock to go low
        ENDIF
        goto    $-1

        bcf     ps2_status,2    ; clock went high and low OK
        return


;;--------------------------------------------------------------------------------

ps2_monitor_host_to_device_byte

;; Timing analysis (capture data on rising edge of clock)
;;
;;          |start|  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |parit|stop | ack |
;;
;;    __.    .__.  .__.  .__.  .__.  .__.  .__.  .__.  .__.  .__.  .__.  .__.  .______
;;      |____|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|
;;    _____        _____ _____ _____ _____ _____ _____ _____ _____ _____ __       ____
;;         \______X_____X_____X_____X_____X_____X_____X_____X_____X_____X  \_____/
;;
;;           ^     ^     ^     ^     ^     ^     ^     ^     ^     ^     ^     ^
;;          [a]   [b]   [c]   [d]   [e]   [f]   [g]   [h]   [i]   [j]   [k]   [l]
;;

        ;; Start bit
        call    ps2_hd_mon_skip    ; [a]

        ;; Bit 0
        call    ps2_hd_mon_bit     ; [b]

        ;; Bit 1
        call    ps2_hd_mon_bit     ; [c]

        ;; Bit 2
        call    ps2_hd_mon_bit     ; [d]

        ;; Bit 3
        call    ps2_hd_mon_bit     ; [e]

        ;; Bit 4
        call    ps2_hd_mon_bit     ; [f]

        ;; Bit 5
        call    ps2_hd_mon_bit     ; [g]

        ;; Bit 6
        call    ps2_hd_mon_bit     ; [h]

        ;; Bit 7
        call    ps2_hd_mon_bit     ; [i]

        ;; Parity bit
        call    ps2_hd_mon_parity  ; [j]

        ;; Stop bit
        call    ps2_hd_mon_skip    ; [k]

        bcf     ps2_status,1       ; [l]
        bcf     ps2_status,2
        retlw   0


;;--------------------------------------------------------------------------------

ps2_hd_mon_parity

        clrw
        IFDEF   PS2_DATA_INV
        btfss   PS2_PORT,PS2_DATA ; Capture data (data inverted)
        ELSE
        btfsc   PS2_PORT,PS2_DATA ; Capture data
        ENDIF
        movlw   b'10000000'
        xorwf   ps2_status,F    ; update parity bit (bit 7 of ps2_status)
        btfss   ps2_status,7    ; Skip if parity bit is 1 (no error)
        bsf     ps2_status,3    ; parity error
        goto    ps2_hd_mon_skip

ps2_hd_mon_bit

        bcf     STATUS,C
        IFDEF   PS2_DATA_INV
        btfss   PS2_PORT,PS2_DATA ; Capture data (data inverted)
        ELSE
        btfsc   PS2_PORT,PS2_DATA ; Capture data
        ENDIF
        bsf     STATUS,C
        rrf     ps2_data,F

        movlw   b'10000000'
        andwf   ps2_data,W
        xorwf   ps2_status,F    ; update parity bit (bit 7 of ps2_status)

ps2_hd_mon_skip

        IFDEF   PS2_CLOCK_INV
        btfss   PS2_PORT,PS2_CLOCK ; Wait for clock to go low (data inverted)
        ELSE
        btfsc   PS2_PORT,PS2_CLOCK ; Wait for clock to go low
        ENDIF
        goto    $-1

        IFDEF   PS2_CLOCK_INV
        btfsc   PS2_PORT,PS2_CLOCK ; Wait for clock to go high (data inverted)
        ELSE
        btfss   PS2_PORT,PS2_CLOCK ; Wait for clock to go high
        ENDIF
        goto    $-1

        return                  ; clock went high OK


;;--------------------------------------------------------------------------------
