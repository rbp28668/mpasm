/***************************************
 $Header: /home/amb/pic/projects/13_rs232-to-ps2/RCS/keyboard-host.c,v 1.2 2008/05/04 13:53:33 amb Exp $

 Program to dump keyboard data from RS232 connection.
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
 unsigned char byte1,byte2,byte3,byte4,byte5;

 if(argc!=2)
   {
    fprintf(stderr,"Usage: keyboard-host <serial-device-name>\n");
    return(1);
   }

 /* Open serial port */

 fd=rs232_open(port,38400,1);

 if(fd<0)
   {
    fprintf(stderr,"Error opening %s\n",port);
    return(1);
   }

 write(fd,"K",1);               /* If already configured then nothing happens */

 /* Read the data and decode it */

 for(;;)
   {
    rs232_read(fd,&byte1,1);

    if(byte1==0xe0)
      {
       rs232_read(fd,&byte2,1);

       if(byte2==0xf0)
         {
          rs232_read(fd,&byte3,1);
          printf(" ^ %02x %02x\n",byte1,byte3);
         }
       else
          printf(" v %02x %02x\n",byte1,byte2);
      }
    else if(byte1==0xe1)
      {
       rs232_read(fd,&byte2,1);

       if(byte2==0xf0)
         {
          rs232_read(fd,&byte3,1);
          rs232_read(fd,&byte4,1);
          rs232_read(fd,&byte5,1);
          printf(" ^ %02x %02x %02x\n",byte1,byte3,byte5);
         }
       else
         {
          rs232_read(fd,&byte3,1);
          printf(" v %02x %02x %02x\n",byte1,byte2,byte3);
         }
      }
    else
      {
       if(byte1==0xf0)
         {
          rs232_read(fd,&byte2,1);
          printf(" ^ %02x\n",byte2);
         }
       else
          printf(" v %02x\n",byte1);
      }
   }

 /* Finish */

 rs232_close(fd);

 return(0);
}
