/*********************************************************************
 * Project : NuoRDS Proxy
 *
 *********************************************************************
 * Description : Network browser client
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
 
#include "nrdnb_client.h"
#include <xtl_inet.h>
#include <xtl_algo.h>

nrdnb_client_t  nrdnb_client_t::m_inst;

nrdnb_client_t::control_t::control_t():
xtl::thread(),owner(NULL), period(0){}

nrdnb_client_t::control_t::~control_t()
{
    terminate_and_wait();
}

void nrdnb_client_t::control_t::main()
{
    time_t tm=0,tmr=0,cnt = 0;
    while(!is_terminated())
    {
        xtl::sleep(250);
        
        if(owner)
        {
            //Check servers list each 0.5 sec
            if(owner->is_created() && ++cnt > 1){
                owner->check_svrs();
                cnt = 0;
            }
            
            //Raise timer event if timer is set
            if(period)
            {
                ::time(&tm);
                if(!tmr || tm >= tmr)
                {
                    if(tmr) owner->raise_event(NRDNBC_EVT_ONTIMER,NULL);
                    tmr = tm + period;
                }
            }
        }
    }
}

nrdnb_client_t::nrdnb_client_t():
xtl::thread(),m_port(0),m_sock(NRD_NOSOCK),
m_refr(NRD_FALSE),m_tout(15),m_tref(300)
{
    m_ctrl.owner = this;
}

nrdnb_client_t::~nrdnb_client_t()
{
    stop_control_timer();
    stop_info_tracker();
}

void nrdnb_client_t::main()
{
    HNRD_NBMSG         message;
    HNRD_NBSITEM       item;
    xtl::inet::addr_t  addr;
    socklen_t          alen;
    unsigned long      sleep = 0;
    bool               append;
    
    restart_point:
    
    if(!is_terminated())
    {
        if(m_refr)
        {
            m_refr = NRD_FALSE;
            raise_event(NRDNBC_EVT_REFRESH,NULL);
        }
        else if(sleep)
        {
            while(!is_terminated() && sleep > 100)
            {
                xtl::sleep(100); sleep -= 100;
            }
            raise_event(NRDNBC_EVT_RESTART,NULL);
        }
        else
        {
            raise_event(NRDNBC_EVT_STARTED,NULL);
        }
        
        /* Set delay for restart-on-error*/
        sleep =  m_tout * 1000;
        
        /*Send the initial WHOHERE 3 times
        (checked with 20% of lost packets)*/
        if(is_terminated() || !create_sock()){
            goto  restart_point;
        }
    }
    
    while(!is_terminated())
    {
        alen = sizeof(addr);
        
        if(!recv_msg(&message,&addr,&alen)) goto restart_point;
        
        if(message.msg == NRDNBM_IAMHERE || message.msg == NRDNBM_IAMLOST || message.msg == NRDNBM_RETINFO)
        {
            ::memcpy(&item.info,&message.dat,sizeof(item.info));
            
            item.host = xtl::inet::addr_to_host(&addr);
            item.lsid = item.host + ":" + xtl::itos(item.info.svc.port);
            
            append = true;
            
            {
                xtl::locker _l(&m_smtx);
                for (nrdnb_items_t::iterator i=m_svrs.begin();
                i != m_svrs.end();i++)
                {
                    if(!strcmp(i->lsid.c_str(),item.lsid.c_str()))
                    {
                        if(message.msg == NRDNBM_IAMLOST)
                        {
                            i->state = NRDNBS_LOST;
                            i->info.svc.stat &= ~(NRDNBS_ACTIVE);
                            i->info.svc.ccnt = 0;
                            item = *i;
                        }
                        else if(message.msg == NRDNBM_IAMHERE || message.msg == NRDNBM_RETINFO)
                        {
                            ::time(&item.time); //Set last-activity time
                            
                            if(!strcmp(item.info.name,i->info.name) &&
                            (i->state == NRDNBS_WAIT || i->state == NRDNBS_LIST)){
                                item.state = NRDNBS_LIST; //Just restore LIST state
                            } else{
                                item.state = NRDNBS_HERE; //Mark the server as new
                            }
                        }
                        
                        *i = item; append = false; // do not append known server
                        break;
                    }
                }
                
                if(append)
                {
                    item.state = NRDNBS_HERE;
                    ::time(&item.time); //set last-activity time
                    m_svrs.push_back(item);
                }
            }
            
            if(message.msg == NRDNBM_IAMHERE)
                raise_event(NRDNBC_EVT_SVRHERE,&item);
            else if(message.msg == NRDNBM_IAMLOST)
                raise_event(NRDNBC_EVT_SVRLOST,&item);
            else if(message.msg == NRDNBM_RETINFO)
                raise_event(NRDNBC_EVT_SVRINFO,&item);
        }
    }
    
    {
        xtl::locker _l(&m_smtx);
        m_svrs.clear();
    }
    
    raise_event(NRDNBC_EVT_STOPPED,NULL);
}

