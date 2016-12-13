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
 * filename: std_shlib.h
 */


/**
 *  @file  std_shlib.h
 *  @brief Shared Library management (load/unload library,
 *     map functions from library to function pointers
 *
 *
 *   @internal
 *     Created  05/11/2014
 * =====================================================================================
 */

#ifndef __STD_SHLIB_H
#define __STD_SHLIB_H

#include "std_error_codes.h"
#include "std_type_defs.h"
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup CommonShLib Common - Shared Library Management
 *
 * API to manage shared object library (load /unload libraries into/out of process space).
 *
 * @{
 */
/** Invalid shared library handle */
#define STD_SHLIB_INVALID_HNDL ((std_shlib_hndl)0)

/** Shared library handle */
typedef void* std_shlib_hndl;

/** Structure used to describe where the function pointer
 * associated to a given function name is to be stored */
typedef struct {
    /** Name of function to map to *pp_func */
    const char *name;

    /** Location where the pointer associated to the function name is to be stored
     * (at the location specified by "pp_func") */
    void** pp_func;
} std_shlib_func_map_t;

/**
 * Load a shared library into the current process address space,
 * and map its functions to the function pointers provided in the func_desc parameters
 * It is guaranteed that the library is not mapped to the process spaced in the case any error occurs.
 *
 * @warning This function is not re-entrant.
 *
 * @param [in] shlib_name name of library to load (must be in the loader path)
 * @param [out] hndl library handle (undefined if the function returns an error)
 * @param [out] func_desc function name map; the 'name' field must be initialized prior to the call in order
 * to provide the names of the functions to be mapped
 * @param [in] func_map_sz size (number of elements) in func_map
 * @return STD_ERR_OK or one of the following error codes:
 * @return STD_ERR(COM,FAIL,EINVAL) the parameters provided as input are invalid
 * @return STD_ERR(COM,FAIL,EEXIST) library already loaded
 * @return STD_ERR(COM,FAIL,ENOENT) library cannot be loaded
 * @return STD_ERR(COM,FAIL,EFAULT) failure to map one of the functions (provided by name)
 *
 * @note In the case of STD_ERR(COM,FAIL,EFAULT),
 * the function pointers that could not be mapped are set to NULL as result of the call.
 * All other function pointers (corresponding to valid library functions) are initialized,
 * but cannot be used (since the library is not mapped to process space in case of error).
 * This allows user applications to figure out the functions implemented by the shared library.
 *
 * @sa std_shlib_unload
 *
 *
 * File "libexample.c"
 @verbatim
 int libexample_func1 (int p1) {
     return 0;
 }
 void libexample_func2(void) {}
 @endverbatim
 *
 * File "use_shlib.c"
 @verbatim
 #include "std_shlib.h"

 #define MY_LIB_EXAMPLE "libexample.so"

 // Function pointers
 typedef struct {
      int (*f1)(int);
      void (*f2)(void);
 } my_functions_t;

 static my_functions_t my_func;

 static std_shlib_func_map_t func_map[] = {
     { "libexample_func1", (void **)&my_func.f1},
     { "libexample_func2", (void **)&my_func.f2},
 };
 static const size_t func_map_size = sizeof(func_map)/sizeof(std_shlib_func_map_t);
 static std_shlib_hndl lib_hndl = STD_SHLIB_INVALID_HNDL;

 if (STD_ERR_OK != std_shlib_load(MY_LIB_EXAMPLE, &lib_hndl, func_map, func_map_size)) {
      // error
      return;
 }

 // Ok, symbols can be used

 my_func.f2();

 if (STD_ERR_OK != std_shlib_unload(lib_hndl)) {
      // the library could not be unloaded
      return;
 }

 // This call would cause a crash, since the library is now unloaded
 my_func.f1();

 @endverbatim

 */
t_std_error std_shlib_load(const char* shlib_name, std_shlib_hndl *hndl,
        std_shlib_func_map_t func_map[], size_t func_map_sz);

/**
 * Unload a previously loaded shared library module. No functions from the library can be accessed after this operation.
 * @param [in] hndl (valid) library handle
 * @return STD_ERR_OK or error code
 * @return STD_ERR(COM,FAIL,EINVAL)  hndl is invalid
 * @return STD_ERR(COM,FAIL,ENOENT)  library could not be closed (was not mapped)
 *
 * @sa std_shlib_load
 */
t_std_error std_shlib_unload(const std_shlib_hndl hndl);

/**
 * Verify whether library "shlib_name" is already loaded in the memory of the calling process.
 *
 * @param [in] shlib_name name of library to verify mapping to current process space
 * @return true if loaded, false otherwise
 */
bool std_shlib_is_loaded(const char* shlib_name);


/**
 * @}
 *
 */

#ifdef __cplusplus
};
#endif

#endif
