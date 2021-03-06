/*
 * libopensync - A synchronization framework
 * Copyright (C) 2006  NetNix Finland Ltd <netnix@netnix.fi>
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
 * Author: Daniel Friedrich <daniel.friedrich@opensync.org>
 * 
 */
 
#include "opensync.h"
#include "opensync_internals.h"

#include "opensync-xmlformat.h"

#include "opensync-xmlformat_internals.h"
#include "opensync_xmlformat_private.h"

#include "opensync_xmlfield_private.h"		/* FIXME: direct access of private header */

const char *osync_xmlformat_root_name(OSyncXMLFormat *xmlformat)
{
	osync_assert(xmlformat);
	
	return (const char *)xmlDocGetRootElement(xmlformat->doc)->name;
}

const char *osync_xmlformat_get_objtype(OSyncXMLFormat *xmlformat)
{
	osync_assert(xmlformat);
	
	return osync_xmlformat_root_name(xmlformat);
}

OSyncXMLFormat *osync_xmlformat_new(const char *objtype, OSyncError **error)
{
	OSyncXMLFormat *xmlformat = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, objtype, error);
	osync_assert(objtype);
	
	xmlformat = osync_try_malloc0(sizeof(OSyncXMLFormat), error);
	if (!xmlformat)
		goto error;

	xmlformat->doc = xmlNewDoc(BAD_CAST "1.0");
	xmlformat->doc->children = xmlNewDocNode(xmlformat->doc, NULL, BAD_CAST objtype, NULL);
	xmlformat->ref_count = 1;
	xmlformat->first_child = NULL;
	xmlformat->last_child = NULL;
	xmlformat->child_count = 0;
	xmlformat->sorted = FALSE;
	xmlformat->doc->_private = xmlformat;

	if (!(xmlformat->xmlfield = osync_xmlfield_new_node(xmlformat->doc->children, error)))
		goto error;
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, xmlformat);
	return xmlformat;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
	return NULL;
}

OSyncXMLFormat *osync_xmlformat_parse(const char *buffer, unsigned int size, OSyncError **error)
{
	OSyncXMLFormat *xmlformat = NULL;
	OSyncXMLField *xmlfield = NULL;
	xmlNodePtr cur = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %i, %p)", __func__, buffer, size, error);
	osync_assert(buffer);

	xmlformat = osync_try_malloc0(sizeof(OSyncXMLFormat), error);
	if (!xmlformat)
		goto error;
	
	xmlformat->doc = xmlReadMemory(buffer, size, NULL, NULL, XML_PARSE_NOBLANKS);
	if(!xmlformat->doc) {
		osync_free(xmlformat);
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Could not parse XML.");
		goto error;
	}

	xmlformat->ref_count = 1;
	xmlformat->doc->_private = xmlformat;

	
	cur = xmlDocGetRootElement(xmlformat->doc);
	if (!(xmlfield = osync_xmlfield_new_node(cur, error)))
		goto error;

	if (!osync_xmlfield_parse(xmlfield, cur->children, &xmlformat->first_child, &xmlformat->last_child, error))
		goto error;

	xmlformat->xmlfield = xmlfield;
	/* TODO: get key-count value with a sepcial parase function which
	 * counts the first-level, by incrementing the node->doc->_private pointer
	 */
	xmlformat->child_count = osync_xmlfield_get_key_count(xmlfield);

	osync_trace(TRACE_EXIT, "%s: %p", __func__, xmlformat);
	return xmlformat;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
	return NULL;
}

OSyncXMLFormat *osync_xmlformat_ref(OSyncXMLFormat *xmlformat)
{
	osync_assert(xmlformat);
	
	g_atomic_int_inc(&(xmlformat->ref_count));

	return xmlformat;
}

