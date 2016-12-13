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
 * filename: std_mergesort.h
 */

/*!
 * \file   std_mergesort.h
 * \brief   This header file contains header file definitions Merge Sort
 *          library functions
 * \date   05-2014
 */

#ifndef __STD_MERGE_SORT_H__
#define __STD_MERGE_SORT_H__

#include <stdio.h>

/**
 * @brief std_merge_sort_cmp function User defined comparison function
 *
 * @param context Context provided by the application.
 * @param array_a  First Array
 * @param index_a  First Array Index
 * @param array_b  Second Array
 * @param index_b  Second Array Index
 */
typedef int (* std_merge_sort_cmp) (void *context, void *array_a, int index_a,
                                 void *array_b, int index_b);

/**
 * @brief std_merge_sort_cmp function User defined copy function
 *
 * @param context Context provided by the application.
 * @param dstArray  Destinary Array
 * @param dstIndex  Destination Index
 * @param srcArray  Source Array
 * @param srcIndex  Source Index
 */
typedef void (* std_merge_sort_copyfn) (void *context,
                                     void *dstArray, int dstIndex,
                                     void *srcArray, int srcIndex);
/**
 * @brief std_mergesort function implements the Merge Sort algorithm
 *
 * @param context Context provided by the application.
 * @param array  The input array that needs to be sorted.
 * @param numElements Number of elements in the array.
 * @param tmpArray A temporary array which is used by this function internally.
 * @param std_merge_sort_cmp  User defined comparison function.
 * @param std_merge_sort_copyfn  User defined copy function.
 * @return void
 */
void std_merge_sort (void *context, void *array, int numElements,
             void *tmpArray, std_merge_sort_cmp cmp_func,
             std_merge_sort_copyfn copy_func);
#endif /* !__STD_MERGE_SORT_H__ */
