;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/projects/06_cable-box-changer/RCS/program.asm,v 1.8 2009/08/01 18:30:28 amb Exp $
;;
;; RS232 to Infra-red Cable box changer.
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

        TITLE   "RS232 to Infra-red Cable box changer"

;; Compilation options

        RADIX   DEC
        EXPAND

;; Processor

        Processor pic12f675

        include "p12f675.inc"

;; Config fuses

        __CONFIG (_CPD_OFF & _CP_OFF & _BODEN_OFF & _MCLRE_ON & _PWRTE_OFF & _WDT_OFF & _INTRC_OSC_NOCLKOUT & b'1111000111111111')

;; Variables and constants

CLOCK           EQU  4000000    ; Required for "delay.inc", "rs232.inc"

RS232_PORT      EQU  GPIO       ; Required for "rs232.inc"
RS232_TRIS      EQU  TRISIO     ; Required for "rs232.inc"
RS232_BAUD      EQU  38400      ; Required for "rs232.inc"
RS232_RXD       EQU  0          ; Required for "rs232.inc"
RS232_TXD       EQU  1          ; Required for "rs232.inc"
RS232_RTS       EQU  2          ; Required for "rs232.inc"

IR_PORT         EQU  GPIO       ; Required for "ir.inc"
IR_TRIS         EQU  TRISIO     ; Required for "ir.inc"
IR_TXD          EQU  4          ; Required for "ir.inc"
IR_NBYTES       EQU  2          ; Required for "ir.inc"

USER_LED        EQU  5

        cblock  0x20
                delay_temp      ; Required for "delay.inc"
                rs232_temp      ; Required for "rs232.inc"

                ir_byte1        ; Required for "ir.inc"
                ir_byte2        ; Required for "ir.inc"
                ir_length       ; Required for "ir.inc"
                ir_temp         ; Required for "ir.inc"

                number
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

;; Enable IR port

        call    ir_init_port

;; Enable LED output

        BANKSEL TRISIO          ; Select RAM bank
        bcf     TRISIO,USER_LED

        BANKSEL GPIO            ; Select RAM bank
        bcf     GPIO,USER_LED

;; Main program

main
        BANKSEL GPIO

        call    rs232_rx_byte
        movwf   number

        ;; Turn user LED on

        bsf     GPIO,USER_LED

        movlw   250
        call    delay_ms

        ;; Transmit number

        movlw   '0'
        subwf   number,F

        call    transmit_cable

        ;; Turn user LED off

        movlw   250
        call    delay_ms

        bcf     GPIO,USER_LED

        ;; Start again

        goto    main


transmit_cable

;; Cable box remote control
;; 16 bits of data
;; 
;; <-N> 0000 0000 <N>      for button <N>

        movf    number,W
        movwf   ir_byte2
        comf    ir_byte2,F
        incf    ir_byte2,F
        swapf   ir_byte2,F
        movlw   b'11110000'
        andwf   ir_byte2,F

        movf    number,W
        movwf   ir_byte1
        movlw   b'00001111'
        andwf   ir_byte1,F

        movlw   16
        movwf   ir_length

        call    ir_cable_tx_data

        return


;; Subroutines loaded from include files

        include "../devices/12f675/reset_device.inc"

        include "../common/delay.inc"

        include "../common/rs232.inc"

        include "../common/ir.inc"

;; End

        end
