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
 * filename: std_cmd_redir.c
 */


/**
 *       @file  std_cmd_redir.c
 *      @brief  implement the redirection lib
 *
 *   @internal
 *     Created  05/11/2014
 *     Company  DELL
 *
 * =====================================================================================
 */


#include "std_cmd_redir.h"
#include "std_type_defs.h"
#include "std_select_tools.h"
#include "std_socket_tools.h"
#include "std_thread_tools.h"
#include "event_log.h"
#include "std_error_codes.h"
#include "std_file_utils.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

#define LOG_ERRNO EV_LOG_ERRNO(ev_log_t_COM,0,"COM-PROXY",errno)

#define LOG_E(LVL,message,...) \
    EV_LOG_ERR(ev_log_t_COM,LVL,"COM-PROXY",message,##__VA_ARGS__)

#define LOG_T(LVL,message,...) \
    EV_LOG_TRACE(ev_log_t_COM,LVL,"COM-PROXY",message,##__VA_ARGS__)


#define ERR_ERRNO STD_ERR_MK(e_std_err_COM,e_std_err_code_FAIL,errno)
#define ERR_PARAM STD_ERR_MK(e_std_err_COM,e_std_err_code_PARAM,0)

#define DEF_BUF_LEN (128)

typedef struct redir_data_s {
    int console;
    int client_socket;
    int client_flag_for_close;
    std_server_socket_desc_t srv;

    std_thread_create_param_t shell_thread;
    std_thread_create_param_t console_thread;
    std_cmd_redir_t param;

    char m_buff[DEF_BUF_LEN];
    size_t m_buff_len;

} redir_data_t;

static t_std_error redirect_io_to_sock(int fd) {
    if (dup2(fd,STDIN_FILENO)<0 ||
            dup2(fd,STDOUT_FILENO)<0) {
            return ERR_ERRNO;
    }
    return STD_ERR_OK;
}

static bool copy_to_fd(int rsock, int wsock, char *b,size_t len) {
    int rc = 0;
    int trc = 0;
    rc = std_read(rsock,b,len-1,false,NULL);
    if (rc>0) {
        b[rc] = '\0';
        if (wsock==-1) {
            //flush only...
            trc = rc;
        } else {
            trc = write(wsock,b,rc);
        }
    }
    return rc!=-1 && rc!=0 && trc==rc;
}

static void flush_console_fd(int sock, fd_set *rset, char *b, size_t len) {
    if (!FD_ISSET(sock,rset)) return;
    //throw out any errors
    copy_to_fd(sock,-1, b,len);
}

static bool handle_server_socket(int ssock, fd_set *s, int *cli, t_std_error *err) {
    *err = STD_ERR_OK;
    if (FD_ISSET(ssock,s)) {
        int client_fd = accept(ssock, NULL, NULL);
        if (client_fd>=0) {
            *cli = client_fd;
            return true;
        }
        if ((client_fd==-1) &&
            ((errno!=EINTR) &&
            (errno!=EWOULDBLOCK) &&
            (errno!=EAGAIN))
            ) {
            *err= ERR_ERRNO;
        }
        LOG_ERRNO;
        *cli = -1;
    }
    return false;
}

static int wait_for_connection(int ssock, redir_data_t *data,
        t_std_error *err) {
    int con_fd = data->console;
    LOG_T(0,"Waiting for a new connection.. on %d",ssock);
    while (true) {
        fd_set rset;
        int max_fd=-1;
        std_sel_adds_set(&ssock,1,&rset,&max_fd,true);
        std_sel_adds_set(&con_fd,1,&rset,&max_fd,false);
        struct timeval tv = {1,0};
        int rc = std_select_ignore_intr(max_fd+1,&rset,NULL,NULL,&tv,err);
        if (rc==0) continue;
        if (rc>0) {
            flush_console_fd(con_fd,&rset,data->m_buff,data->m_buff_len);

            int client_fd = -1;
            if (handle_server_socket(ssock,&rset,&client_fd,err)) {
                if (std_write(client_fd,data->m_buff,strlen(data->m_buff),
                    true,NULL)==-1) {
                    close(client_fd);
                    continue;
                }
                return client_fd;
            } else {
                if (*err!=STD_ERR_OK) {
                    return -1;
                }
            }

        } else {
            if (rc==-1 && STD_ERR_EXT_PRIV(*err)==EINTR) continue;

            LOG_E(0,"two possible issues.. server socket is "
                        "closed (%d) or console fd is bad (%d) "
                        " error is %u exiting...",ssock, con_fd,*err);
            if (rc==-1 && STD_ERR_EXT_PRIV(*err)!=EINTR) break;
        }
    }
    return -1;
}

static void close_socket(int *sock) {
    close(*sock);
    *sock = -1;
}

static void * console_reader (void * param) {
    redir_data_t *data = (redir_data_t*) param;
    int console = data->console;

    memset(&data->srv,0,sizeof(data->srv));

    data->srv.address.type = e_std_sock_UNIX;
    data->srv.address.addr_type = e_std_socket_a_t_STRING;
    strncpy(data->srv.address.address.str,data->param.path,
        sizeof(data->srv.address.address.str)-1);

    data->srv.listeners = 10;

    t_std_error serr=std_server_socket_create(&data->srv);
    if (serr!=STD_ERR_OK) {
        LOG_E(0,"Failed to create server sock %X",serr);
        return NULL;
    }

    if ((serr=std_sock_set_nonblock(data->srv.socket,true))!=STD_ERR_OK) {
        LOG_E(0,"Error setting nonblock srv sock %X",serr);
        return NULL;
    }
    while (true) {
        data->client_socket = wait_for_connection(data->srv.socket,
                        data,&serr);
        if (data->client_socket<0) {
            if (serr!=STD_ERR_OK) {
                //XXXX redirect stdout/stdin to /dev/null
                //cleanup and exit
                close(data->srv.socket);
                close(console);
                free(data);
                LOG_E(0,"Critical failure.. no recovery %u "
                    "no more console redirect",serr);
                return NULL;
            }
            continue;
        }
        data->client_flag_for_close = 0;
        while(data->client_socket!=-1) {
            if (data->client_flag_for_close!=0) {
                close_socket(&data->client_socket);
                break;
            }
            fd_set rset;
            int selfds[]={data->client_socket,console};
            int max_fd = -1;
            std_sel_adds_set(selfds,sizeof(selfds)/sizeof(*selfds),
                    &rset,&max_fd,true);
            int rc = std_select_ignore_intr(max_fd+1,&rset,NULL,NULL,NULL,NULL);
            if (rc<0) {
                close_socket(&data->client_socket);
                break;
            }
            if (rc>0) {
                if (FD_ISSET(console,&rset)) {
                    if (!copy_to_fd(console,data->client_socket,
                        data->m_buff,data->m_buff_len)) {
                        close_socket(&data->client_socket);
                        break;
                    }
                }
                if (FD_ISSET(data->client_socket,&rset)) {
                    if (!copy_to_fd(data->client_socket,console,
                            data->m_buff,data->m_buff_len)) {
                        close_socket(&data->client_socket);
                        break;
                    }
                }
            }
        }
    }
    return NULL;
}

t_std_error std_cmd_redir_term_conn(std_cmd_redir_t *param) {
    redir_data_t *data = (redir_data_t *)param->data;
    data->client_flag_for_close = 1;
    return STD_ERR_OK;
}

t_std_error std_cmd_redir_init(std_cmd_redir_t *param) {
    int sockets[2] = {-1,-1};
    redir_data_t *data = NULL;
    t_std_error serr = STD_ERR_OK;
    signal(SIGPIPE,SIG_IGN);
    do {
        int rc = socketpair(AF_UNIX,SOCK_STREAM,0,sockets);
        if (rc<0) {
            serr = ERR_ERRNO;
            break;
        }

        std_sock_set_sndbuf(sockets[0],DEF_BUF_LEN);

        redir_data_t *data = (redir_data_t*)malloc(sizeof(redir_data_t));

        memset(data,0,sizeof(*data));
        data->m_buff_len = sizeof(data->m_buff);

        data->console = sockets[1];

        data->param = *param;
        param->data = data;
        //store old fds and reset if error occurs or exit
        serr = redirect_io_to_sock(sockets[0]);
        if (serr!=STD_ERR_OK) {
            break;
        }
        close(sockets[0]);

        std_thread_init_struct(&data->shell_thread);
        std_thread_init_struct(&data->console_thread);

        data->console_thread.thread_function = console_reader;
        data->console_thread.name = "Console reader thread";
        data->console_thread.param = data;
        if ((serr=std_thread_create(&data->console_thread))!=STD_ERR_OK) {
            LOG_E(0,"Failed to start shell thread");
            break;
        }

        data->shell_thread.thread_function = param->func;
        data->shell_thread.name = "Shell thread";
        data->shell_thread.param = param->param;
        if ((serr=std_thread_create(&data->shell_thread))!=STD_ERR_OK) {
            LOG_E(0,"Failed to start shell thread");
            break;
        }

        return serr;
    } while (0);
    if (sockets[0]!=-1) close (sockets[0]);
    if (sockets[1]!=-1) close (sockets[1]);
    if (data!=NULL) {
        std_thread_destroy_struct(&data->shell_thread);
        std_thread_destroy_struct(&data->console_thread);
        free(data);
    }
    return serr;

}

t_std_error std_cmd_redir_connect(const char *path, int *outfd) {
    int sock = -1;
    std_socket_address_t add;
    if (outfd==NULL) return ERR_PARAM;

    strncpy(add.address.str,path,sizeof(add.address.str)-1);
    add.type = e_std_sock_UNIX;
    add.addr_type = e_std_socket_a_t_STRING;

    t_std_error serr= std_sock_connect(&add, &sock);
    if (serr!=STD_ERR_OK) return serr;

    *outfd = sock;

    return STD_ERR_OK;
}

