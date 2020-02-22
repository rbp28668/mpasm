;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/projects/11_gps-sd-logger/RCS/gps.inc,v 1.9 2007/07/08 15:25:34 amb Exp $
;;
;; Include file with GPS functions.
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2007 Andrew M. Bishop
;; It may be distributed under the GNU Public License, version 2, or
;; any higher version.  See section COPYING of the GNU Public license
;; for conditions under which this file may be redistributed.
;;
;;--------------------------------------------------------------------------------

;; 
;; GPS functions
;; 


;;--------------------------------------------------------------------------------

GPS_power_up

        ;; Turn on power

        BANKSEL GPS_PORT           ; Select RAM bank
        bcf     GPS_PORT,GPS_POWER ; Turn on GPS power

        movlw   10
        call    delay_ms

        ;; Enable RS232 port

        call    rs232_init_port

        ;; Remove reset

        bsf     GPS_PORT,GPS_RESET ; Remove GPS reset

        ;; Set the power flag

        bsf     status,GPS_PWR

        ;; Finished
        return


;;--------------------------------------------------------------------------------

GPS_power_down

        ;; Apply reset

        BANKSEL GPS_PORT           ; Select RAM bank
        bcf     GPS_PORT,GPS_RESET ; Apply GPS reset

        ;; Set RS232 output low

        BANKSEL RS232_PORT      ; Select RAM bank
        bcf     RS232_PORT,RS232_TXD

        ;; Turn off power

        BANKSEL GPS_PORT           ; Select RAM bank
        bsf     GPS_PORT,GPS_POWER ; Turn off GPS power

        ;; Clear the power flag

        bcf     status,GPS_PWR

        ;; Finished
        return


;;--------------------------------------------------------------------------------

GPS_start

        ;; wait for first string from GPS

GPS_start_loop0
        call    rs232_rx_byte

        xorlw   '\n'
        btfss   STATUS,Z        ; skip if matches '\n'
        goto    GPS_start_loop0

        movlw   10
        call    delay_ms

        ;; Output the initialisation string 1

        movlw   gps_eeprom_init1
        movwf   counter

GPS_start_loop1
        movf    counter,W
        call    eeprom_read

        xorlw   0x00
        btfsc   STATUS,Z        ; skip if not null character
        goto    GPS_start_2

        call    rs232_tx_byte

        incf    counter,F
        goto    GPS_start_loop1

GPS_start_2

        movlw   10
        call    delay_ms

        ;; Output the initialisation string 2

        movlw   gps_eeprom_init2
        movwf   counter

GPS_start_loop2
        movf    counter,W
        call    eeprom_read

        xorlw   0x00
        btfsc   STATUS,Z        ; skip if not null character
        goto    GPS_start_3

        call    rs232_tx_byte

        incf    counter,F
        goto    GPS_start_loop2


GPS_start_3

        movlw   10
        call    delay_ms

        ;; Output the initialisation string 3

        movlw   gps_eeprom_init3
        movwf   counter

GPS_start_loop3
        movf    counter,W
        call    eeprom_read

        xorlw   0x00
        btfsc   STATUS,Z        ; skip if not null character
        goto    GPS_start_4

        call    rs232_tx_byte

        incf    counter,F
        goto    GPS_start_loop3

GPS_start_4

        movlw   10
        call    delay_ms

        ;; Clear the sync flag

        bcf     status,GPS_SYNC

        ;; Finished
        return


;;--------------------------------------------------------------------------------

GPS_finish

        ;; Clear the sync flag

        bcf     status,GPS_SYNC

        ;; Finished
        return


;;--------------------------------------------------------------------------------

GPS_log

        movlw   0
        movwf   counter

        clrf    gps_match

        ;; Get bytes and write to SD, stop after GSA message or 512 bytes

