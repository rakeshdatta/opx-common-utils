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

/**
 * filename: std_event_service.c
 **/

/*

 * std_event_service.cpp
 *
 *  Created on: May 23, 2014
 */


#include "std_mutex_lock.h"

#include "std_event_service.h"
#include "private/std_event_utils.h"

#include "std_socket_tools.h"

#include "std_struct_utils.h"
#include "std_select_tools.h"
#include "std_thread_tools.h"

#include "std_rw_lock.h"
#include "std_socket_service.h"

#include "event_log.h"
#include "std_time_tools.h"


#include <stdio.h>
#include <list>
#include <map>
#include <memory>
#include <vector>

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <set>

#define DEF_LISTENERS (10)
#define COM_EVT_SERV_NAME "EVENT_SERVICE"
/* buffer size is current set to 40Mb to cater scaled system limits.
 * in case of ARP on port down, kernel could notify events and even publish
 * happens at faster rate than it could be consumed at the receiver end.
 * hence for now increasing the buffer size to 40Mb and this may require
 * fine tuning in future depending on the system performance
 */
#define SOCKET_BUFFER_SIZE (1024*1000*40)
#define SOCKET_BUFFER_MIN_SIZE (300000)

#define EV_WRITE_TIMEOUT (100)
#define EV_SEND_TIMEOUT (2000)


#define LE(strid,message,...) EV_LOG_ERR(ev_log_t_COM,0,strid,message,##__VA_ARGS__)
#define LI(lvl,message,...) EV_LOG_ERR(ev_log_t_COM,lvl,"COM",message,##__VA_ARGS__)
#define LT(lvl,message,...) EV_LOG_TRACE(ev_log_t_COM,lvl,"COM",message,##__VA_ARGS__)
#define STD_ERR_RC(type,x) STD_ERR_MK(e_std_err_COM,e_std_err_code_##type,(x))

struct std_node_t {
    typedef std::map<uint32_t,struct std_node_t*> node_type_t;
    typedef node_type_t::iterator iterator;

    std::vector<uint32_t> key;
    std::map<uint32_t,struct std_node_t*> m_nodes;

    std::set<int> clients;

    std_node_t::iterator end() { return m_nodes.end(); }

    std_node_t::iterator find(uint32_t id) {
        return m_nodes.find(id);
    }

    bool create_node(uint32_t node) ;
    bool insert(int fd) ;
    void remove(int fd) ;
    void publish(std_socket_server_handle_t handle, std_event_msg_t *msg, std::set<int> &fds) ;
    void remove_from_tree(int fd);

    bool empty() {
        return m_nodes.size()==0 && clients.size()==0;
    }
    void clean_empty_nodes();
};

class std_client_tree {
private:
    std_node_t head;
    std_node_t * find(uint32_t *list, size_t len, bool create=false) ;
public:
    bool reg_client(int fd, std_event_key_t *key) ;
    bool dereg_client(int fd, std_event_key_t *key) ;
    void remove_from_tree(int fd) ;
    void publish(std_socket_server_handle_t handle, std_event_msg_t *msg);
};

struct event_service_client_data_t {
    std::vector<uint8_t> buff;
};

typedef std::map<int,event_service_client_data_t *> event_service_clients_t;

struct  std_socket_event_server_t {

    std_socket_server_handle_t m_sock_service;
    event_service_clients_t m_event_clients;

    std_rw_lock_t m_tree_lock;
    std_client_tree m_reg_tree;

    void close_client_connection(int fd);
    bool new_client_connection(int fd);
    bool reg_message(int fd, std_event_key_t *key);
    bool dereg_message(int fd, std_event_key_t *key);

    std_socket_event_server_t() {
        m_sock_service = NULL;
    }
    void cleanup() {
        if (m_sock_service!=NULL) {
            std_socket_service_destroy(m_sock_service);
        }
        std_rw_lock_delete(&m_tree_lock);
    }

    ~std_socket_event_server_t() {
        cleanup();
    }

    void publish(std_event_msg_t *msg);
} ;


bool std_node_t::create_node(uint32_t node) {
    std::auto_ptr<std_node_t> n (new std_node_t);
    try {
        m_nodes[node] = n.get();
        n.release();
    } catch(...) {
        return false;
    }
    return true;
}

bool std_node_t::insert(int fd) {
    try {
        clients.insert(fd);
    } catch (...){
        return false;
    }
    return true;
}

void std_node_t::remove(int fd) {
    clients.erase(fd);
}

