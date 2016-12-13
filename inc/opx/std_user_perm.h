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
 * std_user_perm.h
 *
 *  Created on: Sep 11, 2015
 */

#ifndef COMMON_UTILS_INC_STD_USER_PERM_H_
#define COMMON_UTILS_INC_STD_USER_PERM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "std_error_codes.h"

/**
 * Change the ownership of a file on the file system
 * @param path the path and filename of the element being changed
 * @param name the user name to change the path element ownership to
 * @param group the name of the group that should own the file.  If NULL the group parameter will be ignored
 * @return STD_ERR_OK on success otherwise a failure return code
 */
t_std_error std_user_chown(const char *path,const char *name, const char *group);


/**
 * Set permissions on a file/path element given the permission string
 * @param path The path to the file
 *
 * @param perm_str the string of permissions including:
 *          [ugoa]*([-+=]([rwxXst]*|[ugo]))+
 *
 * @return STD_ERR_OK if successful otherwise an error
 */
t_std_error std_user_chmod(const char *path, const char *perm_str);


#ifdef __cplusplus
}
#endif

#endif /* COMMON_UTILS_INC_STD_USER_PERM_H_ */
