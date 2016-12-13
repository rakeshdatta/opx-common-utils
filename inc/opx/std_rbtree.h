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
 * filename: std_rbtree.h
 */

/*!
 * \file   std_rbtree.h
 * \brief  Red-Black tree implementation.
 * \date   05-2014
 */

#ifndef _RBTREE_H_
#define _RBTREE_H_

/*---------------------------------------------------------------*\
 *                    Includes.
\*---------------------------------------------------------------*/

#include <sys/types.h>
#include <stdarg.h>
#include "std_error_codes.h"

/*---------------------------------------------------------------*\
 *                    Defines and Macros.
\*---------------------------------------------------------------*/

/// Max length of name of tree
#define RBT_NAME_MAX_LEN 50

/// Color of the RBT tree node.
enum { RBT_BLACK, RBT_RED };

/// Flag value for selecting an inorder tree walk.
#define RBT_INORDERWALK    (0 << 0)
/// Flag value for selecting an preorder tree walk.
#define RBT_PREORDERWALK    (1 << 0)

/// Compare function for unsigned long keys.
#define RBT_ULONG_KEY    _std_rbtree_compare_ul
/// Compare function for integer keys.
#define RBT_INT_KEY    _std_rbtree_compare_i

/*---------------------------------------------------------------*\
 *                    Data structures.
\*---------------------------------------------------------------*/

/**
 *  Basic RBT tree node. Notice that the tree node doesn't
 *  maintain the key value within the internal node. The key is
 *  assumed to be in the user node off the rbt_data pointer.
 */
struct _std_rbtree_node
{
    /// Left child.
    struct _std_rbtree_node *rbt_left;

    /// Right child.
    struct _std_rbtree_node *rbt_right;

    /// Parent node.
    struct _std_rbtree_node *rbt_parent;

    /// Color of this node: RED or BLACK.
    u_char rbt_color;

    /// Height of the node: used only for printing tree
    u_char rbt_height;

    /// Client data (with key).
    void *rbt_data;
};

/// Typedef for struct _std_rbtree_node
typedef struct _std_rbtree_node std_rbtree_node;


/**
 *  Top level structure for a RBT tree. This maintains tree
 *  related information for each instance of a RBT tree
 *  created by a user. Once a tree is instantiated, a pointer
 *  to this structure is return to the user for all future
 *  operations on the tree.
 */
struct _std_rbtree_table
{
    /// To verify user is providing a 'good' RBT tree.
    u_long rbtt_magic;

    /// Name of this tree.
    char rbtt_name[RBT_NAME_MAX_LEN+1];

    /// Offset to the key in user node.
    int rbtt_keyoffset;

    /// Length of the key (in bytes) in user node.
    int rbtt_keylength;

    /// Root of this tree.
    struct _std_rbtree_node *rbtt_root;

    /// Debug enable/disable; default is enabled. Only one debug level for now.
    int rbtt_debug;

    /// Callback to check data1 is less than data2.
    int (*rbtt_compare)(struct _std_rbtree_table * rbtt, void *data1, void *data2);

    /// Client provided malloc routine.
    void *(* rbtt_malloc)(size_t size);

    /// Client  provided free routine.
    void (* rbtt_free)(void *);

    /// Number of internal RBT nodes.
    u_long rbtt_numinodes;

    /// Number nodes inserted by user.
    u_long rbtt_numinserts;

    /// Number nodes removed by user.
    u_long rbtt_numremoved;

    /// Number of mallocs called by RBT.
    u_long rbtt_nummallocs;

    /// Number of frees called by RBT.
    u_long rbtt_numfrees;

    /// NIL node for this RBT tree.
    struct _std_rbtree_node nil;
};

/// Typedef for struct _std_rbtree_table.
typedef struct _std_rbtree_table std_rbtree_table;

/// Typedef for handle to return to user on instantiation.
typedef struct _std_rbtree_table * rbtree_handle;


