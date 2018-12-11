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
static size_t count;

static std::chrono::microseconds ut_timestamp ()
{
    static auto start = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds> (std::chrono::steady_clock::now() - start);
}

static std::vector<std::string> buff;

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

static std_mutex_type_t timed_mutex;
static std_condition_var_t timed_wait;
static bool timed_wait_exit_ = false;
static uint_t timeout_cntr = 0;
static uint_t signal_cntr = 0;

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

static struct timeval cur_time;

void thread_timed_wait ()
{
    std_mutex_lock (&timed_mutex);
    while (true) {
        bool timedout;
        auto rc = std_condition_var_timed_wait (&timed_wait, &timed_mutex, SEC_TO_MILLISEC(1.5), &timedout);
        ASSERT_EQ (rc, STD_ERR_OK) << "System error in condition var timed wait - errno " << STD_ERR_EXT_PRIV(rc);

        if (timed_wait_exit_) {
            std::cout << "Exiting timed wait - Child thread Terminated"<< std::endl;
            break;
        }
        std_mutex_unlock (&timed_mutex);
        if (timedout) {
            // Timeout
            struct timespec now;
            clock_gettime(CLOCK_MONOTONIC, &now);
            std::cout << "#" << timeout_cntr++ << " Timedout " << now.tv_sec << " sec " << now.tv_nsec << " ns" << std::endl;
        } else {
            std::cout << "#" << signal_cntr++ << " No Timeout" << std::endl;
        }
        std_mutex_lock (&timed_mutex);
    }
    std_mutex_unlock(&timed_mutex);
}

TEST(std_condition_var_timed, test){
    std_mutex_lock_init_non_recursive(&timed_mutex);
    int rc = std_condition_var_timed_init(&timed_wait);
    ASSERT_EQ (rc, STD_ERR_OK) << "System error in condition var timed init - errno " << STD_ERR_EXT_PRIV(rc);

    std::thread* p = new std::thread {[] () {
        thread_timed_wait();
    }};

    std::cout << "Waiting 9 seconds to see if timed cond var fires every 1.5 sec even when clock is adjusted"<< std::endl;
    std_usleep (SEC_TO_MICROSEC(4));
    rc = gettimeofday (&cur_time, nullptr);
    if (rc != 0) {
        std::cout << "Could not get time due to insufficient privileges" << std::endl;
    } else {
        cur_time.tv_sec -= 300; // Set clock back by 5 mins
        rc = settimeofday (&cur_time, nullptr);
        if (rc != 0) {
            std::cout << "Could not change time due to insufficient privileges" << std::endl;
        } else {
            std::cout << "Changed time by 5 mins"<< std::endl;
        }
    }
    std_usleep (SEC_TO_MICROSEC(5) + SEC_TO_MICROSEC(.03));

    ASSERT_TRUE (timeout_cntr == 6) << "Incorrect cond-var timeouts " << timeout_cntr;

    std::cout << "Signaling cond var - attempt 1" << std::endl;
    std_condition_var_signal (&timed_wait);

    std_usleep (SEC_TO_MICROSEC(1));

    std_mutex_lock (&timed_mutex);
    timed_wait_exit_ = true;
    std_mutex_unlock (&timed_mutex);
    std::cout << "Signaling cond var to exit child thread" << std::endl;
    std_condition_var_signal (&timed_wait);

    std::cout << "Joining child thread" << std::endl;
    p->join ();

    ASSERT_EQ (signal_cntr, 1) << "Cond-var did not exit when signaled";
}

TEST(std_condition_var_cpp, pred_test){
    std_condition_var cv_cpp;
    std::vector<int> test_list;
    std_mutex_type_t cv_mtx;
    std_mutex_lock_init_non_recursive(&cv_mtx);

    std::thread* p = new std::thread {[&] () {
        int i=0;
        std::cout << "Pushing item to list every 100ms" << std::endl;
        while (test_list.size()<33) { // Exit thread when count reaches 32
            std::this_thread::sleep_for (std::chrono::milliseconds(100));
            std::cout << "Pushed "<< test_list.size() << " items" << std::endl;
            {
                std_mutex_simple_lock_guard lg {&cv_mtx};
                test_list.push_back (++i);
            }
            cv_cpp.notify (); // Signal cond-var every 100 ms
        }
    }};

    try {
        bool ret;
        {
            std_mutex_simple_lock_guard lg {&cv_mtx};

            ret = cv_cpp.wait_for (cv_mtx, SEC_TO_MILLISEC (4), // 4 sec - should fire only after predicate becomes true
                                  [&] () -> bool {
                                     return (test_list.size() > 30); // Should return from  wait as soon as count reaches 31
                                                                     // approx 3.1 sec
                                  });
            std::cout << "Wait-for returned when items = " << test_list.size() << std::endl;
            ASSERT_TRUE (test_list.size() == 31) << "wait_for() did not exit when Predicate is True - count " << test_list.size();
        }
        ASSERT_TRUE (ret) << "wait_for() returned False when Predicate is True";

        p->join();
        test_list.clear();
    } catch (const std::system_error& e) {
        ASSERT_TRUE (0) << "Error " << e.code() << " - " << e.what ();
    }
}

