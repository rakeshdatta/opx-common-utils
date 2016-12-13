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
 * std_rw_lock.h
 */

#ifndef STD_RW_LOCK_H_
#define STD_RW_LOCK_H_

#include "std_error_codes.h"

#include <pthread.h>
#include <errno.h>

/** \defgroup LocksCommon Locking and Condition variable utilities
*
* \{
*/

typedef pthread_rwlock_t std_rw_lock_t;

/**
 * This will initialize a read/write lock within a single process. Locking/unlocking is within the process only
 * @param lock is a pointer to the std_rw_lock_t that you want to init
 * @return STD_ERR_OK if good otherwise FAIL
 */
static inline t_std_error std_rw_lock_create_default(std_rw_lock_t *lock) {
    int rc = pthread_rwlock_init(lock,NULL);
    if (rc==0) return STD_ERR_OK;
    return STD_ERR(COM,FAIL,errno);
}

/**
 * Delete the previously created rw lock
 * @param lock is the lock to delete - will cancel any pending locking operations (with an error)
 * @return STD_ERR_OK if successful otherwise a failure code
 */
static inline t_std_error std_rw_lock_delete(std_rw_lock_t *lock) {
    int rc = pthread_rwlock_destroy(lock);
    if (rc==0) return STD_ERR_OK;
    return STD_ERR(COM,FAIL,errno);
}

/**
 * lock a rw mutex with a read lock only
 * @param the pointer to the lock
 */
#define std_rw_rlock pthread_rwlock_rdlock

/**
 * lock a rw mutex with a write lock only
 * @param the pointer to the lock
 */
#define std_rw_wlock pthread_rwlock_wrlock

/**
 * unlock a rw mutex with a lock only
 * @param the pointer to the lock
 */
#define std_rw_unlock pthread_rwlock_unlock

#ifdef __cplusplus

/**
 * This is a utility class to lock and unlock "read locks" with a rw lock.
 * On construction this class will lock the rw mutex and on destruction it will
 * release the lock
 */
class std_rw_lock_read_guard {
    std_rw_lock_t * m;    //! a pointer to a rw lock
public:
    /**
     * Create a read rw guard with the specified rw lock
     */
    std_rw_lock_read_guard(std_rw_lock_t *l) : m(l) {
        std_rw_rlock(m);
    }

    /**
     * Clean up lock
     */
    ~std_rw_lock_read_guard() {
        std_rw_unlock(m);
    }
};

/**
 * This is a utility class to lock and unlock "write locks" with a rw lock.
 * On construction this class will lock the rw mutex and on destruction it will
 * release the lock
 */
class std_rw_lock_write_guard {
    std_rw_lock_t * m;    //! a pointer to a rw lock
public:
    /**
     * Create a write rw guard with the specified rw lock
     */
    std_rw_lock_write_guard(std_rw_lock_t *l) : m(l) {
        std_rw_wlock(m);
    }

    /**
     * Clean up lock
     */
    ~std_rw_lock_write_guard() {
        std_rw_unlock(m);
    }
};


#endif

/**
 * \}
 */

#endif /* STD_RW_LOCK_H_ */
