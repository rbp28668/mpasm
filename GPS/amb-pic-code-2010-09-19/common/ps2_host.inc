;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/ps2_host.inc,v 1.3 2010/09/19 09:55:37 amb Exp $
;;
;; PS/2 host functions.
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
;; Required Variables / Constants for PS/2 functions
;;
;; Constants:   PS2_TIMEOUT     - The PS2 clock edge timeout in loop repetitions
;;              PS2_QPERIOD     - One quarter of a clock period in loop repetitions
;;              PS2_PORT        - Selects the port containing the clock/data bits
;;              PS2_TRIS        - Selects the tristate register for the PS2_PORT port
;;              PS2_DATA        - The bit to use for data
;;              PS2_CLOCK       - The bit to use for clock
;;
;; Variables:   ps2_data        - Stores the data byte, same bank as PS2_PORT
;;              ps2_status      - Stores the status, same bank as PS2_PORT
;;              ps2_temp        - Stores temporary data, same bank as PS2_PORT
;;
;;--------------------------------------------------------------------------------


;;--------------------------------------------------------------------------------

;; Temporarily disable messages and warnings

        ERRORLEVEL      2


;;--------------------------------------------------------------------------------
;;
;; Subroutine for PS/2 host byte transmission
;;
;; Parameters:  w (in)          - byte to transmit
;;              ps2_status(out) - status byte bit 0 = interrupted
;;                                bit 1 = 0 if host to device, 1 if device to host
;;                                bit 4 = 0 if ACK seen, 1 if no ACK
;;
;; RAM Bank:    any (in) PS2_PORT (out)
;;
;;--------------------------------------------------------------------------------

ps2_host_tx_byte

        BANKSEL PS2_PORT        ; Select RAM bank

        movwf   ps2_data

        clrf    ps2_status

        clrf    ps2_temp
        comf    ps2_temp,F
        bcf     ps2_temp,PS2_CLOCK
        bcf     ps2_temp,PS2_DATA
        movf    ps2_temp,W
        andwf   PS2_PORT,F     ; Make sure that both bits are cleared at once (R-M-W problem)

        BANKSEL PS2_TRIS
        bcf     PS2_TRIS,PS2_CLOCK ; Enable clock output; clock goes low
        BANKSEL PS2_PORT

        movlw   PS2_TIMEOUT    ; Timeout ~100us
        movwf   ps2_temp
        decfsz  ps2_temp,F
        goto    $-1

        BANKSEL PS2_TRIS
        bcf     PS2_TRIS,PS2_DATA ; Enable data output; data goes low
        BANKSEL PS2_PORT

        movlw   PS2_QPERIOD   ; Quarter of a period
        movwf   ps2_temp
        decfsz  ps2_temp,F
        goto    $-1

        BANKSEL PS2_TRIS
        bsf     PS2_TRIS,PS2_CLOCK ; Disable clock output; clock goes high
        BANKSEL PS2_PORT


;; Timing analysis (output data after falling edge of clock)
;;
;;          |start|  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |parit|stop | ack |
;;
;;    __.    .__.  .__.  .__.  .__.  .__.  .__.  .__.  .__.  .__.  .__.  .__.  .______
;;      |____|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|
;;    _____        _____ _____ _____ _____ _____ _____ _____ _____ _____ __       ____
;;         \______X_____X_____X_____X_____X_____X_____X_____X_____X_____X  \_____/
;;
;;           ^    ^     ^     ^     ^     ^     ^     ^     ^     ^     ^    ^ ^
;;          [a]  [b]   [c]   [d]   [e]   [f]   [g]   [h]   [i]   [j]   [k]  [l][m]
;;

        ;; Start bit
        call    ps2_hd_tx_delay    ; [a]

        ;; Bit 0
        call    ps2_hd_tx_bit      ; [b]

        ;; Bit 1
        call    ps2_hd_tx_bit      ; [c]

        ;; Bit 2
        call    ps2_hd_tx_bit      ; [d]

        ;; Bit 3
        call    ps2_hd_tx_bit      ; [e]

        ;; Bit 4
        call    ps2_hd_tx_bit      ; [f]

        ;; Bit 5
        call    ps2_hd_tx_bit      ; [g]

        ;; Bit 6
        call    ps2_hd_tx_bit      ; [h]

        ;; Bit 7
        call    ps2_hd_tx_bit      ; [i]

        ;; Parity bit
        call    ps2_hd_tx_parity   ; [j]

        ;; Stop bit
        call    ps2_hd_tx_1        ; [k]

        ;; ACK bit
        call    ps2_hd_rx_ack      ; [l]

        retlw   0                  ; [m]


;;--------------------------------------------------------------------------------

ps2_hd_tx_parity

        btfsc   ps2_status,7    ; Skip if parity so far is even
        goto    ps2_hd_tx_0
        goto    ps2_hd_tx_1

ps2_hd_tx_bit

        rrf     ps2_data,F
        btfss   STATUS,C
        goto    ps2_hd_tx_0
        goto    ps2_hd_tx_1

