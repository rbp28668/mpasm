#!/bin/sh
# -*-tcl-*-
#
# Remote control receiver viewer
#
# Written by Andrew M. Bishop
#
# This file Copyright 2008, 2009 Andrew M. Bishop
# It may be distributed under the GNU Public License, version 2, or
# any higher version.  See section COPYING of the GNU Public license
# for conditions under which this file may be redistributed.
#

# the next line restarts using tclsh \
exec tclsh "$0" "$@"

package require Tk

# Load the piclib package

lappend auto_path "../piclib"

package require piclib

# Open the RS232 port

set port [lindex $argv 0]

if { ! [file exists $port] } { puts "Serial port '$port' does not exist" ; exit }

set fd [::piclib::rs232 open $port 38400 1]

# Remote control formats

lappend formats RC5
#                       active, inactive, active                   # T = 889
set start(RC5)  [list  34        0        0 ]; #  889    0    0  #   T  0  T
set zero(RC5)   [list  34       34        0 ]; #  889  889    0  #   T  T  0
set one(RC5)    [list   0       34       34 ]; #    0  889  889  #   0  T  T
set pause(RC5)  [list   0        0        0 ]; # none
set stop(RC5)   [list   0        0        0 ]; # none
set endian(RC5) msb
set coding(RC5) manchester

lappend formats SIRCS
#                         active, inactive, active                   # T = 600
set start(SIRCS)  [list  92       23        0 ]; # 2400  600    0  #  4T  T  0
set zero(SIRCS)   [list  23       23        0 ]; #  600  600    0  #   T  T  0
set one(SIRCS)    [list  46       23        0 ]; # 1200  600    0  #  2T  T  0
set pause(SIRCS)  [list   0        0        0 ]; # none
set stop(SIRCS)   [list   0        0        0 ]; # none
set endian(SIRCS) lsb
set coding(SIRCS) high-low

lappend formats Cable
#                         active, inactive, active                   # T = 700
set start(Cable)  [list 357      167        0 ]; # 9300 4342    0  # 13T 6T  0
set zero(Cable)   [list  26       80        0 ]; #  676 2080    0  #   T 3T  0
set one(Cable)    [list  26      167        0 ]; #  676 4342    0  #   T 6T  0
set pause(Cable)  [list   0        0        0 ]; # none
set stop(Cable)   [list  26       -1        0 ]; #  676 long    0  #   T --
set endian(Cable) lsb
set coding(Cable) high-low-stop

lappend formats Panasonic
#                             active, inactive, active                   # T = 400
set start(Panasonic)  [list 138       62        0 ]; # 3600 1600    0  #  9T 4T  0
set zero(Panasonic)   [list  16       16        0 ]; #  420  420    0  #   T  T  0
set one(Panasonic)    [list  16       48        0 ]; #  420 1260    0  #   T 3T  0
set pause(Panasonic)  [list   0        0        0 ]; # none
set stop(Panasonic)   [list  16       -1        0 ]; #  420 long    0
set endian(Panasonic) lsb
set coding(Panasonic) high-low-stop

lappend formats Samsung
#                           active, inactive, active                   # T = 546
set start(Samsung)  [list 173      173        0 ]; # 4500 4500    0  #  8T  8T  0
set zero(Samsung)   [list  21       21        0 ]; #  562  562    0  #   T   T  0
set one(Samsung)    [list  21       65        0 ]; #  562 1687    0  #   T  3T  0
set pause(Samsung)  [list   0        0        0 ]; # none
set stop(Samsung)   [list  21       -1        0 ]; #  562 long    0  #   T  --
set endian(Samsung) lsb
set coding(Samsung) high-low-stop

lappend formats NEC
#                       active, inactive, active                   # T = 546
set start(NEC)  [list 346      173        0 ]; # 9000 4500    0  #  8T  8T  0
set zero(NEC)   [list  21       21        0 ]; #  562  562    0  #   T   T  0
set one(NEC)    [list  21       65        0 ]; #  562 1687    0  #   T  3T  0
set pause(NEC)  [list   0        0        0 ]; # none
set stop(NEC)   [list  21       -1        0 ]; #  562 long    0  #   T  --
set endian(NEC) lsb
set coding(NEC) high-low-stop

lappend formats Daewoo
#                          active, inactive, active                   # T = 500
set start(Daewoo)  [list 320      160        0 ]; # 8000 4000    0  # 16T 8T  0
set zero(Daewoo)   [list  20       20        0 ]; #  500  500    0  #   T  T  0
set one(Daewoo)    [list  20       60        0 ]; #  500 1500    0  #   T 3T  0
set pause(Daewoo)  [list  20      160        0 ]; #  500 4000    0  #   T 8T  0
set stop(Daewoo)   [list  20       -1        0 ]; #  500 long    0  #   T --
set endian(Daewoo) lsb
set coding(Daewoo) high-low-stop

