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
#include "opensync-capabilities.h"
#include "opensync-engine.h"
#include "opensync-client.h"
#include "opensync-data.h"
#include "opensync-mapping.h"
#include "opensync-format.h"
#include "opensync-plugin.h"
#include "opensync-xmlformat.h"

#include "opensync_engine_internals.h"
#include "opensync_sink_engine_internals.h"
#include "opensync_mapping_entry_engine_internals.h"
#include "opensync_status_internals.h"

#include "opensync_mapping_engine.h"
#include "opensync_mapping_engine_internals.h"

#include "opensync_obj_engine.h"
#include "opensync_obj_engine_internals.h"

#include "archive/opensync_archive_internals.h"
#include "data/opensync_change_internals.h"
#include "client/opensync_client_proxy_internals.h"
#include "group/opensync_member_internals.h"
#include "format/opensync_objformat_internals.h"
#include "format/opensync_format_env_internals.h"
#include "format/opensync_merger_internals.h"
#include "mapping/opensync_mapping_table_internals.h"
#include "mapping/opensync_mapping_table_internals.h"

OSyncMappingEngine *_osync_obj_engine_create_mapping_engine(OSyncObjEngine *engine, OSyncError **error)
{
	/* If there is none, create one */
	OSyncMapping *mapping = osync_mapping_new(error);
	OSyncList *s = NULL;
	OSyncMappingEngine *mapping_engine = NULL;
	if (!mapping)
		goto error;
	
	osync_mapping_set_id(mapping, osync_mapping_table_get_next_id(engine->mapping_table));
	osync_mapping_table_add_mapping(engine->mapping_table, mapping);
	
	for (s = engine->sink_engines; s; s = s->next) {
		OSyncSinkEngine *sink_engine = s->data;
		
		OSyncMember *member = osync_client_proxy_get_member(sink_engine->proxy);
		
		OSyncMappingEntry *mapping_entry = osync_mapping_entry_new(error);
		osync_mapping_entry_set_member_id(mapping_entry, osync_member_get_id(member));
		osync_mapping_add_entry(mapping, mapping_entry);
		osync_mapping_entry_unref(mapping_entry);
	}
	
	mapping_engine = osync_mapping_engine_new(engine, mapping, error);
	if (!mapping_engine)
		goto error_free_mapping;
	osync_mapping_unref(mapping);
	
	return mapping_engine;
	
 error_free_mapping:
	osync_mapping_unref(mapping);
 error:
	return NULL;
}

static void _osync_obj_engine_connect_callback(OSyncClientProxy *proxy, void *userdata, osync_bool slowsync, OSyncError *error)
{
	OSyncSinkEngine *sinkengine = userdata;
	OSyncObjEngine *engine = sinkengine->engine;
	OSyncError *locerror = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %i, %p)", __func__, proxy, userdata, slowsync, error);
	
	if (error) {
		osync_trace(TRACE_INTERNAL, "Obj Engine received connect error: %s", osync_error_print(&error));
		osync_obj_engine_set_error(engine, error);
		engine->sink_errors |= 1 << sinkengine->position;
		osync_status_update_member(engine->parent, osync_client_proxy_get_member(proxy), OSYNC_ENGINE_MEMBER_EVENT_ERROR, engine->objtype, error);
	} else {
		engine->sink_connects |= 1 << sinkengine->position;
		osync_status_update_member(engine->parent, osync_client_proxy_get_member(proxy), OSYNC_ENGINE_MEMBER_EVENT_CONNECTED, engine->objtype, NULL);
	}

	if (slowsync) {
		osync_obj_engine_set_slowsync(engine, TRUE);
		osync_trace(TRACE_INTERNAL, "SlowSync requested during connect.");
	}
			
	if (osync_bitcount(engine->sink_errors | engine->sink_connects) == osync_obj_engine_num_sinkengines(engine)) {
		if (osync_bitcount(engine->sink_errors)) {
			osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "At least one sink_engine failed while connecting");
			osync_obj_engine_set_error(engine, locerror);
		}

		osync_obj_engine_event(engine, OSYNC_ENGINE_EVENT_CONNECTED, locerror ? locerror : error);
	} else
		osync_trace(TRACE_INTERNAL, "Not yet: %i", osync_bitcount(engine->sink_errors | engine->sink_connects));
	
	osync_sink_engine_unref(sinkengine);
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void _osync_obj_engine_connect_done_callback(OSyncClientProxy *proxy, void *userdata, OSyncError *error)
{
	OSyncSinkEngine *sinkengine = userdata;
	OSyncObjEngine *engine = sinkengine->engine;
	OSyncError *locerror = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, proxy, userdata, error);
	
	if (error) {
		osync_obj_engine_set_error(engine, error);
		engine->sink_errors |= 1 << sinkengine->position;
		osync_status_update_member(engine->parent, osync_client_proxy_get_member(proxy), OSYNC_ENGINE_MEMBER_EVENT_ERROR, engine->objtype, error);
	} else {
		engine->sink_connect_done |= 1 << sinkengine->position;
		osync_status_update_member(engine->parent, osync_client_proxy_get_member(proxy), OSYNC_ENGINE_MEMBER_EVENT_CONNECT_DONE, engine->objtype, NULL);
	}
			
	if (osync_bitcount(engine->sink_errors | engine->sink_connect_done) == osync_obj_engine_num_sinkengines(engine)) {
		if (osync_bitcount(engine->sink_connect_done) < osync_bitcount(engine->sink_connects)) {
			osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "Fewer sink_engines reported connect_done than connected");
			osync_obj_engine_set_error(engine, locerror);
		}

		osync_obj_engine_event(engine, OSYNC_ENGINE_EVENT_CONNECT_DONE, locerror ? locerror : error);
	} else
		osync_trace(TRACE_INTERNAL, "Not yet: %i", osync_bitcount(engine->sink_errors | engine->sink_connect_done));
	
	osync_sink_engine_unref(sinkengine);
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void _osync_obj_engine_generate_event_disconnected(OSyncObjEngine *engine, OSyncError *error)
{
	OSyncError *locerror = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, engine);

	if (osync_bitcount(engine->sink_errors | engine->sink_disconnects) == osync_obj_engine_num_sinkengines(engine)) {
		if (osync_bitcount(engine->sink_disconnects) < osync_bitcount(engine->sink_connects)) {
			osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "Fewer sink_engines disconnected than connected");
			osync_obj_engine_set_error(engine, locerror);
			osync_error_unref(&locerror);
		}

		/* Since disconnect errors don't affect the data integrity keep the sync successful, even on
			 a disconnect error. So we have to avoid OSyncEngine->error got set, 
			 just keep this ObjEngine disconnect errors at this engine. */
		osync_obj_engine_event(engine, OSYNC_ENGINE_EVENT_DISCONNECTED, NULL);
	} else
		osync_trace(TRACE_INTERNAL, "Not yet: %i", osync_bitcount(engine->sink_errors | engine->sink_disconnects));

	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void _osync_obj_engine_disconnect_callback(OSyncClientProxy *proxy, void *userdata, OSyncError *error)
{
	OSyncSinkEngine *sinkengine = userdata;
	OSyncObjEngine *engine = sinkengine->engine;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, proxy, userdata, error);
	
	if (error) {
		osync_obj_engine_set_error(engine, error);
		engine->sink_errors |= 1 << sinkengine->position;
		osync_status_update_member(engine->parent, osync_client_proxy_get_member(proxy), OSYNC_ENGINE_MEMBER_EVENT_ERROR, engine->objtype, error);
	} else {
		engine->sink_disconnects |= 1 << sinkengine->position;
		osync_status_update_member(engine->parent, osync_client_proxy_get_member(proxy), OSYNC_ENGINE_MEMBER_EVENT_DISCONNECTED, engine->objtype, NULL);
	}
	
	_osync_obj_engine_generate_event_disconnected(engine, error);

	osync_sink_engine_unref(sinkengine);

	osync_trace(TRACE_EXIT, "%s", __func__);
}


static OSyncChange *_osync_obj_engine_clone_and_demerge_change(OSyncObjEngine *engine, OSyncCapabilities *caps, OSyncChange *change, OSyncError **error)
{
	OSyncList *mergers, *m;
	OSyncFormatEnv *formatenv = engine->formatenv;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %p)", __func__, engine, caps, change, error);

	OSyncChange *clone_change = osync_change_clone(change, error);
	if (!clone_change)
		goto error;


	//OSyncCapabilities *caps = osync_member_get_capabilities(member);
	const char *current_capsformat = osync_capabilities_get_format(caps);
	OSyncObjFormat *objformat = osync_change_get_objformat(clone_change);
	OSyncMerger *merger = osync_format_env_find_merger(formatenv,
			osync_objformat_get_name(objformat), osync_capabilities_get_format(caps));

	if (!merger) {

		osync_trace(TRACE_INTERNAL, "Couldn't found merger for capsformat \"%s\" and objformat \"%s\". Looking further.",
				current_capsformat, osync_objformat_get_name(objformat));

		mergers = osync_format_env_find_mergers_objformat(formatenv, osync_objformat_get_name(objformat));
		for (m = mergers; m; m = m->next) {
			OSyncMerger *it_merger = (OSyncMerger *) m->data;
			OSyncCapsConverter *caps_converter;
			const char *capsformat = osync_merger_get_capsformat(it_merger);

			/* Find OSyncCapsConverter */
			caps_converter = osync_format_env_find_caps_converter(formatenv, current_capsformat, capsformat);
			if (!caps_converter)
				continue;

			/* Convert capabilities if there is a chance to do so ... */
			if (!osync_caps_converter_invoke(caps_converter, &caps, NULL /* config */, error))
				goto error;


			// osync_member_set_capabilities(member, caps, error);

			merger = it_merger;

			break;
		}

	}

	if (!merger) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Couldn't handle the capabilities in format \"%s\" to process a reported change in format  \"%s\"",
				current_capsformat, osync_objformat_get_name(objformat));
		goto error;
	}

	if (!osync_merger_demerge(merger, clone_change, caps, error))
		goto error;

	osync_trace(TRACE_EXIT, "%s: %p", __func__, clone_change);
	return clone_change;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL; 
}

/* Finds the mapping to which the entry should belong. The
 * return value is MISMATCH if no mapping could be found,
 * SIMILAR if a mapping has been found but its not completely the same
 * SAME if a mapping has been found and is the same */
static OSyncConvCmpResult _osync_obj_engine_mapping_find(OSyncList *mapping_engines, OSyncChange *change, OSyncSinkEngine *sinkengine, OSyncMappingEngine **mapping_engine, OSyncError **error)
{	
	OSyncList *m = NULL;
	OSyncList *e = NULL;
	OSyncConvCmpResult result = OSYNC_CONV_DATA_MISMATCH;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %p)", __func__, mapping_engines, change, sinkengine, mapping_engine);

	for (m=mapping_engines; m && (result != OSYNC_CONV_DATA_SAME); m=m->next) {
		OSyncMappingEngine *tmp_mapping_engine = m->data;

		OSyncObjEngine *engine = tmp_mapping_engine->parent;
		OSyncGroup *group = osync_engine_get_group(engine->parent); 

		osync_bool merger_enabled = osync_group_get_merger_enabled(group);

		/* Go through the already existing mapping entries. We only consider mappings
		 * which dont have a entry on our side and where the data comparsion does not
		 * return MISMATCH */
		for (e = tmp_mapping_engine->entries; e; e = e->next) {
			OSyncMappingEntryEngine *entry_engine = e->data;
			OSyncChange *mapping_change = NULL;
			OSyncConvCmpResult tmp_result;

			/* if the mapping already has a entry on our side, its not worth looking */
			if (entry_engine->sink_engine == sinkengine)
				continue;

			mapping_change = osync_entry_engine_get_change(entry_engine);
			if (!mapping_change)
				continue;

			OSyncMember *member1 = osync_client_proxy_get_member(sinkengine->proxy);
			OSyncMember *member2 = osync_client_proxy_get_member(entry_engine->sink_engine->proxy);

			OSyncCapabilities *caps1 = NULL;
			OSyncCapabilities *caps2 = NULL;

			if (merger_enabled) {
				caps1 = osync_member_get_capabilities(member1);
				caps2 = osync_member_get_capabilities(member2);
			}

			OSyncChange *clone_change1 = NULL, *clone_change2 = NULL;
			OSyncChange *change1 = change;
			OSyncChange *change2 = mapping_change;

			osync_trace(TRACE_INTERNAL, "Member1-caps: %p Member2-caps: %p", caps1, caps2);

			if (caps2) {
				clone_change1 = _osync_obj_engine_clone_and_demerge_change(sinkengine->engine, caps2, change1, error);
				if (!clone_change1)
					goto error;

			}

			if (caps1) {
				clone_change2 = _osync_obj_engine_clone_and_demerge_change(sinkengine->engine, caps1, change2, error);
				if (!clone_change2)
					goto error;

			}


			if (clone_change1)
				change1 = clone_change1;

			if (clone_change2)
				change2 = clone_change2;

			tmp_result = osync_change_compare(change1, change2, error);

			if (caps2)
				osync_change_unref(clone_change1);

			if (caps1)
				osync_change_unref(clone_change2);

			if(tmp_result == OSYNC_CONV_DATA_SAME) {
				/* SAME is the best we can get */
				result = OSYNC_CONV_DATA_SAME;
				*mapping_engine = tmp_mapping_engine;
				break;
			} else if((tmp_result == OSYNC_CONV_DATA_SIMILAR) &&
				(result == OSYNC_CONV_DATA_MISMATCH))
			{
				/* SIMILAR is better than MISMATCH */
				result = OSYNC_CONV_DATA_SIMILAR;
				*mapping_engine = tmp_mapping_engine;
			} else if (tmp_result == OSYNC_CONV_DATA_UNKNOWN) {
				/* This is an error, handle th error. */
				goto error;
			}
		}
	}

	osync_trace(TRACE_EXIT, "%s: %d, %p", __func__, (int)result, *mapping_engine);
	return result;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return OSYNC_CONV_DATA_UNKNOWN;
}

osync_bool osync_obj_engine_map_changes(OSyncObjEngine *engine, OSyncError **error)
{
	OSyncMappingEngine *mapping_engine = NULL;
	OSyncList *new_mappings = NULL, *v = NULL;
	OSyncList *unmapped_mappings = NULL;
	OSyncConvCmpResult result = 0;
	OSyncMappingEntryEngine *entry_engine = NULL;
	OSyncChange *old_change;
	
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, engine);
	//osync_trace_disable();

	/* Go through all sink engines that are available */
	for (v = engine->sink_engines; v; v = v->next) {
		OSyncSinkEngine *sinkengine = v->data;
		
		/* We use a temp list to speed things up. We dont have to compare with newly created mappings for
		 * the current sinkengine, since there will be only one entry (for the current sinkengine) so there
		 * is no need to compare */

		osync_memberid memberid = osync_member_get_id(osync_client_proxy_get_member(sinkengine->proxy));
		osync_trace(TRACE_INTERNAL, "Sinkengine of member %i", memberid);

		unmapped_mappings = osync_list_copy(new_mappings);
		
		/* For each sinkengine, go through all unmapped changes */
		while (sinkengine->unmapped) {
			OSyncChange *change = sinkengine->unmapped->data;
			
			osync_trace(TRACE_INTERNAL, "Looking for mapping for change %s, changetype %i from member %i", osync_change_get_uid(change), osync_change_get_changetype(change), memberid);
	
			/* See if there is an exisiting mapping, which fits the unmapped change */
			result = _osync_obj_engine_mapping_find(unmapped_mappings, change, sinkengine, &mapping_engine, error);
			switch (result) {
				case OSYNC_CONV_DATA_MISMATCH:
					/* If there is none, create one */
					mapping_engine = _osync_obj_engine_create_mapping_engine(engine, error);
					if (!mapping_engine)
						goto error;
					
					osync_trace(TRACE_INTERNAL, "Unable to find mapping. Creating new mapping with id %i", osync_mapping_get_id(mapping_engine->mapping));
					/* TODO: what about _prepend (O(1)) instead of _append (O(n))? Order doesn't matter here - right? */
					new_mappings = osync_list_append(new_mappings, mapping_engine);
					unmapped_mappings = osync_list_append(unmapped_mappings, mapping_engine);
					break;
				case OSYNC_CONV_DATA_SIMILAR:
					mapping_engine->conflict = TRUE;
					break;
				case OSYNC_CONV_DATA_SAME:
					unmapped_mappings = osync_list_remove(unmapped_mappings, mapping_engine);
					mapping_engine->conflict = FALSE;
					break;
				case OSYNC_CONV_DATA_UNKNOWN:
					goto error;
					break;
			}

			/* Update the entry which belongs to our sinkengine with the the change */
			entry_engine = osync_mapping_engine_get_entry(mapping_engine, sinkengine);
			osync_assert(entry_engine);

			/* Don't overwrite unprefered entry_engines (e.g. SIMILAR).
			 *
			 * Secenario: First a mapping with SIMILAR get created. Later a mapping
			 * with SAME compare result gets detected. The SAME mapping engine get prefered.
			 * 
			 * The old change gets moved into a new mapping_engine.
			 */
			if ((old_change = osync_entry_engine_get_change(entry_engine))) {
				OSyncMappingEngine *old_mapping_engine = NULL;
				OSyncMappingEntryEngine *old_entry_engine = NULL;
				old_mapping_engine = _osync_obj_engine_create_mapping_engine(engine, error);
				if (!old_mapping_engine)
					goto error;
				
				new_mappings = osync_list_append(new_mappings, old_mapping_engine);
				unmapped_mappings = osync_list_append(unmapped_mappings, old_mapping_engine);

				old_entry_engine = osync_mapping_engine_get_entry(old_mapping_engine, sinkengine);
				osync_entry_engine_update(old_entry_engine, old_change);
			}
			
			osync_entry_engine_update(entry_engine, change);
			sinkengine->unmapped = osync_list_remove(sinkengine->unmapped, sinkengine->unmapped->data);
			osync_change_unref(change);
		}

		osync_list_free(unmapped_mappings);
		
	}

	engine->mapping_engines = osync_list_concat(engine->mapping_engines, new_mappings);
	
	//osync_trace_enable();
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

error:
	osync_trace_enable();
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static void _osync_obj_engine_read_ignored_callback(OSyncClientProxy *proxy, void *userdata, OSyncError *error)
{
	OSyncSinkEngine *sinkengine = userdata;
	/* OSyncObjEngine *engine = sinkengine->engine; */
	/* TODO: Share _generate_read_event fucntion with _osync_obj_engine_read_callback?
	   To report errors .. and handle _timeout problems of _read_ignored call.
	*/
	osync_sink_engine_unref(sinkengine);
}


static void _osync_obj_engine_read_callback(OSyncClientProxy *proxy, void *userdata, OSyncError *error)
{
	OSyncSinkEngine *sinkengine = userdata;
	OSyncObjEngine *engine = sinkengine->engine;
	OSyncError *locerror = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, proxy, userdata, error);
	
	if (error) {
		osync_obj_engine_set_error(engine, error);
		engine->sink_errors |= 1 << sinkengine->position;
		osync_status_update_member(engine->parent, osync_client_proxy_get_member(proxy), OSYNC_ENGINE_MEMBER_EVENT_ERROR, engine->objtype, error);
	} else {
		engine->sink_get_changes |= 1 << sinkengine->position;
		osync_status_update_member(engine->parent, osync_client_proxy_get_member(proxy), OSYNC_ENGINE_MEMBER_EVENT_READ, engine->objtype, NULL);
	}
	
	if (osync_bitcount(engine->sink_errors | engine->sink_get_changes) == osync_obj_engine_num_sinkengines(engine)) {
		
		if (osync_bitcount(engine->sink_get_changes) < osync_bitcount(engine->sink_connects)) {
			osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "Fewer sink_engines reported get_changes than connected");
			osync_obj_engine_set_error(engine, locerror);
		}

		osync_obj_engine_event(engine, OSYNC_ENGINE_EVENT_READ, locerror ? locerror : error);
	} else
		osync_trace(TRACE_INTERNAL, "Not yet: %i", osync_bitcount(engine->sink_errors | engine->sink_get_changes));
	
	osync_sink_engine_unref(sinkengine);
	osync_trace(TRACE_EXIT, "%s", __func__);
}

osync_bool osync_obj_engine_receive_change(OSyncObjEngine *objengine, OSyncClientProxy *proxy, OSyncChange *change, OSyncError **error)
{
	OSyncSinkEngine *sinkengine = NULL;
	OSyncList *e = NULL;
	
	osync_assert(objengine);
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %p)", __func__, objengine, proxy, change, error);
	
	/* Find the sinkengine for the proxy */
	sinkengine = osync_obj_engine_find_proxy_sinkengine(objengine, proxy);
	
	if (!sinkengine) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to find sinkengine");
		osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
		return FALSE;
	}
	
	/* We now have to see if the change matches one of the already existing mappings */
	for (e = sinkengine->entries; e; e = e->next) {
		OSyncMappingEntryEngine *mapping_engine = e->data;
		
		if (osync_entry_engine_matches(mapping_engine, change)) {
			osync_entry_engine_update(mapping_engine, change);
			
			osync_status_update_change(sinkengine->engine->parent, change, osync_client_proxy_get_member(proxy), mapping_engine->mapping_engine->mapping, OSYNC_ENGINE_CHANGE_EVENT_READ, NULL);
			
			osync_trace(TRACE_EXIT, "%s: Updated", __func__);
			return TRUE;
		}
	}
	
	osync_status_update_change(sinkengine->engine->parent, change, osync_client_proxy_get_member(proxy), NULL, OSYNC_ENGINE_CHANGE_EVENT_READ, NULL);
			
	/* If we couldnt find a match entry, we will append it the unmapped changes
	 * and take care of it later */
	sinkengine->unmapped = osync_list_append(sinkengine->unmapped, change);
	osync_change_ref(change);
	
	osync_trace(TRACE_EXIT, "%s: Unmapped", __func__);
	return TRUE;
}

