;---------------------------------------------------------------------
; File: an734_PIC16.asm
;
; Written By: Stephen Bowling, Microchip Technology
;
; Version: 1.00
;
; Assembled using Microchip Assembler
;
; Functionality:
;
; This code implements the basic functions for an I2C slave device
; using the SSP module. All I2C functions are handled in an ISR.
; Bytes written to the slave are stored in a buffer. After a number
; of bytes have been written, the master device can then read the
; bytes back from the buffer.
;
; Variables and Constants used in the program:
;
; The start address for the receive buffer is stored in the variable
; 'RXBuffer'. The length of the buffer is denoted by the constant
; value 'RX_BUF_LEN'. The current buffer index is stored in the
; variable 'Index'.
;
;--------------------------------------------------------------------
;
; The following files should be included in the MPLAB project:
;
; an734_PIC16.asm-- Main source code file
;
; 16f877a.lkr-- Linker script file
; (change this file for the device you are using)
;
;---------------------------------------------------------------------
;---------------------------------------------------------------------
; Include Files
;---------------------------------------------------------------------
#include <p16f877a.inc> ; Change to device that you are using.
;---------------------------------------------------------------------
;Constant Definitions
;---------------------------------------------------------------------
#define NODE_ADDR 0x22 ; I2C address of this node
; Change this value to address that
; you wish to use.
;---------------------------------------------------------------------
; Buffer Length Definition
;---------------------------------------------------------------------
#define RX_BUF_LEN 32 ; Length of receive buffer
;---------------------------------------------------------------------
; Variable declarations
;---------------------------------------------------------------------
    udata
    WREGsave    res 1
    STATUSsave  res 1
    FSRsave     res 1
    PCLATHsave  res 1
    Index       res 1           ; Index to receive buffer
    Temp        res 1           ;
    RXBuffer    res RX_BUF_LEN  ; Holds rec'd bytes from master 
; device.
;---------------------------------------------------------------------
; Vectors
;---------------------------------------------------------------------
STARTUP code 0x00
    nop
    goto    Startup ;
    nop             ; 0x0002
    nop             ; 0x0003
    goto    ISR     ; 0x0004
PROG code
;---------------------------------------------------------------------
; Macros
;---------------------------------------------------------------------
memset macro Buf_addr,Value,Length
    movlw   Length      ; This macro loads a range of data memory
    movwf   Temp        ; with a specified value. The starting
    movlw   Buf_addr    ; address and number of bytes are also
    movwf   FSR         ; specified.
SetNext movlw Value
    movwf   INDF
    incf    FSR,F
    decfsz  Temp,F
    goto    SetNext
    endm
LFSR macro Address,Offset   ; This macro loads the correct value
    movlw   Address         ; into the FSR given an initial data
    movwf   FSR             ; memory address and offset value.
    movf    Offset,W
    addwf   FSR,F
    endm
;---------------------------------------------------------------------
; Main Code
;---------------------------------------------------------------------
Startup
    bcf     STATUS,RP1
    bsf     STATUS,RP0
    call    Setup
    banksel WREGsave
Main clrwdt ; Clear the watchdog timer.
    goto    Main ; Loop forever.
;---------------------------------------------------------------------
; Interrupt Code
;---------------------------------------------------------------------
ISR
    movwf   WREGsave    ; Save WREG
    movf    STATUS,W    ; Get STATUS register
    banksel STATUSsave  ; Switch banks, if needed.
    movwf   STATUSsave  ; Save the STATUS register
    movf    PCLATH,W    ;
    movwf   PCLATHsave  ; Save PCLATH
    movf    FSR,W       ;
    movwf   FSRsave     ; Save FSR
    banksel PIR1
    btfss   PIR1,SSPIF  ; Is this a SSP interrupt?
    goto    $           ; No, just trap here.
    bcf     PIR1,SSPIF
    call    SSP_Handler ; Yes, service SSP interrupt.
    banksel FSRsave
    movf    FSRsave,W   ;
    movwf   FSR         ; Restore FSR
    movf    PCLATHsave,W;
    movwf   PCLATH      ; Restore PCLATH
    movf    STATUSsave,W;
    movwf   STATUS      ; Restore STATUS
    swapf   WREGsave,F  ;
    swapf   WREGsave,W  ; Restore WREG
    retfie              ; Return from interrupt.
;---------------------------------------------------------------------
Setup
;
; Initializes program variables and peripheral registers.
;---------------------------------------------------------------------
    banksel PCON
    bsf     PCON,NOT_POR
    bsf     PCON,NOT_BOR
    banksel Index       ; Clear various program variables
    clrf    Index
    clrf    PORTB
    clrf    PIR1
    banksel TRISB
    clrf    TRISB
    movlw   0x36        ; Setup SSP module for 7-bit
    banksel SSPCON
    movwf   SSPCON      ; address, slave mode
    movlw   NODE_ADDR
    banksel SSPADD
    movwf   SSPADD
    clrf    SSPSTAT
    banksel PIE1        ; Enable interrupts
    bsf     PIE1,SSPIE
    bsf     INTCON,PEIE ; Enable all peripheral interrupts
    bsf     INTCON,GIE  ; Enable global interrupts
    bcf     STATUS,RP0
    return
