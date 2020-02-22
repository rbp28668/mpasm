;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/16f819/RCS/reset_device.inc,v 1.4 2007/05/07 18:30:52 amb Exp $
;;
;; Function to reset 16F818/16F819 devices.
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

        IFNDEF __16F819
        IFNDEF __16F818
           ERROR "Processor-header file mismatch.  Verify selected processor."
        ENDIF
        ENDIF

;; Temporarily disable messages and warnings

        ERRORLEVEL      2

;;
;; Subroutine to reset the device registers
;;

reset_device

;; Clear analogue inputs

disable_analogue

        BANKSEL ADCON0          ; Select RAM bank
        clrf    ADCON0          ; Clear ADC register

        BANKSEL ADCON1          ; Select RAM bank
        movlw   0x06            ; Select bits to disable analogue inputs
        movwf   ADCON1          ; Load into ADC register

;; Disable digital outputs on port A

disable_digital_out_port_a

        BANKSEL PORTA           ; Select RAM bank
        clrf    PORTA           ; Clear port A I/O register

        BANKSEL TRISA           ; Select RAM bank
        movlw   b'11111111'     ; Select all bits as inputs
        movwf   TRISA           ; Write to tristate register

;; Disable digital outputs on port B

disable_digital_out_port_b

        BANKSEL PORTB           ; Select RAM bank
        clrf    PORTB           ; Clear port B I/O register

        BANKSEL OPTION_REG      ; Select RAM bank
        bsf     OPTION_REG,NOT_RBPU ; Set the /RBPU bit in the option register

        BANKSEL TRISB           ; Select RAM bank
        movlw   b'11111111'     ; Select all bits as inputs
        movwf   TRISB           ; Write to tristate register

;; Disable timer 0

disable_timer0

        BANKSEL INTCON          ; Select RAM bank
        bcf     INTCON,TMR0IF   ; Clear Timer 0 interrupt
        bcf     INTCON,TMR0IE   ; Disable Timer 0 interrupt

;; Disable timer 1

disable_timer1

        BANKSEL T1CON           ; Select RAM bank
        clrf    T1CON           ; Clear timer 1 register

;; Disable timer 2

disable_timer2

        BANKSEL T2CON           ; Select RAM bank
        clrf    T2CON           ; Clear timer 2 register

;; Disable Capture/Compare/PWM port

disable_ccp

        BANKSEL CCP1CON         ; Select RAM bank
        clrf    CCP1CON         ; Clear CCP register

;; Disable synchronous serial port

disable_ssp

        BANKSEL SSPCON          ; Select RAM bank
        clrf    SSPCON          ; Clear SSP register

;; Disable all interrupts

disable_all_interrupts

        BANKSEL INTCON          ; Select RAM bank
        clrf    INTCON          ; Clear interrupt control register

;; Finished

        return

;; Re-enable messages and warnings

        ERRORLEVEL      0
