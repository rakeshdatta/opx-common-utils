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
 * filename: std_rbtree.c
 */

/*!
 * \file   std_rbtree.c
 * \brief  Red-Black tree implementation.
 * \date   05-2014
 */


/*---------------------------------------------------------------*\
 *                    Includes.
\*---------------------------------------------------------------*/

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "assert.h"
#include "std_rbtree.h"


/*---------------------------------------------------------------*\
 *                    Defines and Macros.
\*---------------------------------------------------------------*/

#define RBT_DEBUG    1
#define TEST_RBTREE

#define RBT_ASSERT    assert
#define RBT_MALLOC    malloc
#define RBT_FREE    free
#define RBT_MAGIC    0xdeadbeef
#define NIL(rbtt)    &(rbtt)->nil

#define RBT_WALKDOWN    1
#define RBT_WALKUP    2
#define RBT_WALKRIGHT    3

#define TRUE            1
#define FALSE           0

#define RBT_IS_LESS(rbtt, d1, d2) ((rbtt)->rbtt_compare((rbtt), (d1), (d2)) < 0)
#define RBT_IS_EQUAL(rbtt, d1, d2) ((rbtt)->rbtt_compare((rbtt), (d1), (d2)) == 0)


#define RBT_VALIDATE_HANDLE(rbtt) \
            RBT_ASSERT(rbtt); \
            RBT_ASSERT(rbtt->rbtt_magic == RBT_MAGIC);

