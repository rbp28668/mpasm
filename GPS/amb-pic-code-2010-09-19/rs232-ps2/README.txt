                                RS232 to PS/2 converter
                                =======================

Operation
---------

The device acts as a host for a PS/2 connection to either a mouse or a keyboard.
The PIC waits for a command to select the operating mode and then initialises
the device and returns the data from it.

The mode of operation is selected by sending a single byte:

K - PS/2 keyboard.

M - PS/2 mouse.

After selecting the mode of operation the only way to change it is to reset the
PIC and choose again.


Keyboard Mode
-------------

The PIC initialises the keyboard by sending it the reset command (0xff) and
waiting for the correct reply (0xfa, 0xaa).  Then it sends some commands to
select options (LEDs on, typematic rate) and sends all bytes from the keyboard
back down the RS232 interface.


Mouse Mode
----------

The PIC initialises the mouse by sending it the reset command (0xff) and waiting
for the correct reply (0xfa, 0xaa, 0x00).  Then it sends some commands to select
options (samplerate, resolution, scaling) enables stream mode and sends all
bytes from the mouse back down the RS232 interface.


Test Programs
-------------

There are a number of test programs that use this device.

keyboard_host.c - Decodes the keycodes and prints them out as hex.

mouse_host.c    - Decodes the data packet and prints the information out in a
                  readable form.
