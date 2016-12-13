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
 * std_mac_utils.h
 */

#ifndef STD_MAC_UTILS_H_
#define STD_MAC_UTILS_H_

#include "ds_common_types.h"


#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Print out the MAC address to the string buffer passed in
 * @param mac the mac address to print
 * @param buff the size of the character string
 * @param len the length of the buffer
 * @return the pointer to the string or NULL if error
 */
const char * std_mac_to_string(const hal_mac_addr_t *mac, char *buff, size_t len);

/**
 * Convert MAC address string to hal_mac_addr_t structure
 * @param mac the mac address structure to return
 * @param buff the mac address string
 * @param len the length of the buffer
 * @return true if success
 */
bool std_string_to_mac(hal_mac_addr_t *mac, const char *buff, size_t len);

/**
 * Check if MAC address is unicast or no
 * @param mac the mac address to check
 * @return true if success
 */
bool std_mac_isunicast(const hal_mac_addr_t *mac);
#ifdef __cplusplus
}
#endif

#endif /* STD_MAC_UTILS_H_ */
