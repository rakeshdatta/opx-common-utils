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
 * filename: std_bit_masks.c
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <netinet/in.h>
#include "std_bit_masks.h"

static inline unsigned int bittobytelen(unsigned int len) {
    return (len/8) + ((len%8)==0 ? 0 : 1);
}

void * std_bitmap_create_array_with_pattern(unsigned long len, unsigned char pattern) {
    len = bittobytelen(len);
    void *p = malloc(len);
    if (p==NULL) return NULL;
    memset(p,pattern,len);
    return p;
}

void * std_bitmap_create_array_clear_bits(unsigned long len) {
    return std_bitmap_create_array_with_pattern(len,0);
}

void * std_bitmap_create_array(unsigned long len) {
    return std_bitmap_create_array_with_pattern(len,0xff);
}

void std_complement_bitmaparray(void *bits,unsigned long mx) {
    unsigned int ix=0;
    mx = bittobytelen(mx);
    for ( ; ix < mx ; ++ix ) {
        ( (char*)bits)[ix] = ~(( (char*)bits)[ix]);
    }
}

void std_bitmaparray_free_data (void *bitMap) {
    if (bitMap) { free (bitMap); }
}

static int highest_bit[] = {
//    0 1 2 3 4 5 6 7 8 9 a b c d e f
    0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,        //0
    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,        //10
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,        //20
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,        //30
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,        //40
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,        //50
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,        //60
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,        //70
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,        //80
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,        //90
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,        //a0
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,        //b0
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,        //c0
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,        //d0
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,        //e0
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8          //f0
};

static int lowest_bit[] = {
//    0 1 2 3 4 5 6 7 8 9 a b c d e f
    0,1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,            //00
    5,1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,            //10
    6,1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,            //20
    5,1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,            //30
    7,1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,            //40
    5,1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,            //50
    6,1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,            //60
    5,1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,            //70
    8,1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,            //80
    5,1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,            //90
    6,1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,            //a0
    5,1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,            //b0
    7,1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,            //c0
    5,1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,            //d0
    6,1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,            //e0
    5,1,2,1,3,1,2,1,4,1,2,1,3,1,2,1,            //f0
};

inline unsigned int std_find_last_bit8(register unsigned int byte, register unsigned int from) {
    register unsigned int mask = ((unsigned int )(~0)) >> from;
    return highest_bit[byte&mask];
}

inline unsigned int std_find_first_bit8(register unsigned int byte, register unsigned int from) {
    register unsigned int mask = ((unsigned int )(~0)) << from;
    return lowest_bit[byte&mask];
}

int std_find_first_bit(void *varray, size_t len, size_t from) {
    register uint8_t *array = (uint8_t *)varray;
    if (from >= len) return -1;

    size_t ix = from/8;
    size_t mx = (len /8) + ((len%8)!=0 ? 1 : 0);

    size_t bit = from % 8;
    if (bit) {
        size_t pos = std_find_first_bit8(array[ix],bit);
        if(pos!=0) { return pos-1+(ix*8); }
        ++ix;
    }

    for (; ix < mx ; ++ix ) {
        if (array[ix]==0) { continue; }
        return std_find_first_bit8(array[ix],0) -1 + (ix*8);
    }
    return -1;
}

int std_find_last_bit(void *varray, size_t len, size_t from) {
    register uint8_t *array = (uint8_t *)varray;
    if (from >= len) return -1;

    from = len - from;
    register int mx = (from/8) + (from%8!=0 ? 1 : 0);
    if (mx==0) { return -1; }

    --mx;

    size_t bit = from % 8;
    if (bit) {
        size_t pos = std_find_last_bit8(array[mx],bit); //one based
        if(pos!=0) { return (pos-1)+(mx*8);}
        if (mx==0) {return -1;}
        --mx;
    }

    for ( ; mx >=0 ; --mx) {
        if (array[mx]!=0) {
            return std_find_last_bit8(array[mx],0)-1 + (mx*8) ;
        }
    }
    return -1;
}
