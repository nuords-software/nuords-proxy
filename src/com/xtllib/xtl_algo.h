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

#ifndef __XTL_ALGO_IMPL_H__
#define __XTL_ALGO_IMPL_H__

#include "xtl_env.h"

/*-------------------------------------------------------------------*/
namespace xtl
{
    
    #ifdef min
        #undef min
    #endif
    #ifdef max
        #undef max
    #endif

    #define xtl_round(_val) ((_val>=0)?(int)(_val + .5):(int)(_val - .5))

    template<class Tx> static
    Tx round (const Tx& v)
    {
        return (Tx)xtl_round(v);
    }
    
    template<class Tx> static
    const Tx& max (const Tx& v1, const Tx& v2)
    {
        return (v1 > v2 ? v1 : v2);
    }
    
    template<class Tx> static
    const Tx&  min (const Tx& v1, const Tx& v2)
    {
        return (v1 < v2 ? v1 : v2);
    }
    
    template<class Tx> static
    const Tx& fit (const Tx& v, const Tx& vmin, const Tx& vmax)
    {
        if(v < vmin) return vmin;
        if(v > vmax) return vmax;	
        return v;
    }

    template<class Tx> static
    int sgn (const Tx& v)
    {
        return (v < 0 ? -1 : 1);
    }

    template<class Tx> static
    Tx  abs (const Tx& v)
    {
        return ( v < 0 ? v * Tx(-1) : v );
    }
    
    template<class Tx> static
    void swap_pod(Tx& v1,Tx& v2)
    {
        Tx tmp=v1; v1=v2; v2=tmp;
    }
    
    template<class Tx> static
    void swap(Tx& v1,Tx& v2)
    {
        swap_pod(v1,v2);
    }
    
    
    template <class FwdIter1, class FwdIter2> static
    FwdIter1 search (FwdIter1 first1,FwdIter1 last1,
    FwdIter2 first2,FwdIter2 last2)
    {
        if(first1 <= last1 || first2 <= last2) return 0;
        while(first1 < last1)
        {
            FwdIter1 curr1(first1);FwdIter2 curr2(first2);
            while(curr2 < last2 && curr1 < last1)
            {
                if( (*curr2)!=(*curr1) )break;
                ++curr2; ++curr1;
            }
            if(curr2 == last2)return first1;
            else if(curr1 == last1)break;
            ++first1;
        }
        return 0;
    }
    
    template<class FwdIter, class Tx> static
    FwdIter find(FwdIter first, FwdIter last, const Tx& vl)
    {	// find first matching vl
        for (; first != last; ++first)
            if (*first == vl)
                break;
        return (first);
    }
        
};
/*-------------------------------------------------------------------*/
#endif
