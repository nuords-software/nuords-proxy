/*********************************************************************
 * Project : NuoRDS Proxy
 *
 *********************************************************************
 * Description :  Configuration tools. 
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

#ifndef  __NRD_PROXY_CONFIG_TOOLS_H__
#define  __NRD_PROXY_CONFIG_TOOLS_H__

#include <nrdtype.h>
#include <xtl_hash_map.h>
#include <xtl_vector.h>

class nrdpx_section_t
{
    friend class nrdpx_config_t;
    private:
    
    typedef struct _item_t{
        xtl::string  val;
        xtl::strings vls;
    }item_t;
    
    typedef       xtl::hash_map<xtl::string,item_t> items_t;
    items_t       m_items;
    xtl::string   m_name;
    static item_t m_empty;
    void       *  m_data;
    
    nrdpx_section_t(const xtl::string& sName);
    ~nrdpx_section_t();
    
    public:
    
    xtl::string get_name();
    	
    const xtl::strings& get_strings(const xtl::string& sKey);

    xtl::string get_string(const xtl::string& sKey, 
    	                  const xtl::string& sDef=xtl::snull);
    int   get_integer(const xtl::string& sKey, int iDef=0);
    bool  get_boolean(const xtl::string& sKey, bool bDef=false);
    	
    bool  key_exists(const xtl::string& sKey);
    void* set_data(void * pData);
    void* get_data();
};

#ifdef NRD_WINDOWS
 #pragma warning(disable:4996) // deprecated std methods
#endif

class nrdpx_config_t
{
    private:
    
    typedef xtl::vector<nrdpx_section_t *> sections_t;
    sections_t   m_sects;
    xtl::string  m_class;

    void         add_item(nrdpx_section_t * sc,char * ln,size_t sz);
    void         add_sect(nrdpx_section_t * sc);
    
    public:
    	
    typedef sections_t::iterator iterator;
    
    nrdpx_config_t();
    ~nrdpx_config_t();
    
    iterator     begin();
    iterator     next(const iterator& iPrev);
    iterator     end();
    bool         empty();
    void         clear();
     
    nrdpx_section_t*   get_section(const iterator& iIter);
    nrdpx_section_t*   first_section();
    nrdpx_section_t*   find_section(const xtl::string& sName,bool bClass=true);
    nrdpx_section_t*   find_by_value(const xtl::string& sKey,const xtl::string& sVal,bool bCase=false);
    nrdpx_section_t*   find_by_value(const xtl::string& sKey,int iVal);
    bool               load_from_file(const xtl::string& filePath,
    const xtl::string& sClass=xtl::snull);
   
};

#ifdef NRD_WINDOWS
 #pragma warning(default:4996) // deprecated std methods
#endif

#endif
