/*
 * libopensync - A synchronization framework
 * Copyright (C) 2004-2005  Armin Bauer <armin.bauer@opensync.org>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 * 
 */

#ifndef _OPENSYNC_XML_H
#define _OPENSYNC_XML_H

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xmlschemas.h>

#include <opensync/opensync.h>
#include <string.h>
#include <stdio.h>

/**
 * @defgroup OSyncXMLInternalAPI OpenSync XML Internals
 * @ingroup OSyncCommonPrivate
 */

/*@{*/

typedef enum osxmlEncoding {
	OSXML_8BIT = 0,
	OSXML_QUOTED_PRINTABLE = 1,
	OSXML_BASE64 = 2
} osxmlEncoding;

typedef enum osxmlCharset {
	OSXML_ASCII = 0,
	OSXML_UTF8 = 1
} osxmlCharset;

typedef struct OSyncXMLEncoding OSyncXMLEncoding;
struct OSyncXMLEncoding {
	osxmlEncoding encoding;
	osxmlCharset charset;
};

typedef struct OSyncXMLScore {
	int value;
	const char *path;
} OSyncXMLScore;

void osync_xml_free(void *ptr);
void osync_xml_free_doc(xmlDoc *doc);

xmlNode *osync_xml_node_add_root(xmlDoc *doc, const char *name);
xmlNode *osync_xml_node_get_root(xmlDoc *doc, const char *name, OSyncError **error);
xmlNode *osync_xml_get_node(xmlNode *parent, const char *name);

xmlNode *osync_xml_node_add(xmlNode *parent, const char *name, const char *data);
//xmlNode *osync_xml_format_parse(const char *input, int size, const char *rootname, OSyncError **error);
xmlChar *osync_xml_find_node(xmlNode *parent, const char *name);
void osync_xml_node_add_property(xmlNode *parent, const char *name, const char *data);
char *osync_xml_find_property(xmlNode *parent, const char *name);
osync_bool osync_xml_has_property(xmlNode *parent, const char *name);
osync_bool osync_xml_has_property_full(xmlNode *parent, const char *name, const char *data);

void osync_xml_node_mark_unknown(xmlNode *parent);
void osync_xml_node_remove_unknown_mark(xmlNode *node);
void osync_xml_map_unknown_param(xmlNode *node, const char *paramname, const char *newname);

void osync_xml_node_set(xmlNode *node, const char *name, const char *data, OSyncXMLEncoding encoding);
xmlXPathObject *osync_xml_get_nodeset(xmlDoc *doc, const char *expression);
xmlXPathObject *osync_xml_get_unknown_nodes(xmlDoc *doc);
OSyncConvCmpResult osync_xml_compare(xmlDoc *leftinpdoc, xmlDoc *rightinpdoc, OSyncXMLScore *scores, int default_score, int treshold);

/**
 * @brief Dumps the XML tree to a string 
 * @param doc the XML doc value 
 * @return String of XML the tree (the caller is responsible for freeing) 
 */
char *osync_xml_write_to_string(xmlDoc *doc);
osync_bool osync_xml_copy(const char *input, unsigned int inpsize, char **output, unsigned int *outpsize, OSyncError **error);

osync_bool osync_xml_validate_document(xmlDocPtr doc, char *schemafilepath);

/**
 * @brief Help method which return the content of a xmlNode
 * @param node The pointer to a xmlNode
 * @return The value of the xmlNode or a empty string
 */
xmlChar *osync_xml_node_get_content(xmlNodePtr node);

/**
 * @brief Help method which return the content of a xmlAttr
 * @param node The pointer to a xmlAttr
 * @return The value of the xmlAttr or a empty string
 */
xmlChar *osync_xml_attr_get_content(xmlAttrPtr node);

/*! @brief Opens a xml document
 * 
 * Opens a xml document
 * 
 * @param doc Pointer to a xmldoc
 * @param cur The pointer to the first node
 * @param path The path of the document
 * @param topentry the name of the top node
 * @param error Pointer to a error struct
 * @returns TRUE if successful, FALSE otherwise
 * 
 */
osync_bool osync_xml_open_file(xmlDocPtr *doc, xmlNodePtr *cur, const char *path, const char *topentry, OSyncError **error);

/*@}*/

#endif /*_OPENSYNC_XML_H*/
