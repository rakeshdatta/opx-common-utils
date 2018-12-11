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
 * std_mac_utils.c
 */

#include "std_mac_utils.h"

#include <stdio.h>
#include "std_utils.h"

const char * std_mac_to_string(const hal_mac_addr_t *mac, char *buff, size_t len) {
    snprintf(buff,len,"%02x:%02x:%02x:%02x:%02x:%02x",
               ((*mac)[0]),((*mac)[1]),((*mac)[2]),
               ((*mac)[3]),((*mac)[4]),((*mac)[5]));

    return buff;
}

bool std_string_to_mac(hal_mac_addr_t *mac, const char *buff, size_t len)
{
    uint32_t tmp[HAL_MAC_ADDR_LEN];
    uint32_t ix=0;

    sscanf(buff, "%x:%x:%x:%x:%x:%x%*c",
                &tmp[0], &tmp[1], &tmp[2], &tmp[3], &tmp[4], &tmp[5]);

    for (; ix < HAL_MAC_ADDR_LEN; ++ix)
        (*mac)[ix] = (uint8_t) tmp[ix];

    return true;
}

/* Return true if unicast MAC address
 * return false if multicast/broadcast MAC address
 */
bool std_mac_isunicast(const hal_mac_addr_t *mac)
{
    return (((*mac)[0] & 0x01) != 0x01);
}
