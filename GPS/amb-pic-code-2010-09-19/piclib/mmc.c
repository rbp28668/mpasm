/***************************************
 $Header: /home/amb/pic/piclib/RCS/mmc.c,v 1.3 2008/01/29 19:47:32 amb Exp $

 Program file for MMC data.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2007,08 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <stdio.h>
#include <unistd.h>

#include "mmc.h"
#include "spi.h"


/*+ A variable to enable verbose reporting. +*/
int mmc_verbose=0;

/*+ The value of the most recent R1 response. +*/
unsigned int mmc_errno;


/* Data (contains dummy prefix byte to get card started) */

/* CMD0 - GO_IDLE_STATE - response R1 */

static unsigned char mmc_cmd0_string[]={0x40, 0x00, 0x00, 0x00, 0x00, 0x95};
static unsigned int  mmc_cmd0_length=sizeof(mmc_cmd0_string);

/* CMD1 - SEND_OP_COND - response R1 */

static unsigned char mmc_cmd1_string[]={0xff,0x41, 0x00, 0x00, 0x00, 0x00, 0xff};
static unsigned int  mmc_cmd1_length=sizeof(mmc_cmd1_string);

/* CMD6 - SWITCH_FUNC - response R1 */

/* CMD8 - SEND_IF_COND - response R7 */

static unsigned char mmc_cmd8_string[]={0xff,0x48, 0x00, 0x00, 0x01, 0xAA, 0xff};
static unsigned int  mmc_cmd8_length=sizeof(mmc_cmd8_string);

/* CMD9 - SEND_CSD - response R1 + 16 bytes + 2 byte CRC */

static unsigned char mmc_cmd9_string[]={0xff,0x49, 0x00, 0x00, 0x00, 0x00, 0xff};
static unsigned int  mmc_cmd9_length=sizeof(mmc_cmd9_string);

/* CMD10 - SEND_CID - response R1 + 16 bytes + 2 byte CRC */

static unsigned char mmc_cmd10_string[]={0xff,0x4a, 0x00, 0x00, 0x00, 0x00, 0xff};
static unsigned int  mmc_cmd10_length=sizeof(mmc_cmd10_string);

/* CMD12 - STOP_TRANSMISSION - response R1b */

/* CMD13 - SEND_STATUS - response R2 */

// static unsigned char mmc_cmd13_string[]={0xff,0x4d, 0x00, 0x00, 0x00, 0x00, 0xff};
// static unsigned int  mmc_cmd13_length=sizeof(mmc_cmd13_string);

/* CMD16 - SET_BLOCKLEN - response R1 */

/* CMD17 - READ_SINGLE_BLOCK - response R1 + block of data */

/* CMD18 - READ_MULTIPLE_BLOCK - response R1 + block(s) of data */

/* CMD24 - WRITE_BLOCK - response R1 */

/* CMD25 - WRITE_MULTIPLE_BLOCK - response R1 */

/* CMD27 - PROGRAM_CSD - response R1 */

/* CMD28 - SET_WRITE_PROT - response R1b */

/* CMD29 - CLR_WRITE_PROT - response R1b */

/* CMD30 - SEND_WRITE_PROT - response R1 */

/* CMD32 - ERASE_WR_BLK_START_ADDR - response R1 */

/* CMD33 - ERASE_WR_BLK_END_ADDR - response R1 */

/* CMD38 - ERASE - response R1b */

/* CMD42 - LOCK_UNLOCK - response R1 */

/* CMD55 - APP_CMD - response R1 */

static unsigned char mmc_cmd55_string[]={0xff,0x77, 0x00, 0x00, 0x00, 0x00, 0xff};
static unsigned int  mmc_cmd55_length=sizeof(mmc_cmd55_string);

/* CMD56 - GEN_CMD - response R1 */

/* CMD58 - READ_OCR - response R3 */

static unsigned char mmc_cmd58_string[]={0xff,0x7A, 0x00, 0x00, 0x00, 0x00, 0xff};
static unsigned int  mmc_cmd58_length=sizeof(mmc_cmd58_string);

