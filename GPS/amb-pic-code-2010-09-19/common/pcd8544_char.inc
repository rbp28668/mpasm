;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/pcd8544_char.inc,v 1.4 2010/09/19 09:54:51 amb Exp $
;;
;; Include file for PCD8544 based LCD character functions.
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2007, 2010 Andrew M. Bishop
;; It may be distributed under the GNU Public License, version 2, or
;; any higher version.  See section COPYING of the GNU Public license
;; for conditions under which this file may be redistributed.
;;
;;--------------------------------------------------------------------------------
;;
;; The font used is the 5x8 font from X11 which has the following copyright notice.
;;
;; Copyright 1984-1989, 1994 Adobe Systems Incorporated.
;; Copyright 1988, 1994 Digital Equipment Corporation.
;;
;;--------------------------------------------------------------------------------

;;--------------------------------------------------------------------------------
;;
;; Required Variables / Constants for PCD8544 based LCD character functions
;;
;; Variables:   spi_temp        - Stores temporary data, same bank as PCD8544_PORT
;;              pcd8544_temp    - Stores temporary data, same bank as PCD8544_PORT
;;              pcd8544_char_row - Stores the current row in character mode, same bank as PCD8544_PORT
;;              pcd8544_char_col - Stores the current column in character mode, same bank as PCD8544_PORT
;;
;;--------------------------------------------------------------------------------

;;
;; Rows and columns start from zero at the top left.
;;      Row can range from 0 to 5
;;      Column can range from 0 to 15
;;
;; Each character is 8x5 which means that there are 16x6 characters on the screen
;;
;; In pixel terms there are 2 blank pixels at each end (2+16*5+2 = 84).
;;

;;--------------------------------------------------------------------------------
;;
;; Subroutine to initialise character mode
;;
;; Parameters:  none
;;
;; RAM Bank:    any (in) PCD8544_PORT (out)
;;
;;--------------------------------------------------------------------------------

pcd8544_char_init

        ;; Reset X,Y counters

        call    pcd8544_reset_xy

        clrf    pcd8544_char_row
        clrf    pcd8544_char_col

        movlw   0
        call    pcd8544_tx_byte
        call    pcd8544_tx_byte

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine to change the row (column remains unchanged)
;; (Uses macros from pcd8544_basic.inc instead of subroutines for speed)
;;
;; Parameters:  W (in)                  - New row number
;;
;; RAM Bank:    any (in) PCD8544_PORT (out)
;;
;;--------------------------------------------------------------------------------

pcd8544_char_set_row
        BANKSEL PCD8544_PORT

        PCD8544_COMMAND_MODE    ; Enter command mode

        movwf   pcd8544_char_row

        movlw   0x07
        andwf   pcd8544_char_row,F

        ;; Y=W

        movlw   0x40
        iorwf   pcd8544_char_row,W
        call    pcd8544_tx_byte

        PCD8544_DATA_MODE       ; Enter data mode

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine to change the column (row remains unchanged)
;; (Uses macros from pcd8544_basic.inc instead of subroutines for speed)
;;
;; Parameters:  W (in)                  - New column number
;;
;; RAM Bank:    any (in) PCD8544_PORT (out)
;;
;;--------------------------------------------------------------------------------

pcd8544_char_set_column
        BANKSEL PCD8544_PORT

        PCD8544_COMMAND_MODE    ; Enter command mode

        movwf   pcd8544_char_col

        movwf   pcd8544_temp
        bsf     STATUS,C
        rlf     pcd8544_temp,F  ; Multiply by 2 and add 1
        bcf     STATUS,C
        rlf     pcd8544_temp,F  ; Multiply by 2
        addwf   pcd8544_temp,F

        ;; X=5*W+2

        bsf     pcd8544_temp,7
        movf    pcd8544_temp,W
        call    pcd8544_tx_byte

        PCD8544_DATA_MODE       ; Enter data mode

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine to increment the row and reset the column to zero
;; (Uses macros from pcd8544_basic.inc instead of subroutines for speed)
;;
;; Parameters:  none
;;
;; RAM Bank:    any (in) PCD8544_PORT (out)
;;
;;--------------------------------------------------------------------------------