void osync_xmlformat_unref(OSyncXMLFormat *xmlformat)
{
	osync_assert(xmlformat);
	
	if (g_atomic_int_dec_and_test(&(xmlformat->ref_count))) {
		// free the children
		OSyncXMLField *cur, *tmp;
		cur = xmlformat->first_child;
		while(cur != NULL)
			{
				tmp = osync_xmlfield_get_next(cur);
				osync_xmlfield_delete(cur);
				cur = tmp;
			}

		// free the root OSyncXMLField, but NOT the actual node,
		// which is part of the above tree and already freed
		//
		// blank the node pointer (already freed above)
		xmlformat->xmlfield->node = NULL;
		// free the OSyncXMLField object
		osync_xmlfield_free(xmlformat->xmlfield);

		// free the XML document, which frees the xml Doc tree
		osync_xml_free_doc(xmlformat->doc);

		osync_free(xmlformat);
	}
}

OSyncXMLField *osync_xmlformat_get_first_field(OSyncXMLFormat *xmlformat)
{
	osync_assert(xmlformat);
	
	return xmlformat->first_child;
}

OSyncXMLFieldList *osync_xmlformat_search_field(OSyncXMLFormat *xmlformat, const char *name, OSyncError **error, ...)
{
	int index;
	void *ret;
	OSyncXMLField *cur, *key, *res;
	OSyncXMLFieldList *xmlfieldlist = NULL;
	void **liste = NULL;
	osync_bool all_attr_equal;

	osync_trace(TRACE_ENTRY, "%s(%p, %s, %p, ...)", __func__, xmlformat, name, error);
	osync_assert(xmlformat);
	osync_assert(name);
	
	/* Searching breaks if the xmlformat is not sorted (bsearch!).
		 ASSERT in development builds (when NDEBUG is not defined) - see ticket #754. */
	osync_assert(xmlformat->sorted);
	if (!xmlformat->sorted) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "XMLFormat is unsorted. Search result would be not valid.");
		goto error;
	}

	xmlfieldlist = osync_xmlfieldlist_new(error);
	if (!xmlfieldlist)
		goto error;

	if (xmlformat->child_count == 0)
		goto empty;	/* nothing to do */

	liste = osync_try_malloc0(sizeof(OSyncXMLField *) * xmlformat->child_count, error);
	if (!liste)
		goto error;

	index = 0;
	cur = osync_xmlformat_get_first_field(xmlformat);
	for (; cur != NULL; cur = osync_xmlfield_get_next(cur)) {
		liste[index] = cur;
		index++;
	}

	key = osync_try_malloc0(sizeof(OSyncXMLField), error);
	if (!key) {
		g_free(liste);
		goto error;
	}

	key->node = xmlNewNode(NULL, BAD_CAST name);
	
	ret = bsearch(&key, liste, xmlformat->child_count, sizeof(OSyncXMLField *), osync_xmlfield_compare_stdlib);

	/* no result - return empty xmlfieldlist */
	if (!ret)
		goto end;

	/* if ret is valid pointer (not NULL!) - reference it here. avoid segfaults */
	res = *(OSyncXMLField **) ret;

	/* we set the cur ptr to the first field from the fields with name name because -> bsearch -> more than one field with the same name*/
	for (cur = res; cur->prev != NULL && !strcmp(osync_xmlfield_get_name(cur->prev), name); cur = cur->prev) ;

	for (; cur != NULL && !strcmp(osync_xmlfield_get_name(cur), name); cur = cur->next) {
		const char *attr, *value;
		va_list args;
		all_attr_equal = TRUE;
		va_start(args, error);
		do {
			attr = va_arg(args, char *);
			value = va_arg(args, char *);
			if (attr == NULL || value == NULL)
				break;

			if (strcmp(value, osync_xmlfield_get_attr(cur, attr)) != 0)
				all_attr_equal = FALSE;
		} while (1);
		va_end(args);

		if(all_attr_equal)
			osync_xmlfieldlist_add(xmlfieldlist, cur);
	}

 end:	
	/* free lists here (later) - bsearch result is still pointing in liste array */
	xmlFreeNode(key->node);
	g_free(key);
	g_free(liste);
 empty:
	osync_trace(TRACE_EXIT, "%s: %p", __func__, xmlfieldlist);
	return xmlfieldlist;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
	return NULL;
}