ps2_hd_tx_0

        bcf     PS2_PORT,PS2_DATA

        BANKSEL PS2_TRIS
        bcf     PS2_TRIS,PS2_DATA ; Enable data output; data goes low
        BANKSEL PS2_PORT

        goto    ps2_hd_tx_delay

ps2_hd_tx_1

        movlw   b'10000000'
        xorwf   ps2_status,F    ; update parity bit (bit 7 of ps2_status)

        BANKSEL PS2_TRIS
        bsf     PS2_TRIS,PS2_DATA ; Disable data output; data goes high
        BANKSEL PS2_PORT

        goto    ps2_hd_tx_delay

ps2_hd_tx_delay

        btfss   PS2_PORT,PS2_CLOCK ; Wait for clock to go high
        goto    $-1

        btfsc   PS2_PORT,PS2_CLOCK ; Wait for clock to go low
        goto    $-1

        movlw   PS2_QPERIOD   ; Quarter of a period
        movwf   ps2_temp
        decfsz  ps2_temp,F
        goto    $-1

        return

ps2_hd_rx_ack

        btfsc   PS2_PORT,PS2_DATA ; Skip if data is low
        bsf     ps2_status,4

        btfss   PS2_PORT,PS2_CLOCK ; Wait for clock to go high
        goto    $-1

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine for PS/2 host byte reception
;;
;; Parameters:  ps2_data (out)  - data byte
;;              ps2_status(out) - status byte
;;                                bit 1 = 0 if host to device, 1 if device to host
;;                                bit 3 = 0 if parity OK, 1 if parity bad
;;
;; RAM Bank:    any (in) PS2_PORT (out)
;;
;;--------------------------------------------------------------------------------

ps2_host_rx_byte

        BANKSEL PS2_PORT        ; Select RAM bank

        clrf    ps2_data

        clrf    ps2_status
        bsf     ps2_status,1

        btfsc   PS2_PORT,PS2_CLOCK ; Wait for clock to go low
        goto    $-1


;; Timing analysis (sample data on falling edge of clock)
;;
;;          |start|  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |parit|stop
;;
;;    ________.  .__.  .__.  .__.  .__.  .__.  .__.  .__.  .__.  .__.  .__.  .__
;;            |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|
;;    ______       _____ _____ _____ _____ _____ _____ _____ _____ _____ _______
;;          \_____X_____X_____X_____X_____X_____X_____X_____X_____X_____X
;;
;;            ^     ^     ^     ^     ^     ^     ^     ^     ^     ^     ^  ^
;;           [a]   [b]   [c]   [d]   [e]   [f]   [g]   [h]   [i]   [j]   [k][l]
;;


        ;; Start bit
        call    ps2_dh_rx_delay    ; [a]

        ;; Bit 0
        call    ps2_dh_rx_bit      ; [b]

        ;; Bit 1
        call    ps2_dh_rx_bit      ; [c]

        ;; Bit 2
        call    ps2_dh_rx_bit      ; [d]

        ;; Bit 3
        call    ps2_dh_rx_bit      ; [e]

        ;; Bit 4
        call    ps2_dh_rx_bit      ; [f]

        ;; Bit 5
        call    ps2_dh_rx_bit      ; [g]

        ;; Bit 6
        call    ps2_dh_rx_bit      ; [h]

        ;; Bit 7
        call    ps2_dh_rx_bit      ; [i]

        ;; Parity bit
        call    ps2_dh_rx_parity   ; [j]

        ;; Stop bit
        call    ps2_dh_rx_stop     ; [k]

        retlw   0                  ; [l]


;;--------------------------------------------------------------------------------

ps2_dh_rx_parity

        clrw
        btfsc   PS2_PORT,PS2_DATA ; Capture data
        movlw   b'10000000'
        xorwf   ps2_status,F    ; update parity bit (bit 7 of ps2_status)
        btfss   ps2_status,7    ; Skip if parity bit is 1 (no error)
        bsf     ps2_status,3    ; parity error
        goto    ps2_dh_rx_delay

ps2_dh_rx_bit

        bcf     STATUS,C
        btfsc   PS2_PORT,PS2_DATA ; Capture data
        bsf     STATUS,C
        rrf     ps2_data,F

        movlw   b'10000000'
        andwf   ps2_data,W
        xorwf   ps2_status,F    ; update parity bit (bit 7 of ps2_status)

ps2_dh_rx_delay

        btfss   PS2_PORT,PS2_CLOCK ; Wait for clock to go high
        goto    $-1

        btfsc   PS2_PORT,PS2_CLOCK ; Wait for clock to go low
        goto    $-1

        return

ps2_dh_rx_stop

        btfss   PS2_PORT,PS2_CLOCK ; Wait for clock to go high
        goto    $-1

        return


;;--------------------------------------------------------------------------------

;; Re-enable messages and warnings

        ERRORLEVEL      0


;;--------------------------------------------------------------------------------
