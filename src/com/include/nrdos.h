/*********************************************************************
 * Project : NuoRDS Proxy
 *
 *********************************************************************
 * Description :  Operating system definitions.
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

#ifndef  __NRD_OS_DEFINITIONS_H__
#define  __NRD_OS_DEFINITIONS_H__

/*NOTE: Keep C compatibility here*/

#if defined(MACOSX) || defined(__APPLE_CC__)
    #define OS_MACOSX
#elif defined(WIN32) || defined(WIN64)
    #define  OS_WINDOWS
#elif defined(__linux__) 
    #define OS_LINUX
#endif

#if defined(WIN64)     || defined(__LP64__) ||\
    defined(__64BIT__) || defined(_LP64) ||\
    defined(LP64)      || defined(__x86_64__)
  #define OS_64BIT
#else
  #define OS_32BIT
#endif

#ifdef __cplusplus
  #define OS_EXTERN_C extern "C"
  #define OS_EXTERN   extern
#else
  #define OS_EXTERN_C
  #define OS_EXTERN
#endif

#ifndef NULL
   #define NULL    0
#endif

/*COMPILER-DEPENDENT DEFINITIONS*/

#if defined(_MSC_VER)
   #define OS_W64       __w64
   #define OS_STDCALL   __stdcall
   #define OS_CDECL     __cdecl
   #define OS_VISIBLE
   #define OS_HIDDEN
   #define OS_ALIGN(_n) __declspec(align(_n))
#elif defined(__GNUC__)
   #define OS_W64       __attribute__((mode (__pointer__)))
   #ifndef OS_64BIT
      #define OS_STDCALL   __attribute__((stdcall))
      #define OS_CDECL     __attribute__((cdecl))
   #else  /*GCC do not support "stdcall&cdecl" on 64bit*/
      #define OS_STDCALL
      #define OS_CDECL
   #endif

   #define OS_VISIBLE   __attribute__((visibility("default")))
   #define OS_HIDDEN    __attribute__((visibility("hidden")))
   #define OS_ALIGN(_n) __attribute__ ((aligned (_n)))
  
#else
   #define OS_W64
   #define OS_STDCALL
   #define OS_CDECL
   #define OS_VISIBLE
   #define OS_ALIGN(_n)
#endif

/**NOTE: To keep portability, use OS_ALIGN macro
  after the typename declaration. For example:
  typedef struct OS_ALIGN(4) _MyStruct{int a;}  MyStruct;
*/

/*SYSTEM-DEPENDENT DEFINITIONS*/

/*************OS_WINDOWS*************/
#if defined(OS_WINDOWS)
/*******************************/

#if defined(OS_32BIT) /*WIN32*/

  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif

  #ifndef WINVER
    #define WINVER 0x0600             // Allow use of features specific to Windows 2000 or later.
  #endif

  #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0600       // Allow use of features specific to Windows 2000 or later.
  #endif

  #ifndef _WIN32_WINDOWS
    #define _WIN32_WINDOWS 0x0600    // Allow use of features specific to Windows 2000 or later.
  #endif

  #ifndef _WIN32_IE
    #define _WIN32_IE 0x0600      // Allow use of features specific to IE 5.0 or later.
  #endif

#else /*WIN64*/
    /*TODO*/
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <windows.h>
#include <tchar.h>
#include <string.h>

#ifndef LOG_EMERG
    #define	LOG_EMERG	   0	
    #define	LOG_ALERT	   1	
    #define	LOG_CRIT	   2	
    #define	LOG_ERR		   3	
    #define	LOG_WARNING	 4
    #define	LOG_NOTICE	 5	
    #define	LOG_INFO	   6	
    #define	LOG_DEBUG	   7
    #define	LOG_WARN	  LOG_WARNING
#endif 

#ifndef INVALID_SOCKET
    #define INVALID_SOCKET  (SOCKET)(~0) //WinSock2.h
#endif

#ifndef SOCKET_ERROR
    #define SOCKET_ERROR (-1)
#endif

#define OS_EAGAIN WSAEWOULDBLOCK
#define OS_EINTR  WSAEINTR
#define OS_EINPR  WSAEINPROGRESS
#define OS_EALRED WSAEALREADY
#define OS_ETOUT  WSAETIMEDOUT
#define OS_NOHOST WSAEHOSTUNREACH

typedef SOCKET OS_SOCKET;
typedef int pid_t; 
typedef HMODULE os_module_t;

#ifndef snprintf
    #define snprintf  _snprintf
    #define vsnprintf _vsnprintf
#endif

