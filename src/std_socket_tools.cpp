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
 * filename: std_socket_tools.c
 */


#include "std_socket_tools.h"
#include "event_log.h"
#include "std_select_tools.h"

#include <unistd.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <type_traits>

#define GENERIC_FAIL STD_ERR_MK(e_std_err_COM,e_std_err_code_PARAM,0)
#define RET_ERRNO STD_ERR_FROM_ERRNO(e_std_err_COM,e_std_err_code_PARAM)

static const size_t STD_MAX_EINTR = 10;

static const size_t SOCK_CONNECT_TIMEOUT_DEFAULT_MS=30*1000;

typedef union socket_addr_u {
        struct sockaddr serveraddr;
        struct sockaddr_un un_addr;
        struct sockaddr_in in_addr;
        struct sockaddr_in6 in6_addr;

} socket_addr_t;

static void _fill_unix_addr (const std_socket_address_t *in_saddr,
                             socket_addr_t *out_saddr,
                             socklen_t *len) {
    struct sockaddr_un * unix_addr = (struct sockaddr_un *)out_saddr;
    memset(unix_addr, 0, sizeof(*unix_addr));
    unix_addr->sun_family = AF_UNIX;
    strncpy(unix_addr->sun_path, in_saddr->address.str,
            sizeof(unix_addr->sun_path)-1);
    *len = SUN_LEN(unix_addr);
}

static void _fill_inet4_addr (const std_socket_address_t* in_saddr,
                              socket_addr_t *out_saddr,
                              socklen_t *len) {
    out_saddr->in_addr = in_saddr->address.inet4addr;
    out_saddr->in_addr.sin_family = AF_INET;
    *len = sizeof (out_saddr->in_addr);
}

static void create_s_sock_addr(const std_socket_address_t *in_saddr,
                               socket_addr_t *out_saddr, socklen_t *len) {

    switch (in_saddr->type) {
        case e_std_sock_UNIX:  return _fill_unix_addr (in_saddr, out_saddr, len);
        case e_std_sock_INET4: return _fill_inet4_addr (in_saddr, out_saddr, len);
        default:
            EV_LOG_ERR(ev_log_t_COM, 0,0, "Unsupported socket domain %d",
                       in_saddr->type);
            break;
    }
}

extern "C" t_std_error std_server_socket_create(std_server_socket_desc_t *desc) {
    if (desc==NULL) return GENERIC_FAIL;
    if (desc->listeners >= STD_SOCK_MAX_LISTENERS) return GENERIC_FAIL;
    if (desc->address.type!=e_std_sock_UNIX) return GENERIC_FAIL;

    int s = socket(AF_UNIX,SOCK_STREAM,0);
    if (s==-1) {
        return RET_ERRNO;
    }
    socklen_t len =0;

    socket_addr_t addrs;
    create_s_sock_addr(&desc->address,&addrs,&len);

    int rc = 0;

    if (desc->address.type==e_std_sock_UNIX) {
        unlink(addrs.un_addr.sun_path);
    }

    rc = bind(s,(struct sockaddr *)&addrs,len);
    if (rc<0) {
        close(s);
        return RET_ERRNO;
    }
    rc = listen(s,desc->listeners);
    if (rc!=0) {
        EV_LOG_ERR(ev_log_t_NPU,0,0,
            "failed to set listen #%u on socket %d ignored",
            desc->listeners,s);
    }
    desc->socket = s;
    return STD_ERR_OK;
}

extern "C" t_std_error std_sock_set_nonblock(int fd, bool nonblock) {
    int on = nonblock ? 1 : 0;
    int rc = ioctl(fd, FIONBIO, (char *)&on);
    if (rc==-1) return STD_ERR_FROM_ERRNO(e_std_err_COM,
                                 e_std_err_code_FAIL);
    return STD_ERR_OK;
}