;---------------------------------------------------------------------
SSP_Handler
;---------------------------------------------------------------------
; The I2C code below checks for 5 states:
;---------------------------------------------------------------------
; State 1: I2C write operation, last byte was an address byte.
; SSPSTAT bits: S = 1, D_A = 0, R_W = 0, BF = 1
;
; State 2: I2C write operation, last byte was a data byte.
; SSPSTAT bits: S = 1, D_A = 1, R_W = 0, BF = 1
;
; State 3: I2C read operation, last byte was an address byte.
; SSPSTAT bits: S = 1, D_A = 0, R_W = 1 (see Appendix C for more information)
;
; State 4: I2C read operation, last byte was a data byte.
; SSPSTAT bits: S = 1, D_A = 1, R_W = 1, BF = 0
;
; State 5: Slave I2C logic reset by NACK from master.
; SSPSTAT bits: S = 1, D_A = 1, BF = 0 (see Appendix C for more information)
;
; For convenience, WriteI2C and ReadI2C functions have been used.
;----------------------------------------------------------------------
    banksel SSPSTAT
    movf    SSPSTAT,W   ; Get the value of SSPSTAT
    andlw   b'00101101' ; Mask out unimportant bits in SSPSTAT.
    banksel Temp        ; Put masked value in Temp
    movwf   Temp        ; for comparision checking.
State1:                 ; Write operation, last byte was an
    movlw   b'00001001' ; address, buffer is full.
    xorwf   Temp,W      ;
    btfss   STATUS,Z    ; Are we in State1?
    goto    State2      ; No, check for next state.....
    memset RXBuffer,0,RX_BUF_LEN ; Clear the receive buffer.
    clrf    Index       ; Clear the buffer index.
    banksel SSPBUF      ; Do a dummy read of the SSPBUF.
    movf    SSPBUF,W
    return
State2:                 ; Write operation, last byte was data,
    movlw   b'00101001' ; buffer is full.
    xorwf   Temp,W
    btfss   STATUS,Z    ; Are we in State2?
    goto    State3      ; No, check for next state.....
    LFSR RXBuffer,Index ; Point to the buffer.
    banksel SSPBUF      ; Get the byte from the SSP.
    movf    SSPBUF,W
    movwf   INDF        ; Put it in the buffer.
    incf    Index,F     ; Increment the buffer pointer.
    movf    Index,W     ; Get the current buffer index.
    sublw   RX_BUF_LEN  ; Subtract the buffer length.
    btfsc   STATUS,Z    ; Has the index exceeded the buffer length?
    clrf    Index       ; Yes, clear the buffer index.
    return
State3:                 ; Read operation, last byte was an address,
    movf    Temp,W ;
    andlw   b'00101100' ; Mask BF bit in SSPSTAT
    xorlw   b'00001100'
    btfss   STATUS,Z    ; Are we in State3?
    goto    State4      ; No, check for next state.....
    clrf    Index       ; Clear the buffer index.
    LFSR RXBuffer,Index ; Point to the buffer
    movf    INDF,W      ; Get the byte from buffer.
    call    WriteI2C    ; Write the byte to SSPBUF
    incf    Index,F     ; Increment the buffer index.
    return
State4:                 ; Read operation, last byte was data,
    banksel SSPCON      ; buffer is empty.
    btfsc   SSPCON, CKP
    goto    State5

    movlw   b'00101100'
    xorwf   Temp,W
    btfss   STATUS,Z    ; Are we in State4?
    goto    State5      ; No, check for next state....
    movf    Index,W     ; Get the current buffer index.
    sublw   RX_BUF_LEN  ; Subtract the buffer length.
    btfsc   STATUS,Z    ; Has the index exceeded the buffer length?
    clrf    Index       ; Yes, clear the buffer index.
    LFSR RXBuffer,Index ; Point to the buffer
    movf    INDF,W      ; Get the byte
    call    WriteI2C    ; Write to SSPBUF
    incf    Index,F     ; Increment the buffer index.
    return

State5:
    movf    Temp,W      ; NACK received when sending data to the master
    andlw   b'00101000' ; Mask RW bit in SSPSTAT
    xorlw   b'00101000' ;
    btfss   STATUS,Z    ;
    goto    I2CErr  ;
    return              ; If we aren’t in State5, then something is
                        ; wrong.
I2CErr nop
    banksel PORTB       ; Something went wrong! Set LED
    bsf     PORTB,7     ; and loop forever. WDT will reset
    goto    $           ; device, if enabled.
    return
;---------------------------------------------------------------------
; WriteI2C
;---------------------------------------------------------------------
WriteI2C
    banksel SSPSTAT
    btfsc   SSPSTAT,BF  ; Is the buffer full?
    goto    WriteI2C    ; Yes, keep waiting.
    banksel SSPCON      ; No, continue.
DoI2CWrite
    bcf     SSPCON,WCOL ; Clear the WCOL flag.
    movwf   SSPBUF      ; Write the byte in WREG
    btfsc   SSPCON,WCOL ; Was there a write collision?
    goto    DoI2CWrite
    bsf     SSPCON,CKP  ; Release the clock.
    return

end