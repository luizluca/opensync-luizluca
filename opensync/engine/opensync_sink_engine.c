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

#include "opensync-archive.h"
#include "opensync-group.h"
#include "opensync-engine.h"
#include "opensync-data.h"
#include "opensync-format.h"
#include "opensync-mapping.h"

#include "opensync_obj_engine_internals.h"
#include "opensync_sink_engine_internals.h"
#include "opensync_mapping_entry_engine_internals.h"

#include "client/opensync_client_proxy_internals.h"
#include "format/opensync_objformat_internals.h" /* osync_objformat_has_merger() */

OSyncSinkEngine *osync_sink_engine_new(int position, OSyncClientProxy *proxy, OSyncObjEngine *objengine, OSyncError **error)
{
	OSyncSinkEngine *sinkengine = NULL;
	osync_trace(TRACE_ENTRY, "%s(%i, %p, %p, %p)", __func__, position, proxy, objengine, error);
	osync_assert(proxy);
	osync_assert(objengine);
	
	sinkengine = osync_try_malloc0(sizeof(OSyncSinkEngine), error);
	if (!sinkengine)
		goto error;
	sinkengine->ref_count = 1;
	sinkengine->position = position;
	
	/* we dont reference the proxy to avoid circular dependencies. This object is completely
	 * dependent on the proxy anyways */
	sinkengine->proxy = proxy;
	
	sinkengine->engine = objengine;
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, sinkengine);
	return sinkengine;
	
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

OSyncSinkEngine *osync_sink_engine_ref(OSyncSinkEngine *engine)
{
	osync_assert(engine);
	
	g_atomic_int_inc(&(engine->ref_count));

	return engine;
}

void osync_sink_engine_unref(OSyncSinkEngine *engine)
{
	osync_assert(engine);
		
	if (g_atomic_int_dec_and_test(&(engine->ref_count))) {
		while (engine->unmapped) {
			OSyncChange *change = engine->unmapped->data;
			osync_change_unref(change);
			
			engine->unmapped = osync_list_remove(engine->unmapped, engine->unmapped->data);
		}
		
		while (engine->entries) {
			OSyncMappingEntryEngine *entry = engine->entries->data;
			osync_entry_engine_unref(entry);
			
			engine->entries = osync_list_remove(engine->entries, engine->entries->data);
		}
		
		g_free(engine);
	}
}

osync_bool osync_sink_engine_is_connected(OSyncSinkEngine *engine)
{
	OSyncObjEngine *objengine = NULL;
	osync_assert(engine);

	objengine = engine->engine;

	if (!objengine)
		return FALSE;

	return !!(objengine->sink_connects & (1 << engine->position));
}

void osync_sink_engine_set_dummy(OSyncSinkEngine *engine, osync_bool isdummy)
{
	osync_assert(engine);
	engine->dummy_sink = isdummy;
}

osync_bool osync_sink_engine_is_dummy(OSyncSinkEngine *engine)
{
	osync_assert(engine);
	return engine->dummy_sink;
}

const OSyncList *osync_sink_engine_get_mapping_entry_engines(OSyncSinkEngine *engine)
{
	osync_return_val_if_fail(engine, NULL);
	return engine->entries;
}

OSyncMember *osync_sink_engine_get_member(OSyncSinkEngine *engine)
{
	osync_return_val_if_fail(engine, NULL);
	osync_return_val_if_fail(engine->proxy, NULL);
	return osync_client_proxy_get_member(engine->proxy);
}

osync_bool osync_sink_engine_demerge(OSyncSinkEngine *engine, OSyncArchive *archive, OSyncError **error)
{
	OSyncList *o;
	OSyncMember *member;
	OSyncCapabilities *caps;

	osync_assert(engine);
	osync_assert(archive);

	member = osync_client_proxy_get_member(engine->proxy);
	osync_assert(member);
	caps = osync_member_get_capabilities(member); 

	if (!caps)
		return TRUE;

	for (o = engine->entries; o; o = o->next) {
		OSyncMappingEntryEngine *entry_engine = o->data;
		osync_assert(entry_engine);

		if (entry_engine->change == NULL)
			continue;

		if (osync_change_get_changetype(entry_engine->change) == OSYNC_CHANGE_TYPE_DELETED)
			continue;

		if (!osync_objformat_has_merger(osync_change_get_objformat(entry_engine->change)))
			continue;

		if (!osync_entry_engine_demerge(entry_engine, archive, caps, error))
			goto error;

	}

	return TRUE;
error:
	return FALSE;
}

osync_bool osync_sink_engine_convert_to_dest(OSyncSinkEngine *engine, OSyncFormatEnv *formatenv, OSyncError **error)
{
	OSyncList *o;
	OSyncMember *member;
	OSyncObjTypeSink *objtype_sink;
	const char *objtype;
	OSyncFormatConverterPath *path = NULL;

	osync_assert(engine);
	osync_assert(formatenv);

	member = osync_client_proxy_get_member(engine->proxy);
	osync_assert(member);

	objtype = osync_obj_engine_get_objtype(engine->engine);
	objtype_sink = osync_member_find_objtype_sink(member, objtype);
	osync_assert(objtype_sink);

	for (o = engine->entries; o; o = o->next) {
		OSyncMappingEntryEngine *entry_engine = o->data;
		osync_assert(entry_engine);

		if (entry_engine->change == NULL)
			continue;

		if (osync_change_get_changetype(entry_engine->change) == OSYNC_CHANGE_TYPE_DELETED)
			continue;

		if (!osync_entry_engine_convert(entry_engine, formatenv, objtype_sink, &path, error))
			goto error;
	}

	if (path)
		osync_converter_path_unref(path);


	return TRUE;

error:
	if (path)
		osync_converter_path_unref(path);

	return FALSE;
}

