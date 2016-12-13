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
 * filename: std_select.c
 */


#include "std_select_tools.h"

#include <sys/select.h>
#include <stdlib.h>
#include <sys/ioctl.h>

ssize_t std_select_ignore_intr(int fd, fd_set *r, fd_set *w, fd_set *e,
    struct timeval *tv, t_std_error *err) {
    int retries = 100;
    while (retries-- > 0 ) {
        int rc = select(fd,r,w,e,tv);
        if (rc>=0) {
            if (err!=NULL) *err = STD_ERR_OK;
            return rc;
        }
        if (rc==-1 && errno!=EINTR) {
            if (err!=NULL) *err = STD_ERR_FROM_ERRNO(e_std_err_COM,
                                                    e_std_err_code_FAIL);

            return rc;
        }
        //rc==-1 and EINTR repeat
    }
    if (err!=NULL) *err = STD_ERR_MK(e_std_err_COM,e_std_err_code_FAIL,EINTR);
    return -1;
}

void std_sel_adds_set(int *fd,unsigned int len, fd_set *set, int * max_fds, bool init) {
    int maxfd=-1;
    if (max_fds==NULL) max_fds=&maxfd;

    if (init) { FD_ZERO(set); }

    size_t ix = 0;
    size_t mx = len;

    for ( ; ix < mx ; ++ix ) {
        FD_SET(fd[ix],set);
        if (fd[ix] > *max_fds) *max_fds=fd[ix];
    }

}

