/*
 * Copyright (c) 2018 Dell Inc.
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
#include "std_system.h"
#include "event_log.h"


#include <unistd.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#define STD_MAX_EINTR (10)

#define GENERIC_FAIL STD_ERR_MK(e_std_err_COM,e_std_err_code_PARAM,0)
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

typedef ssize_t (*op_on_set)(int, const struct iovec *, int);

static ssize_t _std_op_set(int fd, void**data, size_t *data_lens,  size_t set_len, bool require_all, t_std_error *err,
        op_on_set op) {
    struct iovec *_piov, _iovs[set_len];
    _piov = _iovs;

    size_t _iovs_len = 0;

    size_t _total_len = 0;

    size_t _ix = 0;
    for (; _ix < set_len ; ++_ix ) {
        if (data_lens[_ix]==0) continue;

        _iovs[_iovs_len].iov_base = data[_ix];
        _iovs[_iovs_len].iov_len = data_lens[_ix];

        ++_iovs_len;
        _total_len += data_lens[_ix];
    }
    set_len = _iovs_len;


    int total = 0;
    int fail_retry = 10;

    while (set_len>0) {
        ssize_t rc = op(fd,_piov,set_len);
        if (rc==-1 && errno==EINTR) {
            --fail_retry;
            if (fail_retry < 0) {
                if (err!=NULL) *err = STD_ERR(COM,FAIL,errno);
                return rc;
            }
            continue;
        }

        if (rc==-1) {
            if (err!=NULL) *err = STD_ERR(COM,FAIL,errno);;
            return rc;
        }
        if (rc==0) return total;

        total+=rc;

        if (_total_len==(size_t)total) break;

        while (rc>0) {
            if ((ssize_t)_piov[0].iov_len < rc ) {
                rc -= _piov[0].iov_len;
                ++_piov;
                --set_len;
            } else {
                _piov[0].iov_base= ((char*)_piov[0].iov_base) + rc;
                _piov[0].iov_len-=rc;
                rc = 0;
            }
        }

        if (!require_all) break;
        fail_retry = 10;
    }

    if (err!=NULL) *err = STD_ERR_OK;
    return total;
}

/**
 * @brief   write a set of buffers and give options to continue to write
 *          or finish after the first error that can be ignored..  ignores eintr
 * @param   fd[in] the file descriptor to use
 * @param   data[in] the buffer to write
 * @param   len[in] the length of data
 * @param   require_all[in] true if should send all data otherwise false
 * @param   err the error code if provided
 * @return  length written.  On error returns -1 and can check the return code for the error reason
 */
ssize_t std_write_set(int fd, void**data, size_t *data_lens,  size_t set_len, bool require_all, t_std_error *err) {
    return _std_op_set(fd,data,data_lens,set_len,require_all,err,writev);
}

ssize_t std_read_set(int fd, void**data, size_t *data_lens,  size_t set_len, bool require_all, t_std_error *err) {
    return _std_op_set(fd,data,data_lens,set_len,require_all,err,readv);
}


t_std_error std_netns_fd_open (const char *net_namespace, const char *filename, int flags,
                               int* fd)
{
    t_std_error rc = GENERIC_FAIL;
    int crt_ns_handle = STD_INVALID_FD;

    rc = std_sys_set_netns(net_namespace, &crt_ns_handle);
    if (rc != STD_ERR_OK) {
        EV_LOG_ERR (ev_log_t_COM,ERR,"file", "Name space set error %s", (net_namespace != NULL) ? net_namespace : "n/a");
        return rc;
    }


    *fd = open(filename, flags);
    if (*fd < 0) {
        rc = STD_ERRNO;
        EV_LOGGING(ev_log_t_COM, ERR, "file", "Cannot open %s::%s errno=%s(%d)",
                net_namespace, filename, strerror (errno), errno);
        // continue, we must return to regular name space
    } else {
        rc = STD_ERR_OK;
    }
    (void)std_sys_reset_netns(&crt_ns_handle);

    return rc;
}

