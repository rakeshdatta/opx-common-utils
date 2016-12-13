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
 * filename: std_xml_parser.h
 */

/*!
 * \file   std_xml_parser.h
 * \brief  XML Parser wrapper routines for parsing xml files.
 * The APIs are modelled in the line of libxml2 just to facility easy
 * development for those who are familiar with libxml2 library.
 * \date   01-2015
 */

#ifndef _STD_XML_PARSER_H
#define _STD_XML_PARSER_H

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

/*---------------------------------------------------------------*\
 *                    Defines and Macros.
\*---------------------------------------------------------------*/


/*---------------------------------------------------------------*\
 *                    Data structures.
\*---------------------------------------------------------------*/

typedef xmlDocPtr std_xml_docptr; /// XML document object
typedef xmlnodePtr std_xml_nodeptr; /// XML node object
typedef xmlXPathContextPtr std_xml_xpath_ctxt_ptr; /// XML xpath context
typedef xmlXPathObjectPtr std_xml_xpath_obj_ptr; // XML xpath object
typedef xmlAttrPtr std_xml_attr_ptr; /// XML attribute set
typedef xmlBufferPtr std_xml_buf_ptr; /// XML buffer
typedef xmlNsPtr std_xml_ns_ptr; // XML name space object

enum std_xml_element_type {
    STD_XML_ELEMENT_NODE = XML_ELEMENT_NODE,
    STD_XML_ATTRIBUTE_NODE = XML_ATTRIBUTE_NODE,
    STD_XML_TEXT_NODE = XML_TEXT_NODE,
    STD_XML_CDATA_SECTION_NODE = XML_CDATA_SECTION_NODE,
    STD_XML_ENTITY_REF_NODE = XML_ENTITY_REF_NODE,
}; // More node types can be added as required

/*---------------------------------------------------------------*\
 *                    Prototypes with documentation.
\*---------------------------------------------------------------*/

/**
* @brief
*
* @param xmlver
*
* @return
*/
#define std_xml_new_doc(xmlver) (std_xml_docptr *) xmlNewDoc (BAD_CAST xmlver)

/**
* @brief
*
* @param modestr
*
* @return
*/
#define std_xml_new_node(modestr) (std_xml_new_node *) xmlNewNode (NULL, \
                        BAD_CAST modestr)

/**
* @brief
*
* @param doc
* @param node
*
* @return
*/
#define std_xmldoc_set_root (doc, node) xmlDocSetRootElement ((xmlDocPtr) doc, \
                        (xmlNodePtr) node)

/**
* @brief
*
* @param doc
*
* @return
*/
#define std_xml_xpath_new_ctxt (doc) (std_xml_xpath_ctxt_ptr) \
                        xmlXPathNewContext ((xmlDocPtr) doc)

/**
* @brief
*
* @param xpathstr
* @param ctxt
*
* @return
*/
#define std_xml_xpath_eval (xpathstr, ctxt) (std_xml_xpath_obj_ptr) \
                        xmlXPathEvalExpression (BAD_CAST xpathstr, \
                        (xmlXPathContextPtr) ctxt)

/**
* @brief
*
* @param leaf
* @param leafpath
*
* @return
*/
#define std_xml_new_child (leaf, leafpath) (std_xml_node_ptr) \
                        xmlNewChild ((xmlNodePtr)leaf, NULL, \
                        BAD_CAST leafpath, NULL)

/**
* @brief
*
* @param nodeset
*
* @return
*/
#define std_xml_xpath_results_isempty (nodeset) xmlXPathNodeSetIsEmpty (nodeset)


/**
* @brief
*
* @param xpathobj
*
* @return
*/
#define std_xml_xpath_free_obj (xpathobj) \
                        xmlXPathFreeObject ((std_xml_xpath_obj_ptr) xpathobj)

/**
* @brief
*
* @param node
* @param val
*
* @return
*/
#define std_xml_node_set_content (node, val) \
                        xmlNodeSetContent ((xmlNodePtr *)node, BAD_CAST val)

/**
* @brief
*
* @param node
* @param name
* @param value
*
* @return
*/
#define std_xml_new_prop (node, name, value) \
                        (std_xml_attr_ptr) xmlNewProp ((xmlNodePtr *)node,
                        BAD_CAST name, BAD_CAST value)

/**
* @brief
*
* @param file
* @param doc
*
* @return
*/
#define std_xml_file_save (file, doc, formatopt) xmlSaveFormatFile (file, \
                        (xmlDocPtr) doc, formatopt)

/**
* @brief
*
* @param buf
*
* @return
*/
#define std_xml_buffer_free (buf) xmlBufferFree ((xmlBufferPtr) buf)

/**
* @brief
*
* @param std_xml_buf_ptr
*
* @return
*/
#define std_xml_buffer_create (std_xml_buf_ptr) xmlBufferCreate

/**
* @brief
*
* @param doc
*
* @return
*/
#define std_xml_free_doc (doc) xmlFreeDoc ((xmlDocPtr) doc)

/**
* @brief
*
* @param node
* @param href
* @param prefix
*
* @return
*/
#define std_xml_new_ns (node, href, prefix) (std_xml_ns_ptr) xmlNewNs ( \
                        (std_xml_node_ptr) node, BAD_CAST href, BAD_CAST prefix)

/**
* @brief
*
* @param buf
* @param doc
* @param node
* @param level
* @param format
*
* @return
*/
#define xml_node_dump (buf, doc, node, level, format) \
                        xmlNodeDump ((xmlBufferPtr) buf, (xmlDocPtr) doc, \
                        (xmlNodePtr) node, level, format)

/**
* @brief
*
* @return
*/
#define std_xml_cleanup_parser xmlCleanupParser

/**
* @brief
*
* @return
*/
#define std_xml_memory_dump xmlCleanupParser

#endif /* _STD_XML_PARSER_H */