/*---------------------------------------------------------------*\
 *                    Prototypes with documentation.
\*---------------------------------------------------------------*/

/**
 * @brief compare a unsigned long field in the tree during a search (
 * @param rbtt the tree data structure
 * @param one the left hand side of the compare
 * @param two the right hand side of the compare
 * @return -1 0 or 1 if less equal or greater
 */
extern int _std_rbtree_compare_ul(rbtree_handle rbtt, void *one, void *two);

/**
 * @brief compare a int field in the tree during a search (
 * @param rbtt the tree data structure
 * @param one the left hand side of the compare
 * @param two the right hand side of the compare
 * @return -1 0 or 1 if less equal or greater
 */
extern int _std_rbtree_compare_i(rbtree_handle rbtt, void *one, void *two);


/**@name RBT HAPI Calls
 * A series of high level API calls are provided to manipulate a RBT tree.
 */
//@{

/**
 *  Instantiate a generic RBT tree. This implementation of RBT can operate on
 *  any arbitrary keys. If you have a key that can't be accomadated within
 *  ULONG then talk to the owner of this module.
 *
 *  Two classes of routines are provided to the users of RBT. The first
 *  and recommended set are the calls that start with std_rbtree prefix. The
 *  second set of calls start with std_rbtree prefix. The main difference
 *  being that the former hides the RNT node
 *  entirely from the user. That is the user is never required to either
 *  allocate or free RBT node. The user is required to maintain the
 *  user node that hangs off the RBT node. The key is contained entirely
 *  within the user node. No key is maintained within the RBT node. The
 *  access to the key is provided by giving the offset information at
 *  the time of init. So it is extremely important that the user nodes
 *  have proper allocation and key positioned at the specified offset.
 *  Otherwise, RBT tree will not work and can cause crash.
 *
 *  @param rbtt_name Pointer to character string for name of this tree.
 *  @param keyoffset Offset in number of bytes from the start of the
 *                   user node at which key is located.
 *  @param keylength Length of the key in bytes. This parameter ignored if the
 *                   compare function is one of RBT_ULONG_KEY or RBT_INT_KEY.
 *                   The only purpose of taking this parameter is allow
 *                   the RBT tester routines to be able to attach to any
 *                   user RBT for debugging. Note that the tester debugging
 *                   will work only when the key is contigous. That is,
 *                   RBT as such support non-contigous keys, but the tester
 *                   may not be used for debugging assistance.
 *  @param rbtt_malloc User provided malloc routine of RBT internal node
 *                     allocation. If this parameter is NULL, then malloc
 *                     from libc is used.
 *  @param rbtt_free User provided free routine for freeing RBT internal
 *                   node. If this parameter is NULL then free from libc
 *                   is used.
 *  @param rbtt_compare User is provided with two predefined compare
 *                      functions: RBT_ULONG_KEY for unsigned long
 *                      keys and RBT_INT_KEY for integer keys.
 *                      Alternatively user may choose to provide his own
 *                      compare function. In this case, the compare must
 *                      return a value of -1 if the first data node key
 *                      is strictly less than the second data node key,
 *                      1 if the first data node key is strictly greater
 *                      than the second data node key, and 0 in case
 *                      when first data key equals second data key. Also
 *                      note that the first parameter to the compare
 *                      callback function is the offset provided to RBT
 *                      at instantiation.
 *  @return rbtree_handle - A handle to the instantiated tree. User must
 *          maintain this for all furture operations on the tree.
 *          NULL on failure.
 */
rbtree_handle std_rbtree_create(char *rbtt_name, int keyoffset, int keylength,
                                void *rbtt_malloc(size_t), void rbtt_free(void *),
                                int rbtt_compare(rbtree_handle rbtt, void *, void *));

/**
 * @brief create a simple RB tree using memcmp to compare the actual elements in the tree.
 * @param rbtt_name the string name of the tree
 * @param keyoffset - the offset of the structure to compare (can use std_struct_utils.h to methods to find this)
 * @param keylength - the length of the structure to compare (can use std_struct_utils.h to methods to find this)
 * @return
 */
