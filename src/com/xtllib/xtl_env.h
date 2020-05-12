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

#ifndef __XTL_ENV_H__
#define __XTL_ENV_H__
//--------------------------------------------------------------------
#define XTL_PORT_VERSION  0x100
#define XTL_STATIC_TEMPLATE_DATA 1
//--------------------------------------------------------------------
#if defined(MACOSX) || defined(__APPLE_CC__)
    #define XTL_MACOSX
#elif defined(WIN32) || defined(WIN64) 
    #define XTL_WINDOWS
#endif
//--------------------------------------------------------------------
#if defined(XTL_WINDOWS)
   #define XTL_WTHREADS 1
#else
   #define XTL_PTHREADS 1  
#endif 

#define XTL_BEGIN_NAMESPACE namespace xtl {
#define XTL_END_NAMESPACE }

#define XTL_NULL_TEMPLATE template<class _null_=int>

//Vector page size (default capacity step)
//! Not less than 1
#if !defined(XTL_VECTOR_PAGE_SZ)
     #define XTL_VECTOR_PAGE_SZ 16
#endif

//--------------------------------------------------------------------
#endif