/* Note: This function got shared between _osync_obj_engine_commit_change_callback() and
	 osync_obj_engine_written_callback(). Those function call _osync_obj_engine_generate_written_event()
	 with the most recent error and pass it to this function as last argument "error". If no error
	 appears this function got called with NULL as error.

	 It's quite important that this function only get called with the most recent error. This function
	 MUST NOT get called with (obj)engine->error.

	 If this functions doesn't get called with the most recent commit/committed_all error OSyncEngine
	 will get stuck. (testcases: dual_commit_error, dual_commit_timeout, *_commit_*, *_committed_all_*)
*/
static void _osync_obj_engine_generate_written_event(OSyncObjEngine *engine, OSyncError *error)
{
	osync_bool dirty = FALSE;
	OSyncList *p = NULL;
	OSyncList *e = NULL;
	OSyncSinkEngine *sinkengine = NULL;
	OSyncError *locerror = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, engine, error);
	/* We need to make sure that all entries are written ... */
	
	for (p = engine->sink_engines; p; p = p->next) {
		OSyncMember *member = NULL;
		OSyncObjTypeSink *objtype_sink = NULL;

		sinkengine = p->data;
		member = osync_client_proxy_get_member(sinkengine->proxy);
		objtype_sink = osync_member_find_objtype_sink(member, engine->objtype);

		/* If the sink engine isn't able/allowed to write we don't care if everything got written ("how dirty is it!?") */ 
		if (!objtype_sink || !osync_objtype_sink_get_write(objtype_sink)) 
			break;
		
		for (e = sinkengine->entries; e; e = e->next) {
			OSyncMappingEntryEngine *entry_engine = e->data;
			if (osync_entry_engine_is_dirty(entry_engine) == TRUE) {
				dirty = TRUE;
				break;
			}
		}
		if (dirty) {
			osync_trace(TRACE_EXIT, "%s: Still dirty", __func__);
			return;
		}
	}
	osync_trace(TRACE_INTERNAL, "%s: Not dirty anymore", __func__);

	/* And that we received the written replies from all sinks */
	/* TODO: Review if this is intended to even check for "dummy" sinks here!i
	 *        Other commands/devent checks for osync_obj_engine_num_sinkengines()! */
	if (osync_bitcount(engine->sink_errors | engine->sink_written) == osync_obj_engine_num_sinkengines(engine)) {
		if (osync_bitcount(engine->sink_written) < osync_bitcount(engine->sink_connects)) {
			osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "Fewer sink_engines reported committed all than connected");
			osync_obj_engine_set_error(engine, locerror);
		} else if (osync_bitcount(engine->sink_errors)) {
			/* Emit engine-wide error if one of the sinks got an error (tests: single_commit_error, ...) */
			osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "At least one Sink Engine failed while committing");
			osync_obj_engine_set_error(engine, locerror);
		}

		osync_obj_engine_event(engine, OSYNC_ENGINE_EVENT_WRITTEN, locerror ? locerror : error);

	} else
		osync_trace(TRACE_INTERNAL, "Not yet: %i", osync_bitcount(engine->sink_errors | engine->sink_written));

	osync_trace(TRACE_EXIT, "%s", __func__);
}

void osync_obj_engine_commit_change_callback(OSyncClientProxy *proxy, void *userdata, const char *uid, OSyncError *error)
{
	OSyncMappingEntryEngine *entry_engine = userdata;
	OSyncObjEngine *engine = entry_engine->objengine;
	OSyncSinkEngine *sinkengine = entry_engine->sink_engine;
	OSyncError *locerror = NULL;
	OSyncMapping *mapping = NULL;
	OSyncMember *member = NULL;
	OSyncMappingEntry *entry = NULL;
	const char *objtype = NULL;
	const char *objengine_objtype = NULL;
	osync_mappingid id = 0;

	objengine_objtype = osync_obj_engine_get_objtype(engine);
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %s, %p)", __func__, proxy, userdata, uid, error);
	
	osync_entry_engine_set_dirty(entry_engine, FALSE);
	
	mapping = entry_engine->mapping_engine->mapping;
	member = osync_client_proxy_get_member(proxy);
	entry = entry_engine->entry;
	objtype = osync_change_get_objtype(entry_engine->change);
	id = osync_mapping_entry_get_id(entry);
	
	if (error) {
		/* Error handling (tests: single_commit_error, ...) */

		/* TODO: Review differences between Mapping and Change status events - Are both really needed?! */
		osync_status_update_change(engine->parent, entry_engine->change, osync_client_proxy_get_member(proxy), entry_engine->mapping_engine->mapping, OSYNC_ENGINE_CHANGE_EVENT_ERROR, error);
		osync_status_update_mapping(engine->parent, entry_engine->mapping_engine, OSYNC_ENGINE_MAPPING_EVENT_ERROR, error);

		osync_obj_engine_set_error(engine, error);
		engine->sink_errors |= 1 << sinkengine->position;
		goto error;
	}
	
	if (uid)
		osync_change_set_uid(entry_engine->change, uid);
	
	if (engine->archive) {
		if (osync_change_get_changetype(entry_engine->change) == OSYNC_CHANGE_TYPE_DELETED) {
			/* TODO error handling */
			osync_archive_delete_change(engine->archive, id, objtype, &locerror);
		} else {

			/* TODO error handling */
			osync_archive_save_change(engine->archive, id, osync_change_get_uid(entry_engine->change), objtype, osync_mapping_get_id(mapping), osync_member_get_id(member), objengine_objtype, &locerror);
		}
	}

	osync_assert(entry_engine->mapping_engine);
	osync_status_update_change(engine->parent, entry_engine->change, osync_client_proxy_get_member(proxy), entry_engine->mapping_engine->mapping, OSYNC_ENGINE_CHANGE_EVENT_WRITTEN, NULL);
	osync_entry_engine_update(entry_engine, NULL);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;

 error:	
	_osync_obj_engine_generate_written_event(engine, error);
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(&error));
}

