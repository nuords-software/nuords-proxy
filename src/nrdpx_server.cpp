/*********************************************************************
 * Project : NuoRDS Proxy
 *
 *********************************************************************
 * Description :  Proxy server. 
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

 
#include "nrdpx_server.h"

#ifdef NRD_WINDOWS
  #pragma warning(disable:4996) 
#endif

#ifdef _DEBUG
   //Use these keys for development only
   //#define NRDPX_TRACE_SOURCE_TARGET 1
   //#define NRDPX_TRACE_TARGET_SOURCE 1
#endif 

/********************************************************************/
//GLOBAL VARIABLES

nrdpx_proxy_t*  __proxy = NULL;
int             __log_out = 0;
int             __log_lvl = LOG_INFO;
bool            __log_app = false;
FILE*           __log_fdd = NULL;
xtl::string     __pid_path = NRDPX_PID_PATH;
xtl::string     __cfg_path = NRDPX_CFG_PATH;
xtl::string     __log_path = NRDPX_LOG_PATH;
bool            __detached = false;
bool            __terminated = false;
int             __daemon_mode = NRDPX_DAEMON_MODE_NONE;

#ifndef NRD_WINDOWS
pid_t           __current_pid = NRD_NOPID;
#endif


/********************************************************************/

//TODO: Move all object implementations to a separate cpp files.

/********************************************************************/
//SERVER TYPE

nrdpx_server_t::nrdpx_server_t():first_check(true), redundant(false),max_slots(100), 
num_slots(0),stat_time(0),info_time(0),check_mode(NRDPX_CHECK_MODE_INFO), num_faults(0), 
max_weight(100),port(NRDPX_CONN_PORT), info_port(NRDPX_INFO_PORT)
{
    memset(&info,0,sizeof(info));
};

/********************************************************************/
//CLIENT TYPE

nrdpx_client_t::nrdpx_client_t():num_slots(0),max_slots(1000)
{
}

/********************************************************************/
//BALANCER TYPE

nrdpx_balancer_t::nrdpx_balancer_t():
method(B_METHOD_BYBUSYNESS), metrics(B_METRIC_CAP)
{
}

/********************************************************************/
//ROUTE TYPE

nrdpx_route_t::nrdpx_route_t():balancer(NULL), 
cached(false), client(NULL),server(NULL),act_time(0),num_slots(0)
{
}

/********************************************************************/
//CACHE TYPE

nrdpx_cache_t::nrdpx_cache_t():mode(NRDPX_CACHE_MODE_TEMP), timeout(60)
{
}

nrdpx_route_t*    nrdpx_cache_t::find(const xtl::string& key)
{
    if(mode <= NRDPX_CACHE_MODE_NONE){
        return NULL;
    }
    
    time_t now; ::time(&now);
    
    if(now > chktime){
        
        chktime = now + 3600/*1 hour*/;//TODO: Check period must be configurable.
        
        xtl::vector<xtl::string> keys;
        
        for(nrdpx_routes_t::iterator i=routes.begin(); i != routes.end();i++){
            
            //NOTE: Do not delete routes with active slots.
            if(!i->second->num_slots && now > (i->second->act_time + timeout)){
                delete i->second; keys.push_back(i->first);
            }
        }
        
        for(xtl::vector<xtl::string>::iterator i=keys.begin();  i != keys.end();i++){
            routes.erase(*i);
        }
        
        nrdpx_log(LOG_DEBUG,"Cache has been cleaned, expired=%d",(int)keys.size());
    }
    
    nrdpx_routes_t::iterator i = routes.find(key);
    
    if(i == routes.end()){
        return NULL;
    }
    
    //NOTE: Do not delete routes with active slots.
    if(!i->second->num_slots && now > (i->second->act_time + timeout)){
        delete i->second; routes.erase(key);
        return NULL;
    }
    
    return i->second;
}

bool    nrdpx_cache_t::add(const xtl::string& key, nrdpx_route_t *route)
{
    if(mode <= NRDPX_CACHE_MODE_NONE){
        return false;
    }
    
    nrdpx_routes_t::iterator i = routes.find(key);
    
    if(i != routes.end()){
        nrdpx_log(LOG_WARN,"Could not add a duplicate route.");
        return false;
    }
    
    routes[key] = route;
    ::time(&route->act_time);
    route->cached=true;
    
    return true;
}

void  nrdpx_cache_t::remove(const xtl::string& key)
{
    nrdpx_routes_t::iterator i = routes.find(key);
    
    //NOTE: Do not delete routes with active slots.
    if(i != routes.end() && !i->second->num_slots){
        delete i->second; routes.erase(key);
    }
}

/********************************************************************/
//CHANNEL TYPE

nrdpx_channel_t::nrdpx_channel_t(NRD_SOCKET src, NRD_SOCKET dst,
nrdpx_route_t* rte,  time_t now):connected(false),conn_time(now)
{
    static size_t   id_count = 0;
    
    if(!(id = ++id_count)) id = 1;

    route=rte;
    
    sockets.source = src;
    sockets.target = dst;

    adjust_source();//Source is already active.

    ::time(&route->act_time); 
    
    //Update number of slots.
    route->num_slots++;
    route->client->num_slots++;
    route->server->num_slots++;
}

nrdpx_channel_t::~nrdpx_channel_t()
{
    nrdpx_socket_close(sockets.source);
    nrdpx_socket_close(sockets.target);
    
    if(connected){
        nrdpx_log(LOG_INFO,"Channel[%u]: Disconnected",id);
    }
    
    route->client->num_slots = xtl::max(0,route->client->num_slots - 1);
    route->server->num_slots = xtl::max(0,route->server->num_slots - 1);
    
    if(!route->cached){
        delete route;
    }else{
        ::time(&route->act_time); //update activity time
        route->num_slots = xtl::max(0,route->num_slots - 1);
    }
    
    nrdpx_log(LOG_INFO,"Channel[%u]: Destroyed",id);
}

void nrdpx_channel_t::adjust_source()
{
    int mode = __proxy->socket_mode;

    if(mode > NRDPX_SOCKET_MODE_SYSTEM){
        nrdpx_socket_adjust_buffers(sockets.source,
            (mode == NRDPX_SOCKET_MODE_ALIGN),__proxy->max_buffer);
    }
}

void nrdpx_channel_t::adjust_target()
{
    int mode = __proxy->socket_mode;

    if(mode > NRDPX_SOCKET_MODE_SYSTEM){
        nrdpx_socket_adjust_buffers(sockets.target,
        (mode == NRDPX_SOCKET_MODE_ALIGN),__proxy->max_buffer);
    }
}

void nrdpx_channel_t::adjust_buffers()
{
    //Adjust internal buffers

    size_t snew; int size=0;

    //Calculate target->source buffer
    
    switch(__proxy->buffer_mode){
        
        case NRDPX_SOCKET_MODE_ALIGN:
            {
                snew = 0;

                if (nrdpx_socket_get_recv_buffer(sockets.target, &size))
                {
                    nrdpx_log(LOG_DEBUG,"Channel[%u]: Target recv buffer, sock=%d, size=%d",id,sockets.target,size);
                    if(size > (int)snew) snew = size;
                }
                
                if (nrdpx_socket_get_send_buffer(sockets.source, &size))
                {
                    nrdpx_log(LOG_DEBUG,"Channel[%u]: Source send buffer, sock=%d, size=%d",id,sockets.source,size);
                    if(size > (int)snew) snew = size;
                }

                if(!snew)  snew = __proxy->max_buffer; //did not work, set max buffer.

            }break;
            
        case NRDPX_SOCKET_MODE_MAXBUF:
            snew = __proxy->max_buffer; //maximum configured buffer
            break;
        default:
            snew = NRDPX_BUFFER_SIZE_MAX; //maximum available buffer
            break;
    }
    
    //Update target->source buffer
    
    if(!inputs.target.data || snew > (int)inputs.target.size)
    {
        inputs.target.size = snew;
        inputs.target.data = (char *)::realloc(inputs.target.data,inputs.target.size);   
    }

    nrdpx_log(LOG_DEBUG,"Channel[%u]: Target->Source buffer, size=%u",id,inputs.target.size);
    

    //Calculate source->target buffer size
    
    switch(__proxy->buffer_mode){
        
        case NRDPX_SOCKET_MODE_ALIGN:
            {
                snew = 0;

                if (nrdpx_socket_get_recv_buffer(sockets.source,&size))
                {
                    nrdpx_log(LOG_DEBUG,"Channel[%u]: Source recv buffer, sock=%d, size=%d",id,sockets.source,size);
                    if(size > (int)snew) snew = size;
                }
                
                if (nrdpx_socket_get_send_buffer(sockets.target, &size))
                {
                    nrdpx_log(LOG_DEBUG,"Channel[%u]: Target send buffer, sock=%d, size=%d",id,sockets.target,size);
                    if(size > (int)snew) snew = size;
                }

                if(!snew)  snew = __proxy->max_buffer; //did not work, set max buffer.

            }break;
            
        case NRDPX_SOCKET_MODE_MAXBUF:
            snew = __proxy->max_buffer; //maximum buffer configured 
            break;
        default:
            snew = NRDPX_BUFFER_SIZE_MAX; //maximum buffer available 
            break;
    }
    
    //Update source->target buffer

    if(!inputs.source.data || snew > inputs.source.size)
    {
        inputs.source.size = snew;
        inputs.source.data = (char *)::realloc(inputs.source.data,inputs.source.size);
    }

    nrdpx_log(LOG_DEBUG,"Channel[%u]: Source->Target buffer, size=%u",id,inputs.source.size);
}

