;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/i2c_ssp_master.inc,v 1.3 2010/09/19 09:43:38 amb Exp $
;;
;; I2C master support using SSP module.
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2006,07,10 Andrew M. Bishop
;; It may be distributed under the GNU Public License, version 2, or
;; any higher version.  See section COPYING of the GNU Public license
;; for conditions under which this file may be redistributed.
;;
;;--------------------------------------------------------------------------------

;;--------------------------------------------------------------------------------
;;
;; Required Variables / Constants for I2C functions
;;
;; Constants:   CLOCK           - The PIC clock speed in Hz
;;              SSP_PORT        - Selects the port containing the SSP port (automatically set)
;;              SSP_TRIS        - Selects the tristate register for the SSP port (automatically set)
;;              SSP_SDA         - The bit to use for SDA data in/out (automatically set)
;;              SSP_SCL         - The bit to use for SCL clock in/out (automatically set)
;;
;; Variables:   i2c_temp        - Stores temporary data, same bank(s) as I2C_PORT and I2C_TRIS
;;
;;--------------------------------------------------------------------------------

;;
;; Rules for calling these functions from a program:
;; 1) The timing works for the following sequence (extra delays should be ok):
;;
;;   call  i2c_ssp_master_start_bit
;;   movlw address
;;   call  i2c_ssp_master_tx_byte
;;   movlw byte
;;   call  i2c_ssp_master_tx_byte
;;   call  i2c_ssp_stop_bit
;;


;;--------------------------------------------------------------------------------

;; Temporarily disable messages and warnings

        ERRORLEVEL      2


;;--------------------------------------------------------------------------------
;;
;; Subroutine to transmit a start bit (various versions for different clock speeds)
;;
;; RAM Bank:    any (in) SSP_TRIS (out)
;;
;;--------------------------------------------------------------------------------

        IF CLOCK==4000000

;; Timing - 11 cycles with 4 MHz clock => 90.91 kHz bit rate
;;       ____|_____________        |       
;;  SDA      |             \_______|_______
;;       ____|_____________________|_______
;;  SCL      |                     |       
;;
;; Time  0  1  2  3  4  5  6  7  8  9 10  0

i2c_ssp_master_tx_start
        BANKSEL SSPSTAT
        btfsc   SSPSTAT,S       ; skip if start bit was not last
        goto    $-1

        BANKSEL SSP_TRIS
        bsf     SSP_TRIS,SSP_SDA; Set SDA tri-state
        bsf     SSP_TRIS,SSP_SCL; Set SCL tri-state

        bcf     SSP_TRIS,SSP_SDA; 1 ins (6)
        return                  ; 2 ins (7 = -4)

        ENDIF ; CLOCK==4000000

;;--------------------------------------------------------------------------------

        IF CLOCK==8000000

;; Timing - 20 cycles with 8 MHz clock => 100 kHz bit rate
;;       ____|_______________                           |             
;;  SDA      |               \__________________________|_____________
;;       ____|__________________________________________|_____________
;;  SCL      |                                          |             
;;
;; Time  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19  0

i2c_ssp_master_tx_start
        BANKSEL SSPSTAT
        btfsc   SSPSTAT,S       ; skip if start bit was not last
        goto    $-1

        BANKSEL SSP_TRIS
        bsf     SSP_TRIS,SSP_SDA; Set SDA tri-state
        bsf     SSP_TRIS,SSP_SCL; Set SCL tri-state

        bcf     SSP_TRIS,SSP_SDA; 1 ins (7)
        call    delay_6_cycles  ; 6 ins (8)
        return                  ; 2 ins (14 = -6)

        ENDIF ; CLOCK==8000000


;;--------------------------------------------------------------------------------
;;
;; Subroutine for a master transmitting an I2C serial byte
;;
;; Parameters:  w (in)          - Data byte to transmit
;;              w (out)         - Zero for ACK, 1 for NACK
;;
;; RAM Bank:    SSP_TRIS (in) SSP_TRIS (out)
;;
;;--------------------------------------------------------------------------------

