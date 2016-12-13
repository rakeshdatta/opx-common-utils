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
 * filename: std_ip_utils.h
 */

/*!
 * \file   std_ip_utils.h
 * \brief  Ip address utility functions
 * \date   05-2014
 */

#ifndef __STD_IP_UTILS_H
#define __STD_IP_UTILS_H

#include "std_error_codes.h"
//! TODO move the types from db_common here and redefine them in the db-api to use these
#include "ds_common_types.h"
#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------*\
 *                    Defines and Macros.
\*---------------------------------------------------------------*/
#define STD_IP_IS_AFINDEX_V4(_af_index) (((_af_index) == HAL_INET4_FAMILY))

#define STD_IP_AFINDEX_TO_STR(_af_index)                                      \
        (((_af_index) == HAL_INET4_FAMILY) ? "IPv4" :                         \
         (((_af_index) == HAL_INET6_FAMILY) ? "IPv6" : "Unknown"))

#define STD_IP_AFINDEX_TO_ADDR_LEN(_af_index)                                 \
        (((_af_index) == HAL_INET4_FAMILY) ?  HAL_INET4_LEN : HAL_INET6_LEN)

#define STD_IP_IS_ADDR_ZERO(_p_ip_addr)                                       \
        (((_p_ip_addr)->af_index == HAL_INET4_FAMILY) ?                       \
         (STD_IP_IS_V4_ADDR_ZERO (_p_ip_addr)) :                              \
         (STD_IP_IS_V6_ADDR_ZERO(_p_ip_addr)))

#define STD_IP_IS_V4_ADDR_ZERO(_p_ip_addr)                                    \
        ((((_p_ip_addr)->u.v4_addr) == 0))

#define STD_IP_IS_V6_ADDR_ZERO(_p_ip_addr)                                    \
        std_ip_is_v6_addr_zero(_p_ip_addr)

#define STD_IP_IS_ADDR_LOOP_BACK(_p_ip_addr)                                  \
        (((_p_ip_addr)->af_index == HAL_INET4_FAMILY) ?                       \
         (STD_IP_IS_V4_ADDR_LOOP_BACK (_p_ip_addr)) :                         \
         (STD_IP_IS_V6_ADDR_LOOP_BACK (_p_ip_addr)))

#define STD_IP_IS_V4_ADDR_LOOP_BACK(_p_ip_addr)                               \
        std_ip_is_v4_addr_loopback(_p_ip_addr)

#define STD_IP_IS_V6_ADDR_LOOP_BACK(_p_ip_addr)                               \
        std_ip_is_v6_addr_loopback(_p_ip_addr)

#define STD_IP_IS_ADDR_LINK_LOCAL(_p_ip_addr)                                 \
        (((_p_ip_addr)->af_index == HAL_INET6_FAMILY) ?                       \
         (STD_IP_IS_V6_ADDR_LINK_LOCAL ((_p_ip_addr))) : 0)

#define STD_IP_IS_V6_ADDR_LINK_LOCAL(_p_ip_addr)                              \
        (((((_p_ip_addr)->u.v6_addr [0]) & (0xff)) == (0xfe)) &&              \
         ((((_p_ip_addr)->u.v6_addr [1]) & (0xc0)) == (0x80)))


/*---------------------------------------------------------------*\
 *       Common Utility Function Prototypes
\*---------------------------------------------------------------*/

/*!
 * @brief Converts prefix length to a IPv4 mask
 * @param prefix length
 * @return mask value
 */
uint32_t std_ip_v4_prefix_len_to_mask(unsigned int prefix_len);

/*!
 * @brief Converts prefix length to mask for IPv6 mask
 * @param pointer to hold mask value, mask len and prefix len
 * @return none
 */
void std_ip_v6_prefix_len_to_mask(uint8_t  *mask_ptr, size_t mask_len, unsigned int prefix_len);

/*!
 * @brief Get mask from prefix length for an address family
 * @param address family, prefix length, pointer to mask
 * @return standard error code
 */
t_std_error std_ip_get_mask_from_prefix_len (unsigned int af_index, unsigned int prefix_len,
                                             hal_ip_addr_t *p_out_mask);

/*!
 * @brief Compare IP address
 * @param pointer to ip addresses
 * @return 0 for equal, <0 if address 1 is higher, >0 if address 1 is lower
 */
int std_ip_cmp_ip_addr (const hal_ip_addr_t *p_ip_addr1, const hal_ip_addr_t *p_ip_addr2);

/*!
 * @brief  This routine converts an IP mask to a prefix length
 *         It is assumed that the mask is contiguous.
 * @param  ip mask
 * @return prefix length
 */
int std_ip_v4_mask_to_prefix_len(unsigned int in_mask);

/*!
 * @brief  This routine converts an IPv6 mask to a prefix length
 *         The address length is passed as an argument
 * @param  pointer to mask array, size of pointer
 * @return prefix length
 */
int std_ip_v6_mask_to_prefix_len (const uint8_t *mask_ptr, size_t addr_len);

/*!
 * @brief  This routine checks if the address is an ipv4 loopback
 * @param  address pointer
 * @return true/false
 */
bool std_ip_is_v4_addr_loopback(const hal_ip_addr_t *_p_ip_addr);

/*!
 * @brief  This routine checks if the address is an ipv6 loopback
 * @param  address pointer
 * @return true/false
 */
bool std_ip_is_v6_addr_loopback(const hal_ip_addr_t *_p_ip_addr);

/*!
 * @brief  This routine checks if the address is an ipv6 zero
 * @param  address pointer
 * @return true/false
 */
bool std_ip_is_v6_addr_zero(const hal_ip_addr_t *_p_ip_addr);

/*!
 * @brief  This routine converts the ip address to a human readable string
 * @param  address containing the ip address to convert
 * @param  buffer containing the destination of the string
 * @param  maximum length of the buffer
 * @return a pointer to the string if the conversion worked
 */
const char * std_ip_to_string(const hal_ip_addr_t *_p_ip_addr, char * buff, size_t len);

/**
 * Create a standard IP address format from a in_addr (bigendian)
 * @param ip the ip address to convert to
 * @param addr the address to copy from
 */
void std_ip_from_inet(hal_ip_addr_t *ip,struct in_addr *addr);
/**
 * Create a standard IP address format from a in6_addr (bigendian)
 * @param ip the ip address to convert to
 * @param addr the address to copy from
 */
void std_ip_from_inet6(hal_ip_addr_t *ip,struct in6_addr *addr);

/**
 * Identifies a string as AF_INET or AF_INET6 
 * @param ip_str  string to identify type
 * @param  ip_type inet type
 * @return true on success
 */
bool std_get_ip_type(const char *ip_str, uint32_t *ip_type);

/**
 * Parses string and stores as inet v4 or v6
 * @param ip_str v4/v6 string
 * @param ip_addr_holder structure to store inet v4/v6 value
 * @return true on success
 */
bool std_str_to_ip(const char *ip_str, std_ip_addr_t *ip_addr_holder);

/**
 * Parses string and retrieves inet v4/v6 structure and masklen
 * For example, string 1.1.1.1/14 or 2001:1000:6dcd:8c74:76cc:63bf:ac32:66/100
 * can be input string
 * @param ip_str v4/v6 and mask len string
 * @param ip_prefix_holder structure to store inet v4/v6 value along with mask len
 * @return true on success
 */
bool std_str_to_ip_prefix(const char *ip_prefix_str, std_ip_addr_t *ip_addr_holder, uint8_t *mask_len);






#ifdef __cplusplus
}
#endif

#endif // __STD_IP_UTILS_H
