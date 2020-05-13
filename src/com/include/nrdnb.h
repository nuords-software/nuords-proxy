/*********************************************************************
 * Project : NuoRDS Proxy
 *
 *********************************************************************
 * Description :  The network browser  protocol. 
 *********************************************************************
 *
 * Copyright 2006-2020, Volodymyr Bykov. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *********************************************************************
 * 
 * As of May 1 2020, a revocable permission to distribute binary copies 
 * of this software without source code and/or copyright notice has been
 * granted to Ozolio Inc (a Hawaii Corporation) by the author. 
 *
 ********************************************************************/

#ifndef __NRD_NETWORK_BROWSER_PROTO_H__
#define __NRD_NETWORK_BROWSER_PROTO_H__

#include "nrdtype.h"

#define NRD_BROWSE_PORT  4073 //deafult net-browser port

/*DATAGRAM SIGNATURE*/
static  NRD_CCHAR NRD_NBSIG[7]={'i','R','A','P','P','S','B'};

/*DATA CONTAINER, (372 bytes)*/
NRD_TYPEDEF_STRUCT(_HNRD_NBDAT)
{
    NRD_BYTE     type;      /*Data type (NRD_NBT)*/
    NRD_BYTE     cont[371]; /*Content. Do not decrease size.*/
}HNRD_NBDAT;

/*MESSAGE CONTAINER, (380 bytes)*/
NRD_TYPEDEF_STRUCT(_HNRD_NBMSG)
{
    NRD_CHAR      sig[7]; /*Datagram signature (NRD_NBSIG)*/
    NRD_BYTE      msg;    /*Message number (NRD_NBM)*/
    HNRD_NBDAT    dat;    /*Message data (depending on msg)*/
}HNRD_NBMSG;

/********************************************************************/
/*DATA TYPES*/

//Server information
#define NRDNBT_INFO    1  //HNRD_NBINF

NRD_TYPEDEF_STRUCT(_HNRD_NBINF)
{
    NRD_BYTE     type;        //Must be NRDNBT_INFO
    NRD_BYTE     res1;        /*Reserved, must be 0*/ 
    NRD_USHORT   cmax;        /*Maximum number of connections*/
    NRD_CHAR     name[64];    /*Machine name */
    NRD_CHAR     desc[256];   /*Machine description */
    NRD_USHORT   sver[4];     /*NuoRDS server version*/
    
    struct _svc
    {
    	 NRD_UINT    pver;      /*Must be NRD_MAKEUINT(0,5).*/
    	 NRD_UINT    port;      /*The main service port*/
    	 NRD_USHORT  ccnt;      /*The connections count*/
    	 NRD_USHORT  stat;      /*The service state (NRDNBS)*/
    }svc;
    
    struct _mem
    {
    	 NRD_BYTE    stat;     /*Memory usage flags (NRDNBU)*/
    	 NRD_BYTE    free;     /*Free memory, percentage (0...100%)*/
    	 NRD_BYTE    used;     /*Used memory, percentage (0...100%)*/
    	 NRD_BYTE    res2;     /*1 byte reserved (must be 0)*/
    }mem;
    
    struct _cpu
    {
    	 NRD_BYTE    stat;     /*Memory usage flags (NRDNBU)*/
    	 NRD_BYTE    free;     /*Free CPU, percentage (0...100%)*/
    	 NRD_BYTE    used;     /*Used CPU, percentage (0...100%)*/
    	 NRD_BYTE    res2;     /*1 byte reserved (must be 0)*/
    }cpu;
    
    //TODO: More bytes should be reserved to fit 380 bytes. 
    NRD_BYTE    res2[8];       /*8 bytes reserved(must be 0)*/
}HNRD_NBINF;

//Service state flags (nrd.stat/rdp.stat)
#define NRDNBS_ACTIVE    0x0001 //The service is active now

//Resource usage state flags (mem.stat/cpu.stat) 
#define NRDNBU_FREE      0x01 //mem.free/cpu.free is valid
#define NRDNBU_USED      0x02 //mem.used/cpu.used is valid
#define NRDNBU_HIGH      0x04 //The resource usage is >95%

/********************************************************************/
/*MESSAGES*/
#define NRDNBM_MIN        1 //Minimal message number 

/*
  Message     : NRDNBM_WHOHERE
  Description : Server(s) status request
  Direction   : Client->Broadcast
  Data type   : none, ignored
*/
#define NRDNBM_WHOHERE    1 

/*
  Message     : NRDNBM_IAMHERE
  Description : Server is online
  Direction   : Server->Broadcast
  Data type   : NRDNBT_INFO
*/
#define NRDNBM_IAMHERE    2

/*
  Message     : NRDNBM_IAMLOST
  Description : Server went down
  Direction   : Server->Broadcast
  Data type   : NRDNBT_INFO
*/
#define NRDNBM_IAMLOST    3

/*
  Message     : NRDNBM_GETINFO
  Description : Server info request
  Direction   : Client->Server
  Data type   : none, ignored
*/
#define NRDNBM_GETINFO    4

/*
  Message     : NRDNBM_RETINFO
  Description : Server info response
  Direction   : Server->Client
  Data type   : NRDNBT_INFO
*/
#define NRDNBM_RETINFO    5

//TODO: Update NRDNBM_MAX after adding a new message
#define NRDNBM_MAX        5 //Maximal message number  


#endif //__NRD_SERVERS_BROWSER_UDP_PROTO_H__

