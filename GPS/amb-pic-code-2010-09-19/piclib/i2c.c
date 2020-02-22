/***************************************
 $Header: /home/amb/pic/piclib/RCS/i2c.c,v 1.8 2007/09/11 18:29:28 amb Exp $

 Program file for I2C (via RS232) functions.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2006,07 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include "rs232.h"
#include "i2c.h"


#define DEBUG 0
#define WARN  1


/*++++++++++++++++++++++++++++++++++++++
  Send a standard read command on the I2C bus.

  int i2c_read Returns success (zero) or failure.

  int fd The file descriptor to use for the rs232.

  int bus_address The address on the I2C bus.

  int address_nbytes The number of bytes to send for the address part.

  int data_address The internal address.

  int data_nbytes The number of bytes to read.

  unsigned char *data The location to store the data read back.
  ++++++++++++++++++++++++++++++++++++++*/

int i2c_read(int fd,int bus_address,int address_nbytes,int data_address,int data_nbytes,unsigned char *data)
{
 char format[24];
 unsigned char address_data[4];
 int i;
 int write_address,read_address;

 if(address_nbytes<1 || address_nbytes>4)
    return(1);

 for(i=address_nbytes-1;i>=0;i--)
    address_data[address_nbytes-1-i]=(data_address>>i)&0x0f;

 write_address=bus_address&0xfe;
 read_address=write_address|0x01;

 sprintf(format,"S %%w %%%dw R %%w %%%dr P",address_nbytes,data_nbytes);

 return(i2c_command(fd,format,write_address,address_data,read_address,data));
}


/*++++++++++++++++++++++++++++++++++++++
  Send a standard write command on the I2C bus.

  int i2c_write Returns success (zero) or failure.

  int fd The file descriptor to use for the rs232.

  int bus_address The address on the I2C bus.

  int address_nbytes The number of bytes to send for the address part.

  int data_address The internal address.

  int data_nbytes The number of bytes to write.

  unsigned char *data The data to write.
  ++++++++++++++++++++++++++++++++++++++*/

int i2c_write(int fd,int bus_address,int address_nbytes,int data_address,int data_nbytes,unsigned char *data)
{
 char format[16];
 unsigned char address_data[4];
 int i;
 int write_address;

 if(address_nbytes<1 || address_nbytes>4)
    return(1);

 for(i=address_nbytes-1;i>=0;i--)
    address_data[address_nbytes-1-i]=(data_address>>i)&0x0f;

 write_address=bus_address&0xfe;

 sprintf(format,"S %%w %%%dw %%%dw P",address_nbytes,data_nbytes);

 return(i2c_command(fd,format,write_address,address_data,data));
}


/*++++++++++++++++++++++++++++++++++++++
  Send a command on the I2C bus

  int i2c_command Returns success (zero) or failure.

  int fd The file descriptor to use for the rs232.

  char *format The format of the command to send.

  ... The parameters for the command.
  ++++++++++++++++++++++++++++++++++++++*/

