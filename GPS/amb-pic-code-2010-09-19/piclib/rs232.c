/***************************************
 $Header: /home/amb/pic/piclib/RCS/rs232.c,v 1.14 2008/05/31 14:31:12 amb Exp $

 Program file for RS232 functions.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2006,07,08 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <linux/serial.h>

#include <sys/time.h>
#include <time.h>

#include "rs232.h"


static unsigned int flow_control=0;


/*++++++++++++++++++++++++++++++++++++++
  Open an RS232 device.

  int rs232_open Returns the file descriptor.

  char *device The name of the device node to open.

  int speed The speed of the connection.

  int flow Flow control or not.
  ++++++++++++++++++++++++++++++++++++++*/

int rs232_open(char *device,int speed,int flow)
{
 struct termios options;
 struct serial_struct serial;
 int fd,baud;

 /* Select the baud rate */

 switch(speed)
   {
   case 2400:  baud=B2400;  break;
   case 4800:  baud=B4800;  break;
   case 9600:  baud=B9600;  break;
   case 19200: baud=B19200; break;
   case 38400: baud=B38400; break;
   case 57600: baud=B57600; break;
   default:
    fprintf(stderr,"Invalid serial port speed.\n");
    return(-2);
   }

 /* Open the device */

 fd=open(device,O_RDWR|O_NOCTTY|O_NDELAY|O_SYNC);

 if(fd==-1)
   {
    fprintf(stderr,"Failed to open serial port [%s].\n",strerror(errno));
    return(-1);
   }

 /* Set blocking mode */

 fcntl(fd,F_SETFL,0);

 /* Set the low-level things that setserial normally sets */

 if(ioctl(fd,TIOCGSERIAL,&serial)==-1)
   {
    fprintf(stderr,"Failed to get serial port 'setserial' settings [%s].\n",strerror(errno));
    return(-3);
   }

 serial.flags&=~ASYNC_SPD_MASK; /* Clear to get spd_normal for real 38400 speed */

 if(ioctl(fd,TIOCSSERIAL,&serial)==-1)
   {
    fprintf(stderr,"Failed to set serial port 'setserial' settings [%s].\n",strerror(errno));
    return(-4);
   }

 /* Get current options */

 if(tcgetattr(fd,&options)==-1)
   {
    fprintf(stderr,"Failed to get serial port attributes [%s].\n",strerror(errno));
    return(-5);
   }

 /* Set baud rate */

 if(cfsetispeed(&options,baud)==-1 ||
    cfsetospeed(&options,baud)==-1)
   {
    fprintf(stderr,"Failed to set serial port input/output speed [%s].\n",strerror(errno));
    return(-6);
   }

 /* 8 bit no parity, 1 stop bit */

 options.c_cflag&=~PARENB;
 options.c_cflag&=~CSTOPB;
 options.c_cflag&=~CSIZE;
 options.c_cflag|=CS8;

 /* Local mode */

 options.c_cflag|=CLOCAL;

 /* Enable receiver */

 options.c_cflag|=CREAD;

 /* Raw data flow */

 options.c_lflag&=~(ICANON|ECHO|ECHOE|ISIG);

 /* No Hardware flow control */

 options.c_cflag&=~CRTSCTS;

 /* No software flow control */

 options.c_iflag&=~(IXON|IXOFF|IXANY);

 /* No character translation on input */

 options.c_iflag&=~(INLCR|ICRNL|IUCLC|IGNCR);

 /* Raw output */

 options.c_oflag&=~OPOST;

 /* Set new options */

 if(tcsetattr(fd,TCSAFLUSH,&options))
   {
    fprintf(stderr,"Failed to set serial port attributes [%s].\n",strerror(errno));
    return(-7);
   }

 /* Set the flag for flow control */

 flow_control=flow;

 return(fd);
}


/*++++++++++++++++++++++++++++++++++++++
  Write some data to the RS232 port

  size_t rs232_write Returns the number of bytes written or the error status.

  int fd The file descriptor of the serial port.

  unsigned char *data The data to write.

  size_t length The length of the data to write.
  ++++++++++++++++++++++++++++++++++++++*/

size_t rs232_write(int fd, unsigned char *data,size_t length)
{
 if(flow_control)
   {
    int flags;
    size_t i;

    for(i=0;i<length;i++)
      {
       do
         {
          ioctl(fd,TIOCMGET,&flags);
         }
       while(!(flags&TIOCM_CTS));

       write(fd,&data[i],1);
       tcdrain(fd);
      }
   }
 else
    write(fd,data,length);

 return(length);
}


/*++++++++++++++++++++++++++++++++++++++
  Read all of the data from the RS232 port

  size_t rs232_read Returns the number of bytes read or the error status.

  int fd The file descriptor of the serial port.

  unsigned char *data The data array to read into.

  size_t length The length of the data to read.
  ++++++++++++++++++++++++++++++++++++++*/

size_t rs232_read(int fd, unsigned char *data,size_t length)
{
 size_t nread=0,n;

 nread=read(fd,data,length);

 if(nread>=0)
    while(nread<length)
      {
       usleep(1);               /* Give up timeslice */

       n=read(fd,&data[nread],length-nread);

       if(n<0)
          return(n);

       nread+=n;
      }

 return(nread);
}


/*++++++++++++++++++++++++++++++++++++++
  Read available data from the RS232 port

  size_t rs232_read Returns the number of bytes read or the error status.

  int fd The file descriptor of the serial port.

  unsigned char *data The data array to read into.

  size_t length The length of the data to read.
  ++++++++++++++++++++++++++++++++++++++*/

size_t rs232_read_nowait(int fd, unsigned char *data,size_t length)
{
 size_t nread=0;

 nread=read(fd,data,length);

 return(nread);
}


/*++++++++++++++++++++++++++++++++++++++
  Set the state of the RTS line.

  int fd The file descriptor of the serial port.

  int state The state to set the RTS line to.
  ++++++++++++++++++++++++++++++++++++++*/

void rs232_rts(int fd, int state)
{
 int flags;

 ioctl(fd,TIOCMGET,&flags);

 if(state)
    flags|=TIOCM_RTS;
 else
    flags&=~TIOCM_RTS;

 ioctl(fd,TIOCMSET,&flags);
}


/*++++++++++++++++++++++++++++++++++++++
  Read back the status of the CTS line.

  int rs232_cts Returns the state of the CTS line.

  int fd The file descriptor of the serial port.
  ++++++++++++++++++++++++++++++++++++++*/

int rs232_cts(int fd)
{
 int flags,status;

 ioctl(fd,TIOCMGET,&flags);

 status=flags&TIOCM_CTS;

 return(!!status);
}


/*++++++++++++++++++++++++++++++++++++++
  Close the file descriptor

  int fd The file descriptor of the serial port.
  ++++++++++++++++++++++++++++++++++++++*/

void rs232_close(int fd)
{
 close(fd);
}
