;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/projects/05_rs232-infra-red/RCS/program.asm,v 1.13 2009/08/01 18:30:27 amb Exp $
;;
;; RS232 to Infra-Red converter program.
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2006,07,08,09 Andrew M. Bishop
;; It may be distributed under the GNU Public License, version 2, or
;; any higher version.  See section COPYING of the GNU Public license
;; for conditions under which this file may be redistributed.
;;
;;--------------------------------------------------------------------------------

;;  Project title

        TITLE   "RS232 to Infra-Red"

;; Compilation options

        RADIX   DEC
        EXPAND

;; Processor

        Processor pic12f683

        include "p12f683.inc"

;; Config fuses

        __CONFIG (_CPD_OFF & _CP_OFF & _BOD_OFF & _MCLRE_ON & _PWRTE_OFF & _WDT_OFF & _FCMEN_OFF & _IESO_OFF & _INTOSCIO & b'1111000111111111')

;; Variables and constants

CLOCK           EQU  4000000    ; Required for "delay.inc" and "rs232.inc"

RS232_PORT      EQU  GPIO       ; Required for "rs232.inc"
RS232_TRIS      EQU  TRISIO     ; Required for "rs232.inc"
RS232_BAUD      EQU  38400      ; Required for "rs232.inc"
RS232_RXD       EQU  0          ; Required for "rs232.inc"
RS232_TXD       EQU  1          ; Required for "rs232.inc"
RS232_RTS       EQU  2          ; Required for "rs232.inc"

IR_PORT         EQU  GPIO       ; Required for "ir.inc"
IR_TRIS         EQU  TRISIO     ; Required for "ir.inc"
IR_RXD          EQU  5          ; Required for "ir.inc"
IR_TXD          EQU  4          ; Required for "ir.inc"
IR_NBYTES       EQU  8          ; Required for "ir.inc"

START_A1        EQU  0x20       ; 32
END_A1          EQU  0x7F       ; 127 => 96 bytes of data inclusive
START_A2        EQU  0xA0       ; 160
END_A2          EQU  0xBF       ; 191 => 32 bytes of data inclusive

STATE_RX_RAW    EQU  'R'
STATE_TX_RAW    EQU  'T'

STATE_RX_REMOTE EQU  'r'
STATE_TX_REMOTE EQU  't'

REMOTE_RC5      EQU  'R'
REMOTE_SIRCS    EQU  'S'
REMOTE_CABLE    EQU  'C'
REMOTE_PANASONIC EQU 'P'
REMOTE_SAMSUNG  EQU 's'
REMOTE_NEC      EQU 'N'

delay_temp      EQU  CCPR1L     ; Required for "delay.inc"
rs232_temp      EQU  CCPR1L     ; Required for "rs232.inc"
temp            EQU  CCPR1H

        cblock 0x70
                remote_type
                ir_temp         ; Required for "ir.inc"
                ir_length       ; Required for "ir.inc"
                ir_byte1        ; Required for "ir.inc"
                ir_byte2        ; Required for "ir.inc"
                ir_byte3        ; Required for "ir.inc"
                ir_byte4        ; Required for "ir.inc"
                ir_byte5        ; Required for "ir.inc"
                ir_byte6        ; Required for "ir.inc"
                ir_byte7        ; Required for "ir.inc"
                ir_byte8        ; Required for "ir.inc"
        endc

;; Reset and interrupt vectors

        org     0x00

reset_vector
        goto    program
        nop
        nop
        nop
int_vector
        nop
program

;; Set internal oscillator clock speed

        call    set_internal_oscillator

;; Generic reset of device

        call    reset_device

;; Wait 1 second to make ICSP easier

        movlw   250
        call    delay_ms

        movlw   250
        call    delay_ms

        movlw   250
        call    delay_ms

        movlw   250
        call    delay_ms

;; Enable RS232 ports

        call    rs232_init_port

;; Enable Infra-Red I/O

        call    ir_init_port

;; Main program

main
        BANKSEL GPIO

        ;; Read byte from RS232 and process it.
get_rs232_byte
        call    rs232_rx_byte
        movwf   temp

        ;; Rx command raw data
        movlw   STATE_RX_RAW
        xorwf   temp,W
        btfsc   STATUS,Z
        goto    receive_raw

        ;; Tx command raw data
        movlw   STATE_TX_RAW
        xorwf   temp,W
        btfsc   STATUS,Z
        goto    transmit_raw

        ;; Rx command remote data
        movlw   STATE_RX_REMOTE
        xorwf   temp,W
        btfsc   STATUS,Z
        goto    receive_remote

        ;; Tx command remote data
        movlw   STATE_TX_REMOTE
        xorwf   temp,W
        btfsc   STATUS,Z
        goto    transmit_remote

        ;; Anything else is looped back
        movf    temp,W
        call    rs232_tx_byte

        goto    get_rs232_byte


        ;; Receive Infra-Red signal, store it, transmit it on RS232
receive_raw
        movlw   START_A1
        movwf   FSR

        movlw   b'00000001'     ; start at 1 normally due to cost of first loop
        movwf   temp

        movlw   b'00000101'     ; start at 5 due to delay of 4 periods in glitch removal
        movwf   INDF

ir_rx_first_edge
        btfsc   IR_PORT,IR_RXD
        goto    ir_rx_first_edge

        ;; Ignore glitch (2* 26 us)

        movlw   50
        call    delay_us        ;50 ins
        btfsc   IR_PORT,IR_RXD  ; 2 ins
        goto    ir_rx_first_edge

        ;; Ignore glitch (2* 26 us)

        movlw   50
        call    delay_us        ;50 ins
        btfsc   IR_PORT,IR_RXD  ; 2 ins
        goto    ir_rx_first_edge

        ;; Receive loop, total of 26 instructions @ 4 MHz = 26 us.
ir_rx_loop
        btfsc   IR_PORT,IR_RXD  ; 1 ins (1* loop=1) / 2 ins (0* loop=2)
        goto    ir_rx_is_1      ; 2 ins (1* loop=3)

ir_rx_is_0
        nop                     ; 1 ins (0* loop=3)

        bcf     temp,7          ; 1 ins (0* loop=4)

        btfss   INDF,7          ; 1 ins (00 loop=5) / 2 ins (01 loop=6)
        goto    ir_rx_inc_indf  ; 2 ins (00 loop=7)
        goto    ir_rx_inc_fsr   ; 2 ins (01 loop=8)

ir_rx_is_1
        bsf     temp,7          ; 1 ins (0* loop=4)

        btfsc   INDF,7          ; 1 ins (11 loop=5) / 2 ins (10 loop=6)
        goto    ir_rx_inc_indf  ; 2 ins (11 loop=7)
        goto    ir_rx_inc_fsr   ; 2 ins (10 loop=8)

        ;; Increment counter and check for overflow
ir_rx_inc_indf
        movlw   0x7f            ; 1 ins (A,B loop=8)
        andwf   INDF,W          ; 1 ins (A,B loop=9)
        sublw   0x7f            ; 1 ins (A,B loop=10)
        btfss   STATUS,Z        ; 1 ins (B loop=11) / 2 ins (A loop=12)
        goto    ir_rx_inc_indf_only; 2 ins (B loop=13)

        ;; Increment pointer and check for end
ir_rx_inc_fsr_only
        incf    FSR,F           ; 1 ins (A,C loop=13)

        movf    FSR,W           ; 1 ins (A,C loop=14)
        sublw   END_A2+1        ; 1 ins (A,C loop=15)
        btfsc   STATUS,Z        ; 2 ins (A,C loop=17)
        goto    ir_rx_finished

        movf    FSR,W           ; 1 ins (A,C loop=18)
        sublw   END_A1+1        ; 1 ins (A,C loop=19)
        movlw   START_A2        ; 1 ins (A,C loop=20) doesn't set Z
        btfsc   STATUS,Z        ; 1 ins (A,C loop=21)
        movwf   FSR             ; 1 ins (A,C loop=22)

        movf    temp,W          ; 1 ins (A,C loop=23)
        movwf   INDF            ; 1 ins (A,C loop=24)
        goto    ir_rx_loop      ; 2 ins (A,C loop=26)

        ;; Increment counter, it won't overflow
