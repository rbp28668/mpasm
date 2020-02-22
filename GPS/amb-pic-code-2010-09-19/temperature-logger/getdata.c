/***************************************
 $Header: /home/amb/pic/projects/08_temperature-logger/RCS/getdata.c,v 1.13 2008/05/04 13:53:33 amb Exp $

 Program to get data from temperature logger using I2C.
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

#define PAGE_SIZE         32
#define EEPROM_SIZE    32768

#define RESISTOR      100000.0

#define THERMISTOR_T0    273.0
#define THERMISTOR_T25   298.0
#define THERMISTOR_R25 50000.0
#define THERMISTOR_B    4288.0
#define THERMISTOR_DIS  0.0006

#define DEFAULT_VOLTAGE 5.0

int main(int argc, char **argv)
{
 int fd;
 char *port=argv[1];
 unsigned long address,lastgood=0;
 unsigned char cmd[3];
 unsigned char page[PAGE_SIZE],prevbyte;
 int prevsampleval;
 time_t starttime,period,sampletime;
 double supply_voltage=DEFAULT_VOLTAGE;
 unsigned long eeprom_size=EEPROM_SIZE;

 if(argc!=2)
   {
    int i,argsused=0;

    for(i=1;i<argc-1;i++)
       if(!strcmp(argv[i],"-v"))
         {
          supply_voltage=atof(argv[++i]);
          argsused+=2;
         }
       else if(!strcmp(argv[i],"-size"))
         {
          eeprom_size=atol(argv[++i])*1024;
          argsused+=2;
         }

    argc-=argsused;

    if(argc!=2)
      {
       fprintf(stderr,"Usage: getdata <serial-device-name> [-v voltage] [-size size]\n");
       return(1);
      }
   }

 /* Open serial port */

 fd=rs232_open(port,38400,1);

 if(fd<0)
   {
    fprintf(stderr,"Error opening %s\n",port);
    return(1);
   }

 /* Initialise the read process */

 cmd[0]='D';
 cmd[1]=eeprom_size>>8;
 cmd[2]=eeprom_size%256;
 rs232_write(fd,cmd,3);

 /* First page read (PAGE_SIZE bytes) */

 rs232_read(fd,page,PAGE_SIZE);

 starttime=((((time_t)page[24])&0xff)<<24)|
           ((((time_t)page[25])&0xff)<<16)|
           ((((time_t)page[26])&0xff)<< 8)|
           ((((time_t)page[27])&0xff)    );

 period=page[30];

 /* Print the header */

 printf("########################################\n");
 printf("#\n");
 printf("# Temperature log graph data\n");
 printf("# Column 1: date\n");
 printf("# Column 2: time\n");
 printf("# Column 3: raw value\n");
 printf("# Column 4: temperature\n");
 printf("# \n");
 printf("# (+)---\\/\\/\\/---o---\\/\\/\\/---(-)\n");
 printf("#     Thermistor |  Resistor\n");
 printf("# \n");
 printf("# Voltage         : %.1f V\n",supply_voltage);
 printf("# Resistor        : %.0f Ohms\n",RESISTOR);
 printf("# Thermistor T(0) : %.0f K\n",THERMISTOR_T0);
 printf("# Thermistor T(25): %.0f K\n",THERMISTOR_T25);
 printf("# Thermistor R(25): %.0f Ohms\n",THERMISTOR_R25);
 printf("# Thermistor B    : %.0f\n",THERMISTOR_B);
 printf("# Thermistor heat : %f W/K\n",THERMISTOR_DIS);
 printf("# \n");
 printf("########################################\n");
 printf("\n");

 /* Data pages read (PAGE_SIZE bytes) */

 prevbyte=0xff;
 prevsampleval=0;
 sampletime=starttime;

 for(address=PAGE_SIZE;address<eeprom_size;address+=PAGE_SIZE)
   {
    int s;

    fprintf(stderr,"\rReading: %6ld",address);

    rs232_read(fd,page,PAGE_SIZE);

    for(s=0;s<PAGE_SIZE;s++)
      {
       int sampleval=-1;
       unsigned char byte=page[s];

       if((prevbyte&0xf0)==0x80)  /* 1000 xxxx , yyyy yyyy */
         {
          sampleval=((prevbyte&0x0f)<<8)+byte;
          byte=0xff;            /* forget this byte value next time round in case it is 0x8* */
         }
       else if(byte==0xff)        /* 1111 1111 */
         {
          sampletime+=period;
          if(prevbyte!=0xff)
             printf("\n");
         }
       else if((byte&0xf0)==0x80) /* 1000 xxxx */
          ;
       else if(byte&0x80)         /* 1xxx xxxx */
          sampleval=prevsampleval+(signed char)byte+1;
       else                       /* 0xxx xxxx */
          sampleval=prevsampleval+byte;

       if(sampleval!=-1)
         {
          struct tm *sampletm;
          char samplestr[20];
          double resistance,temperature,current;

          sampletime+=period;
          sampletm=localtime(&sampletime);
          strftime(samplestr,sizeof(samplestr),"%Y-%m-%d %H:%M:%S",sampletm);

          resistance=(1024/((double)sampleval/4))*RESISTOR-RESISTOR;
          current=supply_voltage/(resistance+RESISTOR);
          temperature=1/(log(resistance/THERMISTOR_R25)/THERMISTOR_B+1/THERMISTOR_T25)-THERMISTOR_T0-current*current*resistance/THERMISTOR_DIS;

          printf("%s %7.2f %5.2f\n",samplestr,(double)sampleval/4,temperature);

          prevsampleval=sampleval;

          lastgood=address+s;
         }

       prevbyte=byte;
      }
   }

 fprintf(stderr,"\nReading %lu .. done ... %.0f%% full\n",eeprom_size,(double)((lastgood+1)*100/eeprom_size));

 rs232_read(fd,cmd,3);

 /* Finish */

 rs232_close(fd);

 return(0);
}

