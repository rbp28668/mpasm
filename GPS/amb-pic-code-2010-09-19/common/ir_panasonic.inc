;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/ir_panasonic.inc,v 1.9 2010/09/19 09:44:50 amb Exp $
;;
;; Infra-Red functions for Panasonic TV.
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2006,07,08,10 Andrew M. Bishop
;; It may be distributed under the GNU Public License, version 2, or
;; any higher version.  See section COPYING of the GNU Public license
;; for conditions under which this file may be redistributed.
;;
;;--------------------------------------------------------------------------------

;;--------------------------------------------------------------------------------
;;
;; Required Variables / Constants for Infra-Red functions
;;
;; Constants:   CLOCK           - The PIC clock speed in Hz
;;              IR_PORT         - Selects the port containing the RXD/TXD bits
;;              IR_RXD          - The bit to use for RX data input
;;              IR_TXD          - The bit to use for TX data input
;;              IR_NBYTES       - The number of bytes to use for Tx/Rx data
;;
;; Variables:   ir_byte_1       - Stores LSByte of Rx/Tx data, same bank as IR_PORT
;;              ir_byte_2       - Stores a Byte of Rx/Tx data, same bank as IR_PORT
;;              ir_byte_3       - Stores a Byte of Rx/Tx data, same bank as IR_PORT
;;              ir_byte_4       - Stores a Byte of Rx/Tx data, same bank as IR_PORT
;;              ir_byte_5       - Stores a Byte of Rx/Tx data, same bank as IR_PORT
;;              ir_byte_6       - Stores a Byte of Rx/Tx data, same bank as IR_PORT
;;              ir_byte_7       - Stores a Byte of Rx/Tx data, same bank as IR_PORT
;;              ir_byte_8       - Stores MSByte of Rx/Tx data, same bank as IR_PORT
;;              ir_length       - Stores the length of the data in bits, same bank as IR_PORT
;;              ir_temp         - Temporary byte, same bank as IR_PORT
;;
;;--------------------------------------------------------------------------------


;;--------------------------------------------------------------------------------
;;
;; Subroutine to receive an Infra-Red command in unknown format (Panasonic TV)
;;
;; Parameters:  ir_length (out) - Number of bits detected
;;              ir_byte* (out)  - Received data
;;
;; RAM Bank:    any (in) IR_PORT (out)
;;
;;--------------------------------------------------------------------------------

        IFDEF   IR_RXD

;; Start pulse: active 3600 us (138 cycles), inactive 1600 us (62 cycles)
;; Zero:        active 420 us (16 cycles), inactive 420 us (16 cycles)
;; One:         active 420 us (16 cycles), inactive 1260 us (48 cycles)
;; Stop pulse:  active 420 us (16 cycles)
;; Endianness:  LSB first
;;
;;    |    start    |     |    0    |     |        1     |     |  stop   |
;; ___         _____            ____            _________            _____
;;    \_______/     \ ... \____/    \ ... \____/         \ ... \____/     
;;
;;    ^       ^     ^       ^  ^      ^     ^  ^      ^                     ^
;;   [a]     [b]   [c]     [p][q]    [r]   [t][u]    [v]                   [x]
;;
;; Note: waveform is inverted due to TSOP1738 - high = no signal, low = signal.

ir_panasonic_rx_data
        BANKSEL IR_PORT

        clrf    ir_length
        clrf    ir_byte1
        IF      IR_NBYTES>1
        clrf    ir_byte2
        ENDIF
        IF      IR_NBYTES>2
        clrf    ir_byte3
        ENDIF
        IF      IR_NBYTES>3
        clrf    ir_byte4
        ENDIF
        IF      IR_NBYTES>4
        clrf    ir_byte5
        ENDIF
        IF      IR_NBYTES>5
        clrf    ir_byte6
        ENDIF
        IF      IR_NBYTES>6
        clrf    ir_byte7
        ENDIF
        IF      IR_NBYTES>7
        clrf    ir_byte8
        ENDIF

        ;; start bit

