;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/rs232_rx.inc,v 1.4 2007/05/07 18:31:41 amb Exp $
;;
;; RS232 receive functions.
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2006 Andrew M. Bishop
;; It may be distributed under the GNU Public License, version 2, or
;; any higher version.  See section COPYING of the GNU Public license
;; for conditions under which this file may be redistributed.
;;
;;--------------------------------------------------------------------------------

;;--------------------------------------------------------------------------------
;;
;; Required Variables / Constants for rs232 functions
;;
;; Constants:   RS232_PORT      - Selects the port containing the RXD/TXD bits
;;              RS232_RXD       - The bit to use for RX data input
;;
;; Variables:   rs232_temp      - Stores temporary data, same bank as RS232_PORT
;;
;;--------------------------------------------------------------------------------


;;--------------------------------------------------------------------------------
;;
;; Subroutine to receive an RS232 serial byte
;; 8-bit / no parity / 1 stop bit
;;
;; Parameters:  w (out)         - Received byte
;;
;; RAM Bank:    any (in) RS232_PORT (out)
;;
;;--------------------------------------------------------------------------------

        IFDEF   RS232_RXD

;; Timing analysis
;;
;;          |start|  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |stop |
;;    ______       _____ _____ _____ _____ _____ _____ _____ ___________________
;;          \_____X_____X_____X_____X_____X_____X_____X_____X_____X
;;
;;         ^   ^     ^     ^     ^     ^     ^     ^     ^     ^     ^ 
;;         [a][b]   [c]   [d]   [e]   [f]   [g]   [h]   [i]   [j]   [k]
;;

rs232_rx_byte
        BANKSEL RS232_PORT

        clrf    rs232_temp

        ;; Automatic flow control (allow)

        IFDEF   RS232_RTS
        bcf     RS232_PORT,RS232_RTS
        ENDIF

rs232_rx_wait_high
        btfss   RS232_PORT,RS232_RXD
        goto    rs232_rx_wait_high

rs232_rx_wait_falling_edge
        btfsc   RS232_PORT,RS232_RXD
        goto    rs232_rx_wait_falling_edge

        ;; Delay half a bit period
        call    rs232_dly_hbit  ; 2 ins (A) [a]

        ;; Start bit
        call    rs232_dly_bit   ; 2 ins (A) [b]

        ;; Bit 0 (LSB)
        call    rs232_rx_bit    ; 2 ins (A) [c]

        ;; Bit 1
        call    rs232_rx_bit    ; 2 ins (A) [d]

        ;; Bit 2
        call    rs232_rx_bit    ; 2 ins (A) [e]

        ;; Bit 3
        call    rs232_rx_bit    ; 2 ins (A) [f]

        ;; Bit 4
        call    rs232_rx_bit    ; 2 ins (A) [g]

        ;; Bit 5
        call    rs232_rx_bit    ; 2 ins (A) [h]

        ;; Bit 6
        call    rs232_rx_bit    ; 2 ins (A) [i]

        ;; Bit 7 (MSB)
        call    rs232_rx_bit    ; 2 ins (A) [j]

        movf    rs232_temp,W

        ;; Automatic flow control (disallow)

        IFDEF   RS232_RTS
        bsf     RS232_PORT,RS232_RTS
        ENDIF

        return                  ;           [k]

;;--------------------------------------------------------------------------------

rs232_rx_bit
        btfss   RS232_PORT,RS232_RXD ; 1 ins / 2 ins (A)
        goto    rs232_rx_0      ; 2 ins (A)
        goto    rs232_rx_1      ; 2 ins (A)

rs232_rx_0
        nop                     ; 1 ins (A)
        rrf     rs232_temp,F    ; 1 ins (A)
        bcf     rs232_temp,7    ; 1 ins (A)
        goto    rs232_dly_bit   ; 2 ins (A)

rs232_rx_1
        rrf     rs232_temp,F    ; 1 ins (A)
        bsf     rs232_temp,7    ; 1 ins (A)
        goto    rs232_dly_bit   ; 2 ins (A)


        ENDIF

;;--------------------------------------------------------------------------------
