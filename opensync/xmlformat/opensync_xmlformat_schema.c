/*
 * libopensync - A synchronization framework
 * Copyright (C) 2008  Bjoern Ricks <bjoern.ricks@gmail.com>
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
 * Author: Bjoern Ricks <bjoern.ricks@gmail.com>
 * 
 */

#include "opensync.h"
#include "opensync_internals.h"

#include "opensync-xmlformat.h"
#include "opensync-xmlformat_internals.h"

#include "opensync_xmlformat_private.h"		/* FIXME: direct access of private header */

#include "opensync_xmlformat_schema_private.h"

OSyncXMLFormatSchema *osync_xmlformat_schema_new_path(const char *objtype, const char *path, OSyncError **error) {
	OSyncXMLFormatSchema * osyncschema = NULL;
	char *schemafilepath = NULL;
	xmlSchemaParserCtxtPtr xmlSchemaParserCtxt;

	osync_trace(TRACE_ENTRY, "%s(%s, %p, %p)", __func__, __NULLSTR(objtype), path, error);

	if (!objtype) {
		osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "Can't load XML Schema without defined object type.");
		goto error;
	}
	
	osyncschema = osync_try_malloc0(sizeof(OSyncXMLFormatSchema), error);
	if(!osyncschema) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
		return NULL;
	}
	osyncschema->objtype = g_strdup(objtype);

	schemafilepath = g_strdup_printf("%s%c%s%s%s",
	                                 path ? path : OPENSYNC_SCHEMASDIR,
	                                 G_DIR_SEPARATOR,
	                                 "xmlformat-",
	                                 osyncschema->objtype,
	                                 ".xsd");

	osyncschema->ref_count = 1;

	xmlSchemaParserCtxt = xmlSchemaNewParserCtxt(schemafilepath);
	g_free(schemafilepath);
	if ( xmlSchemaParserCtxt == NULL ) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Creation of new XMLFormatSchema failed. Could not create schema parser context.");
		goto error;
	}
	osyncschema->schema = xmlSchemaParse(xmlSchemaParserCtxt);
	xmlSchemaFreeParserCtxt(xmlSchemaParserCtxt);
	if ( osyncschema->schema == NULL ) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Creation of new XMLFormatSchema failed. Could not read schema file.");
		goto error;
	}

	osyncschema->context = xmlSchemaNewValidCtxt(osyncschema->schema);
	if (osyncschema->context == NULL) {
		xmlSchemaFree(osyncschema->schema);
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Creation of new XMLFormatSchema failed. Could not create schema validation context.");
		goto error;
	}
	osync_trace(TRACE_EXIT, "%s", __func__ );
	return osyncschema;
 error:
	g_free(osyncschema->objtype);
	g_free(osyncschema);
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

OSyncXMLFormatSchema *osync_xmlformat_schema_new_xmlformat(OSyncXMLFormat *xmlformat, const char *path, OSyncError **error) {
	return osync_xmlformat_schema_new_path(osync_xmlformat_get_objtype(xmlformat), path, error);
}


OSyncXMLFormatSchema *osync_xmlformat_schema_new(const char *objtype, OSyncError **error) {
	return osync_xmlformat_schema_new_path(objtype, NULL, error);
}

osync_bool osync_xmlformat_schema_validate(OSyncXMLFormatSchema *schema, OSyncXMLFormat *xmlformat, OSyncError **error)
{
	int rc = 0;
	osync_assert(xmlformat);
	osync_assert(schema);
	
	/* Validate the document */
	rc = xmlSchemaValidateDoc(schema->context, xmlformat->doc);

	if(rc != 0) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "XMLFormat validation failed.");
		return FALSE;
	}
	return TRUE;
}

void osync_xmlformat_schema_unref(OSyncXMLFormatSchema *osyncschema) {

	osync_assert(osyncschema);

	if (g_atomic_int_dec_and_test(&(osyncschema->ref_count))) {
		xmlSchemaFreeValidCtxt(osyncschema->context);
		xmlSchemaFree(osyncschema->schema);
		g_free(osyncschema->objtype);
		g_free(osyncschema);
	}
	
}

OSyncXMLFormatSchema *osync_xmlformat_schema_ref(OSyncXMLFormatSchema *osyncschema)
{
	osync_assert(osyncschema);
	
	g_atomic_int_inc(&(osyncschema->ref_count));

	return osyncschema;
}
