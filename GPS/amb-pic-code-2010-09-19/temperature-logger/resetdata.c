/***************************************
 $Header: /home/amb/pic/projects/08_temperature-logger/RCS/resetdata.c,v 1.10 2008/05/04 13:53:33 amb Exp $

 Program to reset temperature logger using I2C.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2006,08 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

#include "../piclib/rs232.h"

#define PAGE_SIZE      32
#define EEPROM_SIZE 32768

int main(int argc, char **argv)
{
 int fd;
 char *port=argv[1];
 unsigned long address;
 unsigned char cmd[3];
 unsigned char page[PAGE_SIZE],page2[PAGE_SIZE];
 time_t starttime,period,stoptime;
 struct tm *starttm,*stoptm;
 char startstr[20],stopstr[20];
 int i,mute=0;
 unsigned long eeprom_size=EEPROM_SIZE;

 if(argc!=3)
   {
    int i,argsused=0;

    for(i=1;i<argc-1;i++)
       if(!strcmp(argv[i],"-size"))
         {
          eeprom_size=atol(argv[++i])*1024;
          argsused+=2;
         }

    argc-=argsused;

    if(argc!=3)
      {
       fprintf(stderr,"Usage: resetdata <serial-device-name> <sample-period> [-size size]\n");
       return(1);
      }
   }

 period=atoi(argv[2]);

 /* Open serial port */

 fd=rs232_open(port,38400,1);

 if(fd<0)
   {
    fprintf(stderr,"Error opening %s\n",port);
    return(1);
   }

 /* Initialise the process to erase the EEPROM */

 cmd[0]='E';
 cmd[1]=eeprom_size>>8;
 cmd[2]=eeprom_size%256;
 rs232_write(fd,cmd,3);

 printf("\rErasing EEPROM");
 fflush(stdout);

 rs232_read(fd,cmd,3);

 printf("\rErasing EEPROM ... done\n");

 /* Initialise the process to write the EEPROM */

 cmd[0]='L';
 cmd[1]=0;
 cmd[2]=32;
 rs232_write(fd,cmd,3);

 /* Page write (PAGE_SIZE bytes) */

 starttime=60*(time_t)((time(NULL)+120)/60);
 starttm=localtime(&starttime);
 strftime(startstr,sizeof(startstr),"%Y-%m-%d %H:%M:%S",starttm);

 stoptime =starttime+(eeprom_size-PAGE_SIZE)*period;
 stoptm =localtime(&stoptime);
 strftime(stopstr ,sizeof(stopstr) ,"%Y-%m-%d %H:%M:%S",stoptm);

 printf("Start time        : %s\n",startstr);
 printf("Stop time         : %s\n",stopstr);
 printf("Measurement period: %ld seconds\n",period);

 sprintf((char*)page,"%-31s",startstr);

 page[24]=(starttime>>24)&0xff;
 page[25]=(starttime>>16)&0xff;
 page[26]=(starttime>> 8)&0xff;
 page[27]=(starttime    )&0xff;

 page[30]=period;

 rs232_write(fd,page,PAGE_SIZE);

 rs232_read(fd,cmd,3);

 /* Initialise the read process */

 printf("\rVerifying EEPROM");
 fflush(stdout);

 cmd[0]='D';
 cmd[1]=eeprom_size>>8;
 cmd[2]=eeprom_size%256;
 rs232_write(fd,cmd,3);

 /* First page read (PAGE_SIZE bytes) */

 rs232_read(fd,page2,PAGE_SIZE);

 for(i=0;i<PAGE_SIZE;i++)
    if(page2[i]!=page[i])
      {
       fprintf(stderr,"\n*** Verify failed at byte %d. ***\n\n",i);
       mute=1;
       break;
      }

 /* Data pages read (PAGE_SIZE bytes) */

 for(address=PAGE_SIZE;address<eeprom_size;address+=PAGE_SIZE)
   {
    rs232_read(fd,page2,PAGE_SIZE);

    if(!mute)
      {
       for(i=0;i<PAGE_SIZE;i++)
          if(page2[i]!=0xff)
            {
             fprintf(stderr,"\n*** Verify failed at byte %d. ***\n\n",32+i);
             mute=1;
             break;
            }
      }
   }

 rs232_read(fd,cmd,3);

 printf("\rVerifying EEPROM ... done\n");

 /* Finish */

 rs232_close(fd);

 return(0);
}

