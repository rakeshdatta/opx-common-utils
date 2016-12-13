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
 * std_thread_pool.cpp
 */

#include "std_thread_pool.h"
#include "std_mutex_lock.h"
#include "std_condition_variable.h"
#include "event_log.h"
#include "std_time_tools.h"
#include "std_file_utils.h"

#include <unistd.h>
#include <stdio.h>

#include <vector>
#include <list>

struct std_thread_pool_context_t {
    enum {SHUTDOWN_WAIT_TIME=10000};

    std_mutex_type_t m_lock;
    std_condition_var_t m_cond;

    std::vector<std_thread_create_param_t> list;
    std::list<std_thread_pool_job_t> jobs;

    bool shutdown;

    bool dequeue();

    bool add_work(std_thread_pool_job_t *job) ;
    void do_shutdown() ;
    void wait() {
        std_condition_var_wait(&m_cond,&m_lock);
    }
    void signal(void) {
        std_condition_var_signal(&m_cond);
    }
    void lock(void) {
        std_mutex_lock(&m_lock);
    }
    void unlock(void) {
        std_mutex_unlock(&m_lock);
    }
};


bool std_thread_pool_context_t::add_work(std_thread_pool_job_t *job) {
    lock();
    try {
        jobs.push_back(*job);
    } catch (...) {
        unlock();
        return false;
    }
    signal();
    unlock();
    return true;
}

void std_thread_pool_context_t::do_shutdown() {
    shutdown = true;
    size_t ix = 0;
    size_t mx = list.size();
    for ( ; ix < mx ; ++ix ) {
        lock();
        signal();
        unlock();
    }
    std_condition_var_destroy(&m_cond);
    std_mutex_destroy(&m_lock);
    std_usleep(SHUTDOWN_WAIT_TIME);
}

bool std_thread_pool_context_t::dequeue() {
    if (shutdown) {
        signal();
        return false;
    }
    size_t len = jobs.size();
    if (len>0) {
        std_thread_pool_job_t job = jobs.front();
        jobs.pop_front();
        if (len > 1 ) {
            signal();
        }
        unlock();

        job.funct(job.context);
        if (job.free_job_func!=NULL) job.free_job_func(job.context);

        lock();
    }
    return jobs.size()>0;
}

static void * worker_thread(void *param) {
    std_thread_pool_context_t * ctx = (std_thread_pool_context_t*)param;
    ctx->lock();
    while (!ctx->shutdown) {
        if (ctx->dequeue()) continue;
        ctx->wait();
    }
    ctx->unlock();
    return NULL;
}


t_std_error std_thread_pool_create(std_thread_pool_handle_t *handle,
        std_thread_create_param_t * params, size_t thread_pool_size) {

    std_thread_pool_context_t *p = new std_thread_pool_context_t;
    if (p==NULL) return STD_ERR(COM,FAIL,0);

    p->shutdown = false;
    params->param = p;

    std_mutex_lock_init_non_recursive(&p->m_lock);
    std_condition_var_init(&p->m_cond);

    params->thread_function = worker_thread;

    bool valid = true;
    size_t ix = 0;
    size_t mx = thread_pool_size;
    for ( ; ix < mx && valid; ++ix ) {
        valid = std_thread_create(params)==STD_ERR_OK;
        if (valid) p->list.push_back(*params);
    }
    if (valid) {
        *handle = p;
        return STD_ERR_OK;
    }

    p->shutdown = true;
    p->do_shutdown();
    delete p;
    return STD_ERR(COM,FAIL,0);
}

t_std_error std_thread_pool_delete(std_thread_pool_handle_t handle) {
    std_thread_pool_context_t *p = (std_thread_pool_context_t *)handle;
    p->shutdown = true;
    p->do_shutdown();
    delete p;
    return STD_ERR_OK;
}


t_std_error std_thread_pool_job_add(std_thread_pool_handle_t handle,std_thread_pool_job_t *job) {
    std_thread_pool_context_t *p = (std_thread_pool_context_t *)handle;
    return (p->add_work(job)) ? STD_ERR_OK : STD_ERR(COM,FAIL,0);
}
