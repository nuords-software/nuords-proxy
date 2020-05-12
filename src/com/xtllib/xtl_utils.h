/*********************************************************************
 * Project : XTL
 *
 *********************************************************************
 * Description :
 *
 *********************************************************************
 *
 * Copyright 2006, Volodymyr Bykov. All rights reserved.
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
 ********************************************************************/

#ifndef __XTL_UTILS_IMPL_H__
#define __XTL_UTILS_IMPL_H__

#include "xtl_env.h"
#include "xtl_algo.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if !defined (XTL_WINDOWS)
  extern int __argc;          /* count of cmd line args */
  extern char ** __argv;      /* pointer to table of cmd line args */   
#endif

#include "xtl_env.h"
/*-------------------------------------------------------------------*/
namespace xtl
{
    /*Prototypes*/
    static  bool    parse_path(char * val,int slash = 1);
    static  char *  parse_pdir(char * buf,int len,int slash = 1);
    static  int     parse_fname(const char * path,char * buf,int len);

    /* Implementations */
    static  bool  parse_path(char * val,int slash)
    {
        int len;
        if (!val||(len=(int)strlen(val))<=0) return false;
        if(slash)slash=1;
        for (int i=len-1;i>=0;i--)
        {
            if(val[i]=='\\'||val[i]=='/')
            {
                val[i+slash]='\0';
                return true;
            }
            else if(val[i]==':')
            {
                val[i+1]='\0';
                return true;
            }
        }
        return false;
    }

    /* Get filename */
    static  int  parse_fname(const char * path,char * buf,int len)
    {
        int ret=0,cur;
        if (!path|| (buf && len<=0) || (cur=(int)strlen(path)) < 0 )return -1;
        else if(!cur)
        {
            if(buf)buf[0]='\0';
            return 0;
        }

        if(path[cur-1]=='\\' || path[cur-1]=='/') cur -= 2;
        else cur--;
        
        for (; cur>=0;cur--)
        {
            if(path[cur]=='\\'||path[cur]=='/'||path[cur]==':')
                break; 
            else 
                ret++;
        }
        
        if(buf && len > ret)
        {
           if(ret > 0) memcpy(buf,path+cur+1,ret); 
           buf[ret]='\0';
        }
        return ret;
    }
    
    static  char *  parse_pdir(char * buf,int len,int slash)
    {
        if(!buf || len <= 0 || __argc <= 0 || !__argv[0] )return 0;
        memset(buf,0,len);
        strncpy(buf,__argv[0],min((int)strlen(__argv[0]),len-1));
        if(!parse_path(buf,slash))
        {
            #if defined(XTL_WINDOWS)
            /*Trying to get current directory instead*/
            DWORD r=::GetCurrentDirectory(len - 4,buf);
            if(r > 1)
            {
                if(slash && buf[r-1] != '\\' )
                {
                    buf[r]='\\';
                    buf[r+1]='\0';
                }
                else if(!slash && buf[r-1] == '\\')
                {
                   buf[r-1]='\0';
                }
            }
            else
            {
                buf[0]='\0';
            }
            #else
                buf[0]='\0';
            #endif
        }        
        return buf;
    }
    
};
/*-------------------------------------------------------------------*/
#endif
