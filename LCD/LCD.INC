;******************************************************************************
;  LCD.INC
;  This include file defines code and data modules for driving a standard 16x2
;  (or variant of) LCD display.
; Most functions are dependent on the port assignments or clock speed so are
; included in this file so that the correct code is generated.
;
;******************************************************************************

;=======================================================
; Constants
; Note - these need to be set to configure the hardware configuration
; of the PIC driving the LCD.  Sample values given here where
; data is on PORTA and control lines are on PORTB as follows:
;=======================================================
;_ClkIn			EQU		D'8000000'		; Processor clock frequency.

;LCD_DATA         EQU     PORTA          ; Uses LS 4 bits of port - update 
;LCD_DATA_TRIS    EQU     TRISA
;LCD_CNTL         EQU     PORTB
;LCD_DATA_MASK	 EQU	 0x0F			; which bits of the port are used for data bits

; LCD Display Commands and Control Signal names on PORTB
;E				EQU		5				; LCD Enable control line
;RW				EQU		3				; LCD Read/Write control line
;RS				EQU		0				; LCD Register Select control line



; This is used for the ASSEMBLER to recalculate certain frequency
; dependant variables. The value of Dev_Freq must be changed to 
; reflect the frequency that the device actually operates at.
; Note that _Clkin needs to be defined before including this file.
;
LCD_INIT_DELAY	EQU		(D'5' * (_ClkIn/4) / D'1000') / ( 3*256 + 3) + 1


;
; LCD Module commands
;
DISP_ON			EQU		0x0C			; Display on
DISP_ON_C		EQU		0x0E			; Display on, Cursor on
DISP_ON_B		EQU		0x0F			; Display on, Cursor on, Blink cursor
DISP_OFF		EQU		0x08			; Display off
CLR_DISP		EQU		0x01			; Clear the Display
ENTRY_INC		EQU		0x06			;
ENTRY_INC_S		EQU		0x07			;
ENTRY_DEC		EQU		0x04			;
ENTRY_DEC_S		EQU		0x05			;
DD_RAM_ADDR		EQU		0x80			; Least Significant 7-bit are for address
DD_RAM_UL		EQU		0x80			; Upper Left coner of the Display
DD_RAM_LL		EQU		0xC0			; Lower Left coner of the Display
CG_RAM          EQU     0x40            ; start of Character Gen ram

;

; Overlayable data (note, may be overwritten by other modules when called).
lcd_overlay	udata_ovr
TMP         res 1
CHAR		res 1
LCDTMP1     res 1
LCDTMP2		res 1

lcd_data	udata
LCDDATATRIS	res 1	; save desired value of TRIS for LCD Data.
CTRLMASK    res 1   ; current state of control bits - avoiding bit-ops on IO

lcd		code

;*******************************************************************
; The LCD Module Subroutines                                      
; Note that most of these are included inline as they rely on 
; definitions of hardware bits/ports to be set up before this file is 
; included.
;*******************************************************************

SHORT_DELAY Macro
#ifndef SIMULATE
            banksel TMP
            CLRF   TMP
            DECFSZ  TMP,F           ; 255 + 2
            GOTO    $-1             ; 255
#endif
            endm

; LCD_WRT_CTRL writes out the control mask to the control port
LCD_WRT_CTRL Macro
            MOVFW   CTRLMASK
            banksel LCD_CNTL
            MOVWF   LCD_CNTL
            endm

; LCD_WRITE writes W to LCD_DATA.
LCD_WRITE   Macro
#ifndef SIMULATE
            banksel LCD_DATA        ; ports bank 0
            MOVWF   LCD_DATA        ;
            banksel CTRLMASK
            BSF     CTRLMASK, E     ; 
            LCD_WRT_CTRL
            DELAY200 ;SHORT_DELAY
            banksel CTRLMASK
            BCF     CTRLMASK, E
            LCD_WRT_CTRL           
            DELAY200 ;SHORT_DELAY
#endif
            endm

;771 cycles, c. 380uS at 8MHz
DELAY200    Macro
#ifndef SIMULATE
            banksel TMP
            CLRF    TMP
            DECFSZ  TMP,F            
            GOTO    $-1
#endif
            endm