i2c_ssp_master_tx_byte
        movwf   i2c_temp             ; 1 ins (-1) (1 for 4 MHz, too slow)

        ;; Bit 7 (MSB)
        call    i2c_ssp_master_tx_bit; 2 ins (0)

        ;; Bit 6
        call    i2c_ssp_master_tx_bit; 2 ins (0)

        ;; Bit 5
        call    i2c_ssp_master_tx_bit; 2 ins (0)

        ;; Bit 4
        call    i2c_ssp_master_tx_bit; 2 ins (0)

        ;; Bit 3
        call    i2c_ssp_master_tx_bit; 2 ins (0)

        ;; Bit 2
        call    i2c_ssp_master_tx_bit; 2 ins (0)

        ;; Bit 1
        call    i2c_ssp_master_tx_bit; 2 ins (0)

        ;; Bit 0 (LSB)
        call    i2c_ssp_master_tx_bit; 2 ins (0)

        ;; Send a clock for ACK
        call    i2c_ssp_master_tx_clock_ack; 2 ins (0)

        BANKSEL SSP_PORT             ; 1 ins (0)
        btfss   SSP_PORT,SSP_SDA     ; 1 ins / 2 ins (1)
        goto    i2c_ssp_master_tx_got_ack ; 2 ins (2)

i2c_ssp_master_tx_got_nack
        BANKSEL SSP_TRIS             ; 1 ins (3)
        retlw   1                    ; 2 ins (4)

i2c_ssp_master_tx_got_ack
        BANKSEL SSP_TRIS             ; 1 ins (4)
        retlw   0                    ; 2 ins (5)


;;--------------------------------------------------------------------------------
;;
;; Subroutine to transmit a bit (various versions for different clock speeds)
;;
;; RAM Bank:    SSP_TRIS (in) SSP_TRIS (out)
;;
;;--------------------------------------------------------------------------------

        IF CLOCK==4000000

;; Timing - 11 cycles with 4 MHz clock => 90.91 kHz bit rate
;;       ____|_____________ _____________|_
;;  SDA  ____|_____________X_____________|_
;;       ____|_                   _______|_
;;  SCL      | \_________________/       | 
;;
;; Time  0  1  2  3  4  5  6  7  8  9 10  0

i2c_ssp_master_tx_bit
        bcf     SSP_TRIS,SSP_SCL    ; 1 ins (2)
        btfsc   i2c_temp,7          ; 1 ins / 2 ins (3)
        goto    i2c_ssp_master_tx_1 ; 2 ins (4)

i2c_ssp_master_tx_0
        nop                         ; 1 ins (5)
        bcf     SSP_TRIS,SSP_SDA    ; 1 ins (6)
        rlf     i2c_temp,F          ; 1 ins (7)
        bsf     SSP_TRIS,SSP_SCL    ; 1 ins (8)
        return                      ; 2 ins (9 = -2)

i2c_ssp_master_tx_1
        bsf     SSP_TRIS,SSP_SDA    ; 1 ins (6)
        rlf     i2c_temp,F          ; 1 ins (7)
        bsf     SSP_TRIS,SSP_SCL    ; 1 ins (8)
        return                      ; 2 ins (9 = -2)

        ENDIF ; CLOCK==4000000

;;--------------------------------------------------------------------------------

        IF CLOCK==8000000

;; Timing - 20 cycles with 8 MHz clock => 100 kHz bit rate
;;       ____|________________ _____________________________________|_
;;  SDA  ____|________________X_____________________________________|_
;;       ____|_                               ______________________|_
;;  SCL      | \_____________________________/                      | 
;;
;; Time  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19  0

i2c_ssp_master_tx_bit
        bcf     SSP_TRIS,SSP_SCL     ; 1 ins (2)
        btfss   i2c_temp,7           ; 1 ins / 2 ins (3)
        goto    i2c_ssp_master_tx_0  ; 2 ins (4)
        goto    i2c_ssp_master_tx_1  ; 2 ins (5)

i2c_ssp_master_tx_0
        nop                          ; 1 ins (6)
        bcf     SSP_TRIS,SSP_SDA     ; 1 ins (7)
        goto    i2c_ssp_master_tx_dly; 2 ins (8)

i2c_ssp_master_tx_1
        bsf     SSP_TRIS,SSP_SDA     ; 1 ins (7)
        goto    i2c_ssp_master_tx_dly; 2 ins (8)

i2c_ssp_master_tx_dly
        goto    $+1                  ; 2 ins (10)
        bsf     SSP_TRIS,SSP_SCL     ; 1 ins (12)
        rlf     i2c_temp,F           ; 1 ins (13)
        goto    $+1                  ; 2 ins (14)
        goto    $+1                  ; 2 ins (16)
        return                       ; 2 ins (18 = -2)

        ENDIF ; CLOCK==8000000


