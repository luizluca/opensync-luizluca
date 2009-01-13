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

#include "opensync_capabilities_private.h" /* FIXME: include forgein private-header */

#include "opensync_capability_private.h"

OSyncCapability *osync_capability_new_node(OSyncCapabilitiesObjType *capabilitiesobjtype, xmlNodePtr node, OSyncError **error)
{
	OSyncCapability *capability = NULL;
	osync_assert(capabilitiesobjtype);
	osync_assert(node);
	
	capability = osync_try_malloc0(sizeof(OSyncCapability), error);
	if(!capability) {
		osync_trace(TRACE_ERROR, "%s: %s" , __func__, osync_error_print(error));
		return NULL;
	}
	
	capability->next = NULL;
	capability->node = node;
	capability->prev = capabilitiesobjtype->last_child;
	node->_private = capability;
	
	capabilitiesobjtype->capability = capability;
	
	return capability;
}

OSyncCapability *osync_capability_new_capability(OSyncCapability *parent, xmlNodePtr node, OSyncError **error)
{
	OSyncCapability *capability = NULL;
	osync_assert(node);
	
	capability = osync_try_malloc0(sizeof(OSyncCapability), error);

	if (!capability) {
		osync_trace(TRACE_ERROR, "%s: %s" , __func__, osync_error_print(error));
		return NULL;
	}
	
	capability->next = NULL;
	capability->node = node;
	capability->parent = parent;
	node->_private = capability;

	if (parent) {
		capability->prev = parent->child;
		if (capability->prev)
			capability->prev->next = capability;
		parent->child = capability;
	}
	
	return capability;
}

osync_bool osync_capability_parse(OSyncCapability *parent, xmlNodePtr node, OSyncCapability **first_child, OSyncCapability **last_child, unsigned int *child_count, OSyncError **error)
{
	OSyncCapability *capability = NULL;
	unsigned int count = 0;

	if (first_child)
		*first_child = NULL;

	while (node != NULL) {

		capability = osync_capability_new_capability(parent, node, error);
		if (!capability)
			goto error;

		if (first_child && !(*first_child)) {
			*first_child = capability;
		}

		count++;

		if (node->children && node->children->type == XML_ELEMENT_NODE)
			if (!osync_capability_parse(capability, node->children, NULL, NULL, NULL, error))
				goto error_and_free;

		node = node->next;
	}

	if (last_child)
		*last_child = capability;

	if (child_count)
		*child_count = count;

	return TRUE;

error_and_free:
	osync_capability_free(capability);
error:

	if (last_child)
		*last_child = NULL;

	if (first_child)
		*first_child = NULL;

	osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
	return FALSE;

}

void osync_capability_free(OSyncCapability *capability)
{
	OSyncCapability *tmp;
	osync_assert(capability);

	while (capability->child) {
		tmp = osync_capability_get_next(capability->child);
		osync_capability_free(capability->child);
		capability->child = tmp;
	}
	
	osync_free(capability);
}

int osync_capability_compare_stdlib(const void *capability1, const void *capability2)
{
	return strcmp(osync_capability_get_name(*(OSyncCapability **)capability1), osync_capability_get_name(*(OSyncCapability **)capability2));
}

OSyncCapability *osync_capability_new(OSyncCapabilities *capabilities, const char *objtype, const char *name, OSyncError **error)
{
	OSyncCapabilitiesObjType *capabilitiesobjtype = NULL;
	xmlNodePtr node = NULL;
	OSyncCapability *capability = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %s, %s, %p)", __func__, capabilities, objtype, name, error);
	osync_assert(capabilities);
	osync_assert(objtype);
	osync_assert(name);
	
	capabilitiesobjtype = osync_capabilitiesobjtype_get(capabilities, objtype);
	if(!capabilitiesobjtype) {
		xmlNodePtr node = xmlNewTextChild(xmlDocGetRootElement(capabilities->doc), NULL, BAD_CAST objtype, NULL);
		capabilitiesobjtype = osync_capabilitiesobjtype_new(capabilities, node, error);
		if(!capabilitiesobjtype) {
			xmlUnlinkNode(node);
			xmlFreeNode(node);
			osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
			return NULL;
		}
	}
	
	node = xmlNewTextChild(capabilitiesobjtype->node, NULL, (xmlChar *)name, NULL);
	capability = osync_capability_new_node(capabilitiesobjtype, node, error);
	if(!capability) {
		xmlUnlinkNode(node);
		xmlFreeNode(node);
		osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
		return NULL;
	}
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, capability);
	return capability;
}

const char *osync_capability_get_name(OSyncCapability *capability)
{
	osync_assert(capability);
	
	return (const char *) capability->node->name;
}

OSyncCapability *osync_capability_get_next(OSyncCapability *capability)
{
	osync_return_val_if_fail(capability, NULL);
	
	return capability->next;
}

OSyncCapability *osync_capability_get_prev(OSyncCapability *capability)
{
	osync_return_val_if_fail(capability, NULL);
	
	return capability->prev;
}

OSyncCapability *osync_capability_get_child(OSyncCapability *capability)
{
	osync_return_val_if_fail(capability, NULL);
	
	return capability->child;
}

OSyncCapability *osync_capability_get_parent(OSyncCapability *capability)
{
	osync_return_val_if_fail(capability, NULL);
	
	return capability->parent;
}

osync_bool osync_capability_has_key(OSyncCapability *capability)
{
	osync_assert(capability);

	if(capability->node->children)
		return TRUE;
	else
		return FALSE;	
}

int osync_capability_get_key_count(OSyncCapability *capability)
{
	int count;
	xmlNodePtr child = NULL;

	osync_assert(capability);
	
	child = capability->node->xmlChildrenNode;
	
	for(count=0 ; child != NULL; child = child->next)
		count++;
	
	return count;
}

const char *osync_capability_get_nth_key(OSyncCapability *capability, int nth)
{
	int count = 0;
	xmlNodePtr child = NULL;

	osync_assert(capability);
	
	child = capability->node->xmlChildrenNode;
	
	for(count=0; child != NULL; child = child->next) {
		if(count == nth)
			return (const char *)child->name;
		count++;
	}
	
	return NULL;
}

void osync_capability_add_key(OSyncCapability *capability, const char *name)
{
	osync_assert(capability);
	osync_assert(name);
	
	xmlNewTextChild(capability->node, NULL, (xmlChar*)name, NULL);
}