extern "C" t_std_error std_sock_connect_with_timeout(std_socket_address_t *addr,int *sock,
        size_t ms_wait) {

    if (sock==NULL) return GENERIC_FAIL;
    if (addr->type!=e_std_sock_UNIX) {
        return GENERIC_FAIL;
    }

    int s = socket(AF_UNIX,SOCK_STREAM,0);
    if (s==-1) {
        return RET_ERRNO;
    }

    t_std_error rc = GENERIC_FAIL;

    do {
        if ((rc=std_sock_set_nonblock(s,true))!=STD_ERR_OK) {
            break;
        }
        socket_addr_t socka;
        socklen_t len = 0;
        create_s_sock_addr(addr,&socka,&len);

        rc = GENERIC_FAIL;
        int trc = connect(s,(struct sockaddr *)&socka,len);

        if (trc==-1) {
            if (errno==EINTR) continue;
            if (errno!=EINPROGRESS) break;
        }

        struct timeval tv;
        tv.tv_sec = ms_wait / 1000;     //extract the second part of the number
        tv.tv_usec = (ms_wait % 1000) * 1000;    //convert from milli to micro

        fd_set fds;
        int max_fd = -1;

        std_sel_adds_set(&s,1,&fds,&max_fd,true);

        if (std_select_ignore_intr(max_fd+1,NULL,&fds,NULL,&tv,&rc) < 1 ||
                rc!=STD_ERR_OK) {
            if (rc==0) rc=STD_ERR(COM,FAIL,0);
            break;
        }

        *sock = s;

        if ((rc=std_sock_set_nonblock(s,false))!=STD_ERR_OK) {
            break;
        }

        return STD_ERR_OK;
    } while (0);
    close(s);

    return rc;
}

extern "C" t_std_error std_sock_connect(std_socket_address_t *addr,int *sock) {
    return std_sock_connect_with_timeout(addr,sock,SOCK_CONNECT_TIMEOUT_DEFAULT_MS);
}

extern "C" t_std_error std_sock_set_sndbuf(int fd, int len) {
    int rc = setsockopt(fd,SOL_SOCKET,SO_SNDBUFFORCE,&len,sizeof(len));
    return (rc!=0) ? RET_ERRNO : STD_ERR_OK;
}

extern "C" t_std_error std_sock_set_rcvbuf(int fd, int len) {
    int rc = setsockopt(fd,SOL_SOCKET,SO_RCVBUFFORCE,&len,sizeof(len));
    return (rc!=0) ? RET_ERRNO : STD_ERR_OK;
}

extern "C" t_std_error std_sock_create_pair(e_std_socket_domain_t domain, bool stream, int fd[2]){
    if(domain != e_std_sock_UNIX){
        return GENERIC_FAIL;
    }
    int type = stream ? SOCK_STREAM : SOCK_DGRAM;
    int rc = socketpair(AF_UNIX,type,0,fd);
    return (rc != 0) ? RET_ERRNO : STD_ERR_OK;
}

static int _sock_domain_map (e_std_socket_domain_t std_domain) {
    switch (std_domain) {
        case e_std_sock_INET4:  return AF_INET;
        case e_std_sock_UNIX:   return AF_UNIX;
        default:
            EV_LOG_ERR(ev_log_t_COM, 0,0, "Unknown socket domain %d", std_domain);
            return -1;
    }
}

static int _sock_type_map (e_std_sock_type_t std_type) {
    switch (std_type) {
        case e_std_sock_type_STREAM: return SOCK_STREAM;
        case e_std_sock_type_DGRAM:  return SOCK_DGRAM;
        default:
            EV_LOG_ERR(ev_log_t_COM, 0,0, "Unknown socket type %d", std_type);
            return -1;
    }
}

extern "C" t_std_error std_sock_addr_from_ip_str (e_std_socket_domain_t domain,
                                                  const char* ipstr, int port,
                                                  std_socket_address_t* out_saddr) {
    switch (domain) {
        case e_std_sock_INET4:
            out_saddr->address.inet4addr.sin_family = AF_INET;
            if (inet_aton (ipstr, &out_saddr->address.inet4addr.sin_addr) == 0) {
                EV_LOG_ERR (ev_log_t_COM, 0, 0, "Invalid IP address string");
                return GENERIC_FAIL;
            }
            break;
            // Add IPv6 in future
        default:
            return GENERIC_FAIL;
    }
    out_saddr->addr_type = e_std_socket_a_t_INET; // Common for all INET families
    out_saddr->type = domain;
    out_saddr->address.inet4addr.sin_port = htons (port);
    return STD_ERR_OK;
}

extern "C" t_std_error std_socket_create (e_std_socket_domain_t std_domain,
                                          e_std_sock_type_t std_type,
                                          int protocol,
                                          const std_socket_address_t* bind,
                                          int* fd ) {
    int s = socket (_sock_domain_map (std_domain), _sock_type_map (std_type),
                    protocol);
    if (s==-1) {
        EV_LOG_ERR(ev_log_t_COM, 0,0, "Socket create failed %d", errno);
        return RET_ERRNO;
    }
    *fd = s;
    if (bind != NULL) {
        return std_socket_bind (s, bind);
    }
    return STD_ERR_OK;
}

