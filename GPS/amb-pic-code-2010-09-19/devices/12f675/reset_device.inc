;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/12f675/RCS/reset_device.inc,v 1.3 2007/05/07 18:29:28 amb Exp $
;;
;; Reset function for 12F675 devices.
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2006 Andrew M. Bishop
;; It may be distributed under the GNU Public License, version 2, or
;; any higher version.  See section COPYING of the GNU Public license
;; for conditions under which this file may be redistributed.
;;
;;--------------------------------------------------------------------------------

;; Verify Processor Type

        IFNDEF __12F675
           ERROR "Processor-header file mismatch.  Verify selected processor."
        ENDIF

;; Temporarily disable messages and warnings

        ERRORLEVEL      2


;;
;; Subroutine to reset the device registers
;;

reset_device

;; Initialise clock oscillator calibration

set_osccal

        call    0x3ff           ; Load 'w' with oscillator calibration
        BANKSEL OSCCAL          ; Select RAM bank
        movwf   OSCCAL          ; Load oscillator calibration register

;; Clear analogue inputs and turn off comparator

disable_analogue

        BANKSEL ANSEL           ; Select RAM bank
        clrf    ANSEL           ; Clear analogue selection register

        BANKSEL CMCON           ; Select RAM bank
        movlw   b'00000111'     ; Set bits to disable comparator
        movwf   CMCON           ; Load comparator register

        BANKSEL VRCON           ; Select RAM bank
        clrf    VRCON           ; Clear voltage reference register

        BANKSEL ADCON0          ; Select RAM bank
        clrf    ADCON0          ; Clear ADC register

;; Disable digital outputs

disable_digital_out

        BANKSEL GPIO            ; Select RAM bank
        clrf    GPIO            ; Clear general purpose I/O register

        BANKSEL WPU             ; Select RAM bank
        clrf    WPU             ; Clear weak pullup register

        BANKSEL OPTION_REG      ; Select RAM bank
        bsf     OPTION_REG,NOT_GPPU ; Set the /GPPU bit in the option register

        BANKSEL TRISIO          ; Select RAM bank
        movlw   b'00111111'     ; Select all bits as inputs
        movwf   TRISIO          ; Write to tristate register

;; Clear input interrupt on change

disable_ioc

        BANKSEL IOC             ; Select RAM bank
        clrf    IOC             ; Clear interrupt on change register

;; Disable timer 0

disable_timer0

        BANKSEL INTCON          ; Select RAM bank
        bcf     INTCON,T0IF     ; Clear Timer 0 interrupt
        bcf     INTCON,T0IE     ; Disable Timer 0 interrupt

;; Disable timer 1

disable_timer1

        BANKSEL T1CON           ; Select RAM bank
        clrf    T1CON           ; Clear timer 1 register

;; Disable all interrupts

disable_all_interrupts

        BANKSEL INTCON          ; Select RAM bank
        clrf    INTCON          ; Clear interrupt control register

;; Finished

        return

;; Re-enable messages and warnings

        ERRORLEVEL      0
