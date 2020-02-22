;---------------------------------------------------------------------
; File: an734_PIC18.asm
;
; The following files should be included in the MPLAB project:
;;
; an734_PIC18.asm-- Main source code file
;;
; 18F8722.lkr-- Linker script file
; (change this file for the device you are using)
;
;---------------------------------------------------------------------
#define RX_BUF_LEN 32
ADDRESS equ 0x22
    udata   0x00
FSRsave     res 1
PCLATHsave  res 1
Index       res 1
Temp        res 1
RXBuffer    res RX_BUF_LEN
;---------------------------------------------------------------------
; Include Files
;---------------------------------------------------------------------
#include<p18F8722.inc>
    CONFIG OSC = HS,FCMEN = OFF,IESO = OFF,PWRT = OFF,BOREN = OFF
    CONFIG WDT = OFF
    CONFIG STVREN = OFF, LVP = OFF,XINST = OFF,DEBUG = OFF
    CONFIG CP0 = OFF,CP1 = OFF,CP2 = OFF,CP3 = OFF,CPB = OFF
memset macro Buf_addr,Value,Length
    movlw   Length      ; This macro loads a range of data memory
    movwf   Temp        ; with a specified value. The starting
    movlw   Buf_addr    ; address and number of bytes are also
    movwf   FSR0L       ; specified.
SetNext
    movlw   Value
    movwf   INDF0
    incf    FSR0L,F
    decfsz  Temp,F
    goto    SetNext
    endm
load macro Address,Offset   ; This macro loads the correct value
    movlw   Address         ; into the FSR given an initial data
    movwf   FSR0L           ; memory address and offset value.
    movf    Offset,W
    addwf   FSR0L,F
    endm

PRG     CODE 0x00
    goto    Start
INT1    CODE 0x08
    goto    Int
INT2    CODE 0x18
    goto    Int
MAIN    CODE 0x30
;---------------------------------------------------------------------
; Main Code
;---------------------------------------------------------------------
Start
    clrf    Index       ;res 1
    clrf    Temp        ;res 1
    clrf    RXBuffer    ;res RX_BUF_LEN
    call    Setup
Main
    goto    Main
Setup
    bsf     TRISC,3
    bsf     TRISC,4
    clrf    FSR0L
    clrf    FSR0H
    movlw   ADDRESS     ;Load Address , Slave node
    movwf   SSP1ADD
    movlw   0x36
    movwf   SSP1CON1
    clrf    SSP1STAT
    clrf    SSP1CON2
    bsf     SSP1CON2,SEN ;Enable Clock Stretching for both transmit and slave
    bcf     PIR1,SSPIF  ;Clear MSSP interrupt flag
    bsf     PIE1,SSPIE  ;Enable MSSP interrupt enable bit
    movlw   0xC0        ;Enable global and peripheral Interrupt
    movwf   INTCON
    return
;---------------------------------------------------------------------
; Interrupt Code
;---------------------------------------------------------------------
Int
    movf    FSR0L,W     ;
    movwf   FSRsave     ; Save FSR
    btfss   PIR1,SSPIF  ; Is this a SSP interrupt?
    goto    $           ; No, just trap here.
    bcf     PIR1,SSPIF
    call    SSP_Handler ; Yes, service SSP interrupt.
    movf    FSRsave,W   ;
    movwf   FSR0L       ; Restore FSR
    bsf     SSPCON1,CKP ; Release clock( for transmit and receive)
    retfie  FAST        ; Return from interrupt