/* CMD59 - CRC_ON_OFF - response R1 */

/* ACMD13 - SD_STATUS - response R2 */

/* ACMD22 - SEND_NUM_WR_BLOCKS - response R1 */

/* ACMD23 - SET_WR_BLK_ERASE_COUNT - response R1 */

/* ACMD41 - SD_SEND_OP_COND - response R1 */

static unsigned char mmc_acmd41_string[]={0xff,0x69, 0x00, 0x00, 0x00, 0x00, 0xff};
static unsigned int  mmc_acmd41_length=sizeof(mmc_acmd41_string);

/* ACMD42 - SET_CLR_CARD_DETECT - response R1 */

/* ACMD51 - SEND_SCR - response R1 */

static unsigned char mmc_acmd51_string[]={0xff,0x73, 0x00, 0x00, 0x00, 0x00, 0xff};
static unsigned int  mmc_acmd51_length=sizeof(mmc_acmd51_string);



/*++++++++++++++++++++++++++++++++++++++
  Send a CMD0 and print the response

  int mmc_send_cmd0 Returns 0 if OK, something else in case of error.

  int fd The file descriptor of the RS232 connection
  ++++++++++++++++++++++++++++++++++++++*/

int mmc_send_cmd0(int fd)
{
 int retval=1;

 if(mmc_verbose)
    printf("\nMMC: GO_IDLE - Go to the idle state\n");

 spi_enable(fd);

 spi_write(fd,mmc_cmd0_string,mmc_cmd0_length);

 if(mmc_wait_r1(fd)==0x01)
    retval=0;

 spi_disable(fd);

 return(retval);
}


/*++++++++++++++++++++++++++++++++++++++
  Send a CMD1 and print the response

  int mmc_send_cmd1 Returns 0 if OK, something else in case of error.

  int fd The file descriptor of the RS232 connection
  ++++++++++++++++++++++++++++++++++++++*/

int mmc_send_cmd1(int fd)
{
 int retval=1;

 if(mmc_verbose)
    printf("\nMMC: SEND_OP_CMD - Initialise MMC card\n");

 spi_enable(fd);

 while(1)
   {
    spi_write(fd,mmc_cmd1_string,mmc_cmd1_length);

    if(mmc_wait_r1(fd)==0x00)
      {retval=0; break;}

    usleep(1000);
   }

 spi_disable(fd);

 return(retval);
}


/*++++++++++++++++++++++++++++++++++++++
  Send a CMD8 and print the response

  int mmc_send_cmd8 Returns 0 if OK, something else in case of error.

  int fd The file descriptor of the RS232 connection
  ++++++++++++++++++++++++++++++++++++++*/

int mmc_send_cmd8(int fd)
{
 int retval=1;
 unsigned char reply[4];

 if(mmc_verbose)
    printf("\nMMC: SEND_IF_CMD - Sends interface command\n");

 spi_enable(fd);

 spi_write(fd,mmc_cmd8_string,mmc_cmd8_length);

 if(mmc_wait_r7(fd,reply)==0x01 &&
    (mmc_cmd8_string[4]&0x0f)==(reply[2]&0x0f) &&
    mmc_cmd8_string[5]==reply[3])
    retval=0;

 spi_disable(fd);

 return(retval);
}


/*++++++++++++++++++++++++++++++++++++++
  Send a CMD9 and print the response

  int mmc_send_cmd9 Returns 0 if OK, something else in case of error.

  int fd The file descriptor of the RS232 connection
  ++++++++++++++++++++++++++++++++++++++*/