ir_panasonic_rx_start_bit
        btfsc   IR_PORT,IR_RXD  ; [a]
        goto    $-1

        movlw   100
        call    delay_us        ; 100 us noise filter

        btfsc   IR_PORT,IR_RXD
        goto    ir_panasonic_rx_start_bit

        movlw   100
        call    delay_us        ; 100 us noise filter

        btfsc   IR_PORT,IR_RXD
        goto    ir_panasonic_rx_start_bit

        movlw   100
        call    delay_us        ; 100 us noise filter

        btfsc   IR_PORT,IR_RXD
        goto    ir_panasonic_rx_start_bit

        ;; wait for rising edge
        btfss   IR_PORT,IR_RXD  ; [b]
        goto    $-1

        ;; wait for falling edge
        btfsc   IR_PORT,IR_RXD  ; [c]
        goto    $-1

        ;; wait until halfway through first bit active period
        movlw   8               ; 8 * 26 us = 208 us
        bcf     STATUS,C
        call    ir_cycle_delay

        ;; data bit

ir_panasonic_rx_bit
        btfsc   IR_PORT,IR_RXD  ; [p] or [t] or [x]
        goto    ir_panasonic_rx_stop_bit

        ;; wait for rising edge
        btfss   IR_PORT,IR_RXD  ; [q] or [u]
        goto    $-1

        ;; wait until next bit active part if '0', this bit inactive part if '1'
        movlw   16+8            ; (16+8) * 26 us = 624 us
        bcf     STATUS,C
        call    ir_cycle_delay

        btfss   IR_PORT,IR_RXD  ; [r] or [v] or [x]
        goto    ir_panasonic_rx_0
        goto    ir_panasonic_rx_1

ir_panasonic_rx_0
        bcf     STATUS,C        ; [p] or [t]
        goto    ir_panasonic_rx_next_bit

ir_panasonic_rx_1
        ;; wait until halfway through next bit active period
        movlw   48-16           ; (48-16) * 26 us = 832 us
        bcf     STATUS,C
        call    ir_cycle_delay

        bsf     STATUS,C        ; [p] or [t]
        goto    ir_panasonic_rx_next_bit

ir_panasonic_rx_next_bit
        IF      IR_NBYTES>7
        rrf     ir_byte8,F
        ENDIF
        IF      IR_NBYTES>6
        rrf     ir_byte7,F
        ENDIF
        IF      IR_NBYTES>5
        rrf     ir_byte6,F
        ENDIF
        IF      IR_NBYTES>4
        rrf     ir_byte5,F
        ENDIF
        IF      IR_NBYTES>3
        rrf     ir_byte4,F
        ENDIF
        IF      IR_NBYTES>2
        rrf     ir_byte3,F
        ENDIF
        IF      IR_NBYTES>1
        rrf     ir_byte2,F
        ENDIF
        rrf     ir_byte1,F

        incf    ir_length,F
        movlw   IR_NBYTES*8+1
        subwf   ir_length,W
        btfss   STATUS,Z
        goto    ir_panasonic_rx_bit ; [p] or [t]
        goto    ir_panasonic_rx_exit

        ;; stop bit

ir_panasonic_rx_stop_bit
        decf    ir_length,F
        movlw   IR_NBYTES*8-1
        goto    $+2

ir_panasonic_rx_exit
        movlw   IR_NBYTES*8
        subwf   ir_length,W
        movwf   ir_temp

        btfsc   STATUS,C
        return
        btfsc   STATUS,Z
        return

        ;; rotate by IR_NBYTES*8-N bits
ir_panasonic_rx_align
        bcf     STATUS,C
        IF      IR_NBYTES>7
        rrf     ir_byte8,F
        ENDIF
        IF      IR_NBYTES>6
        rrf     ir_byte7,F
        ENDIF
        IF      IR_NBYTES>5
        rrf     ir_byte6,F
        ENDIF
        IF      IR_NBYTES>4
        rrf     ir_byte5,F
        ENDIF
        IF      IR_NBYTES>3
        rrf     ir_byte4,F
        ENDIF
        IF      IR_NBYTES>2
        rrf     ir_byte3,F
        ENDIF
        IF      IR_NBYTES>1
        rrf     ir_byte2,F
        ENDIF
        rrf     ir_byte1,F

        incfsz  ir_temp,F
        goto    ir_panasonic_rx_align
        return

        ENDIF ; IR_RXD