rbtree_handle std_rbtree_create_simple(char *rbtt_name, int keyoffset, int keylength);

/**
 *  Destruct a RBT tree. After the call the rbtt tree handle
 *  is no good. User must ensure that no user nodes are
 *  still on the tree.
 *  @param rbtt Handle to RBT tree to operate upon.
 *  @return Returns nothing.
 */
void std_rbtree_destroy(rbtree_handle rbtt);


/**
 *  Insert a user node in a RBT tree. The user node must contain the
 *  key at the correct offset. Note that the user node need NOT contain
 *  the RBT glue information.
 *  @param rbtt Handle to a RBT tree to operate upon.
 *  @param data User node to be inserted into tree. Note that
 *             this space is NOT required to contain any RBT
 *             node structure. The ONLY thing required is that
 *             the key must be contained in the proper place for
 *             the comparision callback to work properly.
 *  @return Returns STD_ERR_OK or STD_ERR.
 */
t_std_error std_rbtree_insert(rbtree_handle rbtt, void *data);


/**
 *  Remove a user node from the tree. This function releases the
 *  RBT internal node. Note that the user node is only removed from
 *  the tree but is not freed by RBT.
 *  @param rbtt Handle to a RBT tree to operate upon.
 *  @param data Pointer to the data to be removed. The data
 *              should contain the proper key in the right
 *              place for comparision callback to work properly.
 *              The parameter data need not be the same as the
 *              user node on the tree.
 *  @return Pointer to the user node removed from the tree.
 */
void * std_rbtree_remove(rbtree_handle rbtt, void *data);


/**
 *  Get the first user node on the tree. First means the node
 *  that has the lowest key value.
 *  @param rbtt Handle to a RBT tree to operate upon.
 *  @return Pointer to the first data node. Returns NULL
 *          if tree is empty.
 */
void * std_rbtree_getfirst(rbtree_handle rbtt);


/**
 *  Find the exact user node in a RBT tree. Note that the user
 *  data node passed in this case may be a temporary space on
 *  stack, but it is required to have the key at the correct
 *  offset.
 *  @param rbtt Handle to a RBT tree to operate upon.
 *  @param data Pointer to a user node with the key for the exact
 *         match.
 *  @return Returns a pointer to the actual user node on success or
 *          NULL on failure.
 */
void * std_rbtree_getexact(rbtree_handle rbtt, void *data);


/**
 *  Find the exact user node or the next one inorder (successor).
 *  Note that the user node passed in this case may be a
 *  temporary space on stack, but it is required to have the key
 *  at the correct offset.
 *  @param rbtt Handle to a RBT tree to operate upon.
 *  @param data Pointer to a user node that contains the key at
 *              the right offset.
 *  @return Pointer to the user node that exactly matches the given
 *          key or the node next inorder. Otherwise returns NULL.
 */
void * std_rbtree_getexactornext(rbtree_handle rbtt, void *data);


/**
 *  Find the exact user node or the previous one inorder (predecessor).
 *  Note that the user node passed in this case may be a
 *  temporary space on stack, but it is required to have the key
 *  at the correct offset.
 *  @param rbtt Handle to a RBT tree to operate upon.
 *  @param data Pointer to user node that contains the key at
 *              the right offset.
 *  @return Pointer to the user node that exactly matches the given
 *          key or the previous node. Otherwise returns NULL.
 */
void * std_rbtree_getexactorprev(rbtree_handle rbtt, void *data);


/**
 *  Find strictly the next user node. Finds the node that is next
 *  inorder from the node that has the given key.
 *  Note that the user node passed in this case may be a
 *  temporary space on stack, but it is required to have the key
 *  at the correct offset.
 *  @param rbtt Handle to a RBT tree to operate upon.
 *  @param data Pointer to user node that contains the key at
 *              the right offset.
 *  @return Pointer to the next user node from the given key.
 *          Otherwise returns NULL.
 */