;;--------------------------------------------------------------------------------
;;
;; Subroutine to transmit the clock for an ACK
;;
;; RAM Bank:    SSP_TRIS (in) SSP_TRIS (out)
;;
;;--------------------------------------------------------------------------------

        IF CLOCK==4000000

;; Timing - 11 cycles with 4 MHz clock => 90.91 kHz bit rate
;;       ____|___________________________|_
;;  SDA  ____|____X                      | 
;;       ____|_                   _______|_
;;  SCL      | \_________________/       | 
;;
;; Time  0  1  2  3  4  5  6  7  8  9 10  0

i2c_ssp_master_tx_clock_ack
        bcf     SSP_TRIS,SSP_SCL; 1 ins (2)
        bsf     SSP_TRIS,SSP_SDA; 1 ins (3)
        call    delay_4_cycles  ; 4 ins (4)
        bsf     SSP_TRIS,SSP_SCL; 1 ins (8)
        return                  ; 2 ins (9 = -2)

        ENDIF ; CLOCK==4000000

;;--------------------------------------------------------------------------------

        IF CLOCK==8000000

;; Timing - 20 cycles with 8 MHz clock => 100 kHz bit rate
;;       ____|______________________________________________________|_
;;  SDA  ____|____X                                                 | 
;;       ____|_                               ______________________|_
;;  SCL      | \_____________________________/                      | 
;;
;; Time  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19  0

i2c_ssp_master_tx_clock_ack
        bcf     SSP_TRIS,SSP_SCL; 1 ins (2)
        bsf     SSP_TRIS,SSP_SDA; 1 ins (3)
        call    delay_8_cycles  ; 8 ins (4)
        bsf     SSP_TRIS,SSP_SCL; 1 ins (12)
        call    delay_4_cycles  ; 4 ins (13)
        nop                     ; 1 ins (17)
        return                  ; 2 ins (18 = -2)

        ENDIF ; CLOCK==8000000


;;--------------------------------------------------------------------------------
;;
;; Subroutine for a master receiving the non-last I2C serial byte (with ACK)
;;
;; Parameters:  w (out)         - Data byte received
;;
;; RAM Bank:    SSP_TRIS (in) SSP_TRIS (out)
;;
;;--------------------------------------------------------------------------------

;; Timing
;;       ____|__________________|_
;;  SDA  ____|_______X          | 
;;       ____|____              | 
;;  SCL      |    \_____________|_
;;
;; Time -1  0  1  2  3  4  5  6  7

i2c_ssp_master_rx_byte
        clrf    i2c_temp             ; 1 ins (0) (actually too slow after receiving an ACK/NACK)
        bcf     STATUS,C             ; 1 ins (1)

        bcf     SSP_TRIS,SSP_SCL     ; 1 ins (2)
        bsf     SSP_TRIS,SSP_SDA     ; 1 ins (3)
        nop                          ; 1 ins (4)

        ;; Bit 7 (MSB)
        call    i2c_ssp_master_rx_bit; 2 ins (5)

        ;; Bit 6
        call    i2c_ssp_master_rx_bit; 2 ins (5)

        ;; Bit 5
        call    i2c_ssp_master_rx_bit; 2 ins (5)

        ;; Bit 4
        call    i2c_ssp_master_rx_bit; 2 ins (5)

        ;; Bit 3
        call    i2c_ssp_master_rx_bit; 2 ins (5)

        ;; Bit 2
        call    i2c_ssp_master_rx_bit; 2 ins (5)

        ;; Bit 1
        call    i2c_ssp_master_rx_bit; 2 ins (5)

        ;; Bit 0 (LSB)
        call    i2c_ssp_master_rx_bit; 2 ins (5)

        ;; Send ACK
        call    i2c_ssp_master_tx_ack; 2 ins (5)

        movf    i2c_temp,W           ; 1 ins (-2)

        return                       ; 2 ins (-1)


;;--------------------------------------------------------------------------------
;;
;; Subroutine for a master receiving the last I2C serial byte (with NACK)
;;
;; Parameters:  w (out)         - Data byte received
;;
;; RAM Bank:    SSP_TRIS (in) SSP_TRIS (out)
;;
;;--------------------------------------------------------------------------------

