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
 * std_user_perm.cpp
 *
 *  Created on: Sep 11, 2015
 */

#include "std_user_perm.h"
#include "std_assert.h"
#include "std_mutex_lock.h"
#include "event_log.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <fcntl.h>
#include <grp.h>

#include <memory>
#include <map>

namespace {
size_t get_estimated_buff(int tag) {
    int sz = sysconf(tag);
    if (sz==-1) {
        EV_LOG(ERR,COM,0,"COM-USER-PERM","Failed to get default buffer len for (%d) ",tag);
        sz = 2048;
    } else sz*=2;
    return sz;

}

}

extern "C" t_std_error std_user_chown(const char *path,const char *name, const char *group) {
    STD_ASSERT(path!=nullptr && name!=nullptr);
    size_t sz = get_estimated_buff(_SC_GETPW_R_SIZE_MAX);
    std::unique_ptr<char[]> b(new char [sz]);

    if (b.get()==nullptr) return STD_ERR(COM,NOMEM,0);

    struct passwd pass, *_pass=nullptr;

    if (getpwnam_r(name,&pass,b.get(),sz,&_pass)!=0 || _pass==nullptr) {
        EV_LOG(ERR,COM,0,"COM-USER-PERM","No such user %s",name);
        return STD_ERR(COM,FAIL,errno);
    }

    uid_t uid = _pass->pw_uid;

    gid_t gid = -1;
    if (group!=nullptr) {
        sz = get_estimated_buff(_SC_GETGR_R_SIZE_MAX);
        b = std::unique_ptr<char[]>(new char [sz]);

        struct group grp,*pgrp=nullptr;
        if (getgrnam_r(group,&grp,b.get(),sz,&pgrp)!=0 || pgrp==nullptr) {
            EV_LOG(ERR,COM,0,"COM-USER-PERM","No such group %s",group);
            return STD_ERR(COM,FAIL,errno);
        }
        gid = pgrp->gr_gid;
    }


    if (chown(path,uid,gid)!=0) {
        return STD_ERR(COM,FAIL,errno);
    }
    return STD_ERR_OK;
}

namespace {
static std::map<char,std::map<char,mode_t>> _perm_map {
    { 'a' ,{ { 'r',S_IRUSR|S_IRGRP|S_IROTH },
             { 'w',S_IWUSR|S_IWGRP|S_IWOTH },
             { 'x',S_IXUSR|S_IXGRP|S_IXOTH },
           }
    },
    { 'o' ,{ { 'r',S_IROTH },
             { 'w',S_IWOTH },
             { 'x',S_IXOTH },
           }
    },
    { 'g' ,{ { 'r',S_IRGRP },
             { 'w',S_IWGRP },
             { 'x',S_IXGRP },
           }
    },
    { 'u' ,{ { 'r',S_IRUSR },
             { 'w',S_IWUSR },
             { 'x',S_IXUSR },
           }
    },

};
bool _valid_oper(char elem) {
    return (elem=='=' || elem=='+' || elem=='-');
}

bool _chomp(const char * &perm_str, char &op, mode_t &mod) {
    char user = 'u';
    op = '+';
    char elem = *perm_str;


    if (_perm_map.find(elem)!=_perm_map.end()) {
        user = elem;
        ++perm_str;
        elem = *perm_str;
    }
    if (_valid_oper(elem)) {
        op = elem;
        ++perm_str;
        elem = *perm_str;
    }
    //already known that it exists
    auto it = _perm_map.find(user);
    STD_ASSERT(it!=_perm_map.end());

    auto mod_it = it->second.find(elem);

    if (mod_it == it->second.end()) {
        return false;
    }
    while (true) {
        mod_it = it->second.find(*(perm_str));
        if (mod_it == it->second.end()) break;
        mod |= mod_it->second;
        ++perm_str;
    }

    return true;
}

}

// *          [ugoa]*([-+=]([rwxXst]*|[ugo]))+
extern "C" t_std_error std_user_chmod(const char *path, const char *perm_str) {
    //todo wrap this initialization with a mutex
    static std_mutex_lock_create_static_init_fast(lock);
    std_mutex_lock(&lock);

    static const mode_t all_mode_mask = _perm_map.find('a')->second.find('w')->second |
                                        _perm_map.find('a')->second.find('r')->second |
                                        _perm_map.find('a')->second.find('x')->second ;
    std_mutex_unlock(&lock);

    struct stat buf;
    if (stat(path, &buf)!=0) {
        int err = errno;
        EV_LOG(ERR,COM,0,"COM-USER-PERM","path element doesn't exist %s",path);
        return STD_ERR(COM,FAIL,err);
    }

    STD_ASSERT(path!=nullptr && perm_str!=nullptr);

    mode_t mod = 0;

    while(*perm_str!=0) {
        char op = '+';
        mode_t _ch_mod = 0;
        if (!_chomp(perm_str,op,_ch_mod)) {
            EV_LOG(ERR,COM,0,"COM-USER-PERM","permission string invalid %s",perm_str);
            return STD_ERR(COM,PARAM,*perm_str);
        }
        if (op=='=') {
            mod = _ch_mod;
        } else {
            if (mod==0) mod = buf.st_mode & all_mode_mask;
        }
        if (op=='+') {
            mod |= _ch_mod;
        }
        if (op=='-') {
            mod = mod & (~_ch_mod);
        }
    }
    if (chmod(path,mod)!=0) {
        int err = errno;
        EV_LOG(ERR,COM,0,"COM-USER-PERM","Unable to change the file permissions %s:%x (%d)",path,mod,err);
        return STD_ERR(COM,FAIL,err);
    }
    return STD_ERR_OK;
}