#define RBT_DEBUG_START(rbtt) \
            if (RBT_DEBUG) { \
                RBT_VALIDATE_HANDLE(rbtt); \
                if (rbtt->rbtt_debug == TRUE) {\

#define RBT_DEBUG_END      } }

/*---------------------------------------------------------------*\
 *            Private methods
\*---------------------------------------------------------------*/


static void * std_rbtree_malloc(size_t size)
{
    return malloc(size);
}

static void std_rbtree_free(void *ptr)
{
    free(ptr);
    return;
}

static void std_rbtree_rotateleft(std_rbtree_table *rbtt, std_rbtree_node *x)
{
    std_rbtree_node *y;

    y = x->rbt_right;
    x->rbt_right = y->rbt_left;
    if (y->rbt_left != NIL(rbtt))
        y->rbt_left->rbt_parent = x;

    y->rbt_parent = x->rbt_parent;
    if (x->rbt_parent != NIL(rbtt))
    {
        if (x == x->rbt_parent->rbt_left)
            x->rbt_parent->rbt_left = y;
        else
            x->rbt_parent->rbt_right = y;
    }
    else
    {
        rbtt->rbtt_root = y;
    }

    y->rbt_left = x;
    x->rbt_parent = y;

} // std_rbtree_rotateleft()


static void std_rbtree_rotateright(std_rbtree_table *rbtt, std_rbtree_node *x)
{
    std_rbtree_node *y;

    y = x->rbt_left;
    x->rbt_left = y->rbt_right;
    if (y->rbt_right != NIL(rbtt))
        y->rbt_right->rbt_parent = x;

    y->rbt_parent = x->rbt_parent;
    if (x->rbt_parent != NIL(rbtt))
    {
        if (x == x->rbt_parent->rbt_right)
            x->rbt_parent->rbt_right = y;
        else
            x->rbt_parent->rbt_left = y;
    }
    else
    {
        rbtt->rbtt_root = y;
    }

    y->rbt_right = x;
    x->rbt_parent = y;

} // std_rbtree_rotateright()


static void std_rbtree_balanceoninsert(std_rbtree_table *rbtt, std_rbtree_node *x)
{
    std_rbtree_node *y;

    x->rbt_color = RBT_RED;
    while (x != rbtt->rbtt_root && x->rbt_parent->rbt_color == RBT_RED)
    {
        if (x->rbt_parent == x->rbt_parent->rbt_parent->rbt_left)
        {
            y = x->rbt_parent->rbt_parent->rbt_right;
            if (y != NIL(rbtt) && y->rbt_color == RBT_RED)
            {
                /* uncle is RED */
                x->rbt_parent->rbt_color = RBT_BLACK;
                y->rbt_color = RBT_BLACK;
                x->rbt_parent->rbt_parent->rbt_color = RBT_RED;
                x = x->rbt_parent->rbt_parent;
            }
            else
            {
                /* uncle is BLACK */
                if (x == x->rbt_parent->rbt_right)
                {
                    /* make x a rbt_left child */
                    x = x->rbt_parent;
                    std_rbtree_rotateleft(rbtt, x);
                }

                /* rerbt_color and rotate */
                x->rbt_parent->rbt_color = RBT_BLACK;
                x->rbt_parent->rbt_parent->rbt_color = RBT_RED;
                std_rbtree_rotateright(rbtt, x->rbt_parent->rbt_parent);
            }
        }
        else
        {
            /* mirror image of above code */
            y = x->rbt_parent->rbt_parent->rbt_left;
            if (y != NIL(rbtt) && y->rbt_color == RBT_RED)
            {
                /* uncle is RED */
                x->rbt_parent->rbt_color = RBT_BLACK;
                y->rbt_color = RBT_BLACK;
                x->rbt_parent->rbt_parent->rbt_color = RBT_RED;
                x = x->rbt_parent->rbt_parent;
            }
            else
            {
                /* uncle is BLACK */
                if (x == x->rbt_parent->rbt_left)
                {
                    x = x->rbt_parent;
                    std_rbtree_rotateright(rbtt, x);
                }
                x->rbt_parent->rbt_color = RBT_BLACK;
                x->rbt_parent->rbt_parent->rbt_color = RBT_RED;
                std_rbtree_rotateleft(rbtt, x->rbt_parent->rbt_parent);
            }
        }
    }

    rbtt->rbtt_root->rbt_color = RBT_BLACK;

} // std_rbtree_balanceoninsert()


static void std_rbtree_balanceonremove(rbtree_handle rbtt, std_rbtree_node *x)
{
    std_rbtree_node *w;

    while (x != rbtt->rbtt_root && x->rbt_color == RBT_BLACK)
    {
        if (x == x->rbt_parent->rbt_left)
        {
            w = x->rbt_parent->rbt_right;

            if (w->rbt_color == RBT_RED)
            {
                w->rbt_color = RBT_BLACK;
                x->rbt_parent->rbt_color = RBT_RED;
                std_rbtree_rotateleft(rbtt, x->rbt_parent);
                w = x->rbt_parent->rbt_right;
            }

            if (w == NIL(rbtt))
                break;

            if (w->rbt_left->rbt_color == RBT_BLACK && w->rbt_right->rbt_color == RBT_BLACK)
            {
                w->rbt_color = RBT_RED;
                x = x->rbt_parent;
            }
            else
            {
                if (w->rbt_right->rbt_color == RBT_BLACK)
                {
                    w->rbt_left->rbt_color = RBT_BLACK;
                    w->rbt_color = RBT_RED;
                    std_rbtree_rotateright(rbtt, w);
                    w = x->rbt_parent->rbt_right;
                }

                w->rbt_color = x->rbt_parent->rbt_color;
                x->rbt_parent->rbt_color = RBT_BLACK;
                w->rbt_right->rbt_color = RBT_BLACK;
                std_rbtree_rotateleft(rbtt, x->rbt_parent);
                x = rbtt->rbtt_root;
            }
        } else {
            w = x->rbt_parent->rbt_left;

            if (w->rbt_color == RBT_RED)
            {
                w->rbt_color = RBT_BLACK;
                x->rbt_parent->rbt_color = RBT_RED;
                std_rbtree_rotateright(rbtt, x->rbt_parent);
                w = x->rbt_parent->rbt_left;
            }

            if (w == NIL(rbtt))
                break;

            if (w->rbt_left->rbt_color == RBT_BLACK && w->rbt_right->rbt_color == RBT_BLACK)
            {
                w->rbt_color = RBT_RED;
                x = x->rbt_parent;
            }
            else
            {
                if (w->rbt_left->rbt_color == RBT_BLACK)
                {
                    w->rbt_right->rbt_color = RBT_BLACK;
                    w->rbt_color = RBT_RED;
                    std_rbtree_rotateleft(rbtt, w);
                    w = x->rbt_parent->rbt_left;
                }
                w->rbt_color = x->rbt_parent->rbt_color;
                x->rbt_parent->rbt_color = RBT_BLACK;
                w->rbt_left->rbt_color = RBT_BLACK;
                std_rbtree_rotateright(rbtt, x->rbt_parent);
                x = rbtt->rbtt_root;
            }
        }
    }

    x->rbt_color = RBT_BLACK;

} // std_rbtree_balanceonremove()


static void _std_rbtree_rwalk(rbtree_handle rbtt, std_rbtree_node *x,
                              int (* walk_callback)(rbtree_handle rbtt, void *, va_list ap),
                              va_list ap)
{
    va_list ap1;

    if (x != NIL(rbtt))
    {
        _std_rbtree_rwalk(rbtt, x->rbt_left, walk_callback, ap);
        memcpy((void*)&ap1, (void*)&ap, sizeof(va_list));
        walk_callback(rbtt, x->rbt_data, ap1);
        _std_rbtree_rwalk(rbtt, x->rbt_right, walk_callback, ap);
    }
} // _std_rbtree_RWalk()


void std_rbtree_rwalk(rbtree_handle rbtt,
                             int (* walk_callback)(rbtree_handle rbtt, void *, va_list ap),
                             int ncount, ...)
{
    va_list ap;

    RBT_ASSERT(rbtt);
    RBT_ASSERT(rbtt->rbtt_magic == RBT_MAGIC);

    va_start(ap, ncount);
    _std_rbtree_rwalk(rbtt, rbtt->rbtt_root, walk_callback, ap);
    va_end(ap);
} // std_rbtree_RWalk()


/*---------------------------------------------------------------*\
 *            Public methods
\*---------------------------------------------------------------*/

std_rbtree_node * _std_rbtree_getfirst(rbtree_handle rbtt)
{
    std_rbtree_node *x;

    RBT_DEBUG_START(rbtt);
    RBT_DEBUG_END;

    x = rbtt->rbtt_root;
    if (x != NIL(rbtt))
    {
        while (x->rbt_left != NIL(rbtt))
            x = x->rbt_left;
    }

    if (x == NIL(rbtt))
      return (std_rbtree_node *)0;
    else
      return x;
} // _std_rbtree_getfirst()


void * std_rbtree_getfirst(rbtree_handle rbtt)
{
    std_rbtree_node *x;

    RBT_DEBUG_START(rbtt);
    RBT_DEBUG_END;

    x = _std_rbtree_getfirst(rbtt);

    if (x)
        return x->rbt_data;
    else
        return (void *)0;

} // std_rbtree_getfirst()


std_rbtree_node * _std_rbtree_getexact(rbtree_handle rbtt, void *data)
{
    int cmp;
    std_rbtree_node *x;

    RBT_DEBUG_START(rbtt);
    RBT_ASSERT(data);
    RBT_DEBUG_END;

    x = rbtt->rbtt_root;
    while (x != NIL(rbtt))
    {
        cmp = rbtt->rbtt_compare(rbtt, data, x->rbt_data);
        if (cmp == 0)
            break;
        else if (cmp < 0)
            x = x->rbt_left;
        else
            x = x->rbt_right;
    }

    if (x == NIL(rbtt))
        return (std_rbtree_node *)0;
    else
        return x;

} // _std_rbtree_getexact()


void * std_rbtree_getexact(rbtree_handle rbtt, void *data)
{
    std_rbtree_node *x;

    RBT_DEBUG_START(rbtt);
    RBT_ASSERT(data);
    RBT_DEBUG_END;

    x = _std_rbtree_getexact(rbtt, data);

    if (x)
        return x->rbt_data;
    else
        return (void *)0;
} // std_rbtree_getexact()


std_rbtree_node * _std_rbtree_insert(rbtree_handle rbtt, std_rbtree_node *z)
{
    std_rbtree_node *x, *y;

    RBT_DEBUG_START(rbtt);
    RBT_ASSERT(z);
    RBT_DEBUG_END;

    y = NIL(rbtt);
    x = rbtt->rbtt_root;
    while (x != NIL(rbtt))
    {
        y = x;

        if (RBT_IS_LESS(rbtt, z->rbt_data, x->rbt_data))
            x = x->rbt_left;
        else
            x = x->rbt_right;
    }

    z->rbt_parent = y;
    z->rbt_left = z->rbt_right = NIL(rbtt);

    if (y == NIL(rbtt))
        rbtt->rbtt_root = z;
    else if (RBT_IS_LESS(rbtt, z->rbt_data, y->rbt_data))
        y->rbt_left = z;
    else
        y->rbt_right = z;

    std_rbtree_balanceoninsert(rbtt, z);

    rbtt->rbtt_numinserts++;
    rbtt->rbtt_numinodes++;
    return(z);
} // _std_rbtree_insert()


t_std_error std_rbtree_insert(rbtree_handle rbtt, void *data)
{
    std_rbtree_node *z;

    RBT_DEBUG_START(rbtt);
    RBT_ASSERT(data);
    RBT_DEBUG_END;

    if ((z = (std_rbtree_node *)rbtt->rbtt_malloc(sizeof(std_rbtree_node))) == (std_rbtree_node *)0)
        return (STD_ERR_FROM_ERRNO(e_std_err_COM, e_std_err_code_FAIL));
    rbtt->rbtt_nummallocs++;
    z->rbt_data = data;

    if (_std_rbtree_insert(rbtt, z))
        return STD_ERR_OK;
    else
    {
        rbtt->rbtt_free(z);
        rbtt->rbtt_numfrees++;
        return (STD_ERR_FROM_ERRNO(e_std_err_COM, e_std_err_code_FAIL));
    }

} // std_rbtree_insert()


std_rbtree_node * _std_rbtree_getnext(rbtree_handle rbtt, std_rbtree_node *x)
{
    std_rbtree_node *y;

    RBT_DEBUG_START(rbtt);
    RBT_DEBUG_END;

    if (x == (std_rbtree_node *)0)
        return (std_rbtree_node *)0;

    if (x->rbt_right != NIL(rbtt))
    {
        y = x->rbt_right;
        while (y->rbt_left != NIL(rbtt))
            y = y->rbt_left;
    }
    else
    {
        y = x->rbt_parent;
        while (y != NIL(rbtt) && x == y->rbt_right) {
            x = y;
            y = x->rbt_parent;
        }
    }

    if (y == NIL(rbtt))
      return (std_rbtree_node *)0;
    else
      return y;

} // _std_rbtree_getnext()


void * std_rbtree_getnext(rbtree_handle rbtt, void *data)
{
    std_rbtree_node *x, *y;

    RBT_DEBUG_START(rbtt);
    RBT_DEBUG_END;

    if (!data)
        return (void *)0;

    if ((x = _std_rbtree_getexact(rbtt, data)))
        y = _std_rbtree_getnext(rbtt, x);
    else
        y = _std_rbtree_getexactornext(rbtt, data);

    if (y)
        return y->rbt_data;
    else
        return (void *)0;
} // std_rbtree_getnext()


void _std_rbtree_remove(rbtree_handle rbtt,  std_rbtree_node *z)
{
    std_rbtree_node *x, *y;

    RBT_DEBUG_START(rbtt);
    RBT_ASSERT(z);
    RBT_DEBUG_END;

    if (z->rbt_left == NIL(rbtt) || z->rbt_right == NIL(rbtt))
    {
        /* y has a NIL node as a child */
        y = z;
    }
    else
    {
        /* find tree successor with a NIL node as a child */
        y = _std_rbtree_getnext(rbtt, z);
    }

    /* x is y's only child */
    if (y->rbt_left != NIL(rbtt))
        x = y->rbt_left;
    else
        x = y->rbt_right;

    /* remove y from the rbt_parent chain */
    x->rbt_parent = y->rbt_parent;
    if (y->rbt_parent == NIL(rbtt))
    {
        rbtt->rbtt_root = x;
    }
    else
    {
        if (y == y->rbt_parent->rbt_left)
            y->rbt_parent->rbt_left = x;
        else
            y->rbt_parent->rbt_right = x;
    }

    if (y->rbt_color == RBT_BLACK)
        std_rbtree_balanceonremove(rbtt, x);

    if (y != z)
    {
        if (rbtt->rbtt_root == z)
            rbtt->rbtt_root = y;

        y->rbt_parent = z->rbt_parent;
        y->rbt_left = z->rbt_left;
        y->rbt_right = z->rbt_right;
        y->rbt_color = z->rbt_color;

        if (y->rbt_parent != NIL(rbtt))
        {
            if (y->rbt_parent->rbt_left == z)
                y->rbt_parent->rbt_left = y;
            else
                y->rbt_parent->rbt_right = y;
        }

        if (y->rbt_left != NIL(rbtt))
            y->rbt_left->rbt_parent = y;

        if (y->rbt_right != NIL(rbtt))
            y->rbt_right->rbt_parent = y;
    }

    z->rbt_left = z->rbt_right = z->rbt_parent = NIL(rbtt);

    rbtt->rbtt_numremoved++;
    rbtt->rbtt_numinodes--;
} // _std_rbtree_remove()


void * std_rbtree_remove(rbtree_handle rbtt,  void *data)
{
    void *rbt_data;
    std_rbtree_node *x;

    RBT_DEBUG_START(rbtt);
    RBT_ASSERT(data);
    RBT_DEBUG_END;

    if ((x = _std_rbtree_getexact(rbtt, data)) == (std_rbtree_node *)0)
        return (void *)0;

    _std_rbtree_remove(rbtt, x);

    rbt_data = x->rbt_data;
    rbtt->rbtt_free(x);
    rbtt->rbtt_numfrees++;
    return rbt_data;
} // std_rbtree_remove()

/** Macro for callback. This macro calls the user callback
 *  with a certain set of parameters. It passes on the
 *  variale parameters to the callback. Note that we need
 *  do a copy of va_list every time we call the user callback.
 *  The macro also decrements the counter for the number
 *  of user node visits. In addition, if the user callback
 *  returns a non-zero value then the walker returns
 *  immediately, this is achieved by setting the counter to 0.
 */
#define RBTUSERCALLBACK(x)                       \
{                                                \
    va_list ap1;                                 \
                                                 \
    lcnt--;                                      \
    if (walk_fn)                                 \
    {                                            \
        va_copy(ap1,ap);                         \
        if (walk_fn(rbtt, (x), ap1))             \
            lcnt = 0;                            \
    }                                            \
}


/**
 *  A non-recursive inorder walk.
 *
 *  Basic idea is to issue callback on nodes on left before the
 *  current node followed by the nodes on the left. And apply this
 *  recursively. To recurse may be devine, but we will stick to
 *  being human and iterate with this idea. The walk itself is inorder,
 *  but we also support preorder by calling the callback on the first
 *  visit to a node (on the left walk).
 *
 *  Here's how it works. Three basic walking directions are defined:
 *  RBT_WALKDOWN - traverses the left link; RBT_WALKRIGHT - traverses the
 *  right link; RBT_WALKUP - traverses up the parent link.
 *
 *  First move as much to the left as possible. Then move right.
 *  If able to move to right, then again move to left as much
 *  as possible. And so on.
 *  When not able to go right move up. When moving up check if reaching
 *  the parent node from the left link or the right link. If coming
 *  from the left then move right. When coming from the right link
 *  then move further up. Rest is detail.
 */
std_rbtree_node * _std_rbtree_walk(rbtree_handle rbtt, std_rbtree_node *x,
                                   int (* walk_fn)(rbtree_handle rbtt, void *, va_list ap),
                                   int cnt, int flag, va_list ap)
{
    u_long lcnt;
    std_rbtree_node *y;
    int dir;
    int walk = flag & 0x1;

    RBT_DEBUG_START(rbtt);
    RBT_DEBUG_END;

    if (!cnt)
        lcnt = 0xffffffff;
    else
        lcnt = (u_long) cnt;

    if (!x)
        dir = RBT_WALKDOWN, x = rbtt->rbtt_root;
    else
        dir = RBT_WALKRIGHT;

    while (lcnt && x != NIL(rbtt))
    {
        switch (dir)
        {
        case RBT_WALKDOWN:
            if (x->rbt_left != NIL(rbtt))
            {
                if (walk == RBT_PREORDERWALK)
                     RBTUSERCALLBACK(x->rbt_data);
                x = x->rbt_left;
            }
            else
            {
                RBTUSERCALLBACK(x->rbt_data);
                dir = RBT_WALKRIGHT;
            }
            continue;

        case RBT_WALKRIGHT:
            if (x->rbt_right == NIL(rbtt))
            {
                y = x;
                if ((x = x->rbt_parent) == NIL(rbtt))
                    return (std_rbtree_node *)0;
                if (y == x->rbt_left)
                {
                    dir = RBT_WALKRIGHT;
                    if (walk == RBT_INORDERWALK)
                        RBTUSERCALLBACK(x->rbt_data);
                }
                else
                {
                    dir = RBT_WALKUP;
                }
            }
            else
            {
                dir = RBT_WALKDOWN;
                x = x->rbt_right;
            }
            continue;

        case RBT_WALKUP:
            y = x;
            if ((x = x->rbt_parent) == NIL(rbtt))
                return (std_rbtree_node *)0;
            if (y == x->rbt_left)
            {
                dir = RBT_WALKRIGHT;
                if (walk == RBT_INORDERWALK)
                    RBTUSERCALLBACK(x->rbt_data);
            }
            continue;

        default:
            RBT_ASSERT(0);
            continue;
        }

    }

    return x;

} // _std_rbtree_walk()


void * std_rbtree_walk(rbtree_handle rbtt, void *data,
                       int (* walk_fn)(rbtree_handle rbtt, void *, va_list ap), int cnt,
                       int flag, ...)
{
    va_list ap;
    std_rbtree_node *x, *y;

    RBT_DEBUG_START(rbtt);
    RBT_DEBUG_END;

    if (data)
        x = _std_rbtree_getexactornext(rbtt, data);
    else
        x = (std_rbtree_node *)0;

    va_start(ap, flag);
    y = _std_rbtree_walk(rbtt, x, walk_fn, cnt, flag, ap);
    va_end(ap);

    if (!y || y == NIL(rbtt))
        return (void *) 0;
    else
        return y->rbt_data;

} // std_rbtree_Walk()


std_rbtree_node * _std_rbtree_getexactornext(rbtree_handle rbtt, void *data)
{
    int cmp;
    std_rbtree_node *x, *y;

    RBT_DEBUG_START(rbtt);
    RBT_ASSERT(data);
    RBT_DEBUG_END;

    x = rbtt->rbtt_root;
    while (x != NIL(rbtt))
    {
        cmp = rbtt->rbtt_compare(rbtt, data, x->rbt_data);
        if (cmp == 0)
            break;
        if (cmp < 0)
        {
            // Nothing left, only bigger from here when going up
            // so this is the next.
            if (x->rbt_left == NIL(rbtt))
                break;
            x = x->rbt_left;
        }
        else
        {
            if (x->rbt_right == NIL(rbtt))
            {
                // Nothing right, means we're bigger than this guy,
                // back off till the first left turn, there you saw
                // the smallest of the greatest ;-)
                y = x->rbt_parent;
                while (y != NIL(rbtt) && x == y->rbt_right)
                {
                    x = y;
                    y = x->rbt_parent;
                }
                x = y;
                break;
            }
            x = x->rbt_right;
        }
    }

  if (x == NIL(rbtt))
    return (std_rbtree_node *) 0;
  else
    return x;
} // _std_rbtree_getexactornext()


void * std_rbtree_getexactornext(rbtree_handle rbtt, void *data)
{
    std_rbtree_node *x;

    RBT_DEBUG_START(rbtt);
    RBT_ASSERT(data);
    RBT_DEBUG_END;

    if ((x = _std_rbtree_getexactornext(rbtt, data)))
        return x->rbt_data;
    else
        return (void *)0;
} // std_rbtree_getexactornext()


std_rbtree_node * _std_rbtree_getexactorprev(rbtree_handle rbtt, void *data)
{
    int cmp;
    std_rbtree_node *x, *y;

    RBT_DEBUG_START(rbtt);
    RBT_ASSERT(data);
    RBT_DEBUG_END;

    x = rbtt->rbtt_root;
    while (x != NIL(rbtt))
    {
        cmp = rbtt->rbtt_compare(rbtt, data, x->rbt_data);
        if (cmp == 0)
            break;
        if (cmp < 0)
        {
            if (x->rbt_left == NIL(rbtt))
            {
                y = x->rbt_parent;
                while (y != NIL(rbtt) && x == y->rbt_left)
                {
                    x = y;
                    y = x->rbt_parent;
                }
                x = y;
                break;
            }
            x = x->rbt_left;
        }
        else
        {
            if (x->rbt_right == NIL(rbtt))
            {
                break;
            }
            x = x->rbt_right;
        }
    }

    if (x == NIL(rbtt))
        return (std_rbtree_node *) 0;
    else
        return x;
} // _std_rbtree_getexactorprev()


void * std_rbtree_getexactorprev(rbtree_handle rbtt, void *data)
{
    std_rbtree_node *x;

    RBT_DEBUG_START(rbtt);
    RBT_ASSERT(data);
    RBT_DEBUG_END;

    if ((x = _std_rbtree_getexactorprev(rbtt, data)))
        return x->rbt_data;
    else
        return (void *)0;
} // std_rbtree_getexactorprev()


void std_rbtree_Debug(rbtree_handle rbtt, int rb_bool)
{
    RBT_DEBUG_START(rbtt);
    RBT_DEBUG_END;

    rbtt->rbtt_debug = (rb_bool == TRUE ? TRUE : FALSE);
} // std_rbtree_debug()


rbtree_handle std_rbtree_create(char *rbtt_name, int keyoffset, int keylength,
                                void *rbtt_malloc(size_t), void rbtt_free(void *),
                                int rbtt_compare(rbtree_handle rbtt, void *, void *))
{
    std_rbtree_node *nil;
    std_rbtree_table *rbtt;

    if ((rbtt = (std_rbtree_table *) RBT_MALLOC(sizeof(std_rbtree_table)))
        == (std_rbtree_table *)0)
        return (rbtree_handle)0;

    nil = NIL(rbtt);
    nil->rbt_color = RBT_BLACK;
    nil->rbt_left = nil->rbt_right = nil->rbt_parent = NIL(rbtt);

    rbtt->rbtt_magic = RBT_MAGIC;
    strncpy(rbtt->rbtt_name,rbtt_name,RBT_NAME_MAX_LEN);
    rbtt->rbtt_name[RBT_NAME_MAX_LEN] = '\0';
    rbtt->rbtt_keyoffset = keyoffset;
    rbtt->rbtt_root = NIL(rbtt);
    rbtt->rbtt_debug = TRUE;
    rbtt->rbtt_compare = rbtt_compare;
    if (rbtt_compare == RBT_ULONG_KEY)
        rbtt->rbtt_keylength = sizeof(u_long);
    else if (rbtt_compare == RBT_INT_KEY)
        rbtt->rbtt_keylength = sizeof(int);
    else
        rbtt->rbtt_keylength = keylength;

    if (!rbtt_malloc)
        rbtt->rbtt_malloc = std_rbtree_malloc;
    else
        rbtt->rbtt_malloc = rbtt_malloc;
    if (!rbtt_free)
        rbtt->rbtt_free = std_rbtree_free;
    else
        rbtt->rbtt_free = rbtt_free;

    rbtt->rbtt_numinodes = 0;
    rbtt->rbtt_numinserts = 0;
    rbtt->rbtt_numremoved = 0;
    rbtt->rbtt_nummallocs = 0;
    rbtt->rbtt_numfrees = 0;

    RBT_DEBUG_START(rbtt);
    RBT_ASSERT(rbtt_name);
    RBT_ASSERT(rbtt_compare);
    RBT_DEBUG_END;

    return (rbtree_handle) rbtt;
} // std_rbtree_create()


void std_rbtree_destroy(rbtree_handle rbtt)
{
    RBT_DEBUG_START(rbtt);
    RBT_DEBUG_END;

    rbtt->rbtt_magic = 0; /* daggling ptr may give problem; clear it anyway */

    RBT_FREE(rbtt);
} // std_rbtree_destroy()


#define MAXDEPTH    32

void std_rbtree_print(rbtree_handle rbtt, char *print_fn(void *))
{
    struct {
    std_rbtree_node *rb;
    int    state;
    char    right;
    } stack[MAXDEPTH], *sp;
    char prefix[MAXDEPTH];
    int i = MAXDEPTH;
    char number[4];

    int dir;
    u_char height;
    std_rbtree_node *y, *x;

    RBT_DEBUG_START(rbtt);
    RBT_ASSERT(print_fn);
    RBT_DEBUG_END;

    height = 0;
    dir = RBT_WALKDOWN, x = rbtt->rbtt_root;
    while (x != NIL(rbtt))
    {
        switch (dir)
        {
        case RBT_WALKDOWN:
            x->rbt_height = height;
            height++;
            if (x->rbt_left != NIL(rbtt))
                x = x->rbt_left;
            else
                dir = RBT_WALKRIGHT;
            continue;

        case RBT_WALKRIGHT:
            if (x->rbt_right == NIL(rbtt))
            {
                y = x;
                if ((x = x->rbt_parent) == NIL(rbtt))
                    break;
                if (y == x->rbt_left)
                    height = y->rbt_height, dir = RBT_WALKRIGHT;
                else
                    dir = RBT_WALKUP;
            }
            else
            {
                dir = RBT_WALKDOWN;
                x = x->rbt_right;
            }
            continue;

        case RBT_WALKUP:
            y = x;
            if ((x = x->rbt_parent) == NIL(rbtt))
                break;
            if (y == x->rbt_left)
                height = y->rbt_height, dir = RBT_WALKRIGHT;
            continue;

        default:
            RBT_ASSERT(0);
            continue;
        }
    }


    while (i--) {
    prefix[i] = ' ';
    }

    {
    sp = stack;
    (void) printf("\tRed-Black tree %s: %lu numinodes.",
               rbtt->rbtt_name, rbtt->rbtt_numinodes);
    if (rbtt->rbtt_numinodes > 200) {
        (void) printf(" (too large to print)\n\n");
    } else if (!(sp->rb = rbtt->rbtt_root)) {
        (void) printf(" (empty)\n\n");
    } else {
        /* If the tree is small enough, format it */
        (void) printf("\n\n");
        sp->right = TRUE;
        sp->state = 0;
        do {
        std_rbtree_node *rb = sp->rb;
        int ii = (sp - stack) * 3;

        switch (sp->state) {
        case 0:
            sp->state++;
            if (rb->rbt_right != NIL(rbtt)) {
            sp++;
            sp->rb = rb->rbt_right;
            sp->right = TRUE;
            sp->state = 0;
            continue;
            }
            /* Fall through */

        case 1:
            sp->state++;

            (void) sprintf(number, "%3d", rb->rbt_height);
            for (i = 0; i < 3 && number[i] == ' '; i++) {
            number[i] = '-';
            }
            (void) printf("\t\t%.*s+-%s%s+",
                   ii, prefix,
                   (rb->rbt_height > 9) ? "" : "-",
                   (rb->rbt_color == RBT_BLACK) ? "B" : "R");
            if (rb->rbt_data) {
            (void) printf("--[%s\n",
                       print_fn(rb->rbt_data));
            } else {
            (void) printf("\n");
            }

            switch (sp->right) {
            case TRUE:
            prefix[ii] = '|';
            break;

            case FALSE:
            prefix[ii] = ' ';
            break;
            }

            if (rb->rbt_left != NIL(rbtt)) {
            sp++;
            sp->rb = rb->rbt_left;
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

} // std_rbtree_print()


int _std_rbtree_compare_ul(rbtree_handle rbtt, void *one, void *two)
{
    int offset;
    u_long ul1, ul2;

    RBT_ASSERT(one);
    RBT_ASSERT(two);

    offset = rbtt->rbtt_keyoffset;
    ul1 = *(u_long *)(((char *)one) + offset);
    ul2 = *(u_long *)(((char *)two) + offset);

    if (ul1 == ul2)
        return 0;
    else
        return (ul1 < ul2) ? -1 : 1;
} // _std_rbtree_compare_ul()


int _std_rbtree_compare_i(rbtree_handle rbtt, void *one, void *two)
{
    int offset;
    int i1, i2;

    RBT_ASSERT(one);
    RBT_ASSERT(two);

    offset = rbtt->rbtt_keyoffset;
    i1 = *(int *)(((char *)one) + offset);
    i2 = *(int *)(((char *)two) + offset);

    if (i1 == i2)
        return 0;
    else
        return (i1 < i2) ? -1 : 1;
} // _std_rbtree_compare_i()

int std_rbtree_gen_cmp(rbtree_handle rbtt, void *lhs, void *rhs) {
    return memcmp(  ((char*)lhs) + rbtt->rbtt_keyoffset,
                    ((char*)rhs) + rbtt->rbtt_keyoffset,
                    rbtt->rbtt_keylength);
}

void * std_rbtree_malloc_wrapper(size_t len) {
    return calloc(1,len);
}

rbtree_handle std_rbtree_create_simple(char *rbtt_name, int keyoffset,
    int keylength) {
    return std_rbtree_create(rbtt_name,keyoffset,keylength,
        std_rbtree_malloc_wrapper,free,std_rbtree_gen_cmp);
}
