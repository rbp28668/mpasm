/***************************************
 $Header: /home/amb/pic/piclib/RCS/tcl.c,v 1.7 2008/07/11 18:54:47 amb Exp $

 Source file for Tcl access to piclib functions.
 ******************/ /******************
 Written by Andrew M. Bishop

 This file Copyright 2007,08 Andrew M. Bishop
 It may be distributed under the GNU Public License, version 2, or
 any higher version.  See section COPYING of the GNU Public license
 for conditions under which this file may be redistributed.
 ***************************************/


#include <string.h>
#include <stdlib.h>

#include "tcl8.4/tcl.h"

#include "rs232.h"
#include "i2c.h"
#include "spi.h"
#include "mmc.h"


/* A macro to simplify Tcl error checking */

#define TCL_ASSERT(condition,string) \
{ \
 if(!(condition)) \
   { \
    Tcl_SetResult(interp,string,NULL); \
    return(TCL_ERROR); \
   } \
}

/* A macro to simplify getting an integer from Tcl */

#define TCL_INTEGER(object,integer,string) \
{ \
 if(Tcl_GetIntFromObj(interp,object,&(integer))==TCL_ERROR) \
   { \
    Tcl_SetResult(interp,string,NULL); \
    return(TCL_ERROR); \
   } \
}

/* A macro to simplify getting a variable object from Tcl */

#define TCL_VARIABLE(object,variable,string) \
{ \
 if((variable=Tcl_ObjGetVar2(interp,object,NULL,0))==NULL) \
   { \
    Tcl_SetResult(interp,string,NULL); \
    return(TCL_ERROR); \
   } \
}

/* A macro to simplify getting a list from Tcl */

#define TCL_LIST(object,integer,pointer,string) \
{ \
 if(Tcl_ListObjGetElements(interp,object,&(integer),&(pointer))==TCL_ERROR || integer==0) \
   { \
    Tcl_SetResult(interp,string,NULL); \
    return(TCL_ERROR); \
   } \
}


/* Global functions (only used in Tcl) */

int Piclib_Init(Tcl_Interp *interp);

/* Local functions - RS232 */

static int piclib_Tcl_rs232(ClientData clientData,Tcl_Interp *interp, int objc, Tcl_Obj * CONST objv[]);

/* Local functions - I2C */

static int piclib_Tcl_i2c(ClientData clientData,Tcl_Interp *interp, int objc, Tcl_Obj * CONST objv[]);

/* Local functions - SPI */

static int piclib_Tcl_spi(ClientData clientData,Tcl_Interp *interp, int objc, Tcl_Obj * CONST objv[]);

/* Local functions - MMC */

static int piclib_Tcl_mmc(ClientData clientData,Tcl_Interp *interp, int objc, Tcl_Obj * CONST objv[]);


/*++++++++++++++++++++++++++++++++++++++
  Tcl initialisation function that is run when the library is loaded by Tcl.

  int Piclib_Init Returns a status, TCL_OK or TCL_ERROR.

  Tcl_Interp *interp The Tcl interpreter that it is being loaded into.
  ++++++++++++++++++++++++++++++++++++++*/

int Piclib_Init(Tcl_Interp *interp)
{
 Tcl_CreateObjCommand(interp,"::piclib::rs232",piclib_Tcl_rs232,NULL,NULL);

 Tcl_CreateObjCommand(interp,"::piclib::i2c",piclib_Tcl_i2c,NULL,NULL);

 Tcl_CreateObjCommand(interp,"::piclib::spi",piclib_Tcl_spi,NULL,NULL);

 Tcl_CreateObjCommand(interp,"::piclib::mmc",piclib_Tcl_mmc,NULL,NULL);

 return(TCL_OK);
}


/*++++++++++++++++++++++++++++++++++++++
  The command interpreter for the piclib rs232 commands.

  int piclib_Tcl_rs232 Returns a status, TCL_OK or TCL_ERROR.

  ClientData clientData Client data passed in on every call (always NULL in this case).

  Tcl_Interp *interp The Tcl interpreter.

  int objc The number of command arguments.

  Tcl_Obj * CONST objv[] The command arguments.
  ++++++++++++++++++++++++++++++++++++++*/