void * std_rbtree_getnext(rbtree_handle rbtt, void *data);


/**
 *  Walk the tree with inorder or preorder callbacks.
 *  User is assumed not to manipulate the tree during the callbacks.
 *  @param rbtt Handle to a RBT tree to operate upon.
 *  @param data Pointer to user node containing a key. If this
 *              pointer is NULL the walker starts the walk from
 *              the first node. When non-NULL, it expects to find
 *              a key at the correct offset and the walk begins from
 *              the node with that key (or greater) onwards.
 *  @param walkcb User function to callback for each user node found
 *                on the tree. The user callback back function must
 *                return an int. If the return value is 0 the walker
 *                continues. If the return value from the callback is
 *                non-zero the walker terminates the walk on that node.
 *                The variable parameters passed to std_rbtree_walk will be
 *                passed into the callback routine as a va_list. In the
 *                callback you can use va_arg to extract the varible
 *                parameters (see stdarg.h). If no callback routine
 *                is provided (that is, walkcb is zero) then the walker
 *                still walks the tree as specified (number of counts etc.)
 *                without making an attempt to callback.
 *  @param cnt Number of node to visit on this walk. If you want to walk
 *             the rest of the tree from 'data' onwards then use
 *             a value of 0.
 *  @param flag Specify RBT_INORDERWALK or RBT_PREORDERWALK.
 *  @return Returns a pointer to the data node next to the one on
 *          which last callback was issued.
 */
void * std_rbtree_walk(rbtree_handle rbtt, void *data,
                       int (* walkcb)(rbtree_handle rbtt, void *, va_list ap), int cnt,
                       int flag, ...);


/**
 *  Enable/disable debugging.
 *  User may enable or disble debugging/validation checks via this call.
 *  @param rbtt Handle to a RBT tree to operate upon.
 *  @param bool Boolean value of TRUE will enable debugging and a
 *              value of FALSE will disable debugging checks.
 *              Default setting is TRUE.
 *  @return Nothing.
 */
void std_rbtree_debug(rbtree_handle, int);



/**
 *  Insert a RBT node in a RBT tree. It assumed here
 *  that the RBT node is allocated by the user or was obtained
 *  by another underscore call to RBT. The data pointer in the
 *  RBT node is set to the appropriate user node with the key
 *  at the proper offset.
 *  @param rbtt Handle to a RBT tree.
 *  @param z Pointer to a RBT node with the rbt_data pointer
 *           properly assigned to a user node with the key.
 *  @return Returns z on success and 0 on failure.
 */
std_rbtree_node * _std_rbtree_insert(rbtree_handle rbtt, std_rbtree_node *z);


/**
 *  Remove a RBT node from the tree. This call does not
 *  free the RBT node.
 *  @param rbtt Handle to a RBT tree to operate upon.
 *  @param z Pointer to the RBT node to be removed. It is
 *           assumed that this RBT node was obtained from
 *           another underscore call, and is being requested
 *           to be removed here. Therefore, z MUST be a valid
 *           and in use RBT node.
 *  @return Nothing.
 */
void _std_rbtree_remove(rbtree_handle rbtt,  std_rbtree_node *z);


/**
 *  Get the first RBT node on the tree. First means the
 *  RBT node with user node that has the lowest key value.
 *  @param rbtt Handle to a RBT tree to operate upon.
 *  @return Pointer to the first RBT node in the tree.
 *          Returns 0 if tree is empty.
 */
std_rbtree_node * _std_rbtree_getfirst(rbtree_handle rbtt);


/**
 *  Find the exact RBT node in a RBT tree. Note that the
 *  data node passed in this case may be a temporary space on
 *  stack, but it is required to have the key at the correct
 *  offset.
 *  @param rbtt Handle to a RBT tree to operate upon.
 *  @param data Pointer to a user node with the key for the exact
 *         match.
 *  @return On success returns a pointer to the RBT node
 *          that has user node or 0 on failure.
 */