void std_node_t::publish(std_socket_server_handle_t handle, std_event_msg_t *msg,
        std::set<int> &fds) {
    if (clients.size()==0) return;

    std::set<int>::iterator it = clients.begin();
    std::set<int>::iterator end = clients.end();
    event_serv_msg_t m;
    m.op = event_serv_msg_t_PUBLISH;
    std_event_msg_descr_t d;
    d.data = msg;
    d.len = sizeof(*msg)+msg->data_len;
    for ( ; it != end ; ++it ) {
        if (fds.find(*it)!=fds.end()) continue;
        if (std_event_util_event_send(*it,&m,&d,1,EV_WRITE_TIMEOUT)!=STD_ERR_OK) {
            std_socket_service_client_close(handle,*it);
            EV_LOG(ERR,COM,0,"COM-EVENT-SEND","Client not receiving messages.  Terminating (%d)",*it);
        }
        fds.insert(*it);
    }
}

void std_node_t::remove_from_tree(int fd) {
    iterator it = m_nodes.begin();
    iterator end = m_nodes.end();
    for ( ; it != end ; ++it ) {
        (it->second)->remove_from_tree(fd);
    }
    remove(fd);
}

void std_node_t::clean_empty_nodes() {
    iterator it = m_nodes.begin();
    iterator end = m_nodes.end();
    for ( ; it != end ; ++it ) {
        it->second->clean_empty_nodes();
        if (it->second->empty()) {
            delete it->second;
            m_nodes.erase(it);
            it = m_nodes.begin();
        }
    }
}

std_node_t * std_client_tree::find(uint32_t *list, size_t len, bool create) {
    register size_t ix = 0;
    std_node_t * cur = &head;

    for (; ix < len; ++ix ) {
        std_node_t::iterator it = cur->find(list[ix]);
        if (it==cur->end()) {
            if (!create) return NULL;
            cur->create_node(list[ix]);

            it = cur->find(list[ix]);
            if (it==cur->end()) return NULL;
        }
        cur = it->second;
    }
    return cur;
}

bool std_client_tree::reg_client(int fd, std_event_key_t *key) {
    std_node_t * cur = find(key->event_key,key->len,true);
    if (cur==NULL) return false;
    return cur->insert(fd);
}

bool std_client_tree::dereg_client(int fd, std_event_key_t *key) {
    std_node_t * cur = find(key->event_key,key->len,false);
    if (cur==NULL) return true;
    cur->remove(fd);
    return true;
}

void std_client_tree::remove_from_tree(int fd) {
    head.remove_from_tree(fd);
}

void std_client_tree::publish(std_socket_server_handle_t handle, std_event_msg_t *msg) {
    size_t ix = 0;
    std_node_t * cur = &head;
    size_t len = msg->key.len;
    std::set<int> already_sent;
    for ( ; ix < len; ++ix ) {
        std_node_t::iterator it = cur->find(msg->key.event_key[ix]);
        if (it==cur->end()) {
            return;
        }
        cur = it->second;
        cur->publish(handle,msg,already_sent);
    }
}


void std_socket_event_server_t::publish(std_event_msg_t *msg) {
    std_rw_lock_read_guard l(&m_tree_lock);
    m_reg_tree.publish(m_sock_service, msg);
}

bool std_socket_event_server_t::new_client_connection(int fd) {
    try {
        m_event_clients[fd] = new event_service_client_data_t;
    } catch (...) {
        return false;
    }
    return true;
}

void std_socket_event_server_t::close_client_connection(int fd) {

    event_service_clients_t::iterator it = m_event_clients.find(fd);
    if (it!=m_event_clients.end()) {
        delete it->second;
        m_event_clients.erase(it);
    }
    m_reg_tree.remove_from_tree(fd);
}

bool std_socket_event_server_t::reg_message(int fd, std_event_key_t *key) {
    std_rw_lock_write_guard l(&m_tree_lock);
    return m_reg_tree.reg_client(fd, key);
}

bool std_socket_event_server_t::dereg_message(int fd, std_event_key_t *key) {
    std_rw_lock_write_guard l(&m_tree_lock);
    m_reg_tree.dereg_client(fd,key);
    return true;
}


inline std_socket_event_server_t * to_context(std_event_server_handle_t h) {
    return (std_socket_event_server_t*)h;
}

static void setup_address(std_socket_address_t *address, const char * event_channel_name) {
    memset(address,0,sizeof(*address));
    strncpy(address->address.str,event_channel_name,sizeof(address->address.str));
    address->address.str[sizeof(address->address.str)-1] = 0;
    address->type = e_std_sock_UNIX;
    address->addr_type = e_std_socket_a_t_STRING;
}

