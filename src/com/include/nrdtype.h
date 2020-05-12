/*********************************************************************
 * Project : NuoRDS Proxy
 *
 *********************************************************************
 * Programmer(s) :  Volodymyr Bykov
 *
 *********************************************************************
 * Description :  Basic types. 
 *
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

#ifndef  __NRD_TYPES_H__
#define  __NRD_TYPES_H__

#include "nrdos.h"
#include "nrderr.h"

/*OS SYNONYMS**************************/
#define __nrd_export       OS_DYLIB_EXPORT
#define __nrd_hidden       OS_DYLIB_HIDDEN
#define __nrd_extern       OS_EXTERN_C
#define __nrdcall          OS_STDCALL
#define __nrddecl          OS_CDECL

/*NOTE: Use NRD_TYPEDEF_STRUCT to declare
  structure properly aligned by default(on 4)*/
#define  NRD_TYPEDEF_STRUCT(_name) OS_TYPEDEF_ALIGNED(struct,4,_name) 

/*BASIC SIMPLE TYPES********************/

typedef void                NRD_VOID;      /*Void type*/
typedef char                NRD_CHAR;      /*Signed char type (8bit)*/
typedef unsigned char       NRD_UCHAR;     /*Unsigned char type (8bit)*/
typedef short               NRD_SHORT;     /*Signed short type (16bit)*/
typedef unsigned short      NRD_USHORT;    /*Unsigned short type (16bit)*/
typedef int                 NRD_INT;       /*Signed int type (32bit)*/
typedef unsigned int        NRD_UINT;      /*Unsigned int type (32bit)*/
typedef float               NRD_FLOAT;     /*Floating point type(32bit)*/
typedef double              NRD_DOUBLE;    /*Double float (64bit)*/

#ifdef OS_64BIT

  #define  NRD_64BIT_MODE   1
    
  typedef int               NRD_LONG;  /* Signed long type (32bit)*/
  typedef unsigned int      NRD_ULONG; /* Signed long type (32bit)*/  
  
  #if defined(__GNUC__)
        typedef long int  NRD_LONG64,NRD_INT64;   /*Signed long type (64bit)*/
        typedef unsigned long int   NRD_ULONG64,NRD_UINT64; /*Unsigned long type (64bit)*/
  #else
        typedef __int64           NRD_LONG64,NRD_INT64;   /*Signed long type (64bit)*/
        typedef unsigned __int64  NRD_ULONG64,NRD_UINT64; /*Unsigned long type (64bit)*/
  #endif     
#else
  typedef long              NRD_LONG;      /*Signed long type (32bit)*/
  typedef unsigned long     NRD_ULONG;     /*Unsigned long type (32bit)*/ 
#endif



#define NRD_MAXUINT     0xFFFFFFFF
#define NRD_MAXULONG    0xFFFFFFFF
#define NRD_MAXUSHORT   0xFFFF
#define NRD_MAXUCHAR    0xFF

#define NRD_MAXINT      0x7FFFFFFF
#define NRD_MAXLONG     0x7FFFFFFF
#define NRD_MAXSHORT    0x7FFF
#define NRD_MAXCHAR     0x7F

#define NRD_INFINITE    NRD_MAXUINT 

#define NRD_MAKELONG(lo, hi)  \
 ((NRD_LONG)(((NRD_USHORT)((NRD_ULONG)(lo) & 0xffff)) | \
 ((NRD_ULONG)((NRD_USHORT)((NRD_ULONG)(hi) & 0xffff))) << 16))

#define NRD_MAKEUINT(lo, hi) ((NRD_UINT)NRD_MAKELONG(lo, hi))

#define NRD_LOSHORT(l)   ((NRD_SHORT)((NRD_ULONG)(l) & 0xffff))
#define NRD_HISHORT(l)   ((NRD_SHORT)((NRD_ULONG)(l) >> 16))
#define NRD_LOWORD(l)    ((NRD_USHORT)NRD_LOSHORT(l))
#define NRD_HIWORD(l)    ((NRD_USHORT)NRD_HISHORT(l))

#define NRD_CCHAR         const NRD_CHAR  /* "const char" definition */ 

typedef NRD_INT           NRDRC;          /* Return code type */
typedef NRD_CHAR *        NRD_PCHAR;      /* char buffer pointer type */
typedef NRD_CCHAR *       NRD_PCCHAR;     /* const char buffer pointer type */  
typedef NRD_UCHAR         NRD_BYTE;       /* 8 bit unsigned char type */
typedef NRD_BYTE *        NRD_PBYTE;      /* 8 bit unsigned char buffer pointer */
typedef NRD_INT           NRD_BOOL;       /* 32 bit boolean type*/
typedef NRD_VOID*         NRD_PVOID;      
typedef NRD_PVOID         NRD_RGN;

#define NRD_FALSE         0
#define NRD_TRUE          1
#define NRD_NOSOCK        INVALID_SOCKET  /*Invalid socket*/
#define NRD_NOFD          (-1)            /*Invalid fole decriptor*/
#define NRD_NOINDEX       NRD_MAXUINT     /*Undefined index*/
#define NRD_NOUID         NRD_MAXUINT     /*Undefined User ID*/
#define NRD_NOGID         NRD_MAXUINT     /*Undefined Group ID*/
#define NRD_NOPID         NRD_MAXUINT     /*Undefined Process ID*/
#define NRD_NOSID         0               /*Undefined Session ID*/

#define NRD_SOCKET        OS_SOCKET


/**************************************/
#endif