;; Timing
;;       ____|__________________|_
;;  SDA  ____|_______X          | 
;;       ____|____              | 
;;  SCL      |    \_____________|_
;;
;; Time -1  0  1  2  3  4  5  6  7

i2c_ssp_master_rx_last_byte
        clrf    i2c_temp             ; 1 ins (0) (actually too slow after receiving an ACK/NACK)
        bcf     STATUS,C             ; 1 ins (1)

        bcf     SSP_TRIS,SSP_SCL     ; 1 ins (2)
        bsf     SSP_TRIS,SSP_SDA     ; 1 ins (3)
        nop                          ; 1 ins (4)

        ;; Bit 7 (MSB)
        call    i2c_ssp_master_rx_bit; 2 ins (5)

        ;; Bit 6
        call    i2c_ssp_master_rx_bit; 2 ins (5)

        ;; Bit 5
        call    i2c_ssp_master_rx_bit; 2 ins (5)

        ;; Bit 4
        call    i2c_ssp_master_rx_bit; 2 ins (5)

        ;; Bit 3
        call    i2c_ssp_master_rx_bit; 2 ins (5)

        ;; Bit 2
        call    i2c_ssp_master_rx_bit; 2 ins (5)

        ;; Bit 1
        call    i2c_ssp_master_rx_bit; 2 ins (5)

        ;; Bit 0 (LSB)
        call    i2c_ssp_master_rx_bit; 2 ins (5)

        ;; Send NACK
        call    i2c_ssp_master_tx_nack; 2 ins (5)

        movf    i2c_temp,W            ; 1 ins (0)

        return                        ; 2 ins (1)


;;--------------------------------------------------------------------------------
;;
;; Subroutine to receive a bit (various versions for different clock speeds)
;;
;; RAM Bank:    SSP_TRIS (in) SSP_TRIS (out)
;;
;;--------------------------------------------------------------------------------

        IF CLOCK==4000000

;; Timing - 11 cycles with 4 MHz clock => 90.91 kHz bit rate
;;       __|___ ______________________|_______
;;  SDA  __|___X______________________|_______
;;         |    ______________        |       
;;  SCL  __|___/              \_______|_______
;;
;; Time  6  7  8  9 10  0  1  2  3  4  5  6  7

i2c_ssp_master_rx_bit
        rlf     i2c_temp,F      ; 1 ins (7)
        bsf     SSP_TRIS,SSP_SCL; 1 ins (8)
        BANKSEL SSP_PORT        ; 1 ins (9)
        btfss   SSP_PORT,SSP_SCL; check for clock stretch
        goto    $-1             ; check for clock stretch
        btfsc   SSP_PORT,SSP_SDA; 1 ins / 2 ins (10)
        bsf     i2c_temp,0      ; 1 ins (0)
        BANKSEL SSP_TRIS        ; 1 ins (1)
        bcf     SSP_TRIS,SSP_SCL; 1 ins (2)
        return                  ; 2 ins (3)

        ENDIF ; CLOCK==4000000

;;--------------------------------------------------------------------------------

        IF CLOCK==8000000

;; Timing - 20 cycles with 8 MHz clock => 100 kHz bit rate
;;       __|_______________ _____________________________________|_______
;;  SDA  __|_______________X_____________________________________|_______
;;         |                _____________________________        |       
;;  SCL  __|_______________/                             \_______|_______
;;
;; Time  6  7  8  9 10 11 12 13 14 15 16 17 18 19  0  1  2  3  4  5  6  7

i2c_ssp_master_rx_bit
        rlf     i2c_temp,F      ; 1 ins (7)
        call    delay_4_cycles  ; 4 ins (8)
        bsf     SSP_TRIS,SSP_SCL; 1 ins (12)
        goto    $+1             ; 2 ins (13)
        BANKSEL SSP_PORT        ; 1 ins (15)
        btfss   SSP_PORT,SSP_SCL; check for clock stretch
        goto    $-1             ; check for clock stretch
        btfsc   SSP_PORT,SSP_SDA; 1 ins / 2 ins (16)
        bsf     i2c_temp,0      ; 1 ins (17)
        goto    $+1             ; 2 ins (18)
        nop                     ; 1 ins (0)
        BANKSEL SSP_TRIS        ; 1 ins (1)
        bcf     SSP_TRIS,SSP_SCL; 1 ins (2)
        return                  ; 2 ins (3)

        ENDIF ; CLOCK==8000000


