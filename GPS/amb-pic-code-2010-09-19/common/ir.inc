;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/ir.inc,v 1.7 2010/09/19 09:44:50 amb Exp $
;;
;; Main include file for Infra-Red functions.
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2006,08,10 Andrew M. Bishop
;; It may be distributed under the GNU Public License, version 2, or
;; any higher version.  See section COPYING of the GNU Public license
;; for conditions under which this file may be redistributed.
;;
;;--------------------------------------------------------------------------------

;;--------------------------------------------------------------------------------
;;
;; Required Variables / Constants for Infra-Red functions
;;
;; Constants:   CLOCK           - The PIC clock speed in Hz
;;              IR_PORT         - Selects the port containing the RXD/TXD bits
;;              IR_TRIS         - Selects the tristate register for the IR_PORT
;;              IR_RXD          - The bit to use for RX data input
;;              IR_TXD          - The bit to use for TX data input
;;              IR_NBYTES       - The number of bytes to use for Tx/Rx data
;;
;; Variables:   ir_byte_1       - Stores LSByte of Rx/Tx data, same bank as IR_PORT
;;              ir_byte_2       - Stores a Byte of Rx/Tx data, same bank as IR_PORT
;;              ir_byte_3       - Stores a Byte of Rx/Tx data, same bank as IR_PORT
;;              ir_byte_4       - Stores a Byte of Rx/Tx data, same bank as IR_PORT
;;              ir_byte_5       - Stores a Byte of Rx/Tx data, same bank as IR_PORT
;;              ir_byte_6       - Stores a Byte of Rx/Tx data, same bank as IR_PORT
;;              ir_byte_7       - Stores a Byte of Rx/Tx data, same bank as IR_PORT
;;              ir_byte_8       - Stores MSByte of Rx/Tx data, same bank as IR_PORT
;;              ir_length       - Stores the length of the data in bits, same bank as IR_PORT
;;              ir_temp         - Temporary byte, same bank as IR_PORT
;;
;;--------------------------------------------------------------------------------


;;
;; Infra-Red Basic functions
;;

        include "../common/ir_basic.inc"

;;
;; Infra-Red functions in RC5 format (bit-banging)
;;

        include "../common/ir_rc5.inc"

;;
;; Infra-Red functions in SIRCS format (bit-banging)
;;

        include "../common/ir_sircs.inc"

;;
;; Infra-Red functions for Cable box (bit-banging)
;;

        include "../common/ir_cable.inc"

;;
;; Infra-Red functions for Panasonic TV (bit-banging)
;;

        include "../common/ir_panasonic.inc"

;;
;; Infra-Red functions in Samsung format (bit-banging)
;;

        include "../common/ir_samsung.inc"

;;
;; Infra-Red functions in NEC format (bit-banging)
;;

        include "../common/ir_nec.inc"

;;
;; Infra-Red functions for timing and carrier modulation
;;

        include "../common/ir_time.inc"

;;--------------------------------------------------------------------------------