static bool event_some_data ( void *context, int fd ) {
    std_socket_event_server_t *p = (std_socket_event_server_t*)context;

    event_service_clients_t::iterator it = p->m_event_clients.find(fd);
    if (it==p->m_event_clients.end()) {
        return false;
    }
    std::vector<uint8_t> &buff =(*it).second->buff;
    if (std_event_util_event_recv(fd,buff,true)!=STD_ERR_OK) {
        return false;
    }

    if (buff.size()< sizeof(event_serv_msg_t)) return false;

    event_serv_msg_t *serv_msg = (event_serv_msg_t*) &(buff[0]);
    void * data = vector_offset(buff,sizeof(event_serv_msg_t));
    if (serv_msg->op==event_serv_msg_t_ADD_REG) {
        std_event_key_t *key = (std_event_key_t *)data;
        return p->reg_message(fd,key);
    }
    if (serv_msg->op == event_serv_msg_t_DEL_REG) {
        std_event_key_t *key = (std_event_key_t *)data;
        p->dereg_message(fd,key);
    }
    if (serv_msg->op == event_serv_msg_t_PUBLISH) {
        std_event_msg_t *msg = (std_event_msg_t *)data;
        if (buff.size()< (sizeof(event_serv_msg_t)+ sizeof(std_event_msg_t)+msg->data_len)) return false;
        p->publish(msg);
    }
    if (serv_msg->op == event_serv_msg_t_BUFFER) {
        size_t buff_len = *(size_t*)data;
        if (buff_len < SOCKET_BUFFER_MIN_SIZE) { //if less then some minimum, ignore it
            buff_len = SOCKET_BUFFER_MIN_SIZE;
        }
        if (std_sock_set_sndbuf(fd,buff_len)!=STD_ERR_OK) {
            EV_LOG(ERR,COM,0,"COM-EVENT-BUFFER","Failed to set the buffering amount to %d",(int)buff_len);
        } else {
            EV_LOG(ERR,COM,0,"COM-EVENT-BUFFER","Buffer adjusted to %d",(int)buff_len);
        }
    }
    return true;
}

//New client connection established
static bool event_new_client(void *context,  int fd ) {
    std_socket_event_server_t *p = (std_socket_event_server_t*)context;
    std_rw_lock_write_guard l(&p->m_tree_lock);

    if (p->m_event_clients.find(fd)!=p->m_event_clients.end()) {
        //something wrong.. cleanup and exit
        p->close_client_connection(fd);
        return false;
    }
    //set the default socket buffer for the events to something high
    std_sock_set_sndbuf(fd,SOCKET_BUFFER_SIZE);
    return p->new_client_connection(fd);
}

//Client has closed connection
static bool event_del_client ( void *context, int fd ) {
    std_socket_event_server_t *p = (std_socket_event_server_t*)context;
    std_rw_lock_write_guard l(&p->m_tree_lock);
    p->close_client_connection(fd);

    return true;
}

t_std_error std_event_server_init(std_event_server_handle_t *handle,
        const char * event_channel_name, size_t threads) {

    std_socket_event_server_t    * p = new std_socket_event_server_t;

    if (p==NULL) {
        EV_LOG(ERR,COM,0,"COM-EVENT-INIT-FAILED","Failed to initialize the event service");
        return STD_ERR(COM,NOMEM,0);
    }

    std_socket_server_t serv;
    memset(&serv,0,sizeof(serv));
    serv.context = p;
    serv.name = COM_EVT_SERV_NAME;
    serv.thread_pool_size = threads;
    serv.new_client = event_new_client;
    serv.del_client = event_del_client;
    serv.some_data = event_some_data;

    setup_address(&serv.address, event_channel_name);

    if (std_socket_service_init(&p->m_sock_service,&serv)!=STD_ERR_OK) {
        delete p;
        return STD_ERR(COM,FAIL,0);
    }

    t_std_error rc =  STD_ERR(COM,FAIL,0);

    do {
        if ((rc=std_rw_lock_create_default(&p->m_tree_lock))!=STD_ERR_OK) break;
        if (std_socket_service_run(p->m_sock_service)!=STD_ERR_OK) break;

        *handle = p;
        return rc;
    } while (0);

    delete p;
    return rc;
}


t_std_error std_server_client_connect(std_event_client_handle * handle, const char *event_channel_name) {
    std_socket_address_t addr;
    setup_address(&addr,event_channel_name);

    int sock = -1;

    t_std_error rc = std_sock_connect(&addr,&sock);
    if (rc!=STD_ERR_OK) return rc;

    std_sock_set_sndbuf(sock,SOCKET_BUFFER_SIZE);

    *handle = sock;

    return STD_ERR_OK;
}

