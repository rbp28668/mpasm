; MATH16.INC      16 bit Math Routines
; This Version    30-Dec-01
                nolist
; Written by      Chuck McManis (http://www.mcmanis.com/chuck)
; Copyright (c) 2001 Charles McManis, All Rights Reserved
;
; Change Log:
;       30-DEC-01       Created this file
;
; NOTICE: THIS CODE COMES WITHOUT WARRANTY OF ANY KIND EITHER
;         EXPRESSED OR IMPLIED. USE THIS CODE AT YOUR OWN RISK!
;         I WILL NOT BE HELD RESPONSIBLE FOR ANY DAMAGES, DIRECT 
;         OR CONSEQUENTIAL THAT YOU MAY EXPERIENCE BY USING IT.
;
; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
; 
; This include file provides some macros for dealing with
; 16 bit quantities. It assumes a little endian format
; where the least significant byte is lower in address than
; the most significant byte.
;
; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;
; 16 bit unsigned subtraction with carry out.
; Word format is little endian (LSB at lower address)
; Operation is DST = DST - SRC
;
; (This from the "tips and tricks" seminar handout)
;
; DST is replaced, SRC is preserved, Carry is set correctly
;
;
SUB16   MACRO   DST, SRC
        MOVF    (SRC),W
        SUBWF   (DST),F
        MOVF    (SRC)+1,W
        BTFSS   STATUS,C
        INCF    (SRC)+1,W
        SUBWF   (DST)+1,F
        ENDM

;
; 16 bit unsigned addition with carry out.
; Operation: DST = DST + SRC                       
;
; DST is replaced, SRC is preserved, Carry is set correctly
;
ADD16   MACRO   DST,SRC 
        MOVF    (SRC),W         ; Get low byte
        ADDWF   (DST),F         ; Add to destination
        MOVF    (SRC)+1,W       ; Get high byte
        BTFSS   STATUS,C        ; Check for carry
        INCF    (SRC)+1,W       ; Add one for carry
        ADDWF   (DST)+1,F       ; Add high byte into DST
        ENDM

;
; Increment 16 bit value, sets Z on exit.
;
; Operation: DST++
;
INC16   MACRO   DST        
        INCFSZ  (DST),W         ; Add one to low byte
        DECF    (DST)+1,F       ; No carry (negates next step)
        INCF    (DST)+1,F       ; Add one to high byte
        MOVWF   (DST)           ; Store updated low byte back.
        IORWF   (DST)+1,W       ; Set Z flag
        ENDM

;
; Decrement 16 bit value, sets Z on exit
;        
; Operation: DST--
;
DEC16   MACRO   DST
        DECF    (DST),F         ; Decrement low byte
        INCFSZ  (DST),W         ; Check for underflow
        INCF    (DST)+1,F       ; Update
        DECF    (DST)+1,F       ; Fixup
        MOVF    (DST),W
        IORWF   (DST)+1,W       ; Set Z bit
        ENDM
        
        list                      
