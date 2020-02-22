/***************************************
 $Header: /home/amb/pic/projects/06_cable-box-changer/RCS/send.c,v 1.5 2008/05/04 13:53:33 amb Exp $

 Program to send a command to cable box IR changer.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2006,07,08 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "../piclib/rs232.h"


int main(int argc, char **argv)
{
 int fd,i;
 char *port=argv[1];

 if(argc!=3)
   {
    fprintf(stderr,"Usage: send <device-name> <string>\n");
    return(1);
   }

 /* Open serial port */

 fd=rs232_open(port,38400,1);

 if(fd<0)
   {
    fprintf(stderr,"Error opening %s\n",port);
    return(1);
   }

 /* Send a command to transmit Infra-Red */

 for(i=0;argv[2][i];i++)
   {
    rs232_write(fd,(unsigned char*)&argv[2][i],1);

    sleep(1);
   }

 /* Finish */

 rs232_close(fd);

 return(0);
}