bool nrdpx_channel_t::set_connected()
{
    if(connected) return true;
    adjust_target();
    adjust_buffers();
    connected = true;
    route->server->num_faults = 0;
    nrdpx_log(LOG_INFO,"Channel[%u]: Connected",id);
    return true;
}

nrdpx_channel_t::buffer_t::buffer_t(): 
    data(NULL),size(NRDPX_BUFFER_SIZE_MIN), pos(0)
{
    data = (char*)::malloc(size);
}

nrdpx_channel_t::buffer_t::~buffer_t()
{
    if(data)::free(data);
}

/********************************************************************/
//LISTENER TYPE

nrdpx_listener_t::nrdpx_listener_t(const xtl::string& h, int p):
    sock(NRD_NOSOCK),host(h), port(p)
{
   if(host.empty()) host = XTL_INET_IPV4_ANY;
   if(!port) port = NRDPX_CONN_PORT;
}

nrdpx_listener_t::nrdpx_listener_t(const xtl::string& a):
    sock(NRD_NOSOCK),port(0)
{
   xtl::inet::parse_host_port(a, host, port);
   if(host.empty()) host = XTL_INET_IPV4_ANY;
   if(!port) port = NRDPX_CONN_PORT;
}

nrdpx_listener_t::~nrdpx_listener_t()
{
    nrdpx_socket_close(sock);
}

/********************************************************************/
//PROXY TYPE

nrdpx_proxy_t::nrdpx_proxy_t(): max_faults(2),
info_port(NRDPX_INFO_PORT), info_enabled(true),
max_buffer(NRDPX_BUFFER_SIZE_MAX),status_period(300),
socket_mode(NRDPX_SOCKET_MODE_SYSTEM), info_period(15),
buffer_mode(NRDPX_SOCKET_MODE_SYSTEM), conn_timeout(30){}

nrdpx_proxy_t::~nrdpx_proxy_t()
{
    //xtl::locker _l(&mutex);

    for(size_t i=0; i < listeners.size();i++) delete listeners[i];
    listeners.clear();
    
    for(size_t i=0; i < channels.size();i++) delete channels[i];
    channels.clear();
    
    for(size_t i=0; i < balancers.size();i++) delete balancers[i];
    balancers.clear();
    
    for(size_t i=0; i < servers.size();i++) delete servers[i];
    servers.clear();
    
    for(size_t i=0; i < clients.size();i++) delete clients[i];
    clients.clear(); 
}

void nrdpx_proxy_t::add_channel(nrdpx_channel_t* chn)
{
    if(!chn) return;
    channels.push_back(chn);
    nrdpx_log(LOG_INFO,"Active channels: %u",channels.size());
}

void nrdpx_proxy_t::del_channel(nrdpx_channel_t* chn)
{
    if(!chn) return;
    nrdpx_channels_t::iterator i=channels.begin();
    for(;i != channels.end();i++)
    {
        if(*i == chn){
            channels.erase(i); delete chn;
            nrdpx_log(LOG_INFO,"Active channels: %u",channels.size());
            break;
        }
    }
}

NRD_SOCKET nrdpx_proxy_t::fill_fds(fd_set* read_set,fd_set* write_set)
{
    if(!read_set && !write_set) return 0;
    if(read_set) FD_ZERO(read_set);
    if(write_set) FD_ZERO(write_set);

    NRD_SOCKET ret = NRD_NOSOCK;
    
    if(read_set)
    {
        for(nrdpx_listeners_t::iterator i = 
            listeners.begin(); i != listeners.end();i++){
             FD_SET((*i)->sock,read_set);
             ret = xtl::max((*i)->sock, ret);
        }
    }
    
    for(nrdpx_channels_t::iterator i=
        channels.begin(); i != channels.end();i++)
    {
        if(read_set)
        {
            if((*i)->inputs.source.pos < (*i)->inputs.source.size)
            {
                FD_SET((*i)->sockets.source,read_set);
                ret = xtl::max((*i)->sockets.source,ret);
            }
            
            if((*i)->inputs.target.pos < (*i)->inputs.target.size)
            {
                FD_SET((*i)->sockets.target,read_set);
                ret = xtl::max((*i)->sockets.target,ret);
            }
        }
        
        if(write_set)
        {
            if((*i)->inputs.source.pos > 0)
            {
                FD_SET((*i)->sockets.target,write_set);
                ret = xtl::max((*i)->sockets.target,ret);
            }
            
            if((*i)->inputs.target.pos > 0)
            {
                FD_SET((*i)->sockets.source,write_set);
                ret = xtl::max((*i)->sockets.source,ret);
                
            }
        }
    }

    return ret;
}

/********************************************************************/
//IMPLEMENTATION

