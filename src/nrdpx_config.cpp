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
 *********************************************************************
 * 
 * As of May 1 2020, a revocable permission to distribute binary copies 
 * of this software without source code and/or copyright notice has been
 * granted to Ozolio Inc (a Hawaii Corporation) by the author. 
 *
 ********************************************************************/

#include "nrdpx_config.h"

#ifdef NRD_WINDOWS
 #pragma warning(disable:4996) // deprecated std methods
#endif

nrdpx_section_t::item_t nrdpx_section_t::m_empty;

nrdpx_section_t::nrdpx_section_t(const xtl::string& sName):
m_name(sName),
m_data(NULL)
{
}

nrdpx_section_t::~nrdpx_section_t()
{
}

const xtl::string& nrdpx_section_t::get_name()
{
    return m_name;
}

const xtl::strings& nrdpx_section_t::get_strings(const xtl::string& sKey)
{
    items_t::iterator i=m_items.find(sKey);
    if(i == m_items.end()) return m_empty.vls;
    
    if(i->second.vls.size() > 0) return i->second.vls;
    
    xtl::string * str = NULL;bool txt = false;
    char * c = (char *)i->second.val.c_str();
    for(; *c != '\0' ;c++){
        
        if(*c == '"'){
            txt = !txt;
        }
        
        if(!txt && (*c == ' ' || *c == '\t' || *c == ','))
        {
            str = NULL;
            continue;
        }

        if(!str){
            i->second.vls.resize(i->second.vls.size() + 1);
            str = i->second.vls.end() - 1;
        }

        (*str) += *c;
    }

    if(!i->second.vls.size()) i->second.vls.resize(1);
    
    return i->second.vls;
}

const xtl::string& nrdpx_section_t::get_string(
const xtl::string& sKey, const xtl::string& sDef/*=xtl::snull*/)
{
    items_t::iterator i=m_items.find(sKey);
    if(i == m_items.end()) return sDef;
    return i->second.val;
}

int  nrdpx_section_t::get_integer(const xtl::string& sKey, int iDef/*=0*/)
{
    const xtl::string& s = get_string(sKey);
    return s.empty() ? iDef : xtl::stoi(s);
}

bool   nrdpx_section_t::get_boolean(const xtl::string& sKey, bool bDef/*=false*/)
{
    const xtl::string& s =get_string(sKey);
    return s.empty() ? bDef : (!stricmp(s.c_str(),"true") ||
    !stricmp(s.c_str(),"yes") || !stricmp(s.c_str(),"1"));
}

bool  nrdpx_section_t::key_exists(const xtl::string& sKey)
{
    return (bool)(m_items.find(sKey) != m_items.end());
}

void* nrdpx_section_t::set_data(void * pData)
{
    m_data = pData;
    return m_data;
}

void* nrdpx_section_t::get_data()
{
    return m_data;
}

nrdpx_config_t::nrdpx_config_t()
{
}

nrdpx_config_t::~nrdpx_config_t()
{
    clear();
}

nrdpx_config_t::iterator nrdpx_config_t::begin()
{
    return m_sects.begin();
}

nrdpx_config_t::iterator nrdpx_config_t::end()
{
    return m_sects.end();
}

bool    nrdpx_config_t::empty()
{
    return m_sects.empty();
}

nrdpx_config_t::iterator nrdpx_config_t::next(const iterator& iPrev)
{
    if(iPrev == m_sects.end()) return iPrev;
    return (++(*((sections_t::iterator *)&iPrev)));
}

nrdpx_section_t* nrdpx_config_t::get_section(const iterator& iIter)
{
    if(iIter == m_sects.end()) return NULL;
    return *iIter;
}

nrdpx_section_t*   nrdpx_config_t::first_section()
{
    if(m_sects.empty()) return NULL;
    return *m_sects.begin();
}

nrdpx_section_t*  nrdpx_config_t::find_section(const xtl::string& sName,bool bClass/*=true*/)
{
    if(sName.empty()) return NULL;
    
    xtl::string s;
    if(bClass && !m_class.empty())
        s = m_class + "." + sName;
    else
        s = sName;
    
    for(sections_t::iterator i=m_sects.begin();
    i != m_sects.end(); i++)
    {
        if(!stricmp((*i)->m_name.c_str(),s.c_str()))
        {
            return *i;
        }
    }
    
    return NULL;
}

