/*
 * Copyright (c) 2018 Dell Inc.
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
 * filename: std_thread_tools.c
 */


#define _GNU_SOURCE

#include "std_thread_tools.h"
#include "event_log.h"

#include <pthread.h>
#include <sched.h>
#include <linux/sched.h>
#include <stdlib.h>
#include <string.h>

static t_std_error std_thread_valid_params(std_thread_create_param_t * p) {
    t_std_error rc = STD_ERR_MK(e_std_err_COM,e_std_err_code_PARAM,0);
    if(p->name == NULL) return rc;

    if(p->prio == 0) {
        switch(p->sched_type) {
            case SCHED_OTHER:
            case SCHED_IDLE:
                break;
            default:
                return rc;
                break;
        }
    } else {
        switch(p->sched_type) {
            case SCHED_FIFO:
            case SCHED_RR:
                if (p->prio > sched_get_priority_max(p->sched_type)) return rc;
                break;
            default:
                break;
        }
    }
    if (p->thread_function==NULL) return rc;

    return STD_ERR_OK;
}

void std_thread_init_struct(std_thread_create_param_t *p) {
    p->name = NULL;
    p->stack_size = STD_THREAD_DEF_STACK;
    p->prio = 0;
    p->sched_type = SCHED_OTHER;
    p->thread_function = NULL;
    p->thread_id = NULL;
    p->param = NULL;
}

void std_thread_destroy_struct(std_thread_create_param_t *p) {
    if(p->thread_id!=NULL) free(p->thread_id);
    std_thread_init_struct(p);
}
#define STD_THREAD_NAME_LEN 15
t_std_error std_thread_create(std_thread_create_param_t * params) {
    t_std_error rc = std_thread_valid_params(params);

    if (rc!=STD_ERR_OK) {
        EV_LOG_ERR(ev_log_t_COM,0,"THR-SWERR",
                "Invalid parameters passed to thread create");
        return rc;
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setguardsize(&attr,1024);
    pthread_attr_setschedpolicy(&attr,params->sched_type);

    struct sched_param sp;
    sp.sched_priority = params->prio;
    pthread_attr_setschedparam(&attr,&sp);

    params->thread_id = malloc(sizeof(pthread_t));

    int l_errno = pthread_create((pthread_t*)params->thread_id,&attr,
                            params->thread_function,params->param);
    pthread_attr_destroy(&attr);

    if (l_errno!=0) {
        rc = STD_ERR_MK(e_std_err_COM,e_std_err_code_FAIL,l_errno);
        EV_LOG_ERR(ev_log_t_COM,0,"THR-SWERR",
            "Failed to create thread name:%s func:%lu",params->name,
            params->thread_function);
        free( params->thread_id );
        params->thread_id = NULL;
    } else rc = STD_ERR_OK;

    if(rc==STD_ERR_OK && params->name!=NULL) {
        /* Standard pthread name is restricted to 16 including
         * terminating null byte '\0' */
        char std_thread_name[STD_THREAD_NAME_LEN+1];
        strncpy(std_thread_name, params->name, STD_THREAD_NAME_LEN);
        std_thread_name[STD_THREAD_NAME_LEN] = '\0';
        pthread_setname_np(*((pthread_t*)params->thread_id),std_thread_name);
    }
    return rc;
}

t_std_error
std_thread_cancel(
        std_thread_create_param_t *params)
{
    int rc = 0;

    if ((NULL == params) || (NULL==params->thread_id)) {
        /* invalid parameters */
        EV_LOGGING(COM, ERR, "THR",
                "Invalid input params %p",
                params);
        return STD_ERR_MK(e_std_err_COM, e_std_err_code_PARAM, 0);
    }

    rc = pthread_cancel(*((pthread_t*)(params->thread_id)));
    if (0 != rc) {
        /* cancel failed */
        EV_LOGGING(COM, ERR, "THR",
                "Error %d, failed to cancel thread:%s func:%p",
                rc, params->name, params->thread_function);
        return STD_ERR_MK(e_std_err_COM, e_std_err_code_FAIL, rc);
    }

    /* cancel successful */
    return STD_ERR_OK;
}

t_std_error
std_thread_join(
        std_thread_create_param_t *params)
{
    int rc = 0;

    if ((NULL == params) || (NULL==params->thread_id)) {
        /* invalid parameters */
        EV_LOGGING(COM, ERR, "THR",
                "Invalid input params %p",
                params);
        return STD_ERR_MK(e_std_err_COM, e_std_err_code_PARAM, 0);
    }

    rc = pthread_join(*((pthread_t*)(params->thread_id)),NULL);
    if (0 != rc) {
        /* join failed */
        EV_LOGGING(COM, ERR, "THR",
                "Error %d, failed to join thread:%s func:%p",
                rc, params->name, params->thread_function);
        return STD_ERR_MK(e_std_err_COM, e_std_err_code_FAIL, rc);
    }

    /* join successful, destory the params */
    std_thread_destroy_struct(params);

    return STD_ERR_OK;
}

