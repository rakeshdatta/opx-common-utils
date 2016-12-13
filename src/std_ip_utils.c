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
 * filename: std_ip_utils.c
 */

/*!
 * \file   std_ip_utils.c
 * \brief  Ip address utility functions
 * \date   06-2014
 * \author Prince Sunny/Satish Mynam
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "std_utils.h"
#include "std_ip_utils.h"
#include "event_log.h"

#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>

#define NBBY 8

uint32_t std_ip_v4_prefix_len_to_mask(unsigned int prefix_len)
{
    uint32_t mask_val = 0;

    if (prefix_len > (NBBY * sizeof(uint32_t))) {
        return 0;
    }

    mask_val = ((prefix_len)?((1 << ((NBBY * sizeof(uint32_t) - prefix_len))) -1):0xffffffff);

    return (uint32_t)~(mask_val);
}

void std_ip_v6_prefix_len_to_mask(uint8_t *mask_ptr, size_t mask_len, unsigned int prefix_len)
{
    unsigned nbytes;
    static uint8_t mask_val[8] = { 0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, };

    if ((!mask_ptr) || (!prefix_len)) {
        return;
    }

    nbytes = prefix_len / NBBY;
    memset(mask_ptr, 0xff, nbytes);
    mask_ptr += nbytes;
    mask_len -= nbytes;

    if (!mask_len) {
        return;
    }

    *mask_ptr++ = mask_val[prefix_len % NBBY];
    --mask_len;
    memset(mask_ptr, 0, mask_len);
}

t_std_error std_ip_get_mask_from_prefix_len (unsigned int af_index, unsigned int prefix_len, hal_ip_addr_t *p_out_mask)
{
    if (!p_out_mask) {
        EV_LOG_ERR(ev_log_t_ROUTE, 3, "STD_IP" , "%s (): Invalid input param.\
           p_out_mask: %p", __FUNCTION__, p_out_mask);
        return (STD_ERR_MK(e_std_err_ROUTE, e_std_err_code_FAIL, 0));
    }

    memset (p_out_mask, 0, sizeof (hal_ip_addr_t));
    p_out_mask->af_index = af_index;

    if (STD_IP_IS_AFINDEX_V4 (af_index)) {
        p_out_mask->u.v4_addr = std_ip_v4_prefix_len_to_mask (prefix_len);
    }
    else {
        std_ip_v6_prefix_len_to_mask ((uint8_t *)&p_out_mask->u.v6_addr, HAL_INET6_LEN,
                            prefix_len);
    }

    return STD_ERR_OK;
}

int std_ip_cmp_ip_addr (const hal_ip_addr_t *p_ip_addr1, const hal_ip_addr_t *p_ip_addr2)
{
    int rc;

    if (p_ip_addr1->af_index != p_ip_addr2->af_index) {
        return (p_ip_addr1->af_index - p_ip_addr2->af_index);
    }

    rc = memcmp (p_ip_addr1->u.v6_addr, p_ip_addr2->u.v6_addr,
                 STD_IP_AFINDEX_TO_ADDR_LEN (p_ip_addr1->af_index));

    return rc;
}

int std_ip_v4_mask_to_prefix_len(unsigned int in_mask)
{
//! TODO move to use the find bit API from the bitmask api
    int      ix;
    int      prefix_len = 0;
    uint8_t *mask_ptr;
    static uint8_t bit_pos[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 6, 0, 7, 8,
    };

    in_mask = htonl(in_mask);
    mask_ptr = (uint8_t *)&in_mask;
    if (!in_mask) {
        return 0;
    }

    for (ix = 0; ix < sizeof(uint32_t); ++ix) {
        prefix_len += bit_pos[mask_ptr[ix]];
    }

    return prefix_len;
}

int std_ip_v6_mask_to_prefix_len (const uint8_t *mask_ptr, size_t addr_len)
{
//! TODO move to use the find bit API from the bitmask api
    static uint8_t bit_pos[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 6, 0, 7, 8,
    };
    size_t ix;
    int prefix_len = 0;

    if (!mask_ptr) {
        return 0;
    }

    for (ix = 0; ix < addr_len; ++ix) {
        prefix_len += bit_pos[(int)mask_ptr[ix]];
    }

    return prefix_len;
}

bool std_ip_is_v4_addr_loopback(const hal_ip_addr_t *_p_ip_addr)
{
    static uint32_t  g_ip_v4_loopback_addr = 0x7f000001;
    return(((_p_ip_addr)->u.v4_addr) == g_ip_v4_loopback_addr);
}

bool std_ip_is_v6_addr_loopback(const hal_ip_addr_t *_p_ip_addr)
{
    static uint8_t g_ip_v6_loopback_addr [HAL_INET6_LEN] =
                   { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };
    return(memcmp (&((_p_ip_addr)->u.v6_addr), &g_ip_v6_loopback_addr, HAL_INET6_LEN) == 0);
}

bool std_ip_is_v6_addr_zero(const hal_ip_addr_t *_p_ip_addr)
{
    static uint8_t g_ip_v6_zero_addr [HAL_INET6_LEN] =
                   { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    return(memcmp (&((_p_ip_addr)->u.v6_addr), &g_ip_v6_zero_addr, HAL_INET6_LEN) == 0);
}

const char * std_ip_to_string(const hal_ip_addr_t *_p_ip_addr, char * buff, size_t len) {


    if (_p_ip_addr->af_index == AF_INET ) {
        if (len< INET_ADDRSTRLEN ) return NULL;
        struct in_addr v4;
        v4.s_addr = _p_ip_addr->u.v4_addr;
        return inet_ntop(_p_ip_addr->af_index,&v4,buff,len);

    }
    if (_p_ip_addr->af_index == AF_INET6 ) {
        if (len< INET6_ADDRSTRLEN ) return NULL;
        struct in6_addr v6;
        memcpy(v6.__in6_u.__u6_addr8, _p_ip_addr->u.v6_addr,
                sizeof(v6.__in6_u.__u6_addr8));
        return inet_ntop(_p_ip_addr->af_index,&v6,buff,len);
    }
    return NULL;
}

void std_ip_from_inet(hal_ip_addr_t *ip,struct in_addr *addr)
{
    memset (ip, 0, sizeof (hal_ip_addr_t));
    ip->af_index = AF_INET;
    ip->u.v4_addr = addr->s_addr;
}

void std_ip_from_inet6(hal_ip_addr_t *ip,struct in6_addr *addr)
{
    memset (ip, 0, sizeof (hal_ip_addr_t));
    ip->af_index = AF_INET6;
    memcpy(ip->u.v6_addr,addr->__in6_u.__u6_addr8,sizeof(ip->u.v6_addr));
}

bool std_get_ip_type(const char *ip_str, uint32_t *ip_type)
{
    struct addrinfo hints, *addr_ptr = NULL;
    bool retval = false;
    int rc;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = 0;
    hints.ai_protocol = 0;
    /*  We provide only IPv4/IPv6 address string.
     *   No need for name resolution
     */
    hints.ai_flags = AI_NUMERICHOST;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    rc = getaddrinfo(ip_str, NULL, &hints, &addr_ptr);
    if (rc == 0) {
        if (addr_ptr->ai_family == AF_INET) {
           *ip_type = AF_INET;
        } else if (addr_ptr->ai_family == AF_INET6) {
           *ip_type = AF_INET6;
        }
        retval = true;
    }  else {
           EV_LOGGING(COM,DEBUG,"IP_UTILS",
              "Couldn't get ip type for ip union string");
    }
    /* Free the memory before returning */
    freeaddrinfo(addr_ptr);
    return retval;
}

