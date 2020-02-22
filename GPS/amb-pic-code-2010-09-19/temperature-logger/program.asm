;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/projects/08_temperature-logger/RCS/program.asm,v 1.9 2009/08/01 18:30:51 amb Exp $
;;
;; Temperature logger program.
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

        TITLE   "Temperature data logger"

;; Compilation options

        RADIX   DEC
        EXPAND

;; Processor

        Processor pic12f683

        include "p12f683.inc"

;; Config fuses

        __CONFIG (_CPD_OFF & _CP_OFF & _BOD_OFF & _MCLRE_ON & _PWRTE_OFF & _WDT_OFF & _FCMEN_OFF & _IESO_OFF & _INTOSCIO & b'1111000111111111')

;; Variables and constants

CLOCK           EQU  4000000    ; Required for "delay.inc" and "i2c.inc"

I2C_PORT        EQU  GPIO       ; Required for "i2c.inc"
I2C_TRIS        EQU  TRISIO     ; Required for "i2c.inc"
I2C_SCL         EQU  2          ; Required for "i2c.inc"
I2C_SDA         EQU  1          ; Required for "i2c.inc"

ADC_IN          EQU  0          ; AN0 - pin 7


        cblock  0x70
                delay_temp      ; Required for "delay.inc"
                i2c_temp        ; Required for "i2c.inc"

                address_msb
                address_lsb

                period
                periodcounter

                adc_counter

                adc_result_msb
                adc_result_lsb

                prev_adc_result_msb
                prev_adc_result_lsb

                adc_result_delta_msb
                adc_result_delta_lsb
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

;; Enable I2C ports

        call    i2c_init_port

;; Enable ADC

enable_adc

        BANKSEL ANSEL           ; Select RAM bank
        bcf     ANSEL,ADCS2     ; \
        bcf     ANSEL,ADCS1     ;  | Set F/8 ADC frequency
        bsf     ANSEL,ADCS0     ; /
        bsf     ANSEL,ADC_IN    ; Enable ADC

        BANKSEL ADCON0          ; Select RAM bank
        bsf     ADCON0,ADFM     ; Set right aligned
        bcf     ADCON0,VCFG     ; Use Vdd as Vref
        bcf     ADCON0,CHS1     ; \ Use AN0
        bcf     ADCON0,CHS0     ; /
        bsf     ADCON0,ADON     ; Enable ADC

;; Enable timer 1

enable_timer1

        BANKSEL T1CON
        bsf     T1CON,T1OSCEN   ; Enable built-in oscillator
        bsf     T1CON,TMR1CS    ; Set timer 1 to use external clock
        bsf     T1CON,NOT_T1SYNC; Do not synchronise external clock
        bcf     T1CON,T1GE      ; Disable timer 1 gating
        bcf     T1CON,T1CKPS1   ; \
        bcf     T1CON,T1CKPS0   ;  \ Set the prescaler to a value of 1:1
        bsf     T1CON,TMR1ON    ; Start timer 1

        BANKSEL PIE1
        bsf     PIE1,TMR1IE     ; Enable the timer 1 interrupt

        BANKSEL INTCON
        bsf     INTCON,PEIE     ; Peripheral interrupt enable


;; Main program

main

        BANKSEL GPIO

        ;; Reset the variables

        movlw   255
        movwf   prev_adc_result_lsb
        movwf   prev_adc_result_msb

        movlw   32
        movwf   address_lsb
        clrf    address_msb

        ;; Read the period from address 31

        call    i2c_master_tx_start ; I2C Start

        movlw   0xA0
        call    i2c_master_tx_byte ; EEPROM write address

        movlw   0
        call    i2c_master_tx_byte ; Address MSB (0)

        movlw   30
        call    i2c_master_tx_byte ; Address LSB (30)

        call    i2c_master_tx_restart

        movlw   0xa1
        call    i2c_master_tx_byte ; EEPROM read address

        call    i2c_master_rx_last_byte
        movwf   period

        call    i2c_master_tx_stop ; I2C Stop

loop

        ;; Make 4 ADC measurements and sum them

        clrf    adc_result_msb
        clrf    adc_result_lsb

        movlw   4
        movwf   adc_counter