void   nrdnb_client_t::close_sock()
{
    XTL_LOCK_PTR ( this );
    
    if(m_sock >=0 )
    {
        ::shutdown(m_sock, SD_BOTH);//linux
        ::closesocket(m_sock);
        m_sock = NRD_NOSOCK;
    }
}

bool   nrdnb_client_t::create_sock()
{
    XTL_LOCK_PTR ( this );
    
    if( m_sock != NRD_NOSOCK) {
        ::shutdown(m_sock, SD_BOTH);//linux
        ::closesocket(m_sock);
        m_sock = NRD_NOSOCK;
    }
    
    xtl::inet::addr_t addr;
    int               family = xtl::inet::guess_addr_family(m_host);
    
    if(!xtl::inet::host_to_addr(m_host, &addr)) {
        raise_event(NRDNBC_EVT_ONERROR,(void *)"Could not interpret host");
        raise_event(NRDNBC_EVT_ONINFO,(void *)"Switching to 'any' address");
        xtl::inet::set_addr_any(family, &addr);
    }
    
    xtl::inet::set_addr_port(&addr, m_port); //local binding port
    
    m_sock = ::socket(addr.sa_family, SOCK_DGRAM, 0);
    
    if(m_sock == NRD_NOSOCK) {
        raise_event(NRDNBC_EVT_ONERROR,(void *)"Could not create socket");
        return false;
    }
    
    if(0 > ::bind(m_sock,&addr, xtl::inet::get_addr_length(&addr)))
    {
        ::closesocket(m_sock);
        m_sock = -1;
        raise_event(NRDNBC_EVT_ONERROR,(void *)"Could not bind socket");
        return false;
    }
    
    return true;
}

bool nrdnb_client_t::recv_len(void* buf,size_t len,struct sockaddr* from,socklen_t* fromlen)
{
    return m_sock != NRD_NOSOCK && len == ::recvfrom(m_sock, (char *)buf, int(len), 0,from,fromlen);
}

bool nrdnb_client_t::send_len(void* buf,size_t len, struct sockaddr* to, socklen_t tolen)
{
    XTL_LOCK_PTR(this);
    return m_sock != NRD_NOSOCK && len == ::sendto(m_sock,(char *)buf,int(len),0,to,tolen);
}

bool nrdnb_client_t::recv_msg(HNRD_NBMSG* pmsg,struct sockaddr* from,socklen_t* fromlen)
{
    return recv_len(pmsg,sizeof(HNRD_NBMSG),from,fromlen) &&
           !memcmp(pmsg->sig,NRD_NBSIG,sizeof(NRD_NBSIG)) &&
           pmsg->msg >= NRDNBM_MIN  && pmsg->msg <= NRDNBM_MAX;
}