ir_rx_inc_indf_only
        incf    INDF,F          ; 1 ins (B loop=14)

        call    delay_10_cycles ;10 ins (B loop=24)

        goto    ir_rx_loop      ; 2 ins (B loop=26)

        ;; Wait before incrementing pointer and checking for end
ir_rx_inc_fsr
        nop                     ; 1 ins (C loop=9)
        nop                     ; 1 ins (C loop=10)
        goto    ir_rx_inc_fsr_only ; 2 ins (C loop=12)


        ;; Send result back on RS232
ir_rx_finished
        movlw   START_A1
        movwf   FSR

rs232_tx_loop
        movlw   b'10000000'     ;   ** NOTE - inverted MSB from pin state 1=active, 0=inactive
        xorwf   INDF,W
        call    rs232_tx_byte

        incf    FSR,F

        movf    FSR,W
        sublw   END_A2+1
        btfsc   STATUS,Z
        goto    main

        movf    FSR,W
        sublw   END_A1+1
        movlw   START_A2        ; doesn't set Z
        btfsc   STATUS,Z
        movwf   FSR
        goto    rs232_tx_loop


        ;; Receive RS232 command, store it, transmit it on Infra-Red
transmit_raw
        movlw   START_A1
        movwf   FSR

rs232_rx_loop
        call    rs232_rx_byte
        movwf   INDF

        movlw   b'10000000'     ;   ** NOTE - inverted MSB from pin state 1=active, 0=inactive
        xorwf   INDF,F

        incf    FSR,F

        movf    FSR,W
        sublw   END_A2+1
        btfsc   STATUS,Z
        goto    ir_tx

        movf    FSR,W
        sublw   END_A1+1
        movlw   START_A2        ; doesn't set Z
        btfsc   STATUS,Z
        movwf   FSR

        goto    rs232_rx_loop

        ;; Transmit stored command
ir_tx
        movlw   START_A1
        movwf   FSR

        ;; Transmit loop, total of 26 instructions @ 4 MHz = 26 us.
ir_tx_loop
        btfsc   INDF,7          ; 1 ins (1 loop=1) / 2 ins (0 loop=2)
        goto    ir_tx_inactive_entry; 2 ins (1 loop=3)

ir_tx_active_entry
        bsf     IR_PORT,IR_TXD  ; 1 ins (0 loop=3)
        call    delay_8_cycles  ; 8 ins (0 loop=11)

        decfsz  INDF,F          ; 1 ins (0 loop=12) / 2 ins (0 loop=13)
        goto    ir_tx_active    ; 2 ins (0 loop=14)

ir_tx_inc_fsr
        incf    FSR,F           ; 1 ins (loop=14)

        movf    FSR,W           ; 1 ins (loop=15)
        bcf     IR_PORT,IR_TXD  ; 1 ins (loop=16) 50% duty cycle
        sublw   END_A2+1        ; 1 ins (loop=17)
        btfsc   STATUS,Z        ; 2 ins (loop=19)
        goto    main

        movf    FSR,W           ; 1 ins (loop=20)
        sublw   END_A1+1        ; 1 ins (loop=21)
        movlw   START_A2        ; 1 ins (loop=22) doesn't set Z
        btfsc   STATUS,Z        ; 1 ins (loop=23)
        movwf   FSR             ; 1 ins (loop=24)

        goto    ir_tx_loop      ; 2 ins (loop=26)

ir_tx_active
        nop                     ; 1 ins (0 loop=15)
        bcf     IR_PORT,IR_TXD  ; 1 ins (0 loop=16)
        call    delay_10_cycles ;10 ins (0 loop=26)

        goto    ir_tx_active_entry; 2 ins (0 loop=2)

ir_tx_inactive
        call    delay_16_cycles ;16 ins (1 loop=2)
        nop                     ; 1 ins (1 loop=3)