int mmc_send_cmd9(int fd)
{
 int retval=1;
 unsigned char reply[18];

 if(mmc_verbose)
    printf("\nMMC: SEND_CSD - Read CSD from MMC card\n");

 spi_enable(fd);

 spi_write(fd,mmc_cmd9_string,mmc_cmd9_length);

 if(!mmc_wait_r1(fd))
   {
    spi_read(fd,reply,1);
    spi_read(fd,reply,18);

    if((reply[0]&0xc0)==0)
      {
       char *TAAC_value[16]={"?", "1.0", "1.2", "1.3", "1.5", "2.0", "2.5", "3.0", "3.5", "4.0", "4.5", "5.0", "5.5", "6.0", "7.0", "8.0"};
       char *TAAC_units[ 8]={"1ns", "10ns", "100ns", "1us", "10us", "100us", "1ms", "10ms"};
       char *TRAN_SPEED_value[16]={"?", "1.0", "1.2", "1.3", "1.5", "2.0", "2.5", "3.0", "3.5", "4.0", "4.5", "5.0", "5.5", "6.0", "7.0", "8.0"};
       char *TRAN_SPEED_units[ 8]={"100kbit/s", "1Mbit/s", "10Mbit/s", "100Mbit/s", "?", "?", "?", "?"};
       char *CURRENT_value[8]={"0.5", "1", "5", "10", "25", "35", "60", "100"};
       char *FILE_FORMAT_type[4]={"Hard disk like (partition table)", "Floppy like (boot sector)", "Universal file format", "Other/Unknown"};

       printf("MMC: CSD[127:126] Structure Version 1  : %01x\n",(reply[0]&0xc0)>>6);
       printf("MMC: CSD[125:120] reserved             : %02x\n",reply[0]&0x3f);
       printf("MMC: CSD[119:112] Data read access time: %02x = %s * %s\n",reply[1],TAAC_value[(reply[1]&0x78)>>3],TAAC_units[(reply[1]&0x07)]);
       printf("MMC: CSD[111:104] Data read access time: %02x = %d00 clock cycles\n",reply[2],reply[2]);
       printf("MMC: CSD[103: 96] Data transfer speed  : %02x = %s * %s\n",reply[3],TRAN_SPEED_value[(reply[3]&0x78)>>3],TRAN_SPEED_units[(reply[3]&0x07)]);
       printf("MMC: CSD[ 95: 84] Card command classes : %02x%01x\n",reply[4],reply[5]>>4);
       printf("MMC: CSD[ 83: 80] Max read block length: %01x = %d bytes\n",reply[5]&0x0f,1<<(reply[5]&0x0f));
       printf("MMC: CSD[ 79: 79] Read partial allowed : %01x\n",!!(reply[6]&0x80));
       printf("MMC: CSD[ 78: 78] Write block misalign : %01x\n",!!(reply[6]&0x40));
       printf("MMC: CSD[ 77: 77] Read block misalign  : %01x\n",!!(reply[6]&0x20));
       printf("MMC: CSD[ 76: 76] DSR Implemented      : %01x\n",!!(reply[6]&0x10));
       printf("MMC: CSD[ 75: 74] reserved             : %02x\n",(reply[6]&0x0c)>>2);
       printf("MMC: CSD[ 73: 62] Card size (C_SIZE)   : %03x = %d * C_MULT * BLOCK_LEN bytes\n",((reply[6]&0x03)<<10)+(reply[7]<<2)+((reply[8]&0xc0)>>6),((reply[6]&0x03)<<10)+(reply[7]<<2)+((reply[8]&0xc0)>>6)+1);
       printf("MMC: CSD[ 61: 59] Read current min Vdd : %01x = %s mA\n",(reply[8]&0x38)>>3,CURRENT_value[(reply[8]&0x38)>>3]);
       printf("MMC: CSD[ 58: 56] Read current max Vdd : %01x = %s mA\n", reply[8]&0x07    ,CURRENT_value[ reply[8]&0x07    ]);
       printf("MMC: CSD[ 55: 53] Write current min Vdd: %01x = %s mA\n",(reply[9]&0xe0)>>5,CURRENT_value[(reply[9]&0xe0)>>5]);
       printf("MMC: CSD[ 52: 50] Write current max Vdd: %01x = %s mA\n",(reply[9]&0x1c)>>2,CURRENT_value[(reply[9]&0x1c)>>2]);
       printf("MMC: CSD[ 49: 47] Card size (C_MULT)   : %01x = %d * C_SIZE * BLOCK_LEN bytes\n",((reply[9]&0x03)<<1)+((reply[10]&0x80)>>7),4<<(((reply[9]&0x03)<<1)+((reply[10]&0x80)>>7)));
       printf("MMC: CSD[ 46: 46] Erase single block   : %01x\n",!!(reply[10]&0x40));
       printf("MMC: CSD[ 45: 39] Erase sector size    : %02x = %d * WRITE_BLOCK\n",((reply[10]&0x3f)<<1)+((reply[11]&0x80)>>7),((reply[10]&0x3f)<<1)+((reply[11]&0x80)>>7)+1);
       printf("MMC: CSD[ 38: 32] Write prot group size: %02x = %d * ERASE_SECTOR\n",reply[11]&0x7f,(reply[11]&0x7f)+1);
       printf("MMC: CSD[ 31: 31] Write prot group en  : %01x\n",!!(reply[12]&0x80));
       printf("MMC: CSD[ 30: 29] reserved             : %02x\n",(reply[12]&0x60)>>2);
       printf("MMC: CSD[ 28: 26] Write speed factor   : %01x = 1/%d * read speed\n",(reply[12]&0x1c)>>2,1<<((reply[12]&0x1c)>>2));
       printf("MMC: CSD[ 25: 22] Write block length   : %01x = %d bytes\n",((reply[12]&0x03)<<2)+((reply[13]&0xc0)>>6),1<<(((reply[12]&0x03)<<2)+((reply[13]&0xc0)>>6)));
       printf("MMC: CSD[ 21: 21] Write block partial  : %01x\n",!!(reply[13]&0x20));
       printf("MMC: CSD[ 20: 16] reserved             : %02x\n",reply[13]&0x1f);
       printf("MMC: CSD[ 15: 15] File format group    : %01x\n",!!(reply[14]&0x80));
       printf("MMC: CSD[ 14: 14] Copy flag            : %01x\n",!!(reply[14]&0x40));
       printf("MMC: CSD[ 13: 13] Permanent write prot : %01x\n",!!(reply[14]&0x20));
       printf("MMC: CSD[ 12: 12] Temporary write prot : %01x\n",!!(reply[14]&0x10));
       printf("MMC: CSD[ 11: 10] File format          : %01x = %s\n",(reply[14]&0x0c)>>2,FILE_FORMAT_type[(reply[14]&0x0c)>>2]);
       printf("MMC: CSD[  9:  8] reserved             : %02x\n",reply[14]&0x03);
       printf("MMC: CSD[  7:  1] CRC7                 : %02x\n",(reply[15]&0xfe)>>1);
       printf("MMC: CSD[  0:  0] not used, always one : %01x\n",reply[15]&0x01);
      }
    else if((reply[0]&0xc0)==0x40)
      {
       printf("MMC: CSD[127:126] Structure Version 2  : %01x\n",(reply[0]&0xc0)>>6);
      }
    else
      {
       printf("MMC: CSD[127:126] Structure Version Error: %01x\n",(reply[0]&0xc0)>>6);
      }

    retval=0;
   }

 spi_disable(fd);

 return(retval);
}