void osync_obj_engine_written_callback(OSyncClientProxy *proxy, void *userdata, OSyncError *error)
{
	OSyncSinkEngine *sinkengine = userdata;
	OSyncObjEngine *engine = sinkengine->engine;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, proxy, userdata, error);
	
	if (error) {
		osync_obj_engine_set_error(engine, error);
		engine->sink_errors |= 1 << sinkengine->position;
		osync_status_update_member(engine->parent, osync_client_proxy_get_member(proxy), OSYNC_ENGINE_MEMBER_EVENT_ERROR, engine->objtype, error);
	} else {
		engine->sink_written |= 1 << sinkengine->position;
		osync_status_update_member(engine->parent, osync_client_proxy_get_member(proxy), OSYNC_ENGINE_MEMBER_EVENT_WRITTEN, engine->objtype, NULL);
	}
			
	_osync_obj_engine_generate_written_event(engine, error);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void _osync_obj_engine_sync_done_callback(OSyncClientProxy *proxy, void *userdata, OSyncError *error)
{
	OSyncSinkEngine *sinkengine = userdata;
	OSyncObjEngine *engine = sinkengine->engine;
	OSyncError *locerror = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, proxy, userdata, error);
	
	if (error) {
		osync_obj_engine_set_error(engine, error);
		engine->sink_errors |= 1 << sinkengine->position;
		osync_status_update_member(engine->parent, osync_client_proxy_get_member(proxy), OSYNC_ENGINE_MEMBER_EVENT_ERROR, engine->objtype, error);
	} else {
		engine->sink_sync_done |= 1 << sinkengine->position;
		osync_status_update_member(engine->parent, osync_client_proxy_get_member(proxy), OSYNC_ENGINE_MEMBER_EVENT_SYNC_DONE, engine->objtype, NULL);
	}
			
	if (osync_bitcount(engine->sink_errors | engine->sink_sync_done) == osync_obj_engine_num_sinkengines(engine)) {
		if (osync_bitcount(engine->sink_sync_done) < osync_bitcount(engine->sink_connects)) {
			osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "Fewer sink_engines reported sync_done than connected");
			osync_obj_engine_set_error(engine, locerror);
		}

		osync_obj_engine_event(engine, OSYNC_ENGINE_EVENT_SYNC_DONE, locerror ? locerror : error);
	} else
		osync_trace(TRACE_INTERNAL, "Not yet: %i", osync_bitcount(engine->sink_errors | engine->sink_sync_done));
	
	osync_sink_engine_unref(sinkengine);
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static osync_bool _create_mapping_engines(OSyncObjEngine *engine, OSyncError **error)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, engine, error);
	
	OSyncList *mappings = osync_mapping_table_get_mappings(engine->mapping_table);
	OSyncList *m = NULL;
	
	for (m = mappings; m; m = m->next) {
		OSyncMapping *mapping = (OSyncMapping*)m->data;
		
		OSyncMappingEngine *mapping_engine = osync_mapping_engine_new(engine, mapping, error);
		if (!mapping_engine)
			goto error;
		
		engine->mapping_engines = osync_list_append(engine->mapping_engines, mapping_engine);
	}
	
	osync_list_free(mappings);
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error:
	osync_list_free(mappings);
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool _inject_changelog_entries(OSyncObjEngine *engine, OSyncError **error) {
	OSyncList *ids = NULL;
	OSyncList *changetypes = NULL, *memberids = NULL;
	OSyncList *j = NULL, *t = NULL, *mid = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, engine);

	osync_assert(engine);
	osync_assert(engine->archive);
	osync_assert(engine->objtype);
	
	if (!osync_archive_load_ignored_conflicts(engine->archive, engine->objtype, &memberids, &ids, &changetypes, error)) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
		return FALSE;
	}

	t = changetypes;
	mid = memberids;
	for (j = ids; j; j = j->next) {
		osync_archiveid id = GPOINTER_TO_INT(j->data);

		OSyncMapping *ignored_mapping = osync_mapping_table_find_mapping(engine->mapping_table, id);

		OSyncList *e;
		for (e = engine->mapping_engines; e; e = e->next) {
			OSyncMappingEngine *mapping_engine = e->data;

			if (mapping_engine->mapping == ignored_mapping) {
				OSyncList *m;

				for (m = mapping_engine->entries; m; m = m->next) {
					long long int memberid = (long long int)GPOINTER_TO_INT(mid->data);
					OSyncMappingEntryEngine *entry = osync_mapping_engine_find_entry_by_memberid(mapping_engine, memberid);
					OSyncChangeType changetype = (OSyncChangeType) t->data;
					OSyncObjFormat *dummyformat = NULL;
					OSyncData *data = NULL;
					OSyncChange *ignored_change = osync_change_new(error);

					osync_change_set_changetype(ignored_change, changetype); 

					dummyformat = osync_objformat_new("plain", engine->objtype, NULL);
					data = osync_data_new(NULL, 0, dummyformat, NULL);
					osync_objformat_unref(dummyformat);
					osync_change_set_data(ignored_change, data);
					osync_data_unref(data);

					osync_change_set_uid(ignored_change, osync_mapping_entry_get_uid(entry->entry));

					osync_entry_engine_update(entry, ignored_change);
					osync_change_unref(ignored_change);

					osync_trace(TRACE_INTERNAL, "CHANGE: %p", entry->change);
				}
				break;
			}
		}

		t = t->next;
		mid = mid->next;
	}

	osync_list_free(memberids);
	osync_list_free(ids);
	osync_list_free(changetypes);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
}


OSyncObjEngine *osync_obj_engine_new(OSyncEngine *parent, const char *objtype, OSyncFormatEnv *formatenv, OSyncError **error)
{
	OSyncObjEngine *engine = NULL;
	osync_assert(parent);
	osync_assert(objtype);
	
	osync_trace(TRACE_ENTRY, "%s(%p, %s, %p, %p)", __func__, parent, objtype, formatenv, error);
	
	engine = osync_try_malloc0(sizeof(OSyncObjEngine), error);
	if (!engine)
		goto error;
	engine->ref_count = 1;
	engine->slowsync = FALSE;
	engine->written = FALSE;
	
	/* we dont reference the parent to avoid circular dependencies. This object is completely
	 * dependent on the engine anyways */
	engine->parent = parent;
	
	engine->objtype = osync_strdup(objtype);
	engine->formatenv = osync_format_env_ref(formatenv);
	
	engine->mapping_table = osync_mapping_table_new(error);
	if (!engine->mapping_table)
		goto error_free_engine;
	
	engine->archive = osync_engine_get_archive(parent);
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, engine);
	return engine;

 error_free_engine:
	osync_obj_engine_unref(engine);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

OSyncObjEngine *osync_obj_engine_ref(OSyncObjEngine *engine)
{
	osync_assert(engine);
	
	g_atomic_int_inc(&(engine->ref_count));

	return engine;
}

void osync_obj_engine_unref(OSyncObjEngine *engine)
{
	osync_assert(engine);
		
	if (g_atomic_int_dec_and_test(&(engine->ref_count))) {
		while (engine->sink_engines) {
			OSyncSinkEngine *sinkengine = engine->sink_engines->data;
			osync_sink_engine_unref(sinkengine);
			
			engine->sink_engines = osync_list_remove(engine->sink_engines, sinkengine);
		}
		
		while (engine->mapping_engines) {
			OSyncMappingEngine *mapping_engine = engine->mapping_engines->data;
			osync_mapping_engine_unref(mapping_engine);
			
			engine->mapping_engines = osync_list_remove(engine->mapping_engines, mapping_engine);
		}
		
		if (engine->error)
			osync_error_unref(&engine->error);
			
		if (engine->objtype)
			osync_free(engine->objtype);
		
		if (engine->mapping_table)
			osync_mapping_table_unref(engine->mapping_table);

		if (engine->formatenv)
			osync_format_env_unref(engine->formatenv);
		
		osync_free(engine);
	}
}

