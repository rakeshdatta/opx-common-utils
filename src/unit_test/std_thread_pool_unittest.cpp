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
 * std_thread_pool_unittest.cpp
 */


#include <stdio.h>
#include "gtest/gtest.h"

#include "std_mutex_lock.h"
#include <unistd.h>
#include "std_thread_pool.h"

std_mutex_lock_create_static_init_rec(lock);
static volatile int count = 0;

void inc_count(void *param) {
    std_mutex_lock(&lock);
    ++count ;
    printf("(%d) - %d\n",*(int*)param,(int)count);
    sleep(count % 1);
    std_mutex_unlock(&lock);
}

void free_func(void *context) {
    std_mutex_lock(&lock);
    free(context);
    std_mutex_unlock(&lock);
}

void test() {
    std_thread_create_param_t param;
    std_thread_init_struct(&param);
    param.name = "cliff";
    std_thread_pool_handle_t handle;
    if (std_thread_pool_create(&handle,&param,100)!=STD_ERR_OK) {
        exit (1);
    }
    size_t ix = 0;
    size_t mx = 1000;
    std_thread_pool_job_t j;

    j.funct = inc_count;
    j.free_job_func = free_func;
    for ( ; ix < mx ; ++ix ) {
        j.context = (void*)malloc(sizeof(int));
        *((int*)j.context) = ix;
        if (std_thread_pool_job_add(handle,&j)!=STD_ERR_OK) {
            exit(1);
        }
    }
    while ((count+1) < (int)mx ) sleep(1);
    std_thread_pool_delete(handle);
}

TEST(std_thread_pool_create, create)
{
    test();
}



int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
