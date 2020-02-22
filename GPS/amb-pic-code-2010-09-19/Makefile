#
# Makefile for PIC programs
#
# Written by Andrew M. Bishop
#
# This file Copyright 2006,2008 Andrew M. Bishop
# It may be distributed under the GNU Public License, version 2, or
# any higher version.  See section COPYING of the GNU Public license
# for conditions under which this file may be redistributed.
#

MAKEFILES=$(wildcard */Makefile)
DIRS=$(dir $(MAKEFILES))

########

all:
	@for d in $(DIRS); do \
	  $(MAKE) -C $$d ; \
	done

########

clean:
	@for d in $(DIRS); do \
	  $(MAKE) -C $$d clean ; \
	done

########

distclean:
	@for d in $(DIRS); do \
	  $(MAKE) -C $$d distclean ; \
	done
