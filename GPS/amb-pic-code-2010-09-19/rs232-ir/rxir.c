/***************************************
 $Header: /home/amb/pic/projects/05_rs232-infra-red/RCS/rxir.c,v 1.7 2008/05/04 13:53:32 amb Exp $

 Program to receive remote control data on IR connection.
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
 int state,delta,laststate,time;
 int i,n;

 if(argc!=2)
   {
    fprintf(stderr,"Usage: rxir <serial-device-name>\n");
    return(1);
   }

 /* Open serial port */

 fd=rs232_open(port,38400,1);

 if(fd<0)
   {
    fprintf(stderr,"Error opening %s\n",port);
    return(1);
   }

 /* Send a command to receive Infra-Red */

 rs232_write(fd,(unsigned char*)"R",1);

 /* Read back the data */

 n=rs232_read(fd,data,LENGTH);

 if(n!=LENGTH)
   {
    fprintf(stderr,"Error reading data from PIC; not enough.\n");
    return(1);
   }

 /* Print it out in simple format */

 laststate=-1;
 time=0;

 for(i=0;i<LENGTH;i++)
   {
    state=!!(data[i]&128);
    delta=data[i]&127;

    if(state==laststate)
       time+=delta;
    else
      {
       if(time>0)
          printf("%d %4d\n",laststate,time);
       time=delta;
       laststate=state;
      }
   }

 printf("%d %4d\n",laststate,time);

 /* Finish */

 rs232_close(fd);

 return(0);
}

