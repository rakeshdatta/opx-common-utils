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
 * filename: std_radix.h
 */

/*!
 * \file   std_radix.h
 * \brief  Radix tree implementation with Radical CL support
 * \date   05-2014
 */

#ifndef _RADIX_H_
#define _RADIX_H_

#include <stdarg.h>
#include <assert.h>
#include "std_llist.h"

/*---------------------------------------------------------------*\
 *                    Defines and Macros.
\*---------------------------------------------------------------*/

#if _BYTE_ORDER ==_LITTLE_ENDIAN
#define RDX_TREE_SET_CONVERT_FN(_tree, fn) (_tree)->rtt_convert = &(fn);
#else
#define RDX_TREE_SET_CONVERT_FN(_tree, fn) (_tree)->rtt_convert = NULL;
#endif

/// Max length of name of tree
#define RDX_NAME_MAX_LEN 50

/// Flag bit for rt_node to indicate the node is marked for deletion.
#define RDX_RN_DELE_BIT  1

#define RDX_TEST_BIT(flag, bit) ((flag) & (1 << (bit)))
#define RDX_SET_BIT(flag, bit) ((flag) | (1 << (bit)))
#define RDX_CLEAR_BIT(flag, bit) ((flag) & ~(1 << (bit)))

#define ERROR -1
#define NBBY 8

/*---------------------------------------------------------------*\
 *                    Data structures.
\*---------------------------------------------------------------*/

typedef unsigned long long std_radix_version_t;

/**
 *  Radix Tree Node. The glue node or the internal node with which to
 *  build a radix tree.
 *  Note that this radix trie can support keys no geater than 256 bits.
 */
struct _rt_node
{
    /// Child when bit clear.
    struct _rt_node *rtn_left;

    /// Child when bit set.
    struct _rt_node *rtn_right;

    /// Back pointer to parent node.
    struct _rt_node *rtn_parent;

    /// Bit number for node/mask.
    ushort rtn_bit;

    /// Byte to test in address.
    // u_char rtn_tbyte;

    /// Bit to test in byte.
    u_char rtn_tbit;

    /// Node status information
    u_char rtn_flags;

    /// Lock from deletion.
    u_char rtn_lock;

    /// Max version of the sub-tree.
    std_radix_version_t rtn_version;

    /// Our external info; radix user data.
    struct _std_rt_head *rtn_rth;
};

/// Typedef for struct _rt_node.
typedef struct _rt_node rt_node;

/**
 *  Radix tree table.
 *  This is top level structure for maintaining a Radix tree
 *  instantiated by user. User is returned a pointer to this
 *  structure when a tree is instantiated and the pointer
 *  is required in all further operations on the tree.
 */
struct _std_rt_table
{
    /// To verify user is providing a 'good' RBT tree.
    u_long rtt_magic;

    /// Name of this tree; provided by user.
    char rtt_name[RDX_NAME_MAX_LEN+1];

    /// Current root of tree.
    struct _rt_node *rtt_root;

    /// Number of internal nodes in tree.
    u_long rtt_inodes;

    /// Number of routes in tree.
    u_long rtt_routes;

    /// Number of times user inserted a node.
    u_long rtt_ninserts;

    /// Number of times user removed a node.
    u_long rtt_nremoves;

    /// Number of times malloc was called.
    u_long rtt_nmalloc;

    /// Number of times free was called.
    u_long rtt_nfree;

    /// Number of times Radix removed a user node internally.
    u_long rtt_nusrfrees;

    /// Number of times versions wrapped around.
    u_long rtt_nwraps;

    /// Current version of tree.
    std_radix_version_t rtt_version;

    /// Offset at which the std_rt_head is place in user data.
    u_long rtt_rthoffset;

    /// Maximum address/mask length.
    ushort rtt_maxaddrlen;

    /// Debug enbale/disable flag.
    u_char rtt_debug;

#if _BYTE_ORDER == _LITTLE_ENDIAN
    //To store the temporary key value
    u_char *temp;
#endif

    /// User malloc routine here.
    void * (* rtt_malloc)(size_t);

    /// User free routing here.
    void (* rtt_free)(void *);

    /// User free routine here for freeing user node.
    void (* rtt_rmfree)(void *);

   /*Converting key from native to char * (if any)*/
    void (* rtt_convert)(void * ,char *,int);

    /// Count of current radical walkers.
    int rtt_radicalinuse;

    /// Flag to indicate that radical is in use.
    int rtt_radicalused;

