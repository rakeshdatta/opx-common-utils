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
 * filename: std_config_file.h
 */


/*
 * std_config_file.h
 */

#ifndef STD_CONFIG_FILE_H_
#define STD_CONFIG_FILE_H_

#include "std_type_defs.h"
#include "std_error_codes.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 *
 * This is a library that will parse config files.  An example is shown below.
@verbatim

# this is just an example
# there can be comments before the first group

[First Group]

Name=Key File Example\tthis value shows\nescaping

# localized strings are stored in multiple key-value pairs
Welcome=Hello
Welcome[de]=Hallo
Welcome[fr_FR]=Bonjour
Welcome[it]=Ciao
Welcome[be@latin]=Hello

[Another Group]

Numbers=2;20;-200;0

Booleans=true;false;true;true
@endverbatim

 *
 */

/**
 * This is the location of the standard NGOS configuration files
 */
#define STD_CFG_FILE_LOCATION "/etc/opt/dell/os10"

/*
 * @brief the config file handle
 * */
typedef void * std_cfg_file_handle_t;

/**
 * @brief open an existing file and return the handle.  If the file doesn't
 * exist the t_std_error will have ENOEXIST in the private part and
 * the recommendation is to create a new config file with
 * std_config_file_create.
 *
 * @param handle pointer to a handle
 * @param name of the file
 * @return standard error code
 */
t_std_error std_config_file_open(std_cfg_file_handle_t *handle, const char *name);

/**
 * @brief create an empty container for a config file
 * @param handle handle to file created
 * @return standard error code
 */
t_std_error std_config_file_create(std_cfg_file_handle_t *handle);

/**
 * @brief get a key's data from a config file
 * @param handle handle to the file
 * @param group name of the group you care about i.e. hal-port-config for [hal-port-config]
 * @param key the actual key data your looking for eg. key = data
 * @return standard error code
 */
const char * std_config_file_get(std_cfg_file_handle_t handle,
                const char *group, const char *key);

/**
 * @brief set a file in the config file
 * @param handle handle to the file
 * @param group the name of the group
 * @param key the key file
 * @param str the string to set
 * @return  standard error code
 */
t_std_error std_config_file_set(std_cfg_file_handle_t handle,
                const char *group, const char *key, const char *str);

/**
 * Get the number of keys contained by a group in the file
 * @param handle the handle to the config file
 * @param group the group being checked
 * @return the number of keys
 */
size_t std_config_file_get_num_keys(std_cfg_file_handle_t handle,
        const char *group);

/**
 * Get the list of keys in a group
 * @param handle handle to the config file
 * @param group group to query
 * @param key_list a array of const char * holding the strings
 * @param len[out] length of the key_list array (modified to give real size back)
 * @return standard return code
 */
t_std_error std_config_file_get_keys(std_cfg_file_handle_t handle,
        const char *group, const char **key_list, size_t *len);

/**
 * @brief write the config to a new file
 * @param handle to the config file
 * @param name of the file to write /etc/opt/dell/os10/xxx
 * @return standard error code
 */
t_std_error std_config_file_write(std_cfg_file_handle_t handle,
                const char *name);

/**
 * @brief close the file handle and clean up.. must be called for any open config
 * @param handle handle to the file
 * @return standard error code
 */
t_std_error std_config_file_close(std_cfg_file_handle_t handle);

#ifdef __cplusplus
}
#endif


#endif /* STD_CONFIG_FILE_H_ */
