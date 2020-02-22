;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/ir_rc5.inc,v 1.10 2010/09/19 09:44:50 amb Exp $
;;
;; Infra-Red functions for devices using RC5 format.
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
;; Subroutine to receive an Infra-Red command in RC5 type format
;;
;; Parameters:  ir_length (out) - Number of bits detected
;;              ir_byte* (out)  - Received data
;;
;; RAM Bank:    any (in) IR_PORT (out)
;;
;;--------------------------------------------------------------------------------

        IFDEF   IR_RXD

;; Start pulse: active 890 us
;; Zero:        active 890 us, inactive 890 us
;; One:         inactive 890 us, active 890 us
;; Endianness:  MSB first
;;
;;          |  start |           |    0    |       |    1    |
;;    ______                           ____         ____      
;;          \________/      ...  X____/    X  ...  X    \____X
;;
;;         ^         ^   ^         ^    ^    ^       ^    ^    ^ 
;;         [a]      [b] [c]       [t]  [u]  [v]     [x]  [y]  [z]
;;
;; Note: waveform is inverted due to TSOP1738 - high = no signal, low = signal.

ir_rc5_rx_data
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

ir_rc5_rx_start_bit
        btfsc   IR_PORT,IR_RXD  ; [a]
        goto    $-1

        movlw   100
        call    delay_us        ; 100 us noise filter

        btfsc   IR_PORT,IR_RXD
        goto    ir_rc5_rx_start_bit

        movlw   100
        call    delay_us        ; 100 us noise filter

        btfsc   IR_PORT,IR_RXD
        goto    ir_rc5_rx_start_bit

        movlw   100
        call    delay_us        ; 100 us noise filter

        btfsc   IR_PORT,IR_RXD
        goto    ir_rc5_rx_start_bit

        ;; wait for rising edge
        btfss   IR_PORT,IR_RXD  ; [b]
        goto    $-1

        ;; Delay a quarter period
        movlw   10              ; 10 * 26 us = 260 us
        bcf     STATUS,C
        call    ir_cycle_delay  ; [c]

ir_rc5_rx_bit
        btfss   IR_PORT,IR_RXD  ; [t] or [x]
        goto    ir_rc5_rx_0
        goto    ir_rc5_rx_1

ir_rc5_rx_0
        movlw   34              ; 34 * 26 us = 884 us
        bcf     STATUS,C
        call    ir_cycle_delay

        btfss   IR_PORT,IR_RXD  ; [u]
        return
        bcf     STATUS,C
        goto    ir_rc5_rx_next_bit

ir_rc5_rx_1
        movlw   34              ; 34 * 26 us = 884 us
        bcf     STATUS,C
        call    ir_cycle_delay

        btfsc   IR_PORT,IR_RXD  ; [y]
        return
        bsf     STATUS,C
        goto    ir_rc5_rx_next_bit

ir_rc5_rx_next_bit
        rlf     ir_byte1,F
        IF      IR_NBYTES>1
        rlf     ir_byte2,F
        ENDIF
        IF      IR_NBYTES>2
        rlf     ir_byte3,F
        ENDIF
        IF      IR_NBYTES>3
        rlf     ir_byte4,F
        ENDIF
        IF      IR_NBYTES>4
        rlf     ir_byte5,F
        ENDIF
        IF      IR_NBYTES>5
        rlf     ir_byte6,F
        ENDIF
        IF      IR_NBYTES>6
        rlf     ir_byte7,F
        ENDIF
        IF      IR_NBYTES>7
        rlf     ir_byte8,F
        ENDIF

        movlw   34              ; 34 * 26 us = 884 us
        bcf     STATUS,C
        call    ir_cycle_delay

        incf    ir_length,F
        movlw   IR_NBYTES*8+1
        subwf   ir_length,W
        btfss   STATUS,Z
        goto    ir_rc5_rx_bit   ; [v] or [z]
        return


        ENDIF ; IR_RXD


;;--------------------------------------------------------------------------------
;;
;; Subroutine to send an Infra-Red command in RC5 type format
;;
;; Parameters:  ir_length (in)  - Number of bits to transmit
;;              ir_byte* (in)   - Transmit data
;;
;; RAM Bank:    any (in) IR_PORT (out)
;;
;;--------------------------------------------------------------------------------

        IFDEF   IR_TXD

