/***************************************
 $Header: /home/amb/pic/piclib/RCS/i2c.h,v 1.3 2007/09/11 18:29:29 amb Exp $

 Header file for I2C (via RS232) functions.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2006,07 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#ifndef I2C_H
#define I2C_H    /*+ To stop multiple inclusions. +*/

int i2c_read(int fd,int bus_address,int address_nbytes,int data_address,int data_nbytes,unsigned char *data);

#define i2c_read1(fd,bus_address,data_address,data_nbytes,data) i2c_read(fd,bus_address,1,data_address,data_nbytes,data)
#define i2c_read2(fd,bus_address,data_address,data_nbytes,data) i2c_read(fd,bus_address,2,data_address,data_nbytes,data)
#define i2c_read4(fd,bus_address,data_address,data_nbytes,data) i2c_read(fd,bus_address,4,data_address,data_nbytes,data)

int i2c_write(int fd,int bus_address,int address_nbytes,int data_address,int data_nbytes,unsigned char *data);

#define i2c_write1(fd,bus_address,data_address,data_nbytes,data) i2c_write(fd,bus_address,1,data_address,data_nbytes,data)
#define i2c_write2(fd,bus_address,data_address,data_nbytes,data) i2c_write(fd,bus_address,2,data_address,data_nbytes,data)
#define i2c_write4(fd,bus_address,data_address,data_nbytes,data) i2c_write(fd,bus_address,4,data_address,data_nbytes,data)

int i2c_command(int fd,char *format, ...);

#endif /* I2C_H */
