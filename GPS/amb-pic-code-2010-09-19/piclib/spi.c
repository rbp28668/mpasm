/***************************************
 $Header: /home/amb/pic/piclib/RCS/spi.c,v 1.5 2007/09/29 15:07:47 amb Exp $

 Header file for SPI functions.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2007 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "rs232.h"
#include "spi.h"


/*+ A variable to enable verbose reporting. +*/
int spi_verbose=0;


/*++++++++++++++++++++++++++++++++++++++
  Set the SPI communication speed.

  int fd The file descriptor of the RS232.

  int speed The speed to use, 0 => clk/4, 1 => clk/16, 2 => clk/64.
  ++++++++++++++++++++++++++++++++++++++*/

void spi_speed(int fd,int speed)
{
 unsigned char cmd[2];

 cmd[0]='C';
 cmd[1]=speed&0xff;

 rs232_write(fd,cmd,2);

 if(spi_verbose)
    printf("SPI: Set speed to %d\n",speed);
}


/*++++++++++++++++++++++++++++++++++++++
  Start SPI communications over RS232.

  int fd The file descriptor of the RS232.
  ++++++++++++++++++++++++++++++++++++++*/

void spi_start(int fd)
{
 rs232_write(fd,(unsigned char*)"S",1);
}


/*++++++++++++++++++++++++++++++++++++++
  Stop SPI communications over RS232.

  int fd The file descriptor of the RS232.
  ++++++++++++++++++++++++++++++++++++++*/

void spi_stop(int fd)
{
 rs232_write(fd,(unsigned char*)"S",1);
}


/*++++++++++++++++++++++++++++++++++++++
  Enable the SPI chip select line.

  int fd The file descriptor of the RS232.
  ++++++++++++++++++++++++++++++++++++++*/

void spi_enable(int fd)
{
 if(spi_verbose)
    printf("SPI: Chip select enabled\n");

 rs232_write(fd,(unsigned char*)"E",1);
}


/*++++++++++++++++++++++++++++++++++++++
  Disable the SPI chip select line.

  int fd The file descriptor of the RS232.
  ++++++++++++++++++++++++++++++++++++++*/

void spi_disable(int fd)
{
 if(spi_verbose)
    printf("SPI: Chip select disabled\n");

 rs232_write(fd,(unsigned char*)"D",1);
}


/*++++++++++++++++++++++++++++++++++++++
  Write some bytes over RS232 to the SPI.

  int spi_write Returns zero if OK or non-zero for an error.

  int fd The file descriptor of the RS232.

  unsigned char *data The data to be written.

  size_t length The number of bytes to be written.
  ++++++++++++++++++++++++++++++++++++++*/

int spi_write(int fd,unsigned char *data,size_t length)
{
 size_t n;
 unsigned char cmd[2];
 int res;

 if(length<1 || length>256)
   {
    fprintf(stderr,"SPI: Error invalid length %d",length);
    return(-1);
   }

 if(spi_verbose)
   {
    printf("SPI: Writing bytes");
    for(n=0;n<length;n++)
       printf(" %02x",data[n]);
    printf("\n");
   }

 cmd[0]='W';
 cmd[1]=length&0xff;

 if((res=rs232_write(fd,cmd,2))<0)
    return(res);

 if((res=rs232_write(fd,data,length))<0)
    return(res);

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Read some bytes from the SPI bus over RS232.

  int spi_read Returns zero if OK or non-zero for an error.

  int fd The file descriptor of the RS232.

  unsigned char *data Returns the bytes that were read.

  size_t length The number of bytes to read.
  ++++++++++++++++++++++++++++++++++++++*/

int spi_read(int fd,unsigned char *data,size_t length)
{
 size_t n;
 unsigned char cmd[2];
 int res;

 if(length<1 || length>256)
   {
    fprintf(stderr,"SPI: Error invalid length %d",length);
    return(-1);
   }

 cmd[0]='R';
 cmd[1]=length&0xff;

 if((res=rs232_write(fd,cmd,2))<0)
    return(res);

 if((res=rs232_read(fd,data,length))<0)
    return(res);

 if(spi_verbose)
   {
    printf("SPI: Reading bytes");
    for(n=0;n<length;n++)
       printf(" %02x",data[n]);
    printf("\n");
   }

 return(0);
}


/*++++++++++++++++++++++++++++++++++++++
  Exchange some bytes with the SPI bus.

  int spi_xchange Returns zero if OK or non-zero for an error.

  int fd The file descriptor of the RS232.

  unsigned char *data The bytes to write and returns the reply.

  size_t length The number of bytes to exchange.
  ++++++++++++++++++++++++++++++++++++++*/

int spi_xchange(int fd,unsigned char *data,size_t length)
{
 size_t n;
 unsigned char cmd[2];
 int res;

 if(length<1 || length>256)
   {
    fprintf(stderr,"SPI: Error invalid length %d",length);
    return(-1);
   }

 if(spi_verbose)
   {
    printf("SPI: Exchanging bytes (W)");
    for(n=0;n<length;n++)
       printf(" %02x",data[n]);
    printf("\n");
   }

 cmd[0]='X';
 cmd[1]=length&0xff;

 if((res=rs232_write(fd,cmd,2))<0)
    return(res);

 if((res=rs232_write(fd,data,length))<0)
    return(res);

 if((res=rs232_read(fd,data,length))<0)
    return(res);

 if(spi_verbose)
   {
    printf("SPI: Exchanging bytes (R)");
    for(n=0;n<length;n++)
       printf(" %02x",data[n]);
    printf("\n");
   }

 return(0);
}
