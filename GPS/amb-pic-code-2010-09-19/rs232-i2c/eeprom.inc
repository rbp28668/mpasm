;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/projects/04_rs232-to-i2c/RCS/eeprom.inc,v 1.4 2007/05/07 18:32:16 amb Exp $
;;
;; Include file with high level EEPROM functions.
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2006 Andrew M. Bishop
;; It may be distributed under the GNU Public License, version 2, or
;; any higher version.  See section COPYING of the GNU Public license
;; for conditions under which this file may be redistributed.
;;
;;--------------------------------------------------------------------------------

;; 
;; EEPROM I2C commands.
;; * Dump contents of EEPROM to RS232
;; * Load contents of EEPROM from RS232
;; * Erase EEPROM
;; 

START_A2        EQU  0x20       ; 32
END_A2          EQU  0x3F       ; 63 => 32 bytes of data inclusive

        cblock  0x50
                length_lsb
                length_msb
                count_lsb
                count_msb
        endc

;; Load the EEPROM with data from RS232

eeprom_load

        call    eeprom_get_length

eeprom_load_loop

        ;; Read 32 bytes from RS232 to RAM

        movlw   START_A2
        movwf   FSR

eeprom_rs232_to_ram_loop

        call    rs232_rx_byte
        movwf   INDF

        incf    FSR,F
        movlw   END_A2+1
        subwf   FSR,W
        btfss   STATUS,Z
        goto    eeprom_rs232_to_ram_loop

        ;; Write 32 bytes from RAM to EEPROM

        call    i2c_master_tx_start

        movlw   0xa0            ; EEPROM write address
        call    i2c_master_tx_byte

        movf    count_msb,W
        call    i2c_master_tx_byte

        movf    count_lsb,W
        call    i2c_master_tx_byte

        movlw   START_A2
        movwf   FSR

eeprom_ram_to_eeprom_loop

        movf    INDF,W
        call    i2c_master_tx_byte

        incf    FSR,F
        movlw   END_A2+1
        subwf   FSR,W
        btfss   STATUS,Z
        goto    eeprom_ram_to_eeprom_loop

        call    i2c_master_tx_stop

        movlw   5
        call    delay_ms        ; Wait for write to complete (max time in datasheet)

        ;; Increment address counters, compare to size counters

        movlw   32
        addwf   count_lsb,F
        btfsc   STATUS,C
        incf    count_msb,F

        movf    count_lsb,W
        subwf   length_lsb,W
        btfss   STATUS,Z
        goto    eeprom_load_loop

        movf    count_msb,W
        subwf   length_msb,W
        btfss   STATUS,Z
        goto    eeprom_load_loop

        call    eeprom_ok

        return

;; Dump the EEPROM data to RS232

eeprom_dump

        call    eeprom_get_length

eeprom_dump_loop

        ;; Read 32 bytes from EEPROM to RAM

        call    i2c_master_tx_start

        movlw   0xa0            ; EEPROM write address
        call    i2c_master_tx_byte

        movf    count_msb,W
        call    i2c_master_tx_byte

        movf    count_lsb,W
        call    i2c_master_tx_byte

        call    i2c_master_tx_restart

        movlw   0xa1            ; EEPROM read address
        call    i2c_master_tx_byte

        movlw   START_A2
        movwf   FSR

eeprom_eeprom_to_ram_loop

        call    i2c_master_rx_byte
        movwf   INDF

        incf    FSR,F
        movlw   END_A2
        subwf   FSR,W
        btfss   STATUS,Z
        goto    eeprom_eeprom_to_ram_loop

        call    i2c_master_rx_last_byte
        movwf   INDF

        call    i2c_master_tx_stop

        ;; Write 32 bytes from RAM to RS232

        movlw   START_A2
        movwf   FSR

eeprom_ram_to_rs232_loop

        movf    INDF,W
        call    rs232_tx_byte

        incf    FSR,F
        movlw   END_A2+1
        subwf   FSR,W
        btfss   STATUS,Z
        goto    eeprom_ram_to_rs232_loop

        ;; Increment address counters, compare to size counters

        movlw   32
        addwf   count_lsb,F
        btfsc   STATUS,C
        incf    count_msb,F

        movf    count_lsb,W
        subwf   length_lsb,W
        btfss   STATUS,Z
        goto    eeprom_dump_loop

        movf    count_msb,W
        subwf   length_msb,W
        btfss   STATUS,Z
        goto    eeprom_dump_loop

        call    eeprom_ok

        return

;; Erase the EEPROM

eeprom_erase

        call    eeprom_get_length

eeprom_erase_loop

        ;; Write 32 bytes of 0xff to EEPROM

        call    i2c_master_tx_start

        movlw   0xa0            ; EEPROM write address
        call    i2c_master_tx_byte

        movf    count_msb,W
        call    i2c_master_tx_byte

        movf    count_lsb,W
        call    i2c_master_tx_byte

        movlw   START_A2
        movwf   FSR

eeprom_erase32_loop

        movlw   0xff
        call    i2c_master_tx_byte

        incf    FSR,F
        movlw   END_A2+1
        subwf   FSR,W
        btfss   STATUS,Z
        goto    eeprom_erase32_loop

        call    i2c_master_tx_stop

        movlw   5
        call    delay_ms        ; Wait for write to complete (max time in datasheet)

        ;; Increment address counters, compare to size counters

        movlw   32
        addwf   count_lsb,F
        btfsc   STATUS,C
        incf    count_msb,F

        movf    count_lsb,W
        subwf   length_lsb,W
        btfss   STATUS,Z
        goto    eeprom_erase_loop

        movf    count_msb,W
        subwf   length_msb,W
        btfss   STATUS,Z
        goto    eeprom_erase_loop

        call    eeprom_ok

        return

;; Get the EEPROM length from RS232

eeprom_get_length

        ;; Read two bytes from RS232

        call    rs232_rx_byte
        movwf   length_msb

        call    rs232_rx_byte
        movwf   length_lsb

        movlw   0xe0
        andwf   length_lsb,F

        clrf    count_msb
        clrf    count_lsb

        return

;; Print the OK message at the end.

eeprom_ok

        movlw   'O'
        call    rs232_tx_byte

        movlw   'K'
        call    rs232_tx_byte

        movlw   10
        call    rs232_tx_byte

        return