;; Start pulse: active 890 us.
;; Zero:        active 890 us, inactive 890 us
;; One:         inactive 890 us, active 890 us
;; Endianness:  MSB first
;;
;;          |  start |       |    0    |       |    1    |
;;    ______                       ____         ____      
;;          \________/  ...  X____/    X  ...  X    \____X
;;
;;         ^         ^       ^    ^    ^       ^    ^    ^ 
;;         [a]      [b]     [t]  [u]  [v]     [x]  [y]  [z]
;;
;; Note: waveform is inverted for LED driver - high = no signal, low = signal.

ir_rc5_tx_data
        BANKSEL IR_PORT

        ;; rotate by IR_NBYTES*8-N bits

        movlw   IR_NBYTES*8
        subwf   ir_length,W
        movwf   ir_temp

        btfsc   STATUS,Z
        goto    ir_rc5_tx_start

ir_rc5_tx_align
        bcf     STATUS,C
        rlf     ir_byte1,F
        IF      IR_NBYTES>1
        rlf     ir_byte2,F
        ENDIF
        IF      IR_NBYTES>2
        rlf     ir_byte3,F
        ENDIF
        IF      IR_NBYTES>3
        rlf     ir_byte4,F
        ENDIF
        IF      IR_NBYTES>4
        rlf     ir_byte5,F
        ENDIF
        IF      IR_NBYTES>5
        rlf     ir_byte6,F
        ENDIF
        IF      IR_NBYTES>6
        rlf     ir_byte7,F
        ENDIF
        IF      IR_NBYTES>7
        rlf     ir_byte8,F
        ENDIF

        incfsz  ir_temp,F
        goto    ir_rc5_tx_align

ir_rc5_tx_start
        ;; start bit

        movlw   34              ; 33 * 26 us = 884 us
        bsf     STATUS,C        ; transmit
        call    ir_cycle_delay  ; [a]

ir_rc5_tx_bit
        IF      IR_NBYTES==8
        btfss   ir_byte8,7      ; [t] or [x]
        ENDIF
        IF      IR_NBYTES==7
        btfss   ir_byte7,7      ; [t] or [x]
        ENDIF
        IF      IR_NBYTES==6
        btfss   ir_byte6,7      ; [t] or [x]
        ENDIF
        IF      IR_NBYTES==5
        btfss   ir_byte5,7      ; [t] or [x]
        ENDIF
        IF      IR_NBYTES==4
        btfss   ir_byte4,7      ; [t] or [x]
        ENDIF
        IF      IR_NBYTES==3
        btfss   ir_byte3,7      ; [t] or [x]
        ENDIF
        IF      IR_NBYTES==2
        btfss   ir_byte2,7      ; [t] or [x]
        ENDIF
        IF      IR_NBYTES==1
        btfss   ir_byte1,7      ; [t] or [x]
        ENDIF
        goto    ir_rc5_tx_0
        goto    ir_rc5_tx_1

ir_rc5_tx_0
        movlw   34              ; 34 * 26 us = 884 us
        bsf     STATUS,C        ; transmit
        call    ir_cycle_delay  ; [t]

        movlw   34              ; 34 * 26 us = 884 us
        bcf     STATUS,C        ; no transmit
        call    ir_cycle_delay  ; [u]

        goto    ir_rc5_tx_next_bit

ir_rc5_tx_1
        movlw   34              ; 34 * 26 us = 884 us
        bcf     STATUS,C        ; no transmit
        call    ir_cycle_delay  ; [x]

        movlw   34              ; 34 * 26 us = 884 us
        bsf     STATUS,C        ; transmit
        call    ir_cycle_delay  ; [y]

        goto    ir_rc5_tx_next_bit

ir_rc5_tx_next_bit
        rlf     ir_byte1,F
        IF      IR_NBYTES>1
        rlf     ir_byte2,F
        ENDIF
        IF      IR_NBYTES>2
        rlf     ir_byte3,F
        ENDIF
        IF      IR_NBYTES>3
        rlf     ir_byte4,F
        ENDIF
        IF      IR_NBYTES>4
        rlf     ir_byte5,F
        ENDIF
        IF      IR_NBYTES>5
        rlf     ir_byte6,F
        ENDIF
        IF      IR_NBYTES>6
        rlf     ir_byte7,F
        ENDIF
        IF      IR_NBYTES>7
        rlf     ir_byte8,F
        ENDIF

        decfsz  ir_length,F
        goto    ir_rc5_tx_bit   ; [v] or [z]
        return

        ENDIF ; IR_TXD


;;--------------------------------------------------------------------------------
