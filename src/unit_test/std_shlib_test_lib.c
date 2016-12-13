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

/**
 * \file std_shlib_test_lib.c
 * \brief
 *
 * \date: Oct 29, 2014
 */

#include <stdio.h>

void __attribute__((constructor)) test_lib_init(void)
{
    printf("%s\n", __FUNCTION__);
}

void __attribute__((destructor)) test_lib_exit(void)
{
    printf("%s\n", __FUNCTION__);
}

int shlib_test_func1(int p)
{
    printf("%s %d\n", __FUNCTION__, p);
    return p;
}

void shlib_test_func2(void)
{
    printf("%s\n", __FUNCTION__);
}

int shlib_test_func3(int p1, int p2)
{
    printf("%s p1=%d p2=%d\n", __FUNCTION__, p1, p2);
    return (p1+p2);
}
