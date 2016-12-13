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
 * filename: std_radical.c
 */

/*!
 * \file   std_radical.c
 * \brief  RADIx tree with ChAnge List (RADICAL)
 * \date   05-2014
 */

/*---------------------------------------------------------------*\
 *                    Includes.
\*---------------------------------------------------------------*/

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "std_radix.h"
#include "std_radical.h"


/*---------------------------------------------------------------*\
 *            Public methods
\*---------------------------------------------------------------*/


std_radix_version_t std_radical_procverwrap(std_rt_table *rtt)
{
    std_radical_head_t *rth;
    std_radix_version_t rth_version = 0;

    // Go thru the change list and paint the new versions
    // for dummy and active nodes.
    //---------------------------------------------------
    rth = RDCL_HEADFROMDLL(std_dll_getfirst(&rtt->rtt_clhead));
    while (rth)
    {
        if (RDCL_ISDUMMY(rth))
            rth->rth_version = rth_version;
        else
            rth->rth_version = ++rth_version;

        rth = RDCL_HEADFROMDLL(std_dll_getnext(&rtt->rtt_clhead, &rth->rdcl_cl));
    }

    return rth_version;
}


void std_radical_walkconstructor(std_rt_table *rtt, std_radical_ref_t *dummy)
{
    assert(dummy);

    if (RDCL_ISDUMMY(dummy) && std_dll_islinked(&dummy->rdcl_cl))
        std_dll_remove(&rtt->rtt_clhead, &dummy->rdcl_cl);

    RDCL_SETDUMMY(dummy);
    std_dll_insertatfront(&rtt->rtt_clhead, &dummy->rdcl_cl);

    dummy->rth_version = 0;
    rtt->rtt_radicalinuse++;

    return;
}


void std_radical_walkconstructorafter(std_rt_table *rtt, std_radical_ref_t *after,
                                      std_radical_ref_t *dummy)
{
    assert(dummy);
    assert(after);
    assert(RDCL_ISDUMMY(after));
    assert(std_dll_islinked(&after->rdcl_cl));

    if (std_dll_islinked(&dummy->rdcl_cl))
        std_dll_remove(&rtt->rtt_clhead, &dummy->rdcl_cl);

    RDCL_SETDUMMY(dummy);
    std_dll_insertafter(&rtt->rtt_clhead, &after->rdcl_cl, &dummy->rdcl_cl);

    dummy->rth_version = after->rth_version;
    rtt->rtt_radicalinuse++;

    return;
}


void std_radical_walkdestructor(std_rt_table *rtt, std_radical_ref_t *dummy)
{
    assert(dummy);

    RDCL_CLRDUMMY(dummy);
    if (std_dll_islinked(&dummy->rdcl_cl))
        std_dll_remove(&rtt->rtt_clhead, &dummy->rdcl_cl);
    rtt->rtt_radicalinuse--;
}


std_radix_version_t std_radical_appendtochangelist(std_rt_table *rtt, std_radical_head_t *rth)
{
    assert(rth->rth_rtn);

    if (rth->rdcl_flags & RDCL_INCL)
    {
        assert(std_dll_islinked(&rth->rdcl_cl));
        std_dll_remove(&rtt->rtt_clhead, &rth->rdcl_cl);
    }

    std_dll_insertatback(&rtt->rtt_clhead, &rth->rdcl_cl);
    rth->rdcl_flags |= RDCL_INCL;

    rth->rth_version = ++rtt->rtt_version;

    if (!rth->rth_version)
      rtt->rtt_nwraps++;

    return rth->rth_version;
}


std_radical_head_t * std_radical_getfirst(std_rt_table *rtt)
{
    std_radical_head_t *rth;

    rth = RDCL_HEADFROMDLL(std_dll_getfirst(&rtt->rtt_clhead));
    while (rth)
    {
        if (RDCL_ISDUMMY(rth))
            rth = RDCL_HEADFROMDLL(std_dll_getnext(&rtt->rtt_clhead,
                      &rth->rdcl_cl));
        else
            break;
    }

    return rth;
}


std_radical_head_t * std_radical_getnext(std_rt_table *rtt, std_radical_head_t *rth)
{
    std_radical_head_t *t_rth;

    t_rth = RDCL_HEADFROMDLL(std_dll_getnext(&rtt->rtt_clhead, &rth->rdcl_cl));
    while (t_rth)
    {
        if (RDCL_ISDUMMY(t_rth))
            t_rth = RDCL_HEADFROMDLL(std_dll_getnext(&rtt->rtt_clhead,
                      &t_rth->rdcl_cl));
        else
            break;
    }

    return t_rth;
}


