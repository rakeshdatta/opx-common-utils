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
 * filename: std_time_tools.c
 */


#include "std_time_tools.h"
#include "std_error_codes.h"

#include <unistd.h>
#include <time.h>


#ifdef CLOCK_MONOTONIC_RAW
#define _CLOCK_ID CLOCK_MONOTONIC_RAW
#else
#define _CLOCK_ID CLOCK_MONOTONIC
#endif

t_std_error std_usleep(unsigned int usecs) {
    register int rc = 0;
    register int fail_cnt = 5;
    while (fail_cnt-->0) {
        rc = usleep(usecs);
        if (rc==-1 && errno!=EINTR) {
            return STD_ERR_FROM_ERRNO(e_std_err_COM,e_std_err_code_FAIL);
        }
        if (rc==0) break;
    }
    return rc == 0 ? STD_ERR_OK : STD_ERR_FROM_ERRNO(e_std_err_COM,
                                                        e_std_err_code_FAIL);
}


uint64_t std_get_uptime(t_std_error *err) {
    struct timespec now;
    /*really just want clock monotonic but looks like an annoying behaviuor
    of changing based on ntp adjustments.. so use raw*/
    uint64_t val = 0;

    int rc = clock_gettime(_CLOCK_ID,&now);
    if (err!=NULL) {
        *err = rc==0 ? STD_ERR_OK : STD_ERR_FROM_ERRNO(e_std_err_COM,
                                    e_std_err_code_FAIL);
    }
    if (rc==0) {
        val = now.tv_nsec/1000;
        val += ((uint64_t)now.tv_sec)*1000000;
    }

    return val;
}

bool std_time_is_expired(uint64_t before, uint64_t time_in_ms) {
    uint64_t now = std_get_uptime(NULL);
    return (now - before) >= time_in_ms;
}
