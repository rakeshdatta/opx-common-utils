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
 * filename: std_socket_tools.h
 */


/**
 *       @file  std_socket_tools.h
 *      @brief  standard socket tools
 *
 *   @internal
 *     Created  05/11/2014
 *     Company  DELL
 *
 * =====================================================================================
 */

#ifndef __STD_SOCKET_TOOLS_H
#define __STD_SOCKET_TOOLS_H

#include "std_error_codes.h"
#include "std_type_defs.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <stdbool.h>

/** \defgroup SocketsAndFilesCommon Socket and File utilities
*
* \{
*/

#define STD_SOCK_MAX_LISTENERS (100)
/**
 * Socket domain types
 */
typedef enum  {
    e_std_sock_UNIX,  //!< Unix Domain socket
    e_std_sock_INET4, //!< Internet domain IPv4 Socket
}e_std_socket_domain_t ;

/* This type is deprecated - use the socket domain type instead */
typedef e_std_socket_domain_t e_std_soket_type_t;

/**
 * Socket connection types
 */
typedef enum  {
    e_std_sock_type_STREAM, //!< Connection oriented stream support
    e_std_sock_type_DGRAM,  //!< Connection-less packet support
    e_std_sock_type_RAW,    //!< Custom IP packet support
}e_std_sock_type_t ;

/**
 * This is the type of socket addresses
 */
typedef enum  {
    e_std_socket_a_t_STRING,    //!< Address in string form (only UNIX sockets)
    e_std_socket_a_t_INET       //!< Address in binary form
}e_std_socket_addr_type_t;

#define STD_SERVER_SOCK_ADDR_PATH_LEN_UNIX \
    (sizeof(((struct sockaddr_un*)0)->sun_path))

typedef struct sockaddr_in std_sockaddr_inet4;

/**
 * Socket address abstraction
 */
typedef struct std_socket_address_s {
    e_std_socket_domain_t type;                            //!< Socket family or domain
    e_std_socket_addr_type_t addr_type;                 //!< Address format selector
    union {
        char str[STD_SERVER_SOCK_ADDR_PATH_LEN_UNIX+1]; //!< For UNIX domain sockets
        std_sockaddr_inet4 inet4addr;
    } address;
} std_socket_address_t;

typedef struct std_server_socket_desc_s {
    int socket;                            //the server's socket
    unsigned int listeners;                //number of listening mailboxes to have outstanding at one time
    std_socket_address_t address;        //string or ipaddress in bytes
} std_server_socket_desc_t;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct msghdr std_socket_msg_t;

/**
 * The flags indicating send/receive behavior
 */
typedef enum {
    std_socket_transit_f_NONE, //!< std_socket_transit_f_NONE
    std_socket_transit_f_NONBLOCK=(1<<0),//!< std_socket_transit_f_NOWAIT simular to MSG_DONTWAIT
    std_socket_transit_f_ALL=(1<<1) //!< std_socket_transit_f_ALL wait for all data requested to be read
} std_socket_transit_flags_t;

/**
 * Select the type of socket function
 */
typedef enum {
    std_socket_transit_o_READ,//!< std_socket_transit_o_READ perform a read operation
    std_socket_transit_o_WRITE//!< std_socket_transit_o_WRITE perform a write operation
} std_socket_transit_op_t;

/**
 * Send or receive data on a socket.  If ewouldblock is returned and the timeout is > 0, a select will be performed
 * for the specified time wait.  In the case of write, all of the data provided must be written to be successful.
 * For a read operation, the amount of data read will be returned.
 * @param fd the socket
 * @param op should indicate read or write operations
 * @param msg the actual message structures
 * @param flags the flags for the application
 * @param time_wait a timeout in milliseconds in case of ewouldblock or eagain
 * @param rc the std return code
 * @return the number of bytes read/written or -1 on an error
 */
ssize_t std_socket_op(std_socket_transit_op_t op, int fd, std_socket_msg_t *msg,
        std_socket_transit_flags_t flags ,size_t time_wait, t_std_error * rc);

/**
 * @brief   Create a server socket based on passed in data
 * @param   desc of the server socet to create
 * @return  error code or STD_ERR_OK
 */
