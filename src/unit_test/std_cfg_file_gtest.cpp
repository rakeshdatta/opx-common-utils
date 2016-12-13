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
 * filename: std_cfg_file_gtest.cpp
 */

/*
 * std_cfg_file_gtest.cpp
 *
 */


#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "gtest/gtest.h"

extern "C" {
#include "std_config_file.h"
}

TEST(std_cfg_file_test, FileCreate)
{
        std_cfg_file_handle_t handle1;
        ASSERT_EQ(STD_ERR_OK, std_config_file_create(&handle1));
        ASSERT_EQ(STD_ERR_OK, std_config_file_close(handle1));

}

TEST(std_cfg_file_test, FileSet)
{
        std_cfg_file_handle_t handle;
        ASSERT_EQ(STD_ERR_OK, std_config_file_create(&handle));
        ASSERT_EQ(STD_ERR_OK, std_config_file_set(handle,"test","key","123"));
        ASSERT_EQ(STD_ERR_OK, std_config_file_set(handle,"test","keyb","123"));
        ASSERT_EQ(STD_ERR_OK, std_config_file_set(handle,"test1","key","323"));
        ASSERT_EQ(STD_ERR_OK, std_config_file_close(handle));
}

TEST(std_cfg_file_test, FileWrite)
{
        std_cfg_file_handle_t handle2;
        ASSERT_EQ(STD_ERR_OK, std_config_file_create(&handle2));
        ASSERT_EQ(STD_ERR_OK, std_config_file_set(handle2,"test1","key","323"));
        ASSERT_EQ(STD_ERR_OK, std_config_file_write(handle2,"test.cfg"));
        ASSERT_EQ(STD_ERR_OK, std_config_file_close(handle2));
}


TEST(std_cfg_file_test, FileGet)
{
        std_cfg_file_handle_t handle3;
        const char *p;

        ASSERT_EQ(STD_ERR_OK, std_config_file_open(&handle3,"test.cfg"));
        ASSERT_EQ(STD_ERR_OK, std_config_file_set(handle3,"test","key","123"));
        ASSERT_EQ(STD_ERR_OK, std_config_file_set(handle3,"test","keyb","123"));
        ASSERT_EQ(STD_ERR_OK, std_config_file_set(handle3,"test1","key","323"));

        p = std_config_file_get(handle3,"test","key");
        ASSERT_EQ(0,strcmp("123",p));

        p = std_config_file_get(handle3,"test","keyb");
        ASSERT_EQ(0,strcmp("123",p));

        p = std_config_file_get(handle3,"test1","key");
        ASSERT_EQ(0,strcmp("323",p));

        ASSERT_EQ(STD_ERR_OK, std_config_file_close(handle3));

}

TEST(std_cfg_file_test, FileClose)
{
        std_cfg_file_handle_t handle4;
        ASSERT_EQ(STD_ERR_OK, std_config_file_open(&handle4, "test.cfg"));

        ASSERT_EQ(STD_ERR_OK, std_config_file_close(handle4));
}



int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

