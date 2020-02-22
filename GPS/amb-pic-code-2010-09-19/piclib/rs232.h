/***************************************
 $Header: /home/amb/pic/piclib/RCS/rs232.h,v 1.5 2008/05/31 14:31:22 amb Exp $

 Header file for RS232 functions.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2006,07,08 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#ifndef RS232_H
#define RS232_H    /*+ To stop multiple inclusions. +*/

#include <sys/types.h>

int rs232_open(char *device,int speed,int flow);

size_t rs232_write(int fd, unsigned char *data,size_t length);

size_t rs232_read(int fd, unsigned char *data,size_t length);
size_t rs232_read_nowait(int fd, unsigned char *data,size_t length);

void rs232_rts(int fd, int state);
int rs232_cts(int fd);

void rs232_close(int fd);

#endif /* RS232_H */