ir_tx_inactive_entry
        bcf     INDF,7          ; 1 ins (1 loop=4)
        call    delay_4_cycles  ; 4 ins (1 loop=8)
        nop                     ; 1 ins (1 loop=9)

        decfsz  INDF,F          ; 1 ins (1 loop=10) / 2 ins (1 loop=11)
        goto    ir_tx_inactive  ; 2 ins (1 loop=12)

        goto    ir_tx_inc_fsr   ; 2 ins (1 loop=13)


        ;; Receive and decode specific remote control Infra-Red signal, transmit it on RS232
receive_remote
        call    rs232_rx_byte
        movwf   remote_type

        ;; RC5 remote
        movlw   REMOTE_RC5
        xorwf   remote_type,W
        btfsc   STATUS,Z
        call    ir_rc5_rx_data

        ;; SIRCS remote
        movlw   REMOTE_SIRCS
        xorwf   remote_type,W
        btfsc   STATUS,Z
        call    ir_sircs_rx_data

        ;; Cable remote
        movlw   REMOTE_CABLE
        xorwf   remote_type,W
        btfsc   STATUS,Z
        call    ir_cable_rx_data

        ;; Panasonic remote
        movlw   REMOTE_PANASONIC
        xorwf   remote_type,W
        btfsc   STATUS,Z
        call    ir_panasonic_rx_data

        ;; Samsung remote
        movlw   REMOTE_SAMSUNG
        xorwf   remote_type,W
        btfsc   STATUS,Z
        call    ir_samsung_rx_data

        ;; NEC remote
        movlw   REMOTE_NEC
        xorwf   remote_type,W
        btfsc   STATUS,Z
        call    ir_nec_rx_data

        movf    ir_length,W
        call    rs232_tx_byte

        movf    ir_byte8,W
        call    rs232_tx_byte

        movf    ir_byte7,W
        call    rs232_tx_byte

        movf    ir_byte6,W
        call    rs232_tx_byte

        movf    ir_byte5,W
        call    rs232_tx_byte

        movf    ir_byte4,W
        call    rs232_tx_byte

        movf    ir_byte3,W
        call    rs232_tx_byte

        movf    ir_byte2,W
        call    rs232_tx_byte

        movf    ir_byte1,W
        call    rs232_tx_byte

        goto    main


        ;; Receive RS232 command, transmit it on Infra-Red for specific remote control
transmit_remote
        call    rs232_rx_byte
        movwf   remote_type

        call    rs232_rx_byte
        movwf   ir_length

        call    rs232_rx_byte
        movwf   ir_byte8

        call    rs232_rx_byte
        movwf   ir_byte7

        call    rs232_rx_byte
        movwf   ir_byte6

        call    rs232_rx_byte
        movwf   ir_byte5

        call    rs232_rx_byte
        movwf   ir_byte4

        call    rs232_rx_byte
        movwf   ir_byte3

        call    rs232_rx_byte
        movwf   ir_byte2

        call    rs232_rx_byte
        movwf   ir_byte1

        ;; RC5 remote
        movlw   REMOTE_RC5
        xorwf   remote_type,W
        btfsc   STATUS,Z
        call    ir_rc5_tx_data

        ;; SIRCS remote
        movlw   REMOTE_SIRCS
        xorwf   remote_type,W
        btfsc   STATUS,Z
        call    ir_sircs_tx_data

        ;; Cable remote
        movlw   REMOTE_CABLE
        xorwf   remote_type,W
        btfsc   STATUS,Z
        call    ir_cable_tx_data

        ;; Panasonic remote
        movlw   REMOTE_PANASONIC
        xorwf   remote_type,W
        btfsc   STATUS,Z
        call    ir_panasonic_tx_data

        ;; Samsung remote
        movlw   REMOTE_SAMSUNG
        xorwf   remote_type,W
        btfsc   STATUS,Z
        call    ir_samsung_tx_data

        ;; NEC remote
        movlw   REMOTE_NEC
        xorwf   remote_type,W
        btfsc   STATUS,Z
        call    ir_nec_tx_data

        goto    main


;; Subroutines loaded from include files

        include "../devices/12f683/reset_device.inc"

        include "../devices/12f683/set_oscillator.inc"

        include "../common/delay.inc"

        include "../common/rs232.inc"

        include "../common/ir.inc"

;; End

        end