    /// Change-list head for RADICAL.
    std_dll_head rtt_clhead;

    /// Flag to indicate that CAR is in use.
    int rtt_carused;

    /// Pointer to the CAR head.
    void *rtt_carhead;
};

/// Typedef for struct _std_rt_table.
typedef struct _std_rt_table std_rt_table;

#define RT_RTH_ADDR_MAGIC  0xCAFEBABE

#if _BYTE_ORDER == _LITTLE_ENDIAN
#define RT_HEAD \
    struct _rt_node   *rth_rtn; \
    u_char            *rth_addr; \
    std_radix_version_t  rth_version; \
    u_char            *rdx_rth_addr; \
    unsigned long      magic;
#else
#define RT_HEAD \
    struct _rt_node   *rth_rtn; \
    u_char            *rth_addr; \
    std_radix_version_t  rth_version; \
    u_char            *rdx_rth_addr;
#endif

/**
 *  Radix tree header.
 *  All user data structure that need to go on the tree MUST
 *  contain this structure as the first element.
 */
struct _std_rt_head {
    RT_HEAD;
};

/// Typedef for _std_rt_head structure.
typedef struct _std_rt_head std_rt_head;


/*---------------------------------------------------------------*\
 *                    Prototypes with documentation.
\*---------------------------------------------------------------*/

/** Create a Radix tree.
 *  Instantiates a radix tree. The maximum key length of 256 bits
 *  is supported for now.
 *  @param rtt_name Name of the radix tree.
 *  @param maxaddrlen Maximum length of key required for this tree.
 *                    For example, a radix tree with IPv4 address
 *                    as the key will specify 32 as the length.
 *  @param rtt_malloc User specified routine for memory allocation.
 *                    If none is provided then malloc from Libc is
 *                    used.
 *  @param rtt_free User specified routine for releasing memory.
 *                  If none provided then free from Libc is used.
 *  @param rtt_rmfree User specified routine to free the user
 *                    node after removal of a node from the tree.
 *                    Only for users using the yeilding feature
 *                    in the walker routine.
 *  @return Returns a pointer to std_rt_table structure, which is the
 *          top level structure for this Radix implementation. User
 *          must maintain is handle for performing any subsequent
 *          operation on the tree. On failure, returns 0.
 */
std_rt_table * std_radix_create(char *rtt_name, ushort maxaddrlen,
    void *rtt_malloc(size_t), void rtt_free(void *), void rtt_rmfree(void *));

/** Destroy radix tree.
 *  Destroys a previously created radix tree. User must ensure that
 *  there aren't any node on the tree at the time of destruction.
 *
 *  @param rtt Pointer to the radix tree to operate upon.
 *  @return Nothing.
 */
void std_radix_destroy(std_rt_table *rtt);

/** Re-initialize a radix tree's head
 *  Useful for quickly emptying a tree. Tree nodes are expected to
 *  have been removed separately. Hence, the root is set to NULL
 *  without checking the current pointer. Using this API, it isn't
 *  necessary to destroy and recreate the Radix tree
 *
 *  @param rtt Pointer to the radix tree to operate upon.
 *  @return Nothing.
 */
void std_radix_init(std_rt_table *rtt);

/** Validate that a node is a leaf
 *  Useful for checking whether specified node has a sub-tree
 *
 *  @param rth Pointer to the node on which to operate
 *  @return TRUE/FALSE.
 */
int std_radix_nodeisleaf(std_rt_head *rth);

/** Insert a node into radix tree.
 *  User must ensure that std_rt_head is properly allocated and the
 *  address pointer inside is correcly set.
 *
 *  @param rtt Pointer to a radix tree to operate upon.
 *  @param rth Pointer to a user allocated std_rt_head structure with
 *             address field properly assigned. Note that the
 *             address MUST be in network byte order.
 *  @param masklen Length of the significant prefix (prefix lenght).
 *  @return If node is successfully added then returns back the same
 *          std_rt_head pointer given by the user. If a node already
 *          exists for the given address and mask then the std_rt_head
 *          pointer to that node is returned (and the given node is
 *          not added to the tree). Otherwise returns a NULL pointer
 *          to indicate error (e.g. failed to allocate memory).
 */
std_rt_head * std_radix_insert(std_rt_table *rtt, std_rt_head *rth, ushort masklen);


