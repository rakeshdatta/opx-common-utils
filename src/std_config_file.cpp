/*
 * Copyright (c) 2016 Dell Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 * FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 * See the Apache Version 2.0 License for specific language governing
 * permissions and limitations under the License.
 */

/*
 * filename: std_config_file.cpp
 */

/*
 * std_config_file.c
 */

#include "std_config_file.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <vector>
#include <string>
#include <map>
#include <memory>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

typedef std::map<std::string,std::string> map_of_strings_t;
typedef std::map<std::string,map_of_strings_t> config_file_t;

struct cfg_file_data {
    config_file_t map;
};

static inline std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(),
std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}

static inline std::string &rtrim(std::string &s)  {
        s.erase(std::find_if(s.rbegin(), s.rend(),
std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

static inline std::string &trim(std::string &s) {
        return ltrim(rtrim(s));
}

t_std_error std_config_file_create(std_cfg_file_handle_t *handle) {
    try {
        cfg_file_data *d = new cfg_file_data;
        *handle =  static_cast<void*>(d);
    } catch (...) {
        return STD_ERR(COM,FAIL,ENOMEM);
    }
    return STD_ERR_OK;
}

static std::string handle_section(const std::string &section,cfg_file_data *d) {
    std::string::size_type p = section.find('[');
    std::string::size_type last = section.find(']');

    return section.substr(p+1,last - (p+1));
}

static void handle_keyfield(const std::string &line,
        const std::string &section,cfg_file_data *d) {
    std::string::size_type len = line.find('=');
    std::string key = (line.substr(0,len));
    std::string value = (line.substr(len+1));

    d->map[section][trim(key)] = trim(value);
}

const int BUFF_LEN=1024;

static t_std_error load_file(cfg_file_data *d, const char *name) {
    //garbage collector
    std::vector<char> datab;

    char *line = NULL;
    t_std_error rc = STD_ERR(COM,FAIL,ENOMEM);
    try {
        datab.resize(BUFF_LEN);
        line = &(datab[0]);
    } catch (...) {
        return rc;
    }

    FILE * fp = fopen (name,"r");
    if (fp==NULL) {
        return STD_ERR(COM,FAIL,ENOENT);
    }

    std::string section;
    rc = STD_ERR_OK;
    while ( (line=fgets(line,BUFF_LEN,fp))) {
        try {
            std::string data = line;
            if ((data.find('[')==0) && (data.find(']')!=std::string::npos)) {
                 section = handle_section(data,d);
                continue;
            }
            if (data.find('=')==std::string::npos) {
                continue;
            }
            handle_keyfield(data,section,d);
        } catch (...) {
            rc = STD_ERR(COM,FAIL,ENOMEM);
            break;
        }
    }

    fclose(fp);
    return rc;
}

t_std_error std_config_file_open(std_cfg_file_handle_t *handle, const char *name) {
    t_std_error rc = std_config_file_create(handle);

    if (rc!=STD_ERR_OK) return rc;

    cfg_file_data *d = static_cast<cfg_file_data*>(*handle);

    rc = load_file(d,name);

    if (rc!=STD_ERR_OK) {
        std_config_file_close(*handle);
    }

    return rc;
}

size_t std_config_file_get_num_keys(std_cfg_file_handle_t handle,
        const char *group) {
    cfg_file_data *d = static_cast<cfg_file_data*>(handle);
    config_file_t::const_iterator cfg = d->map.find(group);
    if (cfg==d->map.end()) return 0;
    return cfg->second.size();
}

t_std_error std_config_file_get_keys(std_cfg_file_handle_t handle,
        const char *group, const char **key_list, size_t *len) {

    cfg_file_data *d = static_cast<cfg_file_data*>(handle);
    config_file_t::const_iterator cfg = d->map.find(group);
    if (cfg==d->map.end()) return STD_ERR(COM,PARAM,0);

    if (cfg->second.size()>*len)return STD_ERR(COM,TOOBIG,0);
    size_t ix = 0;
    map_of_strings_t::const_iterator it = cfg->second.begin();
    for ( ;it != cfg->second.end() ; ++it ) {
        key_list[ix++] = it->first.c_str();
    }
    *len = ix;
    return STD_ERR_OK;
}

const char * std_config_file_get(std_cfg_file_handle_t handle,
                const char *group, const char *key) {
    cfg_file_data *d = static_cast<cfg_file_data*>(handle);
    config_file_t::const_iterator cfg = d->map.find(group);
    if (cfg!=d->map.end()) {
        map_of_strings_t::const_iterator str = cfg->second.find(key);
        if (str!=cfg->second.end()) return str->second.c_str();
    }
    return NULL;
}

t_std_error std_config_file_set(std_cfg_file_handle_t handle,
                const char *group, const char *key, const char *str) {
    cfg_file_data *d = static_cast<cfg_file_data*>(handle);
    try {
        d->map[group][key] = str;
        return STD_ERR_OK;
    } catch (...) {}
    return STD_ERR(COM,FAIL,ENOMEM);
}


t_std_error std_config_file_write(std_cfg_file_handle_t handle,
                const char *name) {
    FILE *fp = fopen(name,"w");

    if (fp==NULL) return STD_ERR(COM,FAIL,errno);

    cfg_file_data *d = static_cast<cfg_file_data*>(handle);

    config_file_t::const_iterator cfg = d->map.begin();

    const char *open_s="[";
    const char *close_s="]";
    const char *equals="=";

    t_std_error rc = STD_ERR_OK;
    try {
        for ( ; cfg!=d->map.end(); ++cfg) {
            std::string buff;
            buff=open_s;
            buff+=cfg->first;
            buff+=close_s;
            buff+="\n";
            if (fwrite(buff.c_str(),1,buff.length(),fp)!=buff.length()) {
                rc = STD_ERR(COM,FAIL,errno);
                break;
            }
            map_of_strings_t::const_iterator str = cfg->second.begin();
            for ( ; str!=cfg->second.end(); ++str) {
                buff = str->first;
                buff+= equals;
                buff+=str->second;
                buff+="\n";
                if (fwrite(buff.c_str(),1,buff.length(),fp)!=buff.length()) {
                    rc = STD_ERR(COM,FAIL,errno);
                    break;
                }
            }
        }
    } catch (...) {
        rc = STD_ERR(COM,FAIL,ENOMEM);
    }
    fclose(fp);
    return rc ;
}

t_std_error std_config_file_close(std_cfg_file_handle_t handle) {
    cfg_file_data *d = static_cast<cfg_file_data*>(handle);
    delete d;
    return STD_ERR_OK;

}