static int piclib_Tcl_rs232(ClientData clientData,Tcl_Interp *interp, int objc, Tcl_Obj * CONST objv[])
{
 char *command;

 TCL_ASSERT(objc>1,"Allowed rs232 commands are: open close read write rts cts.");

 command=Tcl_GetStringFromObj(objv[1],NULL);

 if(!strcmp(command,"open"))
   {
    int fd,speed,flow;
    char *device;

    TCL_ASSERT(objc==5,"Expected exactly four arguments: open device speed flow");

    device=Tcl_GetStringFromObj(objv[2],NULL);

    TCL_INTEGER(objv[3],speed,"Expected integer as third parameter: speed");

    TCL_INTEGER(objv[4],flow ,"Expected integer as fourth parameter: flow");

    fd=rs232_open(device,speed,flow);

    if(fd<0)
      {
       Tcl_SetResult(interp,"",NULL);
       return(TCL_ERROR);
      }
    else
       Tcl_SetObjResult(interp,Tcl_NewIntObj(fd));
   }
 else if(!strcmp(command,"close"))
   {
    int fd;

    TCL_ASSERT(objc==3,"Expected exactly two arguments: close fd");

    TCL_INTEGER(objv[2],fd,"Expected integer as second parameter: fd");

    rs232_close(fd);
   }
 else if(!strcmp(command,"read"))
   {
    int fd,nbytes,nread;
    Tcl_Obj *variable;
    unsigned char *data;

    TCL_ASSERT(objc==5,"Expected exactly four arguments: read fd &data nbytes");

    TCL_INTEGER(objv[2],fd,"Expected integer as second parameter: fd");

    TCL_VARIABLE(objv[3],variable,"Expected variable name as third parameter: &data");

    TCL_INTEGER(objv[4],nbytes,"Expected integer as fourth parameter: nbytes");

    data=(unsigned char*)malloc(nbytes);

    nread=rs232_read(fd,data,nbytes);

    Tcl_SetObjResult(interp,Tcl_NewIntObj(nread));

    if(nread>=0)
       Tcl_ObjSetVar2(interp,objv[3],NULL,Tcl_NewByteArrayObj(data,nread),0);
    else
       Tcl_ObjSetVar2(interp,objv[3],NULL,Tcl_NewByteArrayObj(data,0),0);

    free(data);

    if(nread<0)
       return(TCL_ERROR);
   }
 else if(!strcmp(command,"write"))
   {
    int fd,nbytes,nwrite;
    Tcl_Obj *variable;
    unsigned char *data;

    TCL_ASSERT(objc==5,"Expected exactly four arguments: read fd &data nbytes");

    TCL_INTEGER(objv[2],fd,"Expected integer as second parameter: fd");

    TCL_VARIABLE(objv[3],variable,"Expected variable name as third parameter: &data");

    TCL_INTEGER(objv[4],nbytes,"Expected integer as fourth parameter: nbytes");

    data=Tcl_GetByteArrayFromObj(variable,NULL);

    nwrite=rs232_write(fd,data,nbytes);

    Tcl_SetObjResult(interp,Tcl_NewIntObj(nwrite));

    if(nwrite<0)
       return(TCL_ERROR);
   }
 else if(!strcmp(command,"rts"))
   {
    int fd,state;

    TCL_ASSERT(objc==4,"Expected exactly three arguments: rts fd state");

    TCL_INTEGER(objv[2],fd,"Expected integer as second parameter: fd");

    TCL_INTEGER(objv[3],state,"Expected integer as third parameter: state");

    rs232_rts(fd,state);
   }
 else if(!strcmp(command,"cts"))
   {
    int fd,state;

    TCL_ASSERT(objc==3,"Expected exactly two arguments: cts fd");

    TCL_INTEGER(objv[2],fd,"Expected integer as second parameter: fd");

    state=rs232_cts(fd);

    Tcl_SetObjResult(interp,Tcl_NewIntObj(state));
   }
 else
   {
    Tcl_SetResult(interp,"Allowed rs232 commands are: open close read write rts cts.",NULL);

    return(TCL_ERROR);
   }

 return(TCL_OK);
}


