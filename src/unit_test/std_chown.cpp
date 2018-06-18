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


#include "std_user_perm.h"
#include <gtest/gtest.h>

TEST(std_user_perm, function){
    //just create a file...
    ASSERT_EQ(system("touch /tmp/logfile"),0);


    ASSERT_TRUE(std_user_chown("/tmp/logfile","admin","admin")==STD_ERR_OK);
    ASSERT_TRUE(std_user_chown("/tmp/logfile","root","root")==STD_ERR_OK);
    ASSERT_TRUE(std_user_chown("/tmp/logfile","root",NULL)==STD_ERR_OK);

    struct stat buf;
    ASSERT_TRUE(std_user_chmod("/tmp/logfile","o+rwx")==STD_ERR_OK);
    ASSERT_TRUE(stat("/tmp/logfile",&buf)==0);
    ASSERT_TRUE((buf.st_mode & 7)==7);

    ASSERT_TRUE(std_user_chmod("/tmp/logfile","o-rwx")==STD_ERR_OK);
    ASSERT_TRUE(stat("/tmp/logfile",&buf)==0);
    ASSERT_TRUE((buf.st_mode & 7)==0);

    ASSERT_TRUE(std_user_chmod("/tmp/logfile","a+rwx")==STD_ERR_OK);
    ASSERT_TRUE(stat("/tmp/logfile",&buf)==0);
    ASSERT_TRUE((buf.st_mode & 0777)==0777);

    ASSERT_TRUE(std_user_chmod("/tmp/logfile","u=rxo+rxg+rx")==STD_ERR_OK);
    ASSERT_TRUE(stat("/tmp/logfile",&buf)==0);
    ASSERT_TRUE((buf.st_mode & 0777)==0555);
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