bool std_str_to_ip(const char *ip_str, std_ip_addr_t *ip_addr_holder)
{
    bool retval = false;
    uint32_t ip_type;
    if (!ip_str || !ip_addr_holder) {
        return false;
    }
    if (std_get_ip_type(ip_str, &ip_type) == true) {
        ip_addr_holder->af_index = ip_type;
        switch (ip_type)  {
            case AF_INET:
                             if (inet_pton(AF_INET, ip_str, &ip_addr_holder->u.ipv4) > 0 ) {
                                 retval = true;
                             }
                             break;
            case AF_INET6:
                             if (inet_pton(AF_INET6, ip_str, &ip_addr_holder->u.ipv6) > 0 ) {
                                 retval = true;
                             }
                             break;
            default : break;
        }
    }
    return retval;
}

/**  Caller has to call the free function for this case
*/
static bool std_splice_prefix_string(const char *ip_prefix_str, const char **ip_part,
                                   const char **len_part, std_parsed_string_t *handle)
{
    unsigned int num_tokens;
    bool retval = false;
    size_t index = 0;
    if (!ip_prefix_str ||  !ip_part || !len_part  || !handle) {
         return false;
    }
    do {
        if(!(std_parse_string(handle, ip_prefix_str, "/")) || !*handle) {
           EV_LOGGING(COM,DEBUG,"IP_UTILS",
              "parsing ip prefix string %s failed : %s %d\n",  ip_prefix_str);
           break;
        }
        num_tokens = std_parse_string_num_tokens(*handle);
        /* There is an ip address and prefix length, so 2 tokens */
        if (num_tokens !=  2) {
           EV_LOGGING(COM,DEBUG,"IP_UTILS",
                "parsing ip prefix string %s failed: %s %d\n", ip_prefix_str);
           break;
        }
        *ip_part = std_parse_string_next(*handle, &index);
        *len_part = std_parse_string_next(*handle, &index);
        if (!*ip_part || !*len_part) {
           break;
        }
        retval = true;
    } while (0);
    return retval;
}

bool std_str_to_ip_prefix(const char *ip_prefix_str, std_ip_addr_t *ip_addr_holder, uint8_t *mask_len  )
{
    bool retval = false;
    std_parsed_string_t handle = NULL;
    const char *ip_part, *len_part;
    if (!ip_prefix_str || !ip_addr_holder) {
         return NULL;
    }
    do {
        if (std_splice_prefix_string(ip_prefix_str, &ip_part, &len_part, &handle) == false) {
            break;
        }
        if (std_str_to_ip(ip_part, ip_addr_holder) == false ) {
            break;
        }
        *mask_len = (uint8_t) strtoul(len_part, NULL, 0);
        retval = true;
    } while (0);
    std_parse_string_free(handle);
    return retval;
}
