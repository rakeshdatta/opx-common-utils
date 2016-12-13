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
 * std_thread_pool.h
 */

#ifndef STD_THREAD_POOL_H_
#define STD_THREAD_POOL_H_

#include "std_error_codes.h"
#include "std_thread_tools.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _std_thread_pool_job_t{
    /**
     * The context parameter to pass to the job funct call
     */
    void * context;

    /**
     * The job function to enqueue
     * @param context
     */
    void (*funct)(void *context);
    /**
     * This function is to free the context that is associated with a job.
     * The job should ideally be short and not a loop
     *
     * @param context that is in the job structure.
     */
    void (*free_job_func)(void * context);
} std_thread_pool_job_t;

typedef void *std_thread_pool_handle_t;

/**
 * Create a thread pool using the thread template (params) that are passed in with
 * the number of threads specified.  Will return a handle to the pool that can be used
 * to shutdown or add jobs
 * @param handle the handle to the thread pool that is created
 * @param params the thread "template" that will be used to create the threads
 * @param thread_pool_size the number of threads in the pool
 * @return STD_ERR_OK if successful otherwise a specific error code
 */
t_std_error std_thread_pool_create(std_thread_pool_handle_t *handle,
        std_thread_create_param_t * params, size_t thread_pool_size);

/**
 * Delete an active thread pool.  Will attempt to shutdown all threads in the pool.
 * @param handle the handle to the thread pool
 * @return STD_ERR_OK on success otherwise a failure
 */
t_std_error std_thread_pool_delete(std_thread_pool_handle_t handle);

/**
 * Add a job to the thread pool
 * @param handle the handle to the threadpool
 * @param job the job to add
 * @return STD_ERR_OK on success and the job will be added immediately otherwise a failure
 */
t_std_error std_thread_pool_job_add(std_thread_pool_handle_t handle,std_thread_pool_job_t *job);

#ifdef __cplusplus
}
#endif

#endif /* STD_THREAD_POOL_H_ */
