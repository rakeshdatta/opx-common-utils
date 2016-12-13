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
 * filename: test_common.c
 */

#include "std_error_codes.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


int main() {

    printf(" Error sub pos: %X, mask: %X\n"
            "    errid pos: %X, mask: %X\n"
            "     priv pos: %X, mask: %X\n"
            "Example... %X,%X,%X = %X\n",
            STD_MK_ERR_SUB_POS,STD_MK_ERR_SUB_MASK,
            STD_MK_ERR_ERRID_POS,STD_MK_ERR_ERRID_MASK,
            STD_MK_ERR_PRIV_POS,STD_MK_ERR_PRIV_MASK,
            3,2,1,STD_ERR_MK(3,2,1));

    return 0;
}


