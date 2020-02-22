;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/ir_cable.inc,v 1.9 2010/09/19 09:44:50 amb Exp $
;;
;; Infra-Red functions for cable TV box.
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2006,08,10 Andrew M. Bishop
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
;; Subroutine to receive an Infra-Red command in General Instrument CFT-2100 cable TV box format.
;;
;; Parameters:  ir_length (out) - Number of bits detected
;;              ir_byte* (out)  - Received data
;;
;; RAM Bank:    any (in) IR_PORT (out)
;;
;;--------------------------------------------------------------------------------

        IFDEF   IR_RXD

;; Start pulse: active 9282 us (357 cycles), inactive 4342 us (167 cycles)
;; Zero:        active 676 us (26 cycles), inactive 2080 us (80 cycles)
;; One:         active 676 us (26 cycles), inactive 4342 us (167 cycles)
;; Stop pulse:  active 676 us (26 cycles)
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

ir_cable_rx_data
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

ir_cable_rx_start_bit
        btfsc   IR_PORT,IR_RXD  ; [a]
        goto    $-1

        movlw   100
        call    delay_us        ; 100 us noise filter

        btfsc   IR_PORT,IR_RXD
        goto    ir_cable_rx_start_bit

        movlw   100
        call    delay_us        ; 100 us noise filter

        btfsc   IR_PORT,IR_RXD
        goto    ir_cable_rx_start_bit

        movlw   100
        call    delay_us        ; 100 us noise filter

        btfsc   IR_PORT,IR_RXD
        goto    ir_cable_rx_start_bit

        ;; wait for rising edge
        btfss   IR_PORT,IR_RXD  ; [b]
        goto    $-1

        ;; wait for falling edge
        btfsc   IR_PORT,IR_RXD  ; [c]
        goto    $-1

        ;; wait until halfway through first bit active period
        movlw   13              ; 13 * 26 us = 338 us
        bcf     STATUS,C
        call    ir_cycle_delay

        ;; data bit

ir_cable_rx_bit
        btfsc   IR_PORT,IR_RXD  ; [p] or [t] or [x]
        goto    ir_cable_rx_stop_bit

        ;; wait for rising edge
        btfss   IR_PORT,IR_RXD  ; [q] or [u]
        goto    $-1

        ;; wait until next bit active part if '0', this bit inactive part if '1'
        movlw   80+13           ; (80+13) * 26 us = 2418 us
        bcf     STATUS,C
        call    ir_cycle_delay

        btfss   IR_PORT,IR_RXD  ; [r] or [v] or [x]
        goto    ir_cable_rx_0
        goto    ir_cable_rx_1

ir_cable_rx_0
        bcf     STATUS,C        ; [p] or [t]
        goto    ir_cable_rx_next_bit

ir_cable_rx_1
        ;; wait until halfway through next bit active period
        movlw   167-80          ; (167-80) * 26 us = 2262 us
        bcf     STATUS,C
        call    ir_cycle_delay

        bsf     STATUS,C        ; [p] or [t]
        goto    ir_cable_rx_next_bit

ir_cable_rx_next_bit
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
        goto    ir_cable_rx_bit ; [p] or [t]
        goto    ir_cable_rx_exit

        ;; stop bit

ir_cable_rx_stop_bit
        decf    ir_length,F
        movlw   IR_NBYTES*8-1
        goto    $+2

ir_cable_rx_exit
        movlw   IR_NBYTES*8
        subwf   ir_length,W
        movwf   ir_temp

        btfsc   STATUS,C
        return
        btfsc   STATUS,Z
        return

        ;; rotate by IR_NBYTES*8-N bits
ir_cable_rx_align
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
        goto    ir_cable_rx_align
        return

        ENDIF ; IR_RXD


;;--------------------------------------------------------------------------------
;;
;; Subroutine to send an Infra-Red command in General Instrument CFT-2100 cable TV box format
;;
;; Parameters:  ir_length (in)  - Number of bits to transmit
;;              ir_byte* (in)   - Transmit data
;;
;; RAM Bank:    any (in) IR_PORT (out)
;;
;;--------------------------------------------------------------------------------

        IFDEF   IR_TXD

;; Start pulse: active 9282 us (357 cycles), inactive 4342 us (167 cycles)
;; Zero:        active 676 us (26 cycles), inactive 2080 us (80 cycles)
;; One:         active 676 us (26 cycles), inactive 4342 us (167 cycles)
;; Stop pulse:  active 676 us (26 cycles)
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

ir_cable_tx_data
        BANKSEL IR_PORT

        ;; start bit

ir_cable_tx_start_bit
        movlw   179             ; 179 * 26 us = 4654 us
        bsf     STATUS,C        ; transmit
        call    ir_cycle_delay  ; [a]

        movlw   178             ; 178 * 26 us = 4628 us
        bsf     STATUS,C        ; transmit
        call    ir_cycle_delay

        movlw   167             ; 167 * 26 us = 4342 us
        bcf     STATUS,C        ; no transmit
        call    ir_cycle_delay  ; [b]

        ;; data bit

ir_cable_tx_bit
        movlw   26              ; 26 * 26 us = 676 us
        bsf     STATUS,C        ; transmit
        call    ir_cycle_delay  ; [p] or [t]

        btfss   ir_byte1,0
        goto    ir_cable_tx_0
        goto    ir_cable_tx_1

ir_cable_tx_0
        movlw   80              ; 80 * 26 us = 2080 us
        bcf     STATUS,C        ; no transmit
        call    ir_cycle_delay  ; [q]

        goto    ir_cable_tx_next_bit

ir_cable_tx_1
        movlw   167             ; 167 * 26 us = 4342 us
        bcf     STATUS,C        ; no transmit
        call    ir_cycle_delay  ; [u]

        goto    ir_cable_tx_next_bit

ir_cable_tx_next_bit
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
        goto    ir_cable_tx_bit ; [r] or [v]

        ;; stop bit

ir_cable_tx_stop_bit
        movlw   26              ; 26 * 26 us = 676 us
        bsf     STATUS,C        ; transmit
        call    ir_cycle_delay  ; [x]

        return

        ENDIF ; IR_TXD

;;--------------------------------------------------------------------------------