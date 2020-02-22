; UTIL.ASM      - Utility Routines
; Written by      Chuck McManis (http://www.mcmanis.com/chuck)
; This Version    28-Dec-01
; Copyright (c) 2001 Charles McManis, All Rights Reserved
;
; Change Log:
;       28-DEC-01       Initially created file as the useful
;                       functions out of my LCD test programs.
;
; NOTICE: THIS CODE COMES WITHOUT WARRANTY OF ANY KIND EITHER
;         EXPRESSED OR IMPLIED. USE THIS CODE AT YOUR OWN RISK!
;         I WILL NOT BE HELD RESPONSIBLE FOR ANY DAMAGES, DIRECT 
;         OR CONSEQUENTIAL THAT YOU MAY EXPERIENCE BY USING IT.
;
; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;
;
; Declare variables that we will use internally.
;
        CBLOCK
            DLY_CNT
            CMD_DELAY
            LCD_TMP
            NUM:2     ; Number to convert low byte ...
            NUM_STR:5  ;         ... high byte
        ENDC

        #include "util.inc"                
        
LED1            EQU     4
LED2            EQU     5        
LCD_E           EQU     6        

CVT_NUM:
        BIN2ASCII   NUM, NUM_STR
        RETURN
        
;
; The LCD on the LAB-X3 (modified) is connected thusly:
;       RA0 - DB4       RA4 - RS
;       RA1 - DB5       
;       RA2 - DB6       RB6 - E (this was my modification)
;       RA3 - DB7
;
; The LCD needs to be initialized into 4 bit mode and then
; we should be able to write letters to it. Note that on the
; LAB-X3 the LCD is ALWAYS in write mode, no reading allowed so
; you just have to hope it isn't busy.
;
; According to the Hitachi data sheet, you first have to
; wait 15 mS to insure the display is "stable" after Vcc is
; applied. Then the following sequence puts it into "4 bit"
; mode (reset by instruction)
;               0x03            ' wait > 4.1 mS
;               0x03            ' wait > .1 mS
;               0x03            ' wait > .1 mS
;               0x02            ' wait > .1 mS
; Display is now in 4-bit mode so initialize it with:
;               0x02 0x08  ' Set 4-bit mode, 2 line display
;               0x00 0x08  ' Display "ON"
;               0x00 0x01  ' Clear Display
;               0x00 0x06  ' Entry mode, auto increment cursor
;
LCD_INIT:
        MOVLW   D'200'          ; Wait for LCD to settle
        CALL    DELAY
        MOVLW   H'03'           ; Set LCD for 4 bits        
        MOVWF   PORTA           ; Data Lines
        BSF     PORTB,LCD_E     ; Toggle E
        BCF     PORTB,LCD_E     ; 
        MOVLW   H'50'           ; Wait 5 mS
        CALL    DELAY
        
        BSF     PORTB, LCD_E    ; Send command again
        BCF     PORTB, LCD_E
        MOVLW   H'2'      
        CALL    DELAY
        
                   
        BSF     PORTB, LCD_E    ; Third time's the charm
        BCF     PORTB, LCD_E
        MOVLW   H'2'      
        CALL    DELAY
        
        MOVLW   H'02'           ; Set for 4 bits
        MOVWF   PORTA           ; like so                          
        BSF     PORTB, LCD_E    ; 
        BCF     PORTB, LCD_E
        MOVLW   H'2'            ; Wait .2 mS
        CALL    DELAY
        ; 
        ; Now at this point its in 4-bit mode so send setup
        ; commands through the 4-bit interface.
        ;        
        MOVLW   H'28'           ; Set 2 line display, 4 bit I/O
        CALL    LCD_CMD         ; 
        MOVLW   H'08'           ; Turn off the Display
        CALL    LCD_CMD
        MOVLW   H'01'           ; Clear the contents of the display
        CALL    LCD_CMD
        MOVLW   H'06'           ; Set the Entry mode
        CALL    LCD_CMD
        MOVLW   H'0C'           ; Turn it on again
        CALL    LCD_CMD
        RETURN                  ; Ready to rock and roll
        
;
; LCD_CMD
;
; Generic routine to send a command to the LCD. Since some
; commands take longer to run this routine waits 1mS after it
; sending the command to insure the LCD is done with it.
;
LCD_CMD:
        MOVWF   LCD_TMP         ; Store command
        MOVLW   H'1'            ; This is the generic delay
        MOVWF   CMD_DELAY       ; Used by default
        MOVLW   H'FC'           ; This is how we check for clear/home
        ANDWF   LCD_TMP,W       ; If any bit other than 0 or 1 is set
        BTFSS   STATUS,Z        ; 
        GOTO    OK_DELAY        ; If non-zero leave delay alone
        MOVLW   D'20'           ; Else store 2mS delay value.
        MOVWF   CMD_DELAY
OK_DELAY: 
        SWAPF   LCD_TMP,W       ; Read it, put upper nibble down
        ANDLW   H'0f'           ; Turn OFF the R/S bit
        MOVWF   PORTA           ; Out it goes
        BSF     PORTB,LCD_E     ; Clock it out
        BCF     PORTB,LCD_E     ; Like so
        MOVF    LCD_TMP,W       ; Get lower nybble
        ANDLW   H'0F'           ; Turn off R/S
        MOVWF   PORTA           ; Put it on PortA
        BSF     PORTB,LCD_E     ; Clock it out
        BCF     PORTB,LCD_E     ;
        MOVF    CMD_DELAY,W     ; Wait for it to complete
        CALL    DELAY
        RETURN                  ;

;
; LCD_CHAR
;
; Generic routine to send a command to the LCD. In this
; version it just sends it, a "smarter" version would watch
; for <CR> or <LF> and do something appropriate to the display.
;
LCD_CHAR:
        MOVWF   LCD_TMP         ; Store it in LCD_TMP
        SWAPF   LCD_TMP,W       ; Upper Nybble
        ANDLW   H'0F'           ; Clear upper bits
        IORLW   H'10'           ; Turn On R/S bit
        MOVWF   PORTA           ; Put it out to PortA
        BSF     PORTB,LCD_E     ; Clock it out
        BCF     PORTB,LCD_E     ;
        MOVF    LCD_TMP,W       ; Get the lower nybble
        ANDLW   H'0F'           ; Clear upper bits
        IORLW   H'10'           ; Turn on R/S Bit
        MOVWF   PORTA           ; Out to PORTA
        BSF     PORTB, LCD_E    ; Clock it out
        BCF     PORTB, LCD_E    ; 
        MOVLW   H'2'            ; Wait a bit
        CALL    DELAY
        RETURN
        
;
; Delay Loop
; 
; This function is designed to delay for 100uS times the number
; of times through the loop
; Once through :
;               1+91+2+1+2+1+2 = 100
;                   
; Twice through
;               1+91+2+1+1+2+96+1+2+1+2 = 200
;                            ******
; Thrice through
;               1+91+2+1+1+2+96+1+1+2+96+1+2+1+2 = 300
;                            ******** ******
; "N" times through (n * 100) cycles.
;
; NOTE: This will have to be changed when you change the 
; frequency from 4Mhz, but for the LAB-X3 it works great!
;
DELAY:
        MOVWF   DLY_CNT         ; Store delay count        (1)
        WAIT    91              ; Delay 95
        GOTO    D2_LOOP         ; Check to see if it was 1 (2)
D1_LOOP:
        WAIT    96              ;       (20)
D2_LOOP:        
        DECF    DLY_CNT,F       ; Subtract 1            (1)
        INCFSZ  DLY_CNT,W       ; Check for underflow   (2)
        GOTO    D1_LOOP          ; Not zero so loop      (2)
        NOP
        RETURN        


