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
 * std_system_unittest.c
 */


#include "std_system.h"
#include <stdbool.h>
#include "gtest/gtest.h"

TEST(std_system_unittest, find_procfs)
{
    char sysfs[250];
    ASSERT_EQ(STD_ERR_OK,std_sys_sysfs_path_get(sysfs,sizeof(sysfs)));
    ASSERT_TRUE(strlen(sysfs)!=0);
}

TEST(std_system_unittest, execve_command_ok)
{
   const char* args[] = {
       "echo",
       "test",
       NULL
   };
   const char* envp[] = {
       NULL
   };
   ASSERT_EQ(STD_ERR_OK,std_sys_execve_command("/bin/echo", args, envp));
}

TEST(std_system_unittest, execve_command_fail)
{
   const char* args1[] = {
       "no_such_command",
       "test",
       NULL
   };
   const char* envp[] = {
       NULL
   };
   ASSERT_NE(STD_ERR_OK,std_sys_execve_command("no_such_command", args1, envp));
   const char* args2[] = {
       "echo",
       "test",
       NULL
   };
   // Not found, no search of PATH is performed
   ASSERT_NE(STD_ERR_OK,std_sys_execve_command("echo", args2, envp));
}

TEST(std_system_unittest, set_netns)
{
    static const char* nsname = "opxtest";

    const char* envp[] = {
            NULL
    };
    const char* args_add[] = {
       "ip",
       "netns",
       "add",
       nsname,
       NULL
    };
    ASSERT_EQ(STD_ERR_OK,std_sys_execve_command("/sbin/ip", args_add, envp));

    int ns_handle = -1;
    ASSERT_EQ(STD_ERR_OK,std_sys_set_netns(nsname, &ns_handle));

    ASSERT_EQ(STD_ERR_OK,std_sys_reset_netns(&ns_handle));
    const char* args_del[] = {
        "ip",
        "netns",
        "del",
        nsname,
        NULL
    };
    ASSERT_EQ(STD_ERR_OK,std_sys_execve_command("/sbin/ip", args_del, envp));

    ASSERT_NE(STD_ERR_OK,std_sys_set_netns(nsname, &ns_handle));
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