lappend formats Unknown_1
#                             active, inactive, active                   # T = 520
set start(Unknown_1)  [list 320      160        0 ]; # 8000 4000    0  # 16T 8T  0
set zero(Unknown_1)   [list  20       20        0 ]; #  500  500    0  #   T  T  0
set one(Unknown_1)    [list  20       60        0 ]; #  500 1500    0  #   T 3T  0
set pause(Unknown_1)  [list   0        0        0 ]; # none
set stop(Unknown_1)   [list  20       -1        0 ]; #  500 long    0  #   T --
set endian(Unknown_1) lsb
set coding(Unknown_1) high-low-stop

lappend formats Unknown_2
#                             active, inactive, active                   # T = 900
set start(Unknown_2)  [list 140      130        0 ]; # 3640 3380    0  #  4T 4T  0
set zero(Unknown_2)   [list  37       33        0 ]; #  962  858    0  #   T  T  0
set one(Unknown_2)    [list  37      100        0 ]; #  962 2600    0  #   T 3T  0
set pause(Unknown_2)  [list   0        0        0 ]; # none
set stop(Unknown_2)   [list  37       -1        0 ]; #  962 long    0  #   T --
set endian(Unknown_2) lsb
set coding(Unknown_2) high-low-stop


# Handle the main window

wm geometry . 1000x300

#wm resizable . 0 0

wm state . normal

wm title . "Raw Infra-Red Data"

set controls [frame .controls]
pack $controls -side top -fill x -expand no

set capture [button $controls.capture -text "Capture" -command capturedata]
pack $capture -side left

set format [lindex $formats 0]
foreach f $formats {
    set format_$f [radiobutton $controls.format_$f -text $f -variable format -value $f -state normal -command decodedata]
    eval pack \$format_$f -side left
}

set canvas [canvas .canvas]
pack $canvas -side bottom -fill both -expand yes


# Capture the data

set result {}
for {set i 0} {$i<1000} {incr i} {
    set invresult($i) 0
}
set invresult(1000) 1

proc capturedata {} {
    global fd result invresult canvas

    set result [rxir $fd]

    $canvas delete all

    set x 20
    set y 100
    set s 1
    set t 0

    foreach r $result {

        set x1 [expr $x + $t/2 ]
        set x2 [expr $x + ($t+$r)/2 ]

        if { $t > 0 } {
            $canvas create line $x1 $y $x1 [expr $y - 50] -width 2 -tag raw
        }

        if { $s == 1 } {
            $canvas create line $x1 [expr $y - 50]  $x2  [expr $y - 50] -width 2 -tag raw
        } else {
            $canvas create line $x1       $y        $x2        $y       -width 2 -tag raw
        }

        set prevt $t
        for {} {$t < [expr $prevt + $r]} {incr t} {
            set invresult($t) $s
        }

        set s [expr 1 - $s]
    }

    set invresult($t) 1
    incr t
    set invresult($t) 0

    update

    decodedata
}


# Decode the data

proc decodedata {} {
    global format canvas
    global start zero one pause stop endian coding

    $canvas delete decoded

    set x 20;                   # Starting point
    set y 200;                  # Bottom edge

    # start

    if { [set tt [testbit 0 $coding($format) $start($format)]] } {
        drawbit 0 $x $y $start($format) "Start"
        set t $tt
    } else {
        return
    }

    # bits

    set bits {}

    for {set bit 0} {1} {incr bit} {

        if { [set tt [testbit $t $coding($format) $one($format)]] } {
            drawbit $t $x $y $one($format) "1"
            set t $tt
            lappend bits 1
        } elseif { [set tt [testbit $t $coding($format) $zero($format)]] } {
            drawbit $t $x $y $zero($format) "0"
            set t $tt
            lappend bits 0
        } elseif { [set tt [testbit $t $coding($format) $pause($format)]] } {
            drawbit $t $x $y $pause($format) "Pause"
            set t $tt
            incr bit -1
        } else {
            if { [set tt [testbit $t $coding($format) $stop($format)]] } {
                drawbit $t $x $y $stop($format) "Stop"
                set t $tt
            }

            if { $endian($format) == "msb" } {
                set nbits {}
                for {set i [expr $bit - 1]} {$i >=0} {incr i -1} {
                    lappend nbits [lindex $bits $i]
                }
                set bits $nbits
            }

            puts -nonewline "$format : $bit bits : "
            for {set i [expr $bit - 1]} {$i >=0} {incr i -1} {
                puts -nonewline [lindex $bits $i]
                if { [expr ! ($i % 4)] } { puts -nonewline " " }
            }
            puts ""

            return
        }
    }

    update
}


# Draw a bit

