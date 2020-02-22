;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/projects/11_gps-sd-logger/RCS/program.asm,v 1.11 2009/08/01 18:30:51 amb Exp $
;;
;; GPS on RS232 to SD card data logger program.
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2007, 2009 Andrew M. Bishop
;; It may be distributed under the GNU Public License, version 2, or
;; any higher version.  See section COPYING of the GNU Public license
;; for conditions under which this file may be redistributed.
;;
;;--------------------------------------------------------------------------------

;;  Project title

        TITLE   "GPS to SD card logger"

;; Compilation options

        RADIX   DEC
        EXPAND

;; Processor

        Processor pic16f819

        include "p16f819.inc"

;; Config fuses

        __CONFIG (_CP_OFF & _CCP1_RB2 & _DEBUG_OFF & _WRT_ENABLE_OFF & _CPD_OFF & _LVP_OFF & _BODEN_OFF & _MCLR_ON & _PWRTE_OFF & _WDT_OFF & _INTRC_IO)

;; Variables and constants

CLOCK           EQU  8000000    ; Required for "delay.inc"

RS232_PORT      EQU  PORTA      ; Required for "rs232.inc"
RS232_TRIS      EQU  TRISA      ; Required for "rs232.inc"
RS232_BAUD      EQU  9600       ; Required for "rs232.inc"
RS232_RXD       EQU  7          ; Required for "rs232.inc"
RS232_TXD       EQU  0          ; Required for "rs232.inc"

SPI_CKE         EQU  1          ; Required for "spi.inc"
SPI_CKP         EQU  0          ; Required for "spi.inc"
SPI_SMP         EQU  1          ; Required for "spi.inc"
SPI_RATE_DIV    EQU  16         ; Required for "spi.inc"

SD_PORT         EQU  PORTA
SD_TRIS         EQU  TRISA
SD_POWER        EQU  2

GPS_PORT        EQU  PORTA
GPS_TRIS        EQU  TRISA
GPS_POWER       EQU  1
GPS_RESET       EQU  6

LED_PORT        EQU  PORTA
LED_TRIS        EQU  TRISA
LED_RED         EQU  4
LED_GREEN       EQU  3

SD_PWR          EQU  0          ; SD card power is on
GPS_PWR         EQU  1          ; GPS power is on
GPS_SYNC        EQU  2          ; GPS is synchronised now
GPS_NSYNC       EQU  3          ; GPS will be synchronised next time
GPS_FIX_R       EQU  4          ; GPS fix requires red LED on
GPS_FIX_G       EQU  5          ; GPS fix requires green LED on

        cblock  0x70
                delay_temp      ; Required for "delay.inc"
                spi_temp        ; Required for "spi.inc"
                rs232_temp      ; Required for "rs232.inc"
                eeprom_temp     ; Required for "eeprom.inc"

                mmc_temp        ; Required for "mmc.inc"
                mmc_addr_0      ; Required for "mmc.inc"
                mmc_addr_1      ; Required for "mmc.inc"
                mmc_addr_2      ; Required for "mmc.inc"

                status          ; Global status

                counter         ; General loop counter

                gps_match       ; Used in gps.inc
                gps_temp        ; Used in gps.inc
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

;; Enable LED outputs

        BANKSEL LED_PORT           ; Select RAM bank
        bcf     LED_PORT,LED_RED   ; Turn off red LED
        bcf     LED_PORT,LED_GREEN ; Turn off green LED

        BANKSEL LED_TRIS           ; Select RAM bank
        bcf     LED_TRIS,LED_RED   ; Enable output
        bcf     LED_TRIS,LED_GREEN ; Enable output

;; Enable GPS outputs

        BANKSEL GPS_PORT           ; Select RAM bank
        bsf     GPS_PORT,GPS_POWER ; Turn off GPS power
        bcf     GPS_PORT,GPS_RESET ; Apply GPS reset

        BANKSEL GPS_TRIS           ; Select RAM bank
        bcf     GPS_TRIS,GPS_POWER ; Enable output
        bcf     GPS_TRIS,GPS_RESET ; Enable output

;; Enable SD outputs

        BANKSEL SD_PORT          ; Select RAM bank
        bsf     SD_PORT,SD_POWER ; Turn off SD power

        BANKSEL SD_TRIS          ; Select RAM bank
        bcf     SD_TRIS,SD_POWER ; Enable output

;; Enable unused pins as outputs (not ICSP pins)

        BANKSEL PORTB           ; Select RAM bank
        bcf     PORTB,3         ; Set output low

        BANKSEL TRISB           ; Select RAM bank
        bcf     TRISB,3         ; Set as output

;        BANKSEL PORTB           ; Select RAM bank
;        bcf     PORTB,6         ; Set output low
;        bcf     PORTB,7         ; Set output low
;
;        BANKSEL TRISB           ; Select RAM bank
;        bcf     TRISB,6         ; Set as output
;        bcf     TRISB,7         ; Set as output

;; Main program

