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
 * std_condition_var_unittest.cpp
 */


#include "gtest/gtest.h"
#include "std_condition_variable.h"
#include "std_mutex_lock.h"
#include "std_thread_tools.h"

#include <vector>
#include <string>

static std_mutex_type_t mutex;
static std_condition_var_t cond;
size_t count;

std::vector<std::string> buff;

void *thread_wait(void*p) {
    std_mutex_lock(&mutex);
    while (true) {
        if (buff.size()>0) {
            printf("%X - buff (%s)\n",(int)pthread_self(),buff[0].c_str());
            buff.erase(buff.begin());
            std_condition_var_signal(&cond);
            ++count;
            continue;
        }
        std_condition_var_wait(&cond,&mutex);
    }
    std_mutex_unlock(&mutex);
    return NULL;
}

void add_string(const std::string &s) {
    std_mutex_lock(&mutex);
    buff.push_back(s);
    std_condition_var_signal(&cond);
    std_mutex_unlock(&mutex);
}

bool test() {
    std_mutex_lock_init_non_recursive(&mutex);
    std_condition_var_init(&cond);

    std_thread_create_param_t p;
    std_thread_init_struct(&p);
    p.name = "asdads";
    p.thread_function = thread_wait;

    size_t ix = 0;
    size_t mx = 100;
    for (  ; ix < mx ; ++ix ) {
        std_thread_create(&p);
    }
    for (  ix  = 0 ; ix < 1000 ; ++ix ) {
        add_string("Cliff");
    }


    while (count < ix) sleep(1);

    return true;
}

TEST(std_thread_pool_create, create)
{
    ASSERT_TRUE(test());
}



int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
