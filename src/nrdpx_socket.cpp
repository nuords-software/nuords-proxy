/*********************************************************************
 * Project : NuoRDS Proxy
 *
 *********************************************************************
 * Description : Socket tools
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
 
#include "nrdpx_socket.h"

/********************************************************************/
//FORWARD DECLARATIONS
//TODO: Workaround. Remove forward declarations this after refactoring

#ifndef nrdpx_log
   #define nrdpx_log nrdpx_log_write
#endif
extern void        nrdpx_log_write(int type, const char *msg, ...);
extern int         nrdpx_proxy_max_buffer();

/********************************************************************/
static bool __nrdpx_socket_status = false;

bool  nrdpx_socket_init()
{
    if(!__nrdpx_socket_status){
        
        #if defined (NRD_WINDOWS)
            // Initialise win sockets
            unsigned short version = MAKEWORD(2, 0);
            WSADATA data;
        
            if (0 != WSAStartup(version, &data)){
                nrdpx_log(LOG_ERR,"Could not initialize socket subsystem.");
                return false;
            }
        #else
            signal(SIGPIPE, SIG_IGN); //disable SIGPIPE
        #endif
        
        __nrdpx_socket_status = true;
    }
    return true;
}

void   nrdpx_socket_exit()
{
	if (__nrdpx_socket_status)
  {
     __nrdpx_socket_status =  false;
     #if defined (NRD_WINDOWS)
          WSACleanup();
     #endif
  }
}


int   nrdpx_socket_error(NRD_SOCKET sock)
{
   int  opt=-1; socklen_t osz=sizeof(opt);
   ::getsockopt(sock, SOL_SOCKET, SO_ERROR,(char *)&opt,&osz);
   return opt;
}

void   nrdpx_socket_close(NRD_SOCKET& sock)
{
   if(sock != NRD_NOSOCK ){
       ::shutdown(sock, SD_BOTH);//important for linux
       ::closesocket(sock);
       nrdpx_log(LOG_DEBUG,"Socket has been closed, sock=%d",sock);
       sock = NRD_NOSOCK;
   }	
}

NRD_SOCKET nrdpx_socket_create(int family, bool blocked/*=false*/)
{
      NRD_SOCKET sock = ::socket(family, SOCK_STREAM, 0);
      
      if(sock == NRD_NOSOCK) return sock;
      
      int  opt=1;
      
      if(!blocked){
         if(0 > ioctlsocket(sock, FIONBIO,(u_long*)&opt)){
            nrdpx_socket_close(sock);
            return -1;
         }
      }
      
      nrdpx_log(LOG_DEBUG,"Socket has been created, sock=%d",sock);
      
      if(0 > ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,sizeof(opt))){
          nrdpx_log(LOG_DEBUG,"Could not set SO_REUSEADDR, sock=%d",sock);    
      }
    
      if(0 > ::setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&opt,sizeof(opt))){
         nrdpx_log(LOG_DEBUG,"Could not set TCP_NODELAY, sock=%d",sock);      
      }

      #if defined (IPV6_V6ONLY)
      if(family == AF_INET6){
          //Important for linux, otherwise mapped IPv4 prevents usage of the same port.
          if(0 > ::setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&opt,sizeof(opt))){
              nrdpx_log(LOG_DEBUG,"Could not set IPV6_V6ONLY, sock=%d",sock);      
          }
      }
      #endif
     
      return sock;
}

NRD_SOCKET  nrdpx_socket_accept(NRD_SOCKET sock)
{
    NRD_SOCKET ret = ::accept(sock, NULL, NULL);
    
    if (ret == NRD_NOSOCK){
        
        int err = xtl::inet::get_last_error();
        
        if (xtl::inet::is_error_fatal(err )){
            nrdpx_log(LOG_ERR,"Could not accept connection, error=%s",
            xtl::inet::get_error_string(err).c_str());
        }
        
    }else{
        nrdpx_log(LOG_DEBUG,"Connection accepted, socket=%d",ret);
        //TCP_NODELAY on a listening socket is inherited by accepted sockets
        /*
        int  opt=1;
        if(0 > ::setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&opt,sizeof(opt))){
            nrdpx_log(LOG_DEBUG,"Could not set TCP_NODELAY, sock=%d",sock);
        }
        */
    }
    
    return ret;
}

