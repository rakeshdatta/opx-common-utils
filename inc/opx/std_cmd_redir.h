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
 * filename: std_cmd_redir.h
 */


/**
 *       @file  std_cmd_redir.h
 *      @brief  redirect function call stdin/out to server socket and tools to
 *              connect
 *
 *
 *   @internal
 *     Created  05/11/2014
 *     Company  DELL
 *   Copyright  Copyright (c) 2014, Cliff Wichmann
 *
 * =====================================================================================
 */

#ifndef __STD_CMD_REDIR_H
#define __STD_CMD_REDIR_H

#include "std_socket_tools.h"

/**
 * This is the structure that needs to be passed to the redirection initializiton fucntions
 */
typedef struct std_cmd_redir_s {
    char path[STD_SERVER_SOCK_ADDR_PATH_LEN_UNIX];
    void *(*func)(void *param); //! the func is a pointer to the service that will
                                //! be called where all standard in and out will be redirected.
    void *param; //! the param will be a context passed to the function
    void *data; //! this is used internally by the API and will be an out parameter
} std_cmd_redir_t;


/**
 * @brief   initialize the redirection library and call the function
 *              redirecting stdout/stdin to clients once connected
 *              assumes text based input/output currently
 * @param   param[in/out] will use path to create a unixdomain socket and
 *                  will call the specified function and fill in data with
 *                  internally allocated data.. not expected to be freed at
 *                  this time
 * @return  STD_ERR_OK or error
 */
t_std_error std_cmd_redir_init(std_cmd_redir_t *param);

/**
 * @brief this function will terminate all existing sessions that are connected to the server.
 * This API has no side effects.
 *
 * @param param the command redirection session structure.
 * @return
 */
t_std_error std_cmd_redir_term_conn(std_cmd_redir_t *param);


/**
 * @brief   connects to a redirected command
 * @param   path[in] the path to the unix socket to connect
 * @param   fd[out] the returned socket that is connected
 * @return  STD_ERR_OK if the fd is valid otherwise an error
 */
t_std_error std_cmd_redir_connect(const char *path, int *fd);



#endif
