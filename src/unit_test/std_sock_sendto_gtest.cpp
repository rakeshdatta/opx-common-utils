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
 * filename: std_sock_tools_gtest.cpp
 */


#include "std_socket_tools.h"
#include "std_file_utils.h"
#include "gtest/gtest.h"

#include <iostream>

TEST(std_sock_tools_test, sendto)
{
    int fd;
    auto rc = std_socket_create (e_std_sock_INET4,
                                 e_std_sock_type_DGRAM, 0,
                                 NULL, &fd);
    ASSERT_TRUE(STD_ERR_OK == rc);

    std::string msg {"This is a sample test"};

    std_socket_address_t dest;
    std_sock_addr_from_ip_str (e_std_sock_INET4, "127.0.0.1", 20000, &dest);

    struct iovec sock_data = {(char*)msg.c_str(), msg.length()};
    std_socket_msg_t sock_msg = { &dest.address.inet4addr,
                                 sizeof (dest.address.inet4addr),
                                 &sock_data,
                                 1, NULL, 0, 0};

    auto n = std_socket_op (std_socket_transit_o_WRITE, fd, &sock_msg,
                            std_socket_transit_f_NONE, 0, &rc);

    if (n < 0) printf ("FAILED\r\n");
    else printf ("Successfully sent %ld bytes\r\n", n);
    ASSERT_TRUE(STD_ERR_OK == rc);

    rc = std_close (fd);
    ASSERT_TRUE(STD_ERR_OK == rc);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