/** Remove a node from the radix tree.
 *  User must have the std_rt_head pointer of the node that needs to
 *  be removed from the tree. The pointer must have bben gotten from
 *  one of the radix tree calls. the pointer must be fresh, that is,
 *  it must not be stale. the routine only removes the std_rt_head
 *  structure from the tree and does NOT release the memory allocation.
 *
 *  @param rtt Pointer to the radix tree to operate upon.
 *  @param rth Pointer to a valid std_rt_head structure to remove from
 *             the tree.
 *  @return Nothing.
 */
void std_radix_remove(std_rt_table *rtt, std_rt_head *rth);

/** Get the parent route for the one passed in. Keep
 *  going up the tree until the parent node has an
 *  external route specified.
 *  @param rtt Pointer to a radix tree to operate upon.
 *  @param rth Pointer to route head who parent is sought
 *  @return Pointer to the std_rt_head of the parent route.
 *  Otherwise returns 0.
 **/
std_rt_head * std_radix_getparent(std_rt_table *rtt, std_rt_head *rth);

/** Get the best route by longest prefix match.
 *  Fetches the best route in the tree that is equal to or less
 *  specific than the one provided in the parameters.
 *
 *  @param rtt Pointer to a radix tree to operate upon.
 *  @param addr Pointer to a bit stream of address in network byte order.
 *  @param masklen Prefix length of address. Only the best route (most
 *                 specific) that has prefix length equal to or less
 *                 than this value is matched.
 *  @return Pointer to the std_rt_head of the best route. Otherwise returns 0.
 */
std_rt_head * std_radix_getbest(std_rt_table *rtt, u_char *addr, ushort masklen);


/** Get the best and second route by longest prefix match.
 *  Fetches the best route in the tree that is equal to or less
 *  specific than the one provided in the parameters.
 *
 *  @param rtt Pointer to a radix tree to operate upon.
 *  @param addr Pointer to a bit stream of address in network byte order.
 *  @param masklen Prefix length of address. Only the best route (most
 *                 specific) that has prefix length equal to or less
 *                 than this value is matched.
 *  @return Pointer to the std_rt_head of the best route. Otherwise returns 0.
 *  @return Pointer to the std_rt_head of the second best route. Otherwise returns 0.
 */

std_rt_head * std_radix_getbestandprev(std_rt_table *rtt, u_char *addr,
                                   ushort bitlen, std_rt_head **lessbest);

/** Get the second route by longest prefix match.
 *  Fetches the best route in the tree that is equal to or less
 *  specific than the one provided in the parameters.
 *
 *  @param rtt Pointer to a radix tree to operate upon.
 *  @param addr Pointer to a bit stream of address in network byte order.
 *  @param masklen Prefix length of address. Only the best route (most
 *                 specific) that has prefix length equal to or less
 *                 than this value is matched.
 *  @return Pointer to the std_rt_head of the second best route. Otherwise returns 0.
 */

std_rt_head * std_radix_getnextbest(std_rt_table *rtt, u_char *addr, ushort bitlen);


/** Find a exact route on the tree.
 *  The routine finds only the exact route specified by the user.
 *
 *  @param rtt Pointer to a radix tree to operate upon.
 *  @param addr Pointer to a bit stream of address in network byte order.
 *  @param masklen Prefix length of the address.
 *  @return Returns a pointer to the std_rt_head of the prefix that
 *          exactly matches the given paramters. Otherwise returns 0.
 */
std_rt_head * std_radix_getexact(std_rt_table *rtt, u_char *addr, ushort masklen);


/** Find a next node from the given address.
 *  The routine finds ONLY the next one from the given address in the
 *  lexicographic order.
 *
 *  @param rtt Pointer to a radix tree to operate upon.
 *  @param addr Pointer to a bit stream of address in network byte order.
 *  @param masklen Prefix length of the address.
 *  @return Returns a pointer to the std_rt_head of the prefix that
 *          is next to the given paramters. Otherwise returns 0.
 */
std_rt_head * std_radix_getnext(std_rt_table *rtt, u_char *addr, ushort masklen);


