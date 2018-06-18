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
 * std_condition_variable.h
 */

#ifndef STD_CONDITION_VARIABLE_H_
#define STD_CONDITION_VARIABLE_H_

#include <pthread.h>

#include "std_error_codes.h"
#include "std_mutex_lock.h"
#include "std_time_tools.h"

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup LocksCommon Locking and Condition variable utilities
* \{
*/

/**
 * Condition variable that can be used in conjunction with a mutex to provide
 * a general purpose signaling functionality with threads.  Normally a thread will
 * have a queue or just of things to do, that it will wait on a condition variable until
 * there is some work in the queue or list to do
 *
 * Generally a thread will:
 * -> lock a mutex
 * -> while want to run..
 *   -> condition wait (when woken up it will have the mutex)
 *   -> do the work required and take the mutex
 *
 * Someone posting work will:
 * -> lock the mutex
 * -> add work to the list
 * -> signal the condition
 * -> unlock the mutex
 *
 */

/**
 * Condition variable that can be used in conjunction with a mutex to provide
 */
typedef  pthread_cond_t std_condition_var_t;

/**
 * Initialize the condition variable with the defaults that should be suitable for most
 * applications
 * @param var is the condition variable to initialize
 * @return STD_ERR_OK if the operation was successful or STD_ERR(COM,FAIL,0) if failed
 */
static inline t_std_error std_condition_var_init(std_condition_var_t *var) {
    return pthread_cond_init(var,NULL)==0 ? STD_ERR_OK : STD_ERR(COM,FAIL,0);
}

/**
 * Destroy the condition lock
 * @param cond is the pointer to the condition variable to destroy
 */
#define std_condition_var_destroy pthread_cond_destroy

/**
 * Signal that the condition variable - essentially wake up a thread in order that it
 * was waiting on the condition
 * @param cond the pointer to the condition variable
 * @return STD_ERR_OK if the condition was signaled otherwise an error code
 */
#define std_condition_var_signal pthread_cond_signal

#define  std_condition_var_broadcast pthread_cond_broadcast
/**
 * Wait for the condition varaible to become active.  Requires the mutex that must be locked
 * prior to entering this function call.
 * @param cond the condition variable to wait on.
 * @param lock the mutex lock guarding the data that the mutex will be operating on
 * @return STD_ERR_OK if there is work discovered otherwise an error code
 */
static inline t_std_error std_condition_var_wait(std_condition_var_t *cond,
        std_mutex_type_t *lock) {
    return pthread_cond_wait(cond,lock)==0 ? STD_ERR_OK : STD_ERR(COM,FAIL,0);
}

/**
 * Initialize the condition variable to use a monotonic clock with a timedwait
 * @param var is the condition variable to initialize
 * @return STD_ERR_OK if the operation was successful or STD_ERR(COM,FAIL,err) if failed
 */
t_std_error std_condition_var_timed_init(std_condition_var_t *var);

/**
 * Wait for either a condition to be signaled or for the relative time to pass.
 * Requires a mutex that must be locked prior to entering this function call.
 * Can unblock spuriously without timeout or condition being signaled.
 * @param cond the condition variable to wait on.
 * @param lock the mutex lock guarding the data that the condition will be operating on
 * @param interval_in_ms the relative time in millisec
 * @param[out] timedout True if unblocked due to interval expires before condition is signaled.
 *                      False otherwise
 * @return STD_ERR_OK if unblocked by timeout, condition signal, other interrupt
 *         STD_ERR(COM,FAIL,err) if failed
 */
t_std_error std_condition_var_timed_wait (std_condition_var_t* cond, std_mutex_type_t* lock,
                                          size_t interval_in_ms, bool* timedout);

/**
 * Wait for either a condition to be signaled or for system clock to reach the specified absolute time.
 * Requires a mutex that must be locked prior to entering this function call.
 * Can unblock spuriously without timeout or condition being signaled.
 * @param cond the condition variable to wait on.
 * @param lock the mutex lock guarding the data that the condition will be operating on
 * @param abs_time the absolute clock time to wait for
 * @param[out] timedout True if absolute time is reached.
 *                      False if the cond var became active or if an interrupt was
 *                      received before absolute time is reached
 * @return STD_ERR_OK if unblocked by timeout, condition signal, other interrupt
 *         STD_ERR(COM,FAIL,err) if failed
 */
t_std_error std_condition_var_timed_wait_until (std_condition_var_t* cond, std_mutex_type_t* lock,
                                                const struct timespec* abs_time, bool* timedout);
/**
 * \}
 */

#ifdef __cplusplus
}

#include <system_error>

class std_condition_var {
    std_condition_var_t m_cv;

