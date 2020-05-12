/*********************************************************************
 * Project : NuoRDS Proxy
 *
 *********************************************************************
 * Description : Proxy server. 
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

#ifndef  __NRD_PROXY_IMPLEMENTAION_H__
#define  __NRD_PROXY_IMPLEMENTAION_H__

#include "nrdpx_version.h"
#include "nrdpx_config.h" 
#include "nrdnb_client.h"
#include "nrdpx_socket.h"

#include <xtl_utils.h>

/********************************************************************/
//COMMON DEFINITIONS

//Default paths

#ifdef NRD_WINDOWS
  #define NRDPX_CFG_PATH ((xtl::parse_pdir_s(1)+"nrdproxyd.cfg").c_str())
  #define NRDPX_LOG_PATH ((xtl::parse_pdir_s(1)+"nrdproxyd.log").c_str())
  #define NRDPX_PID_PATH xtl::snull
#else
  #define NRDPX_CFG_PATH "/etc/nuords/nrdproxyd.cfg"
  #define NRDPX_LOG_PATH "/var/log/nrdproxyd.log"
  #define NRDPX_PID_PATH "/var/run/nrdproxyd.pid"
#endif

//Log output modes 
#define NRDPX_LOG_OUT_FILE      0x00000001 //Write log to file
#define NRDPX_LOG_OUT_CONS      0x00000002 //Write log to stdout/stderr
#define NRDPX_LOG_OUT_SLOG      0x00000004 //Write log to syslog

//Default ports
#define NRDPX_CONN_PORT  3389 
#define NRDPX_INFO_PORT  NRD_BROWSE_PORT

//Daemon modes
#define  NRDPX_DAEMON_MODE_NONE     0 //Do not demonize process
#define  NRDPX_DAEMON_MODE_LITE     1 //Disable stdout and do not exit untill signal received
#define  NRDPX_DAEMON_MODE_FULL     2 //Demonize and detach the process from console

//Server check modes
#define  NRDPX_CHECK_MODE_NONE     0 //Do not poll the server
#define  NRDPX_CHECK_MODE_CONN     1 //Check server availability
#define  NRDPX_CHECK_MODE_INFO     2 //Check availability and resource usage
#define  NRDPX_CHECK_MODE_SYNC     3 //Check all, synchronize limits and port.

//Routing cache modes
#define  NRDPX_CACHE_MODE_NONE     0 //Do not cache routes
#define  NRDPX_CACHE_MODE_TEMP     1 //Cache routes in runtime only
#define  NRDPX_CACHE_MODE_PERM     2 //Save cached routes at restart

//Socket buffer modes
#define  NRDPX_SOCKET_MODE_SYSTEM  0    //Use operating system defaults.
#define  NRDPX_SOCKET_MODE_ALIGN   1    //Align socket buffers with a peer.
#define  NRDPX_SOCKET_MODE_MAXBUF  2    //Use 'max_buffer' for socket buffers. 

//Internal buffering modes
#define  NRDPX_BUFFER_MODE_SYSTEM  0    //Use proxy application defaults.
#define  NRDPX_BUFFER_MODE_ALIGN   1    //Align internal buffers with a socket.
#define  NRDPX_BUFFER_MODE_MAXBUF  2    //Use'max_buffer' for internal buffers.

//Buffer size bounds
#define  NRDPX_BUFFER_SIZE_MIN     NRDPX_STANDARD_BSS 
#define  NRDPX_BUFFER_SIZE_MAX     131072 //128Kb (?)

//Other keywords
#define  NRDPX_ALL_KEY      "all"

/********************************************************************/
//COMMON FUNCTIONS

//Misc
static int         nrdpx_main_proc();
static void        nrdpx_print_help();
static xtl::string nrdpx_real_path(const xtl::string& s);
static int         nrdpx_exit_proc(int err);
static void        nrdpx_check_arguments();
static void        nrdpx_create_daemon();
static bool        nrdpx_load_config();
static xtl::string nrdpx_version_info();
static xtl::string nrdpx_license_info();

//Log
static void        nrdpx_log_init();
extern void        nrdpx_log_write(int type, const char *msg, ...);
static void        nrdpx_log_rotate();

//Cache
static void        nrdpx_cache_load();
static void        nrdpx_cache_save();

//Info
static NRD_BOOL    nrdpx_info_callback(NRD_UINT uEvent,NRD_VOID* pData,NRD_VOID* pParam);

#ifdef NRD_WINDOWS
  static BOOL WINAPI nrdpx_signal_handler(DWORD CEvent);
#else
  static void  nrdpx_signal_handler(int signum);
#endif

#define nrdpx_log nrdpx_log_write

/********************************************************************/

//TODO: Move all object definitions to a separate hpp files.

/********************************************************************/
//SERVER TYPE

class nrdpx_server_t
{
    public:
    
    xtl::string        name;   //Server name
    HNRD_NBINF         info;   //Last info
    xtl::string        host;   //Server host
    int                port;   //Server port
    int                info_port;   //Info protocol port
    time_t             stat_time;   //The next status update time
    time_t             info_time;   //The next info update time
    int                max_slots;   //The maximun number of slots
    int                num_slots;   //The current number of slots
    int                max_weight;  //Maximum server weight
    int                num_faults;  //Failed connection attempts count
    bool               redundant;   //The server is redundant
    int                check_mode;  //The server check mode
    bool               first_check; //The first server check
    
    nrdpx_server_t();
    
};