/*++++++++++++++++++++++++++++++++++++++
  The command interpreter for the piclib i2c commands.

  int piclib_Tcl_i2c Returns a status, TCL_OK or TCL_ERROR.

  ClientData clientData Client data passed in on every call (always NULL in this case).

  Tcl_Interp *interp The Tcl interpreter.

  int objc The number of command arguments.

  Tcl_Obj * CONST objv[] The command arguments.
  ++++++++++++++++++++++++++++++++++++++*/

static int piclib_Tcl_i2c(ClientData clientData,Tcl_Interp *interp, int objc, Tcl_Obj * CONST objv[])
{
 char *command;
 int fd;

 TCL_ASSERT(objc>=2,"Allowed i2c commands are: read{1,2,4} write{1,2,4}.");

 command=Tcl_GetStringFromObj(objv[1],NULL);

 TCL_INTEGER(objv[2],fd,"Expected integer as second parameter: fd");

 if(!strncmp(command,"read",4))
   {
    int bus_address,address_nbytes,data_address,data_nbytes;
    unsigned char *data;
    int result;
    Tcl_Obj *variable;

    address_nbytes=atoi(&command[4]);

    TCL_ASSERT(address_nbytes==1 || address_nbytes==2 || address_nbytes==3,"Allowed i2c commands are: read{1,2,4} write{1,2,4}.");

    TCL_ASSERT(objc==7,"Expected exactly six parameters: read{1,2,4} fd bus_address data_address &data nbytes");

    TCL_INTEGER(objv[3],bus_address,"Expected integer as third parameter: bus_address");

    TCL_INTEGER(objv[4],data_address,"Expected integer as fourth parameter: data_address");

    TCL_VARIABLE(objv[5],variable,"Expected variable name as fifth parameter: &data");

    TCL_INTEGER(objv[6],data_nbytes,"Expected integer as sixth parameter: nbytes");

    data=(unsigned char*)malloc(data_nbytes);

    result=i2c_read(fd,bus_address,address_nbytes,data_address,data_nbytes,data);

    Tcl_SetObjResult(interp,Tcl_NewIntObj(result));

    if(result==0)
       Tcl_ObjSetVar2(interp,objv[5],NULL,Tcl_NewByteArrayObj(data,data_nbytes),0);
    else
       Tcl_ObjSetVar2(interp,objv[5],NULL,Tcl_NewByteArrayObj(data,0),0);

    free(data);

    if(result!=0)
       return(TCL_ERROR);
   }
 else if(!strncmp(command,"write",5))
   {
    int bus_address,address_nbytes,data_address,data_nbytes;
    unsigned char *data;
    int result;
    Tcl_Obj *variable;

    address_nbytes=atoi(&command[5]);

    TCL_ASSERT(address_nbytes==1 || address_nbytes==2 || address_nbytes==3,"Allowed i2c commands are: read{1,2,4} write{1,2,4}.");

    TCL_ASSERT(objc==7,"Expected exactly six parameters: write{1,2,4} fd bus_address data_address &data nbytes");

    TCL_INTEGER(objv[3],bus_address,"Expected integer as third parameter: bus_address");

    TCL_INTEGER(objv[4],data_address,"Expected integer as fourth parameter: data_address");

    TCL_VARIABLE(objv[5],variable,"Expected variable name as fifth parameter: &data");

    TCL_INTEGER(objv[6],data_nbytes,"Expected integer as sixth parameter: nbytes");

    data=Tcl_GetByteArrayFromObj(variable,NULL);

    result=i2c_write(fd,bus_address,address_nbytes,data_address,data_nbytes,data);

    Tcl_SetObjResult(interp,Tcl_NewIntObj(result));

    if(result<0)
       return(TCL_ERROR);
   }
 else
   {
    Tcl_SetResult(interp,"Allowed i2c commands are: read{1,2,4} write{1,2,4}.",NULL);

    return(TCL_ERROR);
   }

 return(TCL_OK);
}


/*++++++++++++++++++++++++++++++++++++++
  The command interpreter for the piclib spi commands.

  int piclib_Tcl_spi Returns a status, TCL_OK or TCL_ERROR.

  ClientData clientData Client data passed in on every call (always NULL in this case).

  Tcl_Interp *interp The Tcl interpreter.

  int objc The number of command arguments.

  Tcl_Obj * CONST objv[] The command arguments.
  ++++++++++++++++++++++++++++++++++++++*/