GPS_log_get_byte_odd

        ;;  Get RS232 byte

        call    rs232_rx_byte
        movwf   rs232_temp

        ;; Ignore '\r' character

        xorlw   '\r'
        btfsc   STATUS,Z        ; skip if doesn't match
        goto    GPS_log_get_byte_odd

        ;; Parse data

        call    GPS_match_GSA

        ;; Send SD card byte

        btfss   status,GPS_SYNC ; skip if GPS is synchronised
        goto    GPS_log_skip_odd

        btfss   status,SD_PWR   ; skip if power is on
        goto    GPS_log_skip_odd

        movf    counter,F
        btfss   STATUS,Z        ; skip if counter=0 (first time through loop)
        call    spi_rx_byte     ; Receive previous byte (clears SPI Rx side)

        movf    rs232_temp,W
        call    spi_tx_byte

GPS_log_skip_odd

        ;; Check for end condition

        btfsc   gps_match,7 ; skip if not seen end of GSA message
        goto    GPS_log_data_end_odd

GPS_log_get_byte_even

        ;;  Get RS232 byte

        call    rs232_rx_byte
        movwf   rs232_temp

        ;; Ignore '\r' character

        xorlw   '\r'
        btfsc   STATUS,Z        ; skip if doesn't match
        goto    GPS_log_get_byte_even

        ;; Parse data

        call    GPS_match_GSA

        ;; Send SD card byte

        btfss   status,GPS_SYNC ; skip if GPS is synchronised
        goto    GPS_log_skip_even

        btfss   status,SD_PWR   ; skip if power is on
        goto    GPS_log_skip_even

        call    spi_rx_byte ; Receive previous byte (clears SPI Rx side)

        movf    rs232_temp,W
        call    spi_tx_byte

GPS_log_skip_even

        ;; Check for end conditions

        btfsc   gps_match,7 ; skip if not seen end of GSA message
        goto    GPS_log_data_end_even

        decfsz  counter,F       ; skip if counter=0 after decrement (last loop finished)
        goto    GPS_log_get_byte_odd

GPS_log_data_error

        ;; Something went wrong and 512 bytes were seen without end of GSA message.

        btfss   status,GPS_SYNC ; skip if GPS is synchronised
        retlw   0               ; not synchronised next time

        btfss   status,SD_PWR   ; skip if power is on
        retlw   0               ; not synchronised next time

        call    spi_wait_rx_byte; Receive previous byte

        retlw   0               ; not synchronised next time

GPS_log_data_end_odd

        ;; End of data, odd number read

        btfss   status,GPS_SYNC ; skip if GPS is synchronised
        retlw   1               ; synchronised next time

        btfss   status,SD_PWR   ; skip if power is on
        retlw   1               ; synchronised next time

        call    spi_wait_rx_byte; Receive previous byte

        movlw   0
        call    spi_tx_byte

GPS_log_data_end_even

        ;; End of data, even number read

        btfss   status,GPS_SYNC ; skip if GPS is synchronised
        retlw   1               ; synchronised next time

        btfss   status,SD_PWR   ; skip if power is on
        retlw   1               ; synchronised next time

        call    spi_wait_rx_byte; Receive previous byte

        decfsz  counter,F       ; skip if counter=0 after decrement (last loop finished)
        goto    GPS_log_pad_with_zeros

        retlw   1               ; synchronised next time

GPS_log_pad_with_zeros

        movlw   0
        call    spi_tx_wait_rx_byte

        movlw   0
        call    spi_tx_wait_rx_byte

        decfsz  counter,F       ; skip if counter=0 after decrement (last loop finished)
        goto    GPS_log_pad_with_zeros

        ;; Finished
        retlw   1               ; synchronised next time


;;--------------------------------------------------------------------------------
;; 
;; State machine state
;; at entry to function: 0 1 2 3 4 5 6 7 8 9 A A A A A
;;
;; Text matched so far:    $ G P G S A , * , ? * * * \n
;;                               1st target--^       ^-- 2nd target


