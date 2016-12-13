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
 * filename: std_radix.c
 */

/*!
 * \file   std_radix.c
 * \brief  Radix tree implementation. This implementation of Radix tree is adapted
 *         from the GateD code (rt_radix.c)
 * \date   05-2014
 * \author Prince Sunny
 */

/*---------------------------------------------------------------*\
 *                    Includes.
\*---------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "std_radix.h"
#include "std_radical.h"
#include "std_llist.h"

/*---------------------------------------------------------------*\
 *                    Defines and Macros.
\*---------------------------------------------------------------*/

/// Define enable/disable Radix tree tester.
#define TEST_RADIX    1
#define RDX_DEBUG    1

#define RDX_MALLOC    malloc
#define RDX_FREE    free

#ifndef ERROR
#define ERROR           (-1)     /* Error return value */
#endif

#define RDX_MAGIC    0xdeadbeef
#define RDX_INITIALVER    0

#define RDX_WALKDOWN    1
#define RDX_WALKUP    2
#define RDX_WALKRIGHT    3
#define DIVISOR         8

#define RDX_ASSERT(x)    assert(x)

#define RDX_VALIDATE_HANDLE(rtt) \
            RDX_ASSERT(rtt); \
            RDX_ASSERT(rtt->rtt_magic == RDX_MAGIC);

