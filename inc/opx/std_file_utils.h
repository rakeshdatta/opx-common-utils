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
 * filename: std_file_utils.h
 */



/**
 *       @file  std_file_utils.h
 *      @brief  standard file utility functions
 *
 *
 * =====================================================================================
 */

#ifndef __STD_FILE_UTILS_H
#define __STD_FILE_UTILS_H

#include "std_error_codes.h"
#include "std_type_defs.h"

#include <stddef.h>
#include <stdio.h>

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
 * @param[in]   fd the file descriptor to use
 * @param[in]   data the buffer to read into
 * @param[in]   len the length of data
 * @param[in]   require_all true if should wait for all data otherwise false
 * @param[out]   err the error code if provided
 * @return  length read, end of file will be less then len regardless of require all.  On error returns -1 and can check the return code for the error reason
 *
 */
int std_read(int fd, void*data, int len, bool require_all, t_std_error *err);


/**
 * @brief     write to a file descriptor and give options to continue to write
 *          or finish after the first error that can be ignored..  ignores eintr
 * @param[in]   fd the file descriptor to use
 * @param[in]   data the buffer to write
 * @param[in]   len the length of data
 * @param[in]   require_all true if should send all data otherwise false
 * @param[out]   err the error code if provided
 * @return  length written.  On error returns -1 and can check the return code for the error reason

 */
int std_write(int fd, void*data, int len, bool require_all, t_std_error *err);


/**
 * @brief   copy data from a file descriptor fdin to to a destination file descriptor fdout
 * @param[in]   fdout fd to write to
 * @param[in]   fdin fd to read from
 * @param[out]   err either a pointer to the error code or NULL to ignore
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
 * @param[in]   fd the fd to redirect
 * @return  std error code
 */
t_std_error std_redir_stdoutin(int fd);

/**
 * @brief   close the object associated with the descriptor
 * @param[in]   fd  desriptor of object to be closed
 * @return  std error code
 */
t_std_error std_close (int fd);

/**
 * @brief   write a set of buffers and give options to continue to write
 *          or finish after the first error that can be ignored..  ignores eintr
 * @param[in]   fd the file descriptor to use
 * @param[in]   data the buffers to write
 * @param[in]   data_lens the length of data
 * @param[in]     set_len the length of the data/data_lens arrays
 * @param[in]   require_all true if should send all data otherwise false
 * @param[out]   err the error code if provided as non NULL pointer
 * @return  length written.  On error returns -1 and can check the return code for the error reason
 */

ssize_t std_write_set(int fd, void**data, size_t *data_lens,  size_t set_len, bool require_all,
        t_std_error *err);


/**
 * @brief   read into a set of buffers and give options to continue to read
 *          or finish after the first error that can be ignored..  ignores eintr
 * @param[in]   fd the file descriptor to use
 * @param[in]   data the buffer to read into
 * @param[in]   data_lens the lengths of the corresponding data buffer
 * @param[in]   set_len the size of the list of data/data_lens
 * @param[in]   require_all true if should send all data otherwise false
 * @param[out]   err the error code if provided
 * @return  length written.  On error returns -1 and can check the return code for the error reason
 */
ssize_t std_read_set(int fd, void**data, size_t *data_lens,  size_t set_len, bool require_all,
        t_std_error *err);

/**
 * @brief   Open a networking specific file descriptor in a given network namespace.
 *
 * @Note. Use 'fdopen' to obtain a 'FILE*' pointer - if needed.
 *
 * @param[in]   net_namespace network namespace
 * @param[in]   filename filename to open
 * @param[in]   flags open flags same as for 'open' e.g. O_RDONLY
 * @param[out]  fd returned file descriptor
 *
 * @return  STD_ERR_OK or an error code
 */
t_std_error std_netns_fd_open (const char *net_namespace, const char *filename, int flags,
                               int* fd);

/**
 * @brief Determine if the filename is safe for an operator to access on the
 *        local filesystem. For security purposes, a file path is considered
 *        unsafe if any of these conditions is true:
 *        1) the filename contains a relative path (UP_DIR "..") token
 *        2) the filename contains invalid chars
 *
 * @param[in] filename - the filename (including path) to inspect
 *
 * @return true if relative path is safe;
 *         false otherwise
 */
bool
std_file_uri_safe_local_path(
        const char *filename);


#ifdef __cplusplus
}
#endif

/**
 * \}
 */

#endif
