/***************************************
 $Header: /home/amb/pic/projects/04_rs232-to-i2c/RCS/testeeprom.c,v 1.8 2008/05/04 13:53:32 amb Exp $

 Program to test EEPROM communication on I2C.
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
#include "../piclib/i2c.h"

#define EEPROM0_W_ADDR 0xA0
#define EEPROM0_R_ADDR 0xA1

int main(int argc, char **argv)
{
 int fd;
 char *port=argv[1];
 int i;
 unsigned reply;
 unsigned rand;
 unsigned char page_write[64];
 unsigned char page_read[64];

 if(argc!=2)
   {
    fprintf(stderr,"Usage: testeeprom <serial-device-name>\n");
    return(1);
   }

 /* Open serial port */

 fd=rs232_open(port,38400,1);

 if(fd<0)
   {
    fprintf(stderr,"Error opening %s\n",port);
    return(1);
   }

 /* Use I2C library functions */

 srand(time(NULL));

 rand=random()&0xff;

 i2c_command(fd,"S %w %w %w %w P",EEPROM0_W_ADDR, 0, 0, rand);

 printf("Wrote %02x to 0\n",rand);

 i2c_command(fd,"S %w %w %w R %w %r P",EEPROM0_W_ADDR, 0, 0, EEPROM0_R_ADDR, &reply);

 reply=reply&0xff;

 printf("Read %02x from 0\n",reply);

 /* Page write / read (16 bytes) */

 printf("Page write:");

 for(i=0;i<16;i++)
   {
    page_write[i]=random()&0xff;
    printf(" %02x",page_write[i]);
   }

 printf("\n");

 i2c_command(fd,"S %w %w %w %16w P",EEPROM0_W_ADDR, 1, 0, page_write);

 i2c_command(fd,"S %w %w %w R %w %16r P",EEPROM0_W_ADDR, 1, 0, EEPROM0_R_ADDR, &page_read);

 printf("Page read:");

 for(i=0;i<16;i++)
   {
    printf(" %02x",page_read[i]);
   }

 printf("\n");

 rs232_close(fd);

 return(0);
}

