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
 * std_mutex_lock.c
 */

#include "std_mutex_lock.h"


static t_std_error std_mutex_lock_init_type(std_mutex_type_t *mutex, int type) {
    pthread_mutexattr_t attr;
    t_std_error rc = STD_ERR(COM,FAIL,0);
    if (pthread_mutexattr_init(&attr)!=0) return rc;
    do {
        if (pthread_mutexattr_settype(&attr,type)!=0) break;
        if (pthread_mutex_init(mutex,&attr)!=0) break;
        rc = STD_ERR_OK;
    } while (0);
    pthread_mutexattr_destroy(&attr);
    return rc;
}


t_std_error std_mutex_lock_init_non_recursive(std_mutex_type_t *mutex) {
    return std_mutex_lock_init_type(mutex,PTHREAD_MUTEX_NORMAL);
}
t_std_error std_mutex_lock_init_recursive(std_mutex_type_t *mutex) {
    return std_mutex_lock_init_type(mutex,PTHREAD_MUTEX_RECURSIVE);
}