#ifdef NRD_WINDOWS
    int main (int argc, char * const argv[]){
#else
    int     __argc = 0;      /* count of cmd line args */
    char ** __argv = 0;      /* pointer to table of cmd line args */
    int main (int argc, char * const argv[]){
      __argc = argc;
      __argv = (char **)argv;
#endif

    #ifdef _DEBUG
    __log_lvl = LOG_DEBUG;
    __log_out = NRDPX_LOG_OUT_CONS;
    #endif

    #ifndef NRD_WINDOWS
      __current_pid = ::getpid();
    #endif
      
    
    /*Parse command line*/
    for (int i=1 ;i < argc ; i++)
    {
        const char  * ar = argv[i];
        
        if(ar == NULL || ar[0]=='\0') continue;
        
        if(!stricmp(ar,"-cf"))
        {
            if(++i < argc ) __cfg_path = xtl::ssafe(argv[i]);
        }
        else if(!stricmp(ar,"-la"))
        {
            __log_app = true;
        }
        else if(!stricmp(ar,"-lf"))
        {
            if(++i < argc ){
                __log_out  |= NRDPX_LOG_OUT_FILE;
                __log_path = xtl::ssafe(argv[i]);
            }
        }
        else if(!stricmp(ar,"-lc"))
        {
            __log_out  |= NRDPX_LOG_OUT_CONS;
        }
        else if(!stricmp(ar,"-ls"))
        {
            __log_out  |= NRDPX_LOG_OUT_SLOG;
        }
        else if(!stricmp(ar,"-lv"))
        {
            __log_lvl = LOG_DEBUG; //compatibility see -ll
        }
        else if(!stricmp(ar,"-ll"))
        {
            if(++i < argc ) __log_lvl = xtl::fit(xtl::stoi(argv[i]),0,7);
        }
        else if(!stricmp(ar,"-c") || !stricmp(ar,"--c") ||
        !stricmp(ar,"-w") || !stricmp(ar,"--w"))
        {
            //Simplified version fo GNU recommendation.
            fprintf(stdout,"%s\n",nrdpx_license_info().c_str());
            return 0;
        }
        else if(!strnicmp(ar,"-h",2) || !strnicmp(ar,"--h",3))
        {
            nrdpx_print_help();
            return 0;
        }
        else if(!strnicmp(ar,"-v",2) || !strnicmp(ar,"--v",3))
        {
            fprintf(stdout,"%s\n",nrdpx_version_info().c_str());
            return 0;
        }
        
        #ifndef NRD_WINDOWS
        else if(!stricmp(ar,"-dd"))
        {
            __daemon_mode = NRDPX_DAEMON_MODE_FULL;
        }
        else if(!stricmp(ar,"-dl"))
        {
            __daemon_mode = NRDPX_DAEMON_MODE_LITE;
        }
        else if(!stricmp(ar,"-pf"))
        {
            if(++i < argc && *argv[i] != '\0') __pid_path = xtl::ssafe(argv[i]);
        }
        #endif
    }

    //Normalize patches
    __cfg_path = nrdpx_real_path(__cfg_path);
    __log_path = nrdpx_real_path(__log_path);
    __pid_path = nrdpx_real_path(__pid_path);
    
    //Demonize & Initialize
    nrdpx_create_daemon();
    nrdpx_log_init();
    
    //Signal handlers
    #ifndef NRD_WINDOWS
    signal(SIGTERM, nrdpx_signal_handler);
    signal(SIGINT,  nrdpx_signal_handler);
    signal(SIGHUP,  nrdpx_signal_handler);
    signal(SIGALRM, nrdpx_signal_handler);
    #else
        SetConsoleCtrlHandler( (PHANDLER_ROUTINE)nrdpx_signal_handler,TRUE);
    #endif
    
    //Create object
    __proxy = new nrdpx_proxy_t();
    
    //Load configuration
    if(!nrdpx_load_config()) {
        return nrdpx_fail_proc();
    }
    
    if(!nrdpx_socket_init()){
        return nrdpx_fail_proc();  
    }
    
    nrdpx_log(LOG_INFO,"Creating listeners...");
    
    for(size_t i=0 ; i<__proxy->listeners.size() ; i++){
        
        nrdpx_listener_t* l = __proxy->listeners[i];
        
        l->sock = nrdpx_socket_listen(l->host,l->port);
        
        if(l->sock == NRD_NOSOCK){
            __proxy->listeners.remove(i);  i--;
        }
    }
    
    if(__proxy->listeners.empty()){
        nrdpx_log(LOG_INFO,"No active listeners found.");
        return nrdpx_fail_proc(); 
    }
    
    nrdnb_client_t::register_callback(nrdpx_info_callback,NULL);
    
    //Always start control timer
    nrdnb_client_t::start_control_timer(1);//1 second period
    
    if(__proxy->info_enabled){
        
        nrdnb_client_t::set_reconnect_time(300);//5 minutes reconnect timeout
        nrdnb_client_t::set_refresh_time(0);//Do not refresh servers automatically
        nrdnb_client_t::start_info_tracker("",__proxy->info_port);
        
    }else{
        nrdpx_log(LOG_INFO,"Info listener is disabled.");
    }
    
    return  nrdpx_main_proc();
}

int  nrdpx_main_proc()
{
    nrdpx_channel_t *chn;
    time_t          now;
    timeval         tmo;
    int  sel_rc, err_rc;
    NRD_SOCKET new_sd, max_sd;
    fd_set write_set, read_set;
    size_t sent_sz, recv_sz;
    unsigned int slp=0;
    
    while(!__terminated)
    {
        sent_sz = recv_sz = 0;
        
        //Fill fd_sets with an active sockets
        max_sd =  __proxy->fill_fds(&read_set,&write_set);
        
        //NOTE: On linux timeout must be reinitialized each time
        //because time values are being reset after select call.
        tmo.tv_sec = 5; tmo.tv_usec = 0;
        
        //Select sockets for processing

        sel_rc = ::select((int)(max_sd+1),&read_set,&write_set, NULL, &tmo);
        
        if(__terminated) break;
        
        if (sel_rc < 0){
            
            err_rc = xtl::inet::get_last_error();
            
            if(xtl::inet::is_error_fatal(err_rc))
            {
                nrdpx_log(LOG_ERR,"Socket selector failed, error=%s",
                xtl::inet::last_error_string().c_str());
                return nrdpx_fail_proc(); 
            }
        }
        
        //Process listeners and channels
        
        ::time(&now);
       
        if (sel_rc)
        {
            //Accept connections
            for(nrdpx_listeners_t::iterator i = __proxy->listeners.begin();
            i != __proxy->listeners.end();i++){
                
                if((*i)->sock != NRD_NOSOCK && FD_ISSET((*i)->sock, &read_set)){
                    
                    new_sd = nrdpx_socket_accept((*i)->sock);
                    
                    if (new_sd != NRD_NOSOCK){
                        nrdpx_channel_create(new_sd,now);
                    }
                }
            }
        }
        
        //Process channels errors/reading/writing

        for(size_t i=0;i < __proxy->channels.size(); i++)
        {
            chn =  __proxy->channels[i];
            
            //Check for connection errors
            if(!chn->connected)
            {
                err_rc = 0;
                
                if((chn->conn_time + __proxy->conn_timeout) < now){
                    err_rc = -1;
                }else{
                    err_rc = nrdpx_socket_error(chn->sockets.target);
                    if (err_rc && !xtl::inet::is_error_fatal(err_rc,true)){
                        err_rc = 0;
                    }
                }
                
                if(err_rc)
                {
                    if(err_rc == -1){
                        nrdpx_log(LOG_INFO,"Channel[%u]: Connecting timeout",chn->id);
                    }else{
                        nrdpx_log(LOG_INFO,"Channel[%u]: Connecting failed, error=%s",
                        chn->id,xtl::inet::get_error_string(err_rc).c_str());
                    }
                    
                    //Check if proto should be disabled
                    if((++chn->route->server->num_faults) >= __proxy->max_faults)
                    {
                        nrdpx_server_deactivate(chn->route->server);
                        
                        nrdpx_log(LOG_WARN,"Server deactivated, name=%s",
                        chn->route->server->name.c_str());
                    }
                    
                    //Try to connect to another server
                    nrdpx_channel_create(chn->sockets.source,now,chn->route->server);
                    
                    //Destroy failed channel, but do not close source socket
                    chn->sockets.source = NRD_NOSOCK;
                    nrdpx_channel_delete(chn);
                    
                    i--; continue;
                }
            }
            
            if(!sel_rc) continue; //No sockets selected
            
            //Read from source socket

            if(FD_ISSET(chn->sockets.source,&read_set)
               && chn->inputs.source.pos < chn->inputs.source.size)
            {
                err_rc = ::recv(chn->sockets.source, chn->inputs.source.data + chn->inputs.source.pos,
                int(chn->inputs.source.size - chn->inputs.source.pos),0);
                
                if(err_rc <= 0){
                    err_rc = xtl::inet::get_last_error();
                    if (xtl::inet::is_error_fatal(err_rc)){
                        nrdpx_channel_delete(chn);i--; continue;
                    }
                }else{
                    chn->inputs.source.pos += err_rc;
                    recv_sz += err_rc;
                }
                
                #ifdef NRDPX_TRACE_SOURCE_TARGET
                nrdpx_log(LOG_DEBUG,"TRACE: Read from source, size=%d, pos=%u", err_rc, chn->inputs.source.pos);
                #endif
            }
            
            //Read from target socket

            if(FD_ISSET(chn->sockets.target,&read_set)
               && chn->inputs.target.pos < chn->inputs.target.size)
            {
                if(!chn->connected) chn->set_connected();
                
                err_rc = ::recv(chn->sockets.target, chn->inputs.target.data + chn->inputs.target.pos,
                int(chn->inputs.target.size - chn->inputs.target.pos),0);
                
                if(err_rc <= 0){
                    err_rc = xtl::inet::get_last_error();
                    if (xtl::inet::is_error_fatal(err_rc)){
                        nrdpx_channel_delete(chn);i--; continue;
                    }
                }else{
                    chn->inputs.target.pos += err_rc;
                    recv_sz += err_rc;
                }
                
                #ifdef NRDPX_TRACE_TARGET_SOURCE
                nrdpx_log(LOG_DEBUG,"TRACE: Read from target, size=%d, pos=%u", err_rc, chn->inputs.target.pos);
                #endif
            }
            
            //Write to source socket

            if(FD_ISSET(chn->sockets.source,&write_set)
               && chn->inputs.target.pos > 0)
            {
                err_rc = ::send(chn->sockets.source,
                chn->inputs.target.data,int(chn->inputs.target.pos), 0);
                
                if(err_rc <= 0){
                    err_rc = xtl::inet::get_last_error();
                    if (xtl::inet::is_error_fatal(err_rc)){
                        nrdpx_channel_delete(chn);i--; continue;
                    }
                }else if(err_rc != chn->inputs.target.pos){
                    memmove(chn->inputs.target.data, chn->inputs.target.data + err_rc,
                    chn->inputs.target.pos - err_rc);
                    chn->inputs.target.pos -= err_rc;
                    sent_sz += err_rc;
                }else{
                    chn->inputs.target.pos = 0;
                    sent_sz += err_rc;
                }
                
                #ifdef NRDPX_TRACE_TARGET_SOURCE
                nrdpx_log(LOG_DEBUG,"TRACE: Write to source, size=%d, pos=%u", err_rc, chn->inputs.target.pos);
                #endif
            }
            
            //Write to target socket

            if(FD_ISSET(chn->sockets.target,&write_set)
               && chn->inputs.source.pos > 0)
            {
                if(!chn->connected) chn->set_connected();
                
                err_rc = ::send(chn->sockets.target,
                chn->inputs.source.data, int(chn->inputs.source.pos), 0);
                
                if(err_rc <= 0){
                    err_rc = xtl::inet::get_last_error();
                    if (xtl::inet::is_error_fatal(err_rc)){
                        nrdpx_channel_delete(chn);i--; continue;
                    }
                }else if(err_rc != chn->inputs.source.pos){
                    memmove(chn->inputs.source.data, chn->inputs.source.data + err_rc,
                    chn->inputs.source.pos - err_rc);
                    chn->inputs.source.pos -= err_rc;
                    sent_sz += err_rc;
                }else{
                    chn->inputs.source.pos = 0;
                    sent_sz += err_rc;
                }
                
                #ifdef NRDPX_TRACE_SOURCE_TARGET
                nrdpx_log(LOG_DEBUG,"TRACE: Write to target, size=%d, pos=%u", err_rc, chn->inputs.source.pos);
                #endif
            }
            
        }
        
        //Thread balancing

        if(slp < 25) slp++;
        
        if(!sent_sz && !recv_sz){
            xtl::sleep(slp);
        }else if(slp >= 25){
            xtl::sleep(0);
            slp = 0;
        }
    }
    
    return nrdpx_exit_proc(0);
}

void  nrdpx_check_arguments()
{
    __cfg_path = nrdpx_real_path(__cfg_path);
    __log_path = nrdpx_real_path(__log_path);    
    if(__daemon_mode){
    	__pid_path = nrdpx_real_path(__pid_path);
    }
}

bool  nrdpx_server_is_active(nrdpx_server_t *svr)
{
    return (svr != NULL && (svr->info.svc.stat & NRDNBS_ACTIVE) != 0);
}

void  nrdpx_server_activate(nrdpx_server_t *svr)
{
    if(svr){
        svr->info.svc.stat |= NRDNBS_ACTIVE;
    }
}

void  nrdpx_server_deactivate(nrdpx_server_t *svr)
{
    if(svr){
        svr->info.svc.stat &= ~(NRDNBS_ACTIVE);
        svr->info.svc.ccnt = 0;
        svr->info.mem.stat = 0;
        svr->info.cpu.stat = 0;
        svr->num_faults    = 0;
    }
}

int  nrdpx_server_slot_count(nrdpx_server_t *svr)
{
    return !svr ? 0 : svr->num_slots;
}
        
int  nrdpx_server_slot_limit(nrdpx_server_t *svr)
{	
	  return !svr ? 0 : svr->max_slots;
}
        
int  nrdpx_server_user_count(nrdpx_server_t *svr)
{
    return !svr ? 0 : svr->info.svc.ccnt;
}
        
int  nrdpx_server_user_limit(nrdpx_server_t *svr)
{    
    if(!svr) return 0;
    return !svr->info.cmax ? 65535/*default*/: svr->info.cmax; 
}

int  nrdpx_server_max_weight(nrdpx_server_t *svr)
{
    return !svr ? 0 : svr->max_weight;
}

nrdpx_server_t *  nrdpx_select_server(nrdpx_balancer_t* bal, nrdpx_client_t* cln, nrdpx_servers_t *svrs)
{
    if(bal == NULL || cln == NULL || svrs == NULL|| !svrs->size()) return NULL;
    
    nrdpx_server_t* ret = NULL,*svr;
    double scmax,score, slimit,scount;
    int ulimit,ucount,weight;
    bool redt;
    
    for (int r=0;!ret && r < 2;r++)
    {
        redt = !!r; scmax = 0;
    
        for (size_t i=0;i<svrs->size();i++)
        {
            svr = svrs->at(i);
            
            if(svr->redundant != redt) continue;
            
            scount  = nrdpx_server_slot_count(svr);
            slimit  = nrdpx_server_slot_limit(svr);
            ucount  = nrdpx_server_user_count(svr);
            ulimit  = nrdpx_server_user_limit(svr);
            weight  = nrdpx_server_max_weight(svr);
            
            //Doublecheck limits
            if( weight <= 0 || scount >= slimit || ucount >= ulimit){
                continue;
            }
            
            score = (double)weight;
            
            if(bal->metrics & B_METRIC_MEM){
                if(svr->info.mem.stat & NRDNBU_FREE){
                    //Adjust weight by free memory percentage
                    score = ((double)svr->info.mem.free + score)/2;
                }
            }
            
            if(bal->metrics & B_METRIC_CPU){
                if(svr->info.cpu.stat & NRDNBU_FREE){
                    //Adjust weight by free CPU percentage
                    score = ((double)svr->info.cpu.free + score)/2;
                }
            }
            
            if(bal->metrics & B_METRIC_CAP){
                
                if(bal->method == B_METHOD_BYREQUESTS){
                    //byrequests - based on number of active slots
                    score = (score + (100./(scount + 1.)))/2;
                }
                else//default B_METHOD_BYBUSYNESS
                {
                    //bybusyness - based on number of free slots
                    score = (score + (100. * (slimit - scount)/slimit))/2;
                }
            }
            
            nrdpx_log(LOG_DEBUG,
            "Selector: server=%s, weight=%d, slimit=%d, scount=%d, ulimit=%d, ucount=%d, mem=%d%%, cpu=%d%%, score=%.2f",
            svr->name.c_str(), (int)weight,(int)slimit,(int)scount,(int)ulimit,(int)ucount,(int)svr->info.mem.used,
            (int)svr->info.cpu.used,(float)score);
            
            //Select server with highest cost
            if(!ret || score > scmax){
                ret   = svr;
                scmax = score;
            }
        }
    }

    return ret;
}

bool   nrdpx_load_config()
{
    nrdpx_config_t proxy_cfg;
    nrdpx_config_t balancers_cfg;
    nrdpx_config_t clients_cfg;
    nrdpx_config_t servers_cfg;
    
    nrdpx_section_t * sc=NULL,*sc2=NULL;
    const xtl::strings* ss;

    nrdpx_log(LOG_INFO,"Loading configuration...");
    
    if(!proxy_cfg.load_from_file(__cfg_path,"proxy"))
    {
        nrdpx_log(LOG_ERR,"Could not load configuration file");
        return false;
    }
    
    sc = proxy_cfg.first_section();
    
    if(sc == NULL)
    {
        nrdpx_log(LOG_ERR,"Common configuration is empty");
        return false;
    }
  
    //Listeners
    if(!sc->key_exists("listeners")){
        //No listeners, set defaults
        __proxy->listeners.push_back(new nrdpx_listener_t("0.0.0.0",NRDPX_CONN_PORT));
        __proxy->listeners.push_back(new nrdpx_listener_t("[::]",NRDPX_CONN_PORT));
    }else{
        ss =  &sc->get_strings("listeners");
        for (size_t i=0;i<ss->size();i++){
            __proxy->listeners.push_back(new nrdpx_listener_t(ss->at(i)));
        }
    }

    if(__proxy->listeners.empty()){
        nrdpx_log(LOG_ERR,"No listeners configured");
        return false;
    }

    for (size_t i=0;i<__proxy->listeners.size();i++){
         nrdpx_listener_t * l = __proxy->listeners.at(i);
         nrdpx_log(LOG_DEBUG,"listeners[%d] = %s:%d",i, l->host.c_str(),l->port);
    }

    __proxy->info_enabled = sc->get_boolean("info_enabled", __proxy->info_enabled);
    
    //Info tracker
    nrdpx_log(LOG_DEBUG,"info_enabled = %d",(int)__proxy->info_enabled);
    
    if(__proxy->info_enabled)
    {
        __proxy->info_port = sc->get_integer("info_port", __proxy->info_port); 
        
        nrdpx_log(LOG_DEBUG,"info_port = %d",__proxy->info_port);
        
        __proxy->info_period = xtl::max(10,sc->get_integer("info_period",__proxy->info_period));
    
         nrdpx_log(LOG_DEBUG,"info_period = %d",__proxy->info_period);  
    }
    
    //Status check
    __proxy->status_period = xtl::max(60,sc->get_integer("status_period",__proxy->status_period));
    
    nrdpx_log(LOG_DEBUG,"status_period = %d",__proxy->status_period);

    //Cache options
    
    __proxy->cache.timeout = sc->get_integer("cache_timeout", 60);
    
    if(__proxy->cache.timeout != 0 && __proxy->cache.timeout < 5){
        __proxy->cache.timeout = 5; //minimum, if not disabled
    }
    
    nrdpx_log(LOG_DEBUG,"cache_timeout = %d",__proxy->cache.timeout);

    //TODO: Set cache_mode property automatically. 
    //Update tafter impementing NRDPX_CACHE_MODE_PERM

    if(__proxy->cache.timeout > 0){
        __proxy->cache.mode = NRDPX_CACHE_MODE_TEMP;
    }else{
        __proxy->cache.mode = NRDPX_CACHE_MODE_NONE;
    }
    //nrdpx_log(LOG_DEBUG,"cache_mode = %d",__proxy->cache.mode);
    
    //Other properties
    
    __proxy->conn_timeout = xtl::fit(sc->get_integer(
        "conn_timeout",__proxy->conn_timeout), 10, 900);
    
    nrdpx_log(LOG_DEBUG,"conn_timeout = %d",__proxy->conn_timeout);
    
    __proxy->max_faults = xtl::max(
        1,sc->get_integer("max_faults",__proxy->max_faults));
    
    nrdpx_log(LOG_DEBUG,"max_faults = %d",__proxy->max_faults);

    __proxy->socket_mode = xtl::fit(sc->get_integer("socket_mode",
        __proxy->socket_mode), NRDPX_SOCKET_MODE_SYSTEM, NRDPX_SOCKET_MODE_MAXBUF);
    
    nrdpx_log(LOG_DEBUG,"socket_mode = %d",__proxy->socket_mode);

    __proxy->buffer_mode = xtl::fit(sc->get_integer("buffer_mode",
        __proxy->buffer_mode), NRDPX_BUFFER_MODE_SYSTEM, NRDPX_SOCKET_MODE_MAXBUF);
    
    nrdpx_log(LOG_DEBUG,"buffer_mode = %d",__proxy->buffer_mode);

    __proxy->max_buffer = xtl::fit(sc->get_integer("max_buffer",
        __proxy->max_buffer), NRDPX_BUFFER_SIZE_MIN, NRDPX_BUFFER_SIZE_MAX);
    
    nrdpx_log(LOG_DEBUG,"max_buffer = %d",__proxy->max_buffer);
    
    //Read the rest of config sections
    
    servers_cfg.load_from_file(__cfg_path,"server");
    clients_cfg.load_from_file(__cfg_path,"client");
    balancers_cfg.load_from_file(__cfg_path,"balancer");
    
    if(servers_cfg.empty()){
        nrdpx_log(LOG_ERR,"No servers configured");
        return false;
    }
    
    if(clients_cfg.empty()){
        nrdpx_log(LOG_ERR,"No clients configured");
        return false;
    }
    
    if(balancers_cfg.empty()){
        nrdpx_log(LOG_ERR,"No balancers configured");
        return false;
    }
    
    nrdpx_config_t::iterator i; xtl::inet::cidr_t cr;
    
    //Load clients
    
    for(i=clients_cfg.begin();i != clients_cfg.end();i++)
    {
        sc = clients_cfg.get_section(i);

        nrdpx_client_t * c = new nrdpx_client_t;
        
        c->name = sc->get_name();
        
        nrdpx_log(LOG_DEBUG,"%s:",c->name.c_str());
        
        ss =  &sc->get_strings("sources");
        
        for (size_t j=0;j<ss->size();j++)
        {
            if(xtl::inet::parse_cidr_notation(ss->at(j),&cr)){
                c->sources.push_back(cr);
                nrdpx_log(LOG_DEBUG,"  sources[%d] = %s",
                    (int)(c->sources.size() - 1),ss->at(j).c_str());
            }
        }
     
        c->max_slots = xtl::max(0,sc->get_integer("max_slots", c->max_slots));
          
        nrdpx_log(LOG_DEBUG,"  max_slots = %d",c->max_slots);
        
        if(!c->sources.size()){
            nrdpx_log(LOG_WARN,"%s: No sources found, ignored",c->name.c_str());
            delete c; continue;
        }
        
        if(!c->max_slots){
        	  //NOTE: Do not delete the client. It should block listed subnets.
            nrdpx_log(LOG_WARN,"%s: No slots available, blocked",c->name.c_str());
        }
        
        __proxy->clients.push_back(c);
        
        sc->set_data(c); //store temporary for balancers loading
    }
    
    //Load servers
    
    for(i=servers_cfg.begin();i != servers_cfg.end();i++)
    {
        sc = servers_cfg.get_section(i);
        nrdpx_server_t* s = new nrdpx_server_t();
        s->name = sc->get_name();
        
        nrdpx_log(LOG_DEBUG,"%s:",s->name.c_str());

        xtl::inet::parse_host_port(sc->get_string("address"), s->host, s->port);
       
        nrdpx_log(LOG_DEBUG,"  address = %s:%d", s->host.c_str(), s->port);

        s->info.svc.port = s->port; //synchronize with info

        s->check_mode =  xtl::fit(sc->get_integer("check_mode", s->check_mode), 0, 3);
        	
        if(s->check_mode >= NRDPX_CHECK_MODE_INFO){
        	   if(__proxy->info_enabled){
                s->info_port = sc->get_integer("info_port", s->info_port);
                nrdpx_log(LOG_DEBUG,"  info_port = %d",s->info_port);
             }else{
             	  s->check_mode = NRDPX_CHECK_MODE_CONN; //Degrade check mode
             }
        }
        
        nrdpx_log(LOG_DEBUG,"  check_mode = %d",s->check_mode);
        
        s->max_slots = xtl::max(0,sc->get_integer("max_slots", s->max_slots));
      
        nrdpx_log(LOG_DEBUG,"  max_slots = %d",s->max_slots);
        
        s->max_weight = xtl::fit(sc->get_integer("max_weight",s->max_weight), 1, 100);
       
        nrdpx_log(LOG_DEBUG,"  max_weight = %d", s->max_weight);
        
        s->redundant = sc->get_boolean("redundant", false);
       
        nrdpx_log(LOG_DEBUG,"  redundant = %d", s->redundant);
        
        if(s->host.empty() || !s->port)
        {
            nrdpx_log(LOG_ERR,"%s: Address is not valid, ignored",s->name.c_str());
            delete s; continue;
        }
     
        if(!s->max_slots){
            nrdpx_log(LOG_WARN,"%s: No slots available, ignored",s->name.c_str());
            delete s; continue; 
        }
        
        nrdpx_server_activate(s); __proxy->servers.push_back(s);
        
        sc->set_data(s); //store temporary for balancers loading
    }
    
    //Load balancers
    
    for(i=balancers_cfg.begin();i != balancers_cfg.end();i++)
    {
        sc = balancers_cfg.get_section(i);
        nrdpx_balancer_t* b = new nrdpx_balancer_t();
        b->name = sc->get_name();
        
        nrdpx_log(LOG_DEBUG,"%s:",b->name.c_str());
        
        // List of allowed clients. Default is "all".
        
        ss=&sc->get_strings("clients");
 
        if(ss->empty() || !stricmp(ss->at(0).c_str(), NRDPX_ALL_KEY)){
            for(size_t x=0;x<__proxy->clients.size();x++){
                nrdpx_client_t *c=__proxy->clients.at(x);
                b->clients.push_back(c);
                nrdpx_log(LOG_DEBUG,"  clients[%d] = %s",
                    (int)(b->clients.size() - 1),c->name.c_str());
            }
        }else{
            for(size_t x=0;x<ss->size();x++){
                sc2 = clients_cfg.find_section(ss->at(x));
                if(sc2 != NULL){
                    nrdpx_client_t *c=(nrdpx_client_t *)sc2->get_data();
                    if(c != NULL){
                         b->clients.push_back(c);
                         nrdpx_log(LOG_DEBUG,"  clients[%d] = %s",
                         (int)(b->clients.size() - 1),c->name.c_str());
                    }else{
                        nrdpx_log(LOG_WARN,"%s: Could not validate %s, ignored",
                         b->name.c_str(),("client."+ss->at(x)).c_str());
                    }
                }else{
                    nrdpx_log(LOG_WARN,"%s: Could not find %s, ignored",
                    b->name.c_str(),("client."+ss->at(x)).c_str());
                }
            }
        }
        
        if(!b->clients.size()){
            nrdpx_log(LOG_WARN,"%s: No clients found, ignored",b->name.c_str());
            delete b; continue;
        }
        
        // List of target servers. Default is "all".
        
        ss=&sc->get_strings("servers");
    
        if(ss->empty() || !stricmp(ss->at(0).c_str(), NRDPX_ALL_KEY)){
            for(size_t x=0;x< __proxy->servers.size();x++){
                nrdpx_server_t *s=__proxy->servers.at(x);
                b->servers.push_back(s);
                nrdpx_log(LOG_DEBUG,"  servers[%d] = %s",(int)(b->servers.size() - 1),s->name.c_str());
            }
        }else{
            for(size_t x=0;x<ss->size();x++){
                sc2 = servers_cfg.find_section(ss->at(x));
                if(sc2 != NULL){
                    nrdpx_server_t *s=(nrdpx_server_t *)sc2->get_data();
                    if(s != NULL){
                        b->servers.push_back(s);
                        nrdpx_log(LOG_DEBUG,"  servers[%d] = %s",(int)(b->servers.size() - 1),s->name.c_str());
                    }else{
                        nrdpx_log(LOG_WARN,"%s: Could not validate %s, ignored",
                        b->name.c_str(),("server."+ss->at(x)).c_str());
                    }
                }else{
                    nrdpx_log(LOG_WARN,"%s: Could not find %s, ignored",
                    b->name.c_str(),("server."+ss->at(x)).c_str());
                }
            }
        }
        
        if(!b->servers.size()){
            nrdpx_log(LOG_WARN,"%s: No servers found, ignored",b->name.c_str());
            delete b; continue;
        }
  
        if(!stricmp(sc->get_string("method").c_str(),"byrequests")){
            b->method = B_METHOD_BYREQUESTS;
        }else{//default
            b->method = B_METHOD_BYBUSYNESS;
        }
        
        nrdpx_log(LOG_DEBUG,"  method = %d",b->method);
        
        ss = &sc->get_strings("metrics");
        
        b->metrics = 0;
        
        for(size_t m=0;m<ss->size();m++){
            if(!stricmp(ss->at(m).c_str(),"cap")){
                b->metrics |= B_METRIC_CAP;
            }else if(!stricmp(ss->at(m).c_str(),"mem")){
                b->metrics |= B_METRIC_MEM;
            }else if(!stricmp(ss->at(m).c_str(),"cpu")){
                b->metrics |= B_METRIC_CPU;
            }
        }
        
        if(!b->metrics) b->metrics |= B_METRIC_CAP;
        
        nrdpx_log(LOG_DEBUG,"  metrics = 0x%X",b->metrics);
        
        __proxy->balancers.push_back(b);
    }
    
    if(!__proxy->balancers.size()){
        nrdpx_log(LOG_ERR,"No valid balancers found");
        return false;
    }
    
    return true;
}

void nrdpx_channel_delete(nrdpx_channel_t* chn)
{
    if(chn != NULL)
    {
        __proxy->del_channel(chn);
    }
}

nrdpx_channel_t* nrdpx_channel_create(NRD_SOCKET src_sock,time_t now,nrdpx_server_t* nosvr/*=NULL*/)
{
    nrdpx_channel_t   *channel=NULL;
    NRD_SOCKET         svr_sock=-1;
    xtl::string        src_host;
    xtl::inet::addr_t  src_addr;
    xtl::inet::addr_t  svr_addr;
    nrdpx_route_t      *route=NULL;
    nrdpx_servers_t    servers;
    nrdpx_client_t    *client;
    int                error;
    
    if(!xtl::inet::get_peer_addr(src_sock, &src_addr)){
        nrdpx_log(LOG_ERR,"Could not get connection source address");
        goto ret_point;
    }
    
    src_host = xtl::inet::addr_to_host(&src_addr);
    
    if(src_host.empty()){//TODO: Is this check necessary?
        nrdpx_log(LOG_ERR,"Connection source is not valid");
        goto ret_point;
    }
    
    route = __proxy->cache.find(src_host);
    
    if(route && route->server == nosvr){
        //Something is wrong with server, delete cached route.
        __proxy->cache.remove(src_host); route = NULL;
    }
    
    if(route == NULL){
        
        //No cached route found, try to detect one.
        
        route = new nrdpx_route_t();
        
        for(nrdpx_balancers_t::iterator bal=__proxy->balancers.begin();
            bal != __proxy->balancers.end(); bal++)
        {
            route->balancer= *bal; client = NULL;
            //Find a client
            
            for(nrdpx_clients_t::iterator cln=(*bal)->clients.begin();
                cln != (*bal)->clients.end(); cln++)
            {
                if((*cln)->num_slots >= (*cln)->max_slots) continue;
                
                //Find source
                for(nrdpx_cidrs_t::iterator cidr=(*cln)->sources.begin();
                    cidr != (*cln)->sources.end(); cidr++)
                {
                    if(xtl::inet::is_addr_in_cidr(&src_addr, cidr)){
                        client = *cln; break;
                    }
                }
                
                if(client) break;
            }
            
            if(!client) continue;
            
            route->client = client;
            
            for(nrdpx_servers_t::iterator svr=(*bal)->servers.begin();
                svr != (*bal)->servers.end(); svr++)
            {
                if((*svr) != nosvr && nrdpx_server_is_active(*svr)
                && nrdpx_server_slot_count(*svr) < nrdpx_server_slot_limit(*svr)
                    && nrdpx_server_user_count(*svr) < nrdpx_server_user_limit(*svr)){
                    servers.push_back(*svr);
                }
            }
            
            route->server = nrdpx_select_server(route->balancer, route->client, &servers);
            
            if(route->server) break;
        }
    }
    
    if(!route->balancer)
    {
        nrdpx_log(LOG_ERR,"No balancer slots available, source=%s",src_host.c_str());
        goto ret_point;
    }
    
    if(!route->client)
    {
        nrdpx_log(LOG_ERR,"No client slots available, source=%s",src_host.c_str());
        goto ret_point;
    }
    
    if(!route->server){
        nrdpx_log(LOG_ERR,"No server slots available, source=%s",src_host.c_str());
        goto ret_point;
    }
    
    if (!route->server->port){
        nrdpx_log(LOG_ERR,
        "Server port is not valid, server=%s, address=%s:%d, error=%s",
        route->server->name.c_str(), route->server->host.c_str(),
        route->server->port, xtl::inet::last_error_string().c_str());
        goto ret_point;
    }
    
    if (!xtl::inet::host_to_addr(route->server->host,&svr_addr)){
        nrdpx_log(LOG_ERR,
        "Could not interpret host, server=%s, address=%s:%d, error=%s",
        route->server->name.c_str(), route->server->host.c_str(),
        route->server->port, xtl::inet::last_error_string().c_str());
        goto ret_point;
    }
    
    xtl::inet::set_addr_port(&svr_addr, route->server->port);
    
    svr_sock = nrdpx_socket_create(svr_addr.sa_family, false);
    
    if (svr_sock == NRD_NOSOCK)
    {
        nrdpx_log(LOG_ERR,
        "Could not create socket, server=%s, address=%s:%d, error=%s",
        route->server->name.c_str(), route->server->host.c_str(),
        route->server->port, xtl::inet::last_error_string().c_str());
        goto ret_point;
    }
    
    if (0 != nrdpx_socket_connect(svr_sock,&svr_addr)){
    	      	
        error = xtl::inet::get_last_error();

        if(xtl::inet::is_error_fatal(error,true)){
            nrdpx_log(LOG_ERR,
            "Could not connect, server=%s, address=%s:%d, error=%s",
            route->server->name.c_str(), route->server->host.c_str(),
            route->server->port,xtl::inet::get_error_string(error).c_str());
            goto ret_point;
        }
    }
    
    channel = new nrdpx_channel_t(src_sock,svr_sock,route,now);
    
    nrdpx_log(LOG_INFO,
    "Channel[%u]: Created, source=%s, target=%s:%d", 
    channel->id,src_host.c_str(),route->server->host.c_str(),route->server->port);
             
    nrdpx_log(LOG_INFO,
    "Channel[%u]: Route, cached=%d, redundant=%d, path=%s->%s",
     channel->id,(int)route->cached, (int)route->server->redundant,
     route->client->name.c_str(),route->server->name.c_str());
    
    ret_point:
    
    if(channel == NULL){
        nrdpx_socket_close(svr_sock);
        nrdpx_socket_close(src_sock);
        if(route){
            if(route->cached){
                __proxy->cache.remove(src_host);
            }else{
                delete route;
            }
        }
    }else{
        if(!route->cached){
            __proxy->cache.add(src_host,route);
        }
        __proxy->add_channel(channel);
    }
    
    return channel;
}

bool  nrdpx_check_server(const xtl::string& host, int port, bool active/*=false*/)
{
    
    static xtl::mutex _check_mutex;
    static NRD_SOCKET _check_socket = NRD_NOSOCK;
    
    {
        xtl::locker _l(&_check_mutex);

         //Abort previous check
        if(_check_socket != NRD_NOSOCK){
            nrdpx_log(LOG_DEBUG,"Checker: Last check has timed out");
            nrdpx_socket_close(_check_socket);
        }
    }
    
    if(!port || host.empty()){
        return false;
    }
    
    nrdpx_log(LOG_DEBUG,"Checker: Connecting to server, address=%s:%d",host.c_str(), port);
    
    xtl::inet::addr_t addr;
    
    if (!xtl::inet::host_to_addr(host,&addr)){
        //Report error for active server only
        nrdpx_log(active ? LOG_ERR : LOG_DEBUG,
        "Checker: Could not interpret host, address=%s:%d, error=%s",
        host.c_str(), port, xtl::inet::last_error_string().c_str());
        return false;
    }

    {
        xtl::locker _l(&_check_mutex);
        _check_socket = nrdpx_socket_create(addr.sa_family,true);
    }
    
    if (_check_socket == NRD_NOSOCK)
    {
        nrdpx_log(LOG_ERR,
        "Checker: Could not create socket, address=%s:%d, error=%s",
        host.c_str(), port, xtl::inet::last_error_string().c_str());
        return false;
    }
    
    xtl::inet::set_addr_port(&addr, port);
    
    bool ret = !nrdpx_socket_connect(_check_socket,&addr);
    
    if(!ret){
        //Report error for active server only
        nrdpx_log(active ? LOG_ERR : LOG_DEBUG,
        "Checker: Could not connect, address=%s:%d, error=%s",
        host.c_str(), port, xtl::inet::last_error_string().c_str());
    }
    
    {
        xtl::locker _l(&_check_mutex);
        nrdpx_socket_close(_check_socket);
    }
    
    return ret;
}

NRD_BOOL nrdpx_info_callback(NRD_UINT uEvent,NRD_VOID* pData,NRD_VOID* pParam)
{
    
    //Write log message
    HNRD_NBSITEM * item= NULL;
    
    switch(uEvent)
    {
        case NRDNBC_EVT_STARTED:
            nrdpx_log(LOG_INFO,"Tracker: Browser has been started");
            return NRD_TRUE;
        case NRDNBC_EVT_STOPPED:
            nrdpx_log(LOG_INFO,"Tracker: Browser has been stopped");
            return NRD_TRUE;
        case NRDNBC_EVT_RESTART:
            nrdpx_log(LOG_INFO,"Tracker: Browser has been restarted");
            return NRD_TRUE;
        case NRDNBC_EVT_REFRESH:
            nrdpx_log(LOG_DEBUG,"Tracker: Browser has been refreshed");
            return NRD_TRUE;
        case NRDNBC_EVT_SVRLIST:
            //nrdpx_log(LOG_DEBUG,"Tracker: Servers list has been changed");
            return NRD_TRUE;
        case NRDNBC_EVT_ONERROR:
            if(pData == NULL)pData = (NRD_VOID *)"Unknown";
            nrdpx_log(LOG_ERR,"Tracker: %s", pData);
            return NRD_TRUE;
        case NRDNBC_EVT_ONINFO:
            if(pData == NULL)pData = (NRD_VOID *)"Unknown";
            nrdpx_log(LOG_INFO,"Tracker: %s", pData);
            return NRD_TRUE;
        case NRDNBC_EVT_ONTIMER:
            //nrdpx_log(LOG_DEBUG,"Info: Timer event has been received");
            break;
        case NRDNBC_EVT_SVRINFO:
            item = (HNRD_NBSITEM *)pData;
            //nrdpx_log(LOG_DEBUG,"Info: Server info has been received");
            break;
        case NRDNBC_EVT_SVRHERE:
            item = (HNRD_NBSITEM *)pData;
            //nrdpx_log(LOG_DEBUG,"Info: Server went online");
            break;
        case NRDNBC_EVT_SVRLOST:
            item = (HNRD_NBSITEM *)pData;
            //nrdpx_log(LOG_DEBUG,"Info: Server went offline");
            break;
        default:
            break;
    }
    
    //Do not process old or invalid info format
    if(item && item->info.type != NRDNBT_INFO) item = NULL;
    
    HNRD_NBINF binf; time_t now; ::time(&now);
    
    for(nrdpx_servers_t::iterator s=__proxy->servers.begin(); s != __proxy->servers.end(); s++)
    {
        binf = (*s)->info; //store previous info
        
        if(uEvent != NRDNBC_EVT_ONTIMER && item)
        {
            if(!xtl::inet::is_same_host((*s)->host, item->host)) continue;
            
            if((*s)->check_mode >= NRDPX_CHECK_MODE_INFO && !(*s)->first_check)
            {
                //Reset info timer, becasue we get the info
                (*s)->info_time = now + __proxy->info_period;
                
                //WORKAROUND: Process only v3+ info here. See also nrdnb.h.
                if(item->info.svc.pver != NRD_MAKEUINT(0,5)){
                    nrdpx_log(LOG_ERR,"Tracker: Protocol is not compatible, server=%s",(*s)->name.c_str());
                    (*s)->check_mode = NRDPX_CHECK_MODE_CONN; //Degrade mode.
                    continue;
                }
                
                bool active = nrdpx_server_is_active(*s);
                
                if(item->state == NRDNBS_HERE || item->state == NRDNBS_LIST)
                {
                    if(item->info.mem.stat & NRDNBU_USED){
                        if(item->info.mem.stat & NRDNBU_HIGH){
                            if(!((*s)->info.mem.stat & NRDNBU_HIGH)){
                                nrdpx_log(LOG_WARN,"Tracker: High memory usage alert, server=%s, usage=%d%%",
                                (*s)->name.c_str(),(int)item->info.mem.used);
                            }
                        }else if((*s)->info.mem.stat & NRDNBU_HIGH){
                            nrdpx_log(LOG_INFO,"Tracker: Memory usage fell down, server=%s, usage=%d%%",
                            (*s)->name.c_str(),(int)item->info.mem.used);
                        }
                    }
                    
                    if(item->info.cpu.stat & NRDNBU_USED){
                        if(item->info.cpu.stat & NRDNBU_HIGH){
                            if(!((*s)->info.cpu.stat & NRDNBU_HIGH)){
                                nrdpx_log(LOG_WARN,"Tracker: High CPU usage detected, server=%s, usage=%d%%",
                                (*s)->name.c_str(),(int)item->info.cpu.used);
                            }
                        }else if((*s)->info.cpu.stat & NRDNBU_HIGH){
                            nrdpx_log(LOG_INFO,"Tracker: CPU usage fell down, server=%s, usage=%d%%",
                            (*s)->name.c_str(),(int)item->info.cpu.used);
                        }
                    }
                    
                    //Update server info
                    (*s)->info = item->info;
                    
                    if((*s)->check_mode >= NRDPX_CHECK_MODE_SYNC){
                        
                        //Synchronize connections limit
                        if((*s)->info.cmax && (*s)->info.cmax < (*s)->max_slots){
                            
                            (*s)->max_slots = (*s)->info.cmax;
                            
                            nrdpx_log(LOG_INFO,
                            "Tracker: Limit has been changed, server=%s, max_slots=%d",
                            (*s)->name.c_str(),(*s)->max_slots);
                            
                        }
                        
                        //Synchronize server port
                        if((*s)->info.svc.port && (*s)->port != (*s)->info.svc.port){
                            
                            (*s)->port = (*s)->info.svc.port;
                            
                            nrdpx_log(LOG_INFO,
                            "Tracker: Server port has been changed, server=%s, port=%d",
                            (*s)->name.c_str(),(*s)->port);
                        }
                        
                        //IMPORTANT: Do not syncronize num_slots with info.svc.ccnt.
                        //The server returns number of logged in connections, but
                        //num_slots represents number of all existing connections.
                    }
                }
                else if(item->state == NRDNBS_LOST)
                {
                    nrdpx_server_deactivate(*s);
                }
                
                //Log changed services status
                if(active != nrdpx_server_is_active(*s)){
                    
                    active = nrdpx_server_is_active(*s);
                    
                    nrdpx_log(LOG_INFO, "Tracker: Server is %s, name=%s",
                    !active ? "offline" : "online" , (*s)->name.c_str());
                }
            }
        }
        else /*NRDNBC_EVT_ONTIMER*/
        {
            bool active = nrdpx_server_is_active(*s);
            
            //Check servers by connecting to the address
            if((*s)->check_mode >= NRDPX_CHECK_MODE_CONN &&
              ((*s)->first_check || now > ((*s)->stat_time)))
            {
                //TODO: implement "half-connect" SYN scanner, because
                //we should not terrorize the  server with periodical
                //empty connections just to make sure that it is alive.
                
                if(!__terminated)
                {
                    //Check the server
                    if((*s)->first_check || !active || !nrdpx_server_slot_count(*s))
                    {
                        #ifndef NRD_WINDOWS
                        ::alarm(__proxy->conn_timeout); //connection timeout
                        #endif
                        
                        if(nrdpx_check_server((*s)->host, (*s)->port, active)){
                            nrdpx_server_activate(*s);
                        }else{
                            nrdpx_server_deactivate(*s);
                        }
                    }
                }
                
                //Set the next status check time, and update 'now'
                (*s)->stat_time = ::time(&now)+ __proxy->status_period;
            }
            
            //Log changed services status
            if((*s)->first_check || active != nrdpx_server_is_active(*s)){
                
                active = nrdpx_server_is_active(*s);
                
                nrdpx_log(LOG_INFO, "Checker: Server is %s, name=%s",
                !active ? "offline" : "online" , (*s)->name.c_str());
            }
            
            (*s)->first_check = false;
            
            //Request the server info
            if(active && (*s)->check_mode >= NRDPX_CHECK_MODE_INFO && now > (*s)->info_time)
            {
                nrdnb_client_t::request_server_info((*s)->host, (*s)->info_port);
                
                //Set the next info check time, and update 'now'
                (*s)->info_time = ::time(&now) + __proxy->info_period;
            }
            
            nrdpx_log_rotate();
        }
    }
    
    return NRD_TRUE;
}

xtl::string  nrdpx_real_path(const xtl::string& s)
{
	 #ifdef NRD_WINDOWS
	    return s;  //TODO: implement if time
	 #else

      if(s.empty() || s[0] == '/') return s;
      	
      xtl::string ret; char path[PATH_MAX];
 
      path[sizeof(path) -1] = '\0';
      
      if(NULL != ::getcwd(path, sizeof(path) -1)){
          ret = xtl::string(path) + "/" + s;
      }else{
      	  ret = s; 
      }
      
      //NOTE: realpath extracts path only if
      //file already exists
      if(NULL != realpath(ret.c_str(),path)){
    	  ret = path;	
      }

      return ret;
    #endif
}


void  nrdpx_log_init()
{
	//Extend log mode during initialization
    
    int lou = __log_out; 

    __log_out |= NRDPX_LOG_OUT_SLOG | ((!__daemon_mode) ? NRDPX_LOG_OUT_CONS : 0);

    //Open log file if it is configured
    if(!lou || (lou & NRDPX_LOG_OUT_FILE))
    {
        lou |= NRDPX_LOG_OUT_FILE; 
        if(!__log_path.empty())
        {
            __log_fdd = ::fopen(__log_path.c_str(),__log_app ? "a" : "w" );
        }
    }

    nrdpx_log(LOG_INFO,nrdpx_version_info().c_str());

    if(lou & NRDPX_LOG_OUT_FILE)
    {
        if(!__log_fdd)
        {
            lou &= ~(NRDPX_LOG_OUT_FILE);

            nrdpx_log(LOG_WARN,"Could not open log file: %s",__log_path.c_str());
            
            if(!lou)
            {
                if(__daemon_mode)
                {
                    lou = NRDPX_LOG_OUT_SLOG;
                    nrdpx_log(LOG_WARN,"Demon log has been redirected to syslog");
                }
                else
                {
                    lou = NRDPX_LOG_OUT_CONS;
                    nrdpx_log(LOG_WARN,"Log has been redirected to console");
                }
            }
        }
        else
        {
            nrdpx_log(LOG_INFO,"Log file: %s",__log_path.c_str());
        }
    }
    
    __log_out = lou; //Restore log mode
    
    nrdpx_log(LOG_INFO,"Config file: %s",__cfg_path.c_str());
    
    if(__daemon_mode)
    {
        nrdpx_log(LOG_DEBUG,"PID file: %s",__pid_path.c_str());
    } 
}

void  nrdpx_log_rotate()
{
    
    #ifdef NRD_WINDOWS
        return; //TODO: Implement log rotation
    #else

        if(!(__log_out & NRDPX_LOG_OUT_FILE) || __log_path.empty()){
            return;
        }
        
        //TODO: Implement a better log-rotation
        
        static time_t _log_tmch=0;
        time_t tm; ::time(&tm);
        
        if(tm > _log_tmch){
            
            _log_tmch = tm + 3600; //1 hour
            
            struct stat st;
            
            if(!stat(__log_path.c_str(),&st) && st.st_size > 10485760 /*10Mb*/)
            {
                xtl::string gzip = "/usr/bin/gzip";
                
                if(stat(gzip.c_str(),&st)){
                    gzip = "/bin/gzip";
                    if(stat(gzip.c_str(),&st)){
                        gzip = "";
                    }
                }
                
                if(!gzip.empty()){
                    system((gzip+" -c \""+__log_path+"\" > \""+__log_path+".0.gz\"").c_str());
                    system(("> \""+__log_path+"\"").c_str());
                }
            }
        }
        
    #endif //NRD_WINDOWS
}

void    nrdpx_log_write(int type, const char *msg, ...)
{
    if((type & 0x07) > __log_lvl) return;
    
    va_list args;
    va_start(args, msg);
    
    char        s_text[1024];
    
    s_text[sizeof(s_text)-1] = '\0';
    
    const char * sid = "X";
    
    switch(type){
        case LOG_EMERG:
            sid = "M";
            break;
        case LOG_ALERT:
            sid = "A";
            break;
        case LOG_CRIT:
            sid = "C";
            break;
        case LOG_ERR:
            sid = "E";
            break;
        case LOG_WARNING:
            sid = "W";
            break;
        case LOG_NOTICE:
            sid = "N";
            break;
        case LOG_INFO:
            sid = "I";
            break;
        case LOG_DEBUG:
            sid = "D";
            break;
    }
   
    vsnprintf(s_text,sizeof(s_text)-1, msg, args);
    
    va_end(args);

    #ifndef NRD_WINDOWS
    if(__log_out & NRDPX_LOG_OUT_SLOG)
    {
        syslog(type,"(%d)[%s]: %s",(int)__current_pid,sid,s_text);
    }
    #endif
    
    if(__log_out & (NRDPX_LOG_OUT_FILE | NRDPX_LOG_OUT_CONS))
    {
        time_t      l_time;
        struct tm * t_time;
        char        s_time[32];
        s_time[sizeof(s_time)-1] = '\0';
        
        time(&l_time);
        t_time = localtime(&l_time);
        if(t_time){
            strftime(s_time, sizeof(s_time) - 1,
            "%m.%d.%y %H:%M:%S",t_time);
        }else{
            s_time[0] ='\0';
        }
        
        if((__log_out & NRDPX_LOG_OUT_FILE) && __log_fdd != NULL){
          fprintf(__log_fdd,"%s [%s]: %s\n",s_time,sid,s_text);
          fflush(__log_fdd);
        }
        
        if(__log_out & NRDPX_LOG_OUT_CONS){
           fprintf(stdout,"%s [%s]: %s\n",s_time,sid,s_text);
           fflush(stdout);
        }
    }
}

void nrdpx_release_pid()
{
    #ifndef NRD_WINDOWS
    //Only detached process can release pid.
    if(__detached && !__pid_path.empty())
    {
        ::unlink(__pid_path.c_str());
        __pid_path.clear();
    }
    #endif	
}

bool nrdpx_retain_pid()
{
    bool ret = false;

    #ifndef NRD_WINDOWS
    
    if(!__pid_path.empty())
    {
        FILE * pfd = ::fopen(__pid_path.c_str(),"w");
        if(pfd != NULL)
        {
           ret = (0 < fprintf(pfd,"%d",(int)__current_pid));
           ::fclose(pfd);
        }
    }
    
    if(!ret){
    	nrdpx_log(LOG_ERR,"Could not store pid, file=%s",__pid_path.c_str());
    }
    
    #endif
    
    return ret;
}

pid_t nrdpx_find_pid()
{
    #ifndef NRD_WINDOWS
    
    pid_t pid = NRD_NOPID;

    if(!__pid_path.empty())
    {
        FILE * pfd=fopen(__pid_path.c_str(),"rb");
        
        if(!pfd) return pid;

        fpos_t pos; size_t siz;
        
        if(!fseek(pfd, 0, SEEK_END) && !fgetpos(pfd,&pos) && 
           (siz = *((size_t *)&pos)) > 0 && !fseek(pfd, 0, SEEK_SET))
        {
            char fbuf[32];
            if(siz >= sizeof(fbuf)) siz = sizeof(fbuf)-1;
            if( fread(fbuf, siz, 1,pfd) == 1)
            {
                fbuf[siz] = '\0';
                pid = (pid_t)atoi(fbuf);
            }
        }

        fclose(pfd);
    }

    if(pid != NRD_NOPID && ::kill(pid,0) == -1 
        && errno == ESRCH ) pid = NRD_NOPID;

    return pid;

    #endif
    
    return 0;
}

int nrdpx_exit_proc(int err)
{
    nrdnb_client_t::stop_control_timer();
    nrdnb_client_t::stop_info_tracker();
    
    if(__proxy != NULL) {
        delete __proxy;
        __proxy = NULL;
    }
    
    //Write exit messages to syslog.
    __log_out |= NRDPX_LOG_OUT_SLOG;

    nrdpx_release_pid();
    
    nrdpx_log(LOG_INFO,"The process exited gracefully");
    
    if(__log_fdd != NULL) {
    	 ::fclose(__log_fdd);
       __log_fdd = NULL;
    }
    
    nrdpx_socket_exit();
    
    return err;
}

int  nrdpx_fail_proc()
{
    //Write failure messages to syslog and console.

    __log_out |= NRDPX_LOG_OUT_SLOG | ((!__daemon_mode) ? NRDPX_LOG_OUT_CONS : 0);
    
    nrdpx_log(LOG_ERR,"The proxy is inactive due to a critical failure");
    nrdpx_log(LOG_ERR,"Fix all logged errors and restart this service");
    
    while(!__terminated){
        xtl::sleep(500);
    }

    return nrdpx_exit_proc(1);
}

void  nrdpx_term_proc()
{
    if(!__terminated)
    {
        nrdpx_log(LOG_INFO,"Termination has been requested");
        __terminated=true;
        
        #ifndef NRD_WINDOWS
        ::alarm(15); //15 seconds for graceful exit
        #endif
        
        if(__proxy != NULL)
        {
            for(nrdpx_listeners_t::iterator i = __proxy->listeners.begin();
                i != __proxy->listeners.end();i++){
                nrdpx_socket_close((*i)->sock);
            }
        }
        
        //Abort the server checker
        nrdpx_check_server(xtl::snull,0);
    }
}

#ifdef NRD_WINDOWS
   BOOL WINAPI  nrdpx_signal_handler(DWORD CEvent)
   {
     switch(CEvent)
     {
         case CTRL_C_EVENT:
         case CTRL_BREAK_EVENT:
         case CTRL_CLOSE_EVENT:
         case CTRL_LOGOFF_EVENT:
         case CTRL_SHUTDOWN_EVENT:
         nrdpx_term_proc();
         break;
      }
      return TRUE;
   }
#else
  static void nrdpx_signal_handler(int signum)
  {
     switch(signum)
     { 
        case SIGTERM:
        case SIGINT:
        case SIGHUP:
            nrdpx_term_proc();
        break;
        case SIGALRM:
              if(__terminated)
              {
                 //The process could not exit gracefully

                 nrdpx_release_pid();
               
                 __log_out |= NRDPX_LOG_OUT_SLOG; //Write to syslog.
                 
                 nrdpx_log(LOG_WARN,"The process exited forcefully");

                 ::_exit(0); //TODO: Should it be 1?

              }else{
                 //Abort the server checker
                 nrdpx_check_server(xtl::snull, 0);
              }
        break;
     }
  }
#endif

void   nrdpx_discard_stdio()
{
    #ifndef NRD_WINDOWS
        __log_out &= ~(NRDPX_LOG_OUT_CONS);
        //Redirect stdout&stderr to dev/null
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);
    #endif
}

