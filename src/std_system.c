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


#include "std_system.h"
#include "std_utils.h"
#include "event_log.h"

#include <limits.h>
#include <stdio.h>

const size_t FS_MOUNT_FIELD_LOC = 1;
const size_t FS_TYPE_FIELD_LOC = 2;
const char * FS_TYPE_SYSFS="sysfs";

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

