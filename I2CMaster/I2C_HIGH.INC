;**********************************************************************************************************
;
;                               I2C Master : General Purpose Macros & Subroutines
;
;               High Level Routines, Uses Low level Routines (in I2C_LOW.ASM)                                   
;
;
;       Program:          I2C_HIGH.ASM 
;       Revision Date:   
;                         1-16-97      Compatibility with MPASMWIN 1.40
;
;**********************************************************************************************************


;**********************************************************************************************************
;
;                               I2C_TEST_DEVICE
; MACRO
;
;   If Slave Device is listening, then _SlaveActive bit is set, else is cleared
;
;	Parameter :  NONE
;
;   Sequence Of Operations :
;       S-SlvAW-A-P
;   If A is +ve device is listening, else either busy, not present or error condition
;
;   This test may also be used to check for eample if a Serial EEPROM is in internal programming
;   mode
;
;  NOTE : The address of the slave must be loaded into SlaveAddress Registers, and 10 or 8 bit
;               mode addressing must be set
;**********************************************************************************************************

I2C_TEST_DEVICE         MACRO
			
			call    IsSlaveActive           ; TEMP ???? : Assembler Error with this MACRO

			ENDM
;
;
;               Test If A Device of SlaveAddr Is Present on Bus
;
;  The Slave Address Is put on the bus and if ACK it is present, if NACK not present
;  or may be device is not responding. The presense can be checked constantly by a master
;  (for ex. the Operating System on an Access.Bus may constantly issue this command)
;
;   Assume the Slave Address (10 or 8 bit) is loaded in SlaveAddr
;   Set  _10BitAddr bit in Control Reg to 1 if 10 bit Address slave else 0
;
;   Returns  1 in _SlaveActive Bit if slave is responding else a 0
;  
;

IsSlaveActive
		bcf     _Slave_RW       ; set for write operation
		call    TxmtStartBit    ; send START bit
		call    Txmt_Slave_Addr ; if successful, then _Txmt_Success bit is set
;
		bcf     _SlaveActive
		btfss   _ACK_Error      ; skip if NACK, device is not present or not responding
		bsf     _SlaveActive    ; ACK received, device present & listening
		call    TxmtStopBit   
		return
;
;**********************************************************************************************************
;                               I2C_WRITE
;
;  A basic macro for writing a block of data to a slave
;
;  Parameters :
;               _BYTES_                 #of bytes starting from RAM pointer _SourcePointer_
;               _SourcePointer_         Data Start Buffer pointer in RAM (file Registers)
;
;   Sequence :
;               S-SlvAW-A-D[0]-A.....A-D[N-1]-A-P
;
;  If an error occurs then the routine simply returns and user should check for
;       flags in Bus_Status Reg (for eg. _Txmt_Success flag)
;
;  NOTE : The address of the slave must be loaded into SlaveAddress Registers, and 10 or 8 bit
;               mode addressing must be set
;**********************************************************************************************************


I2C_WR          MACRO   _BYTES_, _SourcePointer_

		movlw   _BYTES_
		movwf   tempCount
		movlw   _SourcePointer_
		movwf   FSR

		call    _i2c_block_write
		call    TxmtStopBit     ; Issue a stop bit for slave to end transmission
		
		ENDM

_i2c_block_write:
		call    TxmtStartBit    ; send START bit
		bcf     _Slave_RW       ; set for write operation
		call    Txmt_Slave_Addr ; if successful, then _Txmt_Success bit is set
;
_block_wr1_loop:
		btfss   _Txmt_Success
		return
		movf    INDF,W
		movwf   DataByte        ; start from the first byte starting at _DataPointer_
		incf    FSR, F            
		call    SendData        ; send next byte, bus is our's !
		decfsz  tempCount, F
		goto    _block_wr1_loop         ; loop until desired bytes of data transmitted to slave
		return
;
;**********************************************************************************************************

