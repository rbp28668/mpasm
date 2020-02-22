/***************************************
 $Header: /home/amb/pic/piclib/RCS/mmc.h,v 1.3 2008/01/29 19:47:32 amb Exp $

 Header file for MMC functions.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2007,08 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#ifndef MMC_H
#define MMC_H    /*+ To stop multiple inclusions. +*/

/*+ A variable to enable verbose reporting. +*/
extern int mmc_verbose;

/*+ The value of the most recent R1 response. +*/
extern unsigned int mmc_errno;

/* Functions */

int mmc_send_cmd0(int fd);

int mmc_send_cmd1(int fd);

int mmc_send_cmd8(int fd);

int mmc_send_cmd9(int fd);

int mmc_send_cmd10(int fd);

int mmc_send_cmd58(int fd);

int mmc_send_acmd41(int fd);

int mmc_send_acmd51(int fd);

unsigned char mmc_wait_r1(int fd);

unsigned char mmc_wait_r3(int fd,unsigned char reply[4]);

unsigned char mmc_wait_r7(int fd,unsigned char reply[4]);

#endif /* MMC_H */