;*******************************************************************
; Initilize the LCD Display Module
; Needs to have the desired value for TRIS for unused pins passed in
; W.  Allows rest of LCD routines to make sensible use of TRIS without
; changing pins it doesn't use.  Note that it is up to the calling 
; routine to set TRISA/TRISB appropriately beforehand.
;*******************************************************************
LCD_INIT
            
			banksel LCDDATATRIS
            movwf	LCDDATATRIS

#ifndef SIMULATE

			; TRIS - 1 is an input, 0 output.
			ANDLW	~LCD_DATA_MASK  ; zero the bits we use for output.
            banksel LCD_DATA_TRIS
			MOVWF	LCD_DATA_TRIS
			
            banksel OPTION_REG     ; Select Bank 1
            BSF     OPTION_REG,NOT_RBPU  ; Disable PORTB pull-ups
            
			; Zero the control lines  (write to control)
            banksel CTRLMASK
            CLRF    CTRLMASK
            LCD_WRT_CTRL

            ; Ensure power on delay honoured.
            call    LCD_DELAY
            call    LCD_DELAY
            call    LCD_DELAY


			; Initialise the display  - Standard 0x33,0x32 sequence
            MOVLW   0x03            ;
            LCD_WRITE    ;
            CALL    LCD_DELAY
            MOVLW   0x03            ; Command for 4-bit interface low nibble
            LCD_WRITE  
            CALL  LCD_DELAY         ;DELAY200                ; Wait at least 200uS
            MOVLW   0x03            ; Command for 4-bit interface low nibble
            LCD_WRITE
            CALL    LCD_DELAY       ;DELAY200                ; Wait at least 200uS
            MOVLW   0x02            ; Command for 4-bit interface low nibble
            LCD_WRITE
            CALL    LCD_DELAY


;
; Command sequence for 2 lines of 5x7 characters
CMD_SEQ
            MOVLW   0X02            ; 4-bit low nibble xfer
            LCD_WRITE
            CALL    LCD_DELAY
            MOVLW   0x08            ; 4-bit low nibble xfer 
            LCD_WRITE
            CALL    LCD_DELAY

; Busy Flag should be valid after this point
           
            ; FALL THROUGH TO INIT_DISPLAY
#endif

;*******************************************************************
; Does basic LCD display initialisation.
;*******************************************************************
INIT_DISPLAY
#ifndef SIMULATE
            MOVLW   DISP_ON             ; Display On, Cursor On
            CALL    SEND_CMD            ; 
            MOVLW   ENTRY_INC           ; Set Entry Mode Inc., No shift
            CALL    SEND_CMD            ; 
            MOVLW   CLR_DISP            ; Clear the Display
            CALL    SEND_CMD            ; 
            MOVLW   DD_RAM_ADDR         ;
            CALL    SEND_CMD            ;
#endif
            RETURN


;*******************************************************************
;* SEND_CHAR - Sends character to LCD                              *
;* This routine splits the character into the upper and lower      * 
;* nibbles and sends them to the LCD, upper nibble first.          *
;* The data is transmitted on the PORT<3:0> pins                   *
;*******************************************************************
SEND_CHAR
#ifndef SIMULATE
            banksel CHAR
            MOVWF   CHAR            ; Character to be sent is in W
            ;CALL    BUSY_CHECK      ; Wait for LCD to be ready
            CALL LCD_DELAY

            banksel CTRLMASK
            BCF     CTRLMASK, RW    ; Set LCD to read
            BSF     CTRLMASK, RS    ; Set LCD to data mode
            LCD_WRT_CTRL

            banksel CHAR
            SWAPF   CHAR, W
            ANDLW   0x0F            ; Get upper nibble
            LCD_WRITE
            banksel CHAR
            MOVF    CHAR, W
            ANDLW   0x0F            ; Get lower nibble
            LCD_WRITE
#endif
            RETURN