#define RDX_DEBUG_START(rtt) \
            if (RDX_DEBUG) { \
                RDX_VALIDATE_HANDLE(rtt); \
                if (rtt->rtt_debug == TRUE) { \

#define RDX_DEBUG_END      } }


#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif
#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE    0
#endif

/*
 * XXX Assumes NBBY is 8.  If it isn't we're in trouble anyway.
 */
#define RNBBY   8
#define RNSHIFT 3
#define RNBYTE(x)       ((x) >> RNSHIFT)
#define RNBIT(x)        (0x80 >> ((x) & (RNBBY-1)))
#define BIT_TEST(f, b) ((f) & (b))

#define MAXKEYBITS      (SOCK_MAX_ADDRESS_LEN * RNBBY)
#define MAXDEPTH        255

#define RN_SETBIT(rtn, bitlen) \
    do { \
        (rtn)->rtn_bit = (bitlen); \
        (rtn)->rtn_tbit = RNBIT((bitlen)); \
    } while (0)

#define RN_LOCK(rtn)    ((rtn)->rtn_lock++)
#define RN_UNLOCK(rtn)  ((rtn)->rtn_lock--)
#define RN_IFLOCK(rtn)  ((rtn)->rtn_lock)

#define DEBUG_USRLIB 0
/*---------------------------------------------------------------*\
 *                    Global variables.
\*---------------------------------------------------------------*/

/* Return the bit position of the msb */
const u_char first_bit_set[256] = {
    /* 0 - 15 */
    8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
    /* 16 - 31 */
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    /* 32 - 63 */
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    /* 64 - 127 */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    /* 128 - 255 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

/*---------------------------------------------------------------*\
 *            Private methods
\*---------------------------------------------------------------*/

static void * std_radix_malloc(size_t size)
{
    return malloc(size);
}

static void std_radix_free(void *ptr)
{
    free(ptr);
    return;
}

static inline int rdx_compare_address(u_char *ap1, u_char *ap2, u_short tbyte, u_char tbit)
{
    u_char mask;

    mask = (u_char)~(tbit | (tbit - 1));

    if ((ap1[tbyte] ^ ap2[tbyte]) & mask)
        return 1;
    if (tbyte && memcmp(ap1, ap2, tbyte))
        return 1;

    return 0;
}


static char * std_radix_printaddr(u_char *addr, int bitlen)
{
    int i, offset;
    static char tmpstr[256];

    memset(tmpstr, '\0', sizeof(tmpstr));

    if (addr == NULL) {
        return tmpstr;
    }

    for (i = 0, offset = 0; i < ((bitlen/NBBY)-((bitlen%NBBY)==0?1:0)); i++)
        offset += snprintf(&tmpstr[offset],sizeof(tmpstr)-offset, "%d.", (u_int)addr[i]);
    snprintf(&tmpstr[offset],sizeof(tmpstr)-offset, "%d", (u_int)addr[i]);

    return tmpstr;
}

/*---------------------------------------------------------------*\
 *            Public methods
\*---------------------------------------------------------------*/

int std_radix_nodeisleaf(std_rt_head *rth)
{
    rt_node *rtn;

    if (!rth) {
    return ERROR;
    }

    rtn = rth->rth_rtn;
    if ((rtn->rtn_left) || (rtn->rtn_right)) {
    return 0;
    }

    return 1;
}

std_rt_head * std_radix_getparent(std_rt_table *rtt, std_rt_head *rth)
{
    rt_node *rtn;

    RDX_DEBUG_START(rtt);
    RDX_ASSERT(rth);
    RDX_DEBUG_END;

    /* Get the first parent node w/ external head */
    for (rtn = rth->rth_rtn->rtn_parent; rtn; rtn = rtn->rtn_parent) {
        if (rtn->rtn_rth)
            break;
    }

    /* Return external node */
    if (rtn)
        return rtn->rtn_rth;
    else
        return (std_rt_head *)0;
}

std_rt_head * std_radix_getbest(std_rt_table *rtt, u_char *addr, ushort bitlen)
{
    rt_node *rtn;
    u_char *ap;

    if (NULL == addr)
         return (std_rt_head *)0;
#if _BYTE_ORDER == _LITTLE_ENDIAN
    if (rtt->rtt_convert) {
        memset (rtt->temp, '\0', ((rtt->rtt_maxaddrlen + (DIVISOR-1))/DIVISOR));
        rtt->rtt_convert(addr, (char *) rtt->temp,rtt->rtt_maxaddrlen);
        addr = rtt->temp;
    }
#endif

    RDX_DEBUG_START(rtt);

    /*
     * Check if the given address length is valid.
     */
    if (bitlen > rtt->rtt_maxaddrlen)
        return (std_rt_head *)0;

    RDX_DEBUG_END;

    /*
     * If there is no table, or nothing to do, assume nothing found.
     */
    if (!(rtn = rtt->rtt_root))
        return (std_rt_head *)0;

    /*
     * Search down the tree until we find a node which
     * has a bit number the same as ours.
     */
    ap = addr;
    while (rtn && rtn->rtn_bit < bitlen) {
        if (BIT_TEST(ap[RNBYTE(rtn->rtn_bit)], rtn->rtn_tbit)) {
            if (!(rtn->rtn_right)) {
                break;
            }
            rtn = rtn->rtn_right;
        } else {
            if (!(rtn->rtn_left)) {
                break;
            }
            rtn = rtn->rtn_left;
        }
    }

    /*
     * Now backtrack towards the root to find the first
     * match against the given address.
     */
    for (; rtn; rtn = rtn->rtn_parent) {
        if (rtn->rtn_bit > bitlen)
            continue;
        if (!rtn->rtn_rth)
            continue;

        if (!rdx_compare_address(addr, rtn->rtn_rth->rdx_rth_addr,
            RNBYTE(rtn->rtn_bit), rtn->rtn_tbit))
            break;
    }

    if (rtn && !RDX_TEST_BIT(rtn->rtn_flags, RDX_RN_DELE_BIT))
        return rtn->rtn_rth;
    else
        return (std_rt_head *)0;

} // std_radix_getbest()

std_rt_head * std_radix_getbestandprev(std_rt_table *rtt, u_char *addr, ushort bitlen, std_rt_head **lessbest)
{
    rt_node *rtn;
    std_rt_head *best;
    u_char *ap;

    if (NULL == addr)
        return (std_rt_head *)0;

#if _BYTE_ORDER == _LITTLE_ENDIAN
    if (rtt->rtt_convert) {
        memset (rtt->temp, '\0', ((rtt->rtt_maxaddrlen + (DIVISOR-1))/DIVISOR));
        rtt->rtt_convert(addr, (char *)rtt->temp,rtt->rtt_maxaddrlen);
        addr = rtt->temp;
    }
#endif

    RDX_DEBUG_START(rtt);

    /*
     * Check if the given address length is valid.
     */
    if (bitlen > rtt->rtt_maxaddrlen)
        return (std_rt_head *)0;

    RDX_DEBUG_END;

    /*
     * If there is no table, or nothing to do, assume nothing found.
     */
    if (!(rtn = rtt->rtt_root))
        return (std_rt_head *)0;

    /*
     * Search down the tree until we find a node which
     * has a bit number the same as ours.
     */
    ap = addr;
    while (rtn && rtn->rtn_bit < bitlen) {
        if (BIT_TEST(ap[RNBYTE(rtn->rtn_bit)], rtn->rtn_tbit)) {
            if (!(rtn->rtn_right)) {
                break;
            }
            rtn = rtn->rtn_right;
        } else {
            if (!(rtn->rtn_left)) {
                break;
            }
            rtn = rtn->rtn_left;
        }
    }

    /*
     * Now backtrack towards the root to find the first
     * match against the given address.
     */
    for (; rtn; rtn = rtn->rtn_parent) {
        if (rtn->rtn_bit > bitlen)
            continue;
        if (!rtn->rtn_rth)
            continue;

        if (!rdx_compare_address(addr, rtn->rtn_rth->rdx_rth_addr,
            RNBYTE(rtn->rtn_bit), rtn->rtn_tbit))
            break;
    }

    if (rtn && !RDX_TEST_BIT(rtn->rtn_flags, RDX_RN_DELE_BIT))
        {
            /* Save the best node */
            best = rtn->rtn_rth;

            /* Now search for second best */
             rtn = rtn->rtn_parent;
            for (; rtn; rtn = rtn->rtn_parent) {
                if (rtn->rtn_bit > bitlen)
                    continue;
                if (!rtn->rtn_rth)
                    continue;

                if (!rdx_compare_address(addr, rtn->rtn_rth->rdx_rth_addr,
                    RNBYTE(rtn->rtn_bit), rtn->rtn_tbit))
                    break;
            }


            if (rtn && !RDX_TEST_BIT(rtn->rtn_flags, RDX_RN_DELE_BIT)) {
                    if( lessbest )
                    *lessbest = rtn->rtn_rth;
                    return best;
            } else {
                if( lessbest )
                *lessbest = (std_rt_head*)0;
                return best;
            }

      }
    else
      {
        if( lessbest )
        *lessbest = (std_rt_head*)0;
        return (std_rt_head *)0;
      }

} // std_radix_getbestandprev()

std_rt_head * std_radix_getnextbest(std_rt_table *rtt, u_char *addr, ushort bitlen)
{
    rt_node *rtn;
    u_char *ap;

    if (NULL == addr)
        return (std_rt_head *)0;

#if _BYTE_ORDER == _LITTLE_ENDIAN
    if (rtt->rtt_convert) {
        memset (rtt->temp, '\0', ((rtt->rtt_maxaddrlen + (DIVISOR-1))/DIVISOR));
        rtt->rtt_convert(addr, (char *) rtt->temp,rtt->rtt_maxaddrlen);
        addr = rtt->temp;
    }
#endif

    RDX_DEBUG_START(rtt);

    /*
     * Check if the given address length is valid.
     */
    if (bitlen > rtt->rtt_maxaddrlen)
        return (std_rt_head *)0;

    RDX_DEBUG_END;

    /*
     * If there is no table, or nothing to do, assume nothing found.
     */
    if (!(rtn = rtt->rtt_root))
        return (std_rt_head *)0;

    /*
     * Search down the tree until we find a node which
     * has a bit number the same as ours.
     */
    ap = addr;
    while (rtn && rtn->rtn_bit < bitlen) {
        if (BIT_TEST(ap[RNBYTE(rtn->rtn_bit)], rtn->rtn_tbit)) {
            if (!(rtn->rtn_right)) {
                break;
            }
            rtn = rtn->rtn_right;
        } else {
            if (!(rtn->rtn_left)) {
                break;
            }
            rtn = rtn->rtn_left;
        }
    }
    /*
     * Now backtrack towards the root to find the first
     * match against the given address.
     */
    for (; rtn; rtn = rtn->rtn_parent) {
        if (rtn->rtn_bit > bitlen)
            continue;
        if (!rtn->rtn_rth)
            continue;

        if (!rdx_compare_address(addr, rtn->rtn_rth->rdx_rth_addr,
            RNBYTE(rtn->rtn_bit), rtn->rtn_tbit))
            break;
    }

    if (rtn && !RDX_TEST_BIT(rtn->rtn_flags, RDX_RN_DELE_BIT))
        {
             rtn = rtn->rtn_parent;
            for (; rtn; rtn = rtn->rtn_parent) {
                if (rtn->rtn_bit > bitlen)
                    continue;
                if (!rtn->rtn_rth)
                    continue;

                if (!rdx_compare_address(addr, rtn->rtn_rth->rdx_rth_addr,
                    RNBYTE(rtn->rtn_bit), rtn->rtn_tbit))
                    break;
            }


            if (rtn && !RDX_TEST_BIT(rtn->rtn_flags, RDX_RN_DELE_BIT))
                return rtn->rtn_rth;
            else
                return (std_rt_head *)0;

        }
    else
        return (std_rt_head *)0;

} // std_radix_getnextbest()

rt_node * _std_radix_getsubtree(std_rt_table *rtt, u_char *addr, ushort bitlen)
{
    std_rt_head *rth;
    rt_node *rtn;

    RDX_DEBUG_START(rtt);

    /*
     * Check if the given address length is valid.
     */
    if ((bitlen > rtt->rtt_maxaddrlen) || (NULL == addr))
        return (rt_node *)0;

    RDX_DEBUG_END;

    rtn = (rt_node *)0;

    /*
     * If there is no table, or nothing to do, assume nothing found.
     */
    if (!rtt->rtt_root)
        return (rt_node *)0;

    /*
     * Check if we have an exact node for the root of subtree.
     */
    if ((rth = std_radix_getexact(rtt, addr, bitlen))) {
        rtn = rth->rth_rtn;
    } else {
        /*
         * If no such luck then check the next node to the
         * root we are seeking.
         */
        if ((rth = std_radix_getnext(rtt, addr, bitlen))) {
            /*
             * Now validate that this node falls within the
             * subtree we are seeking.
             */

#if _BYTE_ORDER == _LITTLE_ENDIAN
            if (rtt->rtt_convert) {
                memset (rtt->temp, '\0', ((rtt->rtt_maxaddrlen + (DIVISOR-1))/DIVISOR));
                rtt->rtt_convert(addr, (char *)rtt->temp,rtt->rtt_maxaddrlen);
                addr = rtt->temp;
            }
#endif
            if (rdx_compare_address(addr, rth->rdx_rth_addr, RNBYTE(bitlen), RNBIT(bitlen))) {
                return (rt_node *)0;
            }

            /*
             * Consider the node from get-next to be the root
             * unless we find a parent node to be the root.
             */
            rtn = rth->rth_rtn;

            /*
             * Check if any of the parent could be root by
             * checking the bitlen to be with the subtree bitlen.
             */
            while (rtn->rtn_parent && rtn->rtn_parent->rtn_bit >= bitlen)
                rtn = rtn->rtn_parent;
        }
    }

    return rtn;
} // _std_radix_getsubtree()

std_rt_head * std_radix_getexact(std_rt_table *rtt, u_char *addr, ushort bitlen)
{
    rt_node *rtn;
    std_rt_head *rth;
    u_char *ap;

    RDX_DEBUG_START(rtt);
    if (NULL == addr)
        return (std_rt_head *)0;

#if _BYTE_ORDER == _LITTLE_ENDIAN
    if (rtt->rtt_convert && addr) {
        memset (rtt->temp, '\0', ((rtt->rtt_maxaddrlen + (DIVISOR-1))/DIVISOR));
        rtt->rtt_convert(addr, (char *)rtt->temp,rtt->rtt_maxaddrlen);
        addr = rtt->temp;
    }
#endif

    /*
     * Check if the given address length is valid.
     */
    if (bitlen > rtt->rtt_maxaddrlen)
        return (std_rt_head *)0;

    RDX_DEBUG_END;

    /*
     * If there is no table, or nothing to do, assume nothing found.
     */
    if (!(rtn = rtt->rtt_root))
        return (std_rt_head *)0;

    /*
     * Search down the tree until we find a node which
     * has a bit number the same as ours.
     */
    ap = addr;
    while (rtn && rtn->rtn_bit < bitlen) {
        if (BIT_TEST(ap[RNBYTE(rtn->rtn_bit)], rtn->rtn_tbit)) {
            if (!(rtn->rtn_right)) {
                break;
            }
            rtn = rtn->rtn_right;
        } else {
            if (!(rtn->rtn_left)) {
                break;
            }
            rtn = rtn->rtn_left;
        }
    }

    /*
     * If we didn't find an exact bit length match, we're gone.
     * If there is no rth on this node, we're gone too.
     */
    if (!rtn || rtn->rtn_bit != bitlen || !(rth = rtn->rtn_rth))
        return (std_rt_head *)0;

    /*
     * So far so good.  Fetch the address and see if we have an
     * exact match.
     */
    if (rdx_compare_address(ap, rtn->rtn_rth->rdx_rth_addr,
        RNBYTE(rtn->rtn_bit), rtn->rtn_tbit))
        return (std_rt_head *)0;

    if (RDX_TEST_BIT(rtn->rtn_flags, RDX_RN_DELE_BIT))
        return (std_rt_head *)0;

    return rth;

} // std_radix_getexact()

std_rt_head * std_radix_getnext(std_rt_table *rtt, u_char *dest, ushort bitlen)
{
    rt_node *rtn;
    u_char *ap, *ap2;
    u_short bits2chk, dbit;

#if _BYTE_ORDER == _LITTLE_ENDIAN
    if (rtt->rtt_convert && dest) {
        memset (rtt->temp, '\0', ((rtt->rtt_maxaddrlen + (DIVISOR-1))/DIVISOR));
        rtt->rtt_convert(dest, (char *) rtt->temp,rtt->rtt_maxaddrlen);
        dest = rtt->temp;
    }
#endif

    RDX_DEBUG_START(rtt);

    /*
     * Check if the given address length is valid.
     */
    if (bitlen > rtt->rtt_maxaddrlen)
        return (std_rt_head *)0;

    RDX_DEBUG_END;

    /*
     * If there is no table, or nothing to do, assume nothing found.
     */
    if (!(rtn = rtt->rtt_root))
        return (std_rt_head *)0;

    /*
     * The first job here is to find a node in the tree which is
     * known to be after the given dest/mask in lexigraphic order.
     * If no mask was given use a host mask, as this guarantees we
     * will skip all same-address entries in the table.  First compute
     * the bit length of the mask.  If he gave us no destination we
     * start from the top of the tree, otherwise we search.
     */
    if (dest) {
    /*
     * Search down the tree as far as we can until we find a node
     * which has a bit number the same or larger than ours which has
     * an rth attached.
     */
    ap = dest;
    while (rtn->rtn_bit < bitlen || rtn->rtn_rth == (std_rt_head *) 0) {
        if (rtn->rtn_tbit & ap[RNBYTE(rtn->rtn_bit)]) {
        if (!(rtn->rtn_right)) {
            break;
        }
        rtn = rtn->rtn_right;
        } else {
        if (!(rtn->rtn_left)) {
            break;
        }
        rtn = rtn->rtn_left;
        }
    }

    /*
     * Determine the bit position of the first bit which differs between
     * the destination we found and the one we were given, as this will
     * suggest where we should search.  Often this will be an exact
     * match.
     */
    bits2chk = MIN(rtn->rtn_bit, bitlen);
    ap2 = rtn->rtn_rth->rdx_rth_addr;
    for (dbit = 0; dbit < bits2chk; dbit += RNBBY) {
        register int i = dbit >> RNSHIFT;
        if (ap[i] != ap2[i]) {
        dbit += first_bit_set[ap[i] ^ ap2[i]];
        break;
        }
    }

    /*
     * If we got an exact match, this is either our node (if his mask
     * is longer than ours) or we only need to find the next node in
     * the tree.  Do this now since this may be the normal case and
     * is fairly easy.
     */
    if (dbit >= bits2chk) {
        if (rtn->rtn_bit <= bitlen) {
        if (rtn->rtn_left) {
            rtn = rtn->rtn_left;
        } else if (rtn->rtn_right) {
            rtn = rtn->rtn_right;
        } else {
            rt_node *rn_next;

            do {
            rn_next = rtn;
            rtn = rtn->rtn_parent;
            if (!rtn) {
                return (std_rt_head *) 0;
            }
            } while (!(rtn->rtn_right) || rtn->rtn_right == rn_next);
            rtn = rtn->rtn_right;
        }
        }
    } else {
        rt_node *rn_next;

        /*
         * Here we found a node which differs from our target destination
         * in the low order bits.  We need to determine whether our guy
         * is too big, or too small, for the branch we are in.  If
         * he is too big, walk up until we find a node with a smaller
         * bit number than dbit where we branched left and search to
         * the right of this.  Otherwise all the guys in this branch
         * will be larger than us, so walk up the tree to the first
         * node we a bit number > dbit and search from there
         */
        if (ap[RNBYTE(dbit)] & RNBIT(dbit)) {
        do {
            rn_next = rtn;
            rtn = rtn->rtn_parent;
            if (!rtn) {
            return (std_rt_head *) 0;
            }
            RDX_ASSERT(rtn->rtn_bit != dbit);
        } while (rtn->rtn_bit > dbit || (!(rtn->rtn_right)
            || rtn->rtn_right == rn_next));
        rtn = rtn->rtn_right;
        } else {
        rn_next = rtn->rtn_parent;
        while (rn_next && rn_next->rtn_bit > dbit) {
            rtn = rn_next;
            rn_next = rn_next->rtn_parent;
        }
        }
    }
    }

    /*
     * If we have a pointer to a radix node which is in an area of the
     * tree where the address/masks are larger than our own.  Walk the
     * tree from here, checking each node with an rth attached until
     * we find one which matches our criteria.
     */
    for (;;) {
    if (rtn->rtn_rth)
        return rtn->rtn_rth;

    if (rtn->rtn_left) {
        rtn = rtn->rtn_left;
    } else if (rtn->rtn_right) {
        rtn = rtn->rtn_right;
    } else {
        rt_node *rn_next;

        do {
        rn_next = rtn;
        rtn = rtn->rtn_parent;
        if (!rtn) {
            return (std_rt_head *) 0;
        }
        } while (!(rtn->rtn_right) || rtn->rtn_right == rn_next);
        rtn = rtn->rtn_right;
    }
    }

    return (std_rt_head *)0;
}

std_rt_head * std_radix_insert(std_rt_table *rtt, std_rt_head *rth, ushort bitlen)
{
    u_int i;
    u_short bits2chk, dbit;
    u_char *addr, *his_addr;
    rt_node *rtn, *rtn_prev, *rtn_add, *rtn_new;

#if _BYTE_ORDER == _LITTLE_ENDIAN
    if (rtt->rtt_convert && rth->rth_addr) {

        memset (rtt->temp, '\0', ((rtt->rtt_maxaddrlen + (DIVISOR-1))/DIVISOR));

        rtt->rtt_convert (rth->rth_addr, (char *) rtt->temp, rtt->rtt_maxaddrlen);

        if (rth->rdx_rth_addr == NULL) {

            rth->rdx_rth_addr = (u_char *)
                RDX_MALLOC (((rtt->rtt_maxaddrlen + (DIVISOR-1))/DIVISOR) + 1);

            if (rth->rdx_rth_addr == NULL) {

                return (std_rt_head *) 0;
            }

            memcpy (rth->rdx_rth_addr,
                    rtt->temp, ((rtt->rtt_maxaddrlen + (DIVISOR-1))/DIVISOR));

            rth->magic = RT_RTH_ADDR_MAGIC;
        }
        else if (rth->magic == RT_RTH_ADDR_MAGIC) {

            if (memcmp (rth->rdx_rth_addr, rtt->temp,
                        ((rtt->rtt_maxaddrlen + (DIVISOR-1))/DIVISOR)) != 0) {

                RDX_ASSERT (0);
            }
        }
        else {

            /*
             * This is the case where rth structure is not memset by the
             * application, since magic field does not match. So allocate
             * memory and copy the key.
             */
#ifndef FINAL_RELEASE
            /*
             * Wanted to assert in the Engineering build so that all such
             * instances are identified and corrected.
             */
            RDX_ASSERT (0);
#endif
            rth->rdx_rth_addr = (u_char *)
                RDX_MALLOC (((rtt->rtt_maxaddrlen + (DIVISOR-1))/DIVISOR) + 1);

            if (rth->rdx_rth_addr == NULL) {

                return (std_rt_head *) 0;
            }

            memcpy (rth->rdx_rth_addr,
                    rtt->temp, ((rtt->rtt_maxaddrlen + (DIVISOR-1))/DIVISOR));

            rth->magic = RT_RTH_ADDR_MAGIC;
        }
    } else
#endif
    {
        RDX_ASSERT (rth->rth_addr);

        rth->rdx_rth_addr = rth->rth_addr;
    }

    RDX_DEBUG_START(rtt);

    /*
     * Clear all fields in user rth node.
     */
    rth->rth_rtn = (rt_node *)0;
    rth->rth_version = 0;

    if ( (rtt->rtt_radicalused == TRUE)  || (rtt->rtt_carused == TRUE) ){
        ((std_radical_head_t *)rth)->rdcl_flags = 0;
        memset(&((std_radical_head_t *)rth)->rdcl_cl, '\0', sizeof(std_dll));
    }

    /*
     * Check if the given address length is valid.
     */
    if (bitlen > rtt->rtt_maxaddrlen)
        return (std_rt_head *)0;

    RDX_DEBUG_END;

    rtn_prev = rtt->rtt_root;
    rtt->rtt_ninserts++;

    /*
     * If there is no existing root node, this is it.  Catch this
     * case now.
     */
    if (!rtn_prev) {
        if (!(rtn = (rt_node *) rtt->rtt_malloc(sizeof(rt_node))))
            return (std_rt_head *)0;
        memset(rtn, '\0', sizeof(rt_node));
        rtt->rtt_root = rth->rth_rtn = rtn;
        RN_SETBIT(rtn, bitlen);
        rtn->rtn_version = 0;
        rtn->rtn_rth = rth;
        rtt->rtt_inodes++;
        rtt->rtt_routes++;
        rtt->rtt_nmalloc++;
        return rth;
    }

    /*
     * Search down the tree as far as we can, stopping at a node
     * with a bit number >= ours which has an rth attached.  It
     * is possible we won't get down the tree this far, however,
     * so deal with that as well.
     */
    addr = rth->rdx_rth_addr;
    rtn = rtn_prev;
    while (rtn->rtn_bit < bitlen || !(rtn->rtn_rth)) {
        if (BIT_TEST(addr[RNBYTE(rtn->rtn_bit)], rtn->rtn_tbit)) {
            if (!(rtn->rtn_right)) {
                break;
            }
            rtn = rtn->rtn_right;
        } else {
            if (!(rtn->rtn_left)) {
                break;
            }
            rtn = rtn->rtn_left;
        }
    }

    /*
     * Now we need to find the number of the first bit in our address
     * which differs from his address.
     */
    bits2chk = MIN(rtn->rtn_bit, bitlen);
    his_addr = rtn->rtn_rth->rdx_rth_addr;
    for (dbit = 0; dbit < bits2chk; dbit += RNBBY) {
        i = dbit >> RNSHIFT;
        if (addr[i] != his_addr[i]) {
            dbit += first_bit_set[addr[i] ^ his_addr[i]];
            break;
        }
    }

    /*
     * If the different bit is less than bits2chk we will need to
     * insert a split above him.  Otherwise we will either be in
     * the tree above him, or attached below him.
     */
    if (dbit > bits2chk) {
        dbit = bits2chk;
    }
    rtn_prev = rtn->rtn_parent;
    while (rtn_prev && rtn_prev->rtn_bit >= dbit) {
        rtn = rtn_prev;
        rtn_prev = rtn->rtn_parent;
    }

    /*
     * Okay.  If the node rtn points at is equal to our bit number, we
     * may just be able to attach the rth to him.  Check this since it
     * is easy. If a user node is already attached then return back
     * the pointer to that node.
     */
    if (dbit == bitlen && rtn->rtn_bit == bitlen) {
        if (!RDX_TEST_BIT(rtn->rtn_flags, RDX_RN_DELE_BIT) && rtn->rtn_rth)
            return rtn->rtn_rth;
        if (rtn->rtn_rth) {
#if _BYTE_ORDER == _LITTLE_ENDIAN
            if(rtt->rtt_convert && rtn->rtn_rth->rdx_rth_addr)
            {
                RDX_FREE(rtn->rtn_rth->rdx_rth_addr);
            }
#endif
            rtn->rtn_rth->rdx_rth_addr = NULL;

            if (rtt->rtt_rmfree)
            {
                (* rtt->rtt_rmfree)(rtn->rtn_rth);
                rtt->rtt_nusrfrees++;
            }
        }
        rtn->rtn_rth = rth;
        rth->rth_rtn = rtn;
        rtn->rtn_flags = RDX_CLEAR_BIT(rtn->rtn_flags, RDX_RN_DELE_BIT);
        rtt->rtt_routes++;
        return rth;
    }

    /*
     * Allocate us a new node, we are sure to need it now.
     */
    if (!(rtn_add = (rt_node *)rtt->rtt_malloc(sizeof(rt_node))))
        return (std_rt_head *)0;
    memset(rtn_add, '\0', sizeof(rt_node));
    rtt->rtt_inodes++;
    rtt->rtt_nmalloc++;
    RN_SETBIT(rtn_add, bitlen);
    rtn_add->rtn_rth = rth;
    rth->rth_rtn = rtn_add;

    /*
     * There are a couple of possibilities.  The first is that we
     * attach directly to the thing pointed to by rtn.  This will be
     * the case if his bit is equal to dbit.
     */
    if (rtn->rtn_bit == dbit) {
        RDX_ASSERT(dbit < bitlen);
        rtn_add->rtn_parent = rtn;
        if (BIT_TEST(addr[RNBYTE(rtn->rtn_bit)], rtn->rtn_tbit)) {
            RDX_ASSERT(!(rtn->rtn_right));
            rtn->rtn_right = rtn_add;
        } else {
            RDX_ASSERT(!(rtn->rtn_left));
            rtn->rtn_left = rtn_add;
        }
        rtt->rtt_routes++;
        return rth;
    }

    /*
     * The other case where we don't need to add a split is where
     * we were on the same branch as the guy we found.  In this case
     * we insert rtn_add into the tree between rtn_prev and rtn.  Otherwise
     * we add a split between rtn_prev and rtn and append the node we're
     * adding to one side of it.
     */
    if (dbit == bitlen) {
        if (BIT_TEST(his_addr[RNBYTE(rtn_add->rtn_bit)], rtn_add->rtn_tbit)) {
            rtn_add->rtn_right = rtn;
        } else {
            rtn_add->rtn_left = rtn;
        }
        rtn_new = rtn_add;
    } else {
        if (!(rtn_new = (rt_node *) rtt->rtt_malloc(sizeof(rt_node)))) {
            rtt->rtt_free(rtn_add);
            rtt->rtt_nfree++;
            rtt->rtt_inodes--;
            return (std_rt_head *)0;
        }
        memset(rtn_new, '\0', sizeof(rt_node));
        rtt->rtt_inodes++;
        rtt->rtt_nmalloc++;
        RN_SETBIT(rtn_new, dbit);
        rtn_add->rtn_parent = rtn_new;
        if (BIT_TEST(addr[RNBYTE(rtn_new->rtn_bit)], rtn_new->rtn_tbit)) {
            rtn_new->rtn_right = rtn_add;
            rtn_new->rtn_left = rtn;
        } else {
            rtn_new->rtn_left = rtn_add;
            rtn_new->rtn_right = rtn;
        }
    }
    rtn->rtn_parent = rtn_new;
    rtn_new->rtn_version = rtn->rtn_version;
    rtn_new->rtn_parent = rtn_prev;

    /*
     * If rtn_prev is NULL this is a new root node, otherwise it
     * is attached to the guy above in the place where rtn was.
     */
    if (!rtn_prev) {
        rtt->rtt_root = rtn_new;
    } else if (rtn_prev->rtn_right == rtn) {
        rtn_prev->rtn_right = rtn_new;
    } else {
        RDX_ASSERT(rtn_prev->rtn_left == rtn);
        rtn_prev->rtn_left = rtn_new;
    }

    rtt->rtt_routes++;
    return rth;

} // std_radix_insert()

static rt_node * _std_radix_remove(std_rt_table *rtt, rt_node *rn, int *dir)
{
    rt_node *rn_next = 0;
    rt_node *rn_prev = 0;
    rt_node *rn_ret = 0;

    RDX_ASSERT(rtt);
    RDX_ASSERT(rn);

    RDX_ASSERT(!RN_IFLOCK(rn));

    rtt->rtt_routes--;

    /*
     * Catch the easy case.  If this guy has nodes on both his left
     * and right, he stays in the tree.
     */
    if (rn->rtn_left && rn->rtn_right) {
        if (rn->rtn_rth) {
#if _BYTE_ORDER == _LITTLE_ENDIAN
            if(rtt->rtt_convert && rn->rtn_rth->rdx_rth_addr)
            {
                RDX_FREE(rn->rtn_rth->rdx_rth_addr);
            }
#endif
            rn->rtn_rth->rdx_rth_addr = NULL;

            if (rtt->rtt_rmfree)
            {
                rtt->rtt_nusrfrees++;
                rtt->rtt_rmfree(rn->rtn_rth);
            }
        }
        rn->rtn_rth = (std_rt_head *) 0;
        *dir = RDX_WALKUP;
        return rn->rtn_parent;
    }

    /*
     * If this guy has no successor he's a goner.  The guy above
     * him will be too, unless he's got external stuff attached to
     * him.
     */
    if (!(rn->rtn_left) && !(rn->rtn_right)) {
    rn_prev = rn->rtn_parent;
        RDX_ASSERT(!RN_IFLOCK(rn));
        if (rn->rtn_rth) {
#if _BYTE_ORDER == _LITTLE_ENDIAN
            if(rtt->rtt_convert && rn->rtn_rth->rdx_rth_addr)
            {
                RDX_FREE(rn->rtn_rth->rdx_rth_addr);
            }
#endif
            rn->rtn_rth->rdx_rth_addr = NULL;

            if (rtt->rtt_rmfree)
            {
                rtt->rtt_rmfree(rn->rtn_rth);
                rtt->rtt_nusrfrees++;
            }
        }
        rtt->rtt_free(rn);
        rtt->rtt_nfree++;
    rtt->rtt_inodes--;

    if (!rn_prev) {
        /*
         * Last guy in the tree, remove the root node pointer
         */
        rtt->rtt_root = (rt_node *)0;
        return (rt_node *)0;
    }

    if (rn_prev->rtn_left == rn) {
        rn_prev->rtn_left = (rt_node *) 0;
            *dir = RDX_WALKRIGHT;
    } else {
        RDX_ASSERT(rn_prev->rtn_right == rn);
        rn_prev->rtn_right = (rt_node *) 0;
            *dir = RDX_WALKUP;
    }

    if (rn_prev->rtn_rth) {
        return rn_prev;
    }
    rn = rn_prev;
    }

    if (RN_IFLOCK(rn)) {
        RDX_ASSERT(0);
        rn->rtn_flags = RDX_SET_BIT(rn->rtn_flags, RDX_RN_DELE_BIT);
        return rn_prev;
    }

    RDX_ASSERT(!rn->rtn_left || !rn->rtn_right);

    /*
     * Here we have a one-way brancher with no external stuff attached
     * (either we just removed the external stuff or one of his child
     * nodes).  Remove him, promoting his one remaining child.
     */
    rn_prev = rn->rtn_parent;
    if (rn->rtn_left) {
    rn_next = rn->rtn_left;
        rn_ret = rn_prev;
        *dir = RDX_WALKRIGHT;
    } else {
    rn_next = rn->rtn_right;
        rn_ret = rn_next;
        *dir = RDX_WALKDOWN;
    }
    rn_next->rtn_parent = rn_prev;

    if (!rn_prev) {
    /*
     * Our guy's a new root node, put him in.
     */
    rtt->rtt_root = rn_next;
    } else {
    /*
     * Find the pointer to our guy in the parent and replace
     * it with the pointer to our former child.
     */
    if (rn_prev->rtn_left == rn) {
        rn_prev->rtn_left = rn_next;
    } else {
        RDX_ASSERT(rn_prev->rtn_right == rn);
        rn_prev->rtn_right = rn_next;
    }
    }

    /*
     * Done, blow this one away as well.
     */
    RDX_ASSERT(!RN_IFLOCK(rn));
    if (rn->rtn_rth) {
#if _BYTE_ORDER == _LITTLE_ENDIAN
        if(rtt->rtt_convert && rn->rtn_rth->rdx_rth_addr)
        {
            RDX_FREE(rn->rtn_rth->rdx_rth_addr);
        }
#endif
        rn->rtn_rth->rdx_rth_addr = NULL;

        if (rtt->rtt_rmfree)
        {
            rtt->rtt_rmfree(rn->rtn_rth);
            rtt->rtt_nusrfrees++;
        }
    }
    rtt->rtt_free(rn);
    rtt->rtt_nfree++;
    rtt->rtt_inodes--;

    return rn_ret;
} // _std_radix_remove()


void std_radix_remove(std_rt_table *rtt, std_rt_head *rth)
{
    int dir;
    rt_node *rn;

    RDX_DEBUG_START(rtt);
    RDX_ASSERT(rth);
    RDX_DEBUG_END;

    rtt->rtt_nremoves++;
    rn = rth->rth_rtn;

    /*
     * If the node is locked means that a walker is under
     * the subtree, so don't delete the the node, or else
     * we will mess up the walker. Just mark the node for
     * deletion; when the last walker traces back above
     * the node will delete it from the tree.
     *
     * Check that the 'rmfree' pointer is valid in this case.
     */
    if (RN_IFLOCK(rn)) {
        rn->rtn_flags = RDX_SET_BIT(rn->rtn_flags, RDX_RN_DELE_BIT);
        RDX_ASSERT(rtt->rtt_rmfree);
        return;
    }

    _std_radix_remove(rtt, rn, &dir);

    /*
     * This has been added to support RADICAL. Remove this
     * rth from the change-list, do this only if RADICAL is in
     * use, otherwise we may be accessing outside bounds.
     */
     if (rtt->rtt_radicalused == TRUE)
     {
        if (std_dll_islinked(&((std_radical_head_t *)rth)->rdcl_cl))
        {
            std_dll_remove(&rtt->rtt_clhead, &((std_radical_head_t *)rth)->rdcl_cl);
            ((std_radical_head_t *)rth)->rdcl_flags &= ~RDCL_INCL;
        }
     }

} // std_radix_remove()

#define RDXUSERCALLBACK(x)                       \
{                                                \
    va_list ap;                                  \
                                                 \
    lcnt--;                                      \
    if (walk_fn)                                 \
    {                                            \
        va_start(ap, cnt);                       \
        if (walk_fn((x), ap))                    \
            lcnt = 0;                            \
        va_end(ap);                              \
    }                                            \
}

std_rt_head * std_radix_walk(std_rt_table *rtt, std_rt_head *rth, int (* walk_fn)(std_rt_head *, va_list ap), int cnt, ...)
{
    u_long lcnt;
    rt_node *y;
    int dir;
    rt_node *rtn;

    RDX_DEBUG_START(rtt);
    RDX_DEBUG_END;

    if (!cnt)
        lcnt = 0xffffffff;
    else
        lcnt = (u_long) cnt;

    if (!rth)
        dir = RDX_WALKDOWN, rtn = rtt->rtt_root;
    else {
        rtn = rth->rth_rtn;
        if (!rtn)
            return (std_rt_head *)0;
        dir = RDX_WALKRIGHT;
    }

    while (lcnt && rtn) {
        switch (dir) {
            case RDX_WALKDOWN:
                if (rtn->rtn_rth)
                    RDXUSERCALLBACK(rtn->rtn_rth);
                if (rtn->rtn_left) {
                    rtn = rtn->rtn_left;
                } else {
                    dir = RDX_WALKRIGHT;
                }
                continue;

            case RDX_WALKRIGHT:
                if (!rtn->rtn_right) {
                    dir = RDX_WALKUP;
                } else {
                    dir = RDX_WALKDOWN;
                    rtn = rtn->rtn_right;
                }
                continue;

            case RDX_WALKUP:
                y = rtn;
                if (!(rtn = rtn->rtn_parent))
                    return (std_rt_head *)0;
                if (y == rtn->rtn_left) {
                    dir = RDX_WALKRIGHT;
                }
                continue;

            default:
                RDX_ASSERT(0);
        } // switch
    } // while


   if (!rtn)
            return (std_rt_head *)0;
   else
    return rtn->rtn_rth;

} // std_radix_walk()

std_rt_head * std_radix_versionwalk(std_rt_table *rtt, std_rt_head *rth, int (* walk_fn)(std_rt_head *, va_list ap),
                            int cnt, std_radix_version_t min_ver, std_radix_version_t max_ver, ...)
{
    int dir;
    rt_node *y;
    va_list ap;
    u_long lcnt;
    rt_node *rtn;
    std_rt_head *t_rth;

    RDX_DEBUG_START(rtt);
    RDX_DEBUG_END;

    if (!cnt)
        lcnt = 0xffffffff;
    else
        lcnt = (u_long) cnt;

    if (!rth)
        dir = RDX_WALKDOWN, rtn = rtt->rtt_root;
    else {
        rtn = rth->rth_rtn;
        if (!rtn)
            return (std_rt_head *)0;
        dir = RDX_WALKRIGHT;
    }

    while (lcnt && rtn) {
        switch (dir) {
            case RDX_WALKDOWN:
                if (rtn->rtn_version < min_ver) {
                    dir = RDX_WALKUP;
                    continue;
                }

                if (rtn->rtn_rth && walk_fn)
                {
                    t_rth = rtn->rtn_rth;
                    if (t_rth->rth_version >= min_ver
                        && t_rth->rth_version <= max_ver)
                    {
                        lcnt--;
                        RN_LOCK(rtn);

                        va_start(ap, max_ver);
                        if (walk_fn(t_rth, ap)) {
                            lcnt = 0;
                        }
                        va_end(ap);

                        RN_UNLOCK(rtn);
                    }
                }

                if (rtn->rtn_left) {
                    rtn = rtn->rtn_left;
                } else {
                    dir = RDX_WALKRIGHT;
                }
                continue;

            case RDX_WALKRIGHT:
                if (!rtn->rtn_right) {
                    dir = RDX_WALKUP;
                } else {
                    dir = RDX_WALKDOWN;
                    rtn = rtn->rtn_right;
                }
                continue;

            case RDX_WALKUP:
                y = rtn;
                rtn = rtn->rtn_parent;

                if (!RN_IFLOCK(y) && RDX_TEST_BIT(y->rtn_flags,RDX_RN_DELE_BIT)) {
                    rtn = _std_radix_remove(rtt, y, &dir);
                } else {
                    if (rtn && y == rtn->rtn_left) {
                        dir = RDX_WALKRIGHT;
                    } else {
                        dir = RDX_WALKUP;
                    }
                }

                continue;

            default:
                RDX_ASSERT(0);
        } // switch
    } // while

    if (rtn) {
        y = rtn;

        do {
            if (!RN_IFLOCK(y) && RDX_TEST_BIT(y->rtn_flags, RDX_RN_DELE_BIT))
                y = _std_radix_remove(rtt, y, &dir);
            else
                y = y->rtn_parent;
        } while (y);

        return rtn->rtn_rth;
    }

    return (std_rt_head *)0;

} // std_radix_versionwalk()

u_long std_radix_maxprint = 500;

/**
 *  Print a visual representation of the tree.
 *  Display information about the radix tree, including a visual
 *  representation of the tree itself
 */
void std_radix_print(std_rt_table *rtt)
{
    struct {
    rt_node *rn;
    int    state;
    char    right;
    } stack[MAXDEPTH], *sp;
    char prefix[MAXDEPTH];
    int i = MAXDEPTH;
    char number[4];

    RDX_DEBUG_START(rtt);
    RDX_DEBUG_END;

    while (i--) {
    prefix[i] = ' ';
    }

    {
    sp = stack;
    (void) printf("\tRadix tree %s: %lu inodes, %lu routes, %llu version.",
               rtt->rtt_name, rtt->rtt_inodes, rtt->rtt_routes,
                       rtt->rtt_version);
    if (rtt->rtt_inodes > std_radix_maxprint) {
        (void) printf(" (too large to print)\n\n");
    } else if (!(sp->rn = rtt->rtt_root)) {
        (void) printf(" (empty)\n\n");
    } else {
        /* If the tree is small enough, format it */
        (void) printf("\n\n");
        sp->right = TRUE;
        sp->state = 0;
        do {
        rt_node *rn = sp->rn;
        int ii = (sp - stack) * 3;

        switch (sp->state) {
        case 0:
            sp->state++;
            if (rn->rtn_right) {
            sp++;
            sp->rn = rn->rtn_right;
            sp->right = TRUE;
            sp->state = 0;
            continue;
            }
            /* Fall through */

        case 1:
            sp->state++;

            (void) snprintf(number,sizeof(number), "%3d", rn->rtn_bit);
            for (i = 0; i < 3 && number[i] == ' '; i++) {
            number[i] = '-';
            }
            (void) printf("\t\t%.*s+-%s%u+",
                   ii, prefix,
                   (rn->rtn_bit > 9) ? "" : "-",
                   rn->rtn_bit);
            if (rn->rtn_rth) {
            (void) printf("--[%s (v%llu, l%lu, f%lu, 0x%lx)\n",
                       std_radix_printaddr(rn->rtn_rth->rdx_rth_addr,
                                       rtt->rtt_maxaddrlen),
                                       rn->rtn_rth->rth_version,
                                       (u_long)rn->rtn_lock,
                                       (u_long)rn->rtn_flags,
                                       (u_long)rn->rtn_rth);
            } else {
            (void) printf(" (v%llu, l%lu, f%lu, 0x%lx)\n",
                                       rn->rtn_version,
                                       (u_long)rn->rtn_lock,
                                       (u_long)rn->rtn_flags,
                                       (u_long)rn);
            }

            switch (sp->right) {
            case TRUE:
            prefix[ii] = '|';
            break;

            case FALSE:
            prefix[ii] = ' ';
            break;
            }

            if (rn->rtn_left) {
            sp++;
            sp->rn = rn->rtn_left;
            sp->right = FALSE;
            sp->state = 0;
            continue;
            }

        case 2:
            /* Pop the stack */
            sp--;
        }
        } while (sp >= stack) ;
    }
    }
    (void) printf("\n");
} // std_radix_print()

std_rt_table * std_radix_create(char *rtt_name, ushort maxaddrlen, void *rtt_malloc(size_t),
                            void rtt_free(void *), void rtt_rmfree(void *))
{
    std_rt_table *rtt;
    RDX_ASSERT(rtt_name);

    if ((rtt = (std_rt_table *) RDX_MALLOC(sizeof(std_rt_table))) == (std_rt_table *)0)
        return (std_rt_table *)0;

    memset(rtt, '\0', sizeof(std_rt_table));

#if _BYTE_ORDER == _LITTLE_ENDIAN
    if ((rtt->temp = (u_char *)RDX_MALLOC((((maxaddrlen + (DIVISOR-1))/DIVISOR)))) == NULL)
    {
        RDX_FREE(rtt);
        rtt = NULL;
        return (std_rt_table *)0;
    }

    memset (rtt->temp, '\0', ((maxaddrlen + (DIVISOR-1))/DIVISOR));
#endif

    rtt->rtt_magic = RDX_MAGIC;
    strncpy(rtt->rtt_name,rtt_name,RDX_NAME_MAX_LEN);
    rtt->rtt_name[RDX_NAME_MAX_LEN] = '\0';
    rtt->rtt_version = RDX_INITIALVER;
    rtt->rtt_maxaddrlen = maxaddrlen;
    rtt->rtt_rmfree = rtt_rmfree;
    rtt->rtt_radicalused = FALSE;
    rtt->rtt_carused = FALSE;
    rtt->rtt_convert = NULL ;

    if (!rtt_malloc)
        rtt->rtt_malloc = std_radix_malloc;
    else
        rtt->rtt_malloc = rtt_malloc;

    if (!rtt_free)
        rtt->rtt_free = std_radix_free;
    else
        rtt->rtt_free = rtt_free;

    std_dll_init(&rtt->rtt_clhead);

    return rtt;
} // std_radix_create()


void std_radix_destroy(std_rt_table *rtt)
{
    RDX_DEBUG_START(rtt);
    RDX_DEBUG_END;

    if (!rtt)
        return;
    RDX_ASSERT(!rtt->rtt_root);
    RDX_ASSERT(rtt->rtt_magic == RDX_MAGIC);

    rtt->rtt_magic = 0; /* daggling ptr may give problem; so clear it anyway */

#if _BYTE_ORDER == _LITTLE_ENDIAN
    RDX_FREE(rtt->temp);
    rtt->temp = NULL;
#endif

    RDX_FREE(rtt);
    rtt = NULL;
} // std_radix_destroy()

void std_radix_init(std_rt_table *rtt)
{
    RDX_DEBUG_START(rtt);
    RDX_DEBUG_END;

    if (!rtt)
        return;

    RDX_ASSERT(rtt->rtt_magic == RDX_MAGIC);

    rtt->rtt_root = NULL;

    rtt->rtt_inodes = 0;
    rtt->rtt_routes = 0;
    rtt->rtt_ninserts = 0;
    rtt->rtt_nremoves = 0;
    rtt->rtt_nmalloc = 0;
    rtt->rtt_nfree = 0;
    rtt->rtt_nusrfrees = 0;
    rtt->rtt_nwraps = 0;
    rtt->rtt_version = RDX_INITIALVER;
    std_dll_init(&rtt->rtt_clhead);

    return;
} // std_radix_init()

std_radix_version_t std_radix_setversion(std_rt_table *rtt, std_rt_head *rth)
{
    std_radix_version_t ver;
    rt_node *rtn;

    RDX_DEBUG_START(rtt);
    RDX_ASSERT(rth);
    RDX_DEBUG_END;

    rtn = rth->rth_rtn;
    RDX_ASSERT(rtn);

    ver = ++rtt->rtt_version;
    rth->rth_version = ver;

    while (rtn)
    {
        rtn->rtn_version = ver;
        rtn = rtn->rtn_parent;
    }

    if (!ver)
        rtt->rtt_nwraps++;

    return ver;

} // std_radix_setversion()

std_rt_head * std_radix_getlessspecific(std_rt_table *rtt, std_rt_head *rth)
{
    rt_node *rtn;
    std_rt_head *t_rth = (std_rt_head *)0;

    RDX_DEBUG_START(rtt);
    RDX_ASSERT(rth);
    RDX_DEBUG_END;

    rtn = rth->rth_rtn;
    RDX_ASSERT(rtn);

    rtn = rtn->rtn_parent;
    while (rtn)
    {
        if (rtn->rtn_rth) {
            t_rth = rtn->rtn_rth;
            break;
        }
        rtn = rtn->rtn_parent;
    }

    return t_rth;
} // std_radix_getlessspecific()
