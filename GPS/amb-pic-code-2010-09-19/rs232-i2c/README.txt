                                RS232 to I2C Converter
                                ======================

Operation
---------

The device acts as a slave to the RS232 interface.  The PIC waits for commands
to come in on the RS232 and then performs the requested action.

The action is selected by sending a single byte:

S - Generic read/write function.

D - Dump the contents of an I2C EEPROM.

L - Load the contents of an I2C EEPROM.

E - Erase the contents of an I2C EEPROM.

After completing the requested command as described below the PIC will go back
to waiting for a new input command.


Generic Commands
----------------

In generic mode the input on the RS232 interface is expected to be a series of
bytes that make up a complete I2C command.  This data is stored in RAM until it
is complete.

The state machine that is implemented here is shown below, (the brackets show
the error number if the expected state is not reached).  In case of an error
with the states shown in the diagram the PIC will send back on the RS232
interface an 'E', the ASCII character of the digit for the error code and then a
newline.


            START
              | (2)
              v
            WRITE <---------------+
              | (4)               |
    +---------+---------+         |
    |         |         |         |
    v         v         v         |
  STOP       READ     RESTART     |
              | (4)     | (3)     |
              v         +---------+
             STOP

The letters that can be sent are:

S - This byte need not be sent again because it was the byte that enabled the
    generic mode in the first place.  In other words you can send a generic I2C
    command just by starting with the START state on the flowchart which selects
    the generic mode instead of the EEPROM modes.

w - A write command which must be followed by a byte that contains the number of
    bytes of data to write and then that many bytes of data which will be written.

r - A read command which must be followed by a byte that contains the number of
    bytes of data to read and then that many bytes of dummy data.

R - An I2C restart condition.

P - An I2C stop condition.

When the 'P' is sent on the RS232 interface the I2C command will be created and
sent on the I2C interface.  Any read bytes in the command stored in RAM will be
replaced with data read from the I2C interface.  The whole of the sequence of
bytes in RAM which consist of the original command with read data overwriting
the dummy data is then sent back on the RS232 interface followed by a newline
character.


Dump EEPROM
-----------

The next two bytes must be the length of the EEPROM sent MSByte first and the
value is rounded to a multiple of 32 bytes.

The PIC will then read 32 bytes at a time from the EEPROM into RAM in a single
I2C command and then send these bytes on the RS232.  When the complete EEPROM is
dumped the PIC will send the message "OK\n" on the RS232.


Load EEPROM
-----------

The next two bytes must be the length of the EEPROM sent MSByte first and the
value is rounded to a multiple of 32 bytes.

The PIC will then read 32 bytes at a time from the RS232 into RAM and then send
these bytes to the EEPROM in a single I2C command.  When the complete EEPROM is
written the PIC will send the message "OK\n" on the RS232.


Erase EEPROM
------------

The next two bytes must be the length of the EEPROM sent MSByte first and the
value is rounded to a multiple of 32 bytes.

The PIC will loop round writing 32 bytes of value 0xff to the EEPROM in a single
I2C command.  When the complete EEPROM is written the PIC will send the message
"OK\n" on the RS232.


Test Programs
-------------

There are a number of test programs that use this device.

testeeprom.c  - Sends some write commands to an EEPROM on the I2C interface and
                then read back what was written.

testds1307.c  - Sends commands to a DS1307 timekeeper IC connected to the I2C
                interface.

loadeeprom.c  - Loads a complete EEPROM with the data that is read in on standard
                input.

dumpeeprom.c  - Dumps out the entire contents of an EEPROM to standard output.

eraseeeprom.c - Erases the EEPROM.