int i2c_command(int fd,char *format, ...)
{
 va_list ap;
 int length=0;
 unsigned char *string;
 int i,j,lastchar,lastcount;

 /* Calculate the length of the string. */

#ifdef __STDC__
 va_start(ap,format);
#else
 va_start(ap);
#endif

 for(i=0;format[i];i++)
    if(format[i]=='%')
      {
       int len;

       i++;

       if(isdigit(format[i]))
          len=atoi(&format[i]);
       else
          len=1;

       while(isdigit(format[i]))
          i++;

       if(format[i]=='w')
          length+=len+2;
       else if(format[i]=='r')
          length+=len+2;
       else
         {
          fprintf(stderr,"Invalid i2c command format '%s' (Not 'r' or 'w' after '%%')\n",format);
          exit(1);
         }
      }
    else if(format[i]=='S')
       length++;
    else if(format[i]=='R')
       length++;
    else if(format[i]=='P')
       length++;
    else if(format[i]==' ')
       ;
    else
      {
       fprintf(stderr,"Invalid i2c command format '%s' (character other than 'S', 'R' or 'P' in string)\n",format);
       exit(1);
      }

 string=(unsigned char*)malloc(length+1);

 /* Convert the format and arguments into a command */

#ifdef __STDC__
 va_start(ap,format);
#else
 va_start(ap);
#endif

 j=0;
 lastchar='S';
 lastcount=0;

 for(i=0;format[i];i++)
    if(format[i]=='%')
      {
       int pointer,len,l;

       i++;
       if(isdigit(format[i]))
         {pointer=1;len=atoi(&format[i]);}
       else
         {pointer=0;len=1;}

       while(isdigit(format[i]))
          i++;

       if(format[i]=='w')
         {
          if(lastchar!='w')
            {
             string[j++]='w';
             lastcount=j;
             string[j++]=len;
             lastchar='w';
            }
          else
             string[lastcount]+=len;

          if(!pointer)
            {
             int n=va_arg(ap,int);
             string[j++]=n;
            }
          else
            {
             unsigned char *str=va_arg(ap,unsigned char*);
             for(l=0;l<len;l++)
                string[j++]=str[l];
            }
         }

       if(format[i]=='r')
         {
          if(lastchar!='r')
            {
             string[j++]='r';
             lastcount=j;
             string[j++]=len;
             lastchar='r';
            }
          else
             string[lastcount]+=len;

          for(l=0;l<len;l++)
             string[j++]=0;
         }
      }
    else if(format[i]=='S')
      {string[j++]='S'; lastchar='S';}
    else if(format[i]=='R')
      {string[j++]='R'; lastchar='R';}
    else if(format[i]=='P')
      {string[j++]='P'; lastchar='P';}

 /* Write the string and get the reply */

#if DEBUG
 for(i=0;i<j;i++)
    printf(">> %2d : %02x '%c'\n",i,string[i],string[i]);
 fflush(stdout);
#endif

 rs232_write(fd,string,j);

 rs232_read(fd,string,3);
 if(string[0]=='E')
   {
    int retval=string[1]-'0';

    free(string);

#if WARN
    fprintf(stderr,"I2C command '%s' returned %d\n",format,retval);
#endif

    return(retval);
   }

 rs232_read(fd,string,j);

#if DEBUG
 for(i=0;i<j;i++)
    printf("<< %2d : %02x '%c'\n",i,string[i],string[i]);
 fflush(stdout);
#endif

 /* Parse the format again to extract the reply data */

#ifdef __STDC__
 va_start(ap,format);
#else
 va_start(ap);
#endif

 j=0;
 lastchar='S';
 lastcount=0;

 for(i=0;format[i];i++)
    if(format[i]=='%')
      {
       int pointer,len,l;

       i++;
       if(isdigit(format[i]))
         {pointer=1;len=atoi(&format[i]);}
       else
         {pointer=0;len=1;}

       while(isdigit(format[i]))
          i++;

       if(format[i]=='w')
         {
          if(lastchar!='w')
            {
             j++;
             lastcount=j;
             j++;
             lastchar='w';
            }

          (void)va_arg(ap,void*);

          j+=len;
         }

       if(format[i]=='r')
         {
          if(lastchar!='r')
            {
             j++;
             lastcount=j;
             j++;
             lastchar='r';
            }

          if(!pointer)
            {
             unsigned int *intp=va_arg(ap,unsigned int*);
             *intp=string[j++];
            }
          else
            {
             unsigned char *str=va_arg(ap,unsigned char*);
             for(l=0;l<len;l++)
                str[l]=string[j++];
            }
         }
      }
    else if(format[i]=='S')
      {j++; lastchar='S';}
    else if(format[i]=='R')
      {j++; lastchar='R';}
    else if(format[i]=='P')
      {j++; lastchar='P';}

 /* Finish */

 rs232_read(fd,string,1);

 free(string);

 va_end(ap);

 return(0);
}
