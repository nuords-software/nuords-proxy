/*********************************************************************
 * Project : NuoRDS Proxy
 *
 *********************************************************************
 * Programmer(s) :  Volodymyr Bykov
 *
 *********************************************************************
 * Description :  Network browser client
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
 ********************************************************************/
 
#ifndef __NRD_NETWORK_BROWSER_CLIENT_H__
#define __NRD_NETWORK_BROWSER_CLIENT_H__

#include <nrdnb.h>
#include <xtl_thread.h>
#include <xtl_vector.h>
#include <xtl_string.h>


/*Severs list item*/
NRD_TYPEDEF_STRUCT(_HNRD_NBSITEM)
{
    HNRD_NBINF  info;
    xtl::string host;
    xtl::string lsid;
    NRD_UINT    state;
    time_t      time; /*The last activity time*/

    _HNRD_NBSITEM(): state(0),time(0)
    {
       memset(&info,0,sizeof(info));
    }
    _HNRD_NBSITEM(const _HNRD_NBSITEM & src)
    {
       *this = src;
    }
    _HNRD_NBSITEM & operator = (const _HNRD_NBSITEM & src)
    {
        memcpy(&info,&src.info,sizeof(info));
        host  =  src.host;
        lsid  =  src.lsid;
        state =  src.state;
        time  =  src.time;
        return *this;
    }

}HNRD_NBSITEM;

typedef xtl::vector<HNRD_NBSITEM> nrdnb_items_t;

/*Severs list item states*/
#define NRDNBS_WAIT      0  /*Waiting for state (assume LIST)*/
#define NRDNBS_HERE      1  /*The first NRDNBM_IAMHERE is received*/
#define NRDNBS_LIST      2  /*The server is active and listed now*/
#define NRDNBS_LOST      3  /*NRDNBM_IAMLOST is received*/

typedef NRD_BOOL(*nrdnb_callback_t)(NRD_UINT uEvent,NRD_VOID* pData,NRD_VOID* pParam);

#ifndef NRDNBC_EVT_DEFINED
#define NRDNBC_EVT_DEFINED
  /*Net-Browser started, pData is NULL*/
  #define NRDNBC_EVT_STARTED    1
  /*Net-Browser stopped, pData is NULL*/
  #define NRDNBC_EVT_STOPPED    2
  /*Net-Browser restarted, pData is NULL*/
  #define NRDNBC_EVT_RESTART    3
  /*Net-Browser refreshed, pData is NULL*/
  #define NRDNBC_EVT_REFRESH    4 
  /*Servers list chaged, pData is NULL*/
  #define NRDNBC_EVT_SVRLIST    5
   /*Server returned requested info, pData is HNRD_NBSITEM ptr*/
  #define NRDNBC_EVT_SVRINFO    6
  /*Server indicated "up" state, pData is HNRD_NBSITEM ptr*/
  #define NRDNBC_EVT_SVRHERE    7
  /*Server indicated "down" state, pData is HNRD_NBSITEM ptr*/
  #define NRDNBC_EVT_SVRLOST    8
  /*Error message, pData is error string ptr*/
  #define NRDNBC_EVT_ONERROR    9
  /*Info message, pData is debug string ptr*/
  #define NRDNBC_EVT_ONINFO     10
  /*TimerPeriod message, see SetTimerPeriod, pData is NULL*/
  #define NRDNBC_EVT_ONTIMER    11
#endif

class nrdnb_client_t:public xtl::thread
{
	
    private:
        
    typedef struct _callback_t
    {
         nrdnb_callback_t cbc;
         NRD_VOID*           par;
    }callback_t;    
    
    typedef xtl::vector<callback_t> callbacks_t;

    class control_t:public xtl::thread
    {      
      friend class nrdnb_client_t;
      private:
      nrdnb_client_t *owner;
      NRD_UINT   period;
      protected:
      void main();
      control_t();
      ~control_t();
      public: 
    };
 
    static nrdnb_client_t m_inst;
    nrdnb_items_t         m_svrs;
    xtl::mutex            m_smtx;
    xtl::mutex            m_cmtx;
    callbacks_t           m_cbcs;
    control_t             m_ctrl;
    NRD_UINT              m_tout;  
    NRD_UINT              m_tref; 
    NRD_UINT              m_port; 
    xtl::string           m_host;
    NRD_BOOL              m_refr;
    OS_SOCKET             m_sock;

    bool      create_sock();
    void      close_sock();
    bool      raise_event(NRD_UINT event, void * data);
    bool      recv_len(void* buf,size_t len,struct sockaddr* from,socklen_t* fromlen);
    bool      send_len(void* buf,size_t len, struct sockaddr* to, socklen_t tolen);
    bool      send_getinfo(const xtl::string& host,NRD_UINT port=0, int times = 1);
    bool      recv_msg(HNRD_NBMSG* pmsg,struct sockaddr * from,socklen_t* fromlen);
    void      refresh_svrs(bool lostw=false);
    void      check_svrs();

    protected:
    
    void main();
    
    nrdnb_client_t();
    ~nrdnb_client_t();
    
    public:

    static NRD_BOOL        register_callback(nrdnb_callback_t pCbc, NRD_VOID* pPar);
    static NRD_BOOL        unregister_callback(nrdnb_callback_t pCbc, NRD_VOID* pPar);
    
    static NRD_BOOL        start_info_tracker(const xtl::string& sHost, NRD_UINT uPort);
    static NRD_VOID        stop_info_tracker();
    static NRD_BOOL        is_tracker_started();
    
    static NRD_BOOL        start_control_timer(NRD_UINT uPeriod=1/*sec*/);
    static NRD_VOID        stop_control_timer();
    static NRD_BOOL        is_control_started();
    
    static NRD_BOOL        request_server_info(const xtl::string& sHost, NRD_UINT uPort);
    static NRD_BOOL        refresh_servers_list(NRD_BOOL bClear);
    
    static xtl::string     get_tracker_host();
    static NRD_UINT        get_tracker_port();
    
    static NRD_VOID        set_reconnect_time(NRD_UINT uTime/*seconds*/);
    static NRD_UINT        get_reconnect_time();
   
    static NRD_VOID        set_refresh_time(NRD_UINT uTime/*seconds*/);
    static NRD_UINT        get_refresh_time();
       
};

#endif

