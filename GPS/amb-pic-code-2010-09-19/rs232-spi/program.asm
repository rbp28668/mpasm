;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/projects/07_rs232-to-spi/RCS/program.asm,v 1.14 2009/08/01 18:30:51 amb Exp $
;;
;; RS232 to SPI converter program.
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2006, 2009 Andrew M. Bishop
;; It may be distributed under the GNU Public License, version 2, or
;; any higher version.  See section COPYING of the GNU Public license
;; for conditions under which this file may be redistributed.
;;
;;--------------------------------------------------------------------------------

;;  Project title

        TITLE   "SPI Interface test"

;; Compilation options

        RADIX   DEC
        EXPAND

;; Processor

        Processor pic16f818

        include "p16f818.inc"

;; Config fuses

        __CONFIG (_CP_OFF & _CCP1_RB2 & _DEBUG_OFF & _WRT_ENABLE_OFF & _CPD_OFF & _LVP_OFF & _BODEN_OFF & _MCLR_ON & _PWRTE_OFF & _WDT_OFF & _INTRC_IO)

;; Variables and constants

CLOCK           EQU  8000000    ; Required for "delay.inc"

RS232_PORT      EQU  PORTA      ; Required for "rs232.inc"
RS232_TRIS      EQU  TRISA      ; Required for "rs232.inc"
RS232_BAUD      EQU  38400      ; Required for "rs232.inc"
RS232_RXD       EQU  7          ; Required for "rs232.inc"
RS232_TXD       EQU  6          ; Required for "rs232.inc"
RS232_RTS       EQU  1          ; Required for "rs232.inc"

SPI_CKE         EQU  1          ; Required for "spi.inc"
SPI_CKP         EQU  0          ; Required for "spi.inc"
SPI_SMP         EQU  1          ; Required for "spi.inc"
SPI_RATE_DIV    EQU  16         ; Required for "spi.inc"

        cblock  0x70
                delay_temp      ; Required for "delay.inc"
                spi_temp        ; Required for "spi.inc"
                rs232_temp      ; Required for "rs232.inc"

                mmc_temp        ; Required for "mmc.inc"
                mmc_addr_0      ; Required for "mmc.inc"
                mmc_addr_1      ; Required for "mmc.inc"
                mmc_addr_2      ; Required for "mmc.inc"

                counter
                state
        endc

STATE_GENERIC   EQU  'S'
STATE_MMC_INIT  EQU  'I'
STATE_MMC_READ  EQU  'R'
STATE_MMC_WRITE EQU  'W'
STATE_SPEED     EQU  'C'


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

;; Enable RS232 port

        call    rs232_init_port

;; Enable SPI master port

        call    spi_init_master_port

;; Main program

main

        ;; Read one byte from RS232

        call    rs232_rx_byte
        movwf   state

        ;; Decide which function to call

        movlw   STATE_GENERIC
        xorwf   state,W
        btfss   STATUS,Z
        goto    $+3
        call    generic_spi
        goto    main

        movlw   STATE_MMC_INIT
        xorwf   state,W
        btfss   STATUS,Z
        goto    $+3
        call    spi_mmc_init
        goto    main

        movlw   STATE_MMC_READ
        xorwf   state,W
        btfss   STATUS,Z
        goto    $+3
        call    spi_mmc_read
        goto    main

        movlw   STATE_MMC_WRITE
        xorwf   state,W
        btfss   STATUS,Z
        goto    $+3
        call    spi_mmc_write
        goto    main

        movlw   STATE_SPEED
        xorwf   state,W
        btfss   STATUS,Z
        goto    $+3
        call    spi_speed
        goto    main

        ;; Anything else is looped back
        movf    state,W
        call    rs232_tx_byte

        goto    main

spi_speed
        ;; Get the next byte and put it into SSPCON (with a mask)

        BANKSEL SSPCON

        bcf     SSPCON,SSPEN

        movlw   b'11111100'
        andwf   SSPCON,F

        call    rs232_rx_byte

        andlw   b'00000011'
        iorwf   SSPCON,F        

        bsf     SSPCON,SSPEN

        goto    main


;; Subroutines

        include "mmc.inc"

        include "generic.inc"

;; Subroutines loaded from include files

        include "../devices/16f819/reset_device.inc"

        include "../devices/16f819/set_oscillator.inc"

        include "../common/delay.inc"

        include "../common/rs232.inc"

        include "../common/spi.inc"

        include "../common/mmc.inc"

;; End

        end