/*++++++++++++++++++++++++++++++++++++++
  Send a CMD10 and print the response

  int mmc_send_cmd10 Returns 0 if OK, something else in case of error.

  int fd The file descriptor of the RS232 connection
  ++++++++++++++++++++++++++++++++++++++*/

int mmc_send_cmd10(int fd)
{
 int retval=1;
 unsigned char reply[18];

 if(mmc_verbose)
    printf("\nMMC: SEND_CID - Read CID from MMC card\n");

 spi_enable(fd);

 spi_write(fd,mmc_cmd10_string,mmc_cmd10_length);

 if(!mmc_wait_r1(fd))
   {
    spi_read(fd,reply,1);
    spi_read(fd,reply,18);

    printf("MMC: CID[127:120] MID : Manufacturer ID   : %02x\n",reply[0]);
    printf("MMC: CID[119:104] OID : OEM/Application ID: %02x%02x\n",reply[1],reply[2]);
    printf("MMC: CID[103: 64] PNM : Product name      : %02x%02x%02x%02x%02x = '%c%c%c%c%c'\n",reply[3],reply[4],reply[5],reply[6],reply[7],reply[3],reply[4],reply[5],reply[6],reply[7]);
    printf("MMC: CID[ 63: 56] PRV : Product revision  : %02x = %d.%d\n",reply[8],(reply[8]&0xf0)>>4,reply[8]&0x0f);
    printf("MMC: CID[ 55: 24] PSN : Product serial No : %02x%02x%02x%02x\n",reply[9],reply[10],reply[11],reply[12]);
    printf("MMC: CID[ 23: 20] reserved                : %02x\n",(reply[13]&0xf0)>>4);
    printf("MMC: CID[ 19:  8] MDT : Manufacturing date: %01x%02x = %d/%d\n",reply[13]&0x0f,reply[14],2000+((reply[13]&0x0f)<<4)+((reply[14]&0xf0)>>4),reply[14]&0x0f);
    printf("MMC: CID[  7:  1] CRC : CRC7              : %02x\n",(reply[15]&0xfe)>>1);
    printf("MMC: CID[  0:  0] not used, always one    : %01x\n",reply[15]&0x01);

    retval=0;
   }

 spi_disable(fd);

 return(retval);
}


