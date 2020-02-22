/***************************************
 $Header: /home/amb/pic/projects/11_gps-sd-logger/RCS/gps-emulator.c,v 1.5 2008/05/04 13:53:33 amb Exp $

 GPS Emulator to test PIC program
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2007,08 Andrew M. Bishop
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

static char *strings[]={"$HELLO, GPS Emulator here\r\n",
                        "$GPGGA,hhmmss.ss,llll.ll,a,yyyyy.yy,x,x,x,x.x,x.x,M,x.x,M,x.x,xxxx*cc\r\n",
                        "$GPRMC,hhmmss.ss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,xxxx,x.x,a*cc\r\n",
                        "$GPGSV,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x*cc\r\n",
                        "$GPGSA,a,1,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x.x,x.x,x.x*cc\r\n",
                        "$GPGSA,a,2,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x.x,x.x,x.x*cc\r\n",
                        "$GPGSA,a,3,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x.x,x.x,x.x*cc\r\n"};

unsigned long send_string(int fd,char* string);

int main(int argc, char **argv)
{
 int fd;
 char *port=argv[1];
 unsigned char byte;
 int i;

 if(argc!=2)
   {
    fprintf(stderr,"Usage: gps-emulator <serial-device-name>\n");
    return(1);
   }

 /* Open serial port */

 fd=rs232_open(port,9600,0);

 if(fd<0)
   {
    fprintf(stderr,"Error opening %s\n",port);
    return(1);
   }

 /* Send the welcome string */

 printf(">>>SENDING>>>\n");

 send_string(fd,strings[0]);

 /* Read the 3 initialisation strings */

 printf("<<<RECEIVING<<<\n");

 for(i=0;i<3;i++)
    do
      {
       rs232_read(fd,&byte,1);
       putchar(byte);
       fflush(stdout);
      }
    while(byte!='\n');

 /* Write the data strings once a second */

 printf(">>>SENDING>>>\n");

 while(1)
   {
    unsigned long elapsed_time=0;

    elapsed_time+=send_string(fd,strings[1]);
    elapsed_time+=send_string(fd,strings[2]);

    elapsed_time+=send_string(fd,strings[3]);
    if(drand48()<0.5)
       elapsed_time+=send_string(fd,strings[3]);
    if(drand48()<0.5)
       elapsed_time+=send_string(fd,strings[3]);

    if(drand48()<0.2)
       elapsed_time+=send_string(fd,strings[4]);
    else if(drand48()<0.5)
       elapsed_time+=send_string(fd,strings[5]);
    else
       elapsed_time+=send_string(fd,strings[6]);

    usleep(1000000-elapsed_time);
   }

 /* Finish */

 rs232_close(fd);

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Send a string.

  unsigned long send_string Returns the amount of microseconds that it took.

  int fd The file descriptor.

  char* string The string to send.
  ++++++++++++++++++++++++++++++++++++++*/

unsigned long send_string(int fd,char* string)
{
 rs232_write(fd,(unsigned char*)string,strlen(string));

 write(1,(unsigned char*)string,strlen(string));

 return(1000000*10*strlen(string)/9600);
}
