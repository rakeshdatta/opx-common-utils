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
 * std_socket_service.cpp
 */

#include "std_socket_service.h"
#include "std_mutex_lock.h"
#include "std_thread_pool.h"
#include "std_socket_tools.h"
#include "std_select_tools.h"
#include "std_error_codes.h"
#include "event_log.h"
#include "std_file_utils.h"
#include "std_assert.h"

#include <vector>
#include <set>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

struct std_socket_service_data_t {
    std_thread_pool_handle_t thread_pool;

    int server_ctl_sock[2];

    std_server_socket_desc_t sock;
    std_socket_server_t server_init;
    std_mutex_type_t m_mutex;

    //access by server only
    std::set<int> m_clients;

    fd_set pending;

    //access by callback function
    std::set<int> m_pending_close;

    typedef std::set<int>::iterator fd_iterator;

    int max_fd;
    void drain_server_wakeup(fd_set *set) {
        if (!FD_ISSET(server_ctl_sock[0],set)) return;
        char c;
        std_read(server_ctl_sock[0],&c,sizeof(c),true,NULL); //toss error
    }
    void server_wakeup() {
        char c=0;
        std_write(server_ctl_sock[1],&c,sizeof(c),true,NULL); //toss error
    }

    void setup_max_fd() {
        fd_iterator it = m_clients.begin();
        fd_iterator end = m_clients.end();
        max_fd = sock.socket;
        max_fd = std::max(max_fd,server_ctl_sock[0]);
        for ( ; it != end ; ++it) {
            max_fd = std::max(max_fd,*it);
        }
    }

    void new_conn(int fd) {
        std_mutex_simple_lock_guard m(&m_mutex);
        max_fd = std::max(fd,max_fd);
        server_wakeup();
    }

    void busy(int fd) {
        std_mutex_simple_lock_guard m(&m_mutex);
        FD_CLR(fd,&pending);
    }

    void pend_close(int fd) {
        std_mutex_simple_lock_guard m(&m_mutex);
        m_pending_close.insert(fd);
    }

    void process_closed() {
        std_mutex_simple_lock_guard m(&m_mutex);
        if(m_pending_close.size()==0) return;

        while (!m_pending_close.empty()) {
            fd_iterator it = m_pending_close.begin();
            int fd = *it;
            close(fd);
            FD_CLR(fd,&pending);
            if (m_clients.find(fd)!=m_clients.end()) m_clients.erase(fd);
            m_pending_close.erase(fd);
        }
        setup_max_fd();
    }

    void returned(int fd) {
        std_mutex_simple_lock_guard m(&m_mutex);
        if (m_clients.find(fd)==m_clients.end()) {
            return;
        }
        FD_SET(fd,&pending);
        server_wakeup();
    }

    void cleanup() {
        if (sock.socket!=STD_INVALID_FD) { close(sock.socket); sock.socket = STD_INVALID_FD; }

        if (thread_pool!=NULL) std_thread_pool_delete(thread_pool);

        fd_iterator it = m_clients.begin();
        fd_iterator end = m_clients.end();
        for (; it != end ; ++it ) {
            close(*it);
        }
    }

    ~std_socket_service_data_t() {
        cleanup();
    }
};

struct job_entry_t {
    std_socket_service_data_t *service;
    int fd;
};

#define MAX_QUEUED_CLIENTS (35)

static bool queue_job(std_socket_service_data_t *service, int fd, void (*func)(void *)) {
    if (service->server_init.thread_pool_size>1) {
        job_entry_t *je_context = (job_entry_t*)malloc (sizeof(job_entry_t));
        if (je_context!=NULL) {
            je_context->fd = fd;
            je_context->service = service;
            std_thread_pool_job_t je;
            je.context = je_context;
            je.funct = func;
            je.free_job_func = free;
            if(std_thread_pool_job_add(service->thread_pool,&je)==STD_ERR_OK) {
                return true;
            }
            free(je_context);
        }
        return false;
    }
    job_entry_t je_context;
    je_context.fd = fd;
    je_context.service = service;
    func(&je_context);
    return true;
}

static void process_socket_data(void * context) {
    job_entry_t *p = (job_entry_t*)context;

    if (!p->service->server_init.some_data(p->service->server_init.context,p->fd)) {
        p->service->server_init.del_client(p->service->server_init.context,p->fd);
        p->service->pend_close(p->fd);
        return;
    }
    p->service->returned(p->fd);

}

static void queue_and_add_socket(void * context) {
    job_entry_t *p = (job_entry_t*)context;

    if (!p->service->server_init.new_client(p->service->server_init.context,p->fd)) {
        p->service->pend_close(p->fd);
        return;
    }
    std_mutex_simple_lock_guard m(&p->service->m_mutex);
    p->service->new_conn(p->fd);
    p->service->returned(p->fd);
}

static void process_sockets(std_socket_service_data_t *p, fd_set &rset ) {

    p->drain_server_wakeup(&rset);

    if (FD_ISSET(p->sock.socket,&rset)) {
        struct sockaddr_storage sa;
        socklen_t slen = sizeof(sa);
        int sock = accept(p->sock.socket,(sockaddr*)&sa,&slen);

        if (sock!=STD_INVALID_FD) {
            p->m_clients.insert(sock);
            if (!queue_job(p,sock,queue_and_add_socket)) {
                p->pend_close(sock);
            }
        }
    }

    std_socket_service_data_t::fd_iterator it =p->m_clients.begin();
    std_socket_service_data_t::fd_iterator end =p->m_clients.end();

    for ( ; it != end ; ++it ) {
        if (FD_ISSET(*it,&rset)) {
            p->busy(*it);
            if (!queue_job(p,*it,process_socket_data)) {
                p->pend_close(*it);
            }
        }
    }
}

static void socket_service(void *param) {
    std_socket_service_data_t *p = (std_socket_service_data_t*)param;
    FD_SET(p->sock.socket,&p->pending);
    FD_SET(p->server_ctl_sock[0],&p->pending);
    fd_set rset;
    p->setup_max_fd();

    while(true) {
        p->process_closed();

        rset = p->pending;

        struct timeval tv = { 1,0};

        if ((p->max_fd==STD_INVALID_FD) || (p->sock.socket == STD_INVALID_FD)) {
            EV_LOG(ERR,COM,0,"FATAL","Invalid socket list or other server related corruption.  Must exit");
            break;
        }

        t_std_error serr = STD_ERR_OK;

        int rc = std_select_ignore_intr(p->max_fd+1,&rset,NULL,NULL,&tv,&serr);

        if (rc==-1) {
            //find the bad socket and repair
            if (errno!=EINTR) {
                EV_LOG(ERR,COM,0,"CRIT","Received unknown error number %d",errno);
            }
            size_t ix = 0;
            size_t mx = p->max_fd+1;
            tv.tv_sec=0;
            tv.tv_usec =0;
            fd_set s ;
            for ( ; ix < mx ; ++ix ) {
                if (!FD_ISSET((int)ix,&rset)) continue;
                FD_ZERO(&s);
                FD_SET((int)ix,&s);
                if (std_select_ignore_intr(ix+1,&s,NULL,NULL,&tv,&serr)==-1) {
                    EV_LOG(ERR,COM,0,"CRIT","fd %d is invalid",(int)ix);
                    STD_ASSERT((int)ix != p->sock.socket);
                    STD_ASSERT((int)ix != p->server_ctl_sock[0]);
                    p->pend_close((int)ix);
                }
            }
            continue;
        }
        if (rc==0) {
            if (p->server_init.timeout!=NULL) {
                p->server_init.timeout(p->server_init.context);
            }
            continue;
        }
        process_sockets(p,rset);
    }
}

t_std_error std_socket_service_run(std_socket_server_handle_t handle) {
    std_socket_service_data_t *p = (std_socket_service_data_t*)handle;
    std_thread_pool_job_t j;
    j.context = handle;
    j.funct = socket_service;
    j.free_job_func = NULL;
    return std_thread_pool_job_add(p->thread_pool,&j);
}

static bool _user_stub_function(void *context, int fd) {
    return true;
}
static bool _user_stub_function_false(void *context, int fd) {
    return false;
}
t_std_error std_socket_service_init(std_socket_server_handle_t *handle,
        std_socket_server_t *init) {
    signal(SIGPIPE,SIG_IGN);

    std_server_socket_desc_t sock;
    sock.address = init->address;
    sock.listeners = init->listeners == 0 ? MAX_QUEUED_CLIENTS : init->listeners;

    if (std_server_socket_create(&sock)!=STD_ERR_OK) {
        return STD_ERR(COM,FAIL,0);
    }

    std_socket_service_data_t *p = new std_socket_service_data_t;
    if (p==NULL) {
        close(sock.socket);
        return STD_ERR(COM,FAIL,0);
    }
    if (pipe(p->server_ctl_sock)==-1) {
        close(sock.socket);
        delete p;
        return STD_ERR(COM,FAIL,0);
    }
    p->sock = sock;
    p->thread_pool = NULL;
    p->server_init = *init;

    if (p->server_init.new_client==NULL)
        p->server_init.new_client = _user_stub_function;
    if (p->server_init.del_client==NULL)
        p->server_init.del_client = _user_stub_function;
    if (p->server_init.some_data==NULL)
        p->server_init.some_data = _user_stub_function_false;

    FD_ZERO(&p->pending);
    std_mutex_lock_init_recursive(&p->m_mutex);

    std_thread_create_param_t param;
    std_thread_init_struct(&param);
    param.name = init->name;
    if (init->thread_pool_size ==0) init->thread_pool_size = 1;

    if (std_thread_pool_create(&p->thread_pool,&param,init->thread_pool_size)!=STD_ERR_OK) {
        delete p;
        return STD_ERR(COM,FAIL,0);
    }

    *handle = p;
    return STD_ERR_OK;
}

t_std_error std_socket_service_client_close(std_socket_server_handle_t handle, int fd) {
    std_socket_service_data_t *p = (std_socket_service_data_t*)handle;
    p->pend_close(fd);
    return STD_ERR_OK;
}

t_std_error std_socket_service_destroy(std_socket_server_handle_t handle) {
    std_socket_service_data_t *p = (std_socket_service_data_t*)handle;
    delete p;
    return STD_ERR_OK;
}

t_std_error std_socket_service_client_add(std_socket_server_handle_t handle, int fd) {
    std_socket_service_data_t *p = (std_socket_service_data_t*)handle;
    std_mutex_simple_lock_guard m(&p->m_mutex);
    p->m_clients.insert(fd);
    p->new_conn(fd);
    p->returned(fd);
    return STD_ERR_OK;
}
