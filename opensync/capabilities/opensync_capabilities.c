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

OSyncCapabilities *osync_capabilities_new(const char *capsformat, OSyncError **error)
{
	OSyncCapabilities *capabilities = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, error);
	
	capabilities = osync_try_malloc0(sizeof(OSyncCapabilities), error);
	if(!capabilities) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
		return NULL;
	}
	
	capabilities->ref_count = 1;

	osync_capabilities_set_format(capabilities, capsformat);
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, capabilities);
	return capabilities;
}

OSyncCapabilities *osync_capabilities_ref(OSyncCapabilities *capabilities)
{
	osync_assert(capabilities);
	
	g_atomic_int_inc(&(capabilities->ref_count));

	return capabilities;
}

void osync_capabilities_unref(OSyncCapabilities *caps)
{
	osync_assert(caps);
			
	if (g_atomic_int_dec_and_test(&(caps->ref_count))) {
		while (caps->objtypes) {
			osync_capabilities_objtype_unref(caps->objtypes->data);
			caps->objtypes = osync_list_remove(caps->objtypes, caps->objtypes->data);
		}

		osync_free(caps->format);
		osync_xml_free_doc(caps->doc);

		osync_free(caps);
	}
}

OSyncCapabilities *osync_capabilities_parse(const char *buffer, unsigned int size, OSyncError **error)
{
	OSyncCapabilities *capabilities = NULL;
	xmlNodePtr cur = NULL;
	xmlChar *capsformat;
	osync_trace(TRACE_ENTRY, "%s(%p, %u, %p)", __func__, buffer, size, error);
	osync_assert(buffer);
	
	capabilities = osync_try_malloc0(sizeof(OSyncCapabilities), error);
	if(!capabilities) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
		return NULL;
	}

	capabilities->ref_count = 1;
	capabilities->doc = xmlReadMemory(buffer, size, NULL, NULL, XML_PARSE_NOBLANKS);
	if(capabilities->doc == NULL) {
		osync_free(capabilities);
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Could not parse XML.");
		goto error;
	}
	capabilities->doc->_private = capabilities;


	/* Get CapsFormat Name */
        capsformat = xmlGetProp(capabilities->doc->children, (const xmlChar*)"CapsFormat");
	osync_capabilities_set_format(capabilities, (const char *) capsformat);
	xmlFree(capsformat);
	
	cur = xmlDocGetRootElement(capabilities->doc);

	cur = cur->children;
	for(; cur != NULL; cur = cur->next) {

		if (cur->type != XML_ELEMENT_NODE)
			continue;

		OSyncCapabilitiesObjType *capabilitiesobjtype = osync_capabilities_objtype_parse_and_add(capabilities, cur, error);
		if (!capabilitiesobjtype) {
			osync_capabilities_unref(capabilities);
			goto error;
		}
		else {
			// we don't need our copy of the capabilitiesobjtype
			osync_capabilities_objtype_unref(capabilitiesobjtype);
		}
	}

	osync_trace(TRACE_EXIT, "%s: %p", __func__, capabilities);
	return capabilities;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
	return NULL;
}

osync_bool osync_capabilities_assemble(OSyncCapabilities *capabilities, char **buffer, unsigned int *size, OSyncError **error)
{
	xmlDocPtr doc = NULL;
	xmlNodePtr root;
	char *version_str;
	const char *capsformat;
	OSyncList *l;
	osync_assert(capabilities);

	capsformat = osync_capabilities_get_format(capabilities);

	if (!capsformat) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Can't assamble capabilities: Capabilities Format not set.");
		goto error;
	}

	if (capabilities->doc)
		osync_xml_free_doc(capabilities->doc);

	capabilities->doc = doc = xmlNewDoc(BAD_CAST "1.0");
	capabilities->doc->children = xmlNewDocNode(capabilities->doc, NULL, (xmlChar *)"Caps", NULL);
	capabilities->doc->_private = capabilities;

        /* Set version for capabilities configuration */
        version_str = osync_strdup_printf("%u.%u", OSYNC_CAPS_MAJOR_VERSION, OSYNC_CAPS_MINOR_VERSION);
        xmlSetProp(doc->children, (const xmlChar*)"Version", (const xmlChar *)version_str);
        osync_free(version_str);

	/* Set CapsFormat Name */
        xmlSetProp(doc->children, (const xmlChar*)"CapsFormat", (const xmlChar *)capsformat);

	root = doc->children;

	for (l = capabilities->objtypes; l; l = l->next) {
		OSyncCapabilitiesObjType *capobjtype;
		capobjtype = (OSyncCapabilitiesObjType *) l->data;
		if (!osync_capabilities_objtype_assemble(capobjtype, root, error))
			goto error;
	}

	/* XXX Ugly cast, but we try to fit here the opensync API pattern of unsigned size/length types */
	xmlDocDumpFormatMemoryEnc(doc, (xmlChar **) buffer, (int *) size, NULL, 1);
	
	return TRUE;

