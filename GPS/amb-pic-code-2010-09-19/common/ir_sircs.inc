;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/ir_sircs.inc,v 1.8 2010/09/19 09:44:50 amb Exp $
;;
;; Infra-Red functions for devices using SIRCS format.
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
;; Subroutine to receive an Infra-Red command in SIRCS format.
;;
;; Parameters:  ir_length (out) - Number of bits detected
;;              ir_byte* (out)  - Received data
;;
;; RAM Bank:    any (in) IR_PORT (out)
;;
;;--------------------------------------------------------------------------------

        IFDEF   IR_RXD

;; Start pulse: active 2400 us (92 cycles), inactive 600 us (23 cycles)
;; Zero:        active 600 us (23 cycles), inactive 600 us (23 cycles)
;; One:         active 1200 us (46 cycles), inactive 600 us (23 cycles)
;; Endianness:  LSB first
;;
;;          |    start    |       |    0    |       |        1      |
;;    ______          ____              ____                    ____ 
;;          \________/    X  ...  X____/    X  ...  X__________/    X
;;
;;         ^         ^    ^         ^    ^    ^       ^    ^     ^    ^ 
;;         [a]      [b]  [c]       [t]  [u]  [v]     [w]  [x]   [y]  [z]
;;
;; Note: waveform is inverted due to TSOP1738 - high = no signal, low = signal.

ir_sircs_rx_data
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

ir_sircs_rx_start_bit
        btfsc   IR_PORT,IR_RXD  ; [a]
        goto    $-1

        movlw   100
        call    delay_us        ; 100 us noise filter

        btfsc   IR_PORT,IR_RXD
        goto    ir_sircs_rx_start_bit

        movlw   100
        call    delay_us        ; 100 us noise filter

        btfsc   IR_PORT,IR_RXD
        goto    ir_sircs_rx_start_bit

        movlw   100
        call    delay_us        ; 100 us noise filter

        btfsc   IR_PORT,IR_RXD
        goto    ir_sircs_rx_start_bit

        ;; wait for rising edge
        btfss   IR_PORT,IR_RXD  ; [b]
        goto    $-1

        movlw   28              ; 28 * 26 us = 728 us
        bcf     STATUS,C
        call    ir_cycle_delay

ir_sircs_rx_bit
        btfsc   IR_PORT,IR_RXD  ; [t] or [w]
        goto    ir_sircs_rx_exit

        movlw   23              ; 23 * 26 us = 598 us
        bcf     STATUS,C
        call    ir_cycle_delay  ; [t] or [w]

        btfsc   IR_PORT,IR_RXD  ; [u] or [x]
        goto    ir_sircs_rx_0
        goto    ir_sircs_rx_1

ir_sircs_rx_0
        bcf     STATUS,C
        goto    ir_sircs_rx_next_bit

ir_sircs_rx_1
        btfss   IR_PORT,IR_RXD
        goto    $-1

        movlw   5               ; 5 * 26 us = 130 us
        bcf     STATUS,C
        call    ir_cycle_delay

        btfss   IR_PORT,IR_RXD  ; [y]
        goto    ir_sircs_rx_exit

        bsf     STATUS,C
        goto    ir_sircs_rx_next_bit

ir_sircs_rx_next_bit
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

        movlw   23              ; 23 * 26 us = 598 us
        bcf     STATUS,C
        call    ir_cycle_delay  ; [u] or [y]

        incf    ir_length,F
        movlw   IR_NBYTES*8+1
        subwf   ir_length,W
        btfss   STATUS,Z
        goto    ir_sircs_rx_bit ; [v] or [z]

ir_sircs_rx_exit
        ;; rotate by IR_NBYTES*8-N bits

        movlw   IR_NBYTES*8
        subwf   ir_length,W
        movwf   ir_temp

        btfsc   STATUS,Z
        return
        btfsc   STATUS,C
        return

ir_sircs_rx_align
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
        goto    ir_sircs_rx_align
        return

        ENDIF ; IR_RXD


;;--------------------------------------------------------------------------------
;;
;; Subroutine to send an Infra-Red command in SIRCS format
;;
;; Parameters:  ir_length (in)  - Number of bits to transmit
;;              ir_byte* (in)   - Transmit data
;;
;; RAM Bank:    any (in) IR_PORT (out)
;;
;;--------------------------------------------------------------------------------

        IFDEF   IR_TXD

;; Start pulse: active 2400 us (92 cycles), inactive 600 us (23 cycles)
;; Zero:        active 600 us (23 cycles), inactive 600 us (23 cycles)
;; One:         active 1200 us (46 cycles), inactive 600 us (23 cycles)
;; Endianness:  LSB first
;;
;;          |    start    |       |    0    |       |        1      |
;;    ______          ____              ____                    ____ 
;;          \________/    X  ...  X____/    X  ...  X__________/    X
;;
;;         ^         ^    ^       ^    ^    ^       ^          ^    ^ 
;;         [a]      [b]  [c]     [t]  [u]  [v]     [x]        [y]  [z]
;;
;; Note: waveform is inverted for LED driver - high = no signal, low = signal.

ir_sircs_tx_data
        BANKSEL IR_PORT

        ;; start bit

ir_sircs_tx_start_bit
        movlw   98              ; 98 * 26 us = 2548 us
        bsf     STATUS,C        ; transmit
        call    ir_cycle_delay  ; [a]

        movlw   19              ; 19 * 26 us = 494 us
        bcf     STATUS,C        ; no transmit
        call    ir_cycle_delay  ; [b]

ir_sircs_tx_bit
        btfss   ir_byte1,0      ; [t] or [x]
        goto    ir_sircs_tx_0
        goto    ir_sircs_tx_1

ir_sircs_tx_0
        movlw   27              ; 27 * 26 us = 702 us
        bsf     STATUS,C        ; transmit
        call    ir_cycle_delay  ; [t]

        goto    ir_sircs_tx_next_bit

ir_sircs_tx_1
        movlw   50              ; 50 * 26 us = 1300 us
        bsf     STATUS,C        ; transmit
        call    ir_cycle_delay  ; [x]

        goto    ir_sircs_tx_next_bit

ir_sircs_tx_next_bit
        movlw   19              ; 19 * 26 us = 494 us
        bcf     STATUS,C        ; no transmit
        call    ir_cycle_delay  ; [u] or [y]

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
        goto    ir_sircs_tx_bit ; [v] or [z]
        return

        ENDIF ; IR_TXD

;;--------------------------------------------------------------------------------