TEST(std_condition_var_cpp, time_test){
    std_condition_var cv_cpp;
    std::vector<int> test_list;
    std_mutex_type_t cv_mtx;
    std_mutex_lock_init_non_recursive(&cv_mtx);

    std::thread* p = new std::thread {
        [&] () {
            int i = 0;
            std::cout << "Pushing item to list every 100ms" << std::endl;
            while (i<22) { // Exit thread when count reaches 22
                std::this_thread::sleep_for (std::chrono::milliseconds(100));
                std::cout << "Pushed "<< test_list.size() << " items" << std::endl;
                {
                    std_mutex_simple_lock_guard lg {&cv_mtx};
                    test_list.push_back (++i);
                }
                cv_cpp.notify (); // Signal cond-var every 100 ms
            }
        }
    };

    try {
        bool ret;
        {
            std_mutex_simple_lock_guard lg {&cv_mtx};
            ret = cv_cpp.wait_for (cv_mtx, SEC_TO_MILLISEC(1.5), // 1.5 sec - should fire BEFORE predicate becomes true
                                  [&] () -> bool {
                                     return (test_list.size() > 20); // Should return from wait as soon as count reaches 21
                                  });

            std::cout << "Wait-for returned when items = " << test_list.size() << std::endl;
            // There should be 14-15 items in the vector when the wait_for() exits after 1.5 sec
            ASSERT_TRUE (test_list.size() <= 15) << "wait_for() did not exit when Timer popped - count " << count;
            ASSERT_TRUE (test_list.size() >= 14) << "cond-var did not return when signaled - count " << count;
        }
        ASSERT_FALSE (ret) << "wait_for() returned True even when Predicate is False";
        p->join();
    } catch (const std::system_error& e) {
        ASSERT_TRUE (0) << "Error " << e.code() << " - " << e.what ();
    }
}

TEST(std_condition_var_cpp, defer_test){
    bool test_exit = false;
    std_condition_var cv_cpp;
    std::vector<int> test_list;
    std_mutex_lock_create_static_init_rec (cv_mtx);
    uint_t proc_count = 0;
    uint_t push_count = 0;

    std::thread* p = new std::thread {
    [&] () {
        try {
            std_mutex_simple_lock_guard lg {&cv_mtx};
            while (!test_exit) {
                int iter = 0;
                if (test_list.empty()) {
                    std::cout << ut_timestamp().count() << "us: Block until next signal" << std::endl;
                    // Wait for signal
                    cv_cpp.wait (cv_mtx);
                }
                std::cout << ut_timestamp().count() << "us: - First Wake up" << std::endl;

                while (!test_list.empty()) {
                    // Timed wait
                    auto ret = cv_cpp.defer_for (cv_mtx, 1, // defer for 1ms after waking up from condition
                                                 [&] () -> bool {
                                                    return ((test_list.size() > 50) || test_exit); // Quit if either of these are true
                                                 });

                    if (ret) {
                        std::cout << ut_timestamp().count() << "us: - Forced Wake up: ";
                    } else {
                        std::cout << ut_timestamp().count() << "us: - Deferred Wake up: ";
                    }
                    std::cout << "list size = " << test_list.size();
                    std::cout << "; test_exit = " << test_exit << std::endl;
                    // Either timeout or size 50 reached
                    if (++iter == 1) {
                        // First iteration wake up should be caused by max list
                        EXPECT_TRUE ((test_list.size()>50) && ret);
                    } else {
                        // Second iteration should be a timeout
                        EXPECT_TRUE ((test_list.size()<50) && !ret);
                    }
                    proc_count += test_list.size();
                    test_list.clear();
                    std_mutex_unlock (&cv_mtx);

                    // Simulate processing time - 5 ms
                    std::this_thread::sleep_for (std::chrono::milliseconds(5));
                    std::cout << ut_timestamp().count() << "us: - Completed Processing list" << std::endl;

                    // Stagger processing of next bunch
                    std::this_thread::sleep_for (std::chrono::milliseconds(1));
                    std::cout << ut_timestamp().count() << "us: - Resume after stagger" << std::endl;
                    std_mutex_lock (&cv_mtx);
                } // More work pending
            } // test_exit
        } catch (const std::system_error& e) {
            ASSERT_TRUE (0) << "Error " << e.code() << " - " << e.what ();
        }}
    };

    int i = 0;
    std::this_thread::sleep_for (std::chrono::seconds(1));
    std::cout << ut_timestamp().count() << "us: Pushing bunch of items to list" << std::endl;
    while (i<90) {
        {
            std_mutex_simple_lock_guard lg {&cv_mtx};
            test_list.push_back (++i);
            ++push_count;
        }
        cv_cpp.notify ();
        std::this_thread::sleep_for (std::chrono::microseconds(100));
    }
    std::cout << ut_timestamp().count() << "us: Completed Pushing 90 items." << std::endl;
    i= 0;
    std::this_thread::sleep_for (std::chrono::milliseconds(10));
    std::cout << ut_timestamp().count() << "us: Pushing next bunch" << std::endl;
    while (i<75) {
        {
            std_mutex_simple_lock_guard lg {&cv_mtx};
            test_list.push_back (++i);
            ++push_count;
        }
        cv_cpp.notify ();
        std::this_thread::sleep_for (std::chrono::microseconds(100));
    }

    std::cout << ut_timestamp().count() << "us: Completed pushing second bunch" << std::endl;
    std::this_thread::sleep_for (std::chrono::milliseconds(500));
    std::cout << "Sending terminate" << std::endl;
    {
        std_mutex_simple_lock_guard lg {&cv_mtx};
        test_exit = true;
        cv_cpp.notify (); // Signal cond-var every 100 ms
    }

    p->join();
    ASSERT_EQ (push_count, proc_count);
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
