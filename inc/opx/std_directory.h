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
 * std_directory.h
 *
 *  Created on: Apr 20, 2015
 */

#ifndef COMMON_UTILS_INC_STD_DIRECTORY_H_
#define COMMON_UTILS_INC_STD_DIRECTORY_H_

#include "std_error_codes.h"

#include <stdbool.h>

typedef void * std_dir_handle_t;

typedef enum {
    std_dir_file_T_DIR,
    std_dir_file_T_FILE,
    std_dir_file_T_LINK,
    std_dir_file_T_OTHER
} std_dir_file_TYPE_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Open a and return a handle based on the filename specified.  If the directory doesn't exist an error will
 * be returned
 * @param ent_name the name of the directory
 * @param handle a handle that can be used with subsequent calls
 * @return STD_ERR_OK if the directory exists otherwise STD_ERR(COM,NEXIST,errno) if not exist
 * or another more specific return code
 */
t_std_error std_dir_init(const char * ent_name, std_dir_handle_t *handle);

/**
 * Close an open directory handle
 * @param handle the handle to close
 */
void std_dir_close(std_dir_handle_t handle);


/**
 * Open a directory and iterate through its contents calling the provided callback
 * @param ent_name the directory to process
 * @param cb the callback to call on each entry
 * @param context the callback context - to be provided by the caller
 * @param recurse true if you want to traverse all sub directories as well
 * @return STD_ERR_OK if everything went fine
 *            STD_ERR(COM,NEXIST,0) if the directory didn't exist
 *            or another specific return code on an internal error when processing the directories
 */
t_std_error std_dir_iterate(const char * ent_name,bool (*cb)(const char *name,
        std_dir_file_TYPE_t type,void *context),void *context, bool recurse);

#ifdef __cplusplus
}
#endif

#endif /* COMMON_UTILS_INC_STD_DIRECTORY_H_ */
