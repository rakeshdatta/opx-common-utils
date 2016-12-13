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
 * filename: std_mergesort.c
 */


/**
 *      @file  std_mergesort.c
 *      @brief  Standard Merge Sort algorithm
 *
 *     @internal
 *     Created  05/11/2014
 *     Company  DELL
 */

#include <stdio.h>
#include <string.h>
#include "std_mergesort.h"

/*******************************************************************************
 * NAME          : std_merge_sort_merge_arrays
 *
 * DESCRIPTION   : This function merges the 2 input arrays.
 * PARAMETERS     : context  - Context provided by the application. The
 *                            library does not use it. It is passed as
 *                            the argument to the compare and copy
 *                            call back functions.
 *
 *                 array    - The input array that needs to be sorted.
 *
 *                 left     - Start index of Array 1.
 *
 *                 mid      - Start index of Array 2.
 *
 *                 right    - End index of Array 2.
 *
 *                 tmp_array - A temporary array which is used by this
 *                            function internally. It should be of the
 *                            same type as 'array'
 *
 *                 cmp_func    - The User defined comparison function.
 *                            For ascending sorted order, it should return,
 *                              < 0, if element 1 is lesser than element 2
 *                              > 0, if element 1 is greater than element 2
 *                              = 0, otherwise.
 *                            For descending sorted order, it should return,
 *                              > 0, if element 1 is lesser than element 2
 *                              < 0, if element 1 is greater than element 2
 *                              = 0, otherwise.
 *
 *                 copy_func   - The User defined copy function.
 *
 * RETURN VALUES : None
 ***************************************************************************/
static void std_merge_sort_merge_arrays (void *context, void *array,
                            int left, int mid, int right,
                            void *tmp_array, std_merge_sort_cmp cmp_func,
                            std_merge_sort_copyfn copy_func)
{
    int left_end;
    int curr_pos;
    int index;
    int num_elements;

    left_end = mid - 1;
    curr_pos = left;
    num_elements = right - left + 1;

    while ((left <= left_end) && (mid <= right))
    {
        if (cmp_func (context, array, left, array, mid) <= 0)
        {
            copy_func (context, tmp_array, curr_pos, array, left);
            left = left + 1;
        }
        else
        {
            copy_func (context, tmp_array, curr_pos, array, mid);
            mid = mid + 1;
        }

        curr_pos++;
    }

    while (left <= left_end)
    {
        copy_func (context, tmp_array, curr_pos, array, left);
        curr_pos++;
        left++;
    }

    while (mid <= right)
    {
        copy_func (context, tmp_array, curr_pos, array, mid);
        curr_pos++;
        mid++;
    }

    for (index = 0; index < num_elements; index++)
    {
        /* Copy the elements from the work array to the actual array */
        copy_func (context, array, right, tmp_array, right);
        right = right - 1;
    }
}

/*******************************************************************************
 * NAME          : std_merge_sort_divide_n_sort
 *
 * DESCRIPTION   : This function divides the input array into 2 arrays
 *                 recursively until, the arrays contain just one element.
 *
 *                 The 2 divided arrays are then merged to get the sorted
 *                 array.
 *
 *                 This is an internal function.
 *
 * ARGUMENTS     : context  - Context provided by the application. The
 *                            library does not use it. It is passed as
 *                            the argument to the compare and copy
 *                            call back functions.
 *
 *                 array    - The input array that needs to be sorted.
 *
 *                 left     - Start index of the input array.
 *
 *                 right    - End index of the input array.
 *
 *                 tmp_array - A temporary array which is used by this
 *                            function internally. It should be of the
 *                            same type as 'array'
 *
 *                 cmp_func    - The User defined comparison function.
 *                            For ascending sorted order, it should return,
 *                              < 0, if element 1 is lesser than element 2
 *                              > 0, if element 1 is greater than element 2
 *                              = 0, otherwise.
 *                            For descending sorted order, it should return,
 *                              > 0, if element 1 is lesser than element 2
 *                              < 0, if element 1 is greater than element 2
 *                              = 0, otherwise.
 *
 *                 copy_func   - The User defined copy function.
 *
 * RETURN VALUES : None
 ***************************************************************************/
static void std_merge_sort_divide_n_sort (void *context, void *array,
                              int left, int right, void *tmp_array,
                              std_merge_sort_cmp cmp_func,
                  std_merge_sort_copyfn copy_func)
{
    int mid;

    if (left < right)
    {
        mid = (right + left) / 2;

        std_merge_sort_divide_n_sort (context, array,
                          left, mid, tmp_array, cmp_func, copy_func);
        std_merge_sort_divide_n_sort (context, array,
                          mid + 1, right, tmp_array, cmp_func, copy_func);

        std_merge_sort_merge_arrays (context, array, left, mid + 1, right,
                     tmp_array, cmp_func, copy_func);
    }
}

/*******************************************************************************
 * NAME          : std_merge_sort
 *
 * DESCRIPTION   : This function implements the Merge Sort algorithm
 *
 * ARGUMENTS     : context     - Context provided by the application. The
 *                               library does not use it. It is passed as
 *                               the argument to the compare and copy
 *                               call back functions.
 *
 *                 array       - The input array that needs to be sorted.
 *
 *                 num_elements - Number of elements in the array.
 *
 *                 tmp_array    - A temporary array which is used by this
 *                               function internally. It should be of the
 *                               same type as 'array'
 *
 *                 cmp_func       - The User defined comparison function.
 *                               For ascending sorted order, it should return,
 *                                 < 0, if element 1 is lesser than element 2
 *                                 > 0, if element 1 is greater than element 2
 *                                 = 0, otherwise.
 *                               For descending sorted order, it should return,
 *                                 > 0, if element 1 is lesser than element 2
 *                                 < 0, if element 1 is greater than element 2
 *                                 = 0, otherwise.
 *
 *                 copy_func      - The User defined copy function.
 *
 * RETURN VALUES : None
 ******************************************************************************/
void std_merge_sort(void *context, void *array, int num_elements,
            void *tmp_array, std_merge_sort_cmp cmp_func,
             std_merge_sort_copyfn copy_func)
{
    std_merge_sort_divide_n_sort (context,
                      array, 0, num_elements-1, tmp_array, cmp_func, copy_func);
}
