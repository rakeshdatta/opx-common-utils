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
 * filename: std_system.c
 */

// _GNU_SOURCE needed for setns. Must be defined before any include directive
#define _GNU_SOURCE

#include "std_system.h"
#include "std_utils.h"
#include "event_log.h"

#include <sched.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define GENERIC_FAIL STD_ERR_MK(e_std_err_COM,e_std_err_code_PARAM,0)
#define RET_ERRNO STD_ERR_FROM_ERRNO(e_std_err_COM,e_std_err_code_PARAM)

#define NSNET_SZ (128)


static const size_t FS_MOUNT_FIELD_LOC = 1;
static const size_t FS_TYPE_FIELD_LOC = 2;
static const char * FS_TYPE_SYSFS="sysfs";




//!TODO Generalize this for all fields in the /proc/mounts file
/**
 * All searching of the /proc/mounts or any mounts/local directory items should be handled
 * in this file.
 */

/**
 *
 * @param sysfs_destination_string[out]
 * @param len[in]
 * @return
 */
t_std_error std_sys_sysfs_path_get(char *sysfs_destination_string, size_t len)
{
    FILE *f;
    char name[256];

    /* sysfs mount point is found through /proc/mounts */
    if ((f = fopen("/proc/mounts", "r")) == NULL) {
        EV_LOG_ERRNO(ev_log_t_COM,0,"COM-INVALID-SYSFS-MOUNT",errno);
        return STD_ERR(COM,FAIL,0);
    }
    bool found = false;
    while (!found && (fgets(name, sizeof(name), f) != NULL)) {
        std_parsed_string_t handle;
        if (!std_parse_string(&handle,name," ")) {
            continue;
        }
        const char * fsloc = std_parse_string_at(handle,FS_MOUNT_FIELD_LOC);
        const char * fstype = std_parse_string_at(handle,FS_TYPE_FIELD_LOC);
        if (fsloc==NULL || fstype==NULL) continue;

        //!ToDO look for  sysfs type and if not match
        found = (strcasecmp(fstype, FS_TYPE_SYSFS) == 0);
        if (found) {
            safestrncpy(sysfs_destination_string,fsloc,len);
        }
        std_parse_string_free(handle);
    }

    fclose(f);
    if (!found) {
        EV_LOG(ERR,COM,0,"COM-INVALID-SYSFS-MOUNT","Error : %s", "sysfs not found...");
        return STD_ERR(COM,FAIL,0);
    }
    return STD_ERR_OK;
}

t_std_error std_sys_execve_command(const char *cmd, const char *const argv[], const char *const envp[])
{
    pid_t pid = fork();
    if (0 == pid) {
        // Child process
        if(execve(cmd, (char* const*)argv, (char* const*)envp) == -1) {
            EV_LOGGING(ev_log_t_COM,ERR,"sys", "Unable to execve command %s error = %s(%d)", cmd, strerror(errno), errno);
            exit(errno);
        }
        // Note: this is never reached - but will keep the code analysis tools happy
        return STD_ERR_OK;
    }

    if (pid == -1) {
        EV_LOGGING(ev_log_t_COM,ERR,"sys", "Unable to fork command %s error = %s(%d)", cmd, strerror(errno), errno);
        return RET_ERRNO;
    }

    int status=0;
    waitpid(pid, &status, 0);
    if(!WIFEXITED(status) || (WEXITSTATUS(status) != 0)) {
        if(WIFEXITED(status)) {
            EV_LOGGING(ev_log_t_COM,ERR,"sys", "Command %s failed status =%d", cmd, (int)WEXITSTATUS(status));
        } else {
            EV_LOGGING(ev_log_t_COM,ERR,"sys", "Command %s failed", cmd);
        }
        return GENERIC_FAIL;
    }
    return STD_ERR_OK;
}

t_std_error std_sys_set_netns(const char* net_namespace, int* prev_ns_handle)
{

    t_std_error rc = GENERIC_FAIL;
    if (NULL == net_namespace) {
        EV_LOG_ERR (ev_log_t_COM,ERR,"sys", "Name space not provided");
        return rc;
    }
    if (NULL == prev_ns_handle) {
        EV_LOG_ERR (ev_log_t_COM,ERR,"sys", "NULL NS handle for: %s", net_namespace);
        return rc;
    }

    static const char* crt_nsnet = "/proc/self/ns/net";
    int sock_ns_fd = -1;
    char sock_nsnet[NSNET_SZ];

    *prev_ns_handle = -1; // Note: we do not want to include 'std_file_utils.h' for STD_INVALID_FD in order to avoid circular dependencies
    snprintf(sock_nsnet, NSNET_SZ, "/var/run/netns/%s", net_namespace);
    do {
        *prev_ns_handle = open(crt_nsnet, O_RDONLY);
        if (*prev_ns_handle < 0) {
            rc = RET_ERRNO;
            EV_LOGGING(ev_log_t_COM,ERR,"sys", "Cannot open %s errno=%s(%d)", crt_nsnet, strerror (errno), errno);
            break;
        }

        sock_ns_fd = open(sock_nsnet, O_RDONLY);
        if (sock_ns_fd < 0) {
            rc = RET_ERRNO;
            EV_LOGGING(ev_log_t_COM,ERR,"sys", "Cannot open %s errno=%s(%d)", sock_nsnet, strerror (errno), errno);
            break;
        }

        // Set network namespace for the socket
        if (setns(sock_ns_fd, CLONE_NEWNET) < 0) {
            rc = RET_ERRNO;
            EV_LOGGING(ev_log_t_COM,ERR,"sys", "'setns' for %s errno=%s(%d)", sock_nsnet, strerror (errno), errno);
            break;
        }
        rc = STD_ERR_OK;
    } while (0);

    if (sock_ns_fd >= 0) {
        (void)close(sock_ns_fd);
    }
    if (rc != STD_ERR_OK) {
        if (*prev_ns_handle >=0 ) {
            (void)close(*prev_ns_handle);
            *prev_ns_handle = -1;
        }
    }

    return rc;
}

t_std_error std_sys_reset_netns(int* prev_ns_handle)
{
    t_std_error rc = STD_ERR_OK;
    if (NULL == prev_ns_handle) {
        EV_LOG_ERR (ev_log_t_COM,ERR,"sys", "NULL NS handle");
        return GENERIC_FAIL;
    }

    if (setns(*prev_ns_handle, CLONE_NEWNET) < 0) {
        EV_LOGGING(ev_log_t_COM,ERR,"sys", "'setns' - reset errno=%s(%d)", strerror (errno), errno);
    }

    if (close(*prev_ns_handle) != 0) {
        EV_LOGGING(ev_log_t_COM,ERR,"sys", "Close error fd=%d errno=%s(%d)", *prev_ns_handle, strerror (errno), errno);
        rc = RET_ERRNO;
    }
    *prev_ns_handle = -1;
    return rc;
}
