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
 * filename: std_struct_utils.h
 */


#ifndef __STD_STRUCT_UTILS_H
#define __STD_STRUCT_UTILS_H

#include <stddef.h>
/**
 * @brief This will return the offset of the field in the structure (size_t return)
 */
#define STD_STR_OFFSET_OF(type, field) \
    offsetof(type,field)

/**
 * @brief this will return the field size of a element in a structure. size_t returned
 */
#define STD_STR_SIZE_OF(type, field) \
    (  sizeof(( (type*)0) -> field ) )

#endif