extern "C" t_std_error std_socket_bind (int fd, const std_socket_address_t* bind_addr) {
    socklen_t sa_len;
    socket_addr_t sa;
    create_s_sock_addr(bind_addr, &sa, &sa_len);

    auto n = bind (fd, (struct sockaddr*) &sa, sa_len);
    return (n < 0) ? RET_ERRNO: STD_ERR_OK;
}

typedef ssize_t (*__handler)(int sockfd, struct msghdr *msg, int flags);

static inline bool is_errno(int rc, int err) {
    return rc==-1 && err == errno;
}

static inline ssize_t total_len(register std_socket_msg_t *msg) {
    register size_t total = 0;
    for ( register size_t ix = 0,mx = msg->msg_iovlen; ix < mx ; ++ix ) {
        total+=msg->msg_iov[ix].iov_len;
    }
    return total;
}

void fix_offset(std_socket_msg_t *msg, ssize_t offset) {
    for ( size_t mx = msg->msg_iovlen,ix=0; ix < mx && offset > 0; ++ix ) {
        offset -= msg->msg_iov[0].iov_len;

        if (offset < 0) {
            offset += msg->msg_iov[0].iov_len;
            msg->msg_iov[0].iov_base = ((char*)msg->msg_iov[0].iov_base)+ offset;
            msg->msg_iov[0].iov_len -= offset;
            break;
        }

        msg->msg_iov = msg->msg_iov+1;
        --msg->msg_iovlen;

    }
}

template <typename T>
static ssize_t _process_op(T handler,int fd, std_socket_msg_t *msg,
        std_socket_transit_flags_t flags ,size_t time_wait, t_std_error * err) {

    ssize_t total = 0;
    ssize_t _total = total_len(msg);

    bool send = (void*)handler!=(void*)recvmsg;

    size_t fail_retry = STD_MAX_EINTR;

    struct iovec _iov[msg->msg_iovlen];

    struct timeval tv;
    tv.tv_sec = time_wait/1000; //milli to sec
    tv.tv_usec = (((decltype(tv.tv_sec))(time_wait% 1000))) * 1000;    //convert the current msec to usec

    int _flags = 0;
    if ((flags&std_socket_transit_f_NONBLOCK)!=0) _flags+=MSG_DONTWAIT;
    bool send_all = ((flags&std_socket_transit_f_ALL)!=0);

    std_socket_msg_t _msg;
    fd_set set;
    struct timeval tmp_tv = tv;

    while (total != _total) {
        size_t cp = msg->msg_iovlen * sizeof(*msg->msg_iov);

        memcpy(_iov,msg->msg_iov,cp); //copy the array of pointers
        _msg = *msg;
        _msg.msg_iov = &_iov[0];
        fix_offset(&_msg,total);

        int rc = handler(fd,&_msg ,_flags);

        if (is_errno(rc,EINTR)) {
            --fail_retry;
            if (fail_retry < 0) {
                if (err!=NULL) *err = STD_ERR(COM,FAIL,errno);
                return rc;
            }
            continue;
        }
        if (is_errno(rc,EWOULDBLOCK) || is_errno(rc,EAGAIN)) {
            FD_ZERO(&set);
            FD_SET(fd,&set);

            int res = std_select_ignore_intr(fd+1,!send ? &set : NULL,
                    send ? &set : NULL, NULL, &tmp_tv,err);
            if (res<=0) {
                if (err!=NULL) *err = STD_ERR(COM,FAIL,errno);
                return -1;
            }
            continue;
        }

        if (rc==-1) {
            if (err!=NULL) *err = STD_ERR(COM,FAIL,errno);
            return rc;
        }

        if (rc==0) {
            if (!send_all) break;
            if (total==_total) break;

            if (err!=NULL) *err = STD_ERR(COM,FAIL,errno);
            return rc;
        }
        total+=rc;

        if (!send_all) break;
        fail_retry = STD_MAX_EINTR;
    }

    if (err!=NULL) *err = STD_ERR_OK;
    return total;

}

ssize_t std_socket_op(std_socket_transit_op_t op, int fd, std_socket_msg_t *msg,
        std_socket_transit_flags_t flags ,size_t time_wait, t_std_error * rc) {
    if (op==std_socket_transit_o_READ) return _process_op(recvmsg,fd,msg,flags,time_wait,rc);
    if (op==std_socket_transit_o_WRITE) return _process_op(sendmsg,fd,msg,flags,time_wait,rc);
    if (rc!=NULL) *rc = STD_ERR(COM,PARAM,0);
    return -1;
}