bool nrdnb_client_t::send_getinfo(const xtl::string& host,NRD_UINT port/*=0*/,int times /*= 1*/)
{
    if(m_sock == NRD_NOSOCK) return false;
    
    if(times <= 0) times = 1; //default
    if(!port) port = NRD_BROWSE_PORT; // default
    
    //TODO: Remove this code after impementing info protocol IPv6
    if(AF_INET6 == xtl::inet::guess_addr_family(m_inst.m_host)){
        m_inst.raise_event(NRDNBC_EVT_ONERROR,(void *)"Info IPv6 is not supported");
        return false;
    }
    
    xtl::inet::addr_t addr;
    
    if(!xtl::inet::host_to_addr(host, &addr)) {
        raise_event(NRDNBC_EVT_ONERROR,(void *)"Server host is not valid");
        return false;
    }
    
    xtl::inet::set_addr_port(&addr,port); //remote info port
    
    HNRD_NBMSG msg;
    memcpy(msg.sig,NRD_NBSIG,sizeof(msg.sig));
    msg.msg = NRDNBM_GETINFO;
    memset(&msg.dat,0,sizeof(msg.dat));
    
    bool ret = false;
    
    socklen_t  alen = xtl::inet::get_addr_length(&addr);
    
    for (int i=0;i<times;i++)
    {
        if(i) xtl::sleep(1);
        ret |= send_len(&msg,sizeof(msg),&addr,alen);
    }
    
    if(!ret) {
        raise_event(NRDNBC_EVT_ONERROR,(void *)"Could not send GETINFO");
    }
    return ret;
}

void  nrdnb_client_t::refresh_svrs(bool lostw/*=false*/)
{
    if(!is_created()) return;
    
    {
        xtl::locker _l(&m_smtx);
        /* 1. Set the LOST state for the servers that still WAIT.
        2. Set the WAIT state for the servers that in the LIST. */
        
        time_t tmm; time(&tmm);

        for (nrdnb_items_t::iterator i=m_svrs.begin(); i != m_svrs.end();i++)
        {
            i->time=tmm; /*Update activity time*/
            if(lostw && i->state == NRDNBS_WAIT)
                i->state = NRDNBS_LOST;
            else if(i->state == NRDNBS_LIST)
                i->state = NRDNBS_WAIT;
        }
    }
    m_refr = NRD_TRUE;
    close_sock();
}

void  nrdnb_client_t::check_svrs()
{
    bool   evt=false;
    bool   rfr=false;
    
    time_t tmm; time(&tmm);
    {
        xtl::locker _l(&m_smtx);

        for (size_t i=0; i < m_svrs.size();i++)
        {
            if(m_svrs[i].state == NRDNBS_LOST)
            {
                evt = true;
                m_svrs.erase(m_svrs.begin() + i);
                i--;
            }
            else
            {
                if(m_svrs[i].state == NRDNBS_HERE)
                {
                    evt = true;
                    m_svrs[i].state = NRDNBS_LIST;
                }
                
                if(m_tref > 0 )
                {
                    /*Refresh if a listed server is
                    inactive more than refresh time*/
                    rfr= (rfr || ((NRD_UINT)(tmm - m_svrs[i].time)) > m_tref);
                }
            }
        }
        
        if(evt) raise_event(NRDNBC_EVT_SVRLIST,&m_svrs);
    }
    
    if(rfr) refresh_svrs(true);
}

bool   nrdnb_client_t::raise_event(NRD_UINT event, void * data)
{
    xtl::locker _l(&m_cmtx);
    
    bool ret=false;

    for (callbacks_t::iterator i=m_cbcs.begin(); i != m_cbcs.end();i++)
    {
        ret |= (NRD_FALSE != (*(i->cbc))(event,data,i->par));
    }

    return ret;
}

NRD_BOOL  nrdnb_client_t::register_callback(nrdnb_callback_t pCbc, NRD_VOID* pPar)
{
    if(pCbc == NULL) return NRD_FALSE;
    
    xtl::locker _l(&m_inst.m_cmtx);

    for (callbacks_t::iterator i=m_inst.m_cbcs.begin();
         i != m_inst.m_cbcs.end();i++)
    {
        if(i->cbc  == pCbc && i->par == pPar) return NRD_TRUE;
    }
    
    callback_t cbi;
    cbi.cbc = pCbc;
    cbi.par = pPar;

    m_inst.m_cbcs.push_back(cbi);
    
    return NRD_TRUE;
}