static int piclib_Tcl_spi(ClientData clientData,Tcl_Interp *interp, int objc, Tcl_Obj * CONST objv[])
{
 char *command;
 int fd;

 TCL_ASSERT(objc>=2,"Allowed spi commands are: speed start stop enable disable write read xchange.");

 command=Tcl_GetStringFromObj(objv[1],NULL);

 TCL_INTEGER(objv[2],fd,"Expected integer as second parameter: fd");

 if(!strcmp(command,"speed"))
   {
    int speed;

    TCL_ASSERT(objc==4,"Expected exactly three parameters: speed fd speed.");

    TCL_INTEGER(objv[3],speed,"Expected integer as third parameter: speed");

    spi_speed(fd,speed);
   }
 else if(!strcmp(command,"start"))
   {
    TCL_ASSERT(objc==3,"Expected exactly two parameter: start fd.");

    spi_start(fd);
   }
 else if(!strcmp(command,"stop"))
   {
    TCL_ASSERT(objc==3,"Expected exactly two parameter: stop fd.");

    spi_stop(fd);
   }
 else if(!strcmp(command,"enable"))
   {
    TCL_ASSERT(objc==3,"Expected exactly two parameter: enable fd.");

    spi_enable(fd);
   }
 else if(!strcmp(command,"disable"))
   {
    TCL_ASSERT(objc==3,"Expected exactly two parameter: disable fd.");

    spi_disable(fd);
   }
 else if(!strcmp(command,"write"))
   {
    Tcl_Obj *variable;
    unsigned char *data;
    int result,nbytes;

    TCL_ASSERT(objc==5,"Expected exactly four parameters: write fd &data nbytes.");

    TCL_VARIABLE(objv[3],variable,"Expected variable name as third parameter: &data");

    TCL_INTEGER(objv[4],nbytes,"Expected integer as fourth parameter: nbytes");

    data=Tcl_GetByteArrayFromObj(variable,NULL);

    result=spi_write(fd,data,nbytes);

    Tcl_SetObjResult(interp,Tcl_NewIntObj(result));

    if(result<0)
       return(TCL_ERROR);
   }
 else if(!strcmp(command,"read"))
   {
    Tcl_Obj *variable;
    unsigned char *data;
    int result,nbytes;

    TCL_ASSERT(objc==5,"Expected exactly four parameters: read fd &data nbytes.");

    TCL_VARIABLE(objv[3],variable,"Expected variable name as third parameter: &data");

    TCL_INTEGER(objv[4],nbytes,"Expected integer as fourth parameter: nbytes");

    data=(unsigned char*)malloc(nbytes);

    result=spi_read(fd,data,nbytes);

    Tcl_SetObjResult(interp,Tcl_NewIntObj(result));

    if(result==0)
       Tcl_ObjSetVar2(interp,objv[3],NULL,Tcl_NewByteArrayObj(data,nbytes),0);
    else
       Tcl_ObjSetVar2(interp,objv[3],NULL,Tcl_NewByteArrayObj(data,0),0);

    free(data);

    if(result<0)
       return(TCL_ERROR);
   }
 else if(!strcmp(command,"xchange"))
   {
    Tcl_Obj *variable;
    unsigned char *data;
    int result,nbytes;

    TCL_ASSERT(objc==5,"Expected exactly four parameters: xchange fd &data nbytes.");

    TCL_VARIABLE(objv[3],variable,"Expected variable name as third parameter: &data");

    TCL_INTEGER(objv[4],nbytes,"Expected integer as fourth parameter: nbytes");

    data=(unsigned char*)malloc(nbytes);

    memcpy(data,Tcl_GetStringFromObj(variable,NULL),nbytes);

    result=spi_xchange(fd,data,nbytes);

    Tcl_SetObjResult(interp,Tcl_NewIntObj(result));

    if(result==0)
       Tcl_ObjSetVar2(interp,objv[3],NULL,Tcl_NewByteArrayObj(data,nbytes),0);
    else
       Tcl_ObjSetVar2(interp,objv[3],NULL,Tcl_NewByteArrayObj(data,0),0);

    free(data);

    if(result<0)
       return(TCL_ERROR);
   }
 else
   {
    Tcl_SetResult(interp,"Allowed spi commands are: speed start stop enable disable write read xchange.",NULL);
    return(TCL_ERROR);
   }

 return(TCL_OK);
}


