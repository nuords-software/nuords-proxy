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

#ifndef __XTL_INET_IMPL_H__
#define __XTL_INET_IMPL_H__

#include "xtl_env.h"
#include "xtl_algo.h"
#include "xtl_string.h"

#if defined (XTL_WINDOWS)

    #include <windows.h>
    #include <winsock2.h>
    #include <Ws2tcpip.h>

    typedef SOCKET XTL_SOCKET;

    #ifndef INVALID_SOCKET
      #define INVALID_SOCKET (XTL_SOCKET)(~0)
    #endif

    #ifndef SOCKET_ERROR
      #define SOCKET_ERROR (-1)
    #endif
    
    #define XTL_EAGAIN   WSAEWOULDBLOCK
    #define XTL_EINTR    WSAEINTR
    #define XTL_EINPR    WSAEINPROGRESS
    #define XTL_EALRED   WSAEALREADY
    #define XTL_ETOUT    WSAETIMEDOUT
    #define XTL_ENOHOST  WSAEHOSTUNREACH
  
#else
   
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/ioctl.h>
    #include <netinet/tcp.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <sys/un.h>
    #include <errno.h>
    #include <netdb.h>
   
    #ifndef INVALID_SOCKET
      #define INVALID_SOCKET (-1)
    #endif

    #ifndef SOCKET_ERROR
      #define SOCKET_ERROR (-1)
    #endif  

    #define XTL_EAGAIN  EAGAIN
    #define XTL_EINTR   EINTR
    #define XTL_EINPR   EINPROGRESS
    #define XTL_EALRED  EALREADY
    #define XTL_ETOUT   ETIMEDOUT
    #define XTL_ENOHOST EHOSTUNREACH 

    typedef int XTL_SOCKET;

#endif

/*-------------------------------------------------------------------*/
namespace xtl
{
    #define XTL_INET_IPV4_ANY "0.0.0.0"
    #define XTL_INET_IPV6_ANY "[::]"
    
    #define  xtl_inet_ipv4(_paddr)  ((sockaddr_in*)(_paddr))
    #define  xtl_inet_ipv6(_paddr)  ((sockaddr_in6*)(_paddr))
    
    namespace inet
    {
        //Universal sockaddr structire
        typedef struct addr_t: sockaddr{
            char sa_data2[32]; //?
        }addr_t;
        
        typedef struct cidr_t
        {
            public:
            addr_t  addr;
            int     pref;
        }cidr_t;
        
        
        static int get_last_error()
        {
            #ifdef XTL_WINDOWS
            return WSAGetLastError();
            #else
                return errno;
            #endif
        }
        
        static void  set_last_error(int code){
            #ifdef XTL_WINDOWS
            WSASetLastError(code);
            #else
                errno = code;
            #endif
        }
        
        static bool is_error_fatal(int err,bool inproc=false)
        {
            //TODO: Controversial function for non-blocked.
            return err != XTL_EAGAIN && err != XTL_EINTR &&
            err != XTL_EALRED && (!inproc || err != XTL_EINPR);
        }
        
        static xtl::string get_error_string(int err)
        {
            xtl::string ret = xtl::itos(err);
            
            #if defined (__APPLE_CC__)
            
            char err_buf[1024]={'\0'};
            
            if(!::strerror_r(err,err_buf,sizeof(err_buf) - 1)){
                ret += " (" + xtl::string(err_buf)+ ")";
            }
            
            #elif defined(__GNUC__)
            
            static locale_t __en_locale = ::newlocale(LC_ALL_MASK, "en_US.UTF-8", NULL);
            
            if(__en_locale != (locale_t)0 ){
                
                const char * s = ::strerror_l(err, __en_locale);
                
                if(s && *s != '\0'){
                    ret += " (" + xtl::string(s) + ")";
                }
            }
            
            #endif
            
            return ret;
        }

        static xtl::string last_error_string()
        {
            return get_error_string(get_last_error());
        }
   
        static int guess_addr_family(const xtl::string& host, bool ambiguous=false)
        {
            //RFC3986 describes IPv6 host subcomponent as [IPv6], therefore
            //we require that any host literal that should be interpretted
            //as an IPv6 destination, must be always encolosed into square
            //brackets. Other formats make the host subcomponent ambiguous.
            
            if(host.empty()) return  AF_INET; //default
            
            if(host.at(0) == '[') return  AF_INET6; //explicit
            
            if(ambiguous && host.find(':') != xtl::npos) return  AF_INET6; //ambiguous
            
            return AF_INET;
        }
        
        static xtl::string strip_host_name(const xtl::string& host)
        {
            //see  comments in guess_addr_family()
            size_t len = host.length();
            
            if(len > 1 && host.at(0) == '[' && host.at(len -1) == ']'){
                return host.substr(1,len - 2) ;
            }
            
            return host;
        }
        
