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

#include "opensync-merger.h"
#include "opensync-merger_internals.h"

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
	
	if(!capabilitiesobjtype->first_child)
		capabilitiesobjtype->first_child = capability;
	if(capabilitiesobjtype->last_child)
		capabilitiesobjtype->last_child->next = capability;
	capabilitiesobjtype->last_child = capability;
	capabilitiesobjtype->child_count++;
	
	return capability;
}

void osync_capability_free(OSyncCapability *capability)
{
	osync_assert(capability);
	
	g_free(capability);
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
	osync_assert(capability);
	
	return capability->next;
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
