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

#ifndef __XTL_ROUTE_IMPL_H__
#define __XTL_ROUTE_IMPL_H__

#include "xtl_inet.h"

//IMPORTANT: Just an unsuccesfull expeiment with 
//socket routing options. Never use any of this.

/*-------------------------------------------------------------------*/
/*
namespace xtl
{
    namespace route
    {       
        typedef struct _ipv4_origin_opt_t
        {
              unsigned char data[39]; 

              unsigned char get_code(){ return data[0]; }

              unsigned char get_lenght(){ return data[1]; }

              unsigned char get_offset(){ return data[2]; }

              void set_code(unsigned char v){ data[0] = v; }

              void set_lenght(unsigned char v)  { data[1] = v; }

              void  set_offset(unsigned char v) { data[2] = v; }

              unsigned long* get_addrs() { return (((unsigned long*)&data[3])); }

              _ipv4_origin_opt_t(){
                  ::memset(&data, 0, sizeof(data));
                  set_code(7); set_lenght(sizeof(data)); set_offset(4);//minimal
              }
        }ipv4_origin_opt_t;

      
        typedef struct _ipv6_origin_opt_t
        {
              struct{
                   socklen_t      lenght;   //length in bytes, including this structure
                   int            level;    //originating protocol
                   int            type;     //protocol-specific type
                   unsigned short addr[8];  //origin IPv6 address
              }option;
            
              _ipv6_origin_opt_t(){
                  ::memset(&option, 0, sizeof(option));
                  option.lenght = sizeof(option);
                  option.level = IPPROTO_IPV6; 
                  option.type = 0; 
              }
        }ipv6_origin_opt_t;
       

        static bool get_origin_addr(XTL_SOCKET sock, addr_t* paddr)
        {
          
            if(!get_peer_addr(sock,paddr)) return false;
             
             if(paddr->sa_family == AF_INET){
                  
                  ipv4_origin_opt_t opt;
                  socklen_t         len = sizeof(opt.data);
 
                  int  ret = ::getsockopt(sock, IPPROTO_IP, IP_OPTIONS, (char *)&opt.data, &len);

                  if(ret && len == sizeof(opt.data) && 7 == opt.get_code()){
                      unsigned long* addrs = opt.get_addrs();

                      //TBD: Trace it first
                  }

             }else if(paddr->sa_family == AF_INET6){
                 //TODO: return false;

             }else{
                 return false;
             }
        }

        static bool set_origin_addr(XTL_SOCKET sock, addr_t* paddr)
        {
       
            ipv4_origin_opt_t opt;

            unsigned long* addrs = opt.get_addrs();
            addrs[0] = xtl_inet_ipv4(paddr)->sin_addr.s_addr;
            addrs[1] = 0xFFFFFFFF;
            opt.set_offset(12);

            return !::setsockopt(sock, IPPROTO_IP, IP_OPTIONS, (char *)&opt.data, sizeof(opt.data));
        }
    
    }
};
*/
/*-------------------------------------------------------------------*/
#endif
