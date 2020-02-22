;---------------------------------------------------------------------
; File: smb.inc
; Definitions for external SMB functions and callbacks.
; Define _SMBSLAVE when including in smbslave.asm
; Main program needs to implement SMBWrite, SMBRead to write and read
; data in response to I2C requests.  It also needs to implement SMBRdEnd
; which is called when a block read terminates, and also SMBPoll which
; will be called repeatedly whilst the I2C handler is waiting for input.
; The main program will need to call SMBInit and then go to SMBHandler 
; to configure the I2C handling.  Note that SMBInit changes the state
; of TRISB - ensure this is set up before calling SMBInit.
;--------------------------------------------------------------------

;---------------------------------------------------------------------
; Callbacks implemented by calling program.  Calling program needs
; to implement these.
;---------------------------------------------------------------------

#ifdef _SMBSLAVE
        ; Called with byte written to device in W.  Current address in smbaddr.
        extern SMBWrite

        ; Called to get byte from current SMB address (in smbaddr) and return 
        ; it in W
        extern SMBRead

        ; Called once a read from a slave is complete (master has
        ; asserted NACK at the end of the read).
        extern SMBRdEnd

        ; Called in polling loop to allow main app to "do stuff"
        extern SMBPoll

#else
        ; Called with byte written to device in W.  Current address in smbaddr.
        global SMBWrite

        ; Called to get byte from current SMB address (in smbaddr) and return 
        ; it in W
        global SMBRead

        ; Called once a read from a slave is complete (master has
        ; asserted NACK at the end of the read).
        global SMBRdEnd

        ; Called in polling loop
        global SMBPoll

#endif


;---------------------------------------------------------------------
; Entry points (and variable) for SMB slave code.
; Main program needs to reference these.
;---------------------------------------------------------------------

#ifdef _SMBSLAVE

        ; Call with left shifted slave address in W
        global SMBInit

        ; Goto to handle input
        global SMBHandler

        ; Current SMB (i.e. on device) address.
        global smbaddr

        ; Export SSP_Handler for cases where client wants to
        ; implement its own polling loop.
		global SSP_Handler

#else     
        ; Call with left shifted slave address in W
        extern SMBInit

        ; Goto to handle input
        extern SMBHandler

        ; Current SMB (i.e. on device) address.
        extern smbaddr

        ; Handler to call if we want to implement our
        ; own polling loop.
        extern SSP_Handler
#endif