int nrdpx_socket_connect(NRD_SOCKET sock, struct sockaddr* paddr)
{
    if(sock == NRD_NOSOCK || paddr == NULL) return -1;

    return ::connect(sock, paddr,xtl::inet::get_addr_length(paddr));
}

NRD_SOCKET nrdpx_socket_listen(xtl::string& host,int port)
{
    xtl::inet::addr_t  addr;
 
    if(!xtl::inet::host_to_addr(host, &addr)) {

         nrdpx_log(LOG_WARN,"Could not interpret address, host=%s, error=%s",
                 host.c_str(), xtl::inet::last_error_string().c_str());
         int  family = xtl::inet::guess_addr_family(host);
         host = family == AF_INET6 ? XTL_INET_IPV6_ANY : XTL_INET_IPV4_ANY;
         xtl::inet::set_addr_any(family, &addr);
         nrdpx_log(LOG_WARN,"Listener has been switched to default, host=%s",host.c_str());
    }

    NRD_SOCKET sock = nrdpx_socket_create(addr.sa_family);
    
    if (sock == NRD_NOSOCK)
    {
        nrdpx_log(LOG_ERR,"Could not create proxy socket, error=%s",
                  xtl::inet::last_error_string().c_str());
        return sock;
    }

    xtl::inet::set_addr_port(&addr, port);

    nrdpx_log(LOG_DEBUG,"Binding socket, address=%s:%d, sock=%d",host.c_str(),port,sock);
    
    if (0 > ::bind(sock,&addr, xtl::inet::get_addr_length(&addr)))
    {
        nrdpx_log(LOG_ERR,"Could not bind socket, address=%s:%d, error=%s",
        host.c_str(),port,xtl::inet::last_error_string().c_str());
        nrdpx_socket_close(sock);
        return NRD_NOSOCK;
    }
    
    if (0 > ::listen(sock, 32))
    {
        nrdpx_log(LOG_ERR,"Could not start listening, address=%s:%d, error=%s",
        host.c_str(),port,xtl::inet::last_error_string().c_str());
        nrdpx_socket_close(sock);
        return NRD_NOSOCK;
    }
    
    nrdpx_log(LOG_INFO,"Listener has been created, address=%s:%d, sock=%d", host.c_str(),port,sock);
    
    //nrdpx_socket_adjust_buffers(sock);
  
    return sock;
}

static int nrdpx_socket_calc_buffer_size(int mss,int bss,int limit)
{
    if(mss < (NRDPX_STANDARD_MSS/3) || bss < (NRDPX_STANDARD_BSS/3) || (bss / mss) < 4){
        return bss;  //Let system optimize sockets with weird initial values
    }
    
    //Apply limit and align it with a segment size.
    int ret = (xtl::min(bss,limit)/mss)  * mss;
    
    //Check the final buffer size
    if(ret < (mss * 6)){
        ret = xtl::min(bss,mss * 6);
    }
    
    return ret;
}

