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
 * filename: redir_test_s.c
 */


/**
 *       @file  redir_test_s.c
 *      @brief  test the redirection library server side
 *
 *   @internal
 *     Created  05/11/2014
 *     Company  DELL
 *
 * =====================================================================================
 */


#include "std_error_codes.h"
#include "std_cmd_redir.h"
#include "std_select_tools.h"
#include "std_file_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


void * do_loop_for_long_time(void *p) {
    while (true) {
        char buff[1024];
        printf("Enter command:\n");
        fflush(stdout);
        if(fgets(buff,sizeof(buff),stdin)!=NULL) {
            printf("Executing... [%s]\n",buff);
            if (strcmp(buff,"__exit__")==0) {
                return NULL;
            }
            FILE *fp = popen(buff,"r");
            while (fgets(buff,sizeof(buff),fp)!=NULL) {
                printf("%s",buff);
            }
            pclose(fp);
        } else exit(1);
        fflush(stdout);
    }

    return NULL;
}

int main() {
    std_cmd_redir_t rd;
    strncpy(rd.path,"/tmp/com_test",sizeof(rd.path)-1);
    rd.func = do_loop_for_long_time;
    rd.param = NULL;
    rd.data = NULL;
    if (std_cmd_redir_init(&rd)==STD_ERR_OK) {
        sleep(500);
    }
    return 0;
}