std_rbtree_node * _std_rbtree_getexact(rbtree_handle rbtt, void *data);


/**
 *  Find the exact user node or the next one inorder (successor).
 *  Note that the user node passed in this case may be a
 *  temporary space on stack, but it is required to have the key
 *  at the proper offset.
 *  @param rbtt Handle to a RBT tree to operate upon.
 *  @param data Pointer to user node that contains the key at
 *              the right offset.
 *  @return Pointer to the RBT node whose key in user node
 *          exactly matches the given key or the next node inorder.
 *          Otherwise returns 0.
 */
std_rbtree_node * _std_rbtree_getexactornext(rbtree_handle rbtt, void *data);


/**
 *  Find the exact user node or the previous one inorder (predecessor).
 *  Note that the user node passed in this case may be a
 *  temporary space on stack, but it is required to have the key
 *  at the proper offset.
 *  @param rbtt Handle to a RBT tree to operate upon.
 *  @param data Pointer to user node that contains the key at
 *              the right offset.
 *  @return Pointer to the RBT node whose key in user node
 *          exactly matches the given key or the previous node inorder.
 *          Otherwise returns 0.
 */
std_rbtree_node * _std_rbtree_getexactorprev(rbtree_handle rbtt, void *data);


/**
 *  Find strictly the next RBT tree node from the given node.
 *  Note that the pointer to a internal node must have been
 *  returned from one of the other std_rbtree calls.
 *  @param rbtt Handle to a RBT tree to operate upon.
 *  @param x Pointer to valid (pointer return from some other
 *           std_rbtree call) RBT tree node whose next is to
 *           be found.
 *  @return Pointer to the next RBT tree node. Otherwise returns 0.
 */
std_rbtree_node * _std_rbtree_getnext(rbtree_handle rbtt, std_rbtree_node *x);


/**
 *  Walk the tree with inorder or preorder callbacks.
 *  User is assumed not to manipulate the tree during the callbacks.
 *  @param rbtt Handle to a RBT tree to operate upon.
 *  @param x Pointer to a RBT node from where the walk should
 *           start walking (x is inclusive). x must have been returned by
 *           one of the LAPI calls.
 *  @param walkcb User function to callback for each user node found
 *                on the tree. The user callback back function must
 *                return an int. If the return value is 0 the walker
 *                continues. If the return value from the calback is
 *                non-zero the walker terminates the walk on that node.
 *                The variable parameters passed to std_rbtree_walk will be
 *                passed into the callback routine as a va_list. In the
 *                callback you can use va_arg to extract the varible
 *                parameters (see stdarg.h). If no callback routine
 *                is provided (that is, walkcb is zero) then the walker
 *                still walks the tree as specified (number of counts etc.)
 *                without making an attempt to callback.
 *  @param cnt Number of node to visit on this walk. If you want to walk
 *             the rest of the tree from 'x' onwards then use
 *             a value of 0 here.
 *  @param flag Specify RBT_INORDERWALK or RBT_PREORDERWALK.
 *  @return Return the pointer to the RBT node next to the one on
 *          which last callback was issued.
 */
std_rbtree_node * _std_rbtree_walk(rbtree_handle rbtt, std_rbtree_node *x,
                                   int (* walkcb)(rbtree_handle rbtt, void *, va_list ap),
                                   int cnt, int flag, va_list ap);

/**
 * @brief a generic compare function for the RB tree. Used by the simple tree implementation
 * @param rbtt the tree structure
 * @param lhs - left hand side of the compare
 * @param rhs - right hand side of the compare
 * @return -1 0 1 for less, equal or greater comparison results
 */
int std_rbtree_gen_cmp(rbtree_handle rbtt, void *lhs, void *rhs);


#endif /* _RBTREE_H_ */
