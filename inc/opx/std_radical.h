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
 * filename: std_radical.h
 */

/*!
 * \file   std_radical.h
 * \brief  Radical Change List implementation.
 * \date   05-2014
 */

#ifndef _RADICAL_H_
#define _RADICAL_H_

/*---------------------------------------------------------------*\
 *                    Includes.
\*---------------------------------------------------------------*/

#include "std_radix.h"

/*---------------------------------------------------------------*\
 *                    Defines and Macros.
\*---------------------------------------------------------------*/

#define RDCL_INCL   1
#define RDCL_DUMMY  2

#define RDCL_ISDUMMY(rdcl)  ((rdcl)->rdcl_flags & RDCL_DUMMY)
#define RDCL_SETDUMMY(rdcl) ((rdcl)->rdcl_flags |= RDCL_DUMMY)
#define RDCL_CLRDUMMY(rdcl) ((rdcl)->rdcl_flags &= ~RDCL_DUMMY)

#define RDCL_DLLOFFSET      ((size_t)(&((std_radical_head_t *)0)->rdcl_cl))

#define RDCL_HEADFROMDLL(rdcl) (std_radical_head_t *)((rdcl) ? (((char *)(rdcl)) - RDCL_DLLOFFSET) : 0)

/*---------------------------------------------------------------*\
 *                    Data structures.
\*---------------------------------------------------------------*/

/**
 *  Radix tree header.
 *  All user data structure that need to go on the tree MUST
 *  contain this structure as the first element.
 */
struct radical_head {
    /// Radix tree head.
    RT_HEAD;

    /// Radical flag for change-list maintaince.
    int rdcl_flags;

    /// DLL for changelist.
    std_dll rdcl_cl;
};

/// Typedef for RadicalHead structure.
typedef struct radical_head std_radical_head_t;

typedef std_radical_head_t std_radical_ref_t;


/*---------------------------------------------------------------*\
 *                    Prototypes with documentation.
\*---------------------------------------------------------------*/

/** Instantiate a Radical walk.
 *  This place the marker node in the front of the changelist.
 *  Routine may be called any time to reset a walk to the start.
 *  @param rtt Pointer to a Radical tree to operate upon.
 *  @param dummy Marker node for this instance of the walk.
 *  @return Nothing.
 */
void std_radical_walkconstructor(std_rt_table *rtt, std_radical_ref_t *dummy);


/** Destruct an instantiation of a radical walk.
 *  This routine end an instant of Radical walk. The marker node
 *  is no more useable for walking the changelist.
 *  @param rtt Pointer to a Radical tree to operate upon.
 *  @param dummy Marker node for this instance of the walk.
 *  @return Nothing.
 */
void std_radical_walkdestructor(std_rt_table *rtt, std_radical_ref_t *dummy);


/** Adding a node to the changelist.
 *  The node on the Radix tree is appended to the changelist.
 *  It upto the user to identify the condition what constitues
 *  a change that engenders an append to the changelist.
 *  If node is already on the changelist, then it removed and appended.
 *  Each append to the changelist is identified by a version number
 *  that represents the change.
 *  @param rtt Pointer to a Radical tree to operate upon.
 *  @param rth Pointer to the Radix node.
 *  @return Version number of this change.
 */
std_radix_version_t std_radical_appendtochangelist(std_rt_table *rtt, std_radical_head_t *rth);


/** To walk the changelist.
 *  Walks the changelist from the marker node onwards. The user callback
 *  routine is invoked for every node visited.
 *  @param rtt Pointer to a Radical tree to operate upon.
 *  @param dummy Marker node for this instance of the walk.
 *  @param walk_fn User function to callback for each user node found
 *                on the changelist. The user callback back function must
 *                return an int. If the return value is 0 the walker
 *                continues. If the return value from the callback is
 *                non-zero the walker terminates the walk on that node.
 *                A negative return value will make the node to be
 *                revisited on the next walk, and positive return
 *                value will start at the next node on a subsequent walk.
 *                The variable parameters passed to std_radical_walkchangelist
 *                will be passed into the callback routine as a va_list. In the
 *                callback you can use va_arg to extract the varible
 *                parameters (see stdarg.h).
 *  @param cnt Maximum number of node to be processed between thread yield.
 *             A value of 0 is an indication not to yield.
 *  @param maxnodes Maximum number of nodes to be processed in this walk.
 *                  The walk terminates after this many node are processed,
 *                  where a value of 0 is considered 0xffffffff.
 *  @param max_ver Indicates to walk the changelist till nodes have version
 *                 number less than this value.
 *  @param cbretval Pointer to an int where the callback return value is placed.
 *  @return Nothing.
 */
void std_radical_walkchangelist(std_rt_table *rtt, std_radical_ref_t *dummy,
    int (* walk_fn)(std_radical_head_t *, va_list), int cnt, int maxnodes,
    std_radix_version_t max_ver, int *cbretval, ...);


/** Instantiate a radical walk beyond a certain node.
 *  Places the marker node after a given node so the walk
 *  start for changes after that node.
 *  @param rtt Pointer to a Radical tree to operate upon.
 *  @param after Marker node of another instance of walk to which
 *               dummy marked will be aligned.
 *  @param dummy Marker node for this instance of the walk.
 *  @return Nothing.
 */
void std_radical_walkconstructorafter(std_rt_table *rtt, std_radical_ref_t *after,
                                      std_radical_ref_t *dummy);


/** Version of the next node on the changelist.
 *  Returns the version of the next node on the changelist from the
 *  marker node. However, the marker node is not moved.
 *  @param rtt Pointer to a Radical tree to operate upon.
 *  @param dummy Marker node for a instance of the walk.
 *  @return Version number of the next node on changelist.
 */
std_radix_version_t std_radical_nextversion(std_rt_table *rtt, std_radical_ref_t *dummy);


/** First node on the changelist.
 *  Returns the pointer first node in the changelist.
 *  @param rtt Pointer to a Radical tree to operate upon.
 *  @return Returns the pointer first node in the changelist or NULL
 *          if empty changelist.
 */
std_radical_head_t * std_radical_getfirst(std_rt_table *rtt);


/** Next node on the changelist.
 *  Returns the pointer next node in the changelist.
 *  @param rtt Pointer to a Radical tree to operate upon.
 *  @param rth Reference node to which next node is sought.
 *  @return Returns pointer to next node on changelist or NULL.
 */
std_radical_head_t * std_radical_getnext(std_rt_table *rtt, std_radical_head_t *rth);

#endif /* _RADICAL_H_ */