adc_loop

        ;; Make ADC measurement

        call    delay_20_cycles ; Wait for acquisition time (20 us)

        BANKSEL ADCON0          ; Select RAM bank

        bsf     ADCON0,GO_DONE  ; enable conversion
        btfsc   ADCON0,GO_DONE
        goto    $-1

        ;; Accumulate measurement

        movf    ADRESH,W
        addwf   adc_result_msb,F

        BANKSEL ADRESL          ; Select RAM bank
        movf    ADRESL,W
        BANKSEL ADRESH          ; Select RAM bank
        addwf   adc_result_lsb,F

        btfsc   STATUS,C
        incf    adc_result_msb,F

        decfsz  adc_counter,F
        goto    adc_loop

        ;; Check if value = 0 (no thermistor => programming connector)

        movf    adc_result_msb,W
        btfss   STATUS,Z
        goto    result_non_zero ; MSB != 0

        movlw   0xf0
        andwf   adc_result_lsb,W
        btfss   STATUS,Z
        goto    result_non_zero ; LSB != 0000xxxx

        goto    increment_address_1

result_non_zero

        ;; Calculate the difference between this result and the previous one.

        movf    prev_adc_result_msb,W
        subwf   adc_result_msb,W
        movwf   adc_result_delta_msb

        movf    prev_adc_result_lsb,W
        subwf   adc_result_lsb,W
        movwf   adc_result_delta_lsb

        btfss   STATUS,C
        decf    adc_result_delta_msb,F

        movf    adc_result_lsb,W
        movwf   prev_adc_result_lsb

        movf    adc_result_msb,W
        movwf   prev_adc_result_msb

        ;; Check for small delta (1 byte) or large one (2 bytes)

        movf    adc_result_delta_msb,F
        btfsc   STATUS,Z
        goto    delta_positive  ; MSB = 0

        comf    adc_result_delta_msb,W
        btfsc   STATUS,Z
        goto    delta_negative  ; MSB = -1

        goto    write_2

delta_negative
        movlw   b'01101111'     ; 111
        addwf   adc_result_delta_lsb,W
        btfss   STATUS,C
        goto    write_2         ; LSB + 111 < 0

        decf    adc_result_delta_lsb,F ; Fiddle negative numbers by -1 to avoid 0xff==-1 appearing.
        goto    write_1

delta_positive
        movlw   b'01110000'     ; 112
        subwf   adc_result_delta_lsb,W
        btfsc   STATUS,C
        goto    write_2         ; LSB - 112 > 0

        goto    write_1

write_2

        bsf     adc_result_msb,7; Toggle MSB to get bit pattern 1000 xxxx

        ;; Write to the EEPROM 2 bytes

        BANKSEL GPIO

        call    i2c_master_tx_start ; I2C Start

        movlw   0xA0
        call    i2c_master_tx_byte ; EEPROM write address

        movf    address_msb,W
        call    i2c_master_tx_byte ; Address MSB

        movf    address_lsb,W
        call    i2c_master_tx_byte ; Address LSB

        movf    adc_result_msb,W
        call    i2c_master_tx_byte ; Data MSB

        movf    adc_result_lsb,W
        call    i2c_master_tx_byte ; Data LSB

        call    i2c_master_tx_stop ; I2C Stop

        goto    increment_address_2

write_1

        ;; Write to the EEPROM 1 byte

        BANKSEL GPIO

        call    i2c_master_tx_start ; I2C Start

        movlw   0xA0
        call    i2c_master_tx_byte ; EEPROM write address

        movf    address_msb,W
        call    i2c_master_tx_byte ; Address MSB

        movf    address_lsb,W
        call    i2c_master_tx_byte ; Address LSB

        movf    adc_result_delta_lsb,W
        call    i2c_master_tx_byte ; Data

        call    i2c_master_tx_stop ; I2C Stop

        goto    increment_address_1

increment_address_2

        ;; Increment the address counters

        incfsz  address_lsb,F
        goto    $+2
        incf    address_msb,F

increment_address_1

        ;; Increment the address counters

        incfsz  address_lsb,F
        goto    $+2
        incf    address_msb,F

check_for_end

        ;; Check for the end of the EEPROM (32kBytes)

        btfsc   address_msb,7
        goto    sleep_forever

        ;; Check for the end of the EEPROM (8kBytes)

;        btfsc   address_msb,5
;        goto    sleep_forever

sleep_a_while

        ;; Sleep for the required amount of time

        movf    period,W
        movwf   periodcounter

sleep_loop
        sleep

        BANKSEL PIR1
        bcf     PIR1,TMR1IF     ; Clear the flag

        movlw   128
        addwf   TMR1H,F         ; Add 32768 so next interrupt is in 1 second

        decfsz  periodcounter,F
        goto    sleep_loop

        goto    loop

sleep_forever

        ;; Disable timer1 and then sleep forever

        BANKSEL PIE1
        bcf     PIE1,TMR1IE     ; Disable the timer 1 interrupt

        sleep
        goto    $-1             ; Shouldn't need this


;; Subroutines loaded from include files

        include "../devices/12f683/reset_device.inc"

        include "../devices/12f683/set_oscillator.inc"

        include "../common/delay.inc"

        include "../common/i2c.inc"

;; End

        end
