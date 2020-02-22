                                RS232 to SPI converter
                                ======================

Important
---------

1) SD card must be operated at 3.3V (i.e. RS232 must be at 3.3V).

2) Programming the PIC must be done with a 5V power supply to it.


Operation
---------

The device acts as a slave to the RS232 interface.  The PIC waits for commands
to come in on the RS232 and then performs the requested action.

The action is selected by sending a single byte:

S - Generic functions.

I - Send an MMC/SD initialisation sequence.

R - Send an MMC/SD sector read sequence.

W - Send an MMC/SD sector write sequence.

C - Set the SPI clock divider ratio in the PIC.


Generic Commands
----------------

After entering generic mode the PIC will stay in that mode until it is commanded
to exit.  The commands that can be used in generic mode are:

S - Exit generic mode.

E - Enable SPI chip select line.

D - Disable SPI chip select line.

W - Write bytes.

R - Read bytes.

X - Exchange bytes.


For the 'S', 'D' and 'E' commands there is nothing else to do.

For the Write command the next byte must be the length of the data (use 0 to
mean 256) followed by that many bytes of data.  The data is sent out of the SPI
interface and the incoming data is ignored.

For the Read command the next byte must be the length of the data (use 0 to mean
256).  This many dummy bytes are sent out of the SPI interface and the incoming
data is sent back on the RS232.

For the Exchange command the next byte must be the length of the data (use 0 to
mean 256) followed by that many bytes of data.  The data is sent out of the SPI
interface and the incoming data is sent back on the RS232.


MMC/SD Initialisation
---------------------

This command sends the MMC/SD initialisation sequence from the SPI library
function.  Normally this is just a few byytes of dummy data followed by CMD0 and
CMD1 from the SD card specification.  The result sent back is "OK\n" if there
was no error or "E9\n" in case of an error.


MMC/SD Read Sector
------------------

The next three bytes must be the block number (not the address) to read (MSByte
first).  The data sent back on the RS232 interface will be the 512 bytes of data
from the sector followed by "OK\n" if there was no error or "E9\n" in case of an
error.


MMC/SD Write Sector
-------------------

The next three bytes must be the block number (not the address) to write (MSByte
first).  This must be followed by 512 bytes of data to write.  The result sent
back is "OK\n" if there was no error or "E9\n" in case of an error.


SPI Speed
---------

The next byte must have the value 0, 1, 2 or 3 and this value will be written
into the two LSBits of the SSPCON register which will set the divide ratio.  See
the Microchip manual for details.


Test Programs
-------------

There are a number of test programs that use this device.

initmmc.c - Initialises an SD/MMC card.

loadmmc.c - Loads a number of blocks of data into an SD/MMC card.  The block
            number and length are on the command line and the data is read from
            standard input.

dumpmmc.c - Dumps a number of blocks of data from an SD/MMC card.  The block
            number and length are on the command line and the data is written to
            standard output.

testmmc.c - Sends a number of commands using the generic functions to initialise
            an SD/MMC card, report back the type of card (V1.x or V2.x) and then
            read out the internal registers containing card details, serial
            number etc.
