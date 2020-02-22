                                RS232 to Infra-Red
                                ------------------

Notes
-----

1) All remote controls have been reverse engineered based on actual remote
   controls.  Matching these to information on the internet has allowed some of
   them to be identified and named (RC5 and SIRCS).

2) The IR Rx/Tx use 940 nm and 38 kHz nominal modulation rate (26 us for
   transmit = 38.5 kHz).

3) The Sony Video requires that the command is repeated to make it work.


Operation
---------

The device acts as a slave to the RS232 interface.  The PIC waits for commands
to come in on the RS232 and then performs the requested action.

The action is selected by sending a single byte:

R - Receive raw waveform

T - Transmit raw waveform

r - Receive remote control code

t - Transmit remote control code


Receive Raw Waveform
--------------------

After sending the command to receive raw data the PIC will wait for the first
sign of an infra-red signal.  Then it will check at intervals of 26 us for the
presence or absence of the signal.  When there is a change in the signal it will
record this information.

The result sent back over the RS232 link is a series of 128 bytes.  Each one
contains the state of the infra-red signal (on or off) in the top bit and then a
count of the number of 26 us intervals for which it was in that state.


Transmit Raw Waveform
---------------------

After sending the signal to transmit raw data the PIC will expect to see 128
bytes formatted in the same way as for the raw receive format (see above).  The
PIC will then output this sequence of on and off states until all data is used.


Receive Remote Control Code
---------------------------

With the reception of remote control codes there must be a second byte
transmitted which specifies the type of remote control.

R - RC5 format, a common remote control format.

C - The format from my cable TV box.

S - SIRCS (Sony Infra-Red Control System) format.

P - The format used by my Panasonic TV.

The PIC will then wait for a signal matching that type of remote control and
decode it.

The data that is sent back on the RS232 interface is five bytes.  The first byte
is the length of the message (in bits).  The four remaining bytes are the
message itself stored in transmission order from the MSB of the first byte to
the LSB of the last byte (zero padding at the MSB end).


Transmit Remote Control Code
----------------------------

For the transmission of remote control codes the PIC expects the remote control
type (the same as for reception, above) followed by 5 bytes specifying the data
to transmit (same as for reception, above).


Test Programs
-------------

There are a number of test programs that use this device.

rxir.c     - Receives raw data and prints it out in two columns, the first being
             the state and the second being the time spent in that state.

txir.c     - Transmits raw data using the same format as that printed by the rxir
             program above.

rxremote.c - Receives remote control codes, the type of remote is spscified on
             the command line and the data received is printed out in
             hexadecimal.

txremote.c - Transmits remote control codes, the type of remote, the number of
             bits and the hexadecimal data are specified on the command line.