;;--------------------------------------------------------------------------------
;;
;; Subroutine to send an Infra-Red command in unknown format (Panasonic TV)
;;
;; Parameters:  ir_length (in)  - Number of bits to transmit
;;              ir_byte* (in)   - Transmit data
;;
;; RAM Bank:    any (in) IR_PORT (out)
;;
;;--------------------------------------------------------------------------------

        IFDEF   IR_TXD

;; Start pulse: active 3600 us (138 cycles), inactive 1600 us (62 cycles)
;; Zero:        active 420 us (16 cycles), inactive 420 us (16 cycles)
;; One:         active 420 us (16 cycles), inactive 1260 us (48 cycles)
;; Stop pulse:  active 420 us (16 cycles)
;; Endianness:  LSB first
;;
;;    |    start    |     |    0    |     |        1     |     |  stop   |
;; ___         _____            ____            _________            _____
;;    \_______/     \ ... \____/    \ ... \____/         \ ... \____/     
;;
;;    ^       ^     ^     ^    ^    ^     ^    ^         ^     ^    ^
;;   [a]     [b]   [c]   [p]  [q]  [r]   [t]  [u]       [v]   [x]  [y]
;;
;; Note: waveform is inverted for LED driver - high = no signal, low = signal.

ir_panasonic_tx_data
        BANKSEL IR_PORT

        ;; start bit

ir_panasonic_tx_start_bit
        movlw   138             ; 138 * 26 us = 3588 us
        bsf     STATUS,C        ; transmit
        call    ir_cycle_delay  ; [a]

        movlw   62              ; 62 * 26 us = 1612 us
        bcf     STATUS,C        ; no transmit
        call    ir_cycle_delay  ; [b]

        ;; data bit

ir_panasonic_tx_bit
        movlw   16              ; 16 * 26 us = 416 us
        bsf     STATUS,C        ; transmit
        call    ir_cycle_delay  ; [p] or [t]

        btfss   ir_byte1,0
        goto    ir_panasonic_tx_0
        goto    ir_panasonic_tx_1

ir_panasonic_tx_0
        movlw   16              ; 16 * 26 us = 208 us
        bcf     STATUS,C        ; no transmit
        call    ir_cycle_delay  ; [q]

        goto    ir_panasonic_tx_next_bit

ir_panasonic_tx_1
        movlw   48              ; 48 * 26 us = 1248 us
        bcf     STATUS,C        ; no transmit
        call    ir_cycle_delay  ; [u]

        goto    ir_panasonic_tx_next_bit

ir_panasonic_tx_next_bit
        IF      IR_NBYTES>7
        rrf     ir_byte8,F
        ENDIF
        IF      IR_NBYTES>6
        rrf     ir_byte7,F
        ENDIF
        IF      IR_NBYTES>5
        rrf     ir_byte6,F
        ENDIF
        IF      IR_NBYTES>4
        rrf     ir_byte5,F
        ENDIF
        IF      IR_NBYTES>3
        rrf     ir_byte4,F
        ENDIF
        IF      IR_NBYTES>2
        rrf     ir_byte3,F
        ENDIF
        IF      IR_NBYTES>1
        rrf     ir_byte2,F
        ENDIF
        rrf     ir_byte1,F

        decfsz  ir_length,F
        goto    ir_panasonic_tx_bit ; [r] or [v]

        ;; stop bit

ir_panasonic_tx_stop_bit
        movlw   16              ; 16 * 26 us = 416 us
        bsf     STATUS,C        ; transmit
        call    ir_cycle_delay  ; [x]

        return

        ENDIF ; IR_TXD

;;--------------------------------------------------------------------------------
