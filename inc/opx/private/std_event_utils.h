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
 * filename: std_event_utils.h
 */


/*
 * std_event_utils.h
 *
 *  Created on: May 23, 2014
 */

#ifndef STD_EVENT_UTILS_H_
#define STD_EVENT_UTILS_H_

#include "std_error_codes.h"
#include "std_event_service.h"


#include <vector>
#include <stdint.h>

enum event_serv_msg_types_t {
    event_serv_msg_t_ADD_REG,
    event_serv_msg_t_DEL_REG,
    event_serv_msg_t_PUBLISH,
    event_serv_msg_t_BUFFER,
};

struct event_serv_msg_t {
    event_serv_msg_types_t op;
};

static inline uint8_t *vector_offset(std::vector<uint8_t> &v, size_t offset) {
    return &(v[offset]);
}

struct std_event_msg_descr_t {
    void *data;
    size_t len;
};

t_std_error std_event_util_event_send(std_event_client_handle handle,
        event_serv_msg_t *msg, std_event_msg_descr_t *data, size_t len, size_t timeout);

t_std_error std_event_util_event_recv(std_event_client_handle handle,
        std::vector<uint8_t> &buff, bool allow_resize=true);

t_std_error std_event_util_event_recv_msg(std_event_client_handle handle,
        std_event_msg_t *msg, void * data, size_t len) ;

#endif /* STD_EVENT_UTILS_H_ */