/*
error_oom:
	osync_error_set(error, OSYNC_ERROR_GENERIC, "Couldn't allocate memory to assemble capabilities file.");
*/	
error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_capabilities_save(OSyncCapabilities *capabilities, const char *file, OSyncError **error)
{
	unsigned int size;
	char *buffer;
	osync_bool ret;

	osync_trace(TRACE_ENTRY, "%s(%p, %s, %p)", __func__, capabilities, __NULLSTR(file), error);

	osync_assert(capabilities);
	osync_assert(file);

	osync_capabilities_sort(capabilities);

	ret = osync_capabilities_assemble(capabilities, &buffer, &size, error);
	if (!ret)
		goto error;

	ret = osync_file_write(file, buffer, size, 0600, error);
	osync_free(buffer);

	if (!ret)
		goto error;
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

error:	
	osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
	return FALSE;
}

OSyncCapabilities *osync_capabilities_load(const char *file, OSyncError **error)
{
	unsigned int size;
	char *buffer;
	OSyncCapabilities *capabilities;
	osync_bool b;

	osync_trace(TRACE_ENTRY, "%s(%s, %p)", __func__, file, error);
	osync_assert(file);
	
	b = osync_file_read(file, &buffer, &size, error);
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

OSyncCapabilities *osync_capabilities_load_identifier(const char *file, OSyncError **error)
{
	char *filename;
	OSyncCapabilities *capabilities;

	osync_trace(TRACE_ENTRY, "%s(%s, %p)", __func__, file, error);
	osync_assert(file);
	
	filename = osync_strdup_printf("%s%c%s", OPENSYNC_CAPABILITIESDIR, G_DIR_SEPARATOR, file);
	
	capabilities = osync_capabilities_load(filename, error);
	if (!capabilities)
		goto error;
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, capabilities);
	return capabilities;

error:	
	osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
	return NULL;
}

const char *osync_capabilities_get_format(OSyncCapabilities *capabilities)
{
	osync_assert(capabilities);
	return capabilities->format;
}

void osync_capabilities_set_format(OSyncCapabilities *capabilities, const char *capsformat)
{
	osync_assert(capabilities);
	osync_assert(capsformat);

	if (capabilities->format)
		osync_free(capabilities->format);

	capabilities->format = osync_strdup(capsformat);
}

OSyncCapabilitiesObjType *osync_capabilities_get_objtype(OSyncCapabilities *capabilities, const char *objtype)
{
	OSyncList *l = NULL;
	OSyncCapabilitiesObjType *capobjtype = NULL;
	osync_assert(capabilities);
	osync_assert(objtype);
	
	l = capabilities->objtypes;
	for(; l != NULL; l = l->next) {
		capobjtype = (OSyncCapabilitiesObjType *) l->data;
		if(!strcmp(osync_capabilities_objtype_get_name(capobjtype), objtype))
			break;
	}	

	return capobjtype;
}


void osync_capabilities_add_objtype(OSyncCapabilities *capabilities, OSyncCapabilitiesObjType *capabilitiesobjtype)
{
	osync_assert(capabilities);
	osync_assert(capabilitiesobjtype);

	
	capabilities->objtypes = osync_list_append(capabilities->objtypes, capabilitiesobjtype);
	osync_capabilities_objtype_ref(capabilitiesobjtype);
}

void osync_capabilities_sort(OSyncCapabilities *capabilities)
{
	OSyncList *l;
	osync_assert(capabilities);

	for (l = capabilities->objtypes; l; l = l->next) {
		OSyncCapabilitiesObjType *capsobjtype = (OSyncCapabilitiesObjType *) l->data;

		osync_capabilities_objtype_sort(capsobjtype);
	}

}

