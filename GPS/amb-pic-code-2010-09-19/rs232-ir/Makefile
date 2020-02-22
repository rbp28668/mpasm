#
# Makefile for PIC programs
#
# Written by Andrew M. Bishop
#
# This file Copyright 2006,07,08 Andrew M. Bishop
# It may be distributed under the GNU Public License, version 2, or
# any higher version.  See section COPYING of the GNU Public license
# for conditions under which this file may be redistributed.
#

# Programs

GPASM=gpasm

GNETLIST=gnetlist

GSCHEM=gschem

DIA=dia

CC=gcc

# Program options

GPASMFLAGS=-M

CFLAGS=-Wall -O2
LDFLAGS=

# Compilation targets

ASM=program.asm
HEX=program.hex
DEP=program.d

C=$(wildcard *.c)
EXE=$(foreach f,$(C),$(basename $f))
DEP+=$(foreach f,$(C),$(addsuffix .d,$(basename $f)))
PICLIB=../piclib/libpiclib.a

CIRCUIT_SCH=circuit.sch
CIRCUIT_DRC=circuit.drc
CIRCUIT_PCB=circuit.pcb
CIRCUIT_NET=circuit.net
CIRCUIT_PNG=circuit.png

LAYOUT_DIA=layout.dia
LAYOUT_PNG=layout.png

########

all : $(HEX) $(EXE) $(CIRCUIT_DRC) $(CIRCUIT_NET) $(CIRCUIT_PCB) $(CIRCUIT_PNG) $(LAYOUT_PNG)

########

%.hex:%.asm
	$(GPASM) $(GPASMFLAGS) $< -o $@

########

%:%.c $(PICLIB)
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(PICLIB) -MMD -MP -MF $(addsuffix .d,$(basename $<))

########

$(CIRCUIT_DRC) : $(CIRCUIT_SCH)
	$(GNETLIST) -g drc2 $< -o $@
	-@rm -f gnetlist.log

$(CIRCUIT_NET) : $(CIRCUIT_SCH)
	$(GNETLIST) -g PCB $< -o $@
	-@rm -f gnetlist.log

$(CIRCUIT_PCB) : $(CIRCUIT_SCH)
	$(GNETLIST) -g gsch2pcb $< -o $@
	-@rm -f gnetlist.log

$(CIRCUIT_PNG) : $(CIRCUIT_SCH)
	$(GSCHEM) -p -s/usr/share/gEDA/scheme/image.scm $< -o $@
	-@rm -f gschem.log

########

$(LAYOUT_PNG) : $(LAYOUT_DIA)
	$(DIA) -t png $<

########

clean:
	rm -f *~
	rm -f *.cod *.lst *.hex.log
	rm -f gnetlist.log gschem.log

########

distclean: clean
	rm -f $(HEX)
	rm -f $(DEP)
	rm -f $(EXE)
	rm -f $(CIRCUIT_DRC)
	rm -f $(CIRCUIT_NET)
	rm -f $(CIRCUIT_PCB)
	rm -f $(CIRCUIT_PNG)
	rm -f $(LAYOUT_PNG)

########

$(DEP) : .FORCE
	@touch $@

include $(DEP)

########

$(PICLIB) : .FORCE
	$(MAKE) -C $(dir $(PICLIB))

########

.FORCE :
