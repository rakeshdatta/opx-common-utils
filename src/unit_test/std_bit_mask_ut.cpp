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
 * filename: std_bit_mask_ut.cpp
 */

/*
 * std_bit_mask_ut.cpp
 */
#include "std_bit_masks.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gtest/gtest.h"

TEST(std_bit_masks, function){
    STD_BIT_ARRAY_CREATE(t,100);
    memset(t,0,sizeof(t));

    ASSERT_EQ(std_find_first_bit(&t,100,0),-1);

    STD_BIT_ARRAY_SET(t,0);
    ASSERT_EQ(std_find_first_bit(&t,100,0),0);
    memset(t,0,sizeof(t));

    STD_BIT_ARRAY_SET(t,5);
    ASSERT_EQ(std_find_first_bit(&t,100,0),5);
    STD_BIT_ARRAY_SET(t,4);
    ASSERT_EQ(std_find_first_bit(&t,100,0),4);
    STD_BIT_ARRAY_SET(t,8);
    ASSERT_EQ(std_find_first_bit(&t,100,0),4);
    memset(t,0,sizeof(t));


    STD_BIT_ARRAY_SET(t,5);
    ASSERT_EQ(std_find_last_bit(&t,100,0),5);
    STD_BIT_ARRAY_SET(t,4);
    ASSERT_EQ(std_find_last_bit(&t,100,0),5);
    STD_BIT_ARRAY_SET(t,8);
    ASSERT_EQ(std_find_last_bit(&t,100,0),8);
    STD_BIT_ARRAY_SET(t,60);
    ASSERT_EQ(std_find_last_bit(&t,100,0),60);
    ASSERT_EQ(std_find_first_bit(&t,100,0),4);
    STD_BIT_ARRAY_CLR(t,4);
    STD_BIT_ARRAY_CLR(t,5);
    ASSERT_EQ(std_find_first_bit(&t,100,0),8);
    STD_BIT_ARRAY_CLR(t,8);
    ASSERT_EQ(std_find_first_bit(&t,100,0),60);
    memset(t,0,sizeof(t));



}

size_t bit_to_byte(size_t bit) {
    return bit / 8 ;
}

size_t bit_to_bit_offset(size_t bit) {
    return ((bit %8));
}

TEST(std_bit_masks, macros){
    size_t ix = 0;
    size_t mx = 100;
    for ( ; ix < mx ; ++ix ) {
        ASSERT_EQ(STD_BIT_ARRAY_BYTE_OFFSET(ix),bit_to_byte(ix));
    }

    for ( ix =0; ix < mx ; ++ix ) {
        ASSERT_EQ(STD_BIT_ARRAY_BIT_OFFSET(ix),bit_to_bit_offset(ix));
    }

    ASSERT_EQ(STD_BYTES_FOR_BITS(100),100/8 + ((100%8)!=0? 1:0));

    ASSERT_EQ(STD_BIT_MASK_MAKE(char,8),0xff);
    ASSERT_EQ(STD_BIT_MASK_MAKE(int,9),0x1ff);
    ASSERT_EQ(STD_BIT_MASK_MAKE(int,17),0x1ffff);

    ASSERT_EQ(STD_BIT_MASK(int,8,8),0xff00);
    ASSERT_EQ(STD_BIT_MASK(int,17,8),0x1ffff00);


    STD_BIT_ARRAY_CREATE(t,100);
    ASSERT_EQ(sizeof(t),100/8 + ((100%8)!=0? 1:0));
    memset(t,0,sizeof(t));

    ASSERT_FALSE(STD_BIT_ARRAY_TEST(t,99));
    STD_BIT_ARRAY_SET(t,99);
    ASSERT_TRUE(STD_BIT_ARRAY_TEST(t,99));

    ASSERT_FALSE(STD_BIT_ARRAY_TEST(t,50));
    STD_BIT_ARRAY_SET(t,50);
    ASSERT_TRUE(STD_BIT_ARRAY_TEST(t,50));
    STD_BIT_ARRAY_CLR(t,50);
    ASSERT_FALSE(STD_BIT_ARRAY_TEST(t,50));
}



int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

