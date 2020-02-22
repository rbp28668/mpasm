/***************************************
 $Header: /home/amb/pic/projects/05_rs232-infra-red/RCS/txir.c,v 1.7 2008/05/04 13:53:32 amb Exp $

 Program to transmit remote control data on IR connection.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2006,08 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../piclib/rs232.h"

#define LENGTH 128


int main(int argc, char **argv)
{
 int fd;
 char *port=argv[1];
 unsigned char data[LENGTH];
 int n;
 int state,delta;

 if(argc!=2)
   {
    fprintf(stderr,"Usage: txir <serial-device-name>\n");
    return(1);
   }

 /* Open serial port */

 fd=rs232_open(port,38400,1);

 if(fd<0)
   {
    fprintf(stderr,"Error opening %s\n",port);
    return(1);
   }

 /* Read in the data */

 n=0;

 while(scanf("%d %d",&state,&delta)==2)
   {
    if(n==LENGTH)
      {
       fprintf(stderr,"Error reading data from file; too much.\n");
       return(1);
      }

    while(delta>127)
      {
       data[n++]=(128*!!state)+127;
       delta-=127;

       if(n==LENGTH)
         {
          fprintf(stderr,"Error reading data from file; too much.\n");
          return(1);
         }
      }

    data[n++]=(128*!!state)+(delta&127);
   }

 for(;n<LENGTH;n++)
    data[n]=127;

 /* Send a command to transmit Infra-Red */

 rs232_write(fd,(unsigned char*)"T",1);

 rs232_write(fd,data,LENGTH);

 /* Finish */

 rs232_close(fd);

 return(0);
}

