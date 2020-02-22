#!/bin/sh
# -*-tcl-*-
#
# SPI loopback test (connect dout to din)
#
# Written by Andrew M. Bishop
#
# This file Copyright 2007,08,09 Andrew M. Bishop
# It may be distributed under the GNU Public License, version 2, or
# any higher version.  See section COPYING of the GNU Public license
# for conditions under which this file may be redistributed.
#

# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# Load the piclib package

lappend auto_path "../piclib"

package require piclib

# Open the RS232 port, send some data, read some data and close the port again

set port [lindex $argv 0]

if { ! [file exists $port] } { puts "Serial port '$port' does not exist" ; exit }

set fd [::piclib::rs232 open $port 38400 1]

set alphabet "abcdefghijklmnopqrstuvwxyz"

::piclib::spi start $fd

::piclib::spi enable $fd

set outgoing $alphabet

puts "Write:   $outgoing"

::piclib::spi write $fd outgoing 26

set incoming {}

::piclib::spi read  $fd incoming 26

puts "Read:    $incoming"

set exchange $alphabet

::piclib::spi xchange $fd exchange 26

puts "Xchange: $exchange"

::piclib::spi disable $fd

::piclib::spi stop $fd

::piclib::rs232 close $fd
