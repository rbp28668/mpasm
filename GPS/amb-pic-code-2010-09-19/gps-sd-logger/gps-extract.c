/***************************************
 $Header: /home/amb/pic/projects/11_gps-sd-logger/RCS/gps-extract.c,v 1.6 2007/05/21 16:10:00 amb Exp $

 GPS from SD card data extractor
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2007 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>


#define BLOCKSIZE 512

int main(int argc, char **argv)
{
 int fd;
 char *path=argv[1];
 int zeroblocks=0;
 unsigned long address=0;
 time_t lasttime=0;
 char lasttime_str[7],lastdate_str[7];
 FILE *file=NULL;
 off_t skip=0;

 if(argc!=2 && (argc!=4 || strcmp(argv[2],"-skip")))
   {
    fprintf(stderr,"Usage: gps-extract <sd-card> [-skip nn]\n");
    return(1);
   }

 /* Open SD card or image file */

 fd=open(path,O_RDONLY);

 if(fd<0)
   {
    fprintf(stderr,"Error opening %s\n",path);
    return(1);
   }

 /* Skip ahead */

 if(argc==4)
   {
    skip=atol(argv[3]);

    if(lseek(fd,skip*BLOCKSIZE,SEEK_SET)!=skip*BLOCKSIZE)
      {
       fprintf(stderr,"Failed to skip %ld bytes to block %ld.\n",skip*BLOCKSIZE,skip);
       close(fd);
       return(1);
      }
   }

 /* Read blocks of BLOCKSIZE until the end of the file */

 for(address=skip*BLOCKSIZE;;address+=BLOCKSIZE)
   {
    char data[BLOCKSIZE+1];
    int i,n;
    int allzero=1;

    n=read(fd,data,BLOCKSIZE);

    if(n==0)
       break;
    else if(n!=BLOCKSIZE)
      {
       fprintf(stderr,"Failed to read %d bytes at block %ld.\n",BLOCKSIZE,address/BLOCKSIZE);
       exit(1);
      }

    data[BLOCKSIZE]=0;

    for(i=0;i<BLOCKSIZE;i++)
       if(data[i]!=0)
         {allzero=0; break;}

    if(allzero)
       fprintf(stderr,"All zero block at block %ld.\n",address/BLOCKSIZE);

    if(address==skip*BLOCKSIZE || (zeroblocks>0 && !allzero))
      {
       lasttime=0;
       strcpy(lasttime_str,"??????");
       strcpy(lastdate_str,"??????");
      }

    if(allzero)
       zeroblocks++;
    else
       zeroblocks=0;

    if(zeroblocks==0)
      {
       char *rmc=strstr(data,"$GPRMC");

       if(rmc)
         {
          time_t thistime,now=time(NULL);
          char thistime_str[7],thisdate_str[7];
          struct tm tm;

          sscanf(rmc,"$GPRMC,%6s.%*d,%*c,%*f,%*c,%*f,%*c,%*f,%*f,%6s",thistime_str,thisdate_str);

          tm.tm_hour=(thistime_str[0]-'0')*10+(thistime_str[1]-'0');
          tm.tm_min =(thistime_str[2]-'0')*10+(thistime_str[3]-'0');
          tm.tm_sec =(thistime_str[4]-'0')*10+(thistime_str[5]-'0');

          tm.tm_mday=(thisdate_str[0]-'0')*10+(thisdate_str[1]-'0');
          tm.tm_mon =(thisdate_str[2]-'0')*10+(thisdate_str[3]-'0')-1;
          tm.tm_year=(thisdate_str[4]-'0')*10+(thisdate_str[5]-'0'); if(tm.tm_year<80) tm.tm_year+=100;

          tm.tm_isdst=0;

          thistime=mktime(&tm);

          if(abs(thistime-lasttime)>2)
             fprintf(stderr,"Time step from %6s %6s to %6s %6s at block %ld.\n",lastdate_str,lasttime_str,thisdate_str,thistime_str,address/BLOCKSIZE);

          if(abs(thistime-lasttime)>300 && file)
            {
             fprintf(stderr,"Closing file.\n");
             fclose(file);
             file=NULL;
            }

          if(!file && abs(now-thistime)<14*24*3600)
            {
             char filename[32];
             sprintf(filename,"%04d-%02d-%02d-%02d-%02d.nmea",tm.tm_year+1900,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min);
             file=fopen(filename,"w");
             fprintf(stderr,"Opening file '%s'.\n",filename);
            }

          strcpy(lasttime_str,thistime_str);
          strcpy(lastdate_str,thisdate_str);
          lasttime=thistime;
         }

       if(file)
          fprintf(file,"%s",data);
      }
    else if(file)
      {
       fprintf(stderr,"Closing file.\n");
       fclose(file);
       file=NULL;
      }

    if(zeroblocks==2)
       break;
   }

 /* Finish */

 close(fd);

 return(0);
}

