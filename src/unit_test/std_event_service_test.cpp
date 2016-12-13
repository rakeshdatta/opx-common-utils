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
 * filename: std_event_service_test.c
 */

/*
 * std_event_service_test.c
 *
 *  Created on: May 23, 2014
 */


#include "std_event_service.h"
#include <std_time_tools.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "gtest/gtest.h"

std_event_server_handle_t handle;


void send_event(std_event_client_handle handle,uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e,
        uint32_t f, const char *s ) {
    char buff[1500];
    void *p = buff;

    std_event_msg_t *msg = (std_event_msg_t*)p;
    msg->key.len = 5;
    msg->key.event_key[0]=a;
    msg->key.event_key[1]=b;
    msg->key.event_key[2]=c;
    msg->key.event_key[3]=d;
    msg->key.event_key[4]=e;
    msg->key.event_key[5]=f;

    memcpy(std_event_get_data(msg),s,strlen(s)+1);
    msg->data_len = (500+rand()) %1000;

    if(std_client_publish_msg(handle,msg)!=STD_ERR_OK) {
        printf("Send error \n");
    }
}

void *create_child_2(void *p) {
    std_event_client_handle handle;
    if (std_server_client_connect(&handle, "/tmp/event_channel_name")!=STD_ERR_OK) {
        printf("connect failed\n");
        exit(1);
    }
    sleep(1);
    size_t ix = 0;
    size_t mx = 100000;
    for ( ; ix < mx ; ++ix ) {
        send_event(handle,0,1,2,3,4,ix,(char*)"Cliff");
    }
    std_server_client_disconnect(handle);
    printf("finished\n");
    return NULL;
}

void *create_child_1(void *p) {
    std_event_client_handle handle;
    if (std_server_client_connect(&handle, "/tmp/event_channel_name")!=STD_ERR_OK) {
        printf("Client 1 con failed\n");
        exit(1);
    }

    std_event_key_t key;
    key.len = 6;
    key.event_key[0] = 0;
    key.event_key[1] = 1;
    key.event_key[2] = 2;
    key.event_key[3] = 3;
    key.event_key[4] = 4;
    key.event_key[5] = 5;

    if (std_client_register_interest(handle,&key,1)!=STD_ERR_OK) {
        exit(1);
    }
    //purposfully register a handle that is doing nothing...
    if (std_server_client_connect(&handle, "/tmp/event_channel_name")!=STD_ERR_OK) {
        printf("Client 1 con failed\n");
        exit(1);
    }
    if (std_client_register_interest(handle,&key,1)!=STD_ERR_OK) {
        exit(1);
    }

    key.len = 1;
    if (std_client_register_interest(handle,&key,1)!=STD_ERR_OK) {
        printf("Client 1 reg failed\n");
        exit(1);
    }


    pthread_t id;
    pthread_create(&id,NULL,create_child_2,NULL);

   std_event_msg_buff_t buff = std_client_allocate_msg_buff(5000,true);

   std_client_set_receive_buffer(handle,50000000);

    size_t count = 0;
    while (std_client_wait_for_event(handle,buff)==STD_ERR_OK) {
        std_event_msg_t *msg = std_event_msg_from_buff(buff);

        count++;
        std_usleep(1000);
        if (count == 100000) {
            printf("Count reached\n");
            exit(0);
        }

        if (count%1000) continue;

        printf("(%d) - Received key %d:%d:%d:%d:%d:%d - %d\n",(int)count,
                msg->key.event_key[0],msg->key.event_key[1],msg->key.event_key[2],
                msg->key.event_key[3],msg->key.event_key[4],msg->key.event_key[5],
                msg->data_len);
    }
    printf("Receive loop failed at %d\n",(int)count);
    std_client_free_msg_buff(&buff);
    return NULL;
}

void *create_child_3(void *p) {
    std_event_client_handle handle;
    if (std_server_client_connect(&handle, "/tmp/event_channel_name")!=STD_ERR_OK) {
        printf("Client 1 con failed\n");
        exit(1);
    }

    std_event_key_t key;
    key.len = 6;
    key.event_key[0] = 0;
    key.event_key[1] = 1;
    key.event_key[2] = 2;
    key.event_key[3] = 3;
    key.event_key[4] = 4;
    key.event_key[5] = 5;

    if (std_client_register_interest(handle,&key,1)!=STD_ERR_OK) {
        exit(1);
    }
    //purposfully register a handle that is doing nothing...
    if (std_server_client_connect(&handle, "/tmp/event_channel_name")!=STD_ERR_OK) {
        printf("Client 1 con failed\n");
        exit(1);
    }
    if (std_client_register_interest(handle,&key,1)!=STD_ERR_OK) {
        exit(1);
    }
    std_client_set_receive_buffer(handle,10000);
    key.len = 1;
    if (std_client_register_interest(handle,&key,1)!=STD_ERR_OK) {
        printf("Client 1 reg failed\n");
        exit(1);
    }


    pthread_t id;
    pthread_create(&id,NULL,create_child_2,NULL);

   std_event_msg_buff_t buff = std_client_allocate_msg_buff(5000,true);

    size_t count = 0;
    while (std_client_wait_for_event(handle,buff)==STD_ERR_OK) {
        std_event_msg_t *msg = std_event_msg_from_buff(buff);

        count++;
        std_usleep(1000);
        if (count == 100000) {
            printf("Count reached\n");
            exit(0);
        }

        if (count%1000) continue;

        printf("(%d) - Received key %d:%d:%d:%d:%d:%d - %d\n",(int)count,
                msg->key.event_key[0],msg->key.event_key[1],msg->key.event_key[2],
                msg->key.event_key[3],msg->key.event_key[4],msg->key.event_key[5],
                msg->data_len);
    }
    std_client_free_msg_buff(&buff);

    printf("Receive loop failed at %d\n",(int)count);
    return NULL;
}


bool create_service(void *param) {
    std_event_server_handle_t handle;
    if (std_event_server_init(&handle,"/tmp/event_channel_name",10)!=STD_ERR_OK) {
        printf("Server init failed\n");
        return false;
    }
    return true;
}

TEST(std_cfg_file_test, FileClose)
{
    ASSERT_TRUE(create_service(NULL));

    create_child_1(NULL);
    sleep(30);
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
