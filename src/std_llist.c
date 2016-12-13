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
 * filename: std_llist.c
 */

/*!
 * \file   std_llist.h
 * \brief  Linked list functionality
 * \date   05-2014
 * \author
 */

#include <assert.h>
#include "std_llist.h"
#include <stdlib.h>
#include <string.h>
#include "std_type_defs.h"
#include <stdio.h>

#define DLL_MAGIC   0xF10D1F10
#define DLL_DEBUG   0

#if DLL_DEBUG
#define DLL_VALIDATE(head)  assert((head)->dll_magic == DLL_MAGIC)
#else
#define DLL_VALIDATE(head)
#endif

/*----------------------------------------------------------------*\
                     Double Linked List
\*----------------------------------------------------------------*/

void
std_dll_init(std_dll_head *head)
{
    head->head.dll_next = &head->tail;
    head->head.dll_back = (std_dll *)0;

    head->tail.dll_back = &head->head;
    head->tail.dll_next = (std_dll *)0;

    head->dll_magic = DLL_MAGIC;

    head->compare = NULL;

    head->len = 0;
    head->offset = 0;
}

void std_dll_init_sort(std_dll_head *head, std_compare_function compare, unsigned int offset, unsigned int len) {
    std_dll_init(head);
    head->compare = compare;
    head->offset = offset;
    head->len = len;
}

std_dll *
std_dll_getfirst(std_dll_head *head)
{
    DLL_VALIDATE(head);

    if (head->head.dll_next != &head->tail)
        return head->head.dll_next;
    else
        return (std_dll *)0;
}


std_dll *
std_dll_getlast(std_dll_head *head)
{
    DLL_VALIDATE(head);

    if (head->tail.dll_back != &head->head)
        return head->tail.dll_back;
    else
        return (std_dll *)0;
}


std_dll *
std_dll_getnext(std_dll_head *head, std_dll *after)
{
    DLL_VALIDATE(head);

    if (after->dll_next != &head->tail)
        return after->dll_next;
    else
        return (std_dll *)0;
}


std_dll *
std_dll_getprev(std_dll_head *head, std_dll *before)
{
    DLL_VALIDATE(head);

    if (before->dll_back != &head->head)
        return before->dll_back;
    else
        return (std_dll *)0;
}

static inline void * offset(void *ptr, unsigned int len) {
    return ((char*)ptr) + len;
}

int std_compare_int_function(const void *current, const void *node,unsigned int len) {
   printf("Comparing %d and %d\n",*(int*)current,*(int*)node);
   if (( *(int*)current) == (*(int*)node)) return 0;
   return ( *(int*)current) < (*(int*)node) ? -1 : 1  ;
}

int std_compare_uint32_function(const void *current, const void *node,unsigned int len) {
   if (( *(uint32_t*)current) == (*(uint32_t*)node)) return 0;
   return ( *(uint32_t*)current) < (*(uint32_t*)node) ? -1 : 1  ;
}

int std_compare_binary_function(const void *current, const void *node,unsigned int len) {
    return memcmp(current,node,len);
}

void std_dll_insert(std_dll_head *head, std_dll *new) {
    if (head->compare==NULL) {
        std_dll_insertatback(head,new);
        return;
    }
    if (std_dll_getfirst(head) == NULL ||
        head->compare(offset(new,head->offset),offset(head->head.dll_next,head->offset),head->len) < 0) {
        std_dll_insertatfront(head,new);
        return;
    }
    std_dll *walk = head->head.dll_next;
    while (true) {
        if (walk->dll_next==&head->tail ||
            head->compare(offset(new,head->offset),offset(walk->dll_next,head->offset),head->len)<0) {
            std_dll_insertafter(head,walk,new);
            return;
        }
        walk = walk->dll_next;
    }
}

void
std_dll_insertatfront(std_dll_head *head, std_dll *new)
{
    DLL_VALIDATE(head);

    new->dll_next = head->head.dll_next;
    new->dll_back = &head->head;

    head->head.dll_next->dll_back = new;
    head->head.dll_next = new;
}


void
std_dll_insertatback(std_dll_head *head, std_dll *new)
{
    DLL_VALIDATE(head);

    new->dll_next = &head->tail;
    new->dll_back = head->tail.dll_back;

    head->tail.dll_back->dll_next = new;
    head->tail.dll_back = new;
}


void
std_dll_insertafter(std_dll_head *head, std_dll *after, std_dll *new)
{
    DLL_VALIDATE(head);
    assert(after);

    new->dll_next = after->dll_next;
    new->dll_back = after;

    after->dll_next->dll_back = new;
    after->dll_next = new;
}


void
std_dll_insertbefore(std_dll_head *head, std_dll *before, std_dll *new)
{
    DLL_VALIDATE(head);
    assert(before);

    new->dll_next = before;
    new->dll_back = before->dll_back;

    before->dll_back->dll_next = new;
    before->dll_back = new;
}


void
std_dll_remove(std_dll_head *head, std_dll *item)
{
    std_dll *front, *back;
    DLL_VALIDATE(head);

    front = item->dll_back;
    back = item->dll_next;

    front->dll_next = back;
    back->dll_back = front;

    item->dll_back = (std_dll *)0;
    item->dll_next = (std_dll *)0;
}

/*----------------------------------------------------------------*\
                    First In First Out
\*----------------------------------------------------------------*/

#define std_fifo_init std_dll_init

#define std_fifo_insert std_dll_insertatfront

#define std_fifo_remove(head, item) \
{ \
    std_dll *f = (std_dll *)0; \
    if ((f = std_dll_getlast(std_dll_head *head))) \
        std_dll_remove(head, f); \
    (item) = f; \
}


