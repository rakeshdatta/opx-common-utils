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
 * filename: redir_test_c.c
 */


/**
 *       @file  redir_test_c.c
 *      @brief  test the redirection library
 *
 *   @internal - Unit test only
 *     Created  05/11/2014
 *     Company  DELL
 *
 * =====================================================================================
 */


#include "std_cmd_redir.h"
#include "std_select_tools.h"
#include "std_file_utils.h"

#include <stdio.h>
#include <unistd.h>
#include <unistd.h>

int main(int argc, char**argv) {
    char * path = "/tmp/com_test";
    if (argc>1) path = argv[1];
    int sock=-1;

    if (std_cmd_redir_connect(path,&sock)!=STD_ERR_OK)
        return -1;

    while (true) {
       fd_set rset;
        int selfds[]={STDIN_FILENO,sock};
        int max_fd = -1;
        std_sel_adds_set(selfds,sizeof(selfds)/sizeof(*selfds),
                    &rset,&max_fd,true);

         int rc = std_select_ignore_intr(max_fd+1,&rset,NULL,NULL,NULL,NULL);

        if (rc==0) continue;
        if (rc==-1) break;
        if (FD_ISSET(STDIN_FILENO,&rset)) {
            if (std_fd_copy(sock,STDIN_FILENO,NULL)==-1) break;
        }
        if (FD_ISSET(sock,&rset)) {
            if (std_fd_copy(STDOUT_FILENO,sock,NULL)==-1) break;
        }
    }
    return 0;
}