/** Walk the radix tree with inorder or preorder callbacks. Though
 *  radix tree is not inherently ordered, preorder walk fetches nodes
 *  with keys in the lexicographic order. Therefore, and SNMP Get/GetNext
 *  can be achieved by this routine.
 *  @param rtt Pointer to the radix tree to operate upon.
 *  @param rth Pointer to std_rt_head node containing a key. If this
 *             pointer is NULL the walker starts the walk from
 *             the first node. When non-NULL, it expects to find
 *             a key at the correct offset and the walk begins from
 *             the node with that key (or greater) onwards.
 *  @param walkcb User function to callback for each user node found
 *                on the tree. The user callback back function must
 *                return an int. If the return value is 0 the walker
 *                continues. If the return value from the callback is
 *                non-zero the walker terminates the walk on that node.
 *                The variable parameters passed to std_radix_walk will be
 *                passed into the callback routine as a va_list. In the
 *                callback you can use va_arg to extract the varible
 *                parameters (see stdarg.h). If no callback routine
 *                is provided (that is, walkcb is zero) then the walker
 *                still walks the tree as specified (number of counts etc.)
 *                without making an attempt to callback.
 *  @param cnt Number of node to visit on this walk. If you want to walk
 *             the rest of the tree from 'data' onwards then use
 *             a value of 0.
 *  @return Returns a pointer to the data node next to the one on
 *          which last callback was issued.
 */
std_rt_head * std_radix_walk(std_rt_table *rtt, std_rt_head *rth,
                             int (* walk_fn)(std_rt_head *, va_list), int cnt, ...);


/** Print radix tree.
 *  This routine prints a visual representation of the tree to
 *  the standard output. If the tree size is too large (over 200 node)
 *  the routine returns without printing the tree.
 *
 *  @param rtt Pointer to a radix tree to operate upon.
 *  @return Nothing.
 */
void std_radix_print(std_rt_table *rtt);


/** Increment the version of the tree.
 *  This must be called if the version walking call is to be
 *  used.
 *  @param rtt Pointer to the radix tree to operate upon.
 *  @param rth Pointer to the std_rt_head whose version needs to
 *             be bumped.
 *  @return Returns the current version of the tree. Users must handle
 *          version warp around (that is, when value returned is 0).
 */
std_radix_version_t std_radix_setversion(std_rt_table *rtt, std_rt_head *rth);


/** Version walk the radix tree with preorder callbacks. This walker is
 *  different from the regular walk in the sense that it visits ONLY
 *  those nodes that have versions number in the user specified range.
 *  @param rtt Pointer to the radix tree to operate upon.
 *  @param rth Pointer to std_rt_head node containing a key. If this
 *             pointer is NULL the walker starts the walk from
 *             the first node. When non-NULL, it expects to find
 *             a key at the correct offset and the walk begins from
 *             the node with that key (or greater) onwards.
 *  @param walkcb User function to callback for each user node found
 *                on the tree. The user callback back function must
 *                return an int. If the return value is 0 the walker
 *                continues. If the return value from the callback is
 *                non-zero the walker terminates the walk on that node.
 *                The variable parameters passed to std_radix_walk will be
 *                passed into the callback routine as a va_list. In the
 *                callback you can use va_arg to extract the varible
 *                parameters (see stdarg.h). If no callback routine
 *                is provided (that is, walkcb is zero) then the walker
 *                still walks the tree as specified (number of counts etc.)
 *                without making an attempt to callback.
 *  @param cnt Number of node to visit on this walk. If you want to walk
 *             the rest of the tree from 'data' onwards then use
 *             a value of 0.
 *  @param min_ver Minimum version of the version range (inclusive).
 *  @param max_ver Maximum version of the version range (inclusive).
 *  @return Returns a pointer to the data node next to the one on
 *          which last callback was issued.
 */
std_rt_head * std_radix_versionwalk(std_rt_table *rtt, std_rt_head *rth,
                                    int (* walk_fn)(std_rt_head *, va_list ap), int cnt,
                                    std_radix_version_t min_ver, std_radix_version_t max_ver, ...);


/** Get the current version of a tree.
 *  Current version is always the max version.
 *  @param rtt Pointer to a radix tree to operate upon.
 *  @return The current version of the tree.
 */
#define std_radix_getversion(rtt) (rtt)->rtt_version


/** Get the version on the std_rt_head node.
 *  This is the version of the user node on the tree.
 *  @param rth Pointer to a std_rt_head to operate upon.
 *  @return The version of the std_rt_head.
 */
#define std_radix_getrthversion(rth) (rth)->rth_version


/** Get the version of a user node.
 *  Version a particular user node (std_rt_head) is returned.
 *  @param rth Pointer to a user node (std_rt_head *) to operate upon.
 *  @return The version of the node.
 */
#define std_radix_getnodeversion(rth) (rth)->rth_version


/** Clear the version of a tree.
 *  Current version is set to zero. The tree must be empty when
 *  this call is made. If there are nodes on the tree, their
 *  version is not modified by this call, only the tree version
 *  is set to zero.
 *  @param rtt Pointer to a radix tree to operate upon.
 *  @return Nothing.
 */
