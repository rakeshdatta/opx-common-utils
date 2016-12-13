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
 * filename: std_select_tools.h
 */


/**
 *       @file  std_select_tools.h
 *      @brief  standard select operations and fdset tools
 *
 *   @internal
 *     Created  05/11/2014
 *     Company  DELL
 *
 * =====================================================================================
 */

#ifndef __STD_SELECT_TOOLS_H
#define __STD_SELECT_TOOLS_H

#include "std_error_codes.h"
#include "std_type_defs.h"

#include <sys/select.h>
#include <stddef.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief   add a array of fds to a select set
 * @param   fd file desctriptor array
 * @param   length of the array
 * @param   set the fd_set containing the list
 * @param   max_fds update the max fd if specified based on array of fds or NULL
 * @param   init true if you want the fd_set initialized to zero otherwise adds
 * @return  none
 */
void std_sel_adds_set(int *fd,unsigned int len, fd_set *set, int * max_fds, bool init);


/**
 * @brief   select on the fd_sets and return an the results
 * @param   maxfd the maximum file descriptor set in all sets + 1
 * @param   r is the read fd_set
 * @param   w is the write fd_set
 * @param   e is the error fd_set
 * @param   tv[in/out] is the timeval to wait for (will be updated by function)
 * @param   err[out] is the error code - can pass in NULL if want to ignore
 * @return  a return code of >0 if there are fds set, 0 if timeout, -1 on error
 */
ssize_t std_select_ignore_intr(int maxfd, fd_set *r, fd_set *w, fd_set *e,
    struct  timeval*tv, t_std_error *err);

#ifdef __cplusplus
}
#endif


#endif

