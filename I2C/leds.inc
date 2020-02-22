;---------------------------------------------------------------------
; File: leds.inc
; Macros for debug leds on test board.
;---------------------------------------------------------------------

; Debug LEDS
LED1	equ 3
LED2	equ 2
LED3	equ	0
LED4	equ 5


#define NOLEDS 0

#if NOLEDS

LEDS    macro value
        endm

#else 

LEDS	macro value
        banksel PORTB
        if (value & 8) != 0
    	bcf PORTB,LED1 ; ON
        else
		bsf PORTB,LED1 ; OFF
        endif

        if (value & 4) != 0
    	bcf PORTB,LED2 ; ON
        else
		bsf PORTB,LED2 ; OFF
        endif

        if (value & 2) != 0
    	bcf PORTB,LED3 ; ON
        else
		bsf PORTB,LED3 ; OFF
        endif

        if (value & 1) != 0
    	bcf PORTB,LED4 ; ON
        else
		bsf PORTB,LED4 ; OFF
        endif

		endm

#endif

