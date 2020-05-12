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

#ifndef __XTL_THREAD_H__
#define __XTL_THREAD_H__

#include "xtl_env.h"
#include "xtl_vector.h"

#if !defined(XTL_WINDOWS)
    #include <pthread.h>
    #include <sys/types.h>
    #include <sys/ipc.h>
    #include <sys/sem.h>
    #include <unistd.h>
#endif

/*-------------------------------------------------------------------*/    
namespace xtl
{
    
    /******************************************
    BASIC TYPES
    *******************************************/
    #if defined (XTL_WINDOWS)
    typedef HANDLE thread_h;
    typedef unsigned long thread_id;
    #else
        typedef pthread_t thread_h;
        typedef pthread_t thread_id;
    #endif

    /******************************************
    BASIC FUNCTIONS
    *******************************************/
    
    static void sleep(unsigned int millisec)
    {
        #if defined(XTL_WINDOWS)
        Sleep(millisec);
        #else
            usleep(millisec * 1000);
        #endif
    };
    
    static unsigned long tick_count()
    {
        #if defined(XTL_WINDOWS)
        return ::GetTickCount();
        #else
            /*Is not implemented for linux/unix*/
            return 0;
        #endif
    };
    
    static thread_id current_thread_id()
    {
        #if defined(XTL_WINDOWS)
            return ::GetCurrentThreadId();
        #else
            return ::pthread_self();
        #endif
    };
    
    /******************************************
    MUTEX
    *******************************************/

    // Class name       : xtl::base_mutex_iface
    // Description      : Base interface for mutex wrapper
    XTL_NULL_TEMPLATE class base_mutex_iface
    {
        public:
        virtual void lock()   =0;
        virtual void unlock() =0;
    };
    typedef base_mutex_iface<>   base_mutex;
    
    // Class name       : xtl::locker_impl
    // Description      : Static object class to lock xtl::mutex
    XTL_NULL_TEMPLATE class  locker_impl
    {
        private :
        base_mutex * m_mutex;
        public:
        locker_impl<_null_>(const base_mutex * mtx):
                m_mutex((base_mutex *)mtx)
        {
            if(m_mutex)m_mutex->lock();
        }
        ~locker_impl<_null_>()
        {
            if(m_mutex)m_mutex->unlock();
        }
    };
    typedef locker_impl<> locker;

    // Class name       : xtl::dummy_mutex_impl
    // Description      : Dummy mutex implementation
    XTL_NULL_TEMPLATE class dummy_mutex_impl:
    public base_mutex_iface<_null_>
    {
        public:
        void lock(){};
        void unlock(){};
        dummy_mutex_impl<_null_>(){};
        ~dummy_mutex_impl<_null_>(){};
    };
    typedef dummy_mutex_impl<>   dummy_mutex;
     
    // Class name       : xtl::mutex_impl
    // Description      : CRITICAL_SECTION/mutex wrapper
    XTL_NULL_TEMPLATE class mutex_impl: 
    public base_mutex_iface<_null_>
    {
        private:
        #if defined (XTL_WINDOWS)
            CRITICAL_SECTION m_cs;
        #else
            pthread_mutexattr_t m_attr;
            pthread_mutex_t m_mutex;
        #endif
        public:
        
        void lock()
        {
            #if defined (XTL_WINDOWS)
                ::EnterCriticalSection(&m_cs);
            #else
                pthread_mutex_lock(&m_mutex);
            #endif
        }

        void unlock()
        {
            #if defined (XTL_WINDOWS)
                ::LeaveCriticalSection(&m_cs);
            #else
                pthread_mutex_unlock(&m_mutex);
            #endif
        }

        mutex_impl<_null_>()
        {
            #if defined (XTL_WINDOWS)
            ::InitializeCriticalSection(&m_cs);
            #else
            pthread_mutexattr_init(&m_attr);
            pthread_mutexattr_settype(&m_attr,PTHREAD_MUTEX_RECURSIVE);
            pthread_mutex_init(&m_mutex, &m_attr);
            #endif
        }
        ~mutex_impl<_null_>()
        {
            #if defined (XTL_WINDOWS)
            ::DeleteCriticalSection(&m_cs);
            #else
            pthread_mutex_destroy(&m_mutex);
            pthread_mutexattr_destroy(&m_attr);
            #endif
        }
        
    };
    typedef mutex_impl<>     mutex;


    // Macro name       : XTL_DECLARE_SYNCHRONIZER
    // Description      : Declares public/private synchronizer
    // Usage            : XTL_DECLARE_SYNCHRONIZER(public);                 
    #define XTL_DECLARE_SYNCHRONIZER( VISIBILITY )  private: \
    xtl::mutex __class_locker; VISIBILITY :  \
    void *              _section(){ return &__class_locker;}; \
    void                _lock(){__class_locker.lock();}; \
    void                _unlock(){__class_locker.unlock();}
    
    // Macro name       : XTL_DECLARE_NULL_SYNCHRONIZER
    // Description      : Declares public/private dummy synchronizer
    // Usage            : XTL_DECLARE_DUMMY_SYNCHRONIZER(public);
    #define XTL_DECLARE_DUMMY_SYNCHRONIZER( VISIBILITY ) private: \
    xtl::dummy_mutex     __class_locker; VISIBILITY :  \
    void *              _section(){ return &__class_locker;}; \
    void                _lock(){__class_locker.lock();}; \
    void                _unlock(){__class_locker.unlock();}

    // Macro name       : XTL_STATIC_SYNCHRONIZER
    // Description      : Declares static synchronizer
    // Usage            : XTL_STATIC_SYNCHRONIZER(public);
    #define XTL_STATIC_SYNCHRONIZER( VISIBILITY )  private: \
    static xtl::mutex __class_locker; VISIBILITY :  \
    static void *              _section(){ return &__class_locker;}; \
    static void                _lock(){__class_locker.lock();}; \
    static void                _unlock(){__class_locker.unlock();}
    
    #define XTL_STATIC_LOCKER(_CLASS) xtl::mutex _CLASS::__class_locker
    