nrdpx_section_t* nrdpx_config_t::find_by_value(const xtl::string& sKey,
	         const xtl::string& sVal , bool bCase/*=false*/)
{
    for(sections_t::iterator i=m_sects.begin();
        i != m_sects.end(); i++)
    {
        nrdpx_section_t::items_t::iterator v=(*i)->m_items.find(sKey);
        if(v != (*i)->m_items.end() &&
          ((!bCase && !stricmp(v->second.val.c_str(),sVal.c_str())) ||
          (bCase && !strcmp (v->second.val.c_str(),sVal.c_str()))))
        {
            return *i;
        }
    }
    return NULL;
}

nrdpx_section_t* nrdpx_config_t::find_by_value(const xtl::string& sKey,int iVal)
{
    return find_by_value(sKey,xtl::itos(iVal));
}

void  nrdpx_config_t::add_item(nrdpx_section_t * sc,char * ln,size_t sz)
{
    if(!sc || !ln || !sz) return;
    xtl::string k; bool b=false;
    nrdpx_section_t::item_t v;
    
    for (size_t i=0;i<sz;i++)
    {
        if(!b && ln[i] == '=') b = true;
        else if(!b)k += ln[i];
        else v.val += ln[i];
    }
    
    k     = xtl::strim(k);
    v.val = xtl::strim(v.val);
    
    if(!v.val.empty() && v.val[0] == '\"'){
        v.val = v.val.substr(1,v.val.size() - 1);
    }
    
    if(!v.val.empty() && v.val[v.val.size()-1] == '\"'){
        v.val = v.val.substr(0,v.val.size() - 1);
    }
    
    if(!k.empty())sc->m_items[k] = v;
}

void    nrdpx_config_t::add_sect(nrdpx_section_t * sc)
{
    if(!sc) return;
    nrdpx_section_t * esc = find_section(sc->m_name,false);
    if(!esc)
    {
        //Add new item
        m_sects.push_back(sc);
    }
    else
    {
        //Extend/Update existing item
        nrdpx_section_t::items_t::iterator v=sc->m_items.begin();
        for(;v != sc->m_items.end();v++)
        {
            esc->m_items[v->first] = v->second;
        }
        delete sc;
    }
}

bool nrdpx_config_t::load_from_file(const xtl::string& filePath,
                const xtl::string& sClass/*=xtl::snull*/)
{
    
    FILE * fd=fopen(filePath.c_str(), "rb");
    
    fpos_t ps;
    
    if(fd == NULL || fseek(fd, 0, SEEK_END) ||
    fgetpos(fd,&ps) || fseek(fd, 0, SEEK_SET))
    {
        if(fd) fclose(fd);
        return false;
    }
    
    size_t siz  = *((size_t *)&ps),rc;
    char * buf  = (char *)malloc (siz+1);
    rc = fread(buf, siz,1,fd);
    fclose(fd);
    
    if(rc != 1) return false;
    
    buf[siz] = '\0';
    
    char *cc=buf,*ln=NULL;
    nrdpx_section_t *  sc=NULL;
    bool tp=false,cm=false;
    m_class = sClass;
    
    for(;*cc != '\0';cc++)
    {
        if(*cc == '\r' || *cc == '\n')
        {
            if(!cm && !tp && ln && sc){
                add_item(sc,ln,cc - ln);
            }
            ln = NULL; tp = false;
        }
        else if(ln == NULL)
        {
            cm=tp=false;
            if(*cc == '[')
            {
                if(sc){add_sect(sc); sc = NULL;}
                tp = true;
            }
            else if(*cc == ';' || *cc == '#')
            {
                cm = true;
            }
            ln = cc;
        }
        else if(tp)
        {
            if(!sc && ln &&  *cc == ']' )
            {
                *cc ='\0'; ln++;
                if(ln != cc && (m_class.empty() ||
                !strnicmp(m_class.c_str(),ln,m_class.size())))
                {
                    sc = new nrdpx_section_t(ln);
                }
            }
        }
    }
    
    //Add last section
    if(sc) add_sect(sc);
    
    return true;
}

void nrdpx_config_t::clear()
{
    for(sections_t::iterator i=m_sects.begin();
    i != m_sects.end(); i++)
    {
        delete *i;
    }
    
    m_sects.clear();
}
