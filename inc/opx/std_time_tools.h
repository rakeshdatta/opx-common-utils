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
 * filename: std_time_tools.h
 */


/**
 *       @file  std_time_tools.h
 *      @brief  standard sleeping/time utilities
 *
 *   @internal
 *     Created  05/11/2014
 *     Company  DELL
 *
 * =====================================================================================
 */

#ifndef __STD_TIME_TOOLS_H
#define __STD_TIME_TOOLS_H

#include "std_error_codes.h"
#include "std_type_defs.h"



#ifdef __cplusplus
extern "C" {
#endif

#define MILLI_TO_MICRO(x)  ((x) * 1000)
#define MICRO_TO_MILLI(x)  ((x) / 1000)

/**
 * @brief   usleep for a period of time and ignore eintr
 * @param   usecs the usecs to sleep for
 * @return  standard error types
 */
t_std_error std_usleep(unsigned int usecs);

/**
 * @brief   get the current uptime in microseconds
 * @param   error code if desired or NULL
 * @return 0 for error otherwise the results of the call
 * @note 0 is a valid number as well but just very unlikely to be sure look at
 *          err
 */
uint64_t std_get_uptime(t_std_error *err);


/**
 * A simple timer check.  Check to see if the time before + the time in microseconds is longer
 * has elapsed (compared to the now)
 * Can be used as a simple passive timer.
 *
 * @param before the start time
 * @param time_in_ms the time in microseconds
 * @return true if the time has expired
 */
bool std_time_is_expired(uint64_t before, uint64_t time_in_ms);


#ifdef __cplusplus
}
#endif

#endif
