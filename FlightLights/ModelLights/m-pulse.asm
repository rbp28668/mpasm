; M-PULSE.ASM   Pulse Measurement Program
; Written by    Chuck McManis (http://www.mcmanis.com/chuck)
; This Version  30-Dec-01
; Copyright (c) 2001 Charles McManis, All Rights Reserved
;
; Change Log:
;       30-DEC-01       Added pulse measurement w/CCP1 input
;       29-DEC-01       Initial Creation from LCD2
;
; NOTICE: THIS CODE COMES WITHOUT WARRANTY OF ANY KIND EITHER
;         EXPRESSED OR IMPLIED. USE THIS CODE AT YOUR OWN RISK!
;         I WILL NOT BE HELD RESPONSIBLE FOR ANY DAMAGES, DIRECT 
;         OR CONSEQUENTIAL THAT YOU MAY EXPERIENCE BY USING IT.
;
; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;
        TITLE "LCD2 - Measuring a Pulse"
        LIST P=PIC16F628, C=120, N=50, R=HEX
        include "P16F628.inc"
        include "math.inc"
       	__FUSES _CP_OFF&_LP_OSC&_WDT_OFF&_LVP_OFF
        
        CBLOCK H'70'
            TMP_W     
            TMP_STATUS
        ENDC
        
        CBLOCK H'20'
            LEADING:0, LEADING_LO, LEADING_HI
            CAPTURE:0, CAPTURE_LO, CAPTURE_HI
            COUNTER:0, COUNTER_LO, COUNTER_HI
            MSG_NDX
            TMP_CHAR
            GOT_ONE
            T0_COUNT
        ENDC
        
I_CNT   EQU     H'4E'

        ORG H'0000'
        GOTO    MAIN            ; Let's get this puppy rolling
;
; Code Section
;
; The code section of the ISR lives at 0x0004. Its possible to put a 
; jump here however doing so adds two clocks of latency to servicing 
; any interrupt.
; 
        
        ORG     H'0004'          ; Interrupt service routine     
        GOTO    INTR_SVC
        
; Message Space        
msg1:   MOVF    MSG_NDX,W
        ADDWF   PCL,F     
        DT      H'80', "Pulse Width:", 0

msg2:   MOVF    MSG_NDX,W
        ADDWF   PCL,F
        DT      H'C0', "Sample Num :", 0
        
INTR_SVC:        
        MOVWF   TMP_W           ; Copy W to a temporary register
        SWAPF   STATUS,W        ; Swap Status Nibbles and move to W 
        MOVWF   TMP_STATUS      ; Copy STATUS to a temporary register
        BCF     STATUS, RP0     ; Force Bank 0
        BCF     STATUS, RP1     ; 
;
; State is saved, and we've expended 3 Tcy plus the 3 Tcy (4 worst 
; case) of interrupt latency for a total of 6(7) Tcy.
; 
; Now loop through until we've satisfied all the pending interrupts.
;
INTR_POLL:
        ; ... test bit to see if it is set
        BTFSS   INTCON,T0IF     ; Did Timeer0 Overflow?
        GOTO    ISR_1           ; No it didn't, so check next thing.
        ;
        ; Process Timer 0 Overflow Interrupt
        ;
        BCF     INTCON, T0IF    ; Clear Timer 0 interrupt
        DECF    T0_COUNT,F      ; Decrement interrupt counter
        INCFSZ  T0_COUNT,W      ; Read it back to check for overflow
        GOTO    ISR_1           ; Nope, keep counting
        ;
        ; Count underflows when we've hit this interrupt "n" times,
        ; where n is the number in COUNT.
        ;
        MOVLW   I_CNT           ; Restore counter value
        MOVWF   T0_COUNT
        CLRW
        MOVWF   CAPTURE
        MOVWF   CAPTURE+1
        BSF     GOT_ONE,0
        ;
        ; Process interrupts from the Input Capture/Compare pin
        ; (CCP1 on the 16F628)
        ;
ISR_1:  
        BTFSS   PIR1, CCP1IF    ; Check to see that CCP1 interrupted
        GOTO    ISR_2           ; If not continue
        BCF     PIR1, CCP1IF    ; Re-enable it
        BTFSS   CCP1CON, CCP1M0 ; Check for falling edge watch
        GOTO    FALL_EDGE       ; Go pick up the falling edge
        MOVF    CCPR1L,W        ; else store leading edge value
        MOVWF   LEADING         ; into 16 bit work LEADING
        MOVF    CCPR1H,W
        MOVWF   LEADING+1
        BCF     CCP1CON, CCP1M0 ; Now capture the trailing edge
        GOTO    ISR_2           ; Exit the interrupt service routine
        