static int _osync_obj_engine_num_write_sinks(OSyncObjEngine *objengine) {
	int num = 0;
	OSyncList *p = NULL;
	OSyncSinkEngine *sink;

	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, objengine);

	for (p = objengine->sink_engines; p; p = p->next) {
		OSyncMember *member = NULL;
		OSyncObjTypeSink *objtype_sink = NULL;

		sink = p->data;
		member = osync_client_proxy_get_member(sink->proxy);
		objtype_sink = osync_member_find_objtype_sink(member, objengine->objtype);

		/* Is the objtype_sink writable? */
		if (objtype_sink && osync_objtype_sink_get_write(objtype_sink)) {
			num++;
		}

	}

	osync_trace(TRACE_EXIT, "%s: %i", __func__, num);
	return num;
}

osync_bool osync_obj_engine_initialize(OSyncObjEngine *engine, OSyncError **error)
{
	const char *objtype = NULL;
	int num = 0;
	int i = 0;

	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, engine, error);

	osync_trace(TRACE_INTERNAL, "Loaded %i mappings", osync_mapping_table_num_mappings(engine->mapping_table));

	objtype = osync_obj_engine_get_objtype(engine);
	
	num = osync_engine_num_proxies(engine->parent);
	for (i = 0; i < num; i++) {
		OSyncClientProxy *proxy = osync_engine_nth_proxy(engine->parent, i);
		OSyncObjTypeSink *sink = osync_client_proxy_find_objtype_sink(proxy, objtype);
		OSyncSinkEngine *sinkengine = NULL; 

		
		sinkengine = osync_sink_engine_new(i, proxy, engine, error);
		if (!sinkengine)
			goto error;

		if (!sink) 
			engine->dummy_sink_engines = osync_list_append(engine->dummy_sink_engines, sinkengine);
		else
			engine->active_sink_engines = osync_list_append(engine->active_sink_engines, sinkengine);

		engine->sink_engines = osync_list_append(engine->sink_engines, sinkengine);
	}

	if (engine->archive && engine->slowsync) {
		if (!osync_mapping_table_flush(engine->mapping_table, engine->archive, engine->objtype, error))
			goto error;
	}

	if (engine->archive) {
		if (!osync_mapping_table_load(engine->mapping_table, engine->archive, engine->objtype, error))
			goto error;
	}

	if (!_create_mapping_engines(engine, error))
		goto error;
	
	osync_trace(TRACE_INTERNAL, "Created %i mapping engine", osync_list_length(engine->mapping_engines));

	if (engine->archive) {
		/* inject ignored conflicts from previous syncs */
		if (!_inject_changelog_entries(engine, error))
			goto error;
	}

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
 error:

	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

void osync_obj_engine_finalize(OSyncObjEngine *engine)
{
	OSyncMappingEngine *mapping_engine;
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, engine);

	engine->slowsync = FALSE;
	engine->written = FALSE;

	engine->sink_errors = 0;
	engine->sink_connects = 0;
	engine->sink_connect_done = 0;
	engine->sink_disconnects = 0;
	engine->sink_get_changes = 0;
	engine->sink_sync_done = 0;
	engine->sink_written = 0;

	engine->conflicts_solved = 0;

	while (engine->sink_engines) {
		OSyncSinkEngine *sinkengine = engine->sink_engines->data;
		osync_sink_engine_unref(sinkengine);
		
		engine->sink_engines = osync_list_remove(engine->sink_engines, sinkengine);
	}

	osync_list_free(engine->active_sink_engines);
	osync_list_free(engine->dummy_sink_engines);
	engine->active_sink_engines = NULL;
	engine->dummy_sink_engines = NULL;
	
	while (engine->conflicts) {
		mapping_engine = engine->conflicts->data;
		/* No need to unref the mapping engine. They get unref while emptying
			 the mapping_engines list. See next loop. */
		engine->conflicts = osync_list_remove(engine->conflicts, mapping_engine);
	}

	while (engine->mapping_engines) {
		mapping_engine = engine->mapping_engines->data;
		osync_mapping_engine_unref(mapping_engine);
		
		engine->mapping_engines = osync_list_remove(engine->mapping_engines, mapping_engine);
	}
	
	if (engine->mapping_table)
		osync_mapping_table_close(engine->mapping_table);

	osync_trace(TRACE_EXIT, "%s", __func__);
}

const char *osync_obj_engine_get_objtype(OSyncObjEngine *engine)
{
	osync_return_val_if_fail(engine, "");
	return engine->objtype;
}

void osync_obj_engine_set_slowsync(OSyncObjEngine *engine, osync_bool slowsync)
{
	osync_assert(engine);
	engine->slowsync = slowsync;
}

osync_bool osync_obj_engine_get_slowsync(OSyncObjEngine *engine)
{
	osync_assert(engine);
	return engine->slowsync;
}

