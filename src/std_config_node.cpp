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
 * filename: std_config_node.c
 */



#include "std_config_node.h"
#include "std_assert.h"

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xinclude.h>
#include <pthread.h>
#include <memory>
#include <vector>
#include <mutex>

namespace {    //just for general parser cleanup
std::mutex &LOCK() {
    static std::mutex __lock;
    return __lock;
}
}

class __private {
    std::vector<char *> _cache;
    std::mutex _lock;
    ssize_t _cnt = 0;
public:
    std::mutex&lock() { return _lock; }

    char * insert(char *value) {
        std::lock_guard<std::mutex> lg(_lock);
        try {
            _cache.push_back(value);
        } catch (...) {
            free(value);
            value = nullptr;
        }
        return value;
    }

    void clear() {
        {
            std::lock_guard<std::mutex> lg(LOCK());
            xmlCleanupParser();
        }
        std::lock_guard<std::mutex> lg(_lock);
        for ( auto &it : _cache) {
            free(it);
        }
        _cache.clear();
    }
    void ref(ssize_t count) {
        _cnt+=count;
        if (_cnt==0) clear();
    }
    __private(){    }

    ~__private() {
        _cnt =0;
        clear();
    }
};

static __private &__get() { static __private *_d = new __private; return *_d; }

static void one_time_init() {
    //init memory and structures
    __get().clear();
}

std_config_hdl_t std_config_load(const char *filename) {
    static pthread_once_t _once_control = PTHREAD_ONCE_INIT;
    pthread_once(&_once_control,one_time_init);

    STD_ASSERT(filename != NULL);
    xmlDoc * _doc = xmlReadFile(filename, NULL, XML_PARSE_XINCLUDE);

    if (_doc!=nullptr) __get().ref(1);
    xmlXIncludeProcess(_doc);

    return _doc;
}

std_config_node_t std_config_get_root(std_config_hdl_t hdl)
{
    STD_ASSERT(hdl != NULL);
    return xmlDocGetRootElement(((xmlDoc*)hdl));
}

char *std_config_attr_get(std_config_node_t node, const char *attr)
{
    STD_ASSERT(node != NULL);

    char * prop_value =( char *)xmlGetProp((xmlNode *)node, (xmlChar *)attr);
    if (prop_value!=nullptr) {
        prop_value = __get().insert(prop_value);
    }
    return prop_value;
}

std_config_node_t std_config_get_child(std_config_node_t cfg_node)
{
    xmlNode *cur_node;
    xmlNode *node = (xmlNode *)cfg_node;

    STD_ASSERT(node != NULL);

    if (node->children == NULL)
    {
        return NULL;
    }

    cur_node=node->children;

    while ((cur_node) && (cur_node->type != XML_ELEMENT_NODE))
    {
        cur_node=cur_node->next;
    }
    return (std_config_node_t)cur_node;
}

std_config_node_t std_config_next_node(std_config_node_t cfg_node)
{
    xmlNode *node = (xmlNode *)cfg_node;

    STD_ASSERT(node != NULL);
    node=node->next;

    while ((node) && (node->type != XML_ELEMENT_NODE))
    {
        node=node->next;
    }
    return (std_config_node_t)node;
}

const char *std_config_name_get(std_config_node_t node)
{
    STD_ASSERT(node != NULL);
    return (char *)(((xmlNode *)node)->name);
}

void std_config_unload(std_config_hdl_t hdl)
{
    STD_ASSERT(hdl != NULL);

    /*free the document */
    xmlFreeDoc((xmlDoc *)hdl);

    __get().ref(-1);

}

void std_config_for_each_node(std_config_node_t node,
    void (*fn)(std_config_node_t node, void *user_data), void *user_data)
{
    std_config_node_t cur_node = NULL;

    STD_ASSERT(node != NULL);
    STD_ASSERT(fn != NULL);

    for (cur_node = std_config_get_child(node); cur_node != NULL;
        cur_node = std_config_next_node(cur_node)) {
        fn(cur_node, user_data);
    }
}