pcd8544_char_newline
        BANKSEL PCD8544_PORT

        PCD8544_COMMAND_MODE    ; Enter command mode

        ;; Reset the column

        clrf    pcd8544_char_col

        ;; X=2

        movlw   0x80 + 0x02
        call    pcd8544_tx_byte

        ;; Increment the row

        incf    pcd8544_char_row,F
        movlw   6
        subwf   pcd8544_char_row,W
        btfsc   STATUS,Z
        clrf    pcd8544_char_row

        ;; Y=W

        movlw   0x40
        iorwf   pcd8544_char_row,W
        call    pcd8544_tx_byte

        PCD8544_DATA_MODE       ; Enter data mode

        return


;;--------------------------------------------------------------------------------
;;
;; Subroutine to print an ASCII character
;;
;; Parameters:  W (in)                  - Character to print
;;
;; RAM Bank:    any (in) PCD8544_PORT (out)
;;
;;--------------------------------------------------------------------------------

pcd8544_char_print
        BANKSEL PCD8544_PORT

        movwf   pcd8544_temp

        ; Force characters above 127 to be 127

        btfsc   pcd8544_temp,7
        goto    pcd8544_char_print_127

        ; Handle special characters below 32

        movlw   10
        subwf   pcd8544_temp,W
        btfsc   STATUS,Z
        goto    pcd8544_char_print_newline

        ; Force other characters below 32 to be 127

        movlw   32
        subwf   pcd8544_temp,F
        btfss   STATUS,C
        goto    pcd8544_char_print_127

        ; Print the 5 bytes for the character

pcd8544_char_print_do_print

        call    pcd8544_charmap_byte_1
        call    pcd8544_tx_byte

        call    pcd8544_charmap_byte_2
        call    pcd8544_tx_byte

        call    pcd8544_charmap_byte_3
        call    pcd8544_tx_byte

        call    pcd8544_charmap_byte_4
        call    pcd8544_tx_byte

        call    pcd8544_charmap_byte_5
        call    pcd8544_tx_byte

        ;; Increment the column counter

        incf    pcd8544_char_col,F
        btfsc   pcd8544_char_col,4
        goto    pcd8544_char_newline

        return

pcd8544_char_print_127
        movlw   95              ; already subtracted 32
        movwf   pcd8544_temp
        goto    pcd8544_char_print_do_print

pcd8544_char_print_newline
        movlw   16
        subwf   pcd8544_char_col,F
        movlw   0

pcd8544_char_print_newline_loop
        call    pcd8544_tx_byte
        call    pcd8544_tx_byte
        call    pcd8544_tx_byte
        call    pcd8544_tx_byte
        call    pcd8544_tx_byte

        incfsz  pcd8544_char_col,F
        goto    pcd8544_char_print_newline_loop

        goto    pcd8544_char_newline


;;--------------------------------------------------------------------------------

