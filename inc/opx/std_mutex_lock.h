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
 * filename: std_mutex_lock.h
 */

#ifndef __STD_MUTEX_LOCK_H
#define __STD_MUTEX_LOCK_H

/** \defgroup LocksCommon Locking and Condition variable utilities
*
* \{
*/


/*
 * The following is code to work around an issue with pthreads that requires the use of GNU
 * when using static initialization for recursive mutexes
 */

#ifndef _GNU_SOURCE
#define __GNU_SOURCE_DEFINED
#define _GNU_SOURCE
#endif

#include <pthread.h>
#include "std_error_codes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef pthread_mutex_t std_mutex_type_t;

/**
 * The name is name of the mutex variable to be created.  This will create a
 * recursive mutex lock.
 */
#define std_mutex_lock_create_static_init_rec(name) \
        pthread_mutex_t name = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP

/**
 * This will create a simple mutex lock with the specified name
 */
#define std_mutex_lock_create_static_init_fast(name) \
    pthread_mutex_t name = PTHREAD_MUTEX_INITIALIZER


/**
 * Initialize the mutext with the settings to make it a non-recursive mutex
 * @param mutex the pointer mutex to initialize
 * @return
 */
t_std_error std_mutex_lock_init_non_recursive(std_mutex_type_t *mutex);

/**
 * Initialize the mutex pointer with the settings to make it completely recursive
 * @param mutex the pointer mutex to initialize
 * @return a standard return code
 */
t_std_error std_mutex_lock_init_recursive(std_mutex_type_t *mutex);


/**
 * Delete a standard mutex
 */
#define std_mutex_destroy pthread_mutex_destroy

/**
 * This API locks a mutex - pass a pointer to the created mutex.
 * ie. std_mutex_lock(&mutex);
 */
#define std_mutex_lock pthread_mutex_lock
/**
 * This API unlocks a mutex - pass a pointer to the created mutex.
 * ie. std_mutex_lock(&mutex);
 */
#define std_mutex_unlock pthread_mutex_unlock

/**
 * This API trys to lock a mutex - pass a pointer to the created mutex.
 * ie. std_mutex_lock(&mutex);
 */
#define std_mutex_trylock pthread_mutex_trylock


#ifdef __cplusplus
}
#endif


#ifdef __cplusplus

/**
 * This is a CPP Class wrapper to handle mutex locking and unlocking.
 *
 * This wil lock a mutex on construction and unlocks it based on a destruction
 */
class std_mutex_simple_lock_guard {
    std_mutex_type_t * m;    //! a pointer to a mutex
public:
    /**
     * Create a mutex guard with the specified mutex lock
     */
    std_mutex_simple_lock_guard(std_mutex_type_t *l) : m(l) {
        std_mutex_lock(m);
    }

    /**
     * Clean up a mutex lock
     */
    ~std_mutex_simple_lock_guard() {
        std_mutex_unlock(m);
    }
};

#endif

#ifdef __GNU_SOURCE_DEFINED
#undef __GNU_SOURCE_DEFINED
#undef _GNU_SOURCE
#endif

/**
 * \}
 */

#endif

