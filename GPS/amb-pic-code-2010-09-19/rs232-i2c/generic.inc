;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/projects/04_rs232-to-i2c/RCS/generic.inc,v 1.5 2007/05/07 18:32:16 amb Exp $
;;
;; Include file with generic RS232 <-> I2C functions.
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
;; Generic I2C command.
;; * Read from RS232 into RAM
;; * Send from RAM to I2C (including read commands writing to RAM)
;; * Copy from RAM to RS232
;; 

START_A1        EQU  0x20       ; 32
END_A1          EQU  0x5B       ; 91 => 60 bytes of data inclusive

        cblock  0x5c
                count
                end_a
        endc

STATE_START     EQU  'S'
STATE_STOP      EQU  'P'
STATE_RESTART   EQU  'R'
STATE_WRITE_C   EQU  'w'
STATE_WRITE_D   EQU  'x'
STATE_READ_C    EQU  'r'
STATE_READ_D    EQU  's'

generic_i2c

        ;; Reset variables

        movlw   START_A1
        movwf   FSR

        movlw   STATE_START
        movwf   state
        movwf   INDF

        ;; Read bytes from RS232 and process them.

rs232_rx_loop
        incf    FSR,F

        movlw   END_A1+1
        subwf   FSR,W
        btfss   STATUS,Z
        goto    get_rs2332_byte

        movlw   '5'
        movwf   state
        goto    error_condition

        ;; Wait for command on RS232

get_rs2332_byte
        call    rs232_rx_byte
        movwf   INDF

;;
;; State machine that is implemented here.
;; (brackets show error number if expected state is not reached)
;;
;;             START
;;               | (2)
;;               v
;;             WRITE <---------------+
;;               | (4)               |
;;     +---------+---------+         |
;;     |         |         |         |
;;     v         v         v         |
;;   STOP       READ     RESTART     |
;;               | (4)     | (3)     |
;;               v         +---------+
;;              STOP
;;

rs232_rx_state_machine
        movlw   STATE_START
        subwf   state,W
        btfsc   STATUS,Z
        goto    rs232_rx_state_start

        movlw   STATE_RESTART
        subwf   state,W
        btfsc   STATUS,Z
        goto    rs232_rx_state_restart

        movlw   STATE_WRITE_C
        subwf   state,W
        btfsc   STATUS,Z
        goto    rs232_rx_state_write_c

        movlw   STATE_READ_C
        subwf   state,W
        btfsc   STATUS,Z
        goto    rs232_rx_state_read_c

        movlw   STATE_WRITE_D
        subwf   state,W
        btfsc   STATUS,Z
        goto    rs232_rx_state_write_d

        movlw   STATE_READ_D
        subwf   state,W
        btfsc   STATUS,Z
        goto    rs232_rx_state_read_d

        movlw   '1'
        movwf   state
        goto    error_condition

        ;; Currently in START state, expect WRITE or error

rs232_rx_state_start
        movlw   STATE_WRITE_C
        subwf   INDF,W
        btfsc   STATUS,Z
        goto    rs232_rx_state_start_write

        movlw   '2'
        movwf   state
        goto    error_condition

rs232_rx_state_start_write
        movf    INDF,W
        movwf   state
        goto    rs232_rx_loop

        ;; Currently in RESTART state, expect WRITE or error

rs232_rx_state_restart
        movlw   STATE_WRITE_C
        subwf   INDF,W
        btfsc   STATUS,Z
        goto    rs232_rx_state_start_write

        movlw   '3'
        movwf   state
        goto    error_condition

rs232_rx_state_restart_write
        movf    INDF,W
        movwf   state
        goto    rs232_rx_loop

        ;; Currently in READ or WRITE state, expect count

rs232_rx_state_write_c
rs232_rx_state_read_c
        movf    INDF,W
        movwf   count

        incf    state,F
        goto    rs232_rx_loop

        ;; Currently in WRITE state, expect data or count==0

rs232_rx_state_write_d
        movf    count,F
        btfsc   STATUS,Z
        goto    rs232_rx_state_write_d_count0

        decf    count,F
        goto    rs232_rx_loop

        ;; Currently in READ state, expect data or count==0

rs232_rx_state_read_d
        movf    count,F
        btfsc   STATUS,Z
        goto    rs232_rx_state_read_d_count0

        decf    count,F
        goto    rs232_rx_loop

        ;; Currently in WRITE state count==0, expect READ or RESTART or STOP (fallthrough)

rs232_rx_state_write_d_count0
        movlw   STATE_READ_C
        subwf   INDF,W
        btfsc   STATUS,Z
        goto    rs232_rx_state_write_read

        movlw   STATE_RESTART
        subwf   INDF,W
        btfsc   STATUS,Z
        goto    rs232_rx_state_write_restart

        ;; Currently in READ or WRITE state count==0, expect STOP