bool        nrdpx_socket_adjust_buffers(NRD_SOCKET sock,bool align, int limit)
{
    if(sock == NRD_NOSOCK) return false;
    
    if(!align){
        
        //No need to align, push to limit.
        if(!nrdpx_socket_set_send_buffer(sock, limit)){
            nrdpx_log(LOG_DEBUG,"Could not set send buffer size, sock=%d",sock);
            return false;
        }
        
        if(!nrdpx_socket_set_recv_buffer(sock, limit)){
            nrdpx_log(LOG_DEBUG,"Could not set recv buffer size, sock=%d",sock);
            return false;
        }
        
        return true;
    }
    
   //Align buffers with max segment size.

    #ifdef TCP_MAXSEG
    
    int  opt=1; socklen_t osz = sizeof(opt);
    
    if (!::getsockopt(sock, IPPROTO_TCP, TCP_MAXSEG,(char *)&opt,&osz) && opt > 0)
    {
        int mss = opt;
        
        if(nrdpx_socket_get_send_buffer(sock, &opt)){
            
            int bss = opt; opt = nrdpx_socket_calc_buffer_size(mss,bss,limit);
            
            nrdpx_log(LOG_DEBUG,"Aligning send buffer, sock=%d, mss=%d, old=%d, new=%d",sock,mss,bss,opt);
            
            if(!nrdpx_socket_set_send_buffer(sock, opt)){
                nrdpx_log(LOG_DEBUG,"Could not set send buffer size, sock=%d",sock);
            }
            
        }else{
            nrdpx_log(LOG_DEBUG,"Could not get send buffer size, sock=%d",sock);
        }
        
        if(nrdpx_socket_get_recv_buffer(sock, &opt)){
            
            int bss = opt; opt = nrdpx_socket_calc_buffer_size(mss,bss,limit);
            
            nrdpx_log(LOG_DEBUG,"Aligning recv buffer, sock=%d, mss=%d, old=%d, new=%d",sock,mss,bss,opt);
            
            if(!nrdpx_socket_set_recv_buffer(sock, opt)){
                nrdpx_log(LOG_DEBUG,"Could not set recv buffer size, sock=%d",sock);
            }
            
        }else{
            nrdpx_log(LOG_DEBUG,"Could not get recv buffer size, sock=%d",sock);
        }
        
    }else{
        nrdpx_log(LOG_DEBUG,"Could not get TCP_MAXSEG, sock=%d",sock);
    }
    
    #endif //TCP_MAXSEG
    
    return true;
}

bool nrdpx_socket_get_send_buffer(NRD_SOCKET sock,int* psize)
{
    socklen_t osize = sizeof(int); *psize = 0;
    
    if(::getsockopt(sock, SOL_SOCKET, SO_SNDBUF,(char *)psize,&osize) || (*psize) <= 0){
        return false;
    }
    
    /*
    IMPORTANT:  http://man7.org/linux/man-pages/man7/socket.7.html
    ===============================================================
    SO_SNDBUF
    Sets or gets the maximum socket send buffer in bytes. The
    kernel doubles this value (to allow space for bookkeeping
    overhead) when it is set using setsockopt(2), and this doubled
    value is returned by getsockopt(2).
    ===============================================================
    This is obviously a very confusung behavior. If we will return a
    doubled buffer, then nrdpx_socket_calc_buff may work incorrectly.
    Moreover, any custom buffer size we decide to configure will be
    doubled by a linux core. The only solution is to return a half.
    */
    
    #ifdef NRD_LINUX
        (*psize) /= 2;
    #endif
    
    return true;
}

bool  nrdpx_socket_get_recv_buffer(NRD_SOCKET sock,int* psize)
{
    socklen_t osize = sizeof(int); *psize = 0;
    
    if(::getsockopt(sock, SOL_SOCKET, SO_RCVBUF,(char *)psize,&osize) || (*psize) <= 0){
        return false;
    }
    
    /*
    IMPORTANT:  http://man7.org/linux/man-pages/man7/socket.7.html
    ===============================================================
    SO_RCVBUF
    Sets or gets the maximum socket receive buffer in bytes.  The
    kernel doubles this value (to allow space for bookkeeping
    overhead) when it is set using setsockopt(2), and this doubled
    value is returned by getsockopt(2).
    ===============================================================
    This is obviously a very confusung behavior. If we will return a
    doubled buffer, then nrdpx_socket_calc_buff may work incorrectly.
    Moreover, any custom buffer size we decide to configure will be
    doubled by a linux core. The only solution is to return a half.
    */
    
    #ifdef NRD_LINUX
        (*psize) /= 2;
    #endif
    
    return true;
}

bool nrdpx_socket_set_send_buffer(NRD_SOCKET sock,int size)
{
    return !::setsockopt(sock, SOL_SOCKET, SO_SNDBUF,(char *)&size,sizeof(size));
}

bool nrdpx_socket_set_recv_buffer(NRD_SOCKET sock,int size)
{
    return !::setsockopt(sock, SOL_SOCKET, SO_RCVBUF,(char *)&size,sizeof(size));
}