        static void  parse_host_port(const xtl::string& addr, xtl::string& host, int& port)
        {
            //see guess_addr_family().
            size_t pos=addr.rfind("]:");  //IPv6
            
            if(pos == xtl::npos){
                pos = addr.rfind(':'); //IPv4
            }else{
                pos += 1;
            }
            
            if(pos == xtl::npos){
                host = xtl::strim(addr);
                port = 0;
            }else{
                host = xtl::strim(addr.substr(0,pos));
                port = xtl::stoi(addr.substr(pos+1));
            }
        }

        static bool  set_addr_port(struct sockaddr* paddr,int port)
        {
            if(paddr->sa_family == AF_INET6){
                xtl_inet_ipv6(paddr)->sin6_port = htons((short)port);
            }else if(paddr->sa_family == AF_INET){
                xtl_inet_ipv4(paddr)->sin_port =  htons((short)port);
            }else{
                return false;
            }
            return true;
        }
        
        static socklen_t  get_addr_length(int family)
        {
            if(family == AF_INET6){
                return (socklen_t)sizeof(sockaddr_in6);
            }else if(family == AF_INET){
                return (socklen_t)sizeof(sockaddr_in);
            }
            return 0;
        }
        
        static socklen_t  get_addr_length(struct sockaddr* paddr)
        {
            return get_addr_length(paddr->sa_family);
        }
        
        static bool  set_addr_any(int family, struct sockaddr* paddr)
        {     
            if(family == AF_INET6){
                
                ::memset(paddr, 0, sizeof(sockaddr_in6));
                
                ::memcpy(&xtl_inet_ipv6(paddr)->sin6_addr, &in6addr_any, sizeof(in6_addr));
                
                xtl_inet_ipv6(paddr)->sin6_family = family;
                
            }else if(family == AF_INET){
                
                ::memset(paddr, 0, sizeof(sockaddr_in));
                
                xtl_inet_ipv4(paddr)->sin_family = family;
                xtl_inet_ipv4(paddr)->sin_addr.s_addr = htonl(INADDR_ANY);
                
                //TODO: Not every OS declares sin_len
                #if defined(XTL_MACOSX) || defined(_SOCKADDR_LEN)
                xtl_inet_ipv4(paddr)->sin_len = sizeof(sockaddr_in);
                #endif
                
            }else{
                return false;
            }
            
            return true;
        }
        
        static bool  host_to_addr(const xtl::string& host, struct sockaddr* paddr, bool ambiguous=false)
        {
            if(host.empty() || !paddr) return false;
            
            bool result = false;
            
            int family = guess_addr_family(host, ambiguous);
            
            xtl::string naked = strip_host_name(host);
            
            if(family  == AF_INET6){
                
                if(!strcmp(host.c_str(), XTL_INET_IPV6_ANY)){
                    result = set_addr_any(family,paddr);
                }else{
                    ::memset(paddr, 0, sizeof(sockaddr_in6));
                    result = !!inet_pton(family,naked.c_str(), &xtl_inet_ipv6(paddr)->sin6_addr);
                    if(result){
                        xtl_inet_ipv6(paddr)->sin6_family = family;
                    }
                }
                
            }else if(family  == AF_INET){
                if(!strcmp(host.c_str(), XTL_INET_IPV4_ANY)){
                    result = set_addr_any(family,paddr);
                }else{
                    ::memset(paddr, 0, sizeof(sockaddr_in));
                    result = !!inet_pton(family,naked.c_str(), &xtl_inet_ipv4(paddr)->sin_addr);
                    if(result){
                        xtl_inet_ipv4(paddr)->sin_family = family;
                    }
                }
            }else{
                return false;
            }
            
            if(!result){
                
                struct addrinfo hints;
                struct addrinfo *ainfo;
                
                ::memset(&hints, 0, sizeof(struct addrinfo));
                
                hints.ai_family = family; //preferred
                
                result = !::getaddrinfo(naked.c_str(), NULL, &hints, &ainfo);
                
                if(!result){
                    hints.ai_family = AF_UNSPEC;
                    //Last resort. Just try to detect any available protocol.
                    result = !::getaddrinfo(naked.c_str(), NULL, &hints, &ainfo);
                }
                
                if(result){
                    
                    result = (ainfo->ai_family == AF_INET || ainfo->ai_family == AF_INET6)
                        && ainfo->ai_addrlen <= (size_t)get_addr_length(ainfo->ai_family);
                    
                    if(result){
                        ::memcpy(paddr, ainfo->ai_addr, ainfo->ai_addrlen);
                    }
                    
                    ::freeaddrinfo(ainfo);
                }
            }
            
            //TODO: Not every OS declares sin_len
            #if defined(XTL_MACOSX) || defined(_SOCKADDR_LEN)
            if(result && family == AF_INET){
                xtl_inet_ipv4(paddr)->sin_len = sizeof(sockaddr_in);
            }
            #endif
            
            if(!result && !get_last_error()){
                set_last_error(XTL_ENOHOST);
            }
            
            return result;
        }
        
