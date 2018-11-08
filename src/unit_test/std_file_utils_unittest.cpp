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
 * std_system_unittest.c
 */


#include "std_file_utils.h"
#include "std_system.h"
#include <stdbool.h>
#include "gtest/gtest.h"
#include <fcntl.h>


TEST(std_file_utils_unittest, netns_open)
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

    int fd = -1;
    ASSERT_EQ(STD_ERR_OK, std_netns_fd_open(nsname, (const char*)"/proc/net/dev", O_RDONLY, &fd));
    ASSERT_GE(fd, 0);
    std_close(fd);

    const char* args_del[] = {
       "ip",
       "netns",
       "del",
       nsname,
       NULL
    };
    ASSERT_EQ(STD_ERR_OK,std_sys_execve_command("/sbin/ip", args_del, envp));

    ASSERT_NE(STD_ERR_OK,std_netns_fd_open(nsname, (const char*)"/proc/net/dev", O_RDONLY, &fd));
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}




