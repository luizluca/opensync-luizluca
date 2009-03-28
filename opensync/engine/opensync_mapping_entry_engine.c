/*
 * libosengine - A synchronization engine for the opensync framework
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
 
#include "opensync.h"
#include "opensync_internals.h"

#include "opensync-group.h"
#include "opensync-engine.h"
#include "opensync-data.h"
#include "opensync-mapping.h"
#include "opensync-format.h"
#include "opensync-plugin.h"

#include "archive/opensync_archive_internals.h"
#include "format/opensync_objformat_internals.h"

#include "opensync_obj_engine.h"
#include "opensync_obj_engine.h"

#include "opensync_mapping_engine.h"
#include "opensync_sink_engine_internals.h"

#include "opensync_mapping_entry_engine_internals.h"
#include "opensync_mapping_engine_internals.h"

OSyncMappingEntryEngine *osync_entry_engine_new(OSyncMappingEntry *entry, OSyncMappingEngine *mapping_engine, OSyncSinkEngine *sink_engine, OSyncObjEngine *objengine, OSyncError **error)
{
	OSyncMappingEntryEngine *engine = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %p, %p)", __func__, entry, mapping_engine, sink_engine, objengine, error);
	osync_assert(sink_engine);
	osync_assert(entry);
	
	engine = osync_try_malloc0(sizeof(OSyncMappingEntryEngine), error);
	if (!engine)
		goto error;
	engine->ref_count = 1;
	
	engine->sink_engine = sink_engine;
	
	engine->objengine = objengine;
	
	engine->mapping_engine = osync_mapping_engine_ref(mapping_engine);
	engine->entry = osync_mapping_entry_ref(entry);
	
	sink_engine->entries = osync_list_append(sink_engine->entries, engine);
	osync_entry_engine_ref(engine);
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, engine);
	return engine;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

OSyncMappingEntryEngine *osync_entry_engine_ref(OSyncMappingEntryEngine *engine)
{
	osync_assert(engine);
	
	g_atomic_int_inc(&(engine->ref_count));

	return engine;
}

void osync_entry_engine_unref(OSyncMappingEntryEngine *engine)
{
	osync_assert(engine);
		
	if (g_atomic_int_dec_and_test(&(engine->ref_count))) {
	
		if (engine->change)
			osync_change_unref(engine->change);

		if (engine->mapping_engine)
			osync_mapping_engine_unref(engine->mapping_engine);

		if (engine->entry)
			osync_mapping_entry_unref(engine->entry);
		
		osync_free(engine);
	}
}

osync_bool osync_entry_engine_matches(OSyncMappingEntryEngine *engine, OSyncChange *change)
{
	OSyncMappingEntry *entry = NULL;
	const char *mapping_entry_uid = NULL; 
	const char *change_uid = NULL; 
	osync_assert(engine);
	osync_assert(engine->entry);
	osync_assert(change);
	
	entry = engine->entry;
	mapping_entry_uid = osync_mapping_entry_get_uid(entry); 
	change_uid = osync_change_get_uid(change);
	osync_assert(change_uid);
	
	/* If no UID get set for the MappingEntry - MISMATCH */
	if (mapping_entry_uid && !strcmp(mapping_entry_uid, change_uid))
		return TRUE;
	
	return FALSE;
}

void osync_entry_engine_update(OSyncMappingEntryEngine *engine, OSyncChange *change)
{
	osync_assert(engine);
	
	if (engine->change)
		osync_change_unref(engine->change);
	
	engine->change = change;
	engine->mapping_engine->synced = FALSE;
	
	if (change)
		osync_change_ref(change);
}

OSyncChange *osync_entry_engine_get_change(OSyncMappingEntryEngine *engine)
{
	osync_assert(engine);
	return engine->change;
}

OSyncChangeType osync_entry_engine_get_changetype(OSyncMappingEntryEngine *engine)
{
	osync_assert(engine);
	return osync_change_get_changetype(osync_entry_engine_get_change(engine));
}

osync_bool osync_entry_engine_is_dirty(OSyncMappingEntryEngine *engine)
{
	osync_assert(engine);
	return engine->dirty;
}

void osync_entry_engine_set_dirty(OSyncMappingEntryEngine *engine, osync_bool dirty)
{
	osync_assert(engine);
	engine->dirty = dirty;
}

