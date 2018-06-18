/*
 * Copyright (c) 2017 Dell Inc.
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
 * filename: std_condition_variable.c
 **/

#include "std_condition_variable.h"
#include "std_time_tools.h"
#include <pthread.h>

t_std_error std_condition_var_timed_init (std_condition_var_t* cond)
{
    // Set the clock to be MONOTONIC
    pthread_condattr_t attr;
    pthread_condattr_init(&attr);
    int err = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    if (err != 0) {
        return STD_ERR (COM, FAIL, err);
    }

    // Now we can initialize the pthreads objects with that condattr
    err = pthread_cond_init(cond, &attr);
    if (err != 0) {
        return STD_ERR (COM, FAIL, err);
    }
    return STD_ERR_OK;
}

t_std_error std_condition_var_timed_wait_until (std_condition_var_t* cond, std_mutex_type_t* mutex,
                                                const struct timespec* abs_time, bool* timedout)
{
    *timedout = false;
    int rc = pthread_cond_timedwait(cond, mutex, abs_time);
    if (rc == ETIMEDOUT) {
        *timedout = true;
        return STD_ERR_OK;
    }
    return (rc == 0) ? STD_ERR_OK:STD_ERR (COM, FAIL, rc);
}

t_std_error std_condition_var_timed_wait (std_condition_var_t* cond, std_mutex_type_t* mutex, size_t interval_in_ms,
                                          bool* timedout)
{
    struct timespec abs_time;
    std_time_get_monotonic_clock (interval_in_ms, &abs_time);
    return std_condition_var_timed_wait_until (cond, mutex, &abs_time, timedout);
}