proc drawbit {t x y states c} {
    global canvas invresult

    set l1 [lindex $states 0]
    set l2 [lindex $states 1]
    set l3 [lindex $states 2]

    set x0 [expr $x + $t/2 ]

    if { $l1 > 0 } {
        set x1 [expr $x + ($t+$l1)/2 ]
        $canvas create line $x0 [expr $y - 50] $x1  [expr $y - 50] -width 2 -tag decoded

        if { $l2 > 0 } {
            $canvas create line $x1 $y $x1 [expr $y - 50] -width 2 -tag decoded
        }
    } else {
        set x1 $x0
    }

    if { $l2 > 0 } {
        set x2 [expr $x + ($t+$l1+$l2)/2 ]
        $canvas create line $x1      $y        $x2        $y       -width 2 -tag decoded

        if { $l3 > 0 } {
            $canvas create line $x2 $y $x2 [expr $y - 50] -width 2 -tag decoded
        }
    } else {
        set x2 $x1
    }

    if { $l3 > 0 } {
        set x3 [expr $x + ($t+$l1+$l2+$l3)/2 ]
        $canvas create line $x2 [expr $y - 50]  $x3  [expr $y - 50] -width 2 -tag decoded
    } else {
        set x3 $x2
    }

    $canvas create line $x0 [expr $y - 160]  $x0  [expr $y + 30] -width 1 -tag decoded

    $canvas create text [expr ($x0+$x3)/2] [expr $y + 20] -text $c -tag decoded

    $canvas create line $x3 [expr $y - 160]  $x3  [expr $y + 30] -width 1 -tag decoded
}


# Test for a particular bit sequence

proc testbit {t coding states} {
    global canvas invresult

    set l1 [lindex $states 0]
    set l2 [lindex $states 1]
    set l3 [lindex $states 2]

    if { $l1 == 0 && $l2 == 0 && $l3 == 0 } {
        return 0
    }

    if { $coding == "manchester" } {

        if { $l1 > 0 } {
            set tt [expr int($t + $l1 * 0.75)]

            if { $invresult($tt) != 1 } {
                return 0
            }

            if { $l2 > 0 } {
                while {$invresult($tt)==1} {incr tt}; # re-synchronise with the edge
                if { $tt > ($t + $l1 * 1.25 + 2) } {
                    return 0
                }
                set t $tt
            } else {
                set t [expr $t + $l1]
            }
        }

        if { $l2 > 0 } {
            set tt [expr int($t + $l2 * 0.75)]

            if { $invresult($tt) != 0 } {
                return 0
            }

            if { $l3 > 0 } {
                while {$invresult($tt)==0} {incr tt}; # re-synchronise with the edge
                if { $tt > ($t + $l2 * 1.25 + 2) } {
                    return 0
                }
                set t $tt
            } else {
                set t [expr $t + $l2]
            }
        }

        if { $l3 > 0 } {
            set tt [expr int($t + $l3 * 0.75)]

            if { $invresult($tt) != 1 } {
                return 0
            }

            set t [expr $t + $l3]
        }

    } elseif { $coding == "high-low" } {

        set tt $t

        while {$invresult($tt)==1} {incr tt}; # re-synchronise with the edge
        if { $tt < ($t + $l1 * 0.75 - 2) || $tt > ($t + $l1 * 1.25 + 2) } {
            return 0
        }

        set t $tt

        while {$invresult($tt)==0} {incr tt}; # re-synchronise with the edge

        if { $tt < ($t + 200) } { # if this is the last bit
            if { $tt < ($t + $l2 * 0.75 - 2) || $tt > ($t + $l2 * 1.25 + 2) } {
                return 0
            }
        }

        set t $tt

    } elseif { $coding == "high-low-stop" } {

        set tt $t

        while {$invresult($tt)==1} {incr tt}; # re-synchronise with the edge
        if { $tt < ($t + $l1 * 0.75 - 2) || $tt > ($t + $l1 * 1.25 + 2) } {
            return 0
        }

        set t $tt

        while {$invresult($tt)==0} {incr tt}; # re-synchronise with the edge

        if { $l2 > 0 } { # specifically for stop bits
            if { $tt < ($t + $l2 * 0.75 - 2) || $tt > ($t + $l2 * 1.25 + 2) } {
                return 0
            }
        }

        set t $tt

    }

    return $t
}


# Read raw data

proc rxir {fd} {

    set R "R"
    set raw {}

    ::piclib::rs232 write $fd R 1

    ::piclib::rs232 read $fd raw 128

    binary scan $raw c* bytes

    set result {}
    set prevstate 128
    set prevlen 0

    for {set i 0} {$i<=127} {incr i} {
        set byte  [lindex $bytes $i]
        set state [expr $byte & 128 ]
        set len   [expr $byte & 127 ]

        if { $prevstate == $state } {
            set prevlen [expr $prevlen + $len ]
        } else {
            lappend result $prevlen

            set prevstate $state
            set prevlen   $len
        }
    }

    lappend result $prevlen

    return $result
}
