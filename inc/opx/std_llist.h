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
 * filename: std_llist.h
 */

/*!
 * \file   std_llist.h
 * \brief  Linked list functionality
 * \date   05-2014
 */

#ifndef _LLIST_H_
#define _LLIST_H_

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif
/**
 * This is the structure that has to be the first in any linked list or should
 * be the fist to make the code easier to manage
@verbatim

struct my_struct_list_s {
    std_dll list_pointers;
    int value;
    void *data;
};

@endverbatim
 * Must use the std_dll_getnext and std_dll_getfirst to access forward and back
 */
typedef struct _std_dll {
    struct _std_dll *dll_next; //! next structure in list
    struct _std_dll *dll_back;//! previous structure in list
} std_dll;

/**
 * @brief in the case of a sorted list this compare signature needs to be passed
 * to the init function
 * @param current data structure to compare to
 * @param node data structure to compare against
 * @param len length of the data structure
 * @return -1 if current is less then node, 0 if the same and 1 if current is greater
 */
typedef int (*std_compare_function)(const void *current, const void *node,unsigned int len);

/**
 * An internally handled structure.  Please don't access any fields as they
 * may change in the future.. should treat all fields as private
 */
typedef struct _std_dll_head {
    u_long dll_magic; //! magic number.. used by API


    std_dll head;    /// Head node in the DLL

    std_dll tail;         /// Tail node in the DLL

    unsigned int offset;    //!offset of comapred value in structure
    unsigned int len;        //!length of data type to compare
    std_compare_function compare; //!compare function
} std_dll_head;


#define std_dll_islinked(item) ((item)->dll_next && (item)->dll_back)

/**
 * @brief create a standard double linked list that is not sorted
 * @param head a pointer to a head structure to initialize
 */
void std_dll_init(std_dll_head *head);

/**
 * @brief initialize a sorted list head structure.  std_dll_insert will insert elements sorted
 * @param head list structure to init
 * @param compare function to compare keys
 * @param offset of field to compare - use offsetof(struct,field) from stddef.h
 * @param len length of data type to compare
 */
void std_dll_init_sort(std_dll_head *head,std_compare_function compare, unsigned int offset, unsigned int len);

/**
 * @brief compare integers in linked list structure
 * @param current the left hand side for the compare
 * @param node right hand side to compare
 * @param len of the data to compare (int len = sizeof (int) )
 * @return -1 0 or 1
 */
int std_compare_int_function(const void *current, const void *node,unsigned int len);

/**
 * @brief compare unsigned integers in linked list structure
 * @param current the left hand side for the compare
 * @param node right hand side to compare
 * @param len of the data to compare (int len = sizeof (uint32_t) )
 * @return -1 0 or 1
 */
int std_compare_uint32_function(const void *current, const void *node,unsigned int len);

/**
 * @brief compare generic memory in linked list structure
 * @param current the left hand side for the compare
 * @param node right hand side to compare
 * @param len of the data to compare
 * @return -1 0 or 1
 */
int std_compare_binary_function(const void *current, const void *node,unsigned int len);

/**
 * @brief get the first linked list entry
 * @param head pointer to dll structure
 * @return pointer to first element or NULL if empty
 */
std_dll * std_dll_getfirst(std_dll_head *head);

/**
 * @brief get the last linked list entry
 * @param head pointer to dll structure
 * @return pointer to element or NULL if empty
 */
std_dll * std_dll_getlast(std_dll_head *head);
/**
 * @brief get the next linked list entry i.e. xxx->dll_next
 * @param head pointer to dll structure
 * @return pointer to element or NULL if no next
 */
std_dll * std_dll_getnext(std_dll_head *head, std_dll *after);
/**
 * @brief get the previous linked list entry i.e. xxx->dll_prev
 * @param head pointer to dll structure
 * @return pointer to element or NULL if no prev
 */
std_dll * std_dll_getprev(std_dll_head *head, std_dll *after);

/**
 * @brief insert the element either:
 *      - sorted in the case of a sorted initialized list
 *      - at the end in the case of an unsorted list
 * @param head pointer to dll structure
 * @param new is the new element to insert
 */
void std_dll_insert(std_dll_head *head, std_dll *newnode);
/**
 * @brief insert the element at the front of the list
 * @param head pointer to dll structure
 * @param new is the new element to insert
 */
void std_dll_insertatfront(std_dll_head *head, std_dll *newnode);
/**
 * @brief insert the element at the back of the list
 * @param head pointer to dll structure
 * @param new is the new element to insert
 */
void std_dll_insertatback(std_dll_head *head, std_dll *newnode);
/**
 * @brief insert the element after an element in the the list
 * @param head pointer to dll structure
 * @param after is the pointer after the item to insert
 * @param new is the new element to insert
 */
void std_dll_insertafter(std_dll_head *head, std_dll *after, std_dll *newnode);
/**
 * @brief insert the element before an element in the the list
 * @param head pointer to dll structure
 * @param before is the pointer before the item to insert
 * @param new is the new element to insert
 */
void std_dll_insertbefore(std_dll_head *head, std_dll *before, std_dll *newnode);
/**
 * @brief remove an element from the list
 * @param head pointer to dll structure
 * @param new is the new element to remove
 */
void std_dll_remove(std_dll_head *head, std_dll *item);

#ifdef __cplusplus
}
#endif

#endif /* _LLIST_H_ */
