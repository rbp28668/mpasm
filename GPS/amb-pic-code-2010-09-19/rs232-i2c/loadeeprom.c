/***************************************
 $Header: /home/amb/pic/projects/04_rs232-to-i2c/RCS/loadeeprom.c,v 1.6 2008/05/04 13:53:32 amb Exp $

 Program to load EEPROM on I2C connection.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2006,08 Andrew M. Bishop
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

#define PAGE_SIZE 32

int main(int argc, char **argv)
{
 int fd;
 char *port=argv[1];
 unsigned long size,address;
 unsigned char cmd[3];
 unsigned char page[PAGE_SIZE];

 if(argc!=3)
   {
    fprintf(stderr,"Usage: loadeeprom <serial-device-name> <eeprom-size>\n");
    return(1);
   }

 size=atoi(argv[2]);
 if(argv[2][strlen(argv[2])-1]=='k' || argv[2][strlen(argv[2])-1]=='K')
    size*=1024;
 if(argv[2][strlen(argv[2])-1]=='m' || argv[2][strlen(argv[2])-1]=='M')
    size*=1024;

 /* Open serial port */

 fd=rs232_open(port,38400,1);

 if(fd<0)
   {
    fprintf(stderr,"Error opening %s\n",port);
    return(1);
   }

 /* Initialise the process */

 cmd[0]='L';
 cmd[1]=size>>8;
 cmd[2]=size&0xe0;              /* Multiples of 32 only */
 rs232_write(fd,cmd,3);

 /* Page write (PAGE_SIZE bytes) */

 for(address=0;address<size;address+=PAGE_SIZE)
   {
    int n=read(0,page,PAGE_SIZE);

    if(n<PAGE_SIZE)
       for(;n<PAGE_SIZE;n++)
          page[n]=0xff;

    fprintf(stderr,"\rWriting: %6ld",address);

    rs232_write(fd,page,PAGE_SIZE);
   }

 /* Finish */

 rs232_read(fd,cmd,3);

 rs232_close(fd);

 fprintf(stderr,"\rWriting: %6ld Bytes complete\n",size);

 if(cmd[0]!='O' || cmd[1]!='K')
   {
    fprintf(stderr,"An error occured.\n");
    return(1);
   }

 return(0);
}

