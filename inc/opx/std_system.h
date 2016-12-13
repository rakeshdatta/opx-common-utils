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
 * filename: std_system.h
 * provides system level utility functions
 **/


#ifndef __STD_SYSTEM_H
#define __STD_SYSTEM_H

#include "std_error_codes.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 * Find where the sysfs is currently mounted
 *
 * @param sysfs[out] char array where the path will be returned
 * @param len[in] the maximum lenght of the sysfs buffer
 *
 * @return t_std_error
 *
**************************************************************************/
t_std_error std_sys_sysfs_path_get(char *sysfs, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* __STD_SYSTEM_H */
