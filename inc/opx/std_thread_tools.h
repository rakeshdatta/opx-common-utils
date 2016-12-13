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
 * filename: std_thread_tools.h
 */



/**
 *       @file  std_thread_tools.h
 *      @brief  create/general thread tools
 *
 *   @internal
 *     Created  05/11/2014
 *     Company  DELL
 *
 * =====================================================================================
 */

#ifndef __STD_THREAD_TOOLS_H
#define __STD_THREAD_TOOLS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "std_error_codes.h"

#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#define STD_THREAD_DEF_STACK (8192)

typedef void * (*std_thread_function_t)(void * param);
typedef struct std_thread_create_param_s {
    const char *name;
    unsigned int stack_size;
    unsigned int prio;
    int sched_type;
    void *thread_id;
    std_thread_function_t thread_function;
    void * param;
} std_thread_create_param_t;


/**
 * @brief   Initializes the thread structure to default values
 * @param   p the structure to initialize
 * @return  none
 */
void std_thread_init_struct(std_thread_create_param_t *p) ;


/**
 * @brief   Clean out a thread structure and free any associated memory
 * @param   p the thread structure
 * @return  none
 */
void std_thread_destroy_struct(std_thread_create_param_t *p) ;



/**
 * @brief create a thread with the parameters specificed in the structure.
 *          please use the std_thread_init_struct to initialize the structure
 * @param   params is the parameter that describes the return code
 * @return  the error code
 */
t_std_error std_thread_create(std_thread_create_param_t * params);


/**
 * @brief Join with a terminating thread
 * Wait for the thread to exit using data in the std_thread_create_param_t structure.
 * The join takes the create thread parameters as the thread ID is maintained
 * and there may be other fields in the std_thread_create_param_t structure that can be used to shutdown
 * the thread.
 *
 * @param params the structure that was passed to the successful create create which
 *         started the thread.
 */
void std_thread_join(std_thread_create_param_t *params) ;


/**
 * @brief Get the process id for the currently running process.
 * @return the uint64_t identifier of the process.
 */
static inline uint64_t std_thread_id_get() {
    return (uint64_t)pthread_self();
}

/**
 * @brief Get the thread id of the current thread.
 * @return the uint64_t thread identifier of the current thread
 */
static inline uint64_t std_process_id_get() {
    return (uint64_t)getpid();
}

#ifdef __cplusplus
}
#endif

#endif