t_std_error std_server_socket_create(std_server_socket_desc_t *desc);

/**
 * @brief   connect to a server socket
 * @param   addr the address of the server
 * @param   sock[out] the returned socket from the connection request
 * @return  STD_ERR_OK or the error code
 */
t_std_error std_sock_connect(std_socket_address_t *addr,int *sock);

/**
 * @brief   connect to a server socket and return a socket (set to blocking reads/writes)
 * @param   addr the address of the server
 * @param   sock[out] the returned socket from the connection request
 * @param     ms_wait the amount of time to wait for the connection to be established.
 * @return  STD_ERR_OK or the error code
 */
t_std_error std_sock_connect_with_timeout(std_socket_address_t *addr,int *sock, size_t ms_wait);


/**
 * @brief   set the socket to non blocking
 * @param   fd[in] socket or fd to set non blocking
 * @param   nonblock true if want non-blocking otherwise false
 * @return  error code or STD_ERR_OK
 */
t_std_error std_sock_set_nonblock(int fd, bool nonblock);

/**
 * @brief   set the receive buffer size
 * @param   fd to configure
 * @param   len is the size in bytes
 * @return  STD_ERR_OK or error
 */
t_std_error std_sock_set_rcvbuf(int fd, int len);

/**
 * @brief   set the send buffer size on the socket
 * @param   fd to configure
 * @param   len is the size in bytes
 * @return  STD_ERR_OK or error
 */
t_std_error std_sock_set_sndbuf(int fd, int len);

/**
 * @brief   create socket pair for bi-directional communication
 * @param   domain - socket domain only AF_UNIX supported currently
 * @param   stream - if true stream oriented else datagram oriented
 * @param   fd - array of two file descriptor one for rx, one for tx
 * @return  STD_ERR_OK if successful or error
 */
t_std_error std_sock_create_pair(e_std_socket_domain_t domain, bool stream, int fd[2]);

/**
 * @brief  Utility to construct std_socket_address struct from IPv4 string, port
 * @param[in]  domain - Address domain
 * @param[in]  ipstr - IP address in string from - "127.0.0.1"
 * @param[in]  port - Socket port
 * @param[out] out_addr - filled std_socket_address structure
 * @return  STD_ERR_OK if successful or error
 */
t_std_error std_sock_addr_from_ip_str (e_std_socket_domain_t domain,
                                       const char* ipstr,
                                       int port,
                                       std_socket_address_t* out_addr);

/**
 * @brief  Create socket endpoint, Optionally bind to address
 * Use std_close to close the socket.
 * @param[in]  domain - socket domain
 * @param[in]  type   - socket type - connected stream, connectionless datagram
 * @param[in]  protocol - depends on the socket domain
 *                      - for INET domain this would be the IP protocol
 *                        and is valid only if the socket type is
 *                        not STREAM or DGRAM.
 * @param[in]  bind   - if not NULL then bind new socket to specified address
 * @param[out] fd - socket descriptor returned
 * @return  STD_ERR_OK if successful or error
 * @verbatim
 * for eg: to create a IPv4 UDP socket and bind it to 127.0.0.1 port 20000
 *   std_socket_address_t bind;
 *   t_std_error rc = std_sock_addr_from_ip_str (e_std_sock_INET4, "127.0.0.1", 20000,
 *                                               &bind);
 *   int fd;
 *   rc = std_socket_create (e_std_sock_INET4, e_std_sock_type_DGRAM, 0, &bind, &fd);
 */
t_std_error std_socket_create (e_std_socket_domain_t std_domain,
                               e_std_sock_type_t std_type,
                               int protocol,
                               const std_socket_address_t* bind,
                               int* fd);

/**
 * @brief  Bind socket that was previously created to a specific address.
 * This is a prerequisite to receiving messages sent to that address
 * @param   fd - descriptor of socket to be bound
 * @param   bind_addr - address to be bound to
 * @return  STD_ERR_OK if successful or error
 */
t_std_error std_socket_bind (int fd, const std_socket_address_t* bind_addr);

#ifdef __cplusplus
}
#endif

/**
 * \}
 */
#endif
