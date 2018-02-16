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
#include "std_time_tools.h"

#include <time.h>
#include <sys/time.h>
#include <vector>
#include <string>
#include <iostream>
#include <chrono>
#include <thread>

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

#define INTERVAL  1000 // in millisec
std_mutex_type_t timed_mutex;
std_condition_var_t timed_wait;
bool timed_wait_exit_ = false;
uint_t timeout_cntr = 0;
uint_t notimeout_cntr = 0;

void thread_timed_wait ()
{
    std_mutex_lock (&timed_mutex);
    while (true) {
        bool rc = std_condition_var_timed_wait (&timed_wait, &timed_mutex, INTERVAL);
        if (timed_wait_exit_) {
            std::cout << "Exiting timed wait - Child thread Terminated"<< std::endl;
            std_mutex_unlock (&timed_mutex);
            break;
        }
        std_mutex_unlock (&timed_mutex);
        if (rc) {
            // Timeout
            struct timespec now;
            clock_gettime(CLOCK_MONOTONIC, &now);
            std::cout << "#" << timeout_cntr++ << " Timedout " << now.tv_sec << " sec " << now.tv_nsec << " ns" << std::endl;
        } else {

            std::cout << "#" << notimeout_cntr++ << " No Timeout" << std::endl;
        }
        std_mutex_lock (&timed_mutex);
    }
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

TEST(std_condition_var, test)
{
    ASSERT_TRUE(test());
}

struct timeval tv;

TEST(std_condition_var_timed, test){
    std_mutex_lock_init_non_recursive(&timed_mutex);
    std_condition_var_timed_init(&timed_wait);

    std::thread* p = new std::thread {[] () {
        thread_timed_wait();
    }};

    std::cout << "Waiting 30 seconds to see if timed cond var fires"<< std::endl;
    std_usleep (10*1000000);
    int rc = gettimeofday (&tv, nullptr);
    if (rc != 0) {
        std::cout << "Could not get time due to insufficient privileges" << std::endl;
    } else {
        tv.tv_sec -= 300;
        rc = settimeofday (&tv, nullptr);
        if (rc != 0) {
            std::cout << "Could not change time due to insufficient privileges" << std::endl;
        } else {
            std::cout << "Changed time by 5 mins"<< std::endl;
        }
    }
    std_usleep (20*1000000);

    std::cout << "Signaling cond var - attempt 1" << std::endl;
    std_condition_var_signal (&timed_wait);

    std_usleep (1000000);

    std_mutex_lock (&timed_mutex);
    timed_wait_exit_ = true;
    std_mutex_unlock (&timed_mutex);
    std::cout << "Signaling cond var to exit child thread" << std::endl;
    std_condition_var_signal (&timed_wait);

    std::cout << "Joining child thread" << std::endl;
    p->join ();

    ASSERT_TRUE (timeout_cntr > 20);
    ASSERT_EQ (notimeout_cntr, 1);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
