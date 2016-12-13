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
 * std_condition_variable.h
 */

#ifndef STD_CONDITION_VARIABLE_H_
#define STD_CONDITION_VARIABLE_H_

#include <pthread.h>

#include "std_error_codes.h"
#include "std_mutex_lock.h"

/** \defgroup LocksCommon Locking and Condition variable utilities
* \{
*/

/**
 * Condition variable that can be used in conjunction with a mutex to provide
 * a general purpose signaling functionality with threads.  Normally a thread will
 * have a queue or just of things to do, that it will wait on a condition variable until
 * there is some work in the queue or list to do
 *
 * Generally a thread will:
 * -> lock a mutex
 * -> while want to run..
 *   -> condition wait (when woken up it will have the mutex)
 *   -> do the work required and take the mutex
 *
 * Someone posting work will:
 * -> lock the mutex
 * -> add work to the list
 * -> signal the condition
 * -> unlock the mutex
 *
 */

/**
 * Condition variable that can be used in conjunction with a mutex to provide
 */
typedef  pthread_cond_t std_condition_var_t;

/**
 * Initialize the condition variable with the defaults that should be suitable for most
 * applications
 * @param var is the condition variable to initialize
 * @return STD_ERR_OK if the operation was successful or STD_ERR(COM,FAIL,0) if failed
 */
static inline t_std_error std_condition_var_init(std_condition_var_t *var) {
    return pthread_cond_init(var,NULL)==0 ? STD_ERR_OK : STD_ERR(COM,FAIL,0);
}

/**
 * Destroy the condition lock
 * @param cond is the pointer to the condition variable to destroy
 */
#define std_condition_var_destroy pthread_cond_destroy

/**
 * Signal that the condition variable - essentially wake up a thread in order that it
 * was waiting on the condition
 * @param cond the pointer to the condition variable
 * @return STD_ERR_OK if the condition was signaled otherwise an error code
 */
#define std_condition_var_signal pthread_cond_signal

#define  std_condition_var_broadcast pthread_cond_broadcast
/**
 * Wait for the condition varaible to become active.  Requires the mutex that must be locked
 * prior to entering this function call.
 * @param cond the condition variable to wait on.
 * @param lock the mutex lock guarding the data that the mutex will be operating on
 * @return STD_ERR_OK if there is work discovered otherwise an error code
 */
static inline t_std_error std_condition_var_wait(std_condition_var_t *cond,
        std_mutex_type_t *lock) {
    return pthread_cond_wait(cond,lock)==0 ? STD_ERR_OK : STD_ERR(COM,FAIL,0);
}

/**
 * \}
 */

#endif /* STD_CONDITION_VARIABLE_H_ */