        static xtl::string  addr_to_host(struct sockaddr* paddr, bool wrap=true)
        {
            char       buffer[48];
            
            buffer[sizeof(buffer)- 1] = '\0';
            
            xtl::string result;
            
            if(paddr->sa_family == AF_INET6){
                result = xtl::string(xtl::ssafe(inet_ntop(paddr->sa_family,
                &xtl_inet_ipv6(paddr)->sin6_addr, buffer,sizeof(buffer)- 1)));
                if(wrap){
                    result = "[" + result + "]"; //see comments in guess_addr_family()
                }
            }else if(paddr->sa_family == AF_INET){
                result = xtl::ssafe(inet_ntop(paddr->sa_family,
                &xtl_inet_ipv4(paddr)->sin_addr, buffer,sizeof(buffer)- 1));
            }
            
            return result;
        }
        
        static bool  is_same_host(const xtl::string& host1, const xtl::string& host2)
        { 
            if(host1.empty() || host2.empty()) return false;
            
            addr_t addr1,addr2;
            
            if(!host_to_addr(host1, &addr1)) return false;
            
            if(!host_to_addr(host2, &addr2)) return false;
            
            if(addr1.sa_family != addr2.sa_family) return false;
            
            if(addr1.sa_family == AF_INET6){
                return !::memcmp(&xtl_inet_ipv6(&addr1)->sin6_addr,&xtl_inet_ipv6(&addr2)->sin6_addr, sizeof(in6_addr));
            }else if(addr1.sa_family == AF_INET){
                return xtl_inet_ipv4(&addr1)->sin_addr.s_addr == xtl_inet_ipv4(&addr2)->sin_addr.s_addr;
            }
            
            return false;
        }
      
        static bool  parse_cidr_notation(const xtl::string& src, cidr_t* pcidr)
        {
            if(src.empty()) return false;
            
            //RFC4291 describes CIDR format as ipv6-address/prefix-length.
            //Therefore we should include ambiguous results into detection.
            
            size_t pos = src.rfind("/");
            
            if(pos == xtl::npos)
            {
                if(!host_to_addr(src, &pcidr->addr, true)){
                    return false;
                }
                
                if(pcidr->addr.sa_family == AF_INET6){
                    pcidr->pref=128;
                }else if(pcidr->addr.sa_family == AF_INET){
                    pcidr->pref=32;
                }else{
                    return false;
                }
            }
            else
            {
                if(!host_to_addr(src.substr(0,pos), &pcidr->addr, true)){
                    return false;
                }
                
                pcidr->pref=xtl::fit(xtl::stoi(src.substr(pos+1)),
                0, (pcidr->addr.sa_family == AF_INET6) ? 128 : 32);
            }
            
            return true;
        }
        
        static bool  is_addr_in_cidr(struct sockaddr* paddr, cidr_t* pcidr)
        {
            if(paddr->sa_family != pcidr->addr.sa_family) return false;
            
            if(!pcidr->pref) return true;
            
            if(paddr->sa_family == AF_INET6){
                
                unsigned int  *addr1 = (unsigned int*)&xtl_inet_ipv6(paddr)->sin6_addr.s6_addr[0], addr1i;
                unsigned int  *addr2 = (unsigned int*)&xtl_inet_ipv6(&pcidr->addr)->sin6_addr.s6_addr[0], addr2i;
                
                int  left = xtl::fit(128 - pcidr->pref, 0, 128), shift;
                
                for(int i=3; i >= 0 ;i--, left -= 32){
                    
                    shift = xtl::fit(left, 0, 32);
                    
                    addr1i = htonl(addr1[i]); addr2i = htonl(addr2[i]);
                    
                    //NOTE: ( int(x)  >> 32) is undefined behavior in most compilers.
                    //In the current situation (shift == 32) means a "pass-through".
                    if(shift != 32 && (addr1i >> shift) != ( addr2i  >> shift)) return false;
                }
                
                return true;
                
            }else if(paddr->sa_family == AF_INET){
                
                int  shift = xtl::fit(32 - pcidr->pref, 0, 32);
                
                unsigned int  addr1 =  htonl(xtl_inet_ipv4(paddr)->sin_addr.s_addr);
                unsigned int  addr2 =  htonl(xtl_inet_ipv4(&pcidr->addr)->sin_addr.s_addr);
                
                //NOTE: ( int(x)  >> 32) is undefined behavior in most compilers.
                //In the current situation (shift == 32) means a "pass-through".
                return shift == 32  || ( addr1  >> shift) == ( addr2  >> shift);
            }
            
            return false;
        }

        static bool get_peer_addr(XTL_SOCKET sock, addr_t* paddr)
        {
            socklen_t  slen = sizeof(addr_t);
            return !::getpeername(sock, (struct sockaddr *)paddr,&slen);
        }

        static bool get_local_addr(XTL_SOCKET sock, addr_t* paddr)
        {
            socklen_t  slen = sizeof(addr_t);
            return !::getsockname(sock, (struct sockaddr *)paddr,&slen);
        }
    }
};

/*-------------------------------------------------------------------*/
#endif