#define nrdpx_daemon_fail() {nrdpx_log(LOG_WARN,"The process exited forcefully"); ::exit(1); }

void   nrdpx_create_daemon()
{
    if(__daemon_mode == NRDPX_DAEMON_MODE_NONE) return;
    
    #ifndef NRD_WINDOWS
    
        nrdpx_discard_stdio();  //Close standard IO
        
        //Write daemon initialization into syslog.
        int lou = __log_out; __log_out |= NRDPX_LOG_OUT_SLOG;

        if(__daemon_mode == NRDPX_DAEMON_MODE_LITE)
        {
            //Lite daemon mode
            
            ::umask(S_IWGRP | S_IWOTH); //Change permissions

            ::chdir("/"); //Change current directory

            __log_out = lou; //Restore log mode
            
            return; //Do not detach the process.
        }

        //Full daemon mode

        pid_t pid = nrdpx_find_pid();

        if(pid != NRD_NOPID){
            nrdpx_log(LOG_ERR,"The daemon is already launched, pid=%d",pid);
            nrdpx_daemon_fail();
        }

        sync();//syncronize IO
        
        pid = ::fork();
        
        if (pid < 0)
        {
            nrdpx_log(LOG_ERR,"Could not fork the process");
            nrdpx_daemon_fail();
        }
        else if (pid > 0)
        {
            //The parent process.
            xtl::sleep(250); ::exit(0); 
        }
        
        //The child process
        __current_pid = ::getpid();
        
        ::umask(S_IWGRP | S_IWOTH); //Change permissions
        
        //Create a new SID
        if (::setsid() < 0) {
            nrdpx_log(LOG_ERR,"Could not create daemon session");
            nrdpx_daemon_fail();
        }
        
        ::chdir("/"); //Change current directory
        
        //Store pid
        if(!nrdpx_retain_pid()) nrdpx_daemon_fail();
       
        __detached = true; //Mark as detached

        __log_out = lou; //Restore log mode.
   
    #endif
}