static int setenv(const char * var, const char * val,int over)
{
   if(var && (getenv(var) == NULL || over))
   {
     char buf[1024];buf[sizeof(buf)-1] = '\0';
     strncpy(buf,var,sizeof(buf)-1);
     strncat(buf,"=",sizeof(buf)-1);
     if(val) strncat(buf,val,sizeof(buf)-1);
     return _putenv(buf);
   }
   return -1;
};

static int unsetenv(const char * var)
{
   if(getenv(var) == NULL) return 0;
   return setenv(var,NULL,1);
};

#define OS_DYLIB_CONSTRUCTOR()  \
    static void initializer(void)

#define OS_DYLIB_DESTRUCTOR()   \
    static void finalizer(void)

#define OS_DYLIB_ENTRY()                              \
os_module_t os_dylib_module = NULL;                  \
BOOL APIENTRY DllMain( HANDLE hModule,               \
DWORD  _reason, LPVOID _reserved)                    \
{ if(_reason == DLL_PROCESS_ATTACH)                  \
  {os_dylib_module = (os_module_t)hModule;           \
  initializer();} else if                            \
  (_reason == DLL_PROCESS_DETACH){finalizer(); }     \
  return TRUE;  }

#define os_load_module(h)       LoadLibrary(h)
#define os_unload_module(h)     FreeLibrary(h)
#define os_get_proc(h, sym)     GetProcAddress(h, sym)

/**************************************/
#else //assume macOS/Linux/Unix
/**************************************/

typedef void *  os_module_t;

#define OS_DYLIB_CONSTRUCTOR() __attribute__((constructor)) \
    static void initializer(void)

#define OS_DYLIB_DESTRUCTOR()  __attribute__((destructor))  \
    static void finalizer(void)

#define OS_DYLIB_ENTRY()            \
os_module_t os_dylib_module = NULL;\
int         __argc = 0;/*dummy*/   \
char **     __argv = NULL;/*dummy*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/timeb.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/un.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef LOG_WARN
   #define LOG_WARN LOG_WARNING
#endif

#ifndef stricmp
    #define stricmp strcasecmp
#endif
#ifndef closesocket
     #define closesocket close
#endif
#ifndef ioctlsocket
    #define ioctlsocket ioctl
#endif

#include <dlfcn.h>

#define os_load_module(name)    dlopen(name, RTLD_LAZY|RTLD_LOCAL)
#define os_unload_module(h)     dlclose(h)
#define os_get_proc(h, sym)     dlsym(h, sym)
#define os_module_handle()      0

#ifndef INVALID_SOCKET
      #define INVALID_SOCKET (-1)
#endif

#ifndef SOCKET_ERROR
      #define SOCKET_ERROR (-1)
#endif  

typedef int OS_SOCKET;

#ifndef SD_BOTH
   #define SD_RECEIVE      SHUT_RD
   #define SD_SEND         SHUT_WR
   #define SD_BOTH         SHUT_RDWR
#endif

/**************************************/
#endif /*macOS/Linux*/
/**************************************/

/*NOTE: Use *.def file on windows*/
#define OS_DYLIB_EXPORT OS_EXTERN_C OS_VISIBLE
#define OS_DYLIB_HIDDEN OS_HIDDEN

/*Default dynamic library entry declaration*/
/*To customize library entry use the following lines:
  OS_DYLIB_CONSTRUCTOR(){write constructor code here};
  OS_DYLIB_DESTRUCTOR(){write destructor code here};
  OS_DYLIB_ENTRY();
*/
#define OS_DECLARE_DYLIB_ENTRY_DEFAULT()          \
OS_DYLIB_CONSTRUCTOR(){}                          \
OS_DYLIB_DESTRUCTOR(){}                           \
OS_DYLIB_ENTRY()

/*The external value represents the real loadable
  module handle (is valid for WINDOWS only)*/
OS_EXTERN os_module_t os_dylib_module;

/*Helper to declare aligned types.
  For example: Use
  OS_TYPEDEF_ALLIGNED(struct,4,_MyStruct)
  instead of
  typedef struct _MyStruct
*/

#define  OS_TYPEDEF_ALIGNED(_type,_align,_name)  \
     typedef _type OS_ALIGN(_align) _name

/*Workaround: Some NRD files inclede nrdos.h only*/
#if defined(OS_MACOSX) && !defined(NRD_MACOSX)
    #define NRD_MACOSX
#elif defined(OS_WINDOWS) && !defined(NRD_WINDOWS)
    #define NRD_WINDOWS
#elif defined(OS_LINUX) && !defined(NRD_LINUX)
    #define NRD_LINUX
#endif

#endif /*__NRD_OS_DEFINITIONS_H__*/

