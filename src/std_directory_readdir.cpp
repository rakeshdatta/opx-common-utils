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

t_std_error std_dir_iterate(const char * ent_name,bool (*cb)(const char *name,
        std_dir_file_TYPE_t type,void *context),void *context, bool recurse) {
    std_dir_handle_t h;
    t_std_error rc = STD_ERR_OK;
    if ((rc=std_dir_init(ent_name,&h))!=STD_ERR_OK) {
        return rc;
    }
    int res = 0;
    struct dirent *result = nullptr;
    do {
        result = readdir((DIR *)h);
        if (result==nullptr) break;

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

    std_dir_close(h);
    return STD_ERR_OK;
}