FALL_EDGE:
        BSF     CCP1CON, CCP1M0 ; Re-set for trailing edge capture
        MOVF    CCPR1L,W        ; Store the captured value into
        MOVWF   CAPTURE         ; CAPT_LO and ...
        MOVF    CCPR1H,W
        MOVWF   CAPTURE+1       ;             ... CAPT_HI
        ;
        ; 16 bit subtract 
        ;     CAPTURE = CAPTURE - LEAD
        ;
        SUB16   CAPTURE, LEADING
        
        BSF     GOT_ONE,0       ; Indicate we have a new sample.
        MOVLW   I_CNT
        MOVWF   T0_COUNT
        INCF    COUNTER,F
        BTFSC   STATUS,Z
        INCF    COUNTER+1,F
ISR_2:                          ; Process the next interrupt
;
; Exit the interrupt service routine. 
; This involves recovering W and STATUS and then
; returning. Note that putting STATUS back automatically pops the bank
; back as well.
;               This takes 6 Tcy for a total overhead of 12 Tcy for sync
;               interrupts and 13 Tcy for async interrupts.
; 
INTR_EXIT:
        SWAPF   TMP_STATUS,W    ; Pull Status back into W
        MOVWF   STATUS          ; Store it in status 
        SWAPF   TMP_W,F         ; Prepare W to be restored
        SWAPF   TMP_W,W         ; Return it, preserving Z bit in STATUS
        RETFIE
;
; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
        #include util.asm
; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
;
                
MAIN:
        CLRF    STATUS          ; Set Bank 0
        CLRF    PORTA           ; Clear PortA
        CLRF    PORTB           ; and clear PortB
        MOVLW   H'07'           ; Make PortA Digital I/O
        MOVWF   CMCON           ; By setting CMCON<0:3>
        BSF     STATUS,RP0      ; Set Bank 1
        CLRF    TRISA           ; Now A is all outputs
        CLRF    TRISB           ; B all outputs
        BSF     TRISB,7         ; Button S1
        BSF     TRISB,3         ; CCP1 is also an input.
        
        MOVLW   B'0000011'      ; Set TMR0 prescaler to 256
        MOVWF   OPTION_REG      ; Store it in the OPTION register
        BSF     PIE1, CCP1IE    ; Enable interrupts from CCP1        
        CLRF    STATUS          ; Back to BANK 0
        MOVLW   B'00000001'     ; Enable Timer 1 1:1 Prescale
        MOVWF   T1CON
        MOVLW   B'00000101'     ; Capture mode rising edge
        MOVWF   CCP1CON
        
        BSF     INTCON, T0IE    ; Enable Timer 0 to interrupt
        BSF     INTCON, PEIE
        BCF     INTCON,T0IF     ; Reset flag that indicates interrupt
        BSF     INTCON, GIE     ; Enable interrupts
        
        BSF     PORTB,LED1      ; Turn On LED1
        
        MOVLW   D'200'
        CALL    DELAY
        CALL    LCD_INIT
        BSF     PORTB,LED1
        CLRW
        MOVWF   COUNTER
        MOVWF   COUNTER+1
        MOVWF   CAPTURE
        MOVWF   CAPTURE+1

        
                        
;
; Print out a simple message
;
msg_loop:
        
        SENDMSG MSG1, MSG_NDX
        
        MOVF    CAPTURE,W
        MOVWF   NUM
        MOVF    CAPTURE+1,W
        MOVWF   NUM+1
        
        CALL    CVT_NUM
        MOVLW   H'8D'           ; Column 15  for 99999
        CALL    LCD_CMD
        MOVF    NUM_STR,W
        CALL    LCD_CHAR
        MOVF    NUM_STR+1,W
        CALL    LCD_CHAR
        MOVF    NUM_STR+2,W
        CALL    LCD_CHAR
        MOVF    NUM_STR+3,W
        CALL    LCD_CHAR
        MOVF    NUM_STR+4,W
        CALL    LCD_CHAR
        MOVLW   "u"
        CALL    LCD_CHAR
        MOVLW   "S"
        CALL    LCD_CHAR
        
        SENDMSG MSG2, MSG_NDX
        MOVF    COUNTER,W
        MOVWF   NUM
        MOVF    COUNTER+1,W
        MOVWF   NUM+1
        
        CALL    CVT_NUM
        MOVLW   H'CD'           ; Column 15  for 99999
        CALL    LCD_CMD
        MOVF    NUM_STR,W
        CALL    LCD_CHAR
        MOVF    NUM_STR+1,W
        CALL    LCD_CHAR
        MOVF    NUM_STR+2,W
        CALL    LCD_CHAR
        MOVF    NUM_STR+3,W
        CALL    LCD_CHAR
        MOVF    NUM_STR+4,W
        CALL    LCD_CHAR
        
wait_loop:        
        BTFSS   GOT_ONE,0
        GOTO    $-1
        BCF     GOT_ONE,0
        MOVLW   D'200'
        CALL    DELAY
        MOVLW   D'200'
        CALL    DELAY
        MOVF    PORTB,W         ; Get port B
        XORLW   H'FF'           ; Toggle It
        XORLW   H'CF'           ; Revert it back
        MOVWF   PORTB           ; Now LED1 and LED2 are alternates
        GOTO    msg_loop

        END        