typedef xtl::vector<nrdpx_server_t *> nrdpx_servers_t;

/********************************************************************/
//CLIENT TYPE

typedef xtl::vector<xtl::inet::cidr_t> nrdpx_cidrs_t;

class nrdpx_client_t
{
    public:
    xtl::string        name;
    nrdpx_cidrs_t      sources;
    int                num_slots;
    int                max_slots;
    
    nrdpx_client_t();
};

typedef xtl::vector<nrdpx_client_t *> nrdpx_clients_t;

//Balancing methods
#define B_METHOD_BYBUSYNESS  0
#define B_METHOD_BYREQUESTS  1

//Balancing metrics
#define B_METRIC_CAP  0x01
#define B_METRIC_MEM  0x02
#define B_METRIC_CPU  0x04

/********************************************************************/
//BALANCER TYPE

class nrdpx_balancer_t
{
    public:
    
    xtl::string  name;
    
    nrdpx_clients_t    clients;
    nrdpx_servers_t    servers;
    int                metrics;
    int                method;
    
    nrdpx_balancer_t();
    
};

typedef xtl::vector<nrdpx_balancer_t *> nrdpx_balancers_t;

/********************************************************************/
//ROUTE TYPE

class nrdpx_route_t
{
    public:
    
    nrdpx_balancer_t*  balancer;  //Used balancer module
    nrdpx_client_t*    client;    //Used client module
    nrdpx_server_t*    server;    //Used server module
    bool               cached;    //Route is cached
    time_t             act_time;  //Last activity time
    int                num_slots; //Number af active slots
    
    nrdpx_route_t();
};

typedef xtl::hash_map<xtl::string, nrdpx_route_t*> nrdpx_routes_t;

/********************************************************************/
//CACHE TYPE

class nrdpx_cache_t
{
    public:
    
    int            mode;
    nrdpx_routes_t routes;
    int            timeout;
    time_t         chktime;
    
    nrdpx_route_t* find(const xtl::string& key);
    bool           add(const xtl::string& key, nrdpx_route_t *route);
    void           remove(const xtl::string& key);
    
    nrdpx_cache_t();
};

/********************************************************************/
//CHANNEL TYPE

class nrdpx_channel_t
{
    public:
    
    size_t             id;
    nrdpx_route_t     *route;
    time_t             conn_time;
    bool               connected;
    
    class buffer_t
    {
        public:
        
        char * data;
        size_t size;
        size_t pos;

        buffer_t();
        ~buffer_t();
    };
    
    struct inputs_t
    {
        buffer_t    source;
        buffer_t    target;
    }inputs;
    
    struct sockets_t
    {
        NRD_SOCKET  source;
        NRD_SOCKET  target;
    }sockets;
    
    void adjust_source();
    void adjust_target();
    void adjust_buffers();
    bool set_connected();
    
    nrdpx_channel_t(NRD_SOCKET src, NRD_SOCKET dst, nrdpx_route_t* rte,  time_t now);
    ~nrdpx_channel_t();
    
};

typedef xtl::vector<nrdpx_channel_t*> nrdpx_channels_t;


/********************************************************************/
//LISTENER TYPE
class nrdpx_listener_t
{
        public:
        xtl::string    host;
        int            port;
        NRD_SOCKET     sock;
       
        nrdpx_listener_t(const xtl::string& h, int p);
        nrdpx_listener_t(const xtl::string& a);

        ~nrdpx_listener_t();
};

typedef xtl::vector<nrdpx_listener_t*> nrdpx_listeners_t;
/********************************************************************/
//PROXY TYPE

class nrdpx_proxy_t
{
    public:
    
    nrdpx_clients_t   clients;
    nrdpx_servers_t   servers;
    nrdpx_balancers_t balancers;
    nrdpx_channels_t  channels;
    nrdpx_listeners_t listeners;
    nrdpx_cache_t     cache;
    
    int               info_port;
    int               max_faults;
    int               max_buffer;
    int               socket_mode;
    int               buffer_mode;
    int               info_period;
    bool              info_enabled;
    int               conn_timeout;
    int               status_period;
   
    
    
    void add_channel(nrdpx_channel_t* chn);
    void del_channel(nrdpx_channel_t* chn);
    NRD_SOCKET fill_fds(fd_set* read_set,fd_set* write_set);
    
    nrdpx_proxy_t();
    ~nrdpx_proxy_t();
};

/********************************************************************/
//PROXY FUNCTIONS
//TODO: Move these functions to appropriated objects

static nrdpx_server_t *  nrdpx_select_server(nrdpx_balancer_t * bal, nrdpx_client_t* cln, nrdpx_servers_t *svrs);
static nrdpx_channel_t*  nrdpx_channel_create(NRD_SOCKET src_sock,time_t now,nrdpx_server_t *nosvr=NULL);
static void        nrdpx_channel_delete(nrdpx_channel_t* chan);
static bool        nrdpx_server_is_active(nrdpx_server_t *svr);
static void        nrdpx_server_activate(nrdpx_server_t *svr);
static void        nrdpx_server_deactivate(nrdpx_server_t *svr);
static int         nrdpx_server_max_weight(nrdpx_server_t *svr);
static int         nrdpx_server_slot_count(nrdpx_server_t *svr);
static int         nrdpx_server_user_count(nrdpx_server_t *svr);
static int         nrdpx_server_slot_limit(nrdpx_server_t *svr);
static int         nrdpx_server_user_limit(nrdpx_server_t *svr); 

/********************************************************************/
#endif
