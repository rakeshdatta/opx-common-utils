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
 * filename: std_shlib.c
 */


/**
 *      @file  std_shlib.c
 *      @brief Implementation of (code) library management using the Linux dlfcn API.
 */

#include "std_shlib.h"
#include "event_log.h"
#include <dlfcn.h>

t_std_error std_shlib_load(const char* shlib_name, std_shlib_hndl *hndl,
        std_shlib_func_map_t func_map[], size_t func_map_sz)
{
    t_std_error rc = STD_ERR_OK;
    size_t iter = 0;
    if (NULL == hndl || NULL == shlib_name) {
        return STD_ERR(COM,FAIL,EINVAL);
    }
    *hndl = STD_SHLIB_INVALID_HNDL;
    if (std_shlib_is_loaded(shlib_name)) {
        return STD_ERR(COM,FAIL,EEXIST);
    }
    *hndl = dlopen(shlib_name, RTLD_NOW | RTLD_LOCAL);
    if (NULL == *hndl) {
        *hndl = STD_SHLIB_INVALID_HNDL;
        EV_LOG_ERR(ev_log_t_COM,0,"COM-SHLIB","%s not found",
                    shlib_name);
        return STD_ERR(COM,FAIL,ENOENT); // no entity
    }

    for (iter = 0; iter < func_map_sz; ++iter) {
        if (NULL == func_map[iter].name) {
            // Ignore NULL names - this is useful to optionally load some functions...
            *func_map[iter].pp_func = NULL;
            continue;
        }
        (void)dlerror(); // reset error string, as per dlsym man page
        *func_map[iter].pp_func = dlsym(*hndl, func_map[iter].name);
        // Note. As per dlsym man page, in case of dlsym failure, we have dlerror() != NULL
        // Keep load all functions, in order to allow the user app to figura out which functions could not be mapped.
        if (dlerror() != NULL) {
            *func_map[iter].pp_func = NULL;
            EV_LOG_INFO(ev_log_t_COM,0,"COM-SHLIB","%s no such function[%d]: %s",
                        shlib_name, iter,func_map[iter].name);
            rc =  STD_ERR(COM,FAIL,EFAULT);
        }
    }
    if (rc != STD_ERR_OK) {
        (void)dlclose(*hndl);
        *hndl = STD_SHLIB_INVALID_HNDL;
    }
    return rc;
}

t_std_error std_shlib_unload(const std_shlib_hndl hndl)
{
    if (STD_SHLIB_INVALID_HNDL == hndl) {
        return STD_ERR(COM,FAIL,EINVAL);
    }
    if (0 == dlclose(hndl)) {
        return STD_ERR_OK;
    }
    return STD_ERR(COM,FAIL,ENOENT);
}

bool std_shlib_is_loaded(const char* shlib_name)
{
    void *hndl = dlopen(shlib_name, RTLD_NOW | RTLD_NOLOAD);
    if (NULL != hndl) {
        // Decrement the ref count increased by dlopen
        dlclose(hndl);
        return true;
    }
    return false;
}
