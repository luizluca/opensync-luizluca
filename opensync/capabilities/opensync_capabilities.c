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

#include "opensync-capabilities.h"
#include "opensync-capabilities_internals.h"

#include "opensync-group.h"

#include "opensync_capability_private.h"		/* FIXME: direct access of private header */

#include "opensync_capabilities_private.h"

OSyncCapabilitiesObjType *osync_capabilitiesobjtype_new(OSyncCapabilities *capabilities, xmlNodePtr node, OSyncError **error)
{
	OSyncCapabilitiesObjType *objtype = NULL;
	osync_assert(capabilities);
	osync_assert(node);
	
	objtype = osync_try_malloc0(sizeof(OSyncCapabilitiesObjType), error);
	if(!objtype) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
		return NULL;
	}
	
	objtype->child_count = 0;
	objtype->first_child = NULL;
	objtype->last_child = NULL;
	objtype->next = NULL;
	objtype->node = node;
	
	if(!capabilities->first_objtype)
		capabilities->first_objtype = objtype;
	if(capabilities->last_objtype)
		capabilities->last_objtype->next = objtype;
	capabilities->last_objtype = objtype;

	return objtype;
}

OSyncCapabilitiesObjType *osync_capabilitiesobjtype_get(OSyncCapabilities *capabilities, const char *objtype)
{
	OSyncCapabilitiesObjType *tmp = NULL;
	osync_assert(capabilities);
	osync_assert(objtype);
	
	tmp = capabilities->first_objtype;
	for(; tmp != NULL; tmp = tmp->next) {
		if(!strcmp((const char *)tmp->node->name, objtype))
			break;
	}	
	return tmp;	
}

/* end of private part */

OSyncCapabilities *osync_capabilities_new(OSyncError **error)
{
	OSyncCapabilities *capabilities = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, error);
	
	capabilities = osync_try_malloc0(sizeof(OSyncCapabilities), error);
	if(!capabilities) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
		return NULL;
	}
	
	capabilities->ref_count = 1;
	capabilities->first_objtype = NULL;
	capabilities->last_objtype = NULL;
	capabilities->doc = xmlNewDoc(BAD_CAST "1.0");
	capabilities->doc->children = xmlNewDocNode(capabilities->doc, NULL, (xmlChar *)"capabilities", NULL);
	capabilities->doc->_private = capabilities;
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, capabilities);
	return capabilities;
}

OSyncCapabilities *osync_capabilities_parse(const char *buffer, unsigned int size, OSyncError **error)
{
	OSyncCapability *capability = NULL;
	OSyncCapabilities *capabilities = NULL;
	xmlNodePtr cur = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %u, %p)", __func__, buffer, size, error);
	osync_assert(buffer);
	
	capabilities = osync_try_malloc0(sizeof(OSyncCapabilities), error);
	if(!capabilities) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
		return NULL;
	}

	capabilities->ref_count = 1;
	capabilities->first_objtype = NULL;
	capabilities->last_objtype = NULL;	
	capabilities->doc = xmlReadMemory(buffer, size, NULL, NULL, XML_PARSE_NOBLANKS);
	if(capabilities->doc == NULL) {
		osync_free(capabilities);
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Could not parse XML.");
		goto error;
	}
	capabilities->doc->_private = capabilities;
	
	cur = xmlDocGetRootElement(capabilities->doc);

	cur = cur->children;
	for(; cur != NULL; cur = cur->next) {
		OSyncCapabilitiesObjType *capabilitiesobjtype = osync_capabilitiesobjtype_new(capabilities, cur, error);
		if(!capabilitiesobjtype) {
			osync_capabilities_unref(capabilities);
			goto error;
		}

		if (!(capability = osync_capability_new_node(capabilitiesobjtype, cur, error)))
			goto error;
		
		if (!osync_capability_parse(capability, cur->children, &capabilitiesobjtype->first_child, &capabilitiesobjtype->last_child, &capabilitiesobjtype->child_count, error))
			goto error_and_free;

	}
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, capabilities);
	return capabilities;

error_and_free:
	if (capability)
		osync_capability_free(capability);
error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
	return NULL;
}

OSyncCapabilities *osync_capabilities_ref(OSyncCapabilities *capabilities)
{
	osync_assert(capabilities);
	
	g_atomic_int_inc(&(capabilities->ref_count));

	return capabilities;
}

void osync_capabilities_unref(OSyncCapabilities *capabilities)
{
	osync_assert(capabilities);
			
	if (g_atomic_int_dec_and_test(&(capabilities->ref_count))) {
		OSyncCapabilitiesObjType *objtype, *tmp;
		objtype = capabilities->first_objtype;
		while(objtype)
			{
				osync_capability_free(objtype->capability);

				tmp = objtype->next;
				osync_free(objtype);
				objtype = tmp;
			}
		osync_xml_free_doc(capabilities->doc);
		osync_free(capabilities);
	}
}