rs232_rx_state_read_d_count0
        movlw   STATE_STOP
        subwf   INDF,W
        btfsc   STATUS,Z
        goto    rs232_rx_state_rw_stop

        movlw   '4'
        movwf   state
        goto    error_condition

        ;; Currently in READ or WRITE state changing to READ or RESTART

rs232_rx_state_write_read
rs232_rx_state_write_restart
        movf    INDF,W
        movwf   state
        goto    rs232_rx_loop

        ;; Currently in READ or WRITE state changing to STOP

rs232_rx_state_rw_stop
        movlw   'O'
        call    rs232_tx_byte

        movlw   'K'
        call    rs232_tx_byte

        movlw   10
        call    rs232_tx_byte

        ;; Make a note of the end point

        movf    FSR,W
        movwf   end_a

        ;; Reset variables

        movlw   STATE_START
        movwf   state

        movlw   START_A1-1
        movwf   FSR

        ;; Output the loaded information on I2C

i2c_loop

        incf    FSR,F

;;
;; State machine that is implemented here.
;;
;;             START
;;               |
;;               v
;;             WRITE <---------------+
;;               |                   |
;;     +---------+---------+         |
;;     |         |         |         |
;;     v         v         v         |
;;   STOP       READ     RESTART     |
;;               |         |         |
;;               v         +---------+
;;              STOP
;;

i2c_state_machine
        movlw   STATE_START
        subwf   state,W
        btfsc   STATUS,Z
        goto    i2c_state_start

        movlw   STATE_RESTART
        subwf   state,W
        btfsc   STATUS,Z
        goto    i2c_state_restart

        movlw   STATE_WRITE_C
        subwf   state,W
        btfsc   STATUS,Z
        goto    i2c_state_write_c

        movlw   STATE_READ_C
        subwf   state,W
        btfsc   STATUS,Z
        goto    i2c_state_read_c

        movlw   STATE_WRITE_D
        subwf   state,W
        btfsc   STATUS,Z
        goto    i2c_state_write_d

        movlw   STATE_READ_D
        subwf   state,W
        btfsc   STATUS,Z
        goto    i2c_state_read_d

        ;; else must be in state STOP

        goto    i2c_state_stop

        ;; Currently in START state

i2c_state_start
        call    i2c_master_tx_start

        movf    INDF,W
        movwf   state

        goto    i2c_loop

        ;; Currently in RESTART state

i2c_state_restart
        call    i2c_master_tx_restart

        movf    INDF,W
        movwf   state

        goto    i2c_loop

        ;; Currently in READ or WRITE state (counter value)

i2c_state_write_c
i2c_state_read_c
        movf    INDF,W
        movwf   count

        incf    state,F

        goto    i2c_loop

        ;; Currently in WRITE state (data)

i2c_state_write_d
        movf    count,F
        btfsc   STATUS,Z
        goto    i2c_state_write_d_count0

        movf    INDF,W
        call    i2c_master_tx_byte

        decf    count,F
        goto    i2c_loop

        ;; Currently in READ state (data)

i2c_state_read_d
        movf    count,F
        btfsc   STATUS,Z
        goto    i2c_state_read_d_count0

        decf    count,F

        btfsc   STATUS,Z
        goto    i2c_state_read_last_d

i2c_state_read_not_last_d
        call    i2c_master_rx_byte
        movwf   INDF

        goto    i2c_loop

i2c_state_read_last_d
        call    i2c_master_rx_last_byte
        movwf   INDF

        goto    i2c_loop

        ;; Currently in WRITE state count==0, could be READ or RESTART

i2c_state_write_d_count0
        movlw   STATE_READ_C
        subwf   INDF,W
        btfsc   STATUS,Z
        goto    i2c_state_write_read

        movlw   STATE_RESTART
        subwf   INDF,W
        btfsc   STATUS,Z
        goto    i2c_state_write_restart

        ;; Currently in READ state count==0, must be STOP

i2c_state_read_d_count0
        goto    i2c_state_rw_stop

        ;; Currently in READ or WRITE state changing to new state

i2c_state_write_read
i2c_state_write_restart
i2c_state_rw_stop
        movf    INDF,W
        movwf   state
        goto    i2c_loop

        ;; Currently in STOP state

i2c_state_stop
        call    i2c_master_tx_stop

        ;; Reset variables

        movlw   START_A1-1
        movwf   FSR

rs232_tx_loop
        incf    FSR,F

        movf    INDF,W
        call    rs232_tx_byte

        movf    end_a,W
        subwf   FSR,W
        btfss   STATUS,Z
        goto    rs232_tx_loop

        movlw   10
        call    rs232_tx_byte

        ;; The end, start again

        return

        ;; Error occured

error_condition
        movlw   'E'
        call    rs232_tx_byte

        movf    state,W
        call    rs232_tx_byte

        movlw   10
        call    rs232_tx_byte

        return