osync_bool osync_obj_engine_command(OSyncObjEngine *engine, OSyncEngineCmd cmd, OSyncError **error)
{
	OSyncList *p = NULL;
	OSyncList *o = NULL;
	OSyncSinkEngine *sinkengine = NULL;

	
	osync_trace(TRACE_ENTRY, "%s(%p:%s, %s, %p)", __func__, 
			engine, osync_obj_engine_get_objtype(engine),
			osync_engine_get_cmdstr(cmd), error);

	osync_assert(engine);
	
	int write_sinks = 0;
	osync_bool proxy_disconnect = FALSE;
	
	switch (cmd) {
	case OSYNC_ENGINE_COMMAND_CONNECT:
		for (p = engine->active_sink_engines; p; p = p->next) {
			sinkengine = p->data;

			if (!osync_client_proxy_connect(sinkengine->proxy, _osync_obj_engine_connect_callback, sinkengine, engine->objtype, engine->slowsync, error))
				goto error;
			osync_sink_engine_ref(sinkengine); // Note that connect callback has a reference
		}
		break;
	case OSYNC_ENGINE_COMMAND_CONNECT_DONE:
		for (p = engine->active_sink_engines; p; p = p->next) {
			sinkengine = p->data;

			if (!osync_client_proxy_connect_done(sinkengine->proxy, _osync_obj_engine_connect_done_callback, sinkengine, engine->objtype, engine->slowsync, error))
				goto error;
			osync_sink_engine_ref(sinkengine); // Note that connect_done callback has a reference
		}
		break;
	case OSYNC_ENGINE_COMMAND_READ:
		for (p = engine->active_sink_engines; p; p = p->next) {
			sinkengine = p->data;

			for (o = sinkengine->entries; o; o = o->next) {
				OSyncMappingEntryEngine *entry = o->data;
				OSyncChange *change = entry->change;

				if (!change)
					continue;

				if (!osync_client_proxy_read(sinkengine->proxy, _osync_obj_engine_read_ignored_callback, sinkengine, change, error))
					goto error;
				osync_sink_engine_ref(sinkengine); // Note that read_ignored callback has a reference
			}
		}

		if (engine->archive) {
			/* Flush the changelog - to avoid double entries of ignored entries */
			if (!osync_archive_flush_ignored_conflict(engine->archive, engine->objtype, error))
				goto error;
		}

		write_sinks = _osync_obj_engine_num_write_sinks(engine);

		/* Get change entries since last sync. (get_changes) */
		for (p = engine->active_sink_engines; p; p = p->next) {
			OSyncMember *member = NULL;
			OSyncObjTypeSink *objtype_sink = NULL;

			sinkengine = p->data;

			member = osync_client_proxy_get_member(sinkengine->proxy);

			objtype_sink = osync_member_find_objtype_sink(member, engine->objtype);

			/* Is there at least one other writeable sink? */
			if (((objtype_sink && osync_objtype_sink_get_write(objtype_sink)) && write_sinks == 1) || write_sinks == 0) {
				osync_sink_engine_ref(sinkengine); // Note that read callback has a reference
				_osync_obj_engine_read_callback(sinkengine->proxy, sinkengine, *error);
				osync_trace(TRACE_INTERNAL, "no other writable sinks (write_sinks = %d) .... SKIP", write_sinks);
				continue;
			}

			if (!osync_client_proxy_get_changes(sinkengine->proxy, _osync_obj_engine_read_callback, sinkengine, engine->objtype, engine->slowsync, error))
				goto error;
			osync_sink_engine_ref(sinkengine); // Note that read callback has a reference
		}

		break;
	case OSYNC_ENGINE_COMMAND_PREPARE_MAP:

		/* TODO: PLACEHOLDER for conversion and merge */

		osync_obj_engine_event(engine, OSYNC_ENGINE_EVENT_PREPARED_MAP, *error);
		break;
	case OSYNC_ENGINE_COMMAND_MAP:
		/* We are now done reading the changes. so we can now start to create the mappings, conflicts etc */
		if (osync_obj_engine_map_changes(engine, error)) {
			for (p = engine->mapping_engines; p; p = p->next) {
				OSyncMappingEngine *mapping_engine = p->data;

				/* TODO: Is this still needeD? synced does get set in WRITE event - right? */
				if (mapping_engine->synced)
					continue;

				if (!osync_mapping_engine_check_conflict(mapping_engine)) {
					osync_error_set(error, OSYNC_ERROR_GENERIC, "Error while resolving conflicts");
					break;
					/* Don't jump to error - in this case the main engine already
					 * is in error state. And will abort on the next command cycle.
					 * Leave!
					 */
				}
			}
		}

		osync_obj_engine_event(engine, OSYNC_ENGINE_EVENT_MAPPED, *error);
		if (*error) {
			osync_obj_engine_set_error(engine, *error);
			osync_error_unref(error);
		}

		break;
	case OSYNC_ENGINE_COMMAND_END_CONFLICTS:

		osync_trace(TRACE_INTERNAL, "Check for pending conflicts");

		if (engine->conflicts) {
			osync_trace(TRACE_INTERNAL, "Delay. Total pending conflicts: %u", osync_list_length(engine->conflicts));
			break;
		}

		if (engine->conflicts_solved) {
			osync_trace(TRACE_INTERNAL, "Conflicts already solved.");
			break;
		}

		engine->conflicts_solved = TRUE;

		osync_obj_engine_event(engine, OSYNC_ENGINE_EVENT_END_CONFLICTS, *error);
		if (*error) {
			osync_obj_engine_set_error(engine, *error);
			osync_error_unref(error);
		}

		break;
	case OSYNC_ENGINE_COMMAND_MULTIPLY:

		/* Now we can multiply the winner in the mapping */
		osync_trace(TRACE_INTERNAL, "Multiplying %u mappings", osync_list_length(engine->mapping_engines));
		for (p = engine->mapping_engines; p; p = p->next) {
			OSyncMappingEngine *mapping_engine = p->data;
			if (!osync_mapping_engine_multiply(mapping_engine, error))
				break;
		}

		osync_obj_engine_event(engine, OSYNC_ENGINE_EVENT_MULTIPLIED, *error);

		break;
	case OSYNC_ENGINE_COMMAND_PREPARE_WRITE:

		osync_obj_engine_prepare_write(engine, error);

		osync_obj_engine_event(engine, OSYNC_ENGINE_EVENT_PREPARED_WRITE, *error);
		break;
	case OSYNC_ENGINE_COMMAND_WRITE:
		if (engine->conflicts) {
			osync_trace(TRACE_INTERNAL, "We still have conflict. Delaying write");
			break;
		}
			
		if (engine->written) {
			osync_trace(TRACE_INTERNAL, "Already written");
			break;
		}
				
		engine->written = TRUE;


		osync_trace(TRACE_INTERNAL, "Starting to write");
		if (!osync_obj_engine_write(engine, error))
			goto error;

		/* TODO: Reviewe error handling here! */

		break;
	case OSYNC_ENGINE_COMMAND_SYNC_DONE:
		for (p = engine->active_sink_engines; p; p = p->next) {
			sinkengine = p->data;

			if (!osync_client_proxy_sync_done(sinkengine->proxy, _osync_obj_engine_sync_done_callback, sinkengine, engine->objtype, error))
				goto error;
			osync_sink_engine_ref(sinkengine); // Note that sync_done callback has a reference
		}
		break;
	case OSYNC_ENGINE_COMMAND_DISCONNECT:;
		for (p = engine->active_sink_engines; p; p = p->next) {
			sinkengine = p->data;

			/* Don't call client disconnect functions if the sink is already disconnected.
			   This avoids unintended disconnect calls of clients/plugins which might not prepared
			   for a disconnect call when their never got connected. (testcases: *_connect_error, *_connect_timeout ..) */
			if (!osync_sink_engine_is_connected(sinkengine))
				continue;

			proxy_disconnect = TRUE;

			if (!osync_client_proxy_disconnect(sinkengine->proxy, _osync_obj_engine_disconnect_callback, sinkengine, engine->objtype, error))
				goto error;
			osync_sink_engine_ref(sinkengine); // Note that disconnect callback has a reference
		}
			
		/* If no client needs to be disconnected, we MUST NOT expected any 
			 disconnected_callback which generates an OSYNC_ENGINE_EVENT_DISCONNECTED event.
			 So we directly generate such event on our own. (testcases: double_connect_*, triple_connnect_*) */ 
		if (!proxy_disconnect)
			_osync_obj_engine_generate_event_disconnected(engine, NULL);

		break;
	case OSYNC_ENGINE_COMMAND_SOLVE:
	case OSYNC_ENGINE_COMMAND_DISCOVER:
	case OSYNC_ENGINE_COMMAND_ABORT:
		break;
	}
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}


