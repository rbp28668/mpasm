/***************************************
 $Header: /home/amb/pic/projects/07_rs232-to-spi/RCS/initmmc.c,v 1.4 2008/05/04 13:53:33 amb Exp $

 Program to initialise an MMC/SD card using SPI connection.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2006,07,08 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#include "../piclib/rs232.h"

int main(int argc, char **argv)
{
 int fd;
 char *port=argv[1];
 unsigned char cmd[3];

 if(argc!=2)
   {
    fprintf(stderr,"Usage: initmmc <serial-device-name>\n");
    return(1);
   }

 /* Open serial port */

 fd=rs232_open(port,38400,1);

 if(fd<0)
   {
    fprintf(stderr,"Error opening %s\n",port);
    return(1);
   }

 /* Initialise the process */

 cmd[0]='I';
 rs232_write(fd,cmd,1);

 /* Finish */

 rs232_read(fd,cmd,3);

 rs232_close(fd);

 if(cmd[0]!='O' || cmd[1]!='K')
   {
    fprintf(stderr,"An error occured.\n");
    return(1);
   }

 return(0);
}

