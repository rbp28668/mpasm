#
# Makefile for UNIX libraries for use with PIC hardware.
#
# Written by Andrew M. Bishop
#
# This file Copyright 2006 Andrew M. Bishop
# It may be distributed under the GNU Public License, version 2, or
# any higher version.  See section COPYING of the GNU Public license
# for conditions under which this file may be redistributed.
#

# Programs

CC=gcc
LD=gcc

# Program options

CFLAGS=-g -O2 -Wall
INCLUDES=

# Compilation targets

C=$(wildcard *.c)
D=$(foreach f,$(C),$(addsuffix .d,$(basename $f)))
O_STATIC=$(foreach f,$(C),$(addsuffix .static.o,$(basename $f)))
O_DYNAMIC=$(foreach f,$(C),$(addsuffix .dynamic.o,$(basename $f)))
L_STATIC=libpiclib.a
L_DYNAMIC=libpiclib.so

PKGINDEX=pkgIndex.tcl

########

all : $(L_STATIC) $(L_DYNAMIC) $(PKGINDEX)

########

$(L_STATIC) : $(O_STATIC)
	$(AR) -r $(L_STATIC) $(O_STATIC)

$(L_DYNAMIC) : $(O_DYNAMIC)
	$(LD) -shared $(O_DYNAMIC) -o $(L_DYNAMIC) -ltcl8.4

$(PKGINDEX) : piclib.tcl $(L_DYNAMIC)
	echo "pkg_mkIndex -verbose . piclib.tcl" | tclsh -

########

%.static.o:%.c
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@ -MMD -MP -MF $(addsuffix .d,$(basename $<))

%.dynamic.o:%.c
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@ -fpic

########

clean:
	rm -f *.o
	rm -f *~

########

distclean: clean
	-rm -f $(L_STATIC)
	-rm -f $(L_DYNAMIC)
	-rm -f $(D)

########

$(D) : .FORCE
	@touch $@

include $(D)

########

.FORCE :
