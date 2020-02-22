;;--------------------------------------------------------------------------------
;; $Header: /home/amb/pic/common/RCS/delay.inc,v 1.3 2007/05/07 18:31:40 amb Exp $
;;
;; Main include file for delay functions.
;;
;; Written by Andrew M. Bishop
;;
;; This file Copyright 2006 Andrew M. Bishop
;; It may be distributed under the GNU Public License, version 2, or
;; any higher version.  See section COPYING of the GNU Public license
;; for conditions under which this file may be redistributed.
;;
;;--------------------------------------------------------------------------------

;;--------------------------------------------------------------------------------
;;
;; Required Variables / Constants for DELAY functions
;;
;; Variables:   delay_temp      - Stores temporary data
;;
;;--------------------------------------------------------------------------------


;;
;; Instruction cycle delay subroutines
;;

        include "../common/delay_cycle.inc"

;;
;; Microsecond delay subroutines
;;

        include "../common/delay_us.inc"

;;
;; Millisecond delay subroutines
;;

        include "../common/delay_ms.inc"


;;--------------------------------------------------------------------------------
