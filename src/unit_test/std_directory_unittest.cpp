/*
 * Copyright (c) 2018 Dell Inc.
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

#include "std_directory.h"

#include <gtest/gtest.h>
#include <string>
#include <array>
#include <vector>
#include <iostream>
#include <ostream>

std::array<char,1000> __dir = {"/tmp/cliffXXXXXX"};

const int __max_dirs = 10;

#ifndef __cplusplus
#define __cplusplus 201103L
#endif

TEST(std_directory_unittest, prep_test_setup) {
    mkdtemp(&__dir[0]);
    std::string _file = &__dir[0];
    std::cout << "Directory created called " << &__dir[0] << std::endl;
    _file += "/XXXXXX";
    size_t ix = 0;

    for ( ; ix < __max_dirs ; ++ix ) {
        std::vector<char> _l;
        _l.resize(_file.size()+1);
        memcpy(&_l[0],_file.c_str(),_file.size()+1);
        int _fd = mkstemp(&_l[0]);
        ASSERT_NE(_fd,-1);
        close(_fd);
    }

}

static bool _cb(const char *name, std_dir_file_TYPE_t type,void *context) {
    std::vector<std::string> *_lst = static_cast<std::vector<std::string>*> (context);
    if (type==std_dir_file_T_FILE) {
        _lst->push_back(name);
    }
    return true;
}

TEST(std_directory_unittest, list_files) {
    ASSERT_NE(strlen(&__dir[0]),0);
    std_dir_handle_t _h;
    ASSERT_EQ(std_dir_init(&__dir[0],&_h),STD_ERR_OK);
    std_dir_close(_h);

    std::vector<std::string> _lst;
    ASSERT_EQ(std_dir_iterate(&__dir[0],_cb, &_lst,true),STD_ERR_OK);
    ASSERT_EQ(_lst.size(),__max_dirs);
}

TEST(std_directory_unittest, alter_list) {
    ASSERT_NE(strlen(&__dir[0]),0);
    std::vector<std::string> _lst;
    ASSERT_EQ(std_dir_iterate(&__dir[0],_cb, &_lst,true),STD_ERR_OK);
    ASSERT_EQ(_lst.size(),__max_dirs);

    if (_lst.size()>0) {
        unlink(_lst[0].c_str());
    }
    _lst.clear();
    ASSERT_EQ(std_dir_iterate(&__dir[0],_cb, &_lst,true),STD_ERR_OK);

    ASSERT_EQ(_lst.size(),__max_dirs-1);
    for (size_t ix=0; ix < _lst.size() ; ++ix ) {
        unlink(_lst[ix].c_str());
    }

    _lst.clear();
    ASSERT_EQ(std_dir_iterate(&__dir[0],_cb, &_lst,true),STD_ERR_OK);
    ASSERT_EQ(_lst.size(),0);
}

TEST(std_directory_unittest, clean_list) {
    ASSERT_NE(strlen(&__dir[0]),0);

    std::vector<std::string> _lst;
    ASSERT_EQ(std_dir_iterate(&__dir[0],_cb, &_lst,true),STD_ERR_OK);

    for (size_t ix=0; ix < _lst.size() ; ++ix ) {
        unlink(_lst[ix].c_str());
    }
    _lst.clear();
    ASSERT_EQ(std_dir_iterate(&__dir[0],_cb, &_lst,true),STD_ERR_OK);
    ASSERT_EQ(_lst.size(),0);
    unlink(&__dir[0]);
}

TEST(std_directory_unittest, walk_tmp) {
    std::vector<std::string> _lst;
    //var folder always exists
    ASSERT_EQ(std_dir_iterate("/var",_cb, &_lst,true),STD_ERR_OK);
    for ( auto &it : _lst ) {
        std::cout << "Entry: " << it << std::endl;
    }
    ASSERT_NE(_lst.size(),0);
    //don't care about the length of _lst for this test - just verify it works
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  int rc = RUN_ALL_TESTS();
  return rc;
}