void nrdpx_print_help()
{
    ::fprintf(stdout, "%s\n"
    "\nUSAGE: nrdproxyd [options]\n"
    "\nOPTIONS:\n\n"
    "\t-cf <path>\t- Path to a custom config file.\n"
    "\t-lf <path>\t- Path to a custom log file.\n"
    "\t-la\t\t- Append log to an existing file.\n"
    "\t-lc\t\t- Print log messages to console.\n"
    #ifndef NRD_WINDOWS
    "\t-ls\t\t- Print log messages to syslog.\n"
    #endif
    "\t-lv\t\t- Verbose log mode (same as -ll 7).\n"
    "\t-ll <level>\t- A custom log level (0...7).\n"
    #ifndef NRD_WINDOWS
    "\t-dl\t\t- Lite daemon mode (launchd style).\n"
    "\t-dd\t\t- Daemonize, detach and store pid.\n"
    "\t-pf <path>\t-  Path to a custom PID file ('-dd').\n"
    #endif
    "\t-v\t\t- Print version information.\n"
    "\t-c\t\t- Print copyright information.\n"
    "\t-w\t\t- Print warranty information.\n"
    "\t-h\t\t- Print this help message.\n"
    #ifndef NRD_WINDOWS
    "\nNOTES:\n\n"
    "\t- Console output will be canceled by '-dl' or '-dd'.\n"
    #endif
    "\nEXAMPLES:\n\n"
    "Console mode :\n"
    "\n\tnrdproxyd -lc -lf \"mylog.log\" -cf \"myconfig.cfg\"\n"
    #ifndef NRD_WINDOWS
    "\nDaemon mode:\n"
    "\n\tnrdproxyd -dd -lf \"mylog.log\" -cf \"myconfig.cfg\"\n"
    #endif
    "\nDEFAULTS:\n\n"
    "Config file: \"%s\"\n"
    "Log file: \"%s\"\n"
    #ifndef NRD_WINDOWS
    "PID file: \"%s\"\n"
    #endif
    "\n"
    ,nrdpx_version_info().c_str()
    ,NRDPX_CFG_PATH
    ,NRDPX_LOG_PATH
    #ifndef NRD_WINDOWS
    ,NRDPX_PID_PATH
    #endif
   );
};

static xtl::string nrdpx_version_info()
{
	 char ver[128];
     sprintf (ver,"NuoRDS Proxy %u.%u.%08u",VERSION_NUMBER_MAJOR,VERSION_NUMBER_MINOR,VERSION_NUMBER_BUILD);
     return ver;
}

static xtl::string nrdpx_license_info()
{
	return nrdpx_version_info() +  

    "\n\nCopyright 2006-2020, Volodymyr Bykov. All rights reserved.\n\n"

    "This program is free software: you can redistribute it and/or modify\n"
    "it under the terms of the GNU General Public License as published by\n"
    "the Free Software Foundation, either version 3 of the License, or\n"
    "(at your option) any later version.\n\n"

    "This program is distributed in the hope that it will be useful,\n"
    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
    "GNU General Public License for more details.\n\n"

    "You should have received a copy of the GNU General Public License\n"
    "along with this program.  If not, see <http://www.gnu.org/licenses/>.\n\n"

    "As of May 1 2020, a revocable permission to distribute binary copies\n" 
    "of this software without source code and/or copyright notice has been\n"
    "granted to Ozolio Inc (a Hawaii Corporation) by the author.\n";
}
