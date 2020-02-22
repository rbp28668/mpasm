/***************************************
 $Header: /home/amb/pic/projects/05_rs232-infra-red/RCS/txremote.c,v 1.6 2008/07/18 18:52:53 amb Exp $

 Program to transmit remote control code on IR connection.
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

#define REMOTE_RC5       'R'
#define REMOTE_SIRCS     'S'
#define REMOTE_CABLE     'C'
#define REMOTE_PANASONIC 'P'
#define REMOTE_SAMSUNG   's'
#define REMOTE_NEC       'N'

int main(int argc, char **argv)
{
 int fd;
 char *port=argv[1];
 unsigned char type,data[9];
 unsigned long long longlongdata;
 int n,i;

 if(argc!=5)
   {
   usage:
    fprintf(stderr,"Usage: txremote <serial-device-name> <remote-type> <n-bits> <hex-string>\n");
    fprintf(stderr,"       valid remote control types are:\n");
    fprintf(stderr,"           RC5       (e.g. Hauppauge)\n");
    fprintf(stderr,"           SIRCS     (i.e. Sony)\n");
    fprintf(stderr,"           Cable     (i.e. Cable box)\n");
    fprintf(stderr,"           Panasonic (TV)\n");
    fprintf(stderr,"           Samsung\n");
    fprintf(stderr,"           NEC\n");
    return(1);
   }

      if(!strcmp(argv[2],"RC5"))       type=REMOTE_RC5;
 else if(!strcmp(argv[2],"SIRCS"))     type=REMOTE_SIRCS;
 else if(!strcmp(argv[2],"Cable"))     type=REMOTE_CABLE;
 else if(!strcmp(argv[2],"Panasonic")) type=REMOTE_PANASONIC;
 else if(!strcmp(argv[2],"Samsung"))   type=REMOTE_SAMSUNG;
 else if(!strcmp(argv[2],"NEC"))       type=REMOTE_NEC;
 else goto usage;

 /* Open serial port */

 fd=rs232_open(port,38400,1);

 if(fd<0)
   {
    fprintf(stderr,"Error opening %s\n",port);
    return(1);
   }

 /* Send a command to transmit Infra-Red */

 rs232_write(fd,(unsigned char*)"t",1);
 rs232_write(fd,&type,1);

 /* Write the data */

 data[0]=atoi(argv[3]);
 sscanf(argv[4],"%llx",&longlongdata);

 for(i=1;i<=8;i++)
   {
    data[9-i]=longlongdata&0xff;
    longlongdata>>=8;
   }

 n=rs232_write(fd,data,9);

 /* Finish */

 rs232_close(fd);

 return(0);
}

