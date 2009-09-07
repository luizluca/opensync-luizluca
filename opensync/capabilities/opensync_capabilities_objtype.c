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

#include "opensync_capabilities_objtype_internals.h"
#include "opensync_capabilities_objtype_private.h"

OSyncCapabilitiesObjType *osync_capabilities_objtype_new(OSyncCapabilities *capabilities, const char *objtype, OSyncError **error)
{
	OSyncCapabilitiesObjType *capobjtype = NULL;
	osync_assert(capabilities);
	osync_assert(objtype);
	
	capobjtype = osync_try_malloc0(sizeof(OSyncCapabilitiesObjType), error);
	if (!capobjtype)
		goto error;

	capobjtype->name = osync_strdup(objtype);

	osync_capabilities_add_objtype(capabilities, capobjtype);

	return capobjtype;

error:	
	osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
	return NULL;
}

OSyncCapabilitiesObjType *osync_capabilities_objtype_parse(OSyncCapabilities *capabilities, xmlNode *node, OSyncError **error)
{
	xmlChar *objtype;
	xmlNode *cur;
	OSyncCapability *capability;
	OSyncCapabilitiesObjType *capobjtype = NULL;
	osync_assert(capabilities);
	osync_assert(node);

	osync_assert(!xmlStrcmp(node->name, (const xmlChar *)"ObjType"));
	objtype = xmlGetProp(node, (const xmlChar *)"Name");

	/* XXX: Bad cast from unsigned char* to const char* - is there a better way? */
	if (!(capobjtype = osync_capabilities_objtype_new(capabilities, (const char *) objtype, error)))
		goto error;

	osync_xml_free(objtype);

	cur = node->children;
	for(; cur != NULL; cur = cur->next) {

		if (cur->type != XML_ELEMENT_NODE)
			continue;

		if (!(capability = osync_capability_parse(capobjtype, cur, error)))
			goto error;
	}

	return capobjtype;

error:
	osync_xml_free(objtype);

	osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
	return NULL;
}

OSyncCapabilitiesObjType *osync_capabilities_objtype_ref(OSyncCapabilitiesObjType *capsobjtype)
{
	osync_assert(capsobjtype);
	
	g_atomic_int_inc(&(capsobjtype->ref_count));

	return capsobjtype;
}

void osync_capabilities_objtype_unref(OSyncCapabilitiesObjType *capsobjtype)
{
	osync_assert(capsobjtype);
			
	if (g_atomic_int_dec_and_test(&(capsobjtype->ref_count))) {
		OSyncList *l;
		for (l = capsobjtype->capabilities; l; l = l->next) {
			OSyncCapability *capability;
			capability = (OSyncCapability *) l->data;
			osync_capability_unref(capability);
			/* TODO unlink from list */
		}
		osync_free(capsobjtype);
	}
}

const char *osync_capabilities_objtype_get_name(OSyncCapabilitiesObjType *capsobjtype)
{
	osync_assert(capsobjtype);
	return capsobjtype->name;
}

osync_bool osync_capabilities_objtype_assemble(OSyncCapabilitiesObjType *capsobjtype, xmlNode *node, OSyncError **error)
{
	const char *name;
	OSyncList *l;
	xmlNode *cur;
	osync_assert(capsobjtype);
	osync_assert(node);

	name = osync_capabilities_objtype_get_name(capsobjtype);
	osync_assert(name);

	cur = xmlNewChild(node, NULL, (xmlChar*)"ObjType", NULL);
	if (!cur)
		goto error_oom;

	xmlSetProp(cur, (const xmlChar*)"Name", (const xmlChar*)name);

	for (l = capsobjtype->capabilities; l; l = l->next) {
		OSyncCapability *capability;
		capability = (OSyncCapability *) l->data;
		if (!osync_capability_assemble(capability, cur, error))
			goto error;
	}

	return TRUE;

error_oom:
	osync_error_set(error, OSYNC_ERROR_GENERIC, "Couldn't allocate memory to assemble capabilities objtype file.");
error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
	return FALSE;
}

OSyncList *osync_capabilities_objtype_get_caps(OSyncCapabilitiesObjType *capsobjtype)
{
        osync_assert(capsobjtype);
        return capsobjtype->capabilities;
}

void osync_capabilities_objtype_add_capability(OSyncCapabilitiesObjType *capsobjtype, OSyncCapability *capability)
{
        osync_assert(capsobjtype);
        capsobjtype->capabilities = osync_list_append(capsobjtype->capabilities, capability);
}

void osync_capabilities_objtype_sort(OSyncCapabilitiesObjType *capsobjtype)
{
	capsobjtype->capabilities = osync_list_sort(capsobjtype->capabilities, osync_capability_compare);
}