std_radix_version_t std_radical_nextversion(std_rt_table *rtt, std_radical_ref_t *dummy)
{
    std_radical_head_t *rth;

    assert(RDCL_ISDUMMY(dummy));
    assert(std_dll_islinked(&dummy->rdcl_cl));

    rth = dummy;
    do {
        if (RDCL_ISDUMMY(rth))
            rth = RDCL_HEADFROMDLL(std_dll_getnext(&rtt->rtt_clhead, &rth->rdcl_cl));
        else
            return rth->rth_version;
    } while (rth);

    return (std_radix_version_t)0;
}


void std_radical_walkchangelist(std_rt_table *rtt, std_radical_ref_t *dummy,
                                int (* walk_fn)(std_radical_head_t *, va_list), int cnt,
                                int maxnodes, std_radix_version_t max_ver, int *cbretval, ...)
{
    va_list ap;
    u_long lcnt;
    std_radical_head_t *t_rth;
    std_radix_version_t rth_version;
    char setMaxVer = 0;

    assert(dummy);
    assert(cbretval);
    assert(walk_fn);
    assert(RDCL_ISDUMMY(dummy));
    assert(std_dll_islinked(&dummy->rdcl_cl));

    if (!(lcnt = cnt))
        lcnt = 0xffffffff;
    if (!maxnodes)
        maxnodes = 0xffffffff;
    *cbretval = 0;

    t_rth = RDCL_HEADFROMDLL(std_dll_getnext(&rtt->rtt_clhead, &dummy->rdcl_cl));
    std_dll_remove(&rtt->rtt_clhead, &dummy->rdcl_cl);
    while (t_rth)
    {
        // Skip the dummies :-)
        //--------------------------------
        if (RDCL_ISDUMMY(t_rth))
        {
            t_rth = RDCL_HEADFROMDLL(std_dll_getnext(&rtt->rtt_clhead, &t_rth->rdcl_cl));
            continue;
        }

        // If the node we are about to process has version number
        // grater then what the user gave then return leaving the
        // marker in front of this node.
        //-------------------------------------------------------------
        if (t_rth->rth_version > max_ver)
    {
        setMaxVer = 1;
            break;
    }

        // Call user callback and pass the variable parameter list.
        // Insert the dummy node after the current node to be
        // processed. This is because the callback routine may choose
        // to delete the current node. We will use the dummy node
        // as a reference to go to next node indpendent of what
        // happens.
        //-------------------------------------------------------------
        rth_version = t_rth->rth_version; // save in tmp
        va_start(ap, cbretval);
        std_dll_insertafter(&rtt->rtt_clhead, &t_rth->rdcl_cl, &dummy->rdcl_cl);
        *cbretval = walk_fn(t_rth, ap);
        va_end(ap);
        lcnt--;

        // Save the version number of the last node
        // processed in the marker node.
        //-------------------------------------------
        if (*cbretval >= 0)
        {
            dummy->rth_version = rth_version;
        }

        // If callback returns a non-zero value,
        // its an indication to break the walk.
        //-------------------------------------------
        if (*cbretval)
        {
            // By default we skip over the last processed node.
            // But if user requested to revisit the last processed
            // node, then we need to reinstert dummy node before
            // the last processed node.
            //----------------------------------------------------------
            if (*cbretval < 0)
            {
                assert(std_dll_islinked(&t_rth->rdcl_cl));
                std_dll_remove(&rtt->rtt_clhead, &dummy->rdcl_cl);
                std_dll_insertbefore(&rtt->rtt_clhead, &t_rth->rdcl_cl,
                    &dummy->rdcl_cl);
            }

            return;
        }

        // If we have processed maximum number of nodes
        // specified, then return leaving the marker at the
        // right spot, that is right where it is now.
        //-----------------------------------------------------------
        if (!(--maxnodes))
            return;

        // Next node to be processed is after the dummy node.
        //-----------------------------------------------------
        t_rth = RDCL_HEADFROMDLL(std_dll_getnext(&rtt->rtt_clhead, &dummy->rdcl_cl));
        std_dll_remove(&rtt->rtt_clhead, &dummy->rdcl_cl);
    }

    if (!t_rth)
    {
        dummy->rth_version = rtt->rtt_version;
        std_dll_insertatback(&rtt->rtt_clhead, &dummy->rdcl_cl);
    }
    else
    {
    //  If we reached here because max_ver has been reached, set
    //  the dummy's version to max_ver. NOTE: We will arrive here
    //  when the next node to be processed has a version > max_ver.
    //  We may not have actually processed a node with max_ver, since,
    //  the last processed node could have any version <= max_ver.
    //  - Aamod
    if (setMaxVer)
    {
            dummy->rth_version = max_ver;
    }
        std_dll_insertbefore(&rtt->rtt_clhead, &t_rth->rdcl_cl, &dummy->rdcl_cl);
    }
}