pcd8544_charmap_byte_1

        movf    pcd8544_temp,W
        movwf   spi_temp

        movlw   LOW ($+9)       ; low 8 bits of address
        addwf   spi_temp,F      ; 8 bit addition
 
        movlw   HIGH ($+7)      ; high 5 bits of address
        btfsc   STATUS,C        ; skip if no 8 bit add overflow
        addlw   1
        movwf   PCLATH          ; load high address in latch
 
        movf    spi_temp,W      ; Move modified value into W
        goto    $+1             ; Jump to cause update of PCL high bits
        movwf   PCL             ; set PCL low bits

        retlw   0x00            ; ' ' byte 1
        retlw   0x00            ; '!' byte 1
        retlw   0x00            ; '"' byte 1
        retlw   0x14            ; '#' byte 1
        retlw   0x04            ; '$' byte 1
        retlw   0x00            ; '%' byte 1
        retlw   0x36            ; '&' byte 1
        retlw   0x00            ; ''' byte 1
        retlw   0x00            ; '(' byte 1
        retlw   0x00            ; ')' byte 1
        retlw   0x54            ; '*' byte 1
        retlw   0x10            ; '+' byte 1
        retlw   0x00            ; ',' byte 1
        retlw   0x10            ; '-' byte 1
        retlw   0x00            ; '.' byte 1
        retlw   0x60            ; '/' byte 1
        retlw   0x00            ; '0' byte 1
        retlw   0x00            ; '1' byte 1
        retlw   0x64            ; '2' byte 1
        retlw   0x22            ; '3' byte 1
        retlw   0x18            ; '4' byte 1
        retlw   0x2E            ; '5' byte 1
        retlw   0x3C            ; '6' byte 1
        retlw   0x02            ; '7' byte 1
        retlw   0x34            ; '8' byte 1
        retlw   0x0C            ; '9' byte 1
        retlw   0x00            ; ':' byte 1
        retlw   0x00            ; ';' byte 1
        retlw   0x00            ; '<' byte 1
        retlw   0x28            ; '=' byte 1
        retlw   0x00            ; '>' byte 1
        retlw   0x00            ; '?' byte 1
        retlw   0x3C            ; '@' byte 1
        retlw   0x7C            ; 'A' byte 1
        retlw   0x7E            ; 'B' byte 1
        retlw   0x3C            ; 'C' byte 1
        retlw   0x7E            ; 'D' byte 1
        retlw   0x7E            ; 'E' byte 1
        retlw   0x7E            ; 'F' byte 1
        retlw   0x3C            ; 'G' byte 1
        retlw   0x7E            ; 'H' byte 1
        retlw   0x00            ; 'I' byte 1
        retlw   0x20            ; 'J' byte 1
        retlw   0x7E            ; 'K' byte 1
        retlw   0x7E            ; 'L' byte 1
        retlw   0x7E            ; 'M' byte 1
        retlw   0x7E            ; 'N' byte 1
        retlw   0x3C            ; 'O' byte 1
        retlw   0x7E            ; 'P' byte 1
        retlw   0x3C            ; 'Q' byte 1
        retlw   0x7E            ; 'R' byte 1
        retlw   0x24            ; 'S' byte 1
        retlw   0x00            ; 'T' byte 1
        retlw   0x3E            ; 'U' byte 1
        retlw   0x1E            ; 'V' byte 1
        retlw   0x7E            ; 'W' byte 1
        retlw   0x66            ; 'X' byte 1
        retlw   0x06            ; 'Y' byte 1
        retlw   0x62            ; 'Z' byte 1
        retlw   0x00            ; '[' byte 1
        retlw   0x06            ; '\' byte 1
        retlw   0x00            ; ']' byte 1
        retlw   0x00            ; '^' byte 1
        retlw   0x80            ; '_' byte 1
        retlw   0x00            ; '`' byte 1
        retlw   0x30            ; 'a' byte 1
        retlw   0x7E            ; 'b' byte 1
        retlw   0x00            ; 'c' byte 1
        retlw   0x30            ; 'd' byte 1
        retlw   0x30            ; 'e' byte 1
        retlw   0x10            ; 'f' byte 1
        retlw   0x10            ; 'g' byte 1
        retlw   0x7E            ; 'h' byte 1
        retlw   0x00            ; 'i' byte 1
        retlw   0x00            ; 'j' byte 1
        retlw   0x7E            ; 'k' byte 1
        retlw   0x00            ; 'l' byte 1
        retlw   0x78            ; 'm' byte 1
        retlw   0x78            ; 'n' byte 1
        retlw   0x30            ; 'o' byte 1
        retlw   0xF8            ; 'p' byte 1
        retlw   0x10            ; 'q' byte 1
        retlw   0x78            ; 'r' byte 1
        retlw   0x00            ; 's' byte 1
        retlw   0x08            ; 't' byte 1
        retlw   0x38            ; 'u' byte 1
        retlw   0x00            ; 'v' byte 1
        retlw   0x38            ; 'w' byte 1
        retlw   0x48            ; 'x' byte 1
        retlw   0x58            ; 'y' byte 1
        retlw   0x48            ; 'z' byte 1
        retlw   0x08            ; '{' byte 1
        retlw   0x00            ; '|' byte 1
        retlw   0x41            ; '}' byte 1
        retlw   0x04            ; '~' byte 1
        retlw   0xFF            ; ERR byte 1

;;--------------------------------------------------------------------------------

pcd8544_charmap_byte_2

        movf    pcd8544_temp,W
        movwf   spi_temp

        movlw   LOW ($+9)       ; low 8 bits of address
        addwf   spi_temp,F      ; 8 bit addition
 
        movlw   HIGH ($+7)      ; high 5 bits of address
        btfsc   STATUS,C        ; skip if no 8 bit add overflow
        addlw   1
        movwf   PCLATH          ; load high address in latch
 
        movf    spi_temp,W      ; Move modified value into W
        goto    $+1             ; Jump to cause update of PCL high bits
        movwf   PCL             ; set PCL low bits

        retlw   0x00            ; ' ' byte 2
        retlw   0x00            ; '!' byte 2
        retlw   0x0E            ; '"' byte 2
        retlw   0x7F            ; '#' byte 2
        retlw   0x2A            ; '$' byte 2
        retlw   0x16            ; '%' byte 2
        retlw   0x49            ; '&' byte 2
        retlw   0x00            ; ''' byte 2
        retlw   0x3C            ; '(' byte 2
        retlw   0x42            ; ')' byte 2
        retlw   0x38            ; '*' byte 2
        retlw   0x10            ; '+' byte 2
        retlw   0x80            ; ',' byte 2
        retlw   0x10            ; '-' byte 2
        retlw   0x40            ; '.' byte 2
        retlw   0x10            ; '/' byte 2
        retlw   0x3C            ; '0' byte 2
        retlw   0x44            ; '1' byte 2
        retlw   0x52            ; '2' byte 2
        retlw   0x4A            ; '3' byte 2
        retlw   0x14            ; '4' byte 2
        retlw   0x4A            ; '5' byte 2
        retlw   0x4A            ; '6' byte 2
        retlw   0x62            ; '7' byte 2
        retlw   0x4A            ; '8' byte 2
        retlw   0x52            ; '9' byte 2
        retlw   0x6C            ; ':' byte 2
        retlw   0x80            ; ';' byte 2
        retlw   0x18            ; '<' byte 2
        retlw   0x28            ; '=' byte 2
        retlw   0x42            ; '>' byte 2
        retlw   0x04            ; '?' byte 2
        retlw   0x42            ; '@' byte 2
        retlw   0x12            ; 'A' byte 2
        retlw   0x4A            ; 'B' byte 2
        retlw   0x42            ; 'C' byte 2
        retlw   0x42            ; 'D' byte 2
        retlw   0x4A            ; 'E' byte 2
        retlw   0x0A            ; 'F' byte 2
        retlw   0x42            ; 'G' byte 2
        retlw   0x08            ; 'H' byte 2
        retlw   0x42            ; 'I' byte 2
        retlw   0x42            ; 'J' byte 2
        retlw   0x08            ; 'K' byte 2
        retlw   0x40            ; 'L' byte 2
        retlw   0x0C            ; 'M' byte 2
        retlw   0x0C            ; 'N' byte 2
        retlw   0x42            ; 'O' byte 2
        retlw   0x12            ; 'P' byte 2
        retlw   0x52            ; 'Q' byte 2
        retlw   0x12            ; 'R' byte 2
        retlw   0x4A            ; 'S' byte 2
        retlw   0x02            ; 'T' byte 2
        retlw   0x40            ; 'U' byte 2
        retlw   0x60            ; 'V' byte 2
        retlw   0x30            ; 'W' byte 2
        retlw   0x18            ; 'X' byte 2
        retlw   0x08            ; 'Y' byte 2
        retlw   0x52            ; 'Z' byte 2
        retlw   0x7E            ; '[' byte 2
        retlw   0x08            ; '\' byte 2
        retlw   0x42            ; ']' byte 2
        retlw   0x04            ; '^' byte 2
        retlw   0x80            ; '_' byte 2
        retlw   0x02            ; '`' byte 2
        retlw   0x48            ; 'a' byte 2
        retlw   0x48            ; 'b' byte 2
        retlw   0x30            ; 'c' byte 2
        retlw   0x48            ; 'd' byte 2
        retlw   0x68            ; 'e' byte 2
        retlw   0x7C            ; 'f' byte 2
        retlw   0xA8            ; 'g' byte 2
        retlw   0x08            ; 'h' byte 2
        retlw   0x48            ; 'i' byte 2
        retlw   0x40            ; 'j' byte 2
        retlw   0x10            ; 'k' byte 2
        retlw   0x42            ; 'l' byte 2
        retlw   0x08            ; 'm' byte 2
        retlw   0x08            ; 'n' byte 2
        retlw   0x48            ; 'o' byte 2
        retlw   0x28            ; 'p' byte 2
        retlw   0x28            ; 'q' byte 2
        retlw   0x10            ; 'r' byte 2
        retlw   0x50            ; 's' byte 2
        retlw   0x3E            ; 't' byte 2
        retlw   0x40            ; 'u' byte 2
        retlw   0x38            ; 'v' byte 2
        retlw   0x40            ; 'w' byte 2
        retlw   0x30            ; 'x' byte 2
        retlw   0xA0            ; 'y' byte 2
        retlw   0x68            ; 'z' byte 2
        retlw   0x2A            ; '{' byte 2
        retlw   0x00            ; '|' byte 2
        retlw   0x55            ; '}' byte 2
        retlw   0x02            ; '~' byte 2
        retlw   0x81            ; ERR byte 2

