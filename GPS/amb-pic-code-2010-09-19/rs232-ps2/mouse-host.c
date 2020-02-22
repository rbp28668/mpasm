/***************************************
 $Header: /home/amb/pic/projects/13_rs232-to-ps2/RCS/mouse-host.c,v 1.2 2008/05/04 13:53:33 amb Exp $

 Program to dump mouse data from RS232 connection.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2007,08 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "../piclib/rs232.h"


int main(int argc, char **argv)
{
 int fd;
 char *port=argv[1];
 unsigned char packet[3];

 if(argc!=2)
   {
    fprintf(stderr,"Usage: mouse-host <serial-device-name>\n");
    return(1);
   }

 /* Open serial port */

 fd=rs232_open(port,38400,1);

 if(fd<0)
   {
    fprintf(stderr,"Error opening %s\n",port);
    return(1);
   }

 write(fd,"M",1);               /* If already configured then nothing happens */

 /* Read the data and decode it */

 for(;;)
   {
    int left,middle,right;
    int xoverflow,yoverflow;
    int xmove,ymove;

    rs232_read(fd,packet,3);

    left  =packet[0]&0x01;
    right =packet[0]&0x02;
    middle=packet[0]&0x04;

    xoverflow=packet[0]&0x40;
    yoverflow=packet[0]&0x40;

    xmove=(unsigned int)packet[1]-(!!(packet[0]&0x10)*256);
    ymove=(unsigned int)packet[2]-(!!(packet[0]&0x20)*256);

    printf(" %s %s %s %4d%s %4d%s\n",left?"*":"-",middle?"*":"-",right?"*":"-",xmove,xoverflow?"!":" ",ymove,yoverflow?"!":" ");
   }

 /* Finish */

 rs232_close(fd);

 return(0);
}