/*++++++++++++++++++++++++++++++++++++++
  The command interpreter for the piclib mmc commands.

  int piclib_Tcl_mmc Returns a status, TCL_OK or TCL_ERROR.

  ClientData clientData Client data passed in on every call (always NULL in this case).

  Tcl_Interp *interp The Tcl interpreter.

  int objc The number of command arguments.

  Tcl_Obj * CONST objv[] The command arguments.
  ++++++++++++++++++++++++++++++++++++++*/

static int piclib_Tcl_mmc(ClientData clientData,Tcl_Interp *interp, int objc, Tcl_Obj * CONST objv[])
{
 char *command;
 int fd;

 TCL_ASSERT(objc>=2,"Allowed mmc commands are: cmd0 cmd1 cmd8 cmd9 cmd10 cmd58 acmd41 acmd51 wait_r1 wait_r3 wait_r7.");

 command=Tcl_GetStringFromObj(objv[1],NULL);

 TCL_INTEGER(objv[2],fd,"Expected integer as second parameter: fd");

 if(!strcmp(command,"cmd0"))
   {
    TCL_ASSERT(objc==2,"Expected exactly one parameter: fd.");

    if(mmc_send_cmd0(fd))
       return(TCL_ERROR);
   }
 else if(!strcmp(command,"cmd1"))
   {
    TCL_ASSERT(objc==2,"Expected exactly one parameter: fd.");

    if(mmc_send_cmd1(fd))
       return(TCL_ERROR);
   }
 else if(!strcmp(command,"cmd8"))
   {
    TCL_ASSERT(objc==2,"Expected exactly one parameter: fd.");

    if(mmc_send_cmd8(fd))
       return(TCL_ERROR);
   }
 else if(!strcmp(command,"cmd9"))
   {
    TCL_ASSERT(objc==2,"Expected exactly one parameter: fd.");

    if(mmc_send_cmd9(fd))
       return(TCL_ERROR);
   }
 else if(!strcmp(command,"cmd10"))
   {
    TCL_ASSERT(objc==2,"Expected exactly one parameter: fd.");

    if(mmc_send_cmd10(fd))
       return(TCL_ERROR);
   }
 else if(!strcmp(command,"cmd58"))
   {
    TCL_ASSERT(objc==2,"Expected exactly one parameter: fd.");

    if(mmc_send_cmd58(fd))
       return(TCL_ERROR);
   }
 else if(!strcmp(command,"acmd41"))
   {
    TCL_ASSERT(objc==2,"Expected exactly one parameter: fd.");

    if(mmc_send_acmd41(fd))
       return(TCL_ERROR);
   }
 else if(!strcmp(command,"acmd51"))
   {
    TCL_ASSERT(objc==2,"Expected exactly one parameter: fd.");

    if(mmc_send_acmd51(fd))
       return(TCL_ERROR);
   }
 else if(!strcmp(command,"wait_r1"))
   {
    TCL_ASSERT(objc==2,"Expected exactly one parameter: fd.");

    if(mmc_wait_r1(fd))
       return(TCL_ERROR);
   }
 else if(!strcmp(command,"wait_r3"))
   {
    unsigned char reply[4];

    TCL_ASSERT(objc==2,"Expected exactly one parameter: fd.");

    if(mmc_wait_r3(fd,reply))
       return(TCL_ERROR);
   }
 else if(!strcmp(command,"wait_r7"))
   {
    unsigned char reply[4];

    TCL_ASSERT(objc==2,"Expected exactly one parameter: fd.");

    if(mmc_wait_r7(fd,reply))
       return(TCL_ERROR);
   }
 else
   {
    Tcl_SetResult(interp,"Allowed mmc commands are: cmd0 cmd1 cmd8 cmd9 cmd10 cmd58 acmd41 acmd51 wait_r1 wait_r3 wait_r7.",NULL);
    return(TCL_ERROR);
   }

 return(TCL_OK);
}
