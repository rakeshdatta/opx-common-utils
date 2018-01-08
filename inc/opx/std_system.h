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
 * filename: std_system.h
 * provides system level utility functions
 **/


#ifndef __STD_SYSTEM_H
#define __STD_SYSTEM_H

#include "std_error_codes.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
 * Find where the sysfs is currently mounted
 *
 * @param[out] sysfs char array where the path will be returned
 * @param[in] len the maximum lenght of the sysfs buffer
 *
 * @return STD_ERR_OK if success, failure code otherwise
 *
**************************************************************************/
t_std_error std_sys_sysfs_path_get(char *sysfs, size_t len);

/****************************************************************************
 * Execute a command with a variable number of arguments
 *
 * @warning This function calls 'fork' to start a process.
 * @warning 'fork' is a costly system call, so be aware of potential real-time implications, and use only when needed.
 *
 * @param[in] cmd command to execute - specify the absolute path of the command to avoid security problems
 * @param[in] argv the argument list; NULL terminated array - list of command arguments .
 * @param[in] envp environment variables; NULL terminated array - key=value strings .
 *
 * @return STD_ERR_OK if success, failure code otherwise
 *
 * @verbatim

   #include "std_system.h"

   // To execute the command 'ip monitor link'
   const char* args[] = {
      "ip", // You must provide the command name as args[0] - by convention
      "monitor",
      "link",
      NULL;
   };

   const char* envp[] = {
      NULL;
   };

   t_std_error rc = std_sys_execve_command("/sbin/ip", args, envp);
 * @endverbatim
**************************************************************************/
t_std_error std_sys_execve_command(const char *cmd, const char *const argv[], const char *const envp[]);

/**
 *  Set the network name space for the current thread (or process - if single threaded).
 *
 *  <p>The typical usage is to call these functions once at initialization, e.g.
 *  set desired network name space,
 *  open sockets or /proc/net files in a network namespace, and revert to the 'default' namespace.
 *  You can also execute a thread in a different network namespace by calling std_sys_setns
 *  at the beginning of the thread function.
 *  </p>
 *
 *  <p>Please refer to Linux 'setns' and 'namespaces' for more details.
 *
 *  @warning Do not use this function frequently/in a loop - it has relatively a relatively high CPU usage.
 *
 *  @param[in] net_namespace  new network name space to set - e.g. netnsname in 'ip netns ad netnsname'
 *  @param[out] prev_ns_handle handle to previous network name space
 *
 *  @return STD_ERR_OK if success, failure code otherwise
 *  @sa std_sys_reset_netns
 *
 * @verbatim

   #include "std_system.h"

   int ns_handle;
   t_std_error rc = std_sys_set_ns("netnsname", &ns_handle);

   // ... execute code in new namespace netns (e.g. open network device or '/proc/net' pseudofile )
   // ... reset to previous process name space

   rc = std_sys_reset_netns(&ns_handle);
   // Note. 'ns_handle' no longer valid after this call
 * @endverbatim
 *
 */
t_std_error std_sys_set_netns(const char* net_namespace, int* prev_ns_handle);

/**
 *  Reset the network name space for the current thread (or process - if single threaded).
 *  'prev_ns_handle' becomes invalid after this call.
 *
 *  @param[inout] prev_ns_handle handle to previous network name space
 *
 *  @return STD_ERR_OK if success, failure code otherwise
 *  @sa std_sys_set_netns
 *
 */
t_std_error std_sys_reset_netns(int* prev_ns_handle);

#ifdef __cplusplus
}
#endif

#endif /* __STD_SYSTEM_H */
