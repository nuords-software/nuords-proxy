/*********************************************************************
 * Project : XTL
 *
 *********************************************************************
 * Programmer(s) :  Volodymyr Bykov
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

#ifndef __XTL_STRING_IMPL_H__
#define __XTL_STRING_IMPL_H__

#include "xtl_env.h"
#include "xtl_utils.h"
#include "xtl_vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>

#if defined (XTL_WINDOWS)
    #ifndef snprintf
      #define snprintf _snprintf
    #endif
    #ifndef strdup
      #define strdup _strdup
    #endif
    #ifndef vsnprintf
      #define vsnprintf _vsnprintf
    #endif
#else
    #include <strings.h>
    #ifndef stricmp
     #define stricmp strcasecmp
    #endif
    #ifndef strnicmp
      #define strnicmp strncasecmp
    #endif    
#endif
 
/*-------------------------------------------------------------------*/
namespace xtl
{

    typedef std::basic_string<char>  string;
    typedef short wchr_t;
    typedef std::basic_string<wchr_t> wstring;
    static const char * snull("");
    static const size_t npos = size_t(-1);
	  typedef xtl::vector<string>  strings;

    static const char * ssafe(const char * in)
    {
        return  (!in) ? snull : in;
    }

    static int stoi(const string & num)
    {
        return atoi(num.c_str());
    }

    static long nextol( const string & nex)
    {
        char   *stopstring;
        return strtol(nex.c_str(),&stopstring,16);   
    }

    static string itos(int num)
    {
        char buf[32];buf[0]='\0';
        sprintf(buf,"%d",num);
        return string(buf);
    }
    
    static string utos(unsigned int num)
    {
        char buf[32];buf[0]='\0';
        sprintf(buf,"%u",num);
        return string(buf);
    }
   
    static string ttos(time_t total_secs)
    {
        char buf[32];
        int secs,mins,hours,tmp_sec;
        hours = (int)(total_secs/3600);
        tmp_sec = (int)(total_secs - hours * 3600);
        mins = tmp_sec/60;
        secs = tmp_sec - mins * 60;
        
        sprintf(buf, "%02d:%02d:%02d", hours,mins, secs);
        return string(buf);
    }
    
    static string strim(const string & s)
    {
        string::const_iterator i=s.begin();
        string::const_iterator e=s.end();
        int beg=-1,end=-1,pos = 0;
        while ( i!=e )
        {
            if(beg == -1)
            {
                if((*i)!=' ')
                {
                    beg = end = pos;
                }
            }
            else if((*i)!=' ')
            {
                end = pos;
            }
            ++pos; ++i;
        }
        if( beg != (-1))
            return s.substr(beg, end - beg + 1 );
        else
            return s;        
    }

    static string  parse_path_s(const string & in,int slash)
    {
        string ret=in;
        parse_path((char *)ret.c_str(),slash);
        return string(ret.c_str());
    }
   
    static string  parse_path_s(const string & in,int up,int slash)
    {
        if(up <= 0 || !in.size()) return  in;
        
        string ret = strim(in);
        if(*(ret.end()-1) == '\\'|| *(ret.end()-1) == '/'){ 
            ret = ret.substr(0,ret.size()-1);
        }
        
        for (int i=0;i <(up-1);i++) ret = parse_path_s(ret,0);
        
        return parse_path_s(ret,slash);
    }

    static  string  parse_pdir_s(int slash)
    {
        char ret[1024];ret[0]='\0';
        return string(ssafe(parse_pdir(ret,sizeof(ret)-1, slash) ) );
    }

    static  string  parse_fname_s(const string & path)
    {
        char    ret[1024];
        if(parse_fname(path.c_str(),ret,sizeof(ret)) <= sizeof(ret))
            ret[0] = '\0';
        return string(ret);
    }

    static char * replace_chars(char * buf,char c1,char c2)
    {
        if(buf != NULL)
        {
            char * cb=buf;
            while(*cb != '\0')
            {
                if(*cb == c1)*cb=c2;
                cb++;
            }
        }
        return buf;
    }

    static char * replace_bytes(char * buf,unsigned int pos,
                          unsigned int sz,char c1,char c2)
    {
        if(buf != NULL)
        {
            char * cb=buf+pos;
            for(;sz > 0;sz--)
            {
                if(*cb == c1)*cb=c2;
                cb++;
            }
        }
        return buf;
    }
    
    static char *  to_lower(char * buf)
    {
        if(buf != NULL)
        {
            char * cb=buf;
            while(cb[0] != '\0')
            {
                cb[0]=tolower(cb[0]);
                cb++;
            }
        }
        return buf;
    }
    
