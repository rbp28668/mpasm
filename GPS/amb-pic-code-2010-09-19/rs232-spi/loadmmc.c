/***************************************
 $Header: /home/amb/pic/projects/07_rs232-to-spi/RCS/loadmmc.c,v 1.4 2008/05/04 13:53:33 amb Exp $

 Program to load data into an MMC/SD card using SPI connection.
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

#define BLOCK_SIZE 512

int main(int argc, char **argv)
{
 int fd;
 char *port=argv[1];
 unsigned long start,count,address;
 unsigned char cmd[4];
 unsigned char block[BLOCK_SIZE];
 int n;

 if(argc!=4)
   {
    fprintf(stderr,"Usage: loadmmc <serial-device-name> <start-block> <number-blocks>\n");
    return(1);
   }

 start=atoi(argv[2]);
 count=atoi(argv[3]);

 /* Open serial port */

 fd=rs232_open(port,38400,1);

 if(fd<0)
   {
    fprintf(stderr,"Error opening %s\n",port);
    return(1);
   }

 /* Write a number of blocks */

 for(address=start;address<(start+count);address++)
   {
    fprintf(stderr,"\rWriting block: %6ld (%3ld/%3ld) ",address,address-start+1,count);

    /* Initialise the process */

    cmd[0]='W';
    cmd[1]=address>>16;
    cmd[2]=address>>8;
    cmd[3]=address;
    rs232_write(fd,cmd,4);

    /* Block write (BLOCK_SIZE bytes) */

    n=read(0,block,BLOCK_SIZE);

    if(n<BLOCK_SIZE)
       for(;n<BLOCK_SIZE;n++)
          block[n]=0xff;

    rs232_write(fd,block,BLOCK_SIZE);

    /* Check status */

    rs232_read(fd,cmd,3);

    if(cmd[0]!='O' || cmd[1]!='K')
      {
       fprintf(stderr,"\nAn error occured.\n");
       return(1);
      }
   }

 /* Finish */

 rs232_close(fd);

 fprintf(stderr,"\rWriting %ld blocks (from %ld to %ld) complete\n",count,start,start+count-1);

 return(0);
}
