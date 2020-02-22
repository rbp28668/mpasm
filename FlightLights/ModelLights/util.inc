; UTIL.INC      - Utility Routines
; This Version    28-Dec-01
        NOLIST
; Written by      Chuck McManis (http://www.mcmanis.com/chuck)
; Copyright (c) 2001 Charles McManis, All Rights Reserved
;
; Change Log:
;       28-DEC-01       Added Digit conversion routines
;       22-DEC-01       Initially created with the WAIT
;                       macro for computing delays.
;
; NOTICE: THIS CODE COMES WITHOUT WARRANTY OF ANY KIND EITHER
;         EXPRESSED OR IMPLIED. USE THIS CODE AT YOUR OWN RISK!
;         I WILL NOT BE HELD RESPONSIBLE FOR ANY DAMAGES, DIRECT 
;         OR CONSEQUENTIAL THAT YOU MAY EXPERIENCE BY USING IT.
;
; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
util_inc equ 0
;
; Digit Conversion Macro
;
; This macro converts one "digit" of a 5 digit number
; into ASCII It is used in the routine CVT_NUM that
; converts the number in NUM_HIGH and NUM_LOW, into 5
; ASCII digits stored in DIGIT_0, DIGIT_1, ... DIGIT_4.
; DIGIT_0 is the MSD, DIGIT_4 is the LSD.
;
; It works by initializing the DIGIT location to '0' - 1 (ASCII
; zero minus 1) and then begins subtracting the value you pass and
; counts how many times it can be subtracted before underflow (very
; simple divide operation). Then adds back the last value and returns.
; It leaves NUM_LOW and NUM_HIGH with a value that is the modulus
; of NUM % VALUE (the remainder) and DIGIT_n has the divider.
; 
; Cycle computation :
; 3 preamble
; 10 per digit increment
; 6 postamble
; 19 + 10 * actual digit
;
_ONE_DIGIT        MACRO NUMBER, DIGITS, DIG_NUM, VALUE
        MOVLW   '0' - 1
        MOVWF   DIGITS + #v( DIG_NUM )

        LOCAL LOOP
LOOP:        
        INCF    DIGITS + #v( DIG_NUM ),F
        MOVLW   low (VALUE)             ; Low byte of value
        SUBWF   (NUMBER),F              ; Count = count - value
        MOVLW   high (VALUE)
        SKPC
        MOVLW   (high (VALUE)) + 1
        SUBWF   (NUMBER)+1,F
        SKPNC
        GOTO    LOOP                ; Not done yet so loop
        MOVLW   low (VALUE)         ; Else add back VALUE
        ADDWF   (NUMBER),F          ; To the count
        MOVLW   high (VALUE)
        SKPNC
        MOVLW   (high (VALUE)) + 1
        ADDWF   (NUMBER)+1,F
        ENDM

;
; Call's DO_DIGIT 5 times to convert the number in 'NUMBER'
; into a string at 'DIGITS'
;
BIN2ASCII       MACRO   NUMBER, DIGITS
        nolist
        _ONE_DIGIT NUMBER, DIGITS, 0, D'10000'
        _ONE_DIGIT NUMBER, DIGITS, 1, D'1000'
        _ONE_DIGIT NUMBER, DIGITS, 2, D'100'
        _ONE_DIGIT NUMBER, DIGITS, 3, D'10'
        MOVF    (NUMBER),W      ; Get last digit
        ADDLW   '0'             ; Add ascii 0 to it
        MOVWF   ((DIGITS)+4)    ; Store it in last digit
        CLRF    (NUMBER)
        list
        ENDM
                
;
; Wait Macro
; 
; This macro comes from the "Advanced" PIC programming seminar
; and it will build a delay of 'n' machine cycles which are 
; 1/4 the clock frequency. So for a 4Mhz crystal a delay of
; 1 cycle is 4 ticks, or 1 instruction cycle which is 1uS. 
;
WAIT    MACRO   CYCLES
        nolist
X SET (CYCLES) % 4
        IF (X == 1) || (X == 3)
            NOP
        ENDIF
        
        IF (X == 2) || (X == 3)
            GOTO $+1
        ENDIF
        
X SET (CYCLES) / 4
        IF (X)
            IF (X == 256)
X SET 0
            ENDIF
            
            MOVLW   X
            ADDLW   -1
            SKPZ
            GOTO    $-2
        ENDIF
        list
        ENDM

; msg: MOVF     INDEX,W
;      ADDWF    PCL     
;      DT       <pos>,"The Message", 0
;
DM      MACRO   msg, index
        MOVF    INDEX,W
        ADDWF   PCL
        DT      msg,0
        ENDM
        
;
; Send a message to the LCD, the format of the message is
; <position>, text message, nul byte. 
; No interpretation of special numbers is done at all.
;
SENDMSG MACRO   msg, index
        LOCAL   LOOP,MSG_EXIT
        CLRW
        MOVWF   index
        CALL    msg
        CALL    LCD_CMD
LOOP:
        INCF    index,F
        CALL    msg
        ANDLW   H'FF'
        SKPNZ
        GOTO    MSG_EXIT
        CALL    LCD_CHAR
        GOTO    LOOP
MSG_EXIT:
        ENDM

       
        LIST