main

        ;; Clear and enable interrupt and go to sleep

        BANKSEL INTCON          ; Select RAM bank
        bsf     INTCON,INTEDG   ; Select rising edge
        bcf     INTCON,INTF     ; Clear interrupt flag
        bsf     INTCON,INTE     ; Enable interrupt

        sleep

        bcf     INTCON,INTF     ; Clear interrupt flag

        ;; Check if on button has been held down for 10 seconds

        movlw   250
        call    delay_ms

        BANKSEL LED_PORT           ; Select RAM bank
        bsf     LED_PORT,LED_RED   ; Turn on red LED
        bcf     LED_PORT,LED_GREEN ; Turn off green LED

        BANKSEL PORTB           ; Select RAM bank

        movlw   40
        movwf   counter

reset_button_loop
        btfss   PORTB,0         ; skip if button is still pressed
        goto    initialise

        movlw   250
        call    delay_ms

        decfsz  counter,F
        goto    reset_button_loop

        ;; Reset address to 0xfffffe

        movlw   0xff
        movwf   mmc_addr_0
        movwf   mmc_addr_1
        movlw   0xfe
        movwf   mmc_addr_2

        call    SD_write_eeprom

initialise

        ;; Turn off LEDs

        BANKSEL LED_PORT           ; Select RAM bank
        bcf     LED_PORT,LED_RED   ; Turn off red LED
        bcf     LED_PORT,LED_GREEN ; Turn off green LED

        ;; Initialise

        clrf    status

        ;; Power up SD card and initialise it

        call    SD_power_up

        call    SD_start

        ;; Power up GPS and initialise it

        call    GPS_power_up

        call    GPS_start

        ;; Loop

loop

        ;; Check card is powered and GPS is synchronised

        btfss   status,GPS_SYNC ; skip if GPS is synchronised
        goto    skip_start_write

        btfss   status,SD_PWR   ; skip if power is on
        goto    skip_start_write

        ;; Increase the address

        call    SD_incr_address

        ;; Start the write

        call    mmc_write_start

        xorlw   0x00
        btfss   STATUS,Z        ; skip if OK
        call    SD_failure_after_incr

skip_start_write

        bcf     status,GPS_FIX_R
        bcf     status,GPS_FIX_G

        ;; Log 512 bytes (including padding) from GPS to SD

        call    GPS_log

        ;; Turn off LEDs (may not be synchronised next time)

        BANKSEL LED_PORT           ; Select RAM bank
        bcf     LED_PORT,LED_RED   ; Turn off red LED
        bcf     LED_PORT,LED_GREEN ; Turn off green LED

        bcf     status,GPS_NSYNC; clear GPS next sync bit

        ;; Check if synchronised

        xorlw   0x01
        btfss   STATUS,Z        ; skip if GPS_log returned 1
        goto    skip_leds

        ;; Turn on LEDs (synchronised next time)

        BANKSEL LED_PORT        ; Select RAM bank
        btfsc   status,GPS_FIX_G
        bsf     LED_PORT,LED_GREEN
        btfsc   status,GPS_FIX_R
        bsf     LED_PORT,LED_RED

        bsf     status,GPS_NSYNC; set GPS next sync bit

skip_leds

        ;; Check card is powered and GPS is synchronised

        btfss   status,GPS_SYNC ; skip if GPS is synchronised
        goto    skip_finish_write

        btfss   status,SD_PWR   ; skip if power is on
        goto    skip_finish_write

        ;; End the write

        call    mmc_write_end

        xorlw   0x00
        btfss   STATUS,Z        ; skip if OK
        call    SD_failure_after_incr

skip_finish_write

        ;; Check if GPS became synchronised or lost it

        bsf     status,GPS_SYNC ; set GPS sync bit
        btfss   status,GPS_NSYNC; skip if GPS_log returned 1
        bcf     status,GPS_SYNC ; clear GPS sync bit

        ;; Check for interrupt

        BANKSEL INTCON          ; Select RAM bank
        btfss   INTCON,INTF     ; skip if button pressed
        goto    loop

        ;; Turn off LEDs

        BANKSEL LED_PORT           ; Select RAM bank
        bcf     LED_PORT,LED_RED   ; Turn off red LED
        bcf     LED_PORT,LED_GREEN ; Turn off green LED

        ;; Power down GPS

        call    GPS_finish

        call    GPS_power_down

        ;; Power down SD

        call    SD_finish

        call    SD_power_down

        ;; Start again

        goto    main


;; Subroutines

        include "sd.inc"

        include "gps.inc"

;; Subroutines loaded from include files

        include "../devices/16f819/reset_device.inc"

        include "../devices/16f819/set_oscillator.inc"

        include "../common/eeprom.inc"

        include "../common/rs232.inc"

        include "../common/delay.inc"

        include "../common/spi.inc"

        include "../common/mmc.inc"

;; EEPROM DATA

DEEPROM         CODE

sd_eeprom_addr_0 DE     0xff
sd_eeprom_addr_1 DE     0xff
sd_eeprom_addr_2 DE     0xfe

gps_eeprom_init1 DE     "$PNMRX107,ALL,0*1e\r\n", 0
gps_eeprom_init2 DE     "$PNMRX108,GGA,RMC,GSV,GSA*6a\r\n", 0
gps_eeprom_init3 DE     "$PNMRX103,GGA,1,RMC,1,GSV,1,GSA,1*61\r\n", 0

;; End

        end
