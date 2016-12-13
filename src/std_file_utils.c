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
 * filename: std_file_utils.c
 */

#include "std_file_utils.h"
#include <unistd.h>

#define STD_MAX_EINTR (10)


#define STD_ERRNO STD_ERR_FROM_ERRNO(e_std_err_COM, e_std_err_code_FAIL)


typedef ssize_t (*fo)(int fd, void *data, size_t len);

inline bool is_errno(int rc, int id) {
    return rc==-1 && errno==id;
}

static int file_op(fo oper,int fd, void*data, int len,
        bool require_all, t_std_error *err) {

    int total = 0;
    int fail_retry = STD_MAX_EINTR;

    while (total != len) {
        int rc = oper(fd,((char*)(data)) + total,len-total);
        if (is_errno(rc,EINTR)) {
            --fail_retry;
            if (fail_retry < 0) {
                if (err!=NULL) *err = STD_ERRNO;
                return rc;
            }
            continue;
        }

        if (rc==-1) {
            if (err!=NULL) *err = STD_ERRNO;
            return rc;
        }
        if (rc==0) return total;
        total+=rc;
        if (!require_all) break;
        fail_retry = STD_MAX_EINTR;
    }

    if (err!=NULL) *err = STD_ERR_OK;
    return total;
}


int std_read(int fd, void*data, int len, bool require_all, t_std_error *err) {
    return file_op(read,fd,data,len,require_all,err);
}

int std_write(int fd, void*data, int len, bool require_all, t_std_error *err) {
    return file_op((fo)write,fd,data,len,require_all,err);
}

int std_fd_copy(int fdout, int fdin, t_std_error *err) {
    char buff[100];

    int rc = std_read(fdin, buff,sizeof(buff)-1,false,err);
    if (rc==-1 || rc==0) return rc;

    rc = std_write(fdout,buff,rc,true,err);
    return rc;
}

static void close_fds(int *fds, int len) {
    int ix = 0;
    for ( ; ix < len ; ++ix ) {
        close(fds[ix]);
    }
}

t_std_error std_file_clone_fds(int *fclones, int *fin, int len) {
    int ix = 0;
    for ( ; ix < len ; ++ix) {
        fclones[ix] = dup(fin[ix]);
        if (fclones[ix]==-1) {
            close_fds(fclones,ix);
            return STD_ERRNO;
        }
    }
    return STD_ERR_OK;
}


t_std_error std_redir_stdoutin(int fd) {
    int sfds[]={STDIN_FILENO,STDOUT_FILENO};
    int ofds[2];
    t_std_error serr = std_file_clone_fds(ofds,sfds,sizeof(ofds)/sizeof(*ofds));
    if (serr!=STD_ERR_OK) return serr;

    do {
        int rc = dup2(fd,STDIN_FILENO);
        if (rc==-1) {
            serr = STD_ERRNO;
            break;
        }
        rc = dup2(fd,STDOUT_FILENO);
        if (rc==-1) {
            serr = STD_ERRNO;
            dup2(ofds[0],STDIN_FILENO);
            break;
        }
        serr = STD_ERR_OK;
    } while (0);

    close(ofds[0]); close(ofds[1]);
    return serr;
}

t_std_error std_close (int fd) {
    return (close (fd) < 0) ? (STD_ERRNO): STD_ERR_OK;
}
