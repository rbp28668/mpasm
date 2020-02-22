
;***********************************************************
; MAX7219
; Code to drive MAX7219 display
;***********************************************************

#include p16f88.inc

; LCD Display Commands and Control Signal names on PORTB
MAX_CS				EQU		5				; MAX7219 ~CS line
MAX_CLK				EQU		3				; MAX7219 clock line
MAX_DATA			EQU		0				; MAX7219 data line


;=======================================================
; Variables
;=======================================================
                udata
bmask           res 1   ; mask to avoid bit ops on port
addr            res 1   ; address byte
char            res 1   ; store for char
tmp             res 1   ; general purpose counter etc


;=======================================================
; Entry points
;=======================================================
        global INIT_MAX7219
        global ADDR_MAX7219
        global WRITE_MAX7219

                
                code
;=======================================================
; Initialises the MAX7219
;=======================================================
INIT_MAX7219
                banksel addr
                movlw   0x0C  ; Shutdown mode
                movwf   addr
                movlw   0x01  ; wake up
                call    WRITE_MAX7219

                banksel addr
                movlw   0x09  ; Decode mode
                movwf   addr
                movlw   0xFF  ; all digits decoded
                call    WRITE_MAX7219

                banksel addr
                movlw   0x0A  ; intensity
                movwf   addr
                movlw   8      ; 1/2 brightness
                call    WRITE_MAX7219

                banksel addr
                movlw 0x0B  ; scan limit
                movwf   addr
                movlw   7      ; Display all 8
                call    WRITE_MAX7219

                ; Blank all the digits
                banksel addr
                movlw   8  ; ms digit
                movwf   addr

                movlw   0x0F    ; blank
                call    WRITE_MAX7219
                movlw   0x0F    ; blank
                call    WRITE_MAX7219
                movlw   0x0F    ; blank
                call    WRITE_MAX7219
                movlw   0x0F    ; blank
                call    WRITE_MAX7219
                movlw   0x0F    ; blank
                call    WRITE_MAX7219
                movlw   0x0F    ; blank
                call    WRITE_MAX7219
                movlw   0x0F    ; blank
                call    WRITE_MAX7219
                movlw   0x0F    ; blank
                call    WRITE_MAX7219
                    
                return

;=======================================================
; Writes W to the address for the MAX7219
;=======================================================
ADDR_MAX7219
                banksel addr
                movwf   addr
                return


;=======================================================
; Writes W to the register of the MAX7219 pointed to 
; by addr.
;=======================================================
WRITE_MAX7219   
                banksel char  
                movwf   char  ; Save W for sending after addr
                
                ; Drop ~CS to mark start of 16 bit transfer
                bcf     bmask, MAX_CS
                movfw   bmask
                banksel PORTB
                movwf   PORTB
                

                ; Initially, transfer address MSB first
                banksel tmp
                movlw   8
                movwf   tmp

WRT_A_LP

                ; Write ADDR byte MSB first
                bcf     bmask,MAX_DATA
                rlf     addr,F  ; msb -> C
                btfsc   STATUS,C ; skip if C clear
                bsf     bmask,MAX_DATA ; C was set so set bit
                movfw   bmask
                banksel PORTB
                movwf   PORTB

                ; Send clock high
                banksel bmask
                bsf     bmask,MAX_CLK
                movfw   bmask
                banksel PORTB
                movwf   PORTB

                ; send clock low
                banksel bmask
                bcf     bmask,MAX_CLK
                movfw   bmask
                banksel PORTB
                movwf   PORTB

                banksel tmp
                decfsz  tmp,F
                goto    WRT_A_LP

                ;  Address transferred, restore and increment  
                rlf     addr,F
                ;incf    addr,F;
                decf    addr,F ; start at MS and work down

                ; Now transfer 8 data bits MSB first
                banksel tmp
                movlw   8
                movwf   tmp

WRT_D_LP

                ; Write DATA byte MSB first
                bcf     bmask,MAX_DATA
                rlf     char,F
                btfsc   STATUS,C
                bsf     bmask,MAX_DATA
                movfw   bmask
                banksel PORTB
                movwf   PORTB

                ; Send clock high
                banksel bmask
                bsf     bmask,MAX_CLK
                movfw   bmask
                banksel PORTB
                movwf   PORTB

                ; send clock low
                banksel bmask
                bcf     bmask,MAX_CLK
                movfw   bmask
                banksel PORTB
                movwf   PORTB

                banksel tmp
                decfsz  tmp,F
                goto    WRT_D_LP

                ; Raise ~CS to mark end of 16 bit transfer
                bsf     bmask, MAX_CS
                movfw   bmask
                banksel PORTB
                movwf   PORTB

                return
            
                end