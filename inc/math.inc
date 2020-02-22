; MATH16.INC      16 bit Math Routines
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
        BTFSC   STATUS,C        ; Check for carry
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

; Compares a 16 bit literal with a 16 bit register value.
; Subtracts the literal from the value in FILE 	
; Carry set if FILE >= LITERAL, clear if FILE < LITERAL		
CMP16L		macro FILE,LIT
			MOVLW low (LIT)
			SUBWF (FILE),W
			MOVLW high (LIT) 	; Get ms byte to subtract - no change to flags
			BTFSS STATUS,C		; If there was a carry...
			ADDLW 1 			; ...add in borrow
			SUBWF (FILE)+1,W	; And subtract ms bytes to set C for result
			endm

; 16 bit unsigned addition of literal from 16 bit file
; Operation is DST = DST + LIT
;
ADDL16   MACRO   DST, LIT
        MOVLW    low (LIT)
        ADDWF   (DST),F
        MOVLW   high (LIT)
        BTFSC   STATUS,C
		ADDLW 1 			; ...add in borrow
        ADDWF   (DST)+1,F
        ENDM

; 16 bit unsigned subraction of literal from 16 bit file
; Operation is DST = DST - LIT
;
SUBL16   MACRO   DST, LIT
        MOVLW    low (LIT)
        SUBWF   (DST),F		; dst <- dst - low(lit)
        MOVLW   high (LIT)  ; w <- high(lit)
        BTFSS   STATUS,C
		ADDLW 1 			; ...add in borrow (w <- high(lit) + 1
        SUBWF   (DST)+1,F   ; (dst+1) < (dist+1) - w
        ENDM

			
; Macro for loading a 16 bit literal to a file	
LD16L		macro FILE,LIT
			MOVLW low (LIT)
			MOVWF (FILE)
			MOVLW high (LIT) 	
			MOVWF (FILE)+1		
			endm

; Move a 16 bit value from file to file
MV16		macro DST,SRC
			movf  (SRC),W
			movwf (DST)
			movf  (SRC)+1,W
			movwf (DST)+1
			endm

; Clear a 16 bit value to 0
CLRF16		macro DST
			clrf (DST)
			clrf (DST)+1
			endm

; Shifts the destination left one bit.
; Note, arithmetic shift equivalent to multiply by 2
; MSB ends up in C
ASL16		macro DST
			bcf	STATUS,C
			rlf (DST),F
			rlf (DST+1),F
			endm

; Shifts the destination right one bit.
; Arithmetic shift equivalent to divide by 2
; LSB ends up in C
ASR16		macro DST
			bcf	STATUS,C
			rrf (DST+1),F
			rrf (DST),F
			endm


;=======================================================
; CLRF32 sets a 32 bit file location to zero
;=======================================================
CLRF32	Macro DST
		banksel DST
		CLRF (DST)
		CLRF (DST+1)
		CLRF (DST+2)
		CLRF (DST+3)
		endm

;=======================================================
; INCF32 increments a 32 bit value in a file
;=======================================================
INCF32	Macro DST
		banksel DST
		INCF (DST),F
		BTFSC STATUS,C
		INCF (DST)+1,F
		BTFSC STATUS,C
		INCF (DST)+2,F
		BTFSC STATUS,C
		INCF (DST)+3,F
		endm

; Move a 16 bit value from file to file
MV24		macro DST,SRC
			movf  (SRC),W
			movwf (DST)
			movf  (SRC)+1,W
			movwf (DST)+1
			movf  (SRC)+2,W
			movwf (DST)+2
			endm

;
; 24 bit unsigned addition with carry out.
; Operation: DST = DST + SRC                       
;
; DST is replaced, SRC is preserved, Carry is set correctly
;
ADD24   MACRO   DST,SRC 
        MOVF    (SRC),W         ; Get low byte
        ADDWF   (DST),F         ; Add to destination
		MOVLW	0
        BTFSC   STATUS,C        ; Check for carry
		MOVLW   1				; if carry will add this in to mid..
		ADDWF   (DST)+1,F		
		BTFSC	STATUS,C		; Propagate carry to high byte?
		INCF	(DST)+2,F		; if so add it in.
        MOVF    (SRC)+1,W       ; Get mid byte
        ADDWF   (DST)+1,F       ; Add mid byte into DST
        MOVF    (SRC)+2,W       ; Get high byte
        BTFSC   STATUS,C        ; Check for carry
        INCF    (SRC)+2,W       ; Add one for carry
        ADDWF   (DST)+2,F       ; Add high byte into DST
        ENDM

; Shifts the destination left one bit.
; Note, arithmetic shift equivalent to multiply by 2
; MSB ends up in C
ASL24		macro DST
			bcf	STATUS,C
			rlf (DST),F
			rlf (DST)+1,F
			rlf (DST)+2,F
			endm

; Shifts the destination right one bit.
; Arithmetic shift equivalent to divide by 2
; LSB ends up in C
ASR24		macro DST
			bcf	STATUS,C
			rrf (DST)+2,F
			rrf (DST)+1,F
			rrf (DST),F
			endm

;=======================================================
; MUL24x16
; Multiplies a 24 bit (3 byte) number by 16.  Swaps nibbles
; around rather than 12 shifts.  Used for fixed point math
; where there a 4 bits after the binary point.
; Number is assumed to be little-endian.
;=======================================================
MUL24x16  Macro DST				;DST+2  DST+1	DST		    W-reg
		SWAPF	(DST)+1,W		;0	0	A	B	C	D		B	A
		MOVWF	(DST)+1			;0	0	B	A	C	D		B	A
		ANDLW	H'0F'			;0	0	B	A	C	D		0	A
		MOVWF	(DST)+2			;0	A	B	A	C	D		0	A
		XORWF	(DST)+1,F		;0	A	B	0	C	D		0	A
		SWAPF	(DST),W			;0	A	B	0	C	D		D	C
		ANDLW	H'0F'			;0	A	B	0	C	D		0	C
		IORWF	(DST)+1,F		;0	A	B	C	C	D		0	C
		SWAPF	(DST),F			;0	A	B	C	D	C		0	C
		XORWF	(DST),F			;0	A	B	C	D	0		0	C
		endm	
			
			
			
;=======================================================
; DIV24x16
; Divides a 24 bit (3 byte) number by 16.  Swaps nibbles
; around rather than 12 shifts.  Used for fixed point math
; where there a 4 bits after the binary point.
; Number is assumed to be little-endian.
;=======================================================
DIV24x16 Macro DST				;DST+2  DST+1	DST		    W-reg
		SWAPF	(DST),F			;0	A	B	C	0	D		-	-
		SWAPF	(DST)+1,W		;0	A	B	C	0	D		C	B
		ANDLW	H'F0'			;0	A	B	C	0	D		C	0
		IORWF	(DST),F			;0	A	B	C	C	D		C	0
		SWAPF	(DST)+1,F		;0	A	C	B	C	D		C	0
		XORWF	(DST)+1,F		;0	A	0	B	C	D		C	0
		SWAPF	(DST)+2,W		;0	A	0	B	C	D		A	0
		IORWF	(DST)+1,F		;0	A	A	B	C	D		A	0
		CLRF	(DST)+2			;0	0	A	B	C	D		A	0
		endm

;=======================================================
; ROUND24
; Rounds a positive fixed point (x16) number up or down.
; Adds 8 to the lowest byte - if LS nibble contains 8 or 
; more we'll get a carry into the next nibble which can
; potentially ripple up the 24 bit register.
;=======================================================
ROUND24 Macro DST
		MOVLW 8					; force round up or down.
		ADDWF (DST),W
		ANDLW H'F0'				; zero fractional part
		MOVWF (DST)				; and save.
		MOVLW 1					; used to increment higher values
        BTFSC   STATUS,C        ; Check for carry
        ADDWF   (DST)+1,F       ; Add one for carry
        BTFSC   STATUS,C        ; Check for carry
        ADDWF   (DST)+2,F       ; Add one for carry
		endm