void osync_obj_engine_event(OSyncObjEngine *engine, OSyncEngineEvent event, OSyncError *error)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %s, %p)", __func__, engine, osync_engine_get_eventstr(event), error);
	osync_assert(engine);

	/* TODO: Create own enum OSyncObjEngine for objengine events. */
	osync_assert_msg(event != OSYNC_ENGINE_EVENT_ERROR, "OSyncObjEngine isn't supposed to emit OSYNC_ENGINE_EVENT_ERROR events!");
	
	/* engine event callback gets called with most recent OSyncError or NULL.
	   Don't use engine->error for the engine event callback. Previous appears errors
	   in this objengine would get passed to this engine, which will be interpeted by the
	   engine as error for the current event.

	   Example:
	   
	   EVENT_CONNECTED		(obj)engine->error: NULL
	   EVENT_READ **ERROR**		(obj)engine->error: 0x....
	   # OSyncEngine aborts sync and emit disconnect event, we reply with:
	   EVENT_DISCONNECTED		(obj)engine->error: 0x.....

	   If we would pass in this case enigne->error instead of the most recent
	   OSyncError, OSyncEngien would interpret this as the disconnect failed as well.
	   So we just pass the most recent OSyncError pointer, which could be NULL -> no error.
	*/
	 
	engine->callback(engine, event, error, engine->callback_userdata);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
}

void osync_obj_engine_set_callback(OSyncObjEngine *engine, OSyncObjEngineEventCallback callback, void *userdata)
{
	osync_assert(engine);
	engine->callback = callback;
	engine->callback_userdata = userdata;
}

void osync_obj_engine_set_error(OSyncObjEngine *engine, OSyncError *error)
{
	osync_assert(engine);
	if (engine->error) {
		osync_error_stack(&error, &engine->error);
		osync_error_unref(&engine->error);
	}
	engine->error = error;
	osync_error_ref(&error);
}

OSyncSinkEngine *osync_obj_engine_find_proxy_sinkengine(OSyncObjEngine *engine, OSyncClientProxy *proxy)
{
	OSyncList *s;
	OSyncSinkEngine *sinkengine = NULL;
	osync_return_val_if_fail(engine, NULL);
	osync_return_val_if_fail(proxy, NULL);

	for (s = engine->sink_engines; s; s = s->next) {
		sinkengine = s->data;
		if (sinkengine->proxy == proxy)
			break;
		sinkengine = NULL;
	}

	return sinkengine; 
}

OSyncSinkEngine *osync_obj_engine_nth_sinkengine(OSyncObjEngine *engine, unsigned int nth)
{
	osync_return_val_if_fail(engine, NULL);
	/* This is an public interace - we only deal here with "real" and active
	 * sink_engines. Not with dummies.
	 */
	return osync_list_nth_data(engine->active_sink_engines, nth);
}

unsigned int osync_obj_engine_num_sinkengines(OSyncObjEngine *engine)
{
	osync_assert(engine);
	/* This is an public interace - we only deal here with "real" and active
	 * sink_engines. Not with dummies.
	 */
	return osync_list_length(engine->active_sink_engines);
}

OSyncList *osync_obj_engine_get_sinkengines(OSyncObjEngine *engine) {
	return osync_list_copy(engine->active_sink_engines);
}

unsigned int osync_obj_engine_num_mapping_engines(OSyncObjEngine *engine)
{
	osync_assert(engine);
	return osync_list_length(engine->mapping_engines);
}

unsigned int osync_obj_engine_num_members(OSyncObjEngine *engine)
{
	osync_assert(engine);
	return osync_obj_engine_num_sinkengines(engine);
}

OSyncMember *osync_obj_engine_nth_member(OSyncObjEngine *engine, unsigned int nth)
{
	OSyncSinkEngine *sinkengine;
	osync_return_val_if_fail(engine, NULL);
	sinkengine = osync_list_nth_data(engine->active_sink_engines, nth);
	osync_assert(sinkengine);
	return osync_sink_engine_get_member(sinkengine);
}

OSyncList *osync_obj_engine_get_members(OSyncObjEngine* engine) {
	
	OSyncList *list = engine->active_sink_engines;
	OSyncList *new_list = NULL;
	OSyncSinkEngine *sinkengine = NULL;
	
	if (list) {
		OSyncList *last;

		new_list = osync_list_alloc();
		sinkengine = list->data;
		new_list->data = osync_sink_engine_get_member(sinkengine);
		new_list->prev = NULL;
		last = new_list;
		list = list->next;
		while (list) {
			last->next = osync_list_alloc();
			last->next->prev = last;
			last = last->next;
			sinkengine = list->data;
			last->data = osync_sink_engine_get_member(sinkengine);
			list = list->next;
		}
		last->next = NULL;
	}

	return new_list;
}

const OSyncList *osync_obj_engine_get_mapping_entry_engines_of_member(OSyncObjEngine *engine, OSyncMember *member)
{
	OSyncList *s;
	osync_return_val_if_fail(engine, NULL);
	osync_return_val_if_fail(member, NULL);

	for (s = engine->active_sink_engines; s; s = s->next) {
		OSyncSinkEngine *sinkengine = s->data;

		if (member != osync_sink_engine_get_member(sinkengine))
			continue;

		return osync_sink_engine_get_mapping_entry_engines(sinkengine);
	}

	return NULL;
}

osync_bool osync_obj_engine_prepare_write(OSyncObjEngine *engine, OSyncError **error)
{
	OSyncList *p;
	osync_bool merger_enabled, converter_enabled;
	OSyncGroup *group;

	osync_assert(engine);

	group = osync_engine_get_group(engine->parent); 
	merger_enabled = osync_group_get_merger_enabled(group);
	converter_enabled = osync_group_get_converter_enabled(group); 

	if (!merger_enabled && !converter_enabled)
		return TRUE;

	for (p = engine->active_sink_engines; p; p = p->next) {
		OSyncSinkEngine *sinkengine = p->data;

		if (merger_enabled
			&& !osync_sink_engine_demerge(sinkengine, engine->archive, error))
			goto error;

		if (converter_enabled
			&& !osync_sink_engine_convert_to_dest(sinkengine, engine->formatenv, error))
			goto error;

	}

	return TRUE;

error:
	return FALSE;
}

osync_bool osync_obj_engine_write(OSyncObjEngine *engine, OSyncError **error)
{
	OSyncList *p;
	OSyncMember *member;
	OSyncObjTypeSink *objtype_sink;

	osync_assert(engine);

	for (p = engine->active_sink_engines; p; p = p->next) {
		OSyncSinkEngine *sinkengine = p->data;

		member = osync_client_proxy_get_member(sinkengine->proxy);
		objtype_sink = osync_member_find_objtype_sink(member, engine->objtype);

		/* TODO: Review if objtype_sink = NULL is valid at all. */
		osync_assert(objtype_sink);

		/* Only commit change if the objtype sink is able/allowed to write. */
		if (!osync_objtype_sink_get_write(objtype_sink)) 
			continue;

		if (!osync_sink_engine_write(sinkengine, engine->archive, error))
			goto error;
	}

	return TRUE;

error:
	return FALSE;
}


osync_bool osync_objengine_uid_update(OSyncObjEngine *engine, OSyncClientProxy *proxy, const char *olduid, const char *newuid, OSyncError **error)
{
	long long int memberid = osync_member_get_id(osync_client_proxy_get_member(proxy));

	if (!osync_archive_update_change_uid(engine->archive, olduid, newuid, memberid, engine->objtype, error))
		goto error;

	return TRUE;

error:
	return FALSE;
}

