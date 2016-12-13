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

/**
 * filename: std_event_service.h
 **/

/*
 * std_event_service.h
 *
 *  Created on: May 23, 2014
 */

#ifndef STD_EVENT_SERVICE_H_
#define STD_EVENT_SERVICE_H_

#include "std_bit_masks.h"
#include "std_error_codes.h"

#include <stdint.h>

#define STD_EVENT_MAX_EVENT_MSG_SIZE (5000)

#define STD_EVENT_PRIO_RANGE (5000)
#define STD_EVENT_MIDDLE_PRIO (10000)
#define STD_EVENT_HIGH_PRIO (STD_EVENT_MIDDLE_PRIO - STD_EVENT_PRIO_RANGE)
#define STD_EVENT_LOW_PRIO (STD_EVENT_MIDDLE_PRIO + STD_EVENT_PRIO_RANGE)

/**
 * A reserved class.  Events of class 0 are internal only
 */

#define STD_EVENT_CLASS_INTERNAL (0)

/**
 * The first possible event service class ID regardless of the instance...
 */
#define STD_EVENT_FIST_CLASS_ID (1)

typedef void * std_event_server_handle_t;

typedef int std_event_client_handle;

typedef void * std_event_msg_buff_t;


#define STD_EVENT_KEY_MAX 16

typedef struct std_event_key_t {
    uint32_t len;
    uint32_t event_key[STD_EVENT_KEY_MAX];
}std_event_key_t ;

/**
 * The message structure sent and received by the API
 * @TODO byte need to think about alignment of structure - add pragma
 */
typedef struct std_event_msg_s {
    std_event_key_t key;
    uint32_t data_len;/** the length of the data */
} std_event_msg_t;

/**
 * Get the pointer to the events data given the start of the message
 * @param msg the pointer to the message
 * @return a pointer to the message's data
 */
static inline void * std_event_get_data(std_event_msg_t *msg) {
    return (void*)(((uint8_t*)msg) + sizeof(*msg));
}

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create an event service with the given name and returns the handle if successful
 *
 * @param handle the handle to the event service created
 * @param event_channel_name the event channel name
 * @return a standard error code or STD_ERR_OK if successful
 */
t_std_error std_event_server_init(std_event_server_handle_t *handle, const char * event_channel_name,
        size_t threads);

/**
 * @brief connect to the event service from another process (or thread)
 * @param handle the handle to hold the client's connection details
 * @param event_channel_name the event channel name
 * @return standard return code
 */
t_std_error std_server_client_connect(std_event_client_handle * handle, const char *event_channel_name);

/**
 * Close a channel with the common event service
 * @param handle a valid event service handle
 * @return STD_ERR_OK on success otherwise a failure return code
 */
t_std_error std_server_client_disconnect(std_event_client_handle handle);

/**
 * @brief send a message to the event service through an client connection handle
 *
 * @param handle handle created with from the register client API
 * @param msg the message to send
 * @return standard return code
 */
t_std_error std_client_publish_msg(std_event_client_handle handle, std_event_msg_t *msg);

/**
 * The following function will allow a user to publish a message with the components
 * instead of a message structure.  This can be useful when the application
 * has data to send but not a message structure formed
 * @param handle the handle to the event service
 * @param key the key for the data
 * @param data the data to send in the event
 * @param len  the length of the data to send
 * @return STD_ERR_OK if the event has successfully been sent
 */
t_std_error std_client_publish_msg_data(std_event_client_handle handle,
        std_event_key_t *key,void * data, size_t len) ;

/**
 * @brief register the specified event classes with the event service - so when these types of
 * events are sent, the client is sent a copy
 *
 * @param handle created from a register client API
 * @param keys the keys (pointer to a array of keys) to register for  on this event handle
 * @param len the length of the key list being registered
 * @return a standard return code
 */
t_std_error std_client_register_interest(std_event_client_handle handle,
        std_event_key_t *keys, size_t len);

/**
 * @brief register the specified event classes with the event service - so when these types of
 * events are sent, the client is sent a copy
 *
 * @param handle created from a register client API
 * @param keys the keys (pointer to a array of keys) to unregister for  on this event handle
 * @param len the length of the key list being unregistered
 * @return a standard return code
 */
t_std_error std_client_remove_interest(std_event_client_handle handle, std_event_key_t *keys, size_t len);


/**
 * Indicate that the specified amount of buffer space should be reserved on from the event service.
 * Allows engineering of buffers
 *
 * @param handle created from a register client API
 * @param len is the amount of bytes to allow pending on the event channel before the channel is deem filled up.
 *
 * @return a standard return code
 */
t_std_error std_client_set_receive_buffer(std_event_client_handle handle, size_t len);

/**
 * @brief wait for a message from the event service
 * @param handle opened from a previous client registration
 * @param buff the message buffer used to receive the message.  This buffer can be
 *         preallocated and sized appropriately.
 * @return standard return code
 */
t_std_error std_client_wait_for_event(std_event_client_handle handle, std_event_msg_buff_t buff);

/**
 * @brief wait for a message from the event service
 * @param handle opened from a previous client registration
 * @param msg the message header received including the keys and len.
 * @param data this will be where the contents of the message data will be stored on receive
 * @param len this is the maximum length of the buffer that can be received
 * @return standard return code
 */
t_std_error std_client_wait_for_event_data(std_event_client_handle handle,
        std_event_msg_t *msg, void *buff, size_t len);

/**
 * @brief allocate a message with the specified space
 * @param space the maximum size of data that will be received
 * @param  enforce_max_limit if set the buffer will not resize otherwise  this buffer can adjust
 * its space based on the message being received.
 *
 * @return NULL on error otherwise the data
 */
std_event_msg_buff_t std_client_allocate_msg_buff(unsigned int buffer_space, bool enforce_max_limit);

/**
 * @brief free the message buffer
 * @param msg to free
 */
void std_client_free_msg_buff(std_event_msg_buff_t *msg);

/**
 * Given a message buffer, get a pointer to the message data
 * @param buff a message buffer that holds a valid message
 * @return the pointer to the message structure
 */
std_event_msg_t * std_event_msg_from_buff(std_event_msg_buff_t buff);

/**
 * @brief print out the contents of a message
 * @param msg to print
 */
void std_event_util_print_msg(std_event_msg_t *msg);


#ifdef __cplusplus
}
#endif

#endif /* STD_EVENT_SERVICE_H_ */
