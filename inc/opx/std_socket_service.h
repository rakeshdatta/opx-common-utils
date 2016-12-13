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
 * std_socket_service.h
 */

#ifndef STD_SOCKET_SERVICE_H_
#define STD_SOCKET_SERVICE_H_

#include "std_socket_tools.h"
#include <stdbool.h>

/** \defgroup SocketsAndFilesCommon Socket and File utilities
*
* \{
*/

typedef void * std_socket_server_handle_t;

/**
 * Initialization parameters for the socket service
 */
typedef struct {
    const char * name;              //!< The name of the socket service
    size_t thread_pool_size;        //!< Number of threads available in the server
    std_socket_address_t address;   //!< The address to wait for clients on
    size_t  listeners;              //!< the number of pending requests to handle on the socket
    void * context;                 //!< Application used context
    bool (*some_data) ( void *context, int fd );  //!< Socket has some data to process
    bool (*new_client) (void *context,  int fd );  //!< New client connection established
    bool (*del_client) ( void *context, int fd ); //!< Client has closed connection
    void (*timeout)(void * context); //!< process a timeout (periodically called when no work to do)
} std_socket_server_t ;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the socket service for using the address in the init structure.
 * As soon as the init returns, connections can be made to the socket service but
 * no actual handling of the connection request will happen until the run is started.
 *
 * @param handle to the socket service
 * @param init the parameters used to initialize the service
 * @return STD_ERR_OK if things are good otherwise a specific failure condition.
 */
t_std_error std_socket_service_init(std_socket_server_handle_t *handle,
        std_socket_server_t *init);

/**
 * Have the socket service start to listen for connections.  This will use a thread from
 * the threading pool that used to initialize the service earlier
 * @param handle the handle to the service instance
 * @return STD_ERR_OK if things are good otherwise a specific failure condition.
 */
t_std_error std_socket_service_run(std_socket_server_handle_t handle);

/**
 * Close one of the sockets connections to the socket service.
 * @param handle the handle to the socket service
 * @param fd the file descriptor that will be closed
 * @return STD_ERR_OK if things are good otherwise a specific failure condition.
 */
t_std_error std_socket_service_client_close(std_socket_server_handle_t handle, int fd);

/**
 * Close down the socket service and clean up any open connections.
 * @param handle the handle to the socket service
 * @return STD_ERR_OK if things are good otherwise a specific failure condition.
 */
t_std_error std_socket_service_destroy(std_socket_server_handle_t handle);

/**
 * Add the client to the socket service.  This could occur if you want to add a personally
 * created connection (socket connect) manually
 * @param handle the handle to the socket service
 * @param fd the file descriptor to add
 * @return STD_ERR_OK if things are good otherwise a specific failure condition.
 */
t_std_error std_socket_service_client_add(std_socket_server_handle_t handle, int fd);

#ifdef __cplusplus
}
#endif

/**
 * \}
 */

#endif /* STD_SOCKET_SERVICE_H_ */
