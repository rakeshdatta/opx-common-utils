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
 * filename: std_string_utils.cpp
 */

/*!
 * \file   std_utils.c
 * \brief  Common utility functions
 * \date   May-2014
 */

#include "std_utils.h"
#include "std_assert.h"

#include <ctype.h>
#include <memory>
#include <vector>
#include <string>

int dn_std_string_to_enum(
    const char* keys[],
    unsigned int sz,
    const char* strval)
{
    unsigned int enum_val =0;

    if (NULL == keys) {
        return -1;
    }

    for (enum_val = 0; enum_val < sz; enum_val++) {
        if (strcmp(keys[enum_val], strval) == 0) {
            return (int)enum_val;
        }
    }

    return (int)-1;
}

const char* dn_std_enum_to_string(
    const char* keys[],
    const unsigned int sz,
    const unsigned int enumval)
{
    if (enumval < sz) {
        return keys[enumval];
    }
    return NULL;
}

int std_text_to_code(
    const char* text,
    const unsigned int text_sz,
    const std_code_text_t* table,
    const unsigned int table_sz)
{
    unsigned int idx;

    if (table == NULL)
        return -1;
    for (idx=0; idx<table_sz; idx++) {
        if (text_sz) {
            if (strncmp(table[idx].text, text, text_sz) == 0)
                return((int)table[idx].code);
        } else {
            if (strcmp(table[idx].text, text) == 0)
                return((int)table[idx].code);
        }
    }
    return -1;
}

const char* std_code_to_text(
    const unsigned int code,
    const std_code_text_t* table,
    const unsigned int table_sz)
{
    unsigned int idx;

    if (table == NULL)
        return NULL;

    for (idx=0; idx<table_sz; idx++) {
        if (table[idx].code == code) {
            return table[idx].text;
        }
    }
    return NULL;
}

bool std_parse_string(std_parsed_string_t * handle,const char * string, const char *delim) {

    std::auto_ptr<std::vector<std::string> > vect(new std::vector<std::string>);
    if (vect.get()==NULL) return false;
    try {
        std::string data(string);

        std::string::size_type delim_len = strlen(delim);
        std::string::size_type len = data.size();
        std::string::size_type ix = 0;
        for ( ; ix < len ; ) {
            size_t e = data.find(delim,ix);
            if (e==std::string::npos) {
                e = data.size();
            }
            std::string found = data.substr(ix,e-ix);
            const char * ptr = found.c_str();
            vect.get()->push_back(ptr);
            ix = e + delim_len;
        }
    } catch (...) {
        return false;
    }
    *handle = vect.release();
    return true;
}

std::vector<std::string> * TOV(std_parsed_string_t p) {
    return static_cast<std::vector<std::string> *>(p);
}

extern "C" {
size_t std_parse_string_num_tokens(std_parsed_string_t parsed) {
    std::vector<std::string> *p = TOV(parsed);
    return p->size();
}

const char * std_parse_string_at(std_parsed_string_t parsed, size_t index) {
    std::vector<std::string> *p = TOV(parsed);
    if (index<p->size()) {
        return p->at(index).c_str();
    }
    return NULL;
}

const char* std_parse_string_next(std_parsed_string_t parsed, size_t *index) {
    const char * p = std_parse_string_at(parsed,(*index));
    if (p!=NULL) ++(*index);
    return p;
}

void std_parse_string_free(std_parsed_string_t parsed) {
    delete TOV(parsed);
}

typedef int (*check_fun)(int, void*);

static int is_space_wrapper(int c, void *v) {
    return isspace(c);
}

static int is_char_match_wrapper(int c, void *v) {
    STD_ASSERT(v!=NULL);
    const char * fmt = (const char*)v;
    size_t ix = 0;
    size_t mx = strlen(fmt);
    STD_ASSERT(mx!=0);
    for ( ; ix < mx ; ++ix ) {
        if (c == fmt[ix]) return 1;
    }
    return 0;
}

static char *ltrim(char *str, check_fun fun, void * ctx ) {
    size_t ix = 0;
    size_t mx = strlen(str);

    for ( ; ix < mx ; ++ix ) {
        if (fun(str[ix],ctx)) continue;
        break;
    }

    if (ix!=0) memmove(str,str+ix,(mx-ix)+1);    //move the string include null
    return str;
}

static char *rtrim(char *str, check_fun fun, void * ctx ) {
    size_t mx = strlen(str);
    if (mx==0) return str;
    --mx; //set the starting position at the end of the stirng
    for ( ; mx != 0 ; --mx) {
        if (fun(str[mx],ctx)) continue;
        str[mx+1] = '\0';  //in all cases valid since we dec by 1 at the start
            //(ie even no spaces this will overwrite the null
        break;
    }
    return str;
}


char * std_remove_trailing_whitespace(char * str, char *ws_delim) {
    return rtrim(str,(ws_delim==NULL) ? is_space_wrapper:is_char_match_wrapper,
            ws_delim);
}
char * std_remove_leading_whitespace(char * str, char *ws_delim) {
    return ltrim(str,(ws_delim==NULL) ? is_space_wrapper:is_char_match_wrapper,
            ws_delim);
}

char * std_remove_comment_and_trailing_whitespace(char *str) {
    char * ptr = strchr(str,'#');
    if (ptr==NULL) ptr = str;
    else *ptr = '\0';
    return std_remove_trailing_whitespace(str,NULL);
}

}

