/***************************************
 $Header: /home/amb/pic/projects/11_gps-sd-logger/RCS/gps-tester.c,v 1.5 2008/05/04 13:53:33 amb Exp $

 GPS RS232 interface tester
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

static char *strings[]={"$PNMRX107,ALL,0*xx\r\n",
                        "$PNMRX108,GGA,RMC,GSV,GSA*xx\r\n",
                        "$PNMRX103,GGA,1,RMC,1,GSV,1,GSA,1*xx\r\n"};

int main(int argc, char **argv)
{
 int fd;
 char *port=argv[1];
 unsigned char byte,checksum;
 int i,j;
 
 if(argc!=2)
   {
    fprintf(stderr,"Usage: gps-tester <serial-device-name>\n");
    return(1);
   }

 /* Open serial port */

 fd=rs232_open(port,9600,0);

 if(fd<0)
   {
    fprintf(stderr,"Error opening %s\n",port);
    return(1);
   }

 /* Read the welcome string */

 printf("<<<RECEIVING<<<\n");

 do
   {
    rs232_read(fd,&byte,1);
    putchar(byte);
   }
 while(byte!='\n');

 /* Fix up the strings and send them */

 printf(">>>SENDING>>>\n");

 for(j=0;j<sizeof(strings)/sizeof(strings[0]);j++)
   {
    char string[80];
    checksum=0;

    for(i=0;i<strlen(strings[j]);i++)
      {
       string[i]=strings[j][i];

       if(strings[j][i]=='$') continue;
       if(strings[j][i]=='*')
         {
          char hex[3];
          sprintf(hex,"%02x",checksum);
          string[++i]=hex[0];
          string[++i]=hex[1];
         }
       checksum^=strings[j][i];
      }

    string[strlen(strings[j])]=0;

    printf("Sending: %s",string);
    usleep(100);

    rs232_write(fd,(unsigned char*)string,strlen(strings[j]));
   }

 /* Display data back */

 while(1)
   {
    rs232_read(fd,&byte,1);
    putchar(byte);
   }

 /* Finish */

 rs232_close(fd);

 return(0);
}

