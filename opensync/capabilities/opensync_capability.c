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

int osync_capability_compare_stdlib(const void *capability1, const void *capability2)
{
	return strcmp(osync_capability_get_name(*(OSyncCapability **)capability1), osync_capability_get_name(*(OSyncCapability **)capability2));
}

OSyncCapability *osync_capability_parse(OSyncCapabilitiesObjType *capabilitiesobjtype, xmlNodePtr node, OSyncError **error)
{
	xmlNode *cur;
	OSyncCapability *cap = NULL;
	osync_assert(capabilitiesobjtype);
	osync_assert(node);

	cur = node;

	cap = osync_capability_new(capabilitiesobjtype, error);
	if (!cap)
		goto error;

	cur = cur->children;
        for (; cur != NULL; cur = cur->next) {
                char *str = NULL;

                if (cur->type != XML_ELEMENT_NODE)
                        continue;

                str = (char*)xmlNodeGetContent(cur);
                if (!str)
                        continue;

                if (!xmlStrcmp(cur->name, BAD_CAST "Name"))
			osync_capability_set_name(cap, str);
		else if (!xmlStrcmp(cur->name, BAD_CAST "DisplayName"))
			osync_capability_set_displayname(cap, str);
		else if (!xmlStrcmp(cur->name, BAD_CAST "MaxOccurs"))
			osync_capability_set_maxoccurs(cap, atoi(str));
		else if (!xmlStrcmp(cur->name, BAD_CAST "Max"))
			osync_capability_set_max(cap, atoi(str));
		else if (!xmlStrcmp(cur->name, BAD_CAST "Min"))
			osync_capability_set_min(cap, atoi(str));
		else if (!xmlStrcmp(cur->name, BAD_CAST "Parameter"))
				; //TODO
		else if (!xmlStrcmp(cur->name, BAD_CAST "Type"))
				; //TODO
		else if (!xmlStrcmp(cur->name, BAD_CAST "ValEnum"))
			osync_list_prepend(cap->valenum, osync_strdup(str));

		osync_xml_free(str);
	}
	
	return cap;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
	return NULL;
}

osync_bool osync_capability_assemble(OSyncCapability *cap, xmlNodePtr node, OSyncError **error)
{
	char *tmp;
	xmlNode *cur;
	OSyncList *l;
	osync_assert(cap);
	osync_assert(node);

	cur = xmlNewChild(node, NULL, (xmlChar*)"Cap", NULL);
	if (!cur)
		goto error_oom;

	/* DisplayName */
	if (!xmlNewChild(cur, NULL, (xmlChar*)"DisplayName",
				(xmlChar*)osync_capability_get_displayname(cap)))
		goto error_oom;

	/* MaxOccurs */
	tmp = osync_strdup_printf("%u", osync_capability_get_maxoccurs(cap));
	if (!tmp)
		goto error_oom;

	if (!xmlNewChild(cur, NULL, (xmlChar*)"MaxOccurs", (xmlChar*)tmp))
		goto error_oom;

	/* Max */
	tmp = osync_strdup_printf("%u", osync_capability_get_max(cap));
	if (!tmp)
		goto error_oom;

	if (!xmlNewChild(cur, NULL, (xmlChar*)"Max", (xmlChar*)tmp))
		goto error_oom;

	/* Min */
	tmp = osync_strdup_printf("%u", osync_capability_get_min(cap));
	if (!tmp)
		goto error_oom;

	if (!xmlNewChild(cur, NULL, (xmlChar*)"Min", (xmlChar*)tmp))
		goto error_oom;

	/* Name */
	if (!xmlNewChild(cur, NULL, (xmlChar*)"Name",
				(xmlChar*)osync_capability_get_name(cap)))
		goto error_oom;

	/* Parameter TODO */
	/* Type TODO */
	
	/* ValEnum */
	for (l = cap->valenum; l; l = l->next) {
		if (!xmlNewChild(cur, NULL, (xmlChar*)"ValEnum",
					(xmlChar*)l->data))
			goto error_oom;
	}

	return TRUE;

error_oom:	
	osync_error_set(error, OSYNC_ERROR_GENERIC, "No memory left to assemble capability.");
/*	
error:
*/
	osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
	return FALSE;
}