;----------------------------------------------------------------
; State 1: I2C write operation, last byte was an address byte
; SSPSTAT bits: S = 1, D_A = 0, R_W = 0, BF = 1
;
; State 2: I2C write operation, last byte was a data byte
; SSPSTAT bits: S = 1, D_A = 1, R_W = 0, BF = 1
;
; State 3: I2C read operation, last byte was an address byte
; SSPSTAT bits: S = 1, D_A = 0, R_W = 1 (see Appendix C for more information)
;
; State 4: I2C read operation, last byte was a data byte
; SSPSTAT bits: S = 1, D_A = 1, R_W = 1, BF = 0
;
; State 5: Slave I2C logic reset by NACK from master
; SSPSTAT bits: S = 1, D_A = 1, BF = 0, CKP = 1 (see Appendix C for more information)
; For convenience, WriteI2C and ReadI2C functions have been used.
;-----------------------------------------------------------------
SSP_Handler
    movf    SSPSTAT,W       ; Get the value of SSPSTAT
    andlw   b'00101101'     ; Mask out unimportant bits in SSPSTAT.
    movwf   Temp            ; for comparision checking.
State1:                     ; Write operation, last byte was an
    movlw   b'00001001'     ; address, buffer is full.
    xorwf   Temp,W          ;
    btfss   STATUS,Z        ; Are we in State1?
    goto    State2          ; No, check for next state.....
    memset RXBuffer,0,RX_BUF_LEN ; Clear the receive buffer.
    clrf    Index           ; Clear the buffer index.
    movf    SSPBUF,W        ; Do a dummy read of the SSPBUF.
    return
State2:                     ; Write operation, last byte was data,
    movlw   b'00101001'     ; buffer is full.
    xorwf   Temp,W
    btfss   STATUS,Z        ; Are we in State2?
    goto    State3          ; No, check for next state.....
    load RXBuffer,Index     ; Point to the buffer.
    movf    SSPBUF,W        ; Get the byte from the SSP.
    movwf   INDF0           ; Put it in the buffer.
    incf    Index,F         ; Increment the buffer pointer.
    movf    Index,W         ; Get the current buffer index.
    sublw   RX_BUF_LEN      ; Subtract the buffer length.
    btfsc   STATUS,Z        ; Has the index exceeded the buffer length?
    clrf    Index
    return
State3:
    movf    Temp,W ;
    andlw   b'00101100'     ; Mask BF bit in SSPSTAT
    xorlw   b'00001100'
    btfss   STATUS,Z        ; Are we in State3?
    goto    State4          ; No, check for next state.....
    movf    SSPBUF,W
    clrf    Index           ; Clear the buffer index.
    load    RXBuffer,Index  ; Point to the buffer
    movf    INDF0,W         ; Get the byte from buffer.
    call    WriteI2C        ; Write the byte to SSPBUF
    incf    Index,F         ; Increment the buffer index.
    return

State4
    btfsc   SSPCON1,CKP ;
    goto    State5
    movlw   b'00101100'     ; buffer is empty.
    xorwf   Temp,W
    btfss   STATUS,Z        ; Are we in State4?
    goto    State5          ; No, check for next state....
    movf    Index,W         ; Get the current buffer index.
    sublw   RX_BUF_LEN      ; Subtract the buffer length.
    btfsc   STATUS,Z        ; Has the index exceeded the buffer length?
    clrf    Index           ; Yes, clear the buffer index.
    load    RXBuffer,Index  ; Point to the buffer
    movf    INDF0,W         ; Get the byte
    call    WriteI2C        ; Write to SSPBUF
    incf    Index,F         ; Increment the buffer index.
    return
State5
    movf    Temp,W ;
    andlw   b'00101000'     ; Mask RW bit in SSPSTAT
    xorlw   b'00101000'
    btfss   STATUS,Z        ; Are we in State5?
    goto    I2CErr          ; No, check for next state....
    return
I2CErr
    nop                     ; Something went wrong! Set LED
    bsf     PORTB,7         ; and loop forever. WDT will reset
    goto    $               ; device, if enabled.
;---------------------------------------------------------------------
; WriteI2C
;---------------------------------------------------------------------
WriteI2C
    btfsc   SSPSTAT,BF      ; Is the buffer full?
    goto    WriteI2C        ; Yes, keep waiting.
DoI2CWrite
    bcf     SSPCON1,WCOL    ; Clear the WCOL flag.
    movwf   SSPBUF          ; Write the byte in WREG
    btfsc   SSPCON1,WCOL    ; Was there a write collision?
    goto    DoI2CWrite
    return
    end