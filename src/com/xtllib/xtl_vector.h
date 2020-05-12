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

#ifndef __XTL_VECTR_IMPL_H__
#define __XTL_VECTR_IMPL_H__

#include "xtl_env.h"
#include "xtl_except.h"
#include "xtl_algo.h"
#include <memory.h>
#include <stdlib.h>

/*-------------------------------------------------------------------*/
namespace xtl
{
    //Vector page size (default capacity step)
    //! Not less than 1
    #if !defined(XTL_VECTOR_PAGE_SZ)
    #define XTL_VECTOR_PAGE_SZ 16
    #endif
    
    template <class Tx > class vector {
        
        public:

        typedef Tx item_type;
        typedef item_type * item_ptr;
        typedef item_type& reference;
        typedef const item_type& const_reference;
        typedef item_ptr iterator_type;
        typedef iterator_type iterator;
        typedef const item_type * const_iterator;
        typedef ::size_t size_type;
        typedef vector<item_type> vector_type;

        private:
        
        item_ptr  m_data;
        size_type m_capacity;
        size_type m_size;
                
        inline void * mem_ptr(size_type idx)
        {
            return (void *)(m_data+idx);
        }

        inline const void * const_mem_ptr(size_type idx) const
        {
            return (const void *)(m_data+idx);
        }

        inline size_type  iter_index(iterator_type vi)
        {
            size_type ret=vi-m_data;
            if( ret >m_size )xtl_throw("Iterator out of range");
            return ret;
        }

        inline reference set_iter_item(iterator_type vi, const_reference vr)
        {
            void * mem=(void *)vi;
            return *(new (mem) item_type(vr));
        }

        inline reference set_item(size_type idx,const_reference vr)
        {
            void * mem=mem_ptr(idx);
            return *(new (mem) item_type(vr));
        }

        inline void destroy_iter_item(iterator vi)
        {
            if(vi){
                vi->~Tx ();
            }
        }

        inline void destroy_item(size_type idx)
        {
            if(m_data){
                (m_data+idx)->~Tx();
            }
        }

        inline size_type calc_capacity(size_type sz)
        {
            return ( sz / XTL_VECTOR_PAGE_SZ + 1 ) * XTL_VECTOR_PAGE_SZ;
        }

        void check_iterator(iterator vi)
        {
            if(vi < begin() || vi > end()){
              xtl_throw("Iterator out of range"); 
            }
        }

        void set_capacity(size_type cp)
        {
            //todo: maybe we should throw exception here?
            if(cp < m_size) cp = m_size;
            
            if(m_capacity == cp && m_data) return;
            
            if(!cp){
                
                //allocate minimal size object for for begin()/end()
                item_ptr p_tmp=m_data;
                m_data=(item_ptr)::malloc(sizeof(size_type));
                if(p_tmp)::free((void *)p_tmp);
                
            }else if(!m_data || cp < m_capacity){
                
                //create buffer or decrease capacity
                item_ptr p_tmp=m_data;
                m_data=(item_ptr)::malloc(sizeof(item_type) * cp);
                if(p_tmp){
                    if(m_size){
                        ::memcpy((void *)m_data,(void *)p_tmp,m_size * sizeof(item_type));
                    }
                    ::free((void *)p_tmp);
                }
            }else{
                //increase vector capacity
                m_data=(item_ptr)::realloc((void *)m_data,sizeof(item_type) * cp);
            }
            m_capacity = cp;
        }
   
        void before_increase(size_type icount=1)
        {
            if(m_size+icount > max_size() )
            {
                xtl_throw("Vector has maximal size");
            }
            if(m_capacity-m_size < icount )
            {
                //increase capacity for some PAGE_SZ values
                set_capacity(calc_capacity(m_size+icount));
            }
        }
        
        void after_decrease()
        {
            if((m_capacity-m_size) > XTL_VECTOR_PAGE_SZ * 4){
                //decrease capacity
                set_capacity(m_size+XTL_VECTOR_PAGE_SZ);
            }
        }

        public:
        
        vector():m_size(0),m_data(0),m_capacity(0)
        {
            set_capacity(0);
        }

        vector(size_type n, const_reference value):m_size(0),m_data(0),m_capacity(0)
        {
            if(!n){
               set_capacity(0);
            }else{
               resize(n,value);
            }
        }

        vector(size_type n):m_size(0),m_data(0),m_capacity(0)
        {
            if(!n){
            	 set_capacity(0);
            }else{
               resize(n);
            }
        }
        
        vector (const vector_type& v_src):m_size(0),m_data(0),m_capacity(0)
        {
            assign(v_src);
        }

        ~vector()
        {              
            if(m_data){
               clear(); ::free((void*)m_data); 
            }
        }
        
        void pack()
        {
            if( m_capacity > m_size ){
                set_capacity(m_size);
            }
        }
        
        void assign (const vector_type& src)
        {
            if(&src!=this)
            {
                vector_type vtmp;
                vtmp.set_capacity(src.m_capacity);
                #if defined (XTL_WINDOWS)
                for (vector_type::const_iterator it=src.begin();
                it!=src.end();it++)
                {
                    vtmp.push_back(*it);
                }
                #else
                for (size_type i=0;i < src.size() ;i++)
                {
                    vtmp.push_back(src[i]);
                }
                #endif
                swap(vtmp);
            }
        }
        
        reference operator[] (size_type idx)
        {
            return  *(m_data+idx);
        }
        
        const_reference operator[] (size_type idx) const
        {
            return  *(m_data+idx);
        }
        
        reference at (size_type  idx)
        {
            if (idx >= size())
            {
                xtl_throw("Vector index out of range");
            }
            return *(m_data+idx);
        }

        const_reference at (size_type idx) const
        {
            if (idx >= size())
            {
                xtl_throw("Vector index out of range");
            }
            return *(m_data+idx);
        }

        reference front ()
        {
            return at(0);
        }

        const_reference front () const
        {
            return at(0);
        }

        reference back ()
        {
            if(!m_size)
            {
                return at(0);
            }
            return at(m_size-1);
        }

        const_reference back () const
        {
            if(!m_size)
            {
                return at(0);
            }
            return at(m_size-1);
        }
        
        size_type size () const
        {
            return m_size;
        }
        
        size_type max_size () const
        {
            return  size_type(-1)/sizeof(item_type);
        }
        
        void resize(size_type sz, const_reference vr=item_type())
        {
            if(sz == m_size) return;
            else if(sz > max_size() ){
                xtl_throw("Invalid vector size");    
            }
            
            if(sz > m_size)
            {
                size_type old_cap=m_capacity;
                size_type new_cap=calc_capacity(sz);
                   
                if(new_cap > old_cap){
                    set_capacity(new_cap);
                }
                
                //Put the default value
                size_type idx=m_size;
                try
                {
                    for (;idx<sz;idx++)
                    {
                        set_item(idx,vr);
                    }
                }
                catch(...)
                {
                    //Restoring object state
                    while(idx > m_size)
                    {
                        destroy_item(--idx);
                    }
                    if(old_cap != m_capacity)
                    {
                        set_capacity(old_cap);
                    }
                    throw;
                }
            }
            else
            {
                //Remove unused values
                for (size_type i=sz;i<m_size;i++)
                {
                    destroy_item(i);
                }
            }
            m_size=sz;      
        }
        
        size_type capacity () const
        {
            return  m_capacity;
        }
        
        bool empty () const
        {
            return (!m_size);
        }
        
        void reserve (size_type sz)
        {
            size_type new_capacity=calc_capacity(sz);
            if(new_capacity>m_capacity)set_capacity(new_capacity);
        }
        
        void push_back (reference vr)
        {
            before_increase();
            set_item(m_size,vr);
            m_size++;
        }
        
        void push_back (const_reference vr)
        {
            before_increase();
            set_item(m_size,vr);
            m_size++;
        }
        
        void pop_back ()
        {
            if(m_size)
            {
                m_size--;
                destroy_item(m_size);
                after_decrease();
            }
        }
        
        void clear()
        {
            if(m_size)
            {
                size_type i_tmp(m_size); m_size=0;
                for (size_type i=0 ; i < i_tmp ; i++){
                    destroy_item(i);
                }
                after_decrease();
            }
            
        }
        
        vector_type& operator=(const vector_type& vt)
        {
            assign(vt);
            return (*this);
        }
                
        iterator begin ()
        {
            return (iterator)mem_ptr(0);
        }
        
        const_iterator begin () const
        {
            return (const_iterator)const_mem_ptr(0);
        }
        
        iterator end ()
        {
            return (iterator)mem_ptr(m_size);
        }
        
        const_iterator end () const
        {
            return (const_iterator)const_mem_ptr(m_size);
        }
        
        iterator insert (iterator vi, const_reference vr=item_type())
        {
            
            check_iterator(vi);

            size_type idx=iter_index(vi);
            before_increase();
            //Restore iterator
            //because before_increase can increase the capacity
            //therefore iterator will be corrupted
            vi=(iterator)mem_ptr(idx);
            //Move items
            if(idx < m_size)
            {
                memmove(mem_ptr(idx+1),mem_ptr(idx),
                (m_size-idx)* sizeof(item_type));
            }
            //Set item
            try
            {
                set_iter_item(vi,vr);
            }
            catch(...)
            {
                //Restoring object state
                memmove(mem_ptr(idx),mem_ptr(idx+1),
                (m_size-idx)* sizeof(item_type));
                throw;
            }
            m_size++;
            return vi;
            
        }

        iterator erase (iterator vi)
        {
            check_iterator(vi);

            if(vi!=end())
            {
                //Check iterator
                size_type idx=iter_index(vi);
                //Adjust size
                m_size--;
                destroy_iter_item(vi);
                //Move items
                if(idx < m_size)
                {
                    memmove(mem_ptr(idx),mem_ptr(idx+1),
                    (m_size-idx) * sizeof(item_type) );
                }
                after_decrease();
                //Restore iterator
                //because after_decrease can decrease the capacity
                //therefore iterator will be corrupted
                vi=(iterator)mem_ptr(idx);
                
            }
            return vi;
        }

        iterator erase (iterator vi1, iterator vi2)
        {
            check_iterator(vi1);
            check_iterator(vi2);
            
            size_type idx1=iter_index(vi1);
            size_type idx2=iter_index(vi2);

            if(idx2 < idx1)
            {
                xtl_throw("Second iterator less than first iterator");
            }
            else if(idx2 > idx1)
            {
                
                //Adjust size
                m_size-=(idx2-idx1);
                
                //Destroy the items
                while(vi1 != vi2)
                {
                    destroy_iter_item(vi1);
                    ++vi1;
                }
                if(idx1 < m_size)
                {
                    memmove(mem_ptr(idx1),mem_ptr(idx2),
                    (m_size-idx1) * sizeof(item_type) );
                    
                }
                after_decrease();
                vi1=(iterator)mem_ptr(idx1);
            }
            return vi1;
        }

        iterator remove (size_type idx)
        {
            return erase(begin() + idx);
        }
        
        void swap (vector_type& src)
        {
            if((void*)&src != (void*)this)
            {
                swap_pod(m_data,src.m_data);
                swap_pod(m_capacity,src.m_capacity);
                swap_pod(m_size,src.m_size);
            }
        }
        
    };
    
    
    //Non-member Operators is not completed now.
    //First need to implement the vector::iterator
    
    template <class Tx>
    int compare_vectors (const vector<Tx>& vt1,
    const vector <Tx>& vt2)
    {
        
        if(!vt1.size() && !vt2.size())
        {
            return 0;
        }
        else if(!vt2.size())
        {
            return 1;
        }
        else if(!vt1.size())
        {
            return -1;
        }
        
        int ret=0;
        
        #if defined (XTL_WINDOWS)
        
        vector<Tx>::const_iterator it1=vt1.begin();
        vector<Tx>::const_iterator it2=vt2.begin();
        vector<Tx>::const_iterator eit1=vt1.end();
        vector<Tx>::const_iterator eit2=vt2.end();
        
        while(it1!=eit1 && it2!=eit2)
        {
            if(*it1 > *it2)
            {
                ret++;
            }
            else if(*it1 < *it2)
            {
                ret--;
            }
            ++it1;++it2;
        }
        
        #else /*Problems with GCC on linux/unix*/
        
        for (unsigned int i=0;
              i < vt1.size() && i < vt2.size() ;i++)
         {

            if(vt1[i] > vt2[i] )
            {
                ret++;
            }
            else if(vt1[i] < vt2[i])
            {
                ret--;
            }
         }
         
        #endif


        return ret;
    }
    
    template <class Tx> static
    bool vectors_equal (const vector<Tx>& vt1,
    const vector<Tx> & vt2)
    {
        if(vt1.size() != vt2.size())
        {
            return false;
        }
        #if defined (XTL_WINDOWS)
        vector<Tx>::const_iterator it1=vt1.begin();
        vector<Tx>::const_iterator it2=vt2.begin();
        vector<Tx>::const_iterator eit1=vt1.end();
        vector<Tx>::const_iterator eit2=vt2.end();
        
        while(it1!=eit1 && it2!=eit2)
        {
            if(*it1 != *it2)
            {
                return false;
            }
            ++it1;++it2;
        }
        #else /*Problems with GCC on linux/unix*/
         for (unsigned int i=0;
              i < vt1.size() && i < vt2.size() ;i++)
         {
            if(vt1[i] != vt2[i] )
            {
                return false;
            }
         }
        #endif

        return true;
    }
    
    template <class Tx> static
    bool operator == (const vector<Tx>& vt1,
    const vector <Tx>& vt2)
    {
        return vectors_equal(vt1,vt2);
    }
    template <class Tx> static
    bool operator != (const vector<Tx>& vt1,
    const vector <Tx>& vt2)
    {
        return !vectors_equal(vt1,vt2);
    }
    template <class Tx> static
    bool operator < (const vector<Tx>& vt1,
    const vector<Tx>& vt2)
    {
        return (compare_vectors(vt1,vt2) < 0);
    }
    template <class Tx> static
    bool operator > (const vector<Tx>& vt1,
    const vector<Tx>& vt2)
    {
        return (compare_vectors(vt1,vt2) > 0);
    }
    template <class Tx> static
    bool operator <= (const vector<Tx>& vt1,
    const vector<Tx>& vt2)
    {
        return (compare_vectors(vt1,vt2) <= 0);
    }
    template <class Tx> static
    bool operator>= (const vector<Tx>& vt1,
    const vector<Tx>& vt2)
    {
        return (compare_vectors(vt1,vt2) >= 0);
    }
    template <class Tx> static
    void swap (const vector<Tx>& vt1, const vector<Tx>& vt2)
    {
        vt1.swap(vt2);
    }
    
};
/*-------------------------------------------------------------------*/
#endif