;;--------------------------------------------------------------------------------
;;
;; Subroutine to send an ACK (various versions for different clock speeds)
;;
;; RAM Bank:    SSP_TRIS (in) SSP_TRIS (out)
;;
;;--------------------------------------------------------------------------------

        IF CLOCK==4000000

;; Timing - 11 cycles with 4 MHz clock => 90.91 kHz bit rate
;;       __|           | 
;;  SDA  __|X__________|_
;;         |    _______|_
;;  SCL  __|___/       | 
;;
;; Time  6  7  8  9 10  0

i2c_ssp_master_tx_ack
        bcf     SSP_TRIS,SSP_SDA; 1 ins (7)
        bsf     SSP_TRIS,SSP_SCL; 1 ins (8)
        return                  ; 2 ins (10 = -1)

        ENDIF ; CLOCK==4000000

;;--------------------------------------------------------------------------------

        IF CLOCK==8000000

;; Timing - 20 cycles with 8 MHz clock => 100 kHz bit rate
;;       __|                                      | 
;;  SDA  __|X_____________________________________|_
;;         |                ______________________|_
;;  SCL  __|_______________/                      | 
;;
;; Time  6  7  8  9 10 11 12 13 14 15 16 17 18 19  0

i2c_ssp_master_tx_ack
        bcf     SSP_TRIS,SSP_SDA; 1 ins (7)
        call    delay_4_cycles  ; 4 ins (8)
        bsf     SSP_TRIS,SSP_SCL; 1 ins (12)
        call    delay_4_cycles  ; 4 ins (13)
        nop                     ; 1 ins (17)
        return                  ; 2 ins (18 = -2)

        ENDIF ; CLOCK==8000000


;;--------------------------------------------------------------------------------
;;
;; Subroutine to send a NACK (various versions for different clock speeds)
;;
;; RAM Bank:    SSP_TRIS (in) SSP_TRIS (out)
;;
;;--------------------------------------------------------------------------------

        IF CLOCK==4000000

;; Timing - 11 cycles with 4 MHz clock => 90.91 kHz bit rate
;;       __| __________|_
;;  SDA  __|X          | 
;;         |    _______|_
;;  SCL  __|___/       | 
;;
;; Time  6  7  8  9 10  0

i2c_ssp_master_tx_nack
        bsf     SSP_TRIS,SSP_SDA; 1 ins (7)
        bsf     SSP_TRIS,SSP_SCL; 1 ins (8)
        return                  ; 2 ins (10 = -1)

        ENDIF ; CLOCK==4000000

;;--------------------------------------------------------------------------------

        IF CLOCK==8000000

;; Timing - 20 cycles with 8 MHz clock => 100 kHz bit rate
;;       __| _____________________________________|_
;;  SDA  __|X                                     | 
;;         |                ______________________|_
;;  SCL  __|_______________/                      | 
;;
;; Time  6  7  8  9 10 11 12 13 14 15 16 17 18 19  0

i2c_ssp_master_tx_nack
        bcf     SSP_TRIS,SSP_SDA; 1 ins (7)
        call    delay_4_cycles  ; 4 ins (8)
        bsf     SSP_TRIS,SSP_SCL; 1 ins (12)
        call    delay_4_cycles  ; 4 ins (13)
        nop                     ; 1 ins (17)
        return                  ; 2 ins (18 = -2)

        ENDIF ; CLOCK==8000000


;;--------------------------------------------------------------------------------
;;
;; Subroutine to transmit a stop bit (various versions for different clock speeds)
;;
;; RAM Bank:    SSP_TRIS (in) SSP_TRIS (out)
;;
;;--------------------------------------------------------------------------------

        IF CLOCK==4000000

;; Timing - 11 cycles with 4 MHz clock => 90.91 kHz bit rate
;;       ____|_____________                                  _______|_
;;  SDA  ____|_____________X________________________________/       | 
;;       ____|_                   __________________________________|_
;;  SCL      | \_________________/                                  | 
;;
;; Time  0  1  2  3  4  5  6  7  8  9 10  0  1  2  3  4  5  6  7  8  9

i2c_ssp_master_tx_stop
        bcf     SSP_TRIS,SSP_SCL; 1 ins (2)
        goto    $+1             ; 2 ins (3)
        nop                     ; 1 ins (5)
        bcf     SSP_TRIS,SSP_SDA; 1 ins (6)
        nop                     ; 1 ins (7)
        bsf     SSP_TRIS,SSP_SCL; 1 ins (8)
        call    delay_8_cycles  ; 8 ins (9)
        bsf     SSP_TRIS,SSP_SDA; 1 ins (6)
        return                  ; 2 ins (7)

        ENDIF ; CLOCK==4000000

