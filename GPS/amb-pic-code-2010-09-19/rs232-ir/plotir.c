/***************************************
 $Header: /home/amb/pic/projects/05_rs232-infra-red/RCS/plotir.c,v 1.4 2007/02/15 19:29:51 amb Exp $

 Program to convert captured file to gnuplottable format.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2006 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char ** argv)
{
 int state,delta,time;

 /* Read in file and write out in GNUPLOT plottable format */

 printf("%4d %d\n",0,0);

 time=0;

 while(scanf("%d %d",&state,&delta)==2)
   {
    printf("%4d %d\n",time,state);

    time+=delta;

    printf("%4d %d\n",time,state);
   }

 return(0);
}