osync_bool osync_entry_engine_demerge(OSyncMappingEntryEngine *entry_engine, OSyncArchive *archive, OSyncCapabilities *caps, OSyncError **error)
{

	char *buffer = NULL, *marshalbuf;
	unsigned int size = 0, marshalsize;
	const char *objtype = NULL;
	OSyncMapping *mapping = NULL;
	OSyncMarshal *marshal = NULL;
	OSyncObjFormat *objformat = osync_change_get_objformat(entry_engine->change);

	osync_trace(TRACE_INTERNAL, "Entry %s Dirty: %i", osync_change_get_uid(entry_engine->change), osync_entry_engine_is_dirty(entry_engine));

	osync_trace(TRACE_INTERNAL, "Save the entire data and demerge.");
	objtype = osync_change_get_objtype(entry_engine->change);
	mapping = entry_engine->mapping_engine->mapping;
	
	osync_data_get_data(osync_change_get_data(entry_engine->change),  &buffer, &size);

	marshal = osync_marshal_new(error);
	if (!marshal)
		goto error;

	if (!osync_objformat_marshal(objformat, buffer, size, marshal, error))
		goto error_free_marshal;

	osync_marshal_get_buffer(marshal, &marshalbuf, &marshalsize);

	if (!osync_archive_save_data(archive, osync_mapping_get_id(mapping), objtype, marshalbuf, marshalsize, error)) {
		osync_free(buffer); /* TODO: Is this a valid free? */
		goto error_free_marshal;
	}

	if (!osync_objformat_demerge(objformat, &buffer, &size, caps, error))
		goto error_free_marshal;

	osync_trace(TRACE_SENSITIVE, "Post Demerge:\n%s\n",
			osync_objformat_print(objformat, buffer, size));

	osync_marshal_unref(marshal);

	return TRUE;

error_free_marshal:
	osync_marshal_unref(marshal);
error:
	return FALSE;
}

osync_bool osync_entry_engine_convert(OSyncMappingEntryEngine *entry_engine, OSyncFormatEnv *formatenv, OSyncObjTypeSink *objtype_sink, OSyncFormatConverterPath **cachedpath, OSyncError **error)
{
	char *objtype = NULL;
	OSyncList *format_sinks = NULL;
	unsigned int length = 0;
	OSyncFormatConverter *converter = NULL;
	OSyncChange *change = entry_engine->change;
	OSyncFormatConverterPath *path;

	osync_trace(TRACE_INTERNAL, "Starting to convert from objtype %s and format %s", osync_change_get_objtype(entry_engine->change), osync_objformat_get_name(osync_change_get_objformat(entry_engine->change)));
	/* We have to save the objtype of the change so that it does not get
	 * overwritten by the conversion */
	objtype = osync_strdup(osync_change_get_objtype(change));
		
	/* Now we have to convert to one of the formats
	 * that the client can understand */
	format_sinks = osync_objtype_sink_get_objformat_sinks(objtype_sink);
	if (!format_sinks) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "There are no available format sinks.");
		goto error_free_objtype;
	}
	
	/* We cache the converter path for each sink/member couple */
	if (!*cachedpath)
		path = osync_format_env_find_path_formats_with_detectors(formatenv, osync_change_get_data(entry_engine->change), format_sinks, osync_objtype_sink_get_preferred_format(objtype_sink), error);
	else
		path = osync_converter_path_ref(*cachedpath);

	if (!path)
		goto error_free_objtype;

	length = osync_converter_path_num_edges(path);
	converter = osync_converter_path_nth_edge(path, length - 1);
	if (converter) {
		OSyncObjFormat *format = osync_converter_get_targetformat(converter);
		OSyncObjFormatSink *formatsink = osync_objtype_sink_find_objformat_sink(objtype_sink, format);
		osync_converter_path_set_config(path, osync_objformat_sink_get_config(formatsink));
	}

	if (!osync_format_env_convert(formatenv, path, osync_change_get_data(entry_engine->change), error)) {
		goto error_free_path;
	}
	osync_trace(TRACE_INTERNAL, "converted to format %s", osync_objformat_get_name(osync_change_get_objformat(entry_engine->change)));
		
	if (*cachedpath)
		osync_converter_path_unref(*cachedpath);

	*cachedpath = path;
		
	osync_change_set_objtype(change, objtype);
	osync_free(objtype);

	return TRUE;

error_free_path:
	osync_converter_path_unref(path);
error_free_objtype:
	osync_free(objtype);
/*error:*/
	return FALSE;
}