OSyncCapability *osync_capabilities_get_first(OSyncCapabilities *capabilities, const char *objtype)
{
	OSyncCapability *res = NULL;
	OSyncCapabilitiesObjType *tmp = NULL;
	osync_assert(capabilities);
	osync_assert(objtype);
	
	tmp = osync_capabilitiesobjtype_get(capabilities, objtype);
	if(tmp)
		res = tmp->first_child;
	return res;
}	

osync_bool osync_capabilities_assemble(OSyncCapabilities *capabilities, char **buffer, int *size)
{
	osync_assert(capabilities);
	osync_assert(buffer);
	osync_assert(size);
	
	xmlDocDumpFormatMemoryEnc(capabilities->doc, (xmlChar **) buffer, size, NULL, 1);
	return TRUE;
}

void osync_capabilities_sort(OSyncCapabilities *capabilities)
{
	unsigned int index;
	OSyncCapabilitiesObjType *objtype;
	OSyncCapability *cur;
		
	objtype = capabilities->first_objtype;
	for(; objtype != NULL; objtype = objtype->next)
		{
			void **list = NULL;
			if(objtype->child_count <= 1)
				continue;
	
			list = g_malloc0(sizeof(OSyncCapability *) * objtype->child_count);
	
			index = 0;
			for(cur = objtype->first_child; cur != NULL; cur = osync_capability_get_next(cur)) {
				list[index] = cur;
				index++;
				xmlUnlinkNode(cur->node);
			}
	
			qsort(list, objtype->child_count, sizeof(OSyncCapability *), osync_capability_compare_stdlib);
	
			/** bring the capabilities and xmldoc in a consistent state */
			objtype->first_child = ((OSyncCapability *)list[0])->node->_private;
			objtype->last_child = ((OSyncCapability *)list[objtype->child_count - 1])->node->_private;

			for(index = 0; index < objtype->child_count; index++) {
				cur = (OSyncCapability *)list[index];
				xmlAddChild(objtype->node, cur->node);
			
				if(index < objtype->child_count-1)
					cur->next = (OSyncCapability *)list[index+1];
				else
					cur->next = NULL;
			
				if(index)
					cur->prev = (OSyncCapability *)list[index-1];
				else
					cur->prev = NULL;
			}
		
			g_free(list);
		}
}

OSyncCapabilities *osync_capabilities_load(const char *file, OSyncError **error)
{
	unsigned int size;
	char *buffer, *filename;
	OSyncCapabilities *capabilities;
	osync_bool b;

	osync_trace(TRACE_ENTRY, "%s(%s, %p)", __func__, file, error);
	osync_assert(file);
	
	filename = osync_strdup_printf("%s%c%s", OPENSYNC_CAPABILITIESDIR, G_DIR_SEPARATOR, file);
	
	b = osync_file_read(filename, &buffer, &size, error);
	osync_free(filename);
	if(!b) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
		return NULL;
	}
	
	capabilities = osync_capabilities_parse(buffer, size, error);
	osync_free(buffer);
	if(!capabilities) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
		return NULL;
	}
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, capabilities);
	return capabilities;
}

osync_bool osync_capabilities_member_has_capabilities(OSyncMember *member)
{
	char *filename = NULL;
	gboolean res = FALSE;

	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, member);
	osync_assert(member);
	
	filename = osync_strdup_printf("%s%ccapabilities.xml", osync_member_get_configdir(member), G_DIR_SEPARATOR);
	res = g_file_test(filename, G_FILE_TEST_IS_REGULAR);
	osync_free(filename);
	
	osync_trace(TRACE_EXIT, "%s: %i", __func__, res);
	return res;
}

OSyncCapabilities* osync_capabilities_member_get_capabilities(OSyncMember *member, OSyncError** error)
{
	unsigned int size;
	char* buffer, *filename;
	OSyncCapabilities *capabilities;
	osync_bool res;

	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, member, error);
	osync_assert(member);
	
	filename = osync_strdup_printf("%s%ccapabilities.xml", osync_member_get_configdir(member), G_DIR_SEPARATOR);
	res = osync_file_read(filename, &buffer, &size, error);
	osync_free(filename);
	
	if(!res) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
		return NULL;
	}
	
	capabilities = osync_capabilities_parse(buffer, size, error);
	osync_free(buffer);
	if(!capabilities) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
		return NULL;
	}

	/* Sort capabilities, to avoid user modified unsorted capalities: #813 */
	osync_capabilities_sort(capabilities);
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, capabilities);
	return capabilities;
}

osync_bool osync_capabilities_member_set_capabilities(OSyncMember *member, OSyncCapabilities* capabilities, OSyncError** error)
{
	int size;
	char* buffer, *filename;
	osync_bool res;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, member, capabilities, error);
	osync_assert(member);
	osync_assert(capabilities);
	
	osync_capabilities_assemble(capabilities, &buffer, &size);
	filename = osync_strdup_printf("%s%ccapabilities.xml", osync_member_get_configdir(member), G_DIR_SEPARATOR);
	res = osync_file_write(filename, buffer, size, 0600, error);
	osync_free(filename);
	osync_free(buffer);
	if(!res) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
		return FALSE;
	}
	
	osync_trace(TRACE_EXIT, "%s: %i", __func__, res);
	return res;
}