t_std_error std_server_client_disconnect(std_event_client_handle handle) {
    if (handle!=-1) close(handle);
    return STD_ERR_OK;
}

t_std_error std_client_publish_msg(std_event_client_handle handle, std_event_msg_t *msg) {
    event_serv_msg_t m;
    m.op = event_serv_msg_t_PUBLISH;

    std_event_msg_descr_t d;
    d.data  = msg;
    d.len = sizeof(*msg)+msg->data_len;
    return std_event_util_event_send(handle,&m,&d,1,EV_SEND_TIMEOUT);
}

t_std_error std_client_publish_msg_data(std_event_client_handle handle,
        std_event_key_t *key,void * data, size_t len)  {
    event_serv_msg_t m;
    m.op = event_serv_msg_t_PUBLISH;

    std_event_msg_t msg;
    msg.key = *key;
    msg.data_len = len;

    std_event_msg_descr_t d[2];
    d[0].data = &msg;
    d[0].len = sizeof(msg);
    d[1].data = data;
    d[1].len = len;
    return std_event_util_event_send(handle,&m,d,sizeof(d)/sizeof(*d),EV_SEND_TIMEOUT);
}

static t_std_error client_send_key_op(std_event_client_handle handle,
        event_serv_msg_types_t op,std_event_key_t *keys, size_t len) {
    size_t ix = 0;
    event_serv_msg_t m;
    m.op = op;
    for ( ; ix < len ; ++ix ) {
        std_event_msg_descr_t d;
        d.data = &keys[ix];
        d.len = sizeof (keys[ix]);

        if (std_event_util_event_send(handle,&m,&d,1,EV_SEND_TIMEOUT)!=STD_ERR_OK) {
            return STD_ERR(COM,FAIL,0);
        }
    }
    return STD_ERR_OK;
}

t_std_error std_client_register_interest(std_event_client_handle handle, std_event_key_t *keys, size_t len) {
    return client_send_key_op (handle,event_serv_msg_t_ADD_REG,keys,len);
}

t_std_error std_client_remove_interest(std_event_client_handle handle, std_event_key_t *keys, size_t len) {
    return client_send_key_op (handle,event_serv_msg_t_DEL_REG,keys,len);
}

struct event_msg_buff_t {
    std::vector<uint8_t> buff;
    bool limit_max;
    size_t max_len;
};

std_event_msg_buff_t std_client_allocate_msg_buff(unsigned int buffer_space, bool limit_max) {
    event_msg_buff_t *p = new event_msg_buff_t;
    if (p==NULL) return false;

    try {
        p->buff.resize(buffer_space);
    } catch (...) {
        delete p;
        return NULL;
    }
    p->limit_max = limit_max;
    p->max_len = buffer_space;
    return p;
}

/**
 * @brief free the message
 * @param msg to free
 */
void std_client_free_msg_buff(std_event_msg_buff_t *buff) {
    event_msg_buff_t *p = (event_msg_buff_t*)buff;
    delete p;
}

std_event_msg_t * std_event_msg_from_buff(std_event_msg_buff_t buff) {
    event_msg_buff_t *p = (event_msg_buff_t*)buff;
    if (p->buff.size()<sizeof(event_serv_msg_t)) return NULL;
    event_serv_msg_t *m = (event_serv_msg_t*) vector_offset(p->buff,0);
    if (m->op != event_serv_msg_t_PUBLISH) return NULL;
    return (std_event_msg_t *)vector_offset(p->buff,sizeof(event_serv_msg_t));
}

t_std_error std_client_wait_for_event_data(std_event_client_handle handle,
        std_event_msg_t *msg, void *buff, size_t len) {
    return std_event_util_event_recv_msg(handle,msg,buff,len);
}


t_std_error std_client_wait_for_event(std_event_client_handle handle, std_event_msg_buff_t buff) {
    event_msg_buff_t *p = (event_msg_buff_t*)buff;

    return std_event_util_event_recv(handle,p->buff, !p->limit_max);
}

t_std_error std_client_set_receive_buffer(std_event_client_handle handle,
        size_t len)  {
    event_serv_msg_t m;
    m.op = event_serv_msg_t_BUFFER;

    std_event_msg_descr_t d[1];
    d[0].data = &len;
    d[0].len = sizeof(len);
    return std_event_util_event_send(handle,&m,d,1,EV_SEND_TIMEOUT);
}
