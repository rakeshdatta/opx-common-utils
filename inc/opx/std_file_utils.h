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
 * filename: std_file_utils.h
 */



/**
 *       @file  std_file_utils.h
 *      @brief  standard file utility functions
 *
 *   @internal
 *     Created  05/11/2014
 *     Company  DELL
 *   Copyright  Copyright (c) 2014, Cliff Wichmann
 *
 * =====================================================================================
 */

#ifndef __STD_FILE_UTILS_H
#define __STD_FILE_UTILS_H

#include "std_error_codes.h"
#include "std_type_defs.h"

/** \defgroup SocketsAndFilesCommon Socket and File utilities
*
* \{
*/

#define STD_INVALID_FD (-1)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   read from a file descriptor and give options to continue to read
 *          or finish after the first that can be ignored..  ignores eintr
 * @param   fd[in] the file descriptor to use
 * @param   data[in] the buffer to read into
 * @param   len[in] the length of data
 * @param   require_all[in] true if should wait for all data otherwise false
 * @param   err the error code if provided
 * @return  length read, end of file will be less then len regardless of require all.  On error returns -1 and can check the return code for the error reason
 *
 */
int std_read(int fd, void*data, int len, bool require_all, t_std_error *err);


/**
 * @brief     write to a file descriptor and give options to continue to write
 *          or finish after the first error that can be ignored..  ignores eintr
 * @param   fd[in] the file descriptor to use
 * @param   data[in] the buffer to write
 * @param   len[in] the length of data
 * @param   require_all[in] true if should send all data otherwise false
 * @param   err the error code if provided
 * @return  length written.  On error returns -1 and can check the return code for the error reason

 */
int std_write(int fd, void*data, int len, bool require_all, t_std_error *err);


/**
 * @brief   copy data from a file descriptor fdin to to a destination file descriptor fdout
 * @param   fdout[in] fd to write to
 * @param   fdin[in] fd to read from
 * @param   err[out] either a pointer to the error code or NULL if ignored
 * @return  the amount written or -1 on an error - additional data in err if
 *          provided
 */
int std_fd_copy(int fdout, int fdin, t_std_error *err);


/**
 * @brief   clone the a series of file descriptors from fin to fcones
 * @param   fclones is the array of destination fds
 * @param   fin is the array of file descriptors to clone
 * @param   len is the length of the array
 * @return  std error code
 */
t_std_error std_file_clone_fds(int *fclones, int *fin, int len);

/**
 * @brief   redirect the stdin descriptor to an new fd
 * @param   fd the fd to redirect
 * @return  std error code
 */
t_std_error std_redir_stdoutin(int fd);

/**
 * @brief   close the object associated with the descriptor
 * @param   fd  desriptor of object to be closed
 * @return  std error code
 */
t_std_error std_close (int fd);

#ifdef __cplusplus
}
#endif

/**
 * \}
 */

#endif
