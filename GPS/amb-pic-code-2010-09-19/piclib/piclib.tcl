#
# Tcl interface to UNIX libraries for use with PIC hardware.
#
# Written by Andrew M. Bishop
#
# This file Copyright 2007,08 Andrew M. Bishop
# It may be distributed under the GNU Public License, version 2, or
# any higher version.  See section COPYING of the GNU Public license
# for conditions under which this file may be redistributed.
#

# Register the package

package provide piclib 1.0
package require Tcl


# Create the namespace

namespace eval ::piclib {
    # Export commands
    namespace export rs232 \
                     i2c \
                     spi \
                     mmc
}


# Find and load the compiled library

set piclib_so [regsub "piclib\.tcl$" [info script] "libpiclib.so"]

load $piclib_so piclib


# Create a top-level command for accessing the sub-functions

proc piclib {cmd args} {

    switch $cmd {

        rs232   { # usage:
                  #        set fd     [ open  <device> <speed> <flow>]
                  #                     close <fd>
                  #        set nread  [ read  <fd> <&data> <nbytes> ]
                  #        set nwrite [ write <fd> <&data> <nbytes> ]
                  #                     rts   <fd> <&state>
                  #        set state  [ cts  <fd> ]
                  eval ::piclib::rs232 $args }

        i2c     { # usage:
                  #        set result [ read{1,2,4}  <fd> <bus_address> <data_address> <&data> <nbytes> ]
                  #        set result [ write{1,2,4} <fd> <bus_address> <data_address> <&data> <nbytes> ]
                  eval ::piclib::i2c   $args }

        spi     { # usage:
                  #        speed   <fd> <speed>
                  #        start   <fd>
                  #        stop    <fd>
                  #        enable  <fd>
                  #        disable <fd>
                  #        set result [ write   <fd> <&data> <nbytes> ]
                  #        set result [ read    <fd> <&data> <nbytes> ]
                  #        set result [ xchange <fd> <&data> <nbytes> ]
                  eval ::piclib::spi   $args }

        mmc     { # usage:
                  #        cmd0    <fd>
                  #        cmd1    <fd>
                  #        cmd8    <fd>
                  #        cmd9    <fd>
                  #        cmd10   <fd>
                  #        cmd58   <fd>
                  #        acmd41  <fd>
                  #        acmd51  <fd>
                  #        wait_r1 <fd>
                  #        wait_r3 <fd>
                  #        wait_r7 <fd>
                  eval ::piclib::mmc   $args }

        default { error "Invalid piclib sub-command" }
    }
}