;**********************************************************************************************************
;                            I2C_WRITE_SUB
;                               
;  Writes a message just like I2C_WRITE, except that the data is preceeded by a sub-address
;  to a slave device.
;               Eg. : A serial EEPROM would need an address of memory location for Random Writes
;
;  Parameters :
;               _BYTES_                 #of bytes starting from RAM pointer _SourcePointer_ (constant)
;               _SourcePointer_         Data Start Buffer pointer in RAM (file Registers)
;               _Sub_Address_           Sub-address of Slave (constant)
;
;   Sequence :
;               S-SlvAW-A-SubA-A-D[0]-A.....A-D[N-1]-A-P
;
;  If an error occurs then the routine simply returns and user should check for
;       flags in Bus_Status Reg (for eg. _Txmt_Success flag
;
;       Returns :       WREG = 1 on success, else WREG = 0
;
;  NOTE : The address of the slave must be loaded into SlaveAddress Registers, and 10 or 8 bit
;               mode addressing must be set
;
;  COMMENTS :
;               I2C_WR may prove to be more efficient than this macro in most situations
;               Advantages will be found for Random Address Block Writes for Slaves with
;               Auto Increment Sub-Addresses (like Microchip's 24CXX series Serial EEPROMS)
;
;**********************************************************************************************************

I2C_WR_SUB      MACRO   _BYTES_, _SourcePointer_, _Sub_Address_

		movlw   (_BYTES_ + 1)
		movwf   tempCount

		movlw   (_SourcePointer_ - 1)
		movwf   FSR

		movf    INDF,W
		movwf   StoreTemp_1     ; temporarily store contents of (_SourcePointer_ -1)
		movlw   _Sub_Address_
		movwf   INDF           ; store temporarily the sub-address at (_SourcePointer_ -1)

		call    _i2c_block_write        ; write _BYTES_+1 block of data

		movf    StoreTemp_1,W
		movwf   (_SourcePointer_ - 1)   ; restore contents of (_SourcePointer_ - 1)
		
		call    TxmtStopBit     ; Issue a stop bit for slave to end transmission

		ENDM

;**********************************************************************************************************
;                               I2C_WR_SUB_SWINC
;
;  Parameters :
;               _BYTES_                 #of bytes starting from RAM pointer _SourcePointer_ (constant)
;               _SourcePointer_         Data Start Buffer pointer in RAM (file Registers)
;               _Sub_Address_           Sub-address of Slave (constant)
;
;   Sequence :
;               S-SlvAW-A-(SubA+0)-A-D[0]-A-P
;               S-SlvAW-A-(SubA+1)-A-D[1]-A-P
;                       and so on until #of Bytes
;
;  If an error occurs then the routine simply returns and user should check for
;       flags in Bus_Status Reg (for eg. _Txmt_Success flag
;
;       Returns :       WREG = 1 on success, else WREG = 0
;
;  COMMENTS : Very In-efficient, Bus is given up after every Byte Write
;
;               Some I2C devices addresed with a sub-address do not increment automatically
;       after an access of each byte. Thus a block of data sent must have a sub-address
;       followed by a data byte.
;  
;**********************************************************************************************************

I2C_WR_SUB_SWINC        MACRO   _BYTES_, _SourcePointer_, _Sub_Address_

                       variable i              ; TEMP ???? : Assembler Does Not Support This

                       i = 0

                       .while (i < _BYTES_)
                               	movf    (_Source_Pointer_ + i),W
                               	movwf   SrcPtr
                               	movf    (_Sub_Address_ + i),W
                               	movwf   SubAddr                                         
				call    _i2c_byte_wr_sub        ; write a byte of data at sub address

                                i++
                        .endw

			ENDM

;
;
;  Write 1 Byte Of Data (in SrcPtr) to slave at sub-address (SubAddr)
;

_i2c_byte_wr_sub:
		call    TxmtStartBit    ; send START bit
		bcf     _Slave_RW       ; set for write operation
		call    Txmt_Slave_Addr ; if successful, then _Txmt_Success bit is set
		btfss   _Txmt_Success
		goto    _block_wr1_fail ; end
		movf    SubAddr,W
		movwf   DataByte        ; start from the first byte starting at _DataPointer_
		call    SendData        ; send next byte
		btfss   _Txmt_Success
		goto    _block_wr1_fail ; end
		movf    SrcPtr,W
		movwf   DataByte        ; start from the first byte starting at _DataPointer_
		call    SendData        ; send next byte
		btfss   _Txmt_Success
		goto    _block_wr1_fail         ; failed, return 0 in WREG
		goto    _block_wr1_pass         ; successful, return 1 in WREG
;
; return back to called routine from either _block_wr1_pass or _block_wr1_fail
;
_block_wr1_fail:
		call    TxmtStopBit     ; Issue a stop bit for slave to end transmission
		retlw   FALSE
_block_wr1_pass:
		call    TxmtStopBit     ; Issue a stop bit for slave to end transmission
		retlw   TRUE
;

;**********************************************************************************************************
;
;                               I2C_WR_MEM_BYTE
;
;  Some I2C devices like a EEPROM need to wait fo some time after every byte write
;  (when entered into internal programming mode). This MACRO is same as I2C_WR_SUB_SWINC,
;  but in addition adds a delay after each byte.
;          Some EERPOM memories (like Microchip's 24Cxx Series have on-chip data buffer), and hence
;               this routine is not efficient in these cases. In such cases use I2C_WR or I2C_WR_SUB
;               for a block of data and then insert a delay until the whole buffer is written.
;
;  Parameters :
;               _BYTES_                 #of bytes starting from RAM pointer _SourcePointer_ (constant)
;               _SourcePointer_         Data Start Buffer pointer in RAM (file Registers)
;               _Sub_Address_           Sub-address of Slave (constant)
;
;   Sequence :
;               S-SlvAW-A-(SubA+0)-A-D[0]-A-P
;                       Delay 1 mSec            ; The user can chnage this value to desired delay
;               S-SlvAW-A-(SubA+1)-A-D[1]-A-P
;                       Delay 1 mSec
;                       and so on until #of Bytes
;
;**********************************************************************************************************

I2C_WR_BYTE_MEM         MACRO   _BYTES_, _SourcePointer_, _Sub_Address_

                       variable i              ; TEMP ???? : Assembler Does Not Support This

                       i = 0

                       .while (i < _BYTES_)
                               	movf    (_Source_Pointer_ + i),W
                               	movwf   SrcPtr
                               	movf    (_Sub_Address_ + i),W
				movwf   SubAddr                                         
				call    _i2c_byte_wr_sub        ; write a byte of data at sub address
				call    Delay50uSec

                                i++
                        .endw


			ENDM

;**********************************************************************************************************
;                    I2C_WR_MEM_BUF
;
;   This Macro/Function writes #of _BYTES_ to an I2C memory device. However
;   some devices, esp. EEPROMs must wait while the device enters into programming 
;   mode. But some devices have an onchip temp data hold buffer and is used to 
;  store data before the device actually enters into programming mode.
;               For example, the 24C04 series of Serial EEPROMs from Microchip
;       have an 8 byte data buffer. So one can send 8 bytes of data at a time
;       and then the device enters programming mode. The master can either wait 
;       until a fixed time and then retry to program or can continiously poll
;       for ACK bit and then transmit the next Block of data for programming
;
;  Parameters :
;               _BYTES_         # of bytes to write to memory
;               _SourcePointer_ Pointer to the block of data
;               _SubAddress_    Sub-address of the slave
;               _Device__BUF_SIZE_      The on chip buffer size of the i2c slave
;
;  Sequence of operations
;			I2C_SUB_WR operations are performed in loop and each time
;			data buffer of BUF_SIZE is output to the device. Then
;			the device is checked for busy and when not busy another
;			block of data is written
;
;
;**********************************************************************************************************

I2C_WR_BUF_MEM  MACRO   _BYTES_, _SourcePointer_, _SubAddress_, _Device_BUF_SIZE_


		variable i, j


		if ( !_BYTES_)
			exitm

		elif ( _BYTES_ <=  _Device_BUF_SIZE_)

			I2C_WR_SUB	_BYTES_, _SourcePointer_, _SubAddress_

                        exitm

                else
		 
			i = 0
			j = (_BYTES_ / _Device_BUF_SIZE_)
			.while (i < j)

				I2C_WR_SUB     _Device_BUF_SIZE_, (_SourcePointer_ + i*_Device_BUF_SIZE_), (_SubAddress_ + i*_Device_BUF_SIZE_)

				call	IsSlaveActive
				btfss	_SlaveActive
				goto	$-2

                          i++
			.endw

			j = (_BYTES_ - i*_Device_BUF_SIZE_)

                        if (j)
				I2C_WR_SUB	j, (_SourcePointer_ + i*_Device_BUF_SIZE_), (_SubAddress_ + i*_Device_BUF_SIZE_)
                        endif

		endif

		ENDM


;**********************************************************************************************************
;
;                               I2C_READ
;
; The basic MACRO/procedure to read a block message from a slave device
;
;   Parameters :
;               _BYTES_         :  constant : #of bytes to receive
;               _DestPointer_   :  destination pointer of RAM (File Registers)
;
;   Sequence :
;               S-SlvAR-A-D[0]-A-.....-A-D[N-1]-N-P
;
;   If last byte, then Master will NOT Acknowledge (send NACK)
;
;  NOTE : The address of the slave must be loaded into SlaveAddress Registers, and 10 or 8 bit
;               mode addressing must be set
;
;**********************************************************************************************************

I2C_READ        MACRO   _BYTES_, _DestPointer_


		movlw   (_BYTES_ -1)
		movwf   tempCount               ; -1 because, the last byte is used out of loop
		movlw   _DestPointer_   
		movwf   FSR                    ; FIFO destination address pointer

		call    _i2c_block_read               

		ENDM

_i2c_block_read:                
		call    TxmtStartBit    ; send START bit
		bsf     _Slave_RW       ; set for read operation
		bcf     _Last_Byte_Rcv  ; not a last byte to rcv
		call    Txmt_Slave_Addr ; if successful, then _Txmt_Success bit is set
		btfsc   _Txmt_Success
		goto    _block_rd1_loop ; end
		call    TxmtStopBit     ; Issue a stop bit for slave to end transmission
		retlw   FALSE           ; Error : may be device not responding
;
_block_rd1_loop:
		call    GetData
		movf    DataByte,W
		movwf   INDF                   ; start receiving data, starting at Destination Pointer
		incf    FSR, F
		decfsz  tempCount, F
		goto    _block_rd1_loop         ; loop until desired bytes of data transmitted to slave
		bsf     _Last_Byte_Rcv          ; last byte to rcv, so send NACK
		call    GetData
		movf    DataByte,W
		movwf   INDF
		call    TxmtStopBit             ; Issue a stop bit for slave to end transmission
		retlw   TRUE


;**********************************************************************************************************
;
;                               I2C_READ_SUB
;  This MACRO/Subroutine reads a message from a slave device preceeded by a write of the sub-address
;  Between the sub-addrers write & the following reads, a STOP condition is not issued and
;  a "REPEATED START" condition is used so that an other master will not take over the bus,
;  and also that no other master will overwrite the sub-address of the same salve.
;
;   This function is very commonly used in accessing Random/Sequential reads from a
;       memory device (e.g : 24Cxx serial of Serial EEPROMs from Microchip).
;
;  Parameters :
;               _BYTES_         # of bytes to read
;               _DestPointer_   The destination pointer of data to be received.
;               _BubAddress_    The sub-address of the slave
;
;  Sequence :
;		S-SlvAW-A-SubAddr-A-S-SlvAR-A-D[0]-A-.....-A-D[N-1]-N-P
;
;
;**********************************************************************************************************

I2C_READ_SUB    MACRO   _BYTES_, _DestPointer_, _SubAddress_

		bcf     _Slave_RW       ; set for write operation
		call    TxmtStartBit    ; send START bit
		call    Txmt_Slave_Addr ; if successful, then _Txmt_Success bit is set

		movlw   _SubAddress_
		movwf   DataByte        ; START address of EEPROM(slave 1)
		call    SendData        ; write sub address
;
; do not send STOP after this, use REPEATED START condition
;

		I2C_READ _BYTES_, _DestPointer_


		ENDM



;**********************************************************************************************************
;
;                               I2C_READ_STATUS
;
;  This Macro/Function reads a status word (1 byte) from slave. Several I2C devices can
;  send a status byte upon reception of a control byte
;               This is basically same as I2C_READ MACRO for reading a single byte
;
;   For example, in a Serial EEPROM (Microchip's 24Cxx serial EEPROMs) will send the memory
;       data at the current address location
;
;  On success WREG = 1 else = 0
;
;**********************************************************************************************************

I2C_READ_STATUS MACRO   _DestPointer_


		call    TxmtStartBit    ; send START bit
		bsf     _Slave_RW       ; set for read operation
		call    Txmt_Slave_Addr ; if successful, then _Txmt_Success bit is set
		btfsc   _Txmt_Success
		goto    _byte_rd1_loop  ; read a byte
		call    TxmtStopBit     ; Issue a stop bit for slave to end transmission
		retlw   FALSE           ; Error : may be device not responding
_byte_rd1_loop:
		bsf     _Last_Byte_Rcv          ; last byte to rcv, so send NACK
		call    GetData
		movf    DataByte,W
		movwf   _DestPointer_
		call    TxmtStopBit             ; Issue a stop bit for slave to end transmission
		btfss   _Rcv_Success
		retlw   FALSE
		retlw   TRUE

		ENDM

;**********************************************************************************************************

I2C_READ_BYTE   MACRO   _DestPointer_

		I2C_READ_STATUS MACRO   _DestPointer_

		ENDM


;**********************************************************************************************************
;
;                               I2C_WR_SUB_WR
;
;  This Macro write 2 Blocks of Data (variable length) to a slave at a sub-address. This
;  may be useful for devices which need 2 blocks of data in which the first block may be an
;  extended address of a slave device. For example, a large I2C memory device, or a teletext
;  device with an extended addressing scheme, may need multiple bytes of data in the 1st block
;  that represents the actual physical address and is followed by a 2nd block that actually
;  represents the data. 
;
;  Parameters :
;
;               _BYTES1_                1st block #of bytes
;               _SourcePointer1_        Start Pointer of the 1st block
;               _SubAddress_            Sub-Address of slave
;               _BYTES2_                2st block #of bytes
;               _SourcePointer2_        Start Pointer of the 2nd block
;
;  Sequence :
;               S-SlvW-A-SubA-A-D1[0]-A-....-D1[N-1]-A-D2[0]-A-.....A-D2[M-1]-A-P
;   
;  Note : This MACRO is basically same as calling I2C_WR_SUB twice, but
;               a STOP bit is not sent (bus is not given up) in between
;               the two I2C_WR_SUB
;
;  Check Txmt_Success flag for any transmission errors
;
;**********************************************************************************************************

I2C_WR_SUB_WR   MACRO   _COUNT1_, _SourcePointer1_, _Sub_Address_, _COUNT2_, _SourcePointer2_

		movlw   (_COUNT1_ + 1)
		movwf   tempCount
		movlw   (_SourcePointer1_ - 1)
		movwf   FSR

		movf    INDF,W
		movwf   StoreTemp_1     ; temporarily store contents of (_SourcePointer_ -1)
		movlw   _Sub_Address_
		movwf   INDF           ; store temporarily the sub-address at (_SourcePointer_ -1)
		call    _i2c_block_write        ; write _BYTES_+1 block of data
;
		movf    StoreTemp_1,W
		movwf   (_SourcePointer1_ - 1)   ; restore contents of (_SourcePointer_ - 1)
; Block 1 write over 
; Send Block 2
		movlw   _COUNT2_
		movwf   tempCount
		movlw   _SourcePointer2_
		movwf   FSR
		call    _block_wr1_loop
;
		call    TxmtStopBit     ; Issue a stop bit for slave to end transmission

		ENDM


;**********************************************************************************************************
;
;                               I2C_WR_SUB_RD
;
;  This macro writes a block of data from SourcePointer of length _COUNT1_ to a slave
;       at sub-address and then Reads a block of Data of length _COUNT2_ to destination
;       address pointer
;
;
;  Message Structure :
;              S-SlvW-A-SubA-A-D1[0]-A-.....-A-D1[N-1]-A-S-SlvR-A-D2[0]-A-......A-D2[M-1]-N-P
;
;  Parameters :
;		_COUNT1_		Length Of Source Buffer
;		_SourcePointer_		Source Pointer Address
;		_Sub_Address_		The Sub Address Of the slave
;		_COUNT2_		The length of Destination Buffer
;		_DestPointer_		The start address of Destination Pointer                   
;
;**********************************************************************************************************

I2C_WR_SUB_RD   MACRO   _COUNT1_, _SourcePointer_, _Sub_Address_, _COUNT2_, _DestPointer_

		movlw   (_COUNT1_ + 1)
		movwf   tempCount
		movlw   (_SourcePointer_ - 1)
		movwf   FSR

		movf    INDF,W
		movwf   StoreTemp_1     ; temporarily store contents of (_SourcePointer_ -1)
		movlw   _Sub_Address_
		movwf   INDF           ; store temporarily the sub-address at (_SourcePointer_ -1)
		call    _i2c_block_write        ; write _BYTES_+1 block of data
;
		movf    StoreTemp_1,W
		movwf   (_SourcePointer1_ - 1)   ; restore contents of (_SourcePointer_ - 1)
;
; Without sending a STOP bit, read a block of data by using a REPEATED
;  Start Condition
;
		I2C_READ        _COUNT2_, _DestPointer_

		ENDM


;**********************************************************************************************************
;
;                               I2C_WR_COM_WR
;
;  This Macro write 2 blocks of data buffers to a slave in one message. This way no need to give up
;  the bus after sending the first block.
;               For example, this kind of transaction is used in an LCD driver where a
;       a block of control & addresss info is needed and then another block of actual data
;       to be displayed is needed. 
;
;
;  Message Structure :
;               S-SlvW-A-D1[0]-A-.....A-D1[N-1]-A-D2[0]-A-......-A-D2[M-1]-A-P
;  NOTE : This message is same as calling two I2C_WR Macros, except that
;       the bus is not given up between the sending of 2 blocks (this is
;       done by not sending a STOP bit inbetween)
;
;  Parameters :
;		_COUNT1_		Length Of Source Buffer #1
;		_SourcePointer1_	Source Pointer Address of 1st buffer
;		_COUNT2_		The length of Destination Buffer
;		_SourcePointer2_	Source Pointer Address of 2nd Buffer
;
;**********************************************************************************************************

I2C_WR_COM_WR   MACRO   _COUNT1_, _SourcePointer1_, _COUNT2_, _SourcePointer2_

		movlw   _COUNT1_
		movwf   tempCount
		movlw   _SourcePointer1_
		movwf   FSR
		call    _i2c_block_write
;
; First block sent, now send 2nd block of data
;
		movlw   _COUNT2_
		movwf   tempCount
		movlw   _SourcePointer2__
		movwf   FSR
		call    _block_wr1_loop
;
		call    TxmtStopBit     ; End of Double buffer txmt

		ENDM


;**********************************************************************************************************
;                               INCLUDE I2C Low Level Routines Here
;**********************************************************************************************************

			include "i2c_low.inc"
