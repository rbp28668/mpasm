/***************************************
 $Header: /home/amb/pic/piclib/RCS/spi.h,v 1.4 2007/09/29 15:07:48 amb Exp $

 Header file for SPI functions.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2007 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#ifndef SPI_H
#define SPI_H    /*+ To stop multiple inclusions. +*/

#include <sys/types.h>

/*+ A variable to enable verbose reporting. +*/
extern int spi_verbose;

/* Functions */

void spi_speed(int fd,int speed);

void spi_start(int fd);
void spi_stop(int fd);

void spi_enable(int fd);
void spi_disable(int fd);

int spi_write(int fd,unsigned char *data,size_t length);
int spi_read(int fd,unsigned char *data,size_t length);
int spi_xchange(int fd,unsigned char *data,size_t length);

#endif /* SPI_H */
