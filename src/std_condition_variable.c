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
#include <pthread.h>
#include <time.h>

t_std_error std_condition_var_timed_init (std_condition_var_t* cond)
{
    // Set the clock to be MONOTONIC
    pthread_condattr_t attr;
    pthread_condattr_init(&attr);
    int err = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    if (err != 0) {
        return STD_ERR (COM, FAIL, 0);
    }

    // Now we can initialize the pthreads objects with that condattr
    pthread_cond_init(cond, &attr);
    return STD_ERR_OK;
}

bool std_condition_var_timed_wait (std_condition_var_t* cond, std_mutex_type_t* mutex, size_t time_in_millisec)
{
    size_t time_sec = (time_in_millisec / 1000);
    size_t time_nanosec = ((time_in_millisec % 1000) * 1000000);

    struct timespec timeout;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    timeout.tv_sec = now.tv_sec + time_sec;
    timeout.tv_nsec = now.tv_nsec + time_nanosec;

    int rc = pthread_cond_timedwait(cond, mutex, &timeout);
    if (rc == ETIMEDOUT)
        return true;

    return false;
}


