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

#ifndef __XTL_HASHMAP_IMPL_H__
#define __XTL_HASHMAP_IMPL_H__

#include "xtl_env.h"
#include "xtl_string.h"

/*-------------------------------------------------------------------*/
#if defined (XTL_WINDOWS)
    #pragma warning( disable: 4996 )
    #include <hash_map>
#elif defined(XTL_MACOSX) 
    #include <Availability.h>
    #if defined(__MAC_10_9)
        #define XTL_UNORDERED_MAP 1
        #include <unordered_map>
    #else
        #include <ext/hash_map>
    #endif     
#else
    #include <ext/hash_map>
#endif 
/*-------------------------------------------------------------------*/

#if !defined(XTL_WINDOWS) && !defined(XTL_UNORDERED_MAP)
namespace __gnu_cxx
{ 
    template<> struct hash<xtl::string>
    {
          size_t
          operator()(const xtl::string& __s) const
          { return __stl_hash_string(__s.c_str()); }
    };

};
#endif

namespace xtl
{
    #if defined(XTL_UNORDERED_MAP)
    
    template <class Key, class Type, class HashFunc=std::hash<Key>,
    class EqualFunc=std::equal_to<Key>, class Allocator=std::allocator<std::pair<const Key, Type> > >
    class hash_map : public std::unordered_map<Key,Type,HashFunc,EqualFunc,Allocator>
    {
        
    };
    
    #else 
    
    #if defined (XTL_WINDOWS)
      template <class Key, class Type, class Traits=std::hash_compare<Key, std::less<Key> >,
      class Allocator=std::allocator<std::pair <const Key, Type> > >
      class hash_map : public std::hash_map<Key, Type, Traits, Allocator >
      {
        
      };
    #else
      
      template <class Key, class Type, class HashFunc=__gnu_cxx::hash<Key>,
      class EqualFunc=std::equal_to<Key>, class Allocator=std::allocator<Type> >
      class hash_map : public __gnu_cxx::hash_map<Key, Type, HashFunc, EqualFunc, Allocator >
      {
     
      }; 
    #endif
    
    #endif
    
};

/*-------------------------------------------------------------------*/
#if defined (XTL_WINDOWS)
    #pragma warning( default: 4996 )
#endif /*XTL_WINDOWS*/
/*-------------------------------------------------------------------*/
#endif
