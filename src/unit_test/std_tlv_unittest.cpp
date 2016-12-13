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


#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "gtest/gtest.h"
#include "std_tlv.h"


TEST(std_tlv,tlv_create ) {
	char buff[1024];
	size_t len = sizeof(buff);
	void * p = buff;
	p = std_tlv_add(p,&len,0,6,(void*)"Cliff");
	p = std_tlv_add_u16(p,&len,1,1);
	p = std_tlv_add_u32(p,&len,2,2);
	p = std_tlv_add_u64(p,&len,3,3);
	len = sizeof(buff) - len;

	void * ptr = buff;
	ASSERT_FALSE(std_tlv_tag(ptr)!=0);
	ASSERT_FALSE(std_tlv_len(ptr)!=6);
	printf("Tag %d, Len %d\n",(int)std_tlv_tag(ptr),(int)std_tlv_len(ptr));

	ptr = std_tlv_next(ptr,&len);
	ASSERT_FALSE(std_tlv_tag(ptr)!=1);
	ASSERT_FALSE(std_tlv_data_u16(ptr)!=1);
	printf("Tag %d, Len %d\n",(int)std_tlv_tag(ptr),(int)std_tlv_len(ptr));

	ptr = std_tlv_next(ptr,&len);
	ASSERT_FALSE(std_tlv_tag(ptr)!=2);
	ASSERT_FALSE(std_tlv_data_u32(ptr)!=2);
	printf("Tag %d, Len %d\n",(int)std_tlv_tag(ptr),(int)std_tlv_len(ptr));

	ptr = std_tlv_next(ptr,&len);
	ASSERT_FALSE(std_tlv_tag(ptr)!=3);
	ASSERT_FALSE(std_tlv_data_u64(ptr)!=3);
	printf("Tag %d, Len %d\n",(int)std_tlv_tag(ptr),(int)std_tlv_len(ptr));
}

TEST(std_tlv,tlv_create_list ) {
	char buff[4000];
	size_t len = sizeof(buff);
	size_t ix = 0;
	size_t mx = 100;
	void *ptr = buff;
	size_t count = 0;
	for ( ; ix < mx ; ++ix ) {
		printf("Remaining is %d\n",(int)len);
		ptr = std_tlv_add(ptr,&len,ix,6,(void*)"Cliff");
		if (ptr==NULL) break;
		count++;
	}

	len = sizeof(buff) - len;
	ix = 0;
	ptr = buff;

	for ( ; ptr!=NULL && len > 0 ; ptr = std_tlv_next(ptr,&len) ) {
		printf("Tag %d, Len %d (remain %d) \n",(int)std_tlv_tag(ptr),(int)std_tlv_len(ptr),(int)len);
		++ix;
	}
	printf("Count =%d, IX=%d\n",(int)count,(int)ix);
	ASSERT_TRUE(count == ix);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