/*++++++++++++++++++++++++++++++++++++++
  Send a CMD58 and print the response

  int mmc_send_cmd58 Returns 0 if OK, something else in case of error.

  int fd The file descriptor of the RS232 connection
  ++++++++++++++++++++++++++++++++++++++*/

int mmc_send_cmd58(int fd)
{
 int retval=1;
 unsigned char reply[4];

 if(mmc_verbose)
    printf("\nMMC: READ_OCR - Read OCR from MMC card\n");

 spi_enable(fd);

 spi_write(fd,mmc_cmd58_string,mmc_cmd58_length);

 if(!mmc_wait_r3(fd,reply))
   {
    printf("MMC: OCR[31:31] : not busy    : %01x\n",!!(reply[0]&0x80));
    printf("MMC: OCR[30:30] : CCS         : %01x\n",!!(reply[0]&0x40));
    printf("MMC: OCR[29:24] : reserved    : %02x\n",!!(reply[0]&0x3f));
    printf("MMC: OCR[23:23] : 3.5V-3.6V   : %01x\n",!!(reply[1]&0x80));
    printf("MMC: OCR[22:22] : 3.4V-3.5V   : %01x\n",!!(reply[1]&0x40));
    printf("MMC: OCR[21:21] : 3.3V-3.4V   : %01x\n",!!(reply[1]&0x20));
    printf("MMC: OCR[20:20] : 3.2V-3.3V   : %01x\n",!!(reply[1]&0x10));
    printf("MMC: OCR[19:19] : 3.1V-3.2V   : %01x\n",!!(reply[1]&0x08));
    printf("MMC: OCR[18:18] : 3.0V-3.1V   : %01x\n",!!(reply[1]&0x04));
    printf("MMC: OCR[17:17] : 2.9V-3.0V   : %01x\n",!!(reply[1]&0x02));
    printf("MMC: OCR[16:16] : 2.8V-2.9V   : %01x\n",!!(reply[1]&0x01));
    printf("MMC: OCR[15:15] : 2.7V-2.8V   : %01x\n",!!(reply[2]&0x80));
    printf("MMC: OCR[14: 8] : reserved    : %02x\n",!!(reply[2]&0x7f));
    printf("MMC: OCR[ 7: 7] : low voltage : %01x\n",!!(reply[3]&0x80));
    printf("MMC: OCR[ 6: 0] : reserved    : %02x\n",!!(reply[3]&0x7f));

    retval=0;
   }

 spi_disable(fd);

 return(retval);
}


/*++++++++++++++++++++++++++++++++++++++
  Send a ACMD41 and print the response

  int mmc_send_acmd41 Returns 0 if OK, something else in case of error.

  int fd The file descriptor of the RS232 connection
  ++++++++++++++++++++++++++++++++++++++*/

