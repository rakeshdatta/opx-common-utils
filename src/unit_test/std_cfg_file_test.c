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
 * filename: std_cfg_file_test.c
 */

/*
 * std_cfg_file_test.c
 *
 */

#include "std_config_file.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char**argv) {

    if (argc<2) {
        exit (1);
    }
    std_cfg_file_handle_t handle;

    if (strcmp("test",argv[1])==0) {
        printf("Running test...\n");
        printf("Create file...\n");
        if (std_config_file_create(&handle)!=STD_ERR_OK) {
            exit(1);
        }
        printf("set key...\n");
        if (std_config_file_set(handle,"test","key","123")!=STD_ERR_OK) {
            exit(1);
        }
        if (std_config_file_set(handle,"test","keyb","123")!=STD_ERR_OK) {
            exit(1);
        }
        if (std_config_file_set(handle,"test1","key","323")!=STD_ERR_OK) {
            exit(1);
        }
        printf("write file...\n");
        if (std_config_file_write(handle,"test.cfg")!=STD_ERR_OK) {
            exit(1);
        }
        printf("close handle\n");
        if (std_config_file_close(handle)!=STD_ERR_OK) {
            exit (1);
        }
        printf("open file handle\n");
        if (std_config_file_open(handle,"test.cfg")!=STD_ERR_OK) {
            exit(1);
        }
        const char *p = std_config_file_get(handle,"test","key");
        if (strcmp("123",p)!=0) {
            exit(1);
        }
        p = std_config_file_get(handle,"test","keyb");
        if (strcmp("123",p)!=0) {
            exit(1);
        }
        const char *list[3];
        if (std_config_file_get_num_keys(handle,"test")!=2) {
            exit(1);
        }
        size_t num_keys=3;
        if (std_config_file_get_keys(handle,"test",list,&num_keys)!=STD_ERR_OK) {
            exit(1);
        }
        size_t ix = 0;
        for ( ; ix < num_keys ; ++ix ) {
            printf("Key %d = %s \n",(int)ix,list[ix]);
        }
        p = std_config_file_get(handle,"test1","key");
        if (strcmp("323",p)!=0) {
            exit(1);
        }
        printf("Passed\n");
        exit(0);
    }
    exit(0);
}