;;--------------------------------------------------------------------------------

pcd8544_charmap_byte_3

        movf    pcd8544_temp,W
        movwf   spi_temp

        movlw   LOW ($+9)       ; low 8 bits of address
        addwf   spi_temp,F      ; 8 bit addition
 
        movlw   HIGH ($+7)      ; high 5 bits of address
        btfsc   STATUS,C        ; skip if no 8 bit add overflow
        addlw   1
        movwf   PCLATH          ; load high address in latch
 
        movf    spi_temp,W      ; Move modified value into W
        goto    $+1             ; Jump to cause update of PCL high bits
        movwf   PCL             ; set PCL low bits

        retlw   0x00            ; ' ' byte 3
        retlw   0x5E            ; '!' byte 3
        retlw   0x00            ; '"' byte 3
        retlw   0x14            ; '#' byte 3
        retlw   0x7F            ; '$' byte 3
        retlw   0x08            ; '%' byte 3
        retlw   0x36            ; '&' byte 3
        retlw   0x0E            ; ''' byte 3
        retlw   0x42            ; '(' byte 3
        retlw   0x3C            ; ')' byte 3
        retlw   0x38            ; '*' byte 3
        retlw   0x7C            ; '+' byte 3
        retlw   0x60            ; ',' byte 3
        retlw   0x10            ; '-' byte 3
        retlw   0xE0            ; '.' byte 3
        retlw   0x08            ; '/' byte 3
        retlw   0x42            ; '0' byte 3
        retlw   0x7E            ; '1' byte 3
        retlw   0x52            ; '2' byte 3
        retlw   0x4E            ; '3' byte 3
        retlw   0x7E            ; '4' byte 3
        retlw   0x4A            ; '5' byte 3
        retlw   0x4A            ; '6' byte 3
        retlw   0x1A            ; '7' byte 3
        retlw   0x4A            ; '8' byte 3
        retlw   0x52            ; '9' byte 3
        retlw   0x6C            ; ':' byte 3
        retlw   0x6C            ; ';' byte 3
        retlw   0x24            ; '<' byte 3
        retlw   0x28            ; '=' byte 3
        retlw   0x24            ; '>' byte 3
        retlw   0x52            ; '?' byte 3
        retlw   0x99            ; '@' byte 3
        retlw   0x12            ; 'A' byte 3
        retlw   0x4A            ; 'B' byte 3
        retlw   0x42            ; 'C' byte 3
        retlw   0x42            ; 'D' byte 3
        retlw   0x4A            ; 'E' byte 3
        retlw   0x0A            ; 'F' byte 3
        retlw   0x52            ; 'G' byte 3
        retlw   0x08            ; 'H' byte 3
        retlw   0x7E            ; 'I' byte 3
        retlw   0x3E            ; 'J' byte 3
        retlw   0x34            ; 'K' byte 3
        retlw   0x40            ; 'L' byte 3
        retlw   0x0C            ; 'M' byte 3
        retlw   0x38            ; 'N' byte 3
        retlw   0x42            ; 'O' byte 3
        retlw   0x12            ; 'P' byte 3
        retlw   0x62            ; 'Q' byte 3
        retlw   0x12            ; 'R' byte 3
        retlw   0x52            ; 'S' byte 3
        retlw   0x7E            ; 'T' byte 3
        retlw   0x40            ; 'U' byte 3
        retlw   0x60            ; 'V' byte 3
        retlw   0x30            ; 'W' byte 3
        retlw   0x18            ; 'X' byte 3
        retlw   0x70            ; 'Y' byte 3
        retlw   0x4A            ; 'Z' byte 3
        retlw   0x42            ; '[' byte 3
        retlw   0x10            ; '\' byte 3
        retlw   0x42            ; ']' byte 3
        retlw   0x02            ; '^' byte 3
        retlw   0x80            ; '_' byte 3
        retlw   0x04            ; '`' byte 3
        retlw   0x48            ; 'a' byte 3
        retlw   0x48            ; 'b' byte 3
        retlw   0x48            ; 'c' byte 3
        retlw   0x48            ; 'd' byte 3
        retlw   0x58            ; 'e' byte 3
        retlw   0x12            ; 'f' byte 3
        retlw   0xA8            ; 'g' byte 3
        retlw   0x08            ; 'h' byte 3
        retlw   0x7A            ; 'i' byte 3
        retlw   0x80            ; 'j' byte 3
        retlw   0x10            ; 'k' byte 3
        retlw   0x7E            ; 'l' byte 3
        retlw   0x70            ; 'm' byte 3
        retlw   0x08            ; 'n' byte 3
        retlw   0x48            ; 'o' byte 3
        retlw   0x28            ; 'p' byte 3
        retlw   0x28            ; 'q' byte 3
        retlw   0x08            ; 'r' byte 3
        retlw   0x58            ; 's' byte 3
        retlw   0x48            ; 't' byte 3
        retlw   0x40            ; 'u' byte 3
        retlw   0x40            ; 'v' byte 3
        retlw   0x30            ; 'w' byte 3
        retlw   0x30            ; 'x' byte 3
        retlw   0xA0            ; 'y' byte 3
        retlw   0x58            ; 'z' byte 3
        retlw   0x55            ; '{' byte 3
        retlw   0x7E            ; '|' byte 3
        retlw   0x2A            ; '}' byte 3
        retlw   0x04            ; '~' byte 3
        retlw   0x81            ; ERR byte 3

;;--------------------------------------------------------------------------------

pcd8544_charmap_byte_4

        movf    pcd8544_temp,W
        movwf   spi_temp

        movlw   LOW ($+9)       ; low 8 bits of address
        addwf   spi_temp,F      ; 8 bit addition
 
        movlw   HIGH ($+7)      ; high 5 bits of address
        btfsc   STATUS,C        ; skip if no 8 bit add overflow
        addlw   1
        movwf   PCLATH          ; load high address in latch
 
        movf    spi_temp,W      ; Move modified value into W
        goto    $+1             ; Jump to cause update of PCL high bits
        movwf   PCL             ; set PCL low bits

        retlw   0x00            ; ' ' byte 4
        retlw   0x00            ; '!' byte 4
        retlw   0x0E            ; '"' byte 4
        retlw   0x7F            ; '#' byte 4
        retlw   0x2A            ; '$' byte 4
        retlw   0x34            ; '%' byte 4
        retlw   0x40            ; '&' byte 4
        retlw   0x00            ; ''' byte 4
        retlw   0x00            ; '(' byte 4
        retlw   0x00            ; ')' byte 4
        retlw   0x54            ; '*' byte 4
        retlw   0x10            ; '+' byte 4
        retlw   0x20            ; ',' byte 4
        retlw   0x10            ; '-' byte 4
        retlw   0x40            ; '.' byte 4
        retlw   0x06            ; '/' byte 4
        retlw   0x3C            ; '0' byte 4
        retlw   0x40            ; '1' byte 4
        retlw   0x4C            ; '2' byte 4
        retlw   0x32            ; '3' byte 4
        retlw   0x10            ; '4' byte 4
        retlw   0x32            ; '5' byte 4
        retlw   0x30            ; '6' byte 4
        retlw   0x06            ; '7' byte 4
        retlw   0x34            ; '8' byte 4
        retlw   0x3C            ; '9' byte 4
        retlw   0x00            ; ':' byte 4
        retlw   0x2C            ; ';' byte 4
        retlw   0x42            ; '<' byte 4
        retlw   0x28            ; '=' byte 4
        retlw   0x18            ; '>' byte 4
        retlw   0x0C            ; '?' byte 4
        retlw   0xA5            ; '@' byte 4
        retlw   0x7C            ; 'A' byte 4
        retlw   0x34            ; 'B' byte 4
        retlw   0x24            ; 'C' byte 4
        retlw   0x3C            ; 'D' byte 4
        retlw   0x42            ; 'E' byte 4
        retlw   0x02            ; 'F' byte 4
        retlw   0x34            ; 'G' byte 4
        retlw   0x7E            ; 'H' byte 4
        retlw   0x42            ; 'I' byte 4
        retlw   0x02            ; 'J' byte 4
        retlw   0x42            ; 'K' byte 4
        retlw   0x40            ; 'L' byte 4
        retlw   0x7E            ; 'M' byte 4
        retlw   0x7E            ; 'N' byte 4
        retlw   0x3C            ; 'O' byte 4
        retlw   0x0C            ; 'P' byte 4
        retlw   0xBC            ; 'Q' byte 4
        retlw   0x6C            ; 'R' byte 4
        retlw   0x24            ; 'S' byte 4
        retlw   0x02            ; 'T' byte 4
        retlw   0x3E            ; 'U' byte 4
        retlw   0x1E            ; 'V' byte 4
        retlw   0x7E            ; 'W' byte 4
        retlw   0x66            ; 'X' byte 4
        retlw   0x08            ; 'Y' byte 4
        retlw   0x46            ; 'Z' byte 4
        retlw   0x42            ; '[' byte 4
        retlw   0x60            ; '\' byte 4
        retlw   0x7E            ; ']' byte 4
        retlw   0x04            ; '^' byte 4
        retlw   0x80            ; '_' byte 4
        retlw   0x00            ; '`' byte 4
        retlw   0x78            ; 'a' byte 4
        retlw   0x30            ; 'b' byte 4
        retlw   0x48            ; 'c' byte 4
        retlw   0x7E            ; 'd' byte 4
        retlw   0x10            ; 'e' byte 4
        retlw   0x04            ; 'f' byte 4
        retlw   0x70            ; 'g' byte 4
        retlw   0x70            ; 'h' byte 4
        retlw   0x40            ; 'i' byte 4
        retlw   0x7A            ; 'j' byte 4
        retlw   0x68            ; 'k' byte 4
        retlw   0x40            ; 'l' byte 4
        retlw   0x08            ; 'm' byte 4
        retlw   0x70            ; 'n' byte 4
        retlw   0x30            ; 'o' byte 4
        retlw   0x10            ; 'p' byte 4
        retlw   0xF8            ; 'q' byte 4
        retlw   0x10            ; 'r' byte 4
        retlw   0x28            ; 's' byte 4
        retlw   0x20            ; 't' byte 4
        retlw   0x78            ; 'u' byte 4
        retlw   0x38            ; 'v' byte 4
        retlw   0x40            ; 'w' byte 4
        retlw   0x48            ; 'x' byte 4
        retlw   0x78            ; 'y' byte 4
        retlw   0x48            ; 'z' byte 4
        retlw   0x41            ; '{' byte 4
        retlw   0x00            ; '|' byte 4
        retlw   0x08            ; '}' byte 4
        retlw   0x02            ; '~' byte 4
        retlw   0x81            ; ERR byte 4