#define std_radix_clearversion(rtt) (rtt)->rtt_version = 0;


/** Get a less specific node.
 *  Gets a rth whose address is less specific the the address
 *  of the given rth. Can possibly return the default route as well.
 *  @param rtt  Pointer to a radix tree to operate upon.
 *  @param rth Pointer to the node whose less specific node is required.
 *  @return Pointer to a node whose address is less specific or NULL.
 */
std_rt_head * std_radix_getlessspecific(std_rt_table *rtt, std_rt_head *rth);


/** Macros for power walking a std_radix_tree.
 *  Two macros are provided for START and END of a power walk.
 *  This provides a very optimized method of walking the tree
 *  without the frills in the other walker routines. The node
 *  processing must be sandwitched between the START and the END
 *  macro. To break the walk use 'break' keyword in 'C'. One
 *  MUST NOT use 'continue' to go to the next node. The 'continue'
 *  may be used within the context of another loop.
 *  This can also be used to walk a sub-tree under a given root
 *  node.  Nodes must not be added or deleted by the node
 *  processing code between the macros.
 *  Threaded code CANNOT yeild between these macros.
 *
 *  @param rtt  Pointer to a radix tree to operate upon.
 *  @param rootaddr Root of the subtree to walk. A value of 0
 *                  means the entire tree is walked.
 *  @param rootlen Prefix length of the root.
 *  @param nextaddr Address of the node within the specified
 *                  subtree to start the walk from. A value of 0
 *                  will make the walker visit the first node
 *                  in the subtree.
 *  @param nextlen Prefix length of nextaddr.
 *  @param rth User pointer will be set to the std_rt_head of the
 *             node visited.
 *  @return Nothing.
 *  @see std_radix_powerwalkend
 */
#define std_radix_powerwalkstart(rtt, rootaddr, rootlen, nextaddr, nextlen, rth, type) \
{ \
    rt_node * _std_radix_getsubtree(std_rt_table *tt, u_char *addr, ushort bitlen); \
    rt_node *tt_rtn, *t_rtn = (rt_node *)0; \
    rt_node *root = NULL; \
    std_rt_head *t_rth; \
    int insubtree = false; \
    if ((rootaddr)) { \
        root = _std_radix_getsubtree((rtt), (rootaddr), (rootlen)); \
    } else { \
        root = (rtt)->rtt_root; \
    } \
    if ((nextaddr)) { \
        if ((t_rth = std_radix_getexact((rtt), (nextaddr), (nextlen)))) \
            t_rtn = t_rth->rth_rtn; \
        else if ((t_rth = std_radix_getnext((rtt), (nextaddr), (nextlen)))) \
            t_rtn = t_rth->rth_rtn; \
    } else { \
        t_rtn = root; \
    } \
    tt_rtn = t_rtn; \
    while(tt_rtn) { \
        if (tt_rtn == root) { \
            insubtree = true; \
            break; \
        } \
        tt_rtn = tt_rtn->rtn_parent; \
    } \
    if (root && t_rtn && (insubtree == true)) \
    { \
        do { \
            if (t_rtn->rtn_rth && \
                !RDX_TEST_BIT(t_rtn->rtn_flags, RDX_RN_DELE_BIT)) { \
                (rth) = (type)(void *)(t_rtn->rtn_rth);


/** End macro for power walking Radix tree.
 *  This macro must be placed after the node processing
 *  code following the corresponding start macro.
 *  @see std_radix_powerwalkstart
 *  @return None.
 */
#define std_radix_powerwalkend() \
            } \
            if (t_rtn->rtn_left) { \
                t_rtn = t_rtn->rtn_left; \
            } else { \
                if (t_rtn->rtn_right) { \
                    t_rtn = t_rtn->rtn_right; \
                } else { \
                    while (t_rtn != root && t_rtn->rtn_parent) { \
                        if ((t_rtn->rtn_parent->rtn_right == t_rtn) \
                            || !(t_rtn->rtn_parent->rtn_right)) { \
                            t_rtn = t_rtn->rtn_parent; \
                            if (t_rtn == root) \
                                break; \
                        } else { \
                            t_rtn = t_rtn->rtn_parent->rtn_right; \
                            break; \
                        } \
                    } \
                } \
            } \
        } while (t_rtn != root); \
    } \
}

#define std_radix_enable_radical(rttxx)    (rttxx)->rtt_radicalused = true;

#define std_radix_disable_radical(rttxx)    (rttxx)->rtt_radicalused = false;

#endif /* _RADIX_H_ */