osync_bool osync_xmlformat_assemble(OSyncXMLFormat *xmlformat, char **buffer, unsigned int *size, OSyncError **error)
{
	xmlChar *locbuffer = NULL;
	int locsize = 0;

	osync_assert(xmlformat);
	osync_assert(buffer);
	osync_assert(size);

	xmlDocDumpFormatMemoryEnc(xmlformat->doc, &locbuffer, &locsize, NULL, 1);

	if (!locbuffer || locsize < 0)
		goto error;

	// convert buffer into osync memory, so the caller can free it
	// with osync_free instead of xmlFree
	*buffer = osync_strdup((char*)locbuffer);
	*size = locsize;

	xmlFree(locbuffer);

	return TRUE;

error:
	return FALSE;
}

osync_bool osync_xmlformat_sort(OSyncXMLFormat *xmlformat, OSyncError **error)
{
	int index;
	OSyncXMLField *cur;
	void **list = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, xmlformat);
	osync_assert(xmlformat);
	
	if(xmlformat->child_count <= 1) {
		osync_trace(TRACE_INTERNAL, "child_count <= 1 - no need to sort");
		goto end;
	}
	
	list = osync_try_malloc0(sizeof(OSyncXMLField *) * xmlformat->child_count, error);
	if (!list)
		goto error;
	
	index = 0;
	cur = osync_xmlformat_get_first_field(xmlformat);
	for(; cur != NULL; cur = osync_xmlfield_get_next(cur)) {
		list[index] = cur;
		index++;
		xmlUnlinkNode(cur->node);
	}
	
	qsort(list, xmlformat->child_count, sizeof(OSyncXMLField *), osync_xmlfield_compare_stdlib);
	
	/** bring the xmlformat and the xmldoc in a consistent state */
	xmlformat->first_child = ((OSyncXMLField *)list[0])->node->_private;
	xmlformat->last_child = ((OSyncXMLField *)list[xmlformat->child_count-1])->node->_private;

	for(index = 0; index < xmlformat->child_count; index++) {
		cur = (OSyncXMLField *)list[index];
		xmlAddChild(xmlDocGetRootElement(xmlformat->doc), cur->node);
			
		if(index < xmlformat->child_count-1)
			cur->next = (OSyncXMLField *)list[index+1];
		else
			cur->next = NULL;
		
		if(index)
			cur->prev = (OSyncXMLField *)list[index-1];
		else
			cur->prev = NULL;
	}
	g_free(list);

 end:	
	xmlformat->sorted = TRUE;
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_xmlformat_is_sorted(OSyncXMLFormat *xmlformat)
{
	OSyncXMLField *cur, *prev = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, xmlformat);
	osync_assert(xmlformat);
	
	/* No need to check if sorted when 1 or less xmlfields */
	if (xmlformat->child_count <= 1)
		return TRUE;
	
	cur = osync_xmlformat_get_first_field(xmlformat);
	for(; cur != NULL; cur = osync_xmlfield_get_next(cur)) {

		/* Equal when returns 0, like strcmp() */
		if (prev && osync_xmlfield_compare_stdlib(&prev, &cur) > 0)
			return FALSE;

		prev = cur;
	}

	return TRUE;
}

void osync_xmlformat_set_unsorted(OSyncXMLFormat *xmlformat)
{
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, xmlformat);
	osync_assert(xmlformat);

	xmlformat->sorted = FALSE;

	osync_trace(TRACE_EXIT, "%s", __func__);
}

osync_bool osync_xmlformat_copy(OSyncXMLFormat *source, OSyncXMLFormat **destination, OSyncError **error)
{
	char *buffer = NULL;
	unsigned int size;

	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, source, destination);

	if (!osync_xmlformat_assemble(source, &buffer, &size, error))
		goto error;

	*destination = osync_xmlformat_parse(buffer, size, error);
	if (!(*destination))
		goto error;

	if (source->sorted)
		(*destination)->sorted = TRUE;

	g_free(buffer);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

unsigned int osync_xmlformat_size()
{
	return sizeof(OSyncXMLFormat);
}

