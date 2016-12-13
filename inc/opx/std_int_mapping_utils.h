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
 * std_int_mapping_utils.h
 */

#ifndef STD_INT_MAPPING_UTILS_H_
#define STD_INT_MAPPING_UTILS_H_

#include <stddef.h>

/**
 * This API essentially takes a list of array of ints as an input, locates a value "from" in a column
 * "from_field" and returns the "return_field" value.
@verbatim

int mappings[][3] = {
    { ENUMA_1, ENUMB_1, ENUMC_1 },
    { ENUMA_2, ENUMB_2, ENUMC_2 },
    { ENUMA_3, ENUMB_3, ENUMC_3 },
};

int enuma = std_int_translate(ENUMB_2, 1,0,mappings,sizeof(mappings)/sizeof(mappings[0]));
if (enuma==-1) //not found

int enumc = std_int_translate(ENUMB_2, 1,2,mappings,sizeof(mappings)/sizeof(mappings[0]));
if (enumc==-1) //not found

Or...
int mappings[][2] = {
    { ENUMA_1, ENUMB_1},
    { ENUMA_2, ENUMB_2},
    { ENUMA_3, ENUMB_3},
};

int enumb = std_int_translate(ENUMA_2, 0,1,mappings,sizeof(mappings)/sizeof(mappings[0]));
if (enumb==-1) //not found

@endverbatim

 * @param from is the enum that you want to translate from
 * @param from_field is the array index (0->max) that contains the from enum list
 * @param return_field is the corresponding array index containing the mapped value
 * @param array_map the list of array of integers containing the mapping
 * @param int_array_map_len is the length of the array list
 * @return -1 if not found otherwise the located value
 */
int std_int_translate( int from, int from_field, int return_field, int ** array_map , size_t int_array_map_len);

#endif /* STD_INT_MAPPING_UTILS_H_ */
