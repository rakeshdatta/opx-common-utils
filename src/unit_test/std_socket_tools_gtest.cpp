/*
 * Copyright (c) 2017 Dell Inc.
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
 * filename: std_socket_tools_gtest.cpp
 */



#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "gtest/gtest.h"

extern "C" {
  #include "std_socket_tools.h"
}
#include "std_system.h"
#include "std_file_utils.h"


TEST(std_socket_tools_unittest, netns_socket_create_ok)
{
   const char* envp[] = {
       NULL
   };
   const char* args_add[] = {
       "ip",
       "netns",
       "add",
       "opxtest",
       NULL
   };
   ASSERT_EQ(STD_ERR_OK,std_sys_execve_command("/sbin/ip", args_add, envp));

   int fd = STD_INVALID_FD;

   ASSERT_EQ(STD_ERR_OK,std_netns_socket_create(e_std_sock_NETLINK, e_std_sock_type_RAW, 0, NULL, "opxtest", &fd) );
   ASSERT_EQ(STD_ERR_OK,std_close(fd));

   const char* args_del[] = {
       "ip",
       "netns",
       "del",
       "opxtest",
       NULL
   };
   ASSERT_EQ(STD_ERR_OK,std_sys_execve_command("/sbin/ip", args_del, envp));
}

TEST(std_socket_tools_unittest, netns_socket_create_fail)
{
   int fd = STD_INVALID_FD;

   ASSERT_NE(STD_ERR_OK,std_netns_socket_create(e_std_sock_NETLINK, e_std_sock_type_RAW, 0, NULL, "opxtest", &fd) );
   ASSERT_NE(STD_ERR_OK,std_close(fd));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

