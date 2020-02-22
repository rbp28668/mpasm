/***************************************
 $Header: /home/amb/pic/projects/07_rs232-to-spi/RCS/testmmc.c,v 1.6 2008/05/04 13:53:33 amb Exp $

 Program to test access to an MMC/SD card using SPI library functions.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2006,07,08 Andrew M. Bishop
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
#include "../piclib/spi.h"
#include "../piclib/mmc.h"


int main(int argc, char **argv)
{
 int fd;
 char *port=argv[1];
 unsigned char reply[256];

 if(argc!=2)
   {
    fprintf(stderr,"Usage: testmmc <serial-device-name>\n");
    return(1);
   }

 /* Open serial port */

 fd=rs232_open(port,38400,1);

 if(fd<0)
   {
    fprintf(stderr,"Error opening %s\n",port);
    return(1);
   }

 /* Set verbose flags and initialise generic mode in PIC */

 spi_verbose=1;
 mmc_verbose=1;

 spi_start(fd);

 spi_read(fd,reply,16);

 /* Initialise an MMC card */

 printf("\nInitialise MMC card\n");
 printf("-------------------\n");

 if(mmc_send_cmd0(fd))
   {
    fprintf(stderr,"*** Error with CMD0 trying again ***\n");

    if(mmc_send_cmd0(fd))
      {fprintf(stderr,"*** Error with CMD0 ***\n"); goto finish;}
   }

 if(mmc_send_cmd8(fd))
   {
    if(mmc_errno&0x04)
      {
       fprintf(stderr,"*** Version 1.x SD card or MMC card ***\n");
       mmc_send_cmd1(fd);
      }
    else
      {fprintf(stderr,"*** Error with CMD8 ***\n"); goto finish;}
   }
 else
   {
    fprintf(stderr,"*** Version 2.x SD card ***\n");
    mmc_send_acmd41(fd);
   }

 /* Read the OCR and SCR from the MMC card */

 printf("\nRead OCR and SCR from MMC card\n");
 printf("------------------------------\n");

 if(mmc_send_cmd58(fd))
    fprintf(stderr,"*** Error with CMD58 ***\n");

 if(mmc_send_acmd51(fd))
    fprintf(stderr,"*** Error with ACMD51 ***\n");

 /* Read the CSD and CID from the MMC card */

 printf("\nRead CID and CSD from MMC card\n");
 printf("------------------------------\n");

 if(mmc_send_cmd9(fd))
    fprintf(stderr,"*** Error with CMD9 ***\n");

 if(mmc_send_cmd10(fd))
    fprintf(stderr,"*** Error with CMD10 ***\n");

 /* Finish */

 finish:

 spi_stop(fd);

 rs232_close(fd);

 return(0);
}