    static char *  to_upper(char * buf)
    {
        if(buf != NULL)
        {
            char * cb=buf;
            while(cb[0] != '\0')
            {
                cb[0]=toupper(cb[0]);
                cb++;
            }
        }
        return buf;
    }

    /*Returns BOM for wstring. 0- unknown, 1 - FFFE, 2 - FEFF*/
    static  int  get_bom(const wstring & s)
    {
       if(!s.size()) return 0;
       wchr_t wc=s[0]; 
       if( wc == wchr_t(0xFFFE) || 
          (((char *)&wc)[0]!=0x00 && 
          (((char *)&wc)[1]== 0x00))) 
          return 1;
       else if( wc == wchr_t(0xFEFF) || 
          (((char *)&wc)[0]==0x00 && 
          (((char *)&wc)[1]!=0x00)))  return 2;
       else
           return 0;

    }
    
    /*Returns the valid CRLF symbols*/
    static void crlf_chrs(char *cr,char *lf)
    {
      if(cr) *cr = '\r';
      if(lf) *lf = '\n';
    }
    
    static void crlf_chrs(const wstring & s,wchr_t *cr,wchr_t *lf)
    {
       if(get_bom(s) == 2)
       {
            if(cr) *cr = 0x0D00;
            if(lf) *lf = 0x0A00;
       }
       else
       {
            if(cr) *cr = 0x000D;
            if(lf) *lf = 0x000A;
       }
    }

    /*Base function for LF|CR|CRLF -> LF|CR|CRLF conversion for the char-string*/
    static  string  crlf_to(const string & s, const char * chrs, int cnt)
    {
        if(s.empty()) return s;

        string ret; ret.clear();//hotfix for std bug
        char cr=0,lf=0; crlf_chrs(&cr,&lf);
        string::const_iterator b=s.begin();
        string::const_iterator e=s.end();
        string::const_iterator l=e;
        for (string::const_iterator i = b ; i != e ; i++)
        {
            if( *i == cr || *i == lf)
            {
               if(l != e)
               {
                  ret.append(l,i);
                  l = e; 
               }
               /*In case it is not the end of CRLF
                 then we have end-line here*/
               if( (*i) != lf || i == b || (*(i-1)) != cr)
               {
                   ret.append((string::value_type*)chrs,cnt);
               }
            }
            else if(l == e) l = i;
        }

        if(l != e) ret.append(l,e);

        return ret;
    }

    /*Base function for LF|CR|CRLF -> LF|CR|CRLF conversion for the wide-string*/
    static  wstring  crlf_to(const wstring & s, const wchr_t * wchrs, int cnt)
    {
        if(s.empty()) return s;

        wstring ret;ret.clear();//hotfix for std bug
        wchr_t cr=0,lf=0; crlf_chrs(s,&cr,&lf);
        wstring::const_iterator b=s.begin();
        wstring::const_iterator e=s.end();
        wstring::const_iterator l=e;

        for (wstring::const_iterator i = b ; i != e ; i++)
        {
            if( *i == cr || *i == lf)
            {
               if(l != e)
               {
                  ret.append(l,i);
                  l = e; 
               }
               /*In case it is not the end of CRLF
                 then we have end-line here*/
               if( (*i) != lf || i == b || (*(i-1)) != cr)
               {
                   ret.append((wstring::value_type*)wchrs,cnt);
               }
            }
            else if(l == e) l = i;
        }

        if(l != e) ret.append(l,e);

        return ret;
    }

    /*Converts LF|CR to CRLF*/
    static  string  crlf_to_win(const string & s)
    {
        char  crlf[2]={'\r','\n'};
        return crlf_to(s, crlf, 2);      
    }
    static  wstring  crlf_to_win(const wstring & s)
    {
        wchr_t  crlf[2]; crlf_chrs(s,&crlf[0],&crlf[1]);
        return crlf_to(s, crlf, 2); 
    }

    /*Converts CRLF|LF to CR*/
    static  string  crlf_to_mac(const string & s)
    {
        char  cr='\r';
        return crlf_to(s, &cr, 1);
    }
    static  wstring  crlf_to_mac(const wstring & s)
    {
        wchr_t  cr; crlf_chrs(s,&cr,0);
        return crlf_to(s, &cr, 1);
    }

    /*Converts CRLF|CR to LF*/
    static  string  crlf_to_unix(const string & s)
    {
        char  lf='\n';
        return crlf_to(s, &lf, 1);
    }
    static  wstring  crlf_to_unix(const wstring & s)
    {
        wchr_t  lf; crlf_chrs(s,0,&lf);
        return crlf_to(s, &lf, 1);
    }
    
   

};
/*-------------------------------------------------------------------*/
#endif