;;--------------------------------------------------------------------------------

pcd8544_charmap_byte_5

        movf    pcd8544_temp,W
        movwf   spi_temp

        movlw   LOW ($+9)       ; low 8 bits of address
        addwf   spi_temp,F      ; 8 bit addition
 
        movlw   HIGH ($+7)      ; high 5 bits of address
        btfsc   STATUS,C        ; skip if no 8 bit add overflow
        addlw   1
        movwf   PCLATH          ; load high address in latch
 
        movf    spi_temp,W      ; Move modified value into W
        goto    $+1             ; Jump to cause update of PCL high bits
        movwf   PCL             ; set PCL low bits

        retlw   0x00            ; ' ' byte 5
        retlw   0x00            ; '!' byte 5
        retlw   0x00            ; '"' byte 5
        retlw   0x14            ; '#' byte 5
        retlw   0x10            ; '$' byte 5
        retlw   0x00            ; '%' byte 5
        retlw   0x00            ; '&' byte 5
        retlw   0x00            ; ''' byte 5
        retlw   0x00            ; '(' byte 5
        retlw   0x00            ; ')' byte 5
        retlw   0x00            ; '*' byte 5
        retlw   0x10            ; '+' byte 5
        retlw   0x00            ; ',' byte 5
        retlw   0x00            ; '-' byte 5
        retlw   0x00            ; '.' byte 5
        retlw   0x00            ; '/' byte 5
        retlw   0x00            ; '0' byte 5
        retlw   0x00            ; '1' byte 5
        retlw   0x00            ; '2' byte 5
        retlw   0x00            ; '3' byte 5
        retlw   0x00            ; '4' byte 5
        retlw   0x00            ; '5' byte 5
        retlw   0x00            ; '6' byte 5
        retlw   0x00            ; '7' byte 5
        retlw   0x00            ; '8' byte 5
        retlw   0x00            ; '9' byte 5
        retlw   0x00            ; ':' byte 5
        retlw   0x00            ; ';' byte 5
        retlw   0x00            ; '<' byte 5
        retlw   0x00            ; '=' byte 5
        retlw   0x00            ; '>' byte 5
        retlw   0x00            ; '?' byte 5
        retlw   0x1E            ; '@' byte 5
        retlw   0x00            ; 'A' byte 5
        retlw   0x00            ; 'B' byte 5
        retlw   0x00            ; 'C' byte 5
        retlw   0x00            ; 'D' byte 5
        retlw   0x00            ; 'E' byte 5
        retlw   0x00            ; 'F' byte 5
        retlw   0x00            ; 'G' byte 5
        retlw   0x00            ; 'H' byte 5
        retlw   0x00            ; 'I' byte 5
        retlw   0x00            ; 'J' byte 5
        retlw   0x00            ; 'K' byte 5
        retlw   0x00            ; 'L' byte 5
        retlw   0x00            ; 'M' byte 5
        retlw   0x00            ; 'N' byte 5
        retlw   0x00            ; 'O' byte 5
        retlw   0x00            ; 'P' byte 5
        retlw   0x00            ; 'Q' byte 5
        retlw   0x00            ; 'R' byte 5
        retlw   0x00            ; 'S' byte 5
        retlw   0x00            ; 'T' byte 5
        retlw   0x00            ; 'U' byte 5
        retlw   0x00            ; 'V' byte 5
        retlw   0x00            ; 'W' byte 5
        retlw   0x00            ; 'X' byte 5
        retlw   0x06            ; 'Y' byte 5
        retlw   0x00            ; 'Z' byte 5
        retlw   0x00            ; '[' byte 5
        retlw   0x00            ; '\' byte 5
        retlw   0x00            ; ']' byte 5
        retlw   0x00            ; '^' byte 5
        retlw   0x00            ; '_' byte 5
        retlw   0x00            ; '`' byte 5
        retlw   0x00            ; 'a' byte 5
        retlw   0x00            ; 'b' byte 5
        retlw   0x00            ; 'c' byte 5
        retlw   0x00            ; 'd' byte 5
        retlw   0x00            ; 'e' byte 5
        retlw   0x00            ; 'f' byte 5
        retlw   0x00            ; 'g' byte 5
        retlw   0x00            ; 'h' byte 5
        retlw   0x00            ; 'i' byte 5
        retlw   0x00            ; 'j' byte 5
        retlw   0x00            ; 'k' byte 5
        retlw   0x00            ; 'l' byte 5
        retlw   0x70            ; 'm' byte 5
        retlw   0x00            ; 'n' byte 5
        retlw   0x00            ; 'o' byte 5
        retlw   0x00            ; 'p' byte 5
        retlw   0x00            ; 'q' byte 5
        retlw   0x00            ; 'r' byte 5
        retlw   0x00            ; 's' byte 5
        retlw   0x00            ; 't' byte 5
        retlw   0x00            ; 'u' byte 5
        retlw   0x00            ; 'v' byte 5
        retlw   0x38            ; 'w' byte 5
        retlw   0x00            ; 'x' byte 5
        retlw   0x00            ; 'y' byte 5
        retlw   0x00            ; 'z' byte 5
        retlw   0x00            ; '{' byte 5
        retlw   0x00            ; '|' byte 5
        retlw   0x00            ; '}' byte 5
        retlw   0x00            ; '~' byte 5
        retlw   0xFF            ; ERR byte 5

;;--------------------------------------------------------------------------------