NRD_BOOL  nrdnb_client_t::unregister_callback(nrdnb_callback_t pCbc, NRD_VOID* pPar)
{
    if(pCbc == NULL) return NRD_FALSE;
    xtl::locker _l(&m_inst.m_cmtx);
    for (callbacks_t::iterator i=m_inst.m_cbcs.begin();
    i != m_inst.m_cbcs.end();i++)
    {
        if(i->cbc  == pCbc && i->par == pPar)
        {
            m_inst.m_cbcs.erase(i);
            break;
        }
    }
    return NRD_TRUE;
}

NRD_BOOL   nrdnb_client_t::request_server_info(const xtl::string& sHost, NRD_UINT uPort)
{
    if(!m_inst.is_created()) return NRD_FALSE;
    return (NRD_BOOL)m_inst.send_getinfo(sHost,uPort,3);
}

NRD_BOOL  nrdnb_client_t::start_info_tracker(const xtl::string& sHost, NRD_UINT uPort)
{
    if(m_inst.is_created()) return NRD_FALSE;
    
    m_inst.m_port  = !uPort ? NRD_BROWSE_PORT : uPort;
    m_inst.m_host  = sHost.empty() ? XTL_INET_IPV4_ANY : sHost;
    
    //TODO: Remove this code after impementing Info IPv6
    if(AF_INET6 == xtl::inet::guess_addr_family(m_inst.m_host)){
        m_inst.raise_event(NRDNBC_EVT_ONERROR,(void *)"Info IPv6 is not supported");
        m_inst.raise_event(NRDNBC_EVT_ONINFO,(void *)"Switching to 'any' address");
        m_inst.m_host = XTL_INET_IPV4_ANY;
    }
    
    m_inst.m_refr = NRD_FALSE;
    
    return m_inst.create();
}

NRD_VOID  nrdnb_client_t::stop_info_tracker()
{
    if(m_inst.is_created())
    {
        /*Mark thread as terminated*/
        m_inst.terminate();
        
        /*Close the socket to unlock the loop*/
        m_inst.close_sock();
        
        /*Wait the thread end*/
        m_inst.wait_for(60);
    }
    
    /*Just in case clear the servers list*/
    xtl::locker _l(&m_inst.m_smtx);
    m_inst.m_svrs.clear();
}

NRD_BOOL nrdnb_client_t::is_tracker_started()
{
    return m_inst.is_created();
}

NRD_UINT nrdnb_client_t::get_tracker_port()
{
    return m_inst.m_port;
}

xtl::string nrdnb_client_t::get_tracker_host()
{
    return m_inst.m_host;
}

NRD_BOOL  nrdnb_client_t::refresh_servers_list(NRD_BOOL bClear)
{
    if(!m_inst.is_created()) return NRD_FALSE;
    
    if(bClear)
    {
        xtl::locker _l(&m_inst.m_smtx);
        m_inst.m_svrs.clear();
    }
    
    m_inst.refresh_svrs();
    
    return NRD_TRUE;
}

NRD_VOID nrdnb_client_t::set_reconnect_time(NRD_UINT uTime/*seconds*/)
{
    m_inst.m_tout = xtl::min(NRD_UINT(NRD_MAXUINT)/1000,xtl::max(NRD_UINT(1),uTime));
}

NRD_BOOL    nrdnb_client_t::start_control_timer(NRD_UINT uPeriod/*=1 sec*/)
{
    m_inst.m_ctrl.period = xtl::max(NRD_UINT(1), uPeriod);
    return m_inst.m_ctrl.is_created() ? NRD_FALSE : m_inst.m_ctrl.create();
}

NRD_VOID    nrdnb_client_t::stop_control_timer()
{
    m_inst.m_ctrl.destroy();
}

NRD_BOOL  nrdnb_client_t::is_control_started(){
    
    return  m_inst.m_ctrl.is_created();
}

NRD_UINT nrdnb_client_t::get_reconnect_time()
{
    return m_inst.m_tout;
}

NRD_VOID nrdnb_client_t::set_refresh_time(NRD_UINT uTime/*seconds*/)
{
    m_inst.m_tref = uTime;
}

NRD_UINT nrdnb_client_t::get_refresh_time()
{
    return m_inst.m_tref;
}