    // Macro name       : XTL_LOCK_PTR
    // Description      : Use it to synchronize object by pointer
    // Usage            : XTL_LOCK_PTR(obj_pointer);
    #define XTL_LOCK_PTR(F_WITH_PTR)  xtl::locker \
    F_WITH_PTR##__lock_object( (xtl::mutex *)  \
    F_WITH_PTR ->_section() )
    
    // Macro name       : XTL_LOCK_REF
    // Description      : Use it to synchronize object by reference
    // Usage            : XTL_LOCK_REF(obj_ref);
    #define XTL_LOCK_REF(F_WITH_REF)  xtl::locker \
    F_WITH_REF##__lock_object( (xtl::mutex *)  \
    F_WITH_REF._section() )
    
    // Macro name       : XTL_LOCK_STATIC
    // Description      : Use it to synchronize object by static synchronizer
    // Usage            : XTL_LOCK_REF(obj_ref);
    #define XTL_LOCK_STATIC(F_WITH_CLASS)  xtl::locker \
    F_WITH_CLASS##__lock_object( (xtl::mutex *)        \
    F_WITH_CLASS::_section() )
    
    
    /******************************************
    RWMUTEX, single writer, multiple readers
    *******************************************/
    // Class name       : xtl::base_rwmutex_iface
    // Description      : Base interface for red-write mutex
    XTL_NULL_TEMPLATE class base_rwmutex_iface
    {
        public:
        virtual void begin_read()  =0;
        virtual void end_read()    =0;
        virtual bool try_write()   =0;
        virtual void begin_write() =0;
        virtual void end_write()   =0;
    };
    typedef base_rwmutex_iface<>   base_rwmutex;
    
    // Class name       : xtl::rlocker_impl
    // Description      : Reading-locker for rwmutex
    XTL_NULL_TEMPLATE class  rlocker_impl
    {
        private :
        base_rwmutex * m_rwmtx;
        public:
        rlocker_impl<_null_>(base_rwmutex * rwmtx):m_rwmtx(rwmtx)
        {
            if(m_rwmtx)m_rwmtx->begin_read();
        }
        ~rlocker_impl<_null_>()
        {
            if(m_rwmtx)m_rwmtx->end_read();
        }
    };
    typedef rlocker_impl<> rlocker;
    
    
    // Class name       : xtl::wlocker_impl
    // Description      : Writing-locker for rwmutex
    XTL_NULL_TEMPLATE class  wlocker_impl
    {
        private :
        base_rwmutex * m_rwmtx;
        public:
        wlocker_impl<_null_>(base_rwmutex * rwmtx):m_rwmtx(rwmtx)
        {
            if(m_rwmtx)m_rwmtx->begin_write();
        }
        ~wlocker_impl<_null_>()
        {
            if(m_rwmtx)m_rwmtx->end_write();
        }
    };
    typedef wlocker_impl<> wlocker;

    // Class name       : xtl::dummy_rwmutex_impl
    // Description      : Dummy rwmutex implementation
    XTL_NULL_TEMPLATE class dummy_rwmutex_impl:
    public base_rwmutex_iface<_null_>
    {
        public:
        void begin_read(){};
        void end_read(){};
        bool try_write(){return true;};
        void begin_write(){};
        void end_write(){};
        dummy_rwmutex_impl<_null_>(){};
        ~dummy_rwmutex_impl<_null_>(){};
    };
    typedef dummy_rwmutex_impl<>   dummy_rwmutex;
    
    // Class name       : xtl::rwmutex_impl
    // Description      : rwmutex implementation
    XTL_NULL_TEMPLATE class rwmutex_impl:
    public base_rwmutex_iface<_null_>
    {
        
        private:
        mutex                  m_mutex;
        typedef xtl::vector<thread_id> readers_t;
        readers_t              m_readers;
        bool                   m_write; 
       
        public:
        
        void begin_read()
        { 
            xtl::locker _l(&m_mutex);
            m_readers.push_back(current_thread_id());               
        }
        
        void end_read()
        {
           xtl::locker _l(&m_mutex);
           thread_id tid=current_thread_id();
           for(readers_t::iterator i=m_readers.begin();
                 i != m_readers.end(); i++)
           {
                if(*i == tid)
                {
                    /*Erase reader once*/
                    m_readers.erase(i);
                    return;
                }      
           }            
        }
        
        bool try_write()
        {
             m_mutex.lock();
             m_write = true;
             thread_id tid=current_thread_id();
             for(readers_t::iterator i=m_readers.begin();
                 i != m_readers.end(); i++ )
             {
                   if(*i != tid) 
                   {   
                      m_write = false;
                      m_mutex.unlock(); 
                      return false;
                   }       
             }
             return true;            
        }
       
        void begin_write()
        {
            /*Not optimal, but really portable.*/
            while(!try_write()) sleep(1);            
        }
        
        void end_write()
        {
            if(m_write) m_mutex.unlock();             
        }
        
        rwmutex_impl<_null_>():m_write(false)
        {
        }
        
        ~rwmutex_impl<_null_>()
        {
             /*Allow writers to end their job*/
             wlocker _wl(this);
        }
    };
    typedef rwmutex_impl<>         rwmutex;

    XTL_NULL_TEMPLATE class rwlocker_impl
    {
        private :
        xtl::base_rwmutex * m_rwmtx;
        int m_mode;
        public:
        rwlocker_impl<_null_>(xtl::base_rwmutex * rwmtx, int mode)
            :m_rwmtx(rwmtx), m_mode(write) { lock(); }
        ~rwlocker_impl<_null_>() { unlock(); }
        
        enum { read = 0, write = 1 };
        
        void lock() 
        {   if(m_rwmtx)
            { 
              if(m_mode == write)
                m_rwmtx->begin_write();
              else if(m_mode == read)
                 m_rwmtx->begin_read();
            }
        }
        
        void unlock() { 
             if(m_rwmtx){ 
                if(m_mode == write)
                    m_rwmtx->end_write();
                else if(m_mode == read)
                   m_rwmtx->end_read();
             }
        }
    };

    typedef rwlocker_impl<> rwlocker;

    
    /******************************************
    SEMAPHORE
    *******************************************/
    #if defined (XTL_WINDOWS)
       #define XTL_NOSEMAPHORE 0
       #define XTL_SEMAPHORE HANDLE
       #define XTL_SEMOPTS
    #else
       #define XTL_NOSEMAPHORE -1
       #define XTL_SEMAPHORE int
       #ifdef SEM_A
         #define XTL_SEMOPTS SEM_A|SEM_R
       #else
           #define XTL_SEMOPTS IPC_CREAT
       #endif
    #endif
     
    /*Maximal value to signal semaphore*/ 
    #define XTL_SEMMAX 0x7FFE

    // Class name       : xtl::semaphore
    // Description      : Simple local unnamed semaphore
    XTL_NULL_TEMPLATE class semaphore_impl
    {
        private:
        XTL_SEMAPHORE m_sem;

        public:
        
        semaphore_impl<_null_>(bool create_it=false, bool signal_it=false)
            :m_sem(XTL_NOSEMAPHORE)
        {
            if(create_it)
            {
                create(signal_it);
            }
        }
        ~semaphore_impl<_null_>() {
            destroy();
        }

        bool is_created()
        {
          return (m_sem != XTL_NOSEMAPHORE);
        }

        bool create(bool signal_it=false)
        {
            if(is_created()) return false;
            #if defined (XTL_WINDOWS)
                m_sem=::CreateSemaphore(NULL,0,0x7FFFFFFF,NULL);
            #else
                m_sem = semget (IPC_PRIVATE, 1, XTL_SEMOPTS);
            #endif
            if (signal_it) signal();
            return is_created();
        }

        bool destroy()
        {
            if( m_sem != XTL_NOSEMAPHORE)
            {
                signal(XTL_SEMMAX/2);
                XTL_SEMAPHORE tmp_sem = m_sem;
                m_sem = XTL_NOSEMAPHORE;
                #if defined (XTL_WINDOWS)
                  ::CloseHandle(tmp_sem);
                #else
                  semctl (tmp_sem, 0, IPC_RMID, NULL);
                #endif
            }
            return true;
        }

        
        /*Decreases semaphore once.
        In case initial count is 0 then
        waits until another thread will
        not increase it. see Signal()*/
        bool wait()
        {
            if(!is_created()) return false;
            
            #if defined (XTL_WINDOWS)
              return (::WaitForSingleObject(m_sem, INFINITE) != WAIT_FAILED);
            #else
              struct    sembuf    sop;
              sop.sem_num = 0;
              sop.sem_op = (-1);
              sop.sem_flg = 0;
              return (semop (m_sem, &sop, 1) != (-1) );
            #endif
        }
        
        /*Set that this object is signaled (free for usage)*/
        bool signal(int count = 1)
        {
            if( !is_created() || count < 0 || 
                count > XTL_SEMMAX ) return false;
            #if defined (XTL_WINDOWS)
            LONG prev;
            return (::ReleaseSemaphore(m_sem,count,&prev) == TRUE);
            #else
            struct    sembuf    sop;
            sop.sem_num = 0;
            sop.sem_op = (short)count;
            sop.sem_flg = 0;
            return (semop (m_sem, &sop, 1) != (-1) );
            #endif
        }
    };
    typedef semaphore_impl<> semaphore;
    
    #define XTL_INFINITE  0xFFFFFFFF
    
    /******************************************
    THREAD
    *******************************************/
 
    #define XTL_THPRIORITY_BELOW_IDLE      (-20)
    #define XTL_THPRIORITY_IDLE            (-15)
    #define XTL_THPRIORITY_LOWEST          (-2)
    #define XTL_THPRIORITY_BELOW_NORMAL    (-1)
    #define XTL_THPRIORITY_NORMAL          (0)
    #define XTL_THPRIORITY_ABOVE_NORMAL    (1)
    #define XTL_THPRIORITY_HIGHEST         (2)
    #define XTL_THPRIORITY_TIME_CRITICAL   (15)

    // Class name       : xtl::thread
    // Description        : thread wrapper
    XTL_NULL_TEMPLATE class thread_impl
    {
        
        private:
        thread_h   m_thread;
        thread_id  m_thread_id;
        bool m_terminated;
        bool m_free_on_term;
        bool m_waiting;
        bool m_wait_on_destroy;
        int  m_yield_count;
        int  m_priority;
        
        
        #if defined (XTL_WINDOWS)
            static unsigned long WINAPI  thread_main_func( LPVOID lpParam )
        #else
            static void * thread_main_func( void * lpParam )
        #endif
        {
                    thread_impl<_null_> * thread_ptr=
                        (thread_impl<_null_> *)lpParam;
                    if( thread_ptr != 0 )
                    {    
                        try { 
                        	#ifndef XTL_WINDOWS
                        	  thread_ptr->m_thread_id = pthread_self();
                        	#endif  
                        	thread_ptr->main(); 
                        }catch(...){}
                        
                        try {  if( thread_ptr->m_free_on_term ){
                                  thread_ptr->free_thread(); 
                                }else{
                                  thread_ptr->kill_handle();
                                }
                        }catch(...){}
                    }
                    return 0;
        }
        
        void   kill_handle()
        {
            XTL_LOCK_PTR ( this );
            #if defined (XTL_WINDOWS)
            thread_h tmp_h=m_thread;
            if(tmp_h)
            {
                m_thread = 0;
                ::CloseHandle(tmp_h);
            }
            #else
                m_thread    = 0;
                m_thread_id = 0;
            #endif
        }
        
        protected:
        
        virtual  void  main(){};
        virtual  void  free_thread(){delete this;};
        
        public:
        
        XTL_DECLARE_SYNCHRONIZER(public);
        
        bool  get_free_on_term()
        {
            return m_free_on_term;
        }
        
        void  set_free_on_term(bool Value)
        {
            m_free_on_term=Value;
        }
        
        
        bool  is_terminated()
        {
            return m_terminated;
        }
        
        void  terminate()
        {
            m_terminated=true;
        }
        
        //WARNING : Use terminate_and_wait() once per thread.
        //Otherwise you'll receive WAIT_FAILED
        int  terminate_and_wait(unsigned long wTimeOut=60000)
        {
            m_terminated=true;
            return  wait_for(wTimeOut);
        }
        
        //WARNING : Use wait_for() once per thread.
        //Otherwise you'll receive WAIT_FAILED
        int  wait_for(unsigned long wTimeOut)
        {
            if(m_waiting) return -1;
            
            {
                XTL_LOCK_PTR ( this );
                if(m_waiting)return -1;
                m_waiting = true ;
            }
            
            #if defined (XTL_WINDOWS)
            unsigned long nValue=WAIT_TIMEOUT;
            while(m_thread != 0 && nValue==WAIT_TIMEOUT && 
                  wTimeOut > 0 )
            {
                nValue=::WaitForSingleObject(m_thread,25);
                wTimeOut -= 25;
            }
            #else
            unsigned long nValue = 258L;
            while(m_thread != 0 && wTimeOut > 0)
            {
                sleep(25);
                wTimeOut -= 25;
            }
            #endif
            
            return  nValue;
        }
        
        //WARNING : Use destroy() once per thread.
        //Otherwise you'll receive FAIL.
        //Also you should not use destroy() in case
        //FreeOnTerminate mode is on because
        //it will not be destroyed and will return FAIL.
        
        bool destroy()
        {
            {
                XTL_LOCK_PTR ( this );
                if(m_free_on_term || m_waiting || !m_thread)
                {
                    return false;
                }
            }
            if(m_wait_on_destroy)
            {
                terminate_and_wait(60000);
            }
            kill_handle();
            return true;
        }
        
        //NOTE : You should destroy previous thread first
        //Otherwise the return value is FALSE
        bool create()
        {
            XTL_LOCK_PTR ( this );
            
            if(m_thread != 0) return false;
            
            m_terminated = false;
            m_waiting = false;
            m_wait_on_destroy = true;
            m_yield_count = 0;
            
            #if defined (XTL_WINDOWS)
            
            if( (m_thread  =  ::CreateThread ( 0 , 0 ,
            (LPTHREAD_START_ROUTINE) thread_main_func ,(LPVOID)this ,
            CREATE_SUSPENDED  , &m_thread_id ))
            != 0 )
            {
                ::SetThreadPriority(m_thread,m_priority );
                ::ResumeThread(m_thread);
            }
            #else
               
                pthread_attr_t attrs;
                pthread_attr_init(&attrs);
                pthread_attr_setstacksize(&attrs, 1048576);
                pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
                if(pthread_create(&m_thread, &attrs, thread_main_func, 
                    (void *)this) != 0)
                {
                    m_thread = 0;
                }
                pthread_attr_destroy(&attrs);

            #endif
            
            return (m_thread != 0);
        }
        
        void  set_wait_on_destroy(bool Val)
        {
            m_wait_on_destroy = Val;
        }
        
        bool   get_wait_on_destroy()
        {
            return m_wait_on_destroy;
        }
        
        thread_h  get_handle()
        {
            return m_thread;
        }
        
        bool  is_created()
        {
            return (m_thread != 0);
        }
        
        int   get_priority()
        {
            return m_priority;
        }
        
        void   set_priority(int Value)
        {
            m_priority = Value;
            if(is_created())
            {
                #if defined (XTL_WINDOWS)
                  XTL_LOCK_PTR( this ); 
                  ::SetThreadPriority(m_thread,Value);
                #else
                  /*Does nothing on Unix/Linux for now*/
                #endif
            }
        }

        thread_id  get_thread_id()
        {
            return m_thread_id;
        }

        /*Call it in the "thread::main" 
          implementation to provide load balancing
          between threads*/  
        void  yield()
        {
            if( ++m_yield_count > (20 + m_priority))
            {
                m_yield_count =0;
                xtl::sleep(0);
            }
        }
               
        thread_impl<_null_>(bool create_it=false):
        m_terminated(false),
        m_free_on_term(false),
        m_waiting(false),
        m_wait_on_destroy(true),
        m_thread(0),
        m_yield_count(0),
        m_priority(XTL_THPRIORITY_NORMAL)
        {
            if(create_it)
            {
                create();
            }
        }
        
        ~thread_impl<_null_>()
        {
            if(m_thread)
            {
                if(!m_free_on_term && m_wait_on_destroy)
                {
                    terminate_and_wait(60000);
                }
                kill_handle();
            }
        }
        
    };
    typedef thread_impl<> thread;
    
};/*xtl namespace*/

#endif //__XTL_THREAD_H__


