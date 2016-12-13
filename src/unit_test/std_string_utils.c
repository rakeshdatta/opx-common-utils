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
 * filename: std_string_utils.c
 */

#include "std_utils.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char**argv) {

    char *msg="";
    char *tok=",";
    if (argc>1) msg=argv[1];
    if (argc>2) tok = argv[2];

    std_parsed_string_t handle;

    if (!std_parse_string(&handle,"nospacesinhere"," ")) {
        return -1;
    }
    printf("Found %d tokens - in no token test \n",(int)std_parse_string_num_tokens(handle));
    std_parse_string_free(handle);


    if (std_parse_string(&handle,msg,tok)) {
        size_t ix = std_parse_string_num_tokens(handle);
        printf("Found %d elements in %s\n",(int)ix,msg);

        const char * ptr = NULL;
        ix = 0;
        while((ptr=std_parse_string_next(handle,&ix))) {
            printf("Discovered %d:%s\n",(int)ix,ptr);
        }
        std_parse_string_free(handle);
    }
    printf("Remove WS test...\n");
    char buff[1024];
    snprintf(buff,sizeof(buff),"      Cliffasdasdasdasd         ");
    printf("Leading... --%s--\n",std_remove_leading_whitespace(buff,NULL));
    printf("Trailing... --%s--\n",std_remove_trailing_whitespace(buff,NULL));

    snprintf(buff,sizeof(buff),"#      Cliffasdasdasdasd         @#^");
    printf("Leading... --%s--\n",std_remove_leading_whitespace(buff,NULL));
    printf("Trailing... --%s--\n",std_remove_trailing_whitespace(buff,NULL));
    printf("Leading... --%s--\n",std_remove_leading_whitespace(buff,"# "));
    printf("Trailing... --%s--\n",std_remove_trailing_whitespace(buff,"#^"));
    printf("Trailing... --%s--\n",std_remove_trailing_whitespace(buff,"#%@"));
    printf("Trailing... --%s--\n",std_remove_trailing_whitespace(buff,"#%@ "));


    snprintf(buff,sizeof(buff),"      Cliffasdasdasdasd         # Some comment");
    printf("Trailing... --%s--\n",std_remove_comment_and_trailing_whitespace(buff));
    snprintf(buff,sizeof(buff),"      Cliffasdasdasdasd        non comment ");
    printf("Trailing... --%s--\n",std_remove_comment_and_trailing_whitespace(buff));
    snprintf(buff,sizeof(buff),"#      Cliffasdasdasdasd        non comment ");
    printf("Trailing... --%s--\n",std_remove_comment_and_trailing_whitespace(buff));

    return 0;
}

