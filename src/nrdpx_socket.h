/*********************************************************************
 * Project : NuoRDS Proxy
 *
 *********************************************************************
 * Programmer(s) :  Volodymyr Bykov
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
 
#ifndef __NRD_PROXY_SOCKET_TOOLS_H__
#define __NRD_PROXY_SOCKET_TOOLS_H__

#include <nrdtype.h>
#include <xtl_inet.h>

//Standard MTU and optimistic data-segment size (See RFC 879) 
//MSS = MTU - sizeof(TCPHDR) - sizeof(IPHDR) = 1500 - 20 - 20
#define  NRDPX_STANDARD_MTU     1500
#define  NRDPX_STANDARD_MSS     (NRDPX_STANDARD_MTU  - 40)
#define  NRDPX_STANDARD_BSS     (NRDPX_STANDARD_MSS * 6)

extern bool        nrdpx_socket_init();
extern void        nrdpx_socket_exit();
extern int         nrdpx_socket_error(NRD_SOCKET sock);
extern NRD_SOCKET  nrdpx_socket_create(int family, bool blocked=false);
extern int         nrdpx_socket_connect(NRD_SOCKET sock, struct sockaddr* paddr);
extern void        nrdpx_socket_close(NRD_SOCKET& sock);
extern NRD_SOCKET  nrdpx_socket_listen(xtl::string& host,int port);
extern NRD_SOCKET  nrdpx_socket_accept(NRD_SOCKET sock);

extern bool        nrdpx_socket_adjust_buffers(NRD_SOCKET sock,bool align, int limit);
extern bool        nrdpx_socket_get_send_buffer(NRD_SOCKET sock,int* psize);
extern bool        nrdpx_socket_get_recv_buffer(NRD_SOCKET sock,int* psize);
extern bool        nrdpx_socket_set_send_buffer(NRD_SOCKET sock,int size);
extern bool        nrdpx_socket_set_recv_buffer(NRD_SOCKET sock,int size);

#endif