;*******************************************************************
;* SendCmd - Sends command to LCD                                  *
;* This routine splits the command into the upper and lower        * 
;* nibbles and sends them to the LCD, upper nibble first.          *
;* The data is transmitted on the PORT<3:0> pins                   *
;*******************************************************************
SEND_CMD
#ifndef SIMULATE
            banksel CHAR
            MOVWF   CHAR            ; Character to be sent is in W

            ;CALL    BUSY_CHECK      ; Wait for LCD to be ready
            CALL LCD_DELAY

            banksel CTRLMASK
            BCF     CTRLMASK, RW    ; Set LCD to write
            BCF     CTRLMASK, RS    ; Set LCD to command mode
            LCD_WRT_CTRL

            banksel CHAR
            SWAPF   CHAR, W
            ANDLW   0x0F            ; Get upper nibble
            LCD_WRITE
            banksel CHAR
            MOVF    CHAR, W
            ANDLW   0x0F            ; Get lower nibble
            LCD_WRITE
#endif
            RETURN


;*******************************************************************
;* This routine checks the busy flag, returns when not busy        *
;* Note that busy flag is output on bit 7.  1 is busy, 0 is idle   *
;*  Affects:                                                       *
;*      TEMP - Returned with busy/address                          *
;*******************************************************************
;
BUSY_CHECK
#ifndef SIMULATE
            ;GOTO    LCD_DELAY ; DEBUG

			; Set DATA TRIS to allow monitoring of busy bit (1 read, 0 write)
            banksel LCDDATATRIS
			movfw	LCDDATATRIS
			IORLW	LCD_DATA_MASK   ; set the bits we use for input.
            banksel LCD_DATA_TRIS   ; Bank 1
            MOVWF   LCD_DATA_TRIS

            banksel CTRLMASK
            BCF     CTRLMASK, RS    ; Set LCD for Command mode
            BSF     CTRLMASK, RW    ; Setup to read busy flag
            LCD_WRT_CTRL
            banksel CTRLMASK
            BSF     CTRLMASK, E     ; Set E high
            LCD_WRT_CTRL

            SHORT_DELAY

            banksel LCD_DATA        ; Bank 0
            SWAPF   LCD_DATA, W     ; Read upper nibble busy flag, DDRam address
            ANDLW   0xF0            ; Mask out lower nibble
            banksel LCDTMP1
            MOVWF   LCDTMP1         ;

            banksel CTRLMASK
            BCF     CTRLMASK, E     ; Set E low
            LCD_WRT_CTRL

            banksel CTRLMASK
            BSF     CTRLMASK, E     ; Toggle E to get lower nibble
            LCD_WRT_CTRL
            
            SHORT_DELAY

            banksel LCD_DATA        ; Bank 0
            MOVF    LCD_DATA, W     ; Read lower nibble busy flag, DDRam address
            ANDLW   0x0F            ; Mask out upper nibble
            banksel LCDTMP1
            IORWF   LCDTMP1, F      ; Combine nibbles, save in LCDTMP1

            banksel CTRLMASK
            BCF     CTRLMASK, E     ; Toggle E to get lower nibble
            LCD_WRT_CTRL


            BTFSC   LCDTMP1, 7      ; Check busy flag, high = busy
            GOTO    BUSY_CHECK      ; If busy, check again

            banksel CTRLMASK            
            BCF     CTRLMASK, RW
            LCD_WRT_CTRL

			; Restore data pins to output.
            banksel LCDDATATRIS
			movfw	LCDDATATRIS
			ANDLW	~LCD_DATA_MASK  ; set the bits we use for output.
            banksel LCD_DATA_TRIS   ; Bank 1
            MOVWF   LCD_DATA_TRIS   ; RB7 - 4 = inputs, RB3 - 0 = output
#endif
            RETURN

;
; This routine takes the calculated times that the delay loop needs to
; be executed, based on the LCD_INIT_DELAY EQUate that includes the
; frequency of operation. 
LCD_DELAY   NOP
#ifndef SIMULATE
            banksel LCDTMP1
            MOVLW   LCD_INIT_DELAY  ;
            MOVWF   LCDTMP2         ; Use MSD and LSD Registers to Initilize LCD
            CLRF    LCDTMP1         ;
LOOP2       DECFSZ  LCDTMP1, F      ; Delay time = MSD * ((3 * 256) + 3) * Tcy
            GOTO    LOOP2           ;
            DECFSZ  LCDTMP2, F      ;
            GOTO    LOOP2           ;
#endif
            RETURN
    
