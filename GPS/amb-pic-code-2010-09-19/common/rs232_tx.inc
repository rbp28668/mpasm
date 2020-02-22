;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/rs232_tx.inc,v 1.5 2010/09/19 09:56:14 amb Exp $
;;
;; RS232 transmit functions.
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2006, 2010 Andrew M. Bishop
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
;;              RS232_TXD       - The bit to use for TX data output
;;
;; Variables:   rs232_temp      - Stores temporary data, same bank as RS232_PORT
;;
;;--------------------------------------------------------------------------------


;;--------------------------------------------------------------------------------
;;
;; Subroutine to transmit an RS232 serial byte
;; 8-bit / no parity / 1 stop bit
;;
;; Parameters:  w (in)          - Byte to transmit
;;
;; RAM Bank:    any (in) RS232_PORT (out)
;;
;;--------------------------------------------------------------------------------

        IFDEF   RS232_TXD

;; Timing analysis
;;
;;          |start|  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |stop |
;;    ______       _____ _____ _____ _____ _____ _____ _____ ___________________
;;          \_____X_____X_____X_____X_____X_____X_____X_____X_____X
;;
;;          ^     ^     ^     ^     ^     ^     ^     ^     ^     ^     ^     ^
;;         [a]   [b]   [c]   [d]   [e]   [f]   [g]   [h]   [i]   [j]   [k]   [l]
;;

rs232_tx_byte
        BANKSEL RS232_PORT

        movwf   rs232_temp

        ;; Start bit
        call    rs232_tx_0      ; 2 ins     [a]

        ;; Bit 0 (LSB)
        call    rs232_tx_bit    ; 2 ins (A) [b-]

        ;; Bit 1
        call    rs232_tx_bit    ; 2 ins (A) [c-]

        ;; Bit 2
        call    rs232_tx_bit    ; 2 ins (A) [d-]

        ;; Bit 3
        call    rs232_tx_bit    ; 2 ins (A) [e-]

        ;; Bit 4
        call    rs232_tx_bit    ; 2 ins (A) [f-]

        ;; Bit 5
        call    rs232_tx_bit    ; 2 ins (A) [g-]

        ;; Bit 6
        call    rs232_tx_bit    ; 2 ins (A) [h-]

        ;; Bit 7 (MSB)
        call    rs232_tx_bit    ; 2 ins (A) [i-]

        ;; Stop bit
        call    $+1             ; 2 ins (A) [j-]
        call    $+1             ; 2 ins (A)
        nop                     ; 1 ins (A)
        call    rs232_tx_1      ; 2 ins (A) [j]

        ;; Extra (almost) bit delay to be safe
        call    rs232_dly_bit   ;           [k-]

        return                  ;           [l-]

rs232_tx_bit
        rrf     rs232_temp,F    ; 1 ins (A)
        btfss   STATUS,C        ; 1 ins / 2 ins (A)
        goto    rs232_tx_0      ; 2 ins (A)
        goto    rs232_tx_1      ; 2 ins (A)

rs232_tx_0
        nop                     ; 1 ins (A)
        bcf     RS232_PORT,RS232_TXD ; 1 ins (A)
        goto    rs232_dly_bit   ; 2 ins (A)

rs232_tx_1
        bsf     RS232_PORT,RS232_TXD ; 1 ins (A)
        goto    rs232_dly_bit   ; 2 ins (A)


        ENDIF

;;--------------------------------------------------------------------------------
