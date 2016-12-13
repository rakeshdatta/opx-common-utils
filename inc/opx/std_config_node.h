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
 * filename: std_config_node.h
 */


/**
 * @file   std_config_node.h
 *
 * @brief  Defines utilities to parse and retrieve information from hierarchial
 *         configuration file.
 *         A hierarchial configration file is one which configuration is
 *         organized as set of nodes along with configuration of each node.
 *         A node can inturn contain other nodes.  In other words, nodes can be
 *         nested.
 *         @note The current implementation uses xml syntax to describe the configuration.
*/
#ifndef __STD_CONFIG_NODE_H_
#define __STD_CONFIG_NODE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief An opaque handle to an configuration node.
 */
typedef void * std_config_node_t;

/**
 * @brief An opaque handle to an configuration file object.
 */
typedef void * std_config_hdl_t;

/**
 * @brief Load the specified configuration file.
 *
 * This function loads the specified file and parses and stores it
 * in a form that enables easy retrieval of information using other API that
 * make up this module.
 *
 * @param[in] filename The name of the file that holds the configuration file.
 * @return NULL in case of error otherwise returns an handle for use in
 *         future when this configuration has to be referred.
 */
std_config_hdl_t std_config_load(const char *filename);

/**
 * @brief retrieves the root element of the configuration.

 * @param hdl configuration file handle as obtained using std_config_load.
 * @return NULL in case of errors, else returns the root element of the configuration.
 */
std_config_node_t std_config_get_root(std_config_hdl_t hdl);

/**
 * @brief retrieves the name of the node.
 *
 * @param node The node whose attribute value needs to be determined
 * @return returns the name of the specified node.
 */
const char *std_config_name_get(std_config_node_t node);

/**
 * @brief retrieves the value of the attribute specified for given node.
 *
 * @param node The node whose attribute value needs to be determined
 * @param attr The name of the attribute whose value has to be retrieved
 * @return returns NULL if the specified attribute is not available. If attribute is
 *   present, it returns string having value of the attribute.
 */
char *std_config_attr_get(std_config_node_t node, const char *attr);

/**
 * @brief retrieves the first child of the given node.
 *
 * @param node The node whose child node has to be determined
 * @return returns NULL if there are no children. Else, it returns the pointer to the first child.
 */
std_config_node_t std_config_get_child(std_config_node_t node);

/**
 * @brief retrieves the sibling of the given node.
 *
 * @param node The node whose sibiling node has to be determined
 * @return returns NULL if there are no sibilings. Else, it returns the pointer to the sibiling node.
 */
std_config_node_t std_config_next_node(std_config_node_t node);

/**
 * @brief frees up the memory and resources being used by this module for the specified handle
 *
 * @param hld The handle to the configuration object
 * @return returns None.
 */
void std_config_unload(std_config_hdl_t hdl);

/**
 * @brief iterate on every child node and apply specified operation on each node
 *
 * @param[in] node  Node whose children needs to be iterated for.
 * @param[in] fn    Callback function that operates on node and takes void
 *                    pointer as arg
 * @param[in] user_data data passed to callback function
 * @return returns None.
 */
void std_config_for_each_node(std_config_node_t node,
    void (*fn)(std_config_node_t node, void *user_data), void *user_data);

#ifdef __cplusplus
}
#endif

#endif  //__STD_CONFIG_NODE_H_
