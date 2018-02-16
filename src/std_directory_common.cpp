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


#include "std_directory.h"
#include "std_error_codes.h"

#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <string>

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
