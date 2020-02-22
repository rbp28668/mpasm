/***************************************
 $Header: /home/amb/pic/projects/04_rs232-to-i2c/RCS/testds1307.c,v 1.2 2008/05/04 13:53:32 amb Exp $

 Program to test DS1307 communication on I2C.
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
#include "../piclib/i2c.h"

#define DS1307_W_ADDR 0xD0
#define DS1307_R_ADDR 0xD1

char *registername[8]={"Seconds","Minutes","Hours","Day","Date","Month","Year","Control"};
char *dayofweek[8]={"???","Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

int main(int argc, char **argv)
{
 int fd;
 char *port=argv[1];
 int i;
 unsigned char ram[64];

 if(argc<3)
   {
    fprintf(stderr,"Usage: testds1307 <serial-device-name> [-read|-write|-clock|-setclock [time-in-secs]]\n");
    return(1);
   }

 /* Open serial port */

 fd=rs232_open(port,38400,1);

 if(fd<0)
   {
    fprintf(stderr,"Error opening %s\n",port);
    return(1);
   }

 if(!strcmp(argv[2],"-read"))
   {
    /* Use I2C library functions to read all RAM contents */

    i2c_command(fd,"S %w %w R %w %32r P",DS1307_W_ADDR, 0, DS1307_R_ADDR, &ram[0]);

    i2c_command(fd,"S %w %w R %w %32r P",DS1307_W_ADDR,32, DS1307_R_ADDR, &ram[32]);

    for(i=0;i<8;i++)
       printf("Read %02x from address %2d %s\n",ram[i],i,registername[i]);

    for(i=8;i<64;i++)
       printf("Read %02x from address %2d\n",ram[i],i);
   }
 else if(!strcmp(argv[2],"-write"))
   {
    struct tm *tm;
    time_t now;

    /* Use I2C library functions to set the current time */

    now=time(NULL)+1;

    tm=gmtime(&now);

    ram[0]=((tm->tm_sec/10)<<4) + tm->tm_sec%10;
    ram[1]=((tm->tm_min/10)<<4) + tm->tm_min%10;
    ram[2]=((tm->tm_hour/10)<<4)+ tm->tm_hour%10; /* Set to 24 hour mode */
    ram[3]=tm->tm_wday + 1;
    ram[4]=((tm->tm_mday/10)<<4)+ tm->tm_mday%10;
    ram[5]=((tm->tm_mon/10)<<4) + tm->tm_mon%10 + 1;
    ram[6]=(((tm->tm_year-100)/10)<<4) + (tm->tm_year-100)%10;
    ram[7]=0x10;                   /* Enable pulse output at 1 Hz */

    srand(now);

    for(i=8;i<64;i++)
       ram[i]=random()&0xff;

    for(i=0;i<8;i++)
       printf("Write %02x to address %2d %s\n",ram[i],i,registername[i]);

    for(i=8;i<64;i++)
       printf("Write %02x to address %2d [random data]\n",ram[i],i);

    i2c_command(fd,"S %w %w %32w P",DS1307_W_ADDR, 0, &ram[0]);

    i2c_command(fd,"S %w %w %32w P",DS1307_W_ADDR,32, &ram[32]);
   }
 else if(!strcmp(argv[2],"-clock"))
   {
    while(1)
      {
       /* Use I2C library functions to read date / time information */

       i2c_command(fd,"S %w %w R %w %7r P",DS1307_W_ADDR, 0, DS1307_R_ADDR, &ram[0]);

       printf("20%02d-%02d-%02d %02d:%02d:%02d (%s)\n",
              ((ram[6]&0xf0)>>4)*10+(ram[6]&0x0f),
              ((ram[5]&0xf0)>>4)*10+(ram[5]&0x0f),
              ((ram[4]&0xf0)>>4)*10+(ram[4]&0x0f),
              ((ram[2]&0xf0)>>4)*10+(ram[2]&0x0f),
              ((ram[1]&0xf0)>>4)*10+(ram[1]&0x0f),
              ((ram[0]&0xf0)>>4)*10+(ram[0]&0x0f),
              dayofweek[ram[3]]);

       sleep(1);
      }
   }
 else if(!strcmp(argv[2],"-setclock"))
   {
    struct tm *tm;
    time_t then;

    if(argc==3)
       then=time(NULL);
    else
       then=atol(argv[3]);

    /* Use I2C library functions to write date / time information */

    tm=gmtime(&then);

    ram[0]=((tm->tm_sec/10)<<4) + tm->tm_sec%10;
    ram[1]=((tm->tm_min/10)<<4) + tm->tm_min%10;
    ram[2]=((tm->tm_hour/10)<<4)+ tm->tm_hour%10; /* Set to 24 hour mode */
    ram[3]=tm->tm_wday + 1;
    ram[4]=((tm->tm_mday/10)<<4)+ tm->tm_mday%10;
    ram[5]=((tm->tm_mon/10)<<4) + tm->tm_mon%10 + 1;
    ram[6]=(((tm->tm_year-100)/10)<<4) + (tm->tm_year-100)%10;
    ram[7]=0x10;                   /* Enable pulse output at 1 Hz */

    printf("20%02d-%02d-%02d %02d:%02d:%02d (%s)\n",
           ((ram[6]&0xf0)>>4)*10+(ram[6]&0x0f),
           ((ram[5]&0xf0)>>4)*10+(ram[5]&0x0f),
           ((ram[4]&0xf0)>>4)*10+(ram[4]&0x0f),
           ((ram[2]&0xf0)>>4)*10+(ram[2]&0x0f),
           ((ram[1]&0xf0)>>4)*10+(ram[1]&0x0f),
           ((ram[0]&0xf0)>>4)*10+(ram[0]&0x0f),
           dayofweek[ram[3]]);

    i2c_command(fd,"S %w %w %8w P",DS1307_W_ADDR, 0, &ram);
   }
 else
   {
    fprintf(stderr,"Usage: testds1307 <serial-device-name> [-read|-write|-clock|-setclock [time-in-secs]]\n");
    return(1);
   }

 rs232_close(fd);

 return(0);
}

