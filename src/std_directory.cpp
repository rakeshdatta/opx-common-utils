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
 * std_directory.cpp
 *
 *  Created on: Apr 20, 2015
 */

#include "std_directory.h"
#include "std_error_codes.h"

#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <string>

#ifdef __cplusplus
extern "C" {
#endif

t_std_error std_dir_init(const char * ent_name, std_dir_handle_t *handle) {
    DIR *d = opendir(ent_name);
    if (d==NULL) {
        if (errno==ENOENT) return STD_ERR(COM,NEXIST,0);
        return STD_ERR(COM,FAIL,errno);
    }
    *handle = d;
    return STD_ERR_OK;
}


void std_dir_close(std_dir_handle_t handle) {
    DIR *d = (DIR*)handle;
    closedir(d);
}


t_std_error std_dir_iterate(const char * ent_name,bool (*cb)(const char *name,
        std_dir_file_TYPE_t type,void *context),void *context, bool recurse) {
    std_dir_handle_t h;
    t_std_error rc = STD_ERR_OK;
    if ((rc=std_dir_init(ent_name,&h))!=STD_ERR_OK) {
        return rc;
    }
    int res = 0;
    struct dirent *entry;
    //since we don't know the size of the d_name field.. according to the docs, will allocate a buffer
    //at least large enough to handle 1k though MAX path is only 255
    static const size_t SUPER_MAX_UNREACHABLE_LEN = 1024;
    size_t len = offsetof(struct dirent, d_name) + SUPER_MAX_UNREACHABLE_LEN;

    /**
     * I really don't want to use malloc because normally I would use the stack but
     * in this case, the readdir_r man pages lead me to an understanding that the length
     * was non-deterministic and since there is recursion possible,
     * I selected malloc
     */
    entry = (struct dirent*) malloc(len);
    if (entry==NULL) {
        std_dir_close(h);
        return STD_ERR(COM,NOMEM,0);
    }

    do {
        struct dirent *result=NULL;
        res = readdir_r((DIR *)h, entry, &result);
        if ((result==NULL) || (res!=0)) break;

        std_dir_file_TYPE_t t = std_dir_file_T_OTHER;

        if (result->d_type & DT_LNK) t = std_dir_file_T_LINK;
        if (result->d_type & DT_REG) t = std_dir_file_T_FILE;
        if (result->d_type & DT_DIR) t = std_dir_file_T_DIR;

        if (t==std_dir_file_T_DIR && strcmp(result->d_name,".")==0) continue;
        if (t==std_dir_file_T_DIR && strcmp(result->d_name,"..")==0) continue;

        std::string pathname = ent_name;
        pathname += "/";
        pathname+=result->d_name;

        if (t==std_dir_file_T_DIR && recurse) {
            (void)std_dir_iterate(pathname.c_str(),cb,context,recurse);
        }
        if (!cb(pathname.c_str(),t,context)) break;
    } while (res==0);

    free(entry);

    std_dir_close(h);
    return STD_ERR_OK;
}

}