int mmc_send_acmd41(int fd)
{
 int retval=1;

 if(mmc_verbose)
    printf("\nMMC: SD_SEND_OP_CMD - Initialise SD card\n");

 spi_enable(fd);

 while(1)
   {
    spi_write(fd,mmc_cmd55_string,mmc_cmd55_length);

    mmc_wait_r1(fd);

    spi_write(fd,mmc_acmd41_string,mmc_acmd41_length);

    if(mmc_wait_r1(fd)==0x00)
      {retval=0; break;}

    usleep(1000);
   }

 spi_disable(fd);

 return(retval);
}


/*++++++++++++++++++++++++++++++++++++++
  Send a ACMD51 and print the response

  int mmc_send_acmd51 Returns 0 if OK, something else in case of error.

  int fd The file descriptor of the RS232 connection
  ++++++++++++++++++++++++++++++++++++++*/

int mmc_send_acmd51(int fd)
{
 int retval=1;
 unsigned char reply[8];

 if(mmc_verbose)
    printf("\nMMC: SEND_SCR - Read SCR from MMC card\n");

 spi_enable(fd);

 spi_write(fd,mmc_cmd55_string,mmc_cmd55_length);

 mmc_wait_r1(fd);

 spi_write(fd,mmc_acmd51_string,mmc_acmd51_length);

 if(!mmc_wait_r1(fd))
   {
    spi_read(fd,reply,2);
    spi_read(fd,reply,8);

    if((reply[0]&0xc0)==0)
      {
       char *SD_SPEC_value[16]={"1.0-1.01", "1.10", "2.00", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?"};
       char *SD_SECURITY_value[8]={"None", "?", "Version 1.01", "Version 2.00", "?", "?", "?", "?"};
       char *SD_BUS_WIDTHS_value[16]={"1 bit", "?", "?", "?", "4 bits", "1 or 4 bits", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?"};

       printf("MMC: SCR[63:60] SCR_STRUCTURE            : %01x = SD spec 1.01-2.00\n",(reply[0]&0xf0)>>4);
       printf("MMC: SCR[59:56] SD_SPEC                  : %01x = SD spec %s\n",reply[0]&0x0f,SD_SPEC_value[reply[0]&0x0f]);
       printf("MMC: SCR[55:55] DATA_STAT_AFTER_ERASE    : %01x\n",reply[1]&0x80);
       printf("MMC: SCR[54:52] SD_SECURITY              : %01x = %s\n",(reply[1]&0x70)>>4,SD_SECURITY_value[(reply[1]&0x70)>>4]);
       printf("MMC: SCR[48:51] SD_BUS_WIDTHS            : %01x = %s\n",reply[1]&0x0f,SD_BUS_WIDTHS_value[reply[1]&0x0f]);
       printf("MMC: SCR[47:32] reserved                 : %02x%02x\n",reply[2],reply[3]);
       printf("MMC: SCR[31: 0] reserved for manufacturer: %02x%02x%02x%02x\n",reply[4],reply[5],reply[6],reply[7]);
      }
    else
      {
       printf("MMC: SCR[63:60] SCR_STRUCTURE            : %01x (Error)\n",(reply[0]&0xf0)>>4);
      }

    retval=0;
   }

 spi_disable(fd);

 return(retval);
}


/*++++++++++++++++++++++++++++++++++++++
  Wait for an R1 response to appear (single byte, first bit is zero).

  unsigned char mmc_wait_r1 Returns the R1 response.

  int fd The file descriptor of the RS232.
  ++++++++++++++++++++++++++++++++++++++*/

unsigned char mmc_wait_r1(int fd)
{
 unsigned char rep;
 int old_spi_verbose;

 if(mmc_verbose)
    printf("MMC: Waiting for R1");

 old_spi_verbose=spi_verbose;

 if(mmc_verbose)
    spi_verbose=0;

 do
   {
    spi_read(fd,&rep,1);

    if(mmc_verbose)
      {printf(" %02x",rep);fflush(stdout);}
   }
 while(rep&0x80);

 mmc_errno=rep;

 if(mmc_verbose)
   {
    printf("\n");

    if(rep&0x40) printf("MMC:    R1 Error - Parameter error\n");
    if(rep&0x20) printf("MMC:    R1 Error - Address error\n");
    if(rep&0x10) printf("MMC:    R1 Error - Erase sequence error\n");
    if(rep&0x08) printf("MMC:    R1 Error - CRC error error\n");
    if(rep&0x04) printf("MMC:    R1 Error - Illegal command\n");
    if(rep&0x02) printf("MMC:    R1 Error - Erase reset\n");
    if(rep&0x01) printf("MMC:    R1 Error - Idle state\n");
   }

 spi_verbose=old_spi_verbose;

 return(rep);
}


/*++++++++++++++++++++++++++++++++++++++
  Wait for an R3 response to appear (five bytes, first bit is zero).

  unsigned char mmc_wait_r3 Returns the R1 part of the response.

  unsigned char reply[4] Returns the other 4 bytes of the response.

  int fd The file descriptor of the RS232.
  ++++++++++++++++++++++++++++++++++++++*/

unsigned char mmc_wait_r3(int fd, unsigned char reply[4])
{
 unsigned char rep;
 int old_spi_verbose;

 if(mmc_verbose)
    printf("MMC: Waiting for R3");

 old_spi_verbose=spi_verbose;

 if(mmc_verbose)
    spi_verbose=0;

 do
   {
    spi_read(fd,&rep,1);

    if(mmc_verbose)
      {printf(" %02x",rep);fflush(stdout);}
   }
 while(rep&0x80);

 mmc_errno=rep;

 spi_read(fd,reply,4);

 if(mmc_verbose)
   {printf(" %02x %02x %02x %02x",reply[0],reply[1],reply[2],reply[3]);fflush(stdout);}

 if(mmc_verbose)
   {
    printf("\n");

    if(rep&0x40) printf("MMC:    R3 Error - Parameter error\n");
    if(rep&0x20) printf("MMC:    R3 Error - Address error\n");
    if(rep&0x10) printf("MMC:    R3 Error - Erase sequence error\n");
    if(rep&0x08) printf("MMC:    R3 Error - CRC error error\n");
    if(rep&0x04) printf("MMC:    R3 Error - Illegal command\n");
    if(rep&0x02) printf("MMC:    R3 Error - Erase reset\n");
    if(rep&0x01) printf("MMC:    R3 Error - Idle state\n");
   }

 spi_verbose=old_spi_verbose;

 return(rep);
}


/*++++++++++++++++++++++++++++++++++++++
  Wait for an R7 response to appear (five bytes, first bit is zero).

  unsigned char mmc_wait_r7 Returns the R1 part of the response.

  unsigned char reply[4] Returns the other 4 bytes of the response.

  int fd The file descriptor of the RS232.
  ++++++++++++++++++++++++++++++++++++++*/

unsigned char mmc_wait_r7(int fd, unsigned char reply[4])
{
 unsigned char rep;
 int old_spi_verbose;

 if(mmc_verbose)
    printf("MMC: Waiting for R7");

 old_spi_verbose=spi_verbose;

 if(mmc_verbose)
    spi_verbose=0;

 do
   {
    spi_read(fd,&rep,1);

    if(mmc_verbose)
      {printf(" %02x",rep);fflush(stdout);}
   }
 while(rep&0x80);

 mmc_errno=rep;

 spi_read(fd,reply,4);

 if(mmc_verbose)
   {printf(" %02x %02x %02x %02x",reply[0],reply[1],reply[2],reply[3]);fflush(stdout);}

 if(mmc_verbose)
   {
    printf("\n");

    if(rep&0x40) printf("MMC:    R7 Error - Parameter error\n");
    if(rep&0x20) printf("MMC:    R7 Error - Address error\n");
    if(rep&0x10) printf("MMC:    R7 Error - Erase sequence error\n");
    if(rep&0x08) printf("MMC:    R7 Error - CRC error error\n");
    if(rep&0x04) printf("MMC:    R7 Error - Illegal command\n");
    if(rep&0x02) printf("MMC:    R7 Error - Erase reset\n");
    if(rep&0x01) printf("MMC:    R7 Error - Idle state\n");
   }

 spi_verbose=old_spi_verbose;

 return(rep);
}