GPS_match_GSA

        ;; Check if it is $GPGSA

        ;; Switch statement taking care of all 13 bits (see Microchip App note 556)

        movf    gps_match,W
        andlw   0x0f
        movwf   gps_temp

        movlw   LOW GPS_match_switch; low 8 bits of address
        addwf   gps_temp,F      ; 8 bit addition

        movlw   HIGH GPS_match_switch;high 5 bits of address
        btfsc   STATUS,C        ; skip if no 8 bit add overflow
        addlw   1
        movwf   PCLATH          ; load high address in latch

        movf    gps_temp,W
        goto    $+1
        movwf   PCL

GPS_match_switch
        goto    GPS_match_GSA_0 ; case 0
        goto    GPS_match_GSA_1 ; case 1
        goto    GPS_match_GSA_2 ; case 2
        goto    GPS_match_GSA_3 ; case 3
        goto    GPS_match_GSA_4 ; case 4
        goto    GPS_match_GSA_5 ; case 5
        goto    GPS_match_GSA_6 ; case 6
        goto    GPS_match_GSA_7 ; case 7
        goto    GPS_match_GSA_8 ; case 8
        goto    GPS_match_GSA_9 ; case 9
        goto    GPS_match_GSA_10; case 10
        return                  ; case 11
        return                  ; case 12
        return                  ; case 13
        return                  ; case 14
        return                  ; case 15

GPS_match_GSA_0
        movlw   '$'
        xorwf   rs232_temp,W
        btfsc   STATUS,Z        ; skip if doesn't match '$'
        incf    gps_match,F     ; increment match counter
        return

GPS_match_GSA_1
        incf    gps_match,F     ; increment match counter
        movlw   'G'
        xorwf   rs232_temp,W
        btfss   STATUS,Z        ; skip if matches 'G'
        clrf    gps_match       ; reset counter
        return

GPS_match_GSA_2
        incf    gps_match,F     ; increment match counter
        movlw   'P'
        xorwf   rs232_temp,W
        btfss   STATUS,Z        ; skip if matches 'P'
        clrf    gps_match       ; reset counter
        return

GPS_match_GSA_3
        incf    gps_match,F     ; increment match counter
        movlw   'G'
        xorwf   rs232_temp,W
        btfss   STATUS,Z        ; skip if matches 'G'
        clrf    gps_match       ; reset counter
        return

GPS_match_GSA_4
        incf    gps_match,F     ; increment match counter
        movlw   'S'
        xorwf   rs232_temp,W
        btfss   STATUS,Z        ; skip if matches 'S'
        clrf    gps_match       ; reset counter
        return

GPS_match_GSA_5
        incf    gps_match,F     ; increment match counter
        movlw   'A'
        xorwf   rs232_temp,W
        btfss   STATUS,Z        ; skip if matches 'A'
        clrf    gps_match       ; reset counter
        return

GPS_match_GSA_6
        incf    gps_match,F     ; increment match counter
        movlw   ','
        xorwf   rs232_temp,W
        btfss   STATUS,Z        ; skip if matches ','
        clrf    gps_match       ; reset counter
        return

GPS_match_GSA_7
        incf    gps_match,F     ; increment match counter
        return

GPS_match_GSA_8
        incf    gps_match,F     ; increment match counter
        movlw   ','
        xorwf   rs232_temp,W
        btfss   STATUS,Z        ; skip if matches ','
        clrf    gps_match       ; reset counter
        return

GPS_match_GSA_9
        ;; Decide which LED should be on; red, orange or green

        btfsc   rs232_temp,1    ; skip if bit 1 is not set ('1')
        bsf     status,GPS_FIX_G

        movlw   '3'
        xorwf   rs232_temp,W
        btfss   STATUS,Z        ; skip if matches '3'
        bsf     status,GPS_FIX_R

        incf    gps_match,F     ; increment match counter

        return

GPS_match_GSA_10
        movlw   '\n'
        xorwf   rs232_temp,W
        btfss   STATUS,Z        ; skip if matches '\n'
        return

        clrf    gps_match       ; reset counter
        bsf     gps_match,7     ; set bit 7

        return


;;--------------------------------------------------------------------------------