    public:
    std_condition_var () {
        auto rc = std_condition_var_timed_init (&m_cv);
        if (STD_ERR_OK != rc) {
            int ec = STD_ERR_EXT_PRIV(rc);
            throw std::system_error {std::error_code(ec, std::system_category()),
                                     "Condition Variable init failed"};
        }
    }

    /**
     * Wait for Condition to be signaled and Predicate to be True OR for the interval to expire.
     * Requires a mutex that must be locked prior to entering this function call.
     *
     * @param pred callable object that should return true in addition to signaling condition to unblock
     * @param mutex the mutex lock guarding the data that the condition will be operating on
     * @param interval_in_ms the relative time to wait for in millisec
     * @return True if the Predicate is True
     *         False if unblocked otherwise
     * @throw std::system_error if invalid interval or mutex is passed in or if mutex is not owned by calling thread
     */
     template< class Predicate >
        bool wait_for(std_mutex_type_t& mutex,
                      size_t interval_in_ms, Predicate pred)
    {
        struct timespec abs_time;
        std_time_get_monotonic_clock (interval_in_ms, &abs_time);

        while (!pred()) {
            bool timeout = false;
            auto rc = std_condition_var_timed_wait_until (&m_cv, &mutex, &abs_time,
                                                          &timeout);
            if (STD_ERR_OK != rc) {
                int ec = STD_ERR_EXT_PRIV(rc);
                throw std::system_error {std::error_code(ec, std::system_category()),
                                         "Condition Variable timed wait failed"};
            }
            if (timeout) {
                // Timeout
                return pred();
            }
        }
        return true;
    }

    /**
     * Wait for the condition variable to be signaled and the Predicate to return True.
     * Requires a mutex that must be locked prior to entering this function call.
     *
     * @param pred callable object that should return true in addition to signaling condition to unblock
     * @param mutex the mutex lock guarding the data that the condition will be operating on
     * @throw std::system_error if invalid mutex is passed in or if mutex is not owned by calling thread
     */
    template< class Predicate >
        void wait (std_mutex_type_t& mutex, Predicate pred)
    {
        while (!pred()) {
            auto rc = std_condition_var_wait (&m_cv, &mutex);
            if (STD_ERR_OK != rc) {
                int ec = STD_ERR_EXT_PRIV(rc);
                throw std::system_error {std::error_code(ec, std::system_category()), "Condition Variable wait failed"};
            }
        }
    }
    /**
     * Wait for the condition to be signaled.
     * Requires a mutex that must be locked prior to entering this function call.
     * Can unblock spuriously without condition being signaled.
     * @param mutex lock guarding the data that the condition will be operating on
     * @throw std::system_error if invalid mutex is passed in or if mutex is not owned by calling thread
     */
    void wait (std_mutex_type_t& mutex) {
        auto rc = std_condition_var_wait (&m_cv, &mutex);
        if (STD_ERR_OK != rc) {
            int ec = STD_ERR_EXT_PRIV(rc);
            throw std::system_error {std::error_code(ec, std::system_category()), "Condition Variable wait failed"};
        }
    }

    /**
     * Defer for specified interval, as long as Predicate evaluates to False.
     * If condition is signaled before interval expires, it still defers unblocking
     * for specified interval from the last condition signal.
     * Requires a mutex that must be locked prior to entering this function call.
     *
     * @param pred callable object that should evaluate to True to override and unblock without delay
     * @param mutex the mutex lock guarding the data that the condition will be operating on
     * @param delay_in_ms interval in millisec to delay after condition is signaled
     * @return True if unblocked before delay time because Pred() evaluates to True
     *         False if the delay time expired
     * @throw std::system_error if invalid delay or mutex is passed in or if mutex is not owned by calling thread
     */
     template< class Predicate >
        bool defer_for (std_mutex_type_t& mutex, size_t delay_in_ms, Predicate pred)
    {
        // Keep defering for interval every time after waking up from condition wait
        // Unblock if interval expires or if predicate evaluates to True
        while (!pred()) {
            bool timeout = false;
            auto rc = std_condition_var_timed_wait (&m_cv, &mutex, delay_in_ms, &timeout);
            if (STD_ERR_OK != rc) {
                int ec = STD_ERR_EXT_PRIV(rc);
                throw std::system_error {std::error_code(ec, std::system_category()),
                                         "Condition Variable timed wait failed"};
            }
            if (timeout) {return false;}
        }
        return true;
    }

    void notify_all () noexcept {
        std_condition_var_broadcast (&m_cv);
    }
    void notify () noexcept {
        std_condition_var_signal (&m_cv);
    }
};


#endif
#endif /* STD_CONDITION_VARIABLE_H_ */