;;--------------------------------------------------------------------------------

        IF CLOCK==8000000

;; Timing - 20 cycles with 8 MHz clock => 100 kHz bit rate
;;       ____|________________                                                             _______|_
;;  SDA  ____|________________X___________________________________________________________/       | 
;;       ____|_                               ____________________________________________________|_
;;  SCL      | \_____________________________/                                                    | 
;;
;; Time  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19  0  1  2  3  4  5  6  7  8  9 10

i2c_ssp_master_tx_stop
        bcf     SSP_TRIS,SSP_SCL; 1 ins (2)
        call    delay_4_cycles  ; 4 ins (3)
        bcf     SSP_TRIS,SSP_SDA; 1 ins (7)
        call    delay_4_cycles  ; 4 ins (8)
        bsf     SSP_TRIS,SSP_SCL; 1 ins (12)
        call    delay_14_cycles ;14 ins (13)
        bsf     SSP_TRIS,SSP_SDA; 1 ins (7)
        return                  ; 2 ins (8)

        ENDIF ; CLOCK==8000000


;;--------------------------------------------------------------------------------
;;
;; Subroutine to transmit a restart bit (various versions for different clock speeds)
;;
;; RAM Bank:    SSP_TRIS (in) SSP_TRIS (out)
;;
;;--------------------------------------------------------------------------------

        IF CLOCK==4000000

;; Timing - 11 cycles with 4 MHz clock => 90.91 kHz bit rate
;;       ____|_____________                                  ________________________________        | 
;;  SDA  ____|_____________X________________________________/                                \_______|_
;;       ____|_                   ___________________________________________________________________|_
;;  SCL      | \_________________/                                                                   | 
;;
;; Time  0  1  2  3  4  5  6  7  8  9 10  0  1  2  3  4  5  6  7  8  9 10  0  1  2  3  4  5  6  7  8  9

i2c_ssp_master_tx_restart
        bcf     SSP_TRIS,SSP_SCL; 1 ins (2)
        goto    $+1             ; 2 ins (3)
        nop                     ; 1 ins (5)
        bcf     SSP_TRIS,SSP_SDA; 1 ins (6)
        nop                     ; 1 ins (7)
        bsf     SSP_TRIS,SSP_SCL; 1 ins (8)
        call    delay_8_cycles  ; 8 ins (9)
        bsf     SSP_TRIS,SSP_SDA; 1 ins (6)
        call    delay_10_cycles ;10 ins (7)
        bcf     SSP_TRIS,SSP_SDA; 1 ins (6)
        return                  ; 2 ins (7 = -4)

        ENDIF ; CLOCK==4000000

;;--------------------------------------------------------------------------------

        IF CLOCK==8000000

;; Timing - 20 cycles with 8 MHz clock => 100 kHz bit rate
;;       ____|________________                                                             ___________________________________________________________                          |             
;;  SDA  ____|________________X___________________________________________________________/                                                           \_________________________|_____________
;;       ____|_                               __________________________________________________________________________________________________________________________________|_____________
;;  SCL      | \_____________________________/                                                                                                                                  |             
;;
;; Time  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19  0

i2c_ssp_master_tx_restart
        bcf     SSP_TRIS,SSP_SCL; 1 ins (2)
        call    delay_4_cycles  ; 4 ins (3)
        bcf     SSP_TRIS,SSP_SDA; 1 ins (7)
        call    delay_4_cycles  ; 4 ins (8)
        bsf     SSP_TRIS,SSP_SCL; 1 ins (12)
        call    delay_14_cycles ;14 ins (13)
        bsf     SSP_TRIS,SSP_SDA; 1 ins (7)
        call    delay_18_cycles ;18 ins (8)
        nop                     ; 1 ins (6)
        bcf     SSP_TRIS,SSP_SDA; 1 ins (7)
        call    delay_6_cycles  ; 6 ins (8)
        return                  ; 2 ins (14 = -6)

        ENDIF ; CLOCK==8000000


;;--------------------------------------------------------------------------------

;; Re-enable messages and warnings

        ERRORLEVEL      0


;;--------------------------------------------------------------------------------
