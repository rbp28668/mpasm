;===========================================================
; MathTest.asm
; Tests the 32 bit math routines.  Note that it is designed
; to be run under the MPASM simulator.
;===========================================================

        TITLE "32 bit Math routine test"
        LIST P=PIC16F88
        include "P16F88.inc"
       	
        __CONFIG _CONFIG1, _INTRC_IO & _WDT_OFF & _PWRTE_OFF & _MCLR_ON & _BODEN_OFF & _LVP_OFF & _CPD_OFF & _WRT_PROTECT_OFF & _DEBUG_OFF & _CCP1_RB0 & _CP_OFF
        __CONFIG _CONFIG2, _FCMEN_OFF & _IESO_OFF

		radix dec

#define GLOBAL_FAIL 1  ; Make equals32 macro jump to FAIL label if not equal.
        include "math32.inc"
        

;=======================================================
; Initial startup and ISR vectors
    
        ; Startup entry point
STARTUP	code 0
        GOTO    MAIN            ; startup
        
		; Interrupt entry point
        code     H'0004'          ; Interrupt service routine     
        retfie
        

PROG	code


;=======================================================
; Main code entry point.  
;=======================================================
                
MAIN:
        CLRF    STATUS          ; Set Bank 0
        
		banksel OSCCON
		MOVLW	B'01100000'		; Set 4MHz internal clock
		MOVWF	OSCCON

        MOVL32 REGA, 0x00FFFFFF
        MOVL32 REGB, 0x00FFFFFF 
        pagesel add32        
        call add32
        EQUALS32 REGA, 0x01FFFFFE

        MOVL32 REGA, 0x00808080
        MOVL32 REGB, 0x00808080         
        pagesel add32
        call add32
        EQUALS32 REGA, 0x01010100

        MOVL32 REGA, 0x01020304
        MOVL32 REGB, 0x10203040     
        pagesel add32
        call add32
        EQUALS32 REGA, 0x11223344


        MOVL32 REGA, 42
        MOVL32 REGB, 95
        pagesel add32     
        call add32
        EQUALS32 REGA, 42+95

        MOVL32 REGA, 395
        MOVL32 REGB, 44
        pagesel add32     
        call add32
        EQUALS32 REGA, 395+44

        ;Div32

        MOVL32 REGA, 30
        MOVL32 REGB, 5
        pagesel div32     
        call div32
        EQUALS32 REGA, 30/5

        MOVL32 REGA, 30
        MOVL32 REGB, -5
        pagesel div32     
        call div32
        EQUALS32 REGA, 30/(-5)

        MOVL32 REGA, -30
        MOVL32 REGB, 5
        pagesel div32     
        call div32
        EQUALS32 REGA, -30/5


        MOVL32 REGA, -30
        MOVL32 REGB, -5
        pagesel div32     
        call div32
        EQUALS32 REGA, -30/(-5)


        MOVL32 REGA, -17840128
        MOVL32 REGB, 7611
        pagesel div32
        call div32
        EQUALS32 REGA, -17840128/7611

        ; UADD32

        MOVL32 REGA, 0x00FF00FF
        MOVL32 REGB, 0xFF00FF00
        pagesel uadd32
        call uadd32
        EQUALS32 REGA, 0x0FFFFFFFF

        MOVL32 REGA, 0xFFFFFFFF
        MOVL32 REGB, 0x00000001
        pagesel uadd32
        call uadd32
        EQUALS32 REGA, 0
        

        MOVL32 REGA, 0x0FFFFFFF
        MOVL32 REGB, 0x0FFFFFFF
        pagesel uadd32
        call uadd32
        EQUALS32 REGA, 0x0FFFFFFF + 0x0FFFFFFF

        MOVL32 REGA, 0xFFFFFFFF
        MOVL32 REGB, 0
        pagesel uadd32
        call uadd32
        EQUALS32 REGA, 0xFFFFFFFF


        ; UMUL32
        MOVL32 REGA, 1
        MOVL32 REGB, 0xFFFFFFFF
        pagesel umul32
        call umul32
        EQUALS32 REGA, 0xFFFFFFFF

        MOVL32 REGA, 0xFFFFFFFF
        MOVL32 REGB, 1
        pagesel umul32
        call umul32
        EQUALS32 REGA, 0xFFFFFFFF

        MOVL32 REGA, 0xFFFFFFFF
        MOVL32 REGB, 0
        pagesel umul32
        call umul32
        EQUALS32 REGA, 0

        MOVL32 REGA, 0
        MOVL32 REGB, 0xFFFFFFFF
        pagesel umul32
        call umul32
        EQUALS32 REGA, 0

        MOVL32 REGA, 1
        MOVL32 REGB, 1
        pagesel umul32
        call umul32
        EQUALS32 REGA, 1

        MOVL32 REGA, 0x80000000
        MOVL32 REGB, 1
        pagesel umul32
        call umul32
        EQUALS32 REGA, 0x80000000

        MOVL32 REGA, 1
        MOVL32 REGB, 0x80000000
        pagesel umul32
        call umul32
        EQUALS32 REGA, 0x80000000

        MOVL32 REGA, 64505
        MOVL32 REGB, 25000
        pagesel umul32
        call umul32
        EQUALS32 REGA, 1612625000


PASS
        goto $ ; if we end here all the tests passed


FAIL
        goto $ ; one of the tests failed



		
		End
