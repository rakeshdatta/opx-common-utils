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
 * filename: std_utils.h
 */


#ifndef __STD_UTILS_H
#define __STD_UTILS_H
/*!
 * \file   std_utils.h
 * \brief  Common utility functions
 * \date   4-2014
 */

#include <string.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/** \defgroup StringUtilities Common - String Utilities
 *  Miscellaneous string utilities
 *
 *  \{
 */
/**
  Safe strncpy: appends a 0 at the end of a destination string.

  \return pointer to the destination string dest
 */
static inline char *safestrncpy(
    char *dest,      /**< [out] destination string */
    const char *src, /**< [in] source string */
    size_t n         /**< [in] sizeof of destination string */)
{
    char *ret = strncpy(dest, src, n);
    dest[n-1] = 0;
    return ret;
}

/**
  Convert a value passed in as a string (strval) to an enum value (returned as integer).
  'keys' should be a table of the form:

  \verbatim

  typedef enum _my_enum_t {
      dn_enum_value1 = 0,
      dn_enum_value2,
      dn_enum_value3,
      DN_ENUM_MAX
  } my_enum_t;

  const char* my_keys[DN_ENUM_MAX] = {
     "value1",
     "value2",
     "value3",
  };

  char *val = "value1";
  //...
  my_enum_val = dn_std_string_to_enum(my_keys, DN_ENUM_MAX, val);

  \endverbatim

  The mapping is positional, so 'my_keys' strings must be defined
  in the same order as their counterparts enum values.

  \sa dn_std_enum_to_string

  \return enum value, -1 if not found
 */
int dn_std_string_to_enum(
    const char* keys[],    /**< [in] table containing string constants to be converted to enum values */
    unsigned int sz, /**< [in] table size (number of entries) */
    const char* strval     /**< [in] string value to be converted */);

/**
  Convert a value passed in as an enum value (integer) to its corresponding string value.
   Assumes 'keys' is defined as a 'static'/global variable (not automatic)

  \sa dn_std_string_to_enum
  \return enum value, -1 if not found
 */
const char* dn_std_enum_to_string(
    const char* keys[],        /**< [in] table containing string constants to be converted to enum values */
    const unsigned int sz,     /**< [in] table size (number of entries) */
    const unsigned int enumval /**< [in] string value to be converted */);


typedef struct {
    const unsigned int code;
    const char*        text;
} std_code_text_t;

/**
  Convert a string passed in to an code value.

  @param text sting to convert
  @param text_sz string length
  @param table table holds string and code mapping
  @param table_sz number of records in dn_std_code_txt_t table
  @return code value, -1 if not found
 */
int std_text_to_code(
    const char* text,
    const unsigned int text_sz,
    const std_code_text_t* table,
    const unsigned int table_sz);

/**
  Convert a code passed in as a code to its corresponding string value.

  @param code code to convert
  @param table table holds string and code mapping
  @param table_sz number of records in dn_std_code_txt_t table
  @return sting, NULL if not found
 */
const char* std_code_to_text(
    const unsigned int code,
    const std_code_text_t* table,
    const unsigned int table_sz);

/**
 * Type to handle the contents of a parsed string
 */
typedef void * std_parsed_string_t;

/**
 * Parse a standard string and return a handle to keep track of the data.
 * @param handle[out] the handle that will be used to iterate over the contents of the string
 * @param string string to parse (break into tokens)
 * @param delim the delimiter used to break the string into pieces
 * @return true if possible otherwise false
 */
bool std_parse_string(std_parsed_string_t * handle,const char * string, const char *delim);

/**
 * Get the number of parsed elements in the handle
 * @param parsed handle to a parsed string
 * @return number of elements
 */
size_t std_parse_string_num_tokens(std_parsed_string_t parsed);

/**
 * Iterate through the string.  This will return the current element specified by index
 * if valid, and then will increment index to point to the next entry.
 * @param parsed the handle to the parsed string
 * @param index[out] the current index to return (and incremented on return)
 * @return NULL if index is invalid (or past the last entry)
 */
const char * std_parse_string_next(std_parsed_string_t parsed, size_t *index);

/**
 * get the string at the index specified if it is valid
 * @param parsed the handle to the parsed string
 * @param index the current index to return
 * @return NULL if index is invalid (or past the last entry)
 */
const char * std_parse_string_at(std_parsed_string_t parsed, size_t index);


/**
 * Free up a parsed string previously parsed with the API
 * @param parsed the parsed string handle
 */
void std_parse_string_free(std_parsed_string_t parsed);



/**
 * The following are WS removal functions

  \verbatim
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
  \endverbatim
 */

/**
 * Removing the trailing whitespace or series of characters from a string.  This will
 * always return a valid char *
 * @param str must be non NULL character string that can be modified (no const char *)
 * @param ws_delim can be NULL if the user wants to check for isspace alternatelt can
 *        be a string of characters that if any are matched is considered whitespace
 *        eg.. "# " will search for both spaces and @
 * @return a valid character string
 */
char * std_remove_trailing_whitespace(char * str, char *ws_delim);

/**
 * this function will search the string starting at the end and working back for
 * either characters defined in ws_delim or isspace and remove them
 *
 * @param str must be non NULL character string that can be modified (no const char *)
 * @param ws_delim can be NULL if the user wants to check for isspace alternatelt can
 *        be a string of characters that if any are matched is considered whitespace
 *        eg.. "# " will search for both spaces and @
 * @return a valid character string
 */
char * std_remove_leading_whitespace(char * str, char *ws_delim);


/**
 * Assumes that a comments starts with a # and will remove all trailing whitespace
 * before comment but after the last non-space.  If no comment is found will removing
 * any trailing whtiespace (like running std_remove_trailing_whitespace(str,NULL)
 * @param str a valid input character string
 * @return a character string
 */
char * std_remove_comment_and_trailing_whitespace(char *str);

/**
 *  \}
 */
#ifdef __cplusplus
}
#endif

#endif