OSyncCapability *osync_capability_new(OSyncCapabilitiesObjType *capobjtype, OSyncError **error)
{
	OSyncCapability *capability = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %s, %p)", __func__, capobjtype, error);
	osync_assert(capobjtype);
	
	capability = osync_try_malloc0(sizeof(OSyncCapability), error);
	if(!capability)
		goto error;

	osync_capabilities_objtype_add_capability(capobjtype, capability);
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, capability);
	return capability;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
	return NULL;

}

OSyncCapability *osync_capability_ref(OSyncCapability *capability)
{
	osync_assert(capability);
	
	g_atomic_int_inc(&(capability->ref_count));

	return capability;
}

void osync_capability_unref(OSyncCapability *capability)
{
	osync_assert(capability);
			
	if (g_atomic_int_dec_and_test(&(capability->ref_count))) {
		/* TODO free struct members */
		osync_free(capability);
	}
}

const char *osync_capability_get_name(OSyncCapability *capability)
{
	osync_assert(capability);
	
	return (const char *) capability->name;
}

void osync_capability_set_name(OSyncCapability *capability, const char *name)
{
	osync_assert(capability);
	osync_assert(name);

	if (capability->name)
		osync_free(capability->name);

	capability->name = osync_strdup(name);
}


const char *osync_capability_get_displayname(OSyncCapability *capability)
{
	osync_assert(capability);

	return (const char *) capability->displayname;
}

void osync_capability_set_displayname(OSyncCapability *capability, const char *displayname)
{
	osync_assert(capability);
	osync_assert(displayname);

	if (capability->displayname)
		osync_free(capability->displayname);

	capability->displayname = osync_strdup(displayname);
}

unsigned int osync_capability_get_maxoccurs(OSyncCapability *capability)
{
	osync_assert(capability);

	return capability->maxoccurs;
}

void osync_capability_set_maxoccurs(OSyncCapability *capability, unsigned int maxoccurs)
{
	osync_assert(capability);

	capability->maxoccurs = maxoccurs;
}

unsigned int osync_capability_get_max(OSyncCapability *capability)
{
	osync_assert(capability);

	return capability->max;
}

void osync_capability_set_max(OSyncCapability *capability, unsigned int max)
{
	osync_assert(capability);

	capability->max = max;
}

unsigned int osync_capability_get_min(OSyncCapability *capability)
{
	osync_assert(capability);

	return capability->min;
}

void osync_capability_set_min(OSyncCapability *capability, unsigned int min)
{
	osync_assert(capability);

	capability->min = min;
}

OSyncCapabilityParameter *osync_capability_get_parameter(OSyncCapability *capability)
{
	osync_assert(capability);

	return capability->parameter;
}

void osync_capability_set_parameter(OSyncCapability *capability, OSyncCapabilityParameter *parameter)
{
	osync_assert(capability);
	osync_assert(parameter);

	if (capability->parameter)
		osync_capability_parameter_unref(capability->parameter);

	capability->parameter = osync_capability_parameter_ref(parameter);
}

OSyncCapabilityType osync_capability_get_type(OSyncCapability *capability)
{
	osync_assert(capability);

	return capability->type;
}

void osync_capability_set_type(OSyncCapability *capability, OSyncCapabilityType type)
{
	osync_assert(capability);

	capability->type = type;
}

OSyncList *osync_capability_get_valenums(OSyncCapability *capability)
{
	osync_assert(capability);

	return capability->valenum;
}

OSyncCapabilityParameter *osync_capability_parameter_new(OSyncError **error)
{
	OSyncCapabilityParameter *capparam;

	capparam = osync_try_malloc0(sizeof(OSyncCapabilityParameter), error);
	if (!capparam)
		goto error;

	capparam->ref_count = 1;

	return capparam;

error:
	return NULL;
}

OSyncCapabilityParameter *osync_capability_parameter_ref(OSyncCapabilityParameter *capparam)
{
	osync_assert(capparam);
	g_atomic_int_inc(&(capparam->ref_count));
	return capparam;
}

void osync_capability_parameter_unref(OSyncCapabilityParameter *capparam)
{
	osync_assert(capparam);
		
	if (g_atomic_int_dec_and_test(&(capparam->ref_count))) {
		osync_trace(TRACE_ENTRY, "%s(%p)", __func__, capparam);
		osync_trace(TRACE_EXIT, "%s", __func__);
	}

}

