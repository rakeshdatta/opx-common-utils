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
 * filename: std_shlib_gtest.cpp
 */


#include "std_shlib.h"
#include "gtest/gtest.h"

#include <stdio.h>

#define MY_LIB_EXAMPLE "std_shlib_test_lib"

// Function pointers
typedef struct {
    int (*f1)(int);
    void (*f2)(void);
    int (*f3)(int, int);
} functions_t;

static std_shlib_hndl lib_hndl = STD_SHLIB_INVALID_HNDL;
static functions_t my_func;

// Please refer to "std_shlib_test_lib.c" for valid function names and signatures
static std_shlib_func_map_t func_map[] = {
     { "shlib_test_func1", (void **)&my_func.f1},
     { "shlib_test_func2", (void **)&my_func.f2},
     { "shlib_test_func3", (void **)&my_func.f3},
};
static const size_t func_map_size = sizeof(func_map)/sizeof(std_shlib_func_map_t);


TEST(std_shlib_test, LoadInvalidLib)
{
    // Cannot load not-existing library
    t_std_error rc = std_shlib_load("libnonexisting.so", &lib_hndl, func_map, func_map_size);
    ASSERT_FALSE(STD_ERR_OK == rc);
    t_std_error expected_rc = STD_ERR(COM,FAIL,ENOENT);
    ASSERT_TRUE(expected_rc == rc);
    ASSERT_FALSE(std_shlib_is_loaded("libnonexisting.so"));

    rc = std_shlib_load("libnonexisting.so", NULL, func_map, func_map_size);
    expected_rc = STD_ERR(COM,FAIL,EINVAL);
    ASSERT_TRUE(expected_rc == rc);

    rc = std_shlib_load(NULL, &lib_hndl, func_map, func_map_size);
    expected_rc = STD_ERR(COM,FAIL,EINVAL);
    ASSERT_TRUE(expected_rc == rc);
}

TEST(std_shlib_test, UnloadInvalidLib)
{
    t_std_error rc = std_shlib_unload(STD_SHLIB_INVALID_HNDL);
    ASSERT_FALSE (STD_ERR_OK == rc);
    t_std_error expected_rc = STD_ERR(COM,FAIL,EINVAL);
    ASSERT_TRUE(expected_rc == rc);
}

TEST(std_shlib_test, LoadLib)
{
    ASSERT_FALSE(std_shlib_is_loaded(MY_LIB_EXAMPLE));
    t_std_error rc = std_shlib_load(MY_LIB_EXAMPLE, &lib_hndl, func_map, func_map_size);
    if (rc != STD_ERR_OK) {
        fprintf(stderr, "%s must be in the shared lib search path\n", MY_LIB_EXAMPLE);
    }
    ASSERT_TRUE (STD_ERR_OK == rc);
    ASSERT_TRUE (lib_hndl != STD_SHLIB_INVALID_HNDL);

    std_shlib_hndl hndl = STD_SHLIB_INVALID_HNDL;
    rc = std_shlib_load(MY_LIB_EXAMPLE, &hndl, func_map, func_map_size);
    t_std_error expected_rc = STD_ERR(COM,FAIL,EEXIST);
    ASSERT_TRUE (expected_rc == rc);

    ASSERT_TRUE(std_shlib_is_loaded(MY_LIB_EXAMPLE));
    ASSERT_TRUE(my_func.f1 != NULL);
    ASSERT_TRUE(my_func.f2 != NULL);
    ASSERT_TRUE(my_func.f3 != NULL);

    ASSERT_TRUE(my_func.f1(2) == 2);
    my_func.f2();
    ASSERT_TRUE(my_func.f3(2,3) == (2+3));
}

TEST(std_shlib_test, UnloadLib)
{
    ASSERT_TRUE (lib_hndl != STD_SHLIB_INVALID_HNDL);
    ASSERT_TRUE(std_shlib_is_loaded(MY_LIB_EXAMPLE));
    t_std_error rc = std_shlib_unload(lib_hndl);
    ASSERT_TRUE (STD_ERR_OK == rc);
    ASSERT_FALSE(std_shlib_is_loaded(MY_LIB_EXAMPLE));
}

TEST(std_shlib_test, LoadLibInvalidSyms)
{
    std_shlib_hndl hndl = STD_SHLIB_INVALID_HNDL;

    int (*f4)(void) = NULL;
    int (*f5)(void) = NULL;

    // Please refer to "std_shlib_test_lib.c" for valid function names and signatures
    std_shlib_func_map_t func_map[] = {
         { "shlib_test_func1", (void **)&my_func.f1},
         { "shlib_test_func2", (void **)&my_func.f2},
         { "shlib_test_func4", (void **)&f4},
         { "shlib_test_func3", (void **)&my_func.f3},
         { "shlib_test_func5", (void **)&f5},
    };
    const size_t func_map_size = sizeof(func_map)/sizeof(std_shlib_func_map_t);

    ASSERT_FALSE(std_shlib_is_loaded(MY_LIB_EXAMPLE));
    t_std_error rc = std_shlib_load(MY_LIB_EXAMPLE, &hndl, func_map, func_map_size);
    t_std_error expected_rc = STD_ERR(COM,FAIL,EFAULT);
    ASSERT_TRUE (expected_rc == rc);

    ASSERT_FALSE(std_shlib_is_loaded(MY_LIB_EXAMPLE));
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

