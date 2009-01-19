/*
 * libopensync - A synchronization engine for the opensync framework
 * Copyright (C) 2004-2005  Armin Bauer <armin.bauer@opensync.org>
 * Copyright (C) 2007-2008  Daniel Gollub <dgollub@suse.de>
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

#include "opensync-client.h"
#include "opensync-engine.h"
#include "opensync-group.h"
#include "opensync-format.h"
#include "opensync-data.h"
#include "opensync-plugin.h"
#include "opensync-archive.h"
#include "opensync-merger.h"
#include "opensync-xmlformat.h"

#include "archive/opensync_archive_internals.h"
#include "client/opensync_client_proxy_internals.h"
#include "group/opensync_group_internals.h"
#include "group/opensync_member_internals.h"
#include "format/opensync_objformat_internals.h"

#include "opensync_status_internals.h"
#include "opensync_obj_engine_internals.h"
#include "opensync_sink_engine_internals.h"
#include "opensync_mapping_entry_engine_internals.h"

#include "opensync_engine.h"
#include "opensync_engine_private.h"
#include "opensync_engine_internals.h"


#ifdef OPENSYNC_UNITTESTS
#include "xmlformat/opensync-xmlformat_internals.h"
#endif

void osync_engine_set_error(OSyncEngine *engine, OSyncError *error)
{
	osync_assert(engine);
	if (engine->error) {
		osync_error_stack(&error, &engine->error);
		osync_error_unref(&engine->error);
	}
	
	engine->error = error;
	if (error)
		osync_error_ref(&error);
}
 
osync_bool osync_engine_has_error(OSyncEngine *engine)
{
	osync_assert(engine);
	return engine->error ? TRUE : FALSE;
}

static void _finalize_callback(OSyncClientProxy *proxy, void *userdata, OSyncError *error)
{
	OSyncEngine *engine = userdata;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, proxy, userdata, error);
	
	if (error) {
		osync_engine_set_error(engine, error);
	}
	
	engine->busy = FALSE;
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static gboolean _command_prepare(GSource *source, gint *timeout_)
{
	*timeout_ = 1;
	return FALSE;
}

static gboolean _command_check(GSource *source)
{
	OSyncEngine *engine = *((OSyncEngine **)(source + 1));
	if (g_async_queue_length(engine->command_queue) > 0)
		return TRUE;
	
	return FALSE;
}

void osync_engine_command(OSyncEngine *engine, OSyncEngineCommand *command);

static OSyncObjFormat *_osync_engine_get_internal_format(OSyncEngine *engine, const char *objtype)
{
	char *format = g_hash_table_lookup(engine->internalFormats, objtype);
	if (!format)
		return NULL;
	return osync_format_env_find_objformat(engine->formatenv, format);
}

static int _osync_engine_get_proxy_position(OSyncEngine *engine, OSyncClientProxy *proxy)
{
	int ret = 0;
	osync_assert(engine);
	osync_assert(proxy);

	ret = osync_list_index(engine->proxies, proxy);

	osync_assert(ret >= 0);

	return ret;
}

static osync_bool _osync_engine_is_proxy_connected(OSyncEngine *engine, OSyncClientProxy *proxy)
{
	osync_assert(engine);
	osync_assert(proxy);

	return !!(engine->proxy_connects & (1 << _osync_engine_get_proxy_position(engine, proxy)));
}

static int _osync_engine_get_objengine_position(OSyncEngine *engine, OSyncObjEngine *objengine)
{
	int ret = 0;
	osync_assert(engine);
	osync_assert(objengine);

	ret = osync_list_index(engine->object_engines, objengine);

	osync_assert(ret >= 0);

	return ret;
}

static void _osync_engine_set_internal_format(OSyncEngine *engine, const char *objtype, OSyncObjFormat *format)
{
	if (!format)
		return;

	osync_trace(TRACE_INTERNAL, "Setting internal format of %s to %p:%s", objtype, format, osync_objformat_get_name(format));
	g_hash_table_insert(engine->internalFormats, osync_strdup(objtype), osync_strdup(osync_objformat_get_name(format)));
}

static OSyncFormatConverterPath *_osync_engine_get_converter_path(OSyncEngine *engine, const char *member_objtype)
{
	OSyncFormatConverterPath *converter_path = g_hash_table_lookup(engine->converterPathes, member_objtype);
	return converter_path;
}

static void _osync_engine_set_converter_path(OSyncEngine *engine, const char *member_objtype, OSyncFormatConverterPath *converter_path)
{
	osync_trace(TRACE_INTERNAL, "Setting converter_path of %s to %p", member_objtype, converter_path);
	if (!converter_path)
		return;
	g_hash_table_insert(engine->converterPathes, osync_strdup(member_objtype), converter_path);
}

static void _osync_engine_converter_path_unref(gpointer data) {
	OSyncFormatConverterPath * converter_path = data;
	osync_converter_path_unref(converter_path);
}

static void _osync_engine_receive_change(OSyncClientProxy *proxy, void *userdata, OSyncChange *change)
{
	OSyncEngine *engine = userdata;
	OSyncError *error = NULL;
	osync_bool found = FALSE;
	OSyncMember *member = NULL;
	long long int memberid = 0;
	const char *uid = NULL;
	OSyncChangeType changetype = 0;
	const char *format = NULL;
	const char *objtype = NULL;
	OSyncObjTypeSink *objtype_sink = NULL;
	char *member_objtype = NULL;
	OSyncData *data = NULL;
	OSyncObjFormat *internalFormat = NULL;
	OSyncObjFormat *detected_format = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, proxy, userdata, change);

	member = osync_client_proxy_get_member(proxy);
	memberid = osync_member_get_id(member);
	uid = osync_change_get_uid(change);		
	changetype = osync_change_get_changetype(change);
	format = osync_objformat_get_name(osync_change_get_objformat(change));
	objtype = osync_change_get_objtype(change);

	objtype_sink = osync_member_find_objtype_sink(member, objtype);

	osync_trace(TRACE_INTERNAL, "Received change %s, changetype %i, format %s, objtype %s from member %lli", uid, changetype, format, objtype, memberid);

	data = osync_change_get_data(change);

	/* try to detect encapsulated formats */
	if (osync_change_get_changetype(change) != OSYNC_CHANGE_TYPE_DELETED)
		if (!osync_format_env_detect_objformat_full(engine->formatenv, data, &detected_format, &error))
			goto error;

	if (detected_format && detected_format != osync_change_get_objformat(change)) {
		osync_trace(TRACE_INTERNAL, "Detected format (%s) different then the reported format (%s)!",
		osync_objformat_get_name(detected_format),
		osync_objformat_get_name(osync_change_get_objformat(change)));
		objtype = osync_objformat_get_objtype(detected_format);
	}
	
	member_objtype = osync_strdup_printf("%lli_%s", memberid, objtype); 

	/* Convert the format to the internal format */
	internalFormat = _osync_engine_get_internal_format(engine, objtype);
	osync_trace(TRACE_INTERNAL, "common format %p for objtype %s", internalFormat, objtype);

	/* Only convert if the engine is allowed to convert and if an "internal" or detected format is available. 
		 The reason that the engine isn't allowed to convert could be backup. dumping the changes. 
		 Do not convert anything if the chagetype is DELETED. */

	/* TODO: Move common format conversion into objengine. Use this conversion only for
	 * encapsulating formats, if required.
	 */
	if ((internalFormat || detected_format) && osync_group_get_converter_enabled(engine->group) && (osync_change_get_changetype(change) != OSYNC_CHANGE_TYPE_DELETED)) {
		OSyncFormatConverterPath *path = NULL;
		OSyncObjFormatSink *formatsink = NULL;
		OSyncObjFormat *common_format = internalFormat ? internalFormat : detected_format;
		osync_trace(TRACE_INTERNAL, "converting to format %s", osync_objformat_get_name(common_format));

		path = _osync_engine_get_converter_path(engine, member_objtype);
		if(!path) {
			path = osync_format_env_find_path_with_detectors(engine->formatenv, osync_change_get_data(change), common_format, NULL, &error);
			_osync_engine_set_converter_path(engine, member_objtype, path);
		}

		if (!path)
			goto error;
	
		/* Get the current configured format to honor the plugin configuration! */
		formatsink = osync_objtype_sink_find_objformat_sink(objtype_sink, osync_change_get_objformat(change));
		if (formatsink) {
			const char *config = osync_objformat_sink_get_config(formatsink); 
			osync_converter_path_set_config(path, config);
		}

		if (!osync_format_env_convert(engine->formatenv, path, data, &error)) {
			goto error;
		}
	}
	
	/* TODO: Move this into the objengine. */
	/* Merger - Merge lost information to the change (don't merger anything when changetype is DELETED.) */
	if( osync_group_get_merger_enabled(engine->group) &&
			osync_group_get_converter_enabled(engine->group) &&	
			(osync_change_get_changetype(change) != OSYNC_CHANGE_TYPE_DELETED) &&
			/* only use the merger if the objformat has merger registered. */
			osync_objformat_has_merger(osync_change_get_objformat(change)) )

		{
			OSyncCapabilities *caps;
			OSyncObjFormat *objformat = osync_change_get_objformat(change);
			char *entirebuf, *buffer;
			unsigned int entsize, size = 0;
			osync_trace(TRACE_INTERNAL, "Merge.");

			member = osync_client_proxy_get_member(proxy);
			caps = osync_member_get_capabilities(member);

			if(caps) {

				/* TODO: Merger save the archive data with the member so we have to load it only for one time*/
				// osync_archive_load_data() is fetching the mappingid by uid in the db
				int ret = osync_archive_load_data(engine->archive, uid, osync_change_get_objtype(change), &entirebuf, &entsize, &error);
				if (ret < 0) {
					goto error; 
				} 
			
				if (ret > 0) {
					OSyncMarshal *marshal;

					marshal = osync_marshal_new(&error);
					if (!marshal)
						goto error;

					osync_marshal_write_data(marshal, entirebuf, entsize);

					if (!osync_objformat_demarshal(objformat, marshal, &entirebuf, &entsize, &error)) {
						osync_marshal_unref(marshal);
						goto error;
					}

					osync_marshal_unref(marshal);
					
					osync_data_get_data(osync_change_get_data(change), &buffer, &size);

					ret = osync_objformat_merge(objformat, &buffer, &size, entirebuf, entsize, caps, &error);
					osync_free(entirebuf);

					if (ret != TRUE)
						goto error;

					osync_trace(TRACE_SENSITIVE, "Merge result:\n%s\n",
						osync_objformat_print(objformat, buffer, size));
				}
			}
		}
	
	/* Search for the correct objengine */
	{OSyncList * o = NULL;
		for (o = engine->object_engines; o; o = o->next) {
			OSyncObjEngine *objengine = o->data;
			if (!strcmp(osync_change_get_objtype(change), osync_obj_engine_get_objtype(objengine))) {
				found = TRUE;
				if (!osync_obj_engine_receive_change(objengine, proxy, change, &error))
					goto error;
				break;
			}	
		}}
	
	if (!found) {
		osync_error_set(&error, OSYNC_ERROR_GENERIC, "Unable to find engine which can handle objtype %s", osync_change_get_objtype(change));
		goto error;
	}

	osync_free(member_objtype);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;

 error:
	osync_free(member_objtype);
	
	osync_engine_set_error(engine, error);
	osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_ERROR, NULL, error);
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(&error));
}

/* This function is called from the master thread. The function dispatched incoming data from
 * the remote end */
static gboolean _command_dispatch(GSource *source, GSourceFunc callback, gpointer user_data)
{
	OSyncEngine *engine = user_data;
	OSyncEngineCommand *command = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, user_data);
	
	while ((command = g_async_queue_try_pop(engine->command_queue))) {
		/* We check if the message is a reply to something */
		osync_trace(TRACE_INTERNAL, "Dispatching %p: %i", command, command->cmd);
		
		osync_engine_command(engine, command);
		osync_free(command);
	}
	
	osync_trace(TRACE_EXIT, "%s: Done dispatching", __func__);
	return TRUE;
}

osync_bool osync_engine_mapping_solve(OSyncEngine *engine, OSyncMappingEngine *mapping_engine, OSyncChange *change, OSyncError **error)
{
	OSyncEngineCommand *cmd = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, mapping_engine, change, error);
	
	cmd = osync_try_malloc0(sizeof(OSyncEngineCommand), error);
	if (!cmd) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
		return FALSE;
	}
	
	cmd->cmd = OSYNC_ENGINE_COMMAND_SOLVE;
	cmd->mapping_engine = mapping_engine;
	cmd->master = change;
	cmd->solve_type = OSYNC_ENGINE_SOLVE_CHOOSE;
	
	g_async_queue_push(engine->command_queue, cmd);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
}

osync_bool osync_engine_mapping_duplicate(OSyncEngine *engine, OSyncMappingEngine *mapping_engine, OSyncError **error)
{
	OSyncEngineCommand *cmd = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, mapping_engine, error);
	
	cmd = osync_try_malloc0(sizeof(OSyncEngineCommand), error);
	if (!cmd) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
		return FALSE;
	}
	
	cmd->cmd = OSYNC_ENGINE_COMMAND_SOLVE;
	cmd->mapping_engine = mapping_engine;
	cmd->solve_type = OSYNC_ENGINE_SOLVE_DUPLICATE;
	
	g_async_queue_push(engine->command_queue, cmd);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
}

osync_bool osync_engine_mapping_ignore_conflict(OSyncEngine *engine, OSyncMappingEngine *mapping_engine, OSyncError **error)
{
	OSyncEngineCommand *cmd = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, mapping_engine, error);
	
	cmd = osync_try_malloc0(sizeof(OSyncEngineCommand), error);
	if (!cmd) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
		return FALSE;
	}
	
	cmd->cmd = OSYNC_ENGINE_COMMAND_SOLVE;
	cmd->mapping_engine = mapping_engine;
	cmd->solve_type = OSYNC_ENGINE_SOLVE_IGNORE;
	
	g_async_queue_push(engine->command_queue, cmd);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
}

osync_bool osync_engine_mapping_use_latest(OSyncEngine *engine, OSyncMappingEngine *mapping_engine, OSyncError **error)
{
	OSyncEngineCommand *cmd = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, mapping_engine, error);
	
	cmd = osync_try_malloc0(sizeof(OSyncEngineCommand), error);
	if (!cmd) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
		return FALSE;
	}
	
	cmd->cmd = OSYNC_ENGINE_COMMAND_SOLVE;
	cmd->mapping_engine = mapping_engine;
	cmd->solve_type = OSYNC_ENGINE_SOLVE_USE_LATEST;
	
	g_async_queue_push(engine->command_queue, cmd);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
}

OSyncEngine *osync_engine_new(OSyncGroup *group, OSyncError **error)
{
	OSyncEngine *engine = NULL;
	OSyncEngine **engineptr = NULL;
	char *enginesdir = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, group, error);
	g_assert(group);
	
	engine = osync_try_malloc0(sizeof(OSyncEngine), error);
	if (!engine)
		goto error;
	engine->ref_count = 1;

	if (!g_thread_supported ())
		g_thread_init (NULL);
	
	engine->internalFormats = g_hash_table_new_full(g_str_hash, g_str_equal, osync_free, osync_free);
	engine->internalSchemas = g_hash_table_new_full(g_str_hash, g_str_equal, osync_free, NULL);
	engine->converterPathes = g_hash_table_new_full(g_str_hash, g_str_equal, osync_free, _osync_engine_converter_path_unref);
	
	engine->context = g_main_context_new();
	engine->thread = osync_thread_new(engine->context, error);
	if (!engine->thread)
		goto error_free_engine;
	
	engine->group = group;
	osync_group_ref(group);

	engine->command_queue = g_async_queue_new();

	if (!osync_group_get_configdir(group)) {
		osync_trace(TRACE_INTERNAL, "No config dir found. Making stateless sync");
	} else {
		char *filename = osync_strdup_printf("%s%carchive.db", osync_group_get_configdir(group), G_DIR_SEPARATOR);
		engine->archive = osync_archive_new(filename, error);
		osync_free(filename);
		if (!engine->archive)
			goto error_free_engine;
	}
	
	/* Now we attach a queue to the engine which handles our commands */
	engine->command_functions = osync_try_malloc0(sizeof(GSourceFuncs), error);
	if (!engine->command_functions)
		goto error_free_engine;
	engine->command_functions->prepare = _command_prepare;
	engine->command_functions->check = _command_check;
	engine->command_functions->dispatch = _command_dispatch;
	engine->command_functions->finalize = NULL;

	engine->command_source = g_source_new(engine->command_functions, sizeof(GSource) + sizeof(OSyncEngine *));

	/* Overwriting pointer of callback_data of GSource (->command_source) with engine 
		 FIXME: Make it work without such dirty hacks to inject pointers to a private struct */ 
	engineptr = (OSyncEngine **)(engine->command_source + 1); 
	*engineptr = engine;

	g_source_set_callback(engine->command_source, NULL, engine, NULL);
	g_source_attach(engine->command_source, engine->context);
	g_main_context_ref(engine->context);

	enginesdir = osync_strdup_printf("%s%cengines", osync_group_get_configdir(group), G_DIR_SEPARATOR);
	engine->engine_path = osync_strdup_printf("%s%cenginepipe", enginesdir, G_DIR_SEPARATOR);
	
	if (g_mkdir_with_parents(enginesdir, 0755) < 0) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Couldn't create engines directory: %s", g_strerror(errno));
		osync_free(enginesdir);
		goto error_free_engine;
	}
	osync_free(enginesdir);
	
	engine->syncing_mutex = g_mutex_new();
	engine->syncing = g_cond_new();
	
	engine->started_mutex = g_mutex_new();
	engine->started = g_cond_new();
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, engine);
	return engine;

 error_free_engine:
	osync_engine_unref(engine);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

OSyncEngine *osync_engine_ref(OSyncEngine *engine)
{
	osync_assert(engine);
	
	g_atomic_int_inc(&(engine->ref_count));

	return engine;
}

void osync_engine_unref(OSyncEngine *engine)
{
	osync_assert(engine);
		
	if (g_atomic_int_dec_and_test(&(engine->ref_count))) {
		osync_trace(TRACE_ENTRY, "%s(%p)", __func__, engine);

		while (engine->object_engines) {
			OSyncObjEngine *objengine = engine->object_engines->data;
			osync_obj_engine_unref(objengine);
			engine->object_engines = osync_list_remove(engine->object_engines, engine->object_engines->data);
		}

		if (engine->internalFormats)
			g_hash_table_destroy(engine->internalFormats);

		if (engine->converterPathes)
			g_hash_table_destroy(engine->converterPathes);
		
		if (engine->group)
			osync_group_unref(engine->group);
		
		if (engine->engine_path)
			osync_free(engine->engine_path);
			
		if (engine->plugin_dir)
			osync_free(engine->plugin_dir);
			
		if (engine->format_dir)
			osync_free(engine->format_dir);
		
		if (engine->thread)
			osync_thread_unref(engine->thread);
			
		if (engine->context)
			g_main_context_unref(engine->context);
			
		if (engine->syncing)
			g_cond_free(engine->syncing);
			
		if (engine->syncing_mutex)
			g_mutex_free(engine->syncing_mutex);
			
		if (engine->started)
			g_cond_free(engine->started);
			
		if (engine->started_mutex)
			g_mutex_free(engine->started_mutex);
		
		if (engine->command_queue)
			g_async_queue_unref(engine->command_queue);
	
		if (engine->command_source)
			g_source_unref(engine->command_source);
	
		if (engine->command_functions)
			osync_free(engine->command_functions);
	
		if (engine->archive)
			osync_archive_unref(engine->archive);
		
		if (engine->error)
			osync_error_unref(&(engine->error));

		if (engine->internalSchemas)
			g_hash_table_destroy(engine->internalSchemas);

#ifdef OPENSYNC_UNITTESTS
		if (engine->schema_dir)
			osync_free(engine->schema_dir);
#endif /* OPENSYNC_UNITTESTS */
		
		osync_free(engine);
		osync_trace(TRACE_EXIT, "%s", __func__);
	}
}

void osync_engine_set_plugindir(OSyncEngine *engine, const char *dir)
{
	osync_assert(engine);
	if (engine->plugin_dir)
		osync_free(engine->plugin_dir);
	engine->plugin_dir = osync_strdup(dir);
}

OSyncGroup *osync_engine_get_group(OSyncEngine *engine)
{
	osync_assert(engine);
	return engine->group;
}

OSyncArchive *osync_engine_get_archive(OSyncEngine *engine)
{
	osync_assert(engine);
	return engine->archive;
}

void osync_engine_set_formatdir(OSyncEngine *engine, const char *dir)
{
	osync_assert(engine);
	if (engine->format_dir)
		osync_free(engine->format_dir);
	engine->format_dir = osync_strdup(dir);
}

static osync_bool _osync_engine_start(OSyncEngine *engine, OSyncError **error)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, engine, error);
	
	/* For testing purpose, it's possible to preload a instrumented plugin_env */
	if (!engine->pluginenv) {
		engine->pluginenv = osync_plugin_env_new(error);
		if (!engine->pluginenv)
			goto error;
		
		if (!osync_plugin_env_load(engine->pluginenv, engine->plugin_dir, error))
			goto error;
	}
	
	osync_thread_start(engine->thread);

	osync_engine_ref(engine);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
	
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static void _osync_engine_stop(OSyncEngine *engine)
{
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, engine);
	
	if (engine->thread)
		osync_thread_stop(engine->thread);

	osync_engine_unref(engine);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static osync_bool _osync_engine_finalize_member(OSyncEngine *engine, OSyncClientProxy *proxy, OSyncError **error)
{
	unsigned int i = 2000;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, engine, proxy, error);
		
	engine->busy = TRUE;
	
	if (!osync_client_proxy_finalize(proxy, _finalize_callback, engine, error))
		goto error;
	
	//FIXME
	while (engine->busy && i > 0) { g_usleep(1000); g_main_context_iteration(engine->context, FALSE); i--; }
	osync_trace(TRACE_INTERNAL, "Done waiting");
	
	if (!osync_client_proxy_shutdown(proxy, error))
		goto error;
	
	engine->proxies = osync_list_remove(engine->proxies, proxy);
	
	osync_client_proxy_unref(proxy);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
	
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static OSyncClientProxy *_osync_engine_initialize_member(OSyncEngine *engine, OSyncMember *member, OSyncError **error)
{
	OSyncPluginConfig *config = NULL;
	OSyncPlugin *plugin = NULL;
	OSyncClientProxy *proxy = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, engine, member, error);

	plugin = osync_plugin_env_find_plugin(engine->pluginenv, osync_member_get_pluginname(member));
	if (!plugin) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to find plugin %s", osync_member_get_pluginname(member));
		goto error;
	}
		
	/* If we don't have a config we have to ask the plugin if it needs a config */
	if (!osync_member_has_config(member)) {

		switch (osync_plugin_get_config_type(plugin)) {
		case OSYNC_PLUGIN_NO_CONFIGURATION:
			break;
		case OSYNC_PLUGIN_OPTIONAL_CONFIGURATION:
			config = osync_member_get_config_or_default(member, error);
			if (!config)
				goto error;
			break;
		case OSYNC_PLUGIN_NEEDS_CONFIGURATION:
			config = osync_member_get_config(member, error);
			if (!config)
				goto error;
			break;
		}
	} else {
		config = osync_member_get_config(member, error);
		if (!config)
			goto error;
	}
	
	proxy = osync_client_proxy_new(engine->formatenv, member, error);
	if (!proxy)
		goto error;
		
	osync_client_proxy_set_context(proxy, engine->context);
	osync_client_proxy_set_change_callback(proxy, _osync_engine_receive_change, engine);

	if (!osync_client_proxy_spawn(proxy, osync_plugin_get_start_type(plugin), osync_member_get_configdir(member), error))
		goto error_free_proxy;
	
	engine->busy = TRUE;
	
	if (!osync_client_proxy_initialize(proxy, _finalize_callback, engine, engine->format_dir, engine->plugin_dir, osync_member_get_pluginname(member), osync_group_get_name(engine->group), osync_member_get_configdir(member), config, error))
		goto error_shutdown;
	
	//FIXME
	while (engine->busy) { g_usleep(100); }
	
	engine->proxies = osync_list_append(engine->proxies, proxy);
	
	if (engine->error) {
		_osync_engine_finalize_member(engine, proxy, NULL);
		osync_error_set_from_error(error, &(engine->error));
		osync_error_unref(&(engine->error));
		engine->error = NULL;
		osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
		return NULL;
	}
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return proxy;
	
 error_shutdown:
	osync_client_proxy_shutdown(proxy, NULL);
 error_free_proxy:
	osync_client_proxy_unref(proxy);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

static osync_bool _osync_engine_generate_connected_event(OSyncEngine *engine)
{
	OSyncError *locerror = NULL;

	if (osync_bitcount(engine->proxy_errors | engine->proxy_connects) != osync_list_length(engine->proxies))
		return FALSE;
	
	if (osync_bitcount(engine->obj_errors | engine->obj_connects) == osync_list_length(engine->object_engines)) {
		if (osync_bitcount(engine->obj_errors) == osync_list_length(engine->object_engines)) {
			osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "No objtypes left without error. Aborting");
			osync_trace(TRACE_ERROR, "%s", osync_error_print(&locerror));
			osync_engine_set_error(engine, locerror);
			osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_ERROR, locerror);
			osync_engine_event(engine, OSYNC_ENGINE_EVENT_ERROR);
			osync_error_unref(&locerror);
		} else if (osync_bitcount(engine->proxy_errors) || osync_bitcount(engine->obj_errors)) {
			osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "At least one object engine failed while connecting. Aborting");
			osync_trace(TRACE_ERROR, "%s", osync_error_print(&locerror));
			osync_engine_set_error(engine, locerror);
			osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_ERROR, locerror);
			osync_engine_event(engine, OSYNC_ENGINE_EVENT_ERROR);
			osync_error_unref(&locerror);
		} else {
			osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_CONNECTED, NULL);
			osync_engine_event(engine, OSYNC_ENGINE_EVENT_CONNECTED);
		}

		return TRUE;
	}
	
	return FALSE;
}

static void _osync_engine_generate_connect_done_event(OSyncEngine *engine)
{
	if (osync_bitcount(engine->proxy_errors | engine->proxy_connect_done) != osync_list_length(engine->proxies))
		return;
	
	if (osync_bitcount(engine->obj_errors | engine->obj_connect_done) == osync_list_length(engine->object_engines)) {
		if (osync_bitcount(engine->obj_errors)) {
			OSyncError *locerror = NULL;
			osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "At least one object engine failed within connect_done. Aborting");
			osync_engine_set_error(engine, locerror);
			osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_ERROR, locerror);
			osync_engine_event(engine, OSYNC_ENGINE_EVENT_ERROR);
		} else {
			osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_CONNECT_DONE, NULL);
			osync_engine_event(engine, OSYNC_ENGINE_EVENT_CONNECT_DONE);
		}
	} else
		osync_trace(TRACE_INTERNAL, "Not yet: %i", osync_bitcount(engine->obj_errors | engine->obj_connect_done));
}

osync_bool osync_engine_check_get_changes(OSyncEngine *engine)
{
	if (osync_bitcount(engine->proxy_errors | engine->proxy_get_changes) != osync_list_length(engine->proxies)) {
		osync_trace(TRACE_INTERNAL, "Not yet. main sinks still need to read: %i", osync_bitcount(engine->proxy_errors | engine->proxy_get_changes), osync_list_length(engine->proxies));
		return FALSE;
	}
	
	if (osync_bitcount(engine->obj_errors | engine->obj_get_changes) == osync_list_length(engine->object_engines))
		return TRUE;
		
	osync_trace(TRACE_INTERNAL, "Not yet. Obj Engines still need to read: %i", osync_bitcount(engine->obj_errors | engine->obj_get_changes));
	return FALSE;
}

static void _osync_engine_generate_get_changes_event(OSyncEngine *engine)
{
	if (!osync_engine_check_get_changes(engine))
		return;
		
	if (osync_bitcount(engine->obj_errors)) {
		OSyncError *locerror = NULL;
		osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "At least one object engine failed while getting changes. Aborting");
		osync_trace(TRACE_ERROR, "%s", osync_error_print(&locerror));
		osync_engine_set_error(engine, locerror);
		osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_ERROR, locerror);
		osync_engine_event(engine, OSYNC_ENGINE_EVENT_ERROR);
	} else {
		osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_READ, NULL);
		
		osync_engine_event(engine, OSYNC_ENGINE_EVENT_READ);
	}
}

static void _osync_engine_generate_prepared_map(OSyncEngine *engine)
{

	if (osync_bitcount(engine->obj_errors | engine->obj_prepared_map) == osync_list_length(engine->object_engines)) {
		if (osync_bitcount(engine->obj_errors)) {
			OSyncError *locerror = NULL;
			osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "At least one object engine failed while preparing for mapping the changes. Aborting");
			osync_trace(TRACE_ERROR, "%s", osync_error_print(&locerror));
			osync_engine_set_error(engine, locerror);
			osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_ERROR, locerror);
			osync_engine_event(engine, OSYNC_ENGINE_EVENT_ERROR);
			osync_error_unref(&locerror);
		} else {
			osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_PREPARED_MAP, NULL);
			
			osync_engine_event(engine, OSYNC_ENGINE_EVENT_PREPARED_MAP);
		}
	} else
		osync_trace(TRACE_INTERNAL, "Not yet: %i", osync_bitcount(engine->obj_errors | engine->obj_prepared_map));

}

static void _osync_engine_generate_mapped_event(OSyncEngine *engine)
{

	if (osync_bitcount(engine->obj_errors | engine->obj_mapped) == osync_list_length(engine->object_engines)) {
		if (osync_bitcount(engine->obj_errors)) {
			OSyncError *locerror = NULL;
			osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "At least one object engine failed while mapping changes. Aborting");
			osync_trace(TRACE_ERROR, "%s", osync_error_print(&locerror));
			osync_engine_set_error(engine, locerror);
			osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_ERROR, locerror);
			osync_engine_event(engine, OSYNC_ENGINE_EVENT_ERROR);
			osync_error_unref(&locerror);
		} else {
			osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_MAPPED, NULL);
			
			osync_engine_event(engine, OSYNC_ENGINE_EVENT_MAPPED);
			/* TODO: error handling! */
			osync_engine_queue_command(engine, OSYNC_ENGINE_COMMAND_END_CONFLICTS, NULL);
		}
	} else
		osync_trace(TRACE_INTERNAL, "Not yet: %i", osync_bitcount(engine->obj_errors | engine->obj_mapped));

}

osync_bool osync_engine_handle_mixed_objtypes(OSyncEngine *engine, OSyncError **error)
{
	OSyncList *o, *s, *e;

	for (o = engine->object_engines; o; o = o->next) {
		OSyncObjEngine *objengine = o->data;
		const char *objtype = objengine->objtype; 

		osync_trace(TRACE_INTERNAL, "ObjEngine: %s", objtype);

		for (s = objengine->dummy_sink_engines; s; s = s->next) {
			OSyncSinkEngine *sinkengine = s->data;

			for (e = sinkengine->entries; e; e = e->next) {
				OSyncMappingEntryEngine *mapping_entry_engine = e->data;
				OSyncChange *change;
				OSyncObjFormat *objformat;
				const char *current_objtype;
				OSyncObjEngine *new_objengine;
				OSyncSinkEngine *new_sinkengine;

				change = osync_entry_engine_get_change(mapping_entry_engine);
				objformat = osync_change_get_objformat(change);
				osync_assert(objformat);
				current_objtype = osync_objformat_get_objtype(objformat);

				/* ... otherwise we need to reassign the mappin entry engine
				 * to different objengine
				 */
				new_objengine = osync_engine_find_objengine(engine, current_objtype);
				if (!new_objengine) {
					osync_error_set(error, OSYNC_ERROR_GENERIC,
							"Couldn't find Object Type Engine for Object Type \"%s\" "
							"while preparing for write phase.", __NULLSTR(current_objtype));
					goto error;
				}

				new_sinkengine = osync_obj_engine_find_proxy_sinkengine(new_objengine, sinkengine->proxy);
				if (!new_sinkengine) {
					osync_error_set(error, OSYNC_ERROR_GENERIC,
							"Couldn't find Sink Engine for Object Type \"%s\" "
							"while preparing for write phase.", __NULLSTR(current_objtype));
					goto error;
				}

				sinkengine->entries = osync_list_remove(sinkengine->entries, mapping_entry_engine);
				new_sinkengine->entries = osync_list_append(new_sinkengine->entries, mapping_entry_engine);
				
			}
		}
	}

	return TRUE;

error:
	return FALSE;
}

static void _osync_engine_generate_end_conflicts_event(OSyncEngine *engine)
{

	if (osync_bitcount(engine->obj_errors | engine->obj_solved) == osync_list_length(engine->object_engines)) {
		if (osync_bitcount(engine->obj_errors)) {
			OSyncError *locerror = NULL;
			osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "At least one object engine failed while solving conflicts. Aborting");
			osync_trace(TRACE_ERROR, "%s", osync_error_print(&locerror));
			osync_engine_set_error(engine, locerror);
			osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_ERROR, locerror);
			osync_engine_event(engine, OSYNC_ENGINE_EVENT_ERROR);
			osync_error_unref(&locerror);
		} else {
			osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_END_CONFLICTS, NULL);
			
			osync_engine_event(engine, OSYNC_ENGINE_EVENT_END_CONFLICTS);
			/* TODO: error handling! */
			osync_engine_queue_command(engine, OSYNC_ENGINE_COMMAND_MULTIPLY, NULL);
		}
	} else
		osync_trace(TRACE_INTERNAL, "Not yet: %i", osync_bitcount(engine->obj_errors | engine->obj_solved));

}

void osync_engine_trace_multiply_summary(OSyncEngine *engine)
{
	OSyncList *o, *s;
	OSyncList *e;
	unsigned int added, modified, deleted, unmodified, unknown;
	long long int memberid;

	if (!osync_trace_is_enabled())
		return;

	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, engine);

	for (o = engine->object_engines; o; o = o->next) {
		OSyncObjEngine *objengine = o->data;
		const char *objtype = objengine->objtype; 

		osync_trace(TRACE_INTERNAL, "ObjEngine: %s", objtype);

		for (s = objengine->sink_engines; s; s = s->next) {
			OSyncSinkEngine *sinkengine = s->data;

			added = modified = deleted = unmodified = unknown = 0;
			memberid = osync_member_get_id(osync_client_proxy_get_member(sinkengine->proxy));

			for (e = sinkengine->entries; e; e = e->next) {
				OSyncMappingEntryEngine *mapping_entry_engine = e->data;

				if (!mapping_entry_engine->dirty)
					continue;

				switch (osync_change_get_changetype(mapping_entry_engine->change)) {
					case OSYNC_CHANGE_TYPE_ADDED:
						added++;
						break;
					case OSYNC_CHANGE_TYPE_MODIFIED:
						modified++;
						break;
					case OSYNC_CHANGE_TYPE_DELETED:
						deleted++;
						break;
					case OSYNC_CHANGE_TYPE_UNMODIFIED:
						unmodified++;
						break;
					case OSYNC_CHANGE_TYPE_UNKNOWN:
						unknown++;
						break;
				}
			}

			osync_trace(TRACE_INTERNAL, "\tMember: %lli "
					"added:%u modified:%u deleted:%u "
					"(unmodified:%u unknown:%u)",
					memberid, added, modified, deleted, unmodified, unknown);
		}

		
	}

	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void _osync_engine_generate_multiplied_event(OSyncEngine *engine)
{

	if (osync_bitcount(engine->obj_errors | engine->obj_multiplied) == osync_list_length(engine->object_engines)) {
		if (osync_bitcount(engine->obj_errors)) {
			OSyncError *locerror = NULL;
			osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "At least one object engine failed while multiplying changes. Aborting");
			osync_trace(TRACE_ERROR, "%s", osync_error_print(&locerror));
			osync_engine_set_error(engine, locerror);
			osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_ERROR, locerror);
			osync_engine_event(engine, OSYNC_ENGINE_EVENT_ERROR);
		} else {
			osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_MULTIPLIED, NULL);
			osync_engine_event(engine, OSYNC_ENGINE_EVENT_MULTIPLIED);
		}
	} else
		osync_trace(TRACE_INTERNAL, "Not yet: %i", osync_bitcount(engine->obj_errors | engine->obj_multiplied));

}

static void _osync_engine_generate_prepared_write_event(OSyncEngine *engine)
{
	OSyncError *locerror = NULL;

	if (osync_bitcount(engine->obj_errors | engine->obj_prepared_write) == osync_list_length(engine->object_engines)) {
		if (osync_bitcount(engine->obj_errors)) {
			osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "At least one object engine failed while preparing the write event. Aborting");
			goto error;
		} else {
			osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_PREPARED_WRITE, NULL);
			if (!osync_engine_handle_mixed_objtypes(engine, &locerror))
				goto error;

			/* This is only for debugging purposes */
			osync_engine_trace_multiply_summary(engine);

			if (engine->multiply_callback)
				engine->multiply_callback(engine, engine->multiply_userdata);
			else
				osync_engine_event(engine, OSYNC_ENGINE_EVENT_PREPARED_WRITE);
		}
	} else
		osync_trace(TRACE_INTERNAL, "Not yet: %i", osync_bitcount(engine->obj_errors | engine->obj_prepared_write));

	return;

error:
	osync_trace(TRACE_ERROR, "%s", osync_error_print(&locerror));
	osync_engine_set_error(engine, locerror);
	osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_ERROR, locerror);
	osync_engine_event(engine, OSYNC_ENGINE_EVENT_ERROR);
	//osync_error_unref(&locerror);
}

static void _osync_engine_generate_written_event(OSyncEngine *engine)
{
	if (osync_bitcount(engine->proxy_errors | engine->proxy_written) != osync_list_length(engine->proxies))
		return;
	
	if (osync_bitcount(engine->obj_errors | engine->obj_written) == osync_list_length(engine->object_engines)) {
		if (osync_bitcount(engine->obj_errors)) {
			OSyncError *locerror = NULL;
			osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "At least one object engine failed while writting changes. Aborting");
			osync_trace(TRACE_ERROR, "%s", osync_error_print(&locerror));
			osync_engine_set_error(engine, locerror);
			osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_ERROR, locerror);
			osync_engine_event(engine, OSYNC_ENGINE_EVENT_ERROR);
			//osync_error_unref(&locerror);
		} else {
			osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_WRITTEN, NULL);
			osync_engine_event(engine, OSYNC_ENGINE_EVENT_WRITTEN);
		}
	} else
		osync_trace(TRACE_INTERNAL, "Not yet: %i", osync_bitcount(engine->obj_errors | engine->obj_written));

}

static void _osync_engine_generate_sync_done_event(OSyncEngine *engine)
{
	if (osync_bitcount(engine->proxy_errors | engine->proxy_sync_done) != osync_list_length(engine->proxies))
		return;
	
	if (osync_bitcount(engine->obj_errors | engine->obj_sync_done) == osync_list_length(engine->object_engines)) {
		if (osync_bitcount(engine->obj_errors)) {
			OSyncError *locerror = NULL;
			osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "At least one object engine failed within sync_done. Aborting");
			osync_engine_set_error(engine, locerror);
			osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_ERROR, locerror);
			osync_engine_event(engine, OSYNC_ENGINE_EVENT_ERROR);
			//osync_error_unref(&locerror);
		} else {
			osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_SYNC_DONE, NULL);
			osync_engine_event(engine, OSYNC_ENGINE_EVENT_SYNC_DONE);
		}
	} else
		osync_trace(TRACE_INTERNAL, "Not yet: %i", osync_bitcount(engine->obj_errors | engine->obj_sync_done));
}

static osync_bool _osync_engine_generate_disconnected_event(OSyncEngine *engine)
{
	if (osync_bitcount(engine->proxy_errors | engine->proxy_disconnects) != osync_list_length(engine->proxies))
		return FALSE;
	
	if (osync_bitcount(engine->obj_errors | engine->obj_disconnects) == osync_list_length(engine->object_engines)) {

		/* Error handling in this case is quite special. We have to call OSYNC_ENGINE_EVENT_DISCONNECTED,
			 even on errors. Since OSYNC_ENGINE_EVENT_ERROR would emit this DISCONNECTED event again - deadlock! */
		osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_DISCONNECTED, NULL);
		osync_engine_event(engine, OSYNC_ENGINE_EVENT_DISCONNECTED);
		return TRUE;
	}
	
	osync_trace(TRACE_INTERNAL, "Not yet: %i", osync_bitcount(engine->obj_errors | engine->obj_disconnects));
	return FALSE;
}

static void _osync_engine_connect_callback(OSyncClientProxy *proxy, void *userdata, osync_bool slowsync, OSyncError *error)
{
	OSyncList *o = NULL;
	OSyncEngine *engine = NULL;
	int position = 0;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %i, %p)", __func__, proxy, userdata, slowsync, error);

	engine = userdata;
	position = _osync_engine_get_proxy_position(engine, proxy);
	
	if (error) {
		osync_engine_set_error(engine, error);
		engine->proxy_errors = engine->proxy_errors | (0x1 << position);
		osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_ERROR, NULL, error);
	} else {
		engine->proxy_connects = engine->proxy_connects | (0x1 << position);
		osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_CONNECTED, NULL, NULL);
	}

	/* If MainSink request a SlowSync, flag all objengines with SlowSync */
	if (slowsync) {
		for (o = engine->object_engines; o; o = o->next) {
			OSyncObjEngine *objengine = o->data;
			osync_obj_engine_set_slowsync(objengine, TRUE);
		}
	}
	
	_osync_engine_generate_connected_event(engine);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void _osync_engine_connect_done_callback(OSyncClientProxy *proxy, void *userdata, OSyncError *error)
{
	OSyncEngine *engine = userdata;
	int position = 0;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, proxy, userdata, error);
	
	position = _osync_engine_get_proxy_position(engine, proxy);
	
	if (error) {
		osync_engine_set_error(engine, error);
		engine->proxy_errors = engine->proxy_errors | (0x1 << position);
		osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_ERROR, NULL, error);
	} else {
		engine->proxy_connect_done = engine->proxy_connect_done | (0x1 << position);
		osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_CONNECT_DONE, NULL, NULL);
	}
	
	_osync_engine_generate_connect_done_event(engine);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void _osync_engine_disconnect_callback(OSyncClientProxy *proxy, void *userdata, OSyncError *error)
{
	OSyncEngine *engine = userdata;
	int position = 0;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, proxy, userdata, error);
	
	position = _osync_engine_get_proxy_position(engine, proxy);
	
	if (error) {
		osync_engine_set_error(engine, error);
		engine->proxy_errors = engine->proxy_errors | (0x1 << position);
		osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_ERROR, NULL, error);
	} else {
		engine->proxy_disconnects = engine->proxy_disconnects | (0x1 << position);
		osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_DISCONNECTED, NULL, NULL);
	}
	
	_osync_engine_generate_disconnected_event(engine);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void _osync_engine_get_changes_callback(OSyncClientProxy *proxy, void *userdata, OSyncError *error)
{
	OSyncEngine *engine = userdata;
	int position = 0;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, proxy, userdata, error);
	
	position = _osync_engine_get_proxy_position(engine, proxy);
	
	if (error) {
		osync_engine_set_error(engine, error);
		engine->proxy_errors = engine->proxy_errors | (0x1 << position);
		osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_ERROR, NULL, error);
	} else {
		engine->proxy_get_changes = engine->proxy_get_changes | (0x1 << position);
		osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_READ, NULL, NULL);
	}
	
	_osync_engine_generate_get_changes_event(engine);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void _osync_engine_written_callback(OSyncClientProxy *proxy, void *userdata, OSyncError *error)
{
	OSyncEngine *engine = userdata;
	int position = 0;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, proxy, userdata, error);
	
	position = _osync_engine_get_proxy_position(engine, proxy);
	
	if (error) {
		osync_engine_set_error(engine, error);
		engine->proxy_errors = engine->proxy_errors | (0x1 << position);
		osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_ERROR, NULL, error);
	} else {
		engine->proxy_written = engine->proxy_written | (0x1 << position);
		osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_WRITTEN, NULL, NULL);
	}
	
	_osync_engine_generate_written_event(engine);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void _osync_engine_sync_done_callback(OSyncClientProxy *proxy, void *userdata, OSyncError *error)
{
	OSyncEngine *engine = userdata;
	int position = 0;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, proxy, userdata, error);
	
	position = _osync_engine_get_proxy_position(engine, proxy);
	
	if (error) {
		osync_engine_set_error(engine, error);
		engine->proxy_errors = engine->proxy_errors | (0x1 << position);
		osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_ERROR, NULL, error);
	} else {
		engine->proxy_sync_done = engine->proxy_sync_done | (0x1 << position);
		osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_SYNC_DONE, NULL, NULL);
	}
	
	_osync_engine_generate_sync_done_event(engine);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void _osync_engine_get_objengine_error(OSyncEngine *engine, OSyncObjEngine *objengine, int position, OSyncError *error)
{
	engine->obj_errors = engine->obj_errors | (0x1 << position);
	osync_engine_set_error(engine, error);
}

static void _osync_engine_get_objengine_event(OSyncEngine *engine, OSyncObjEngine *objengine, int position, OSyncEngineEvent event)
{
	switch (event) {
	case OSYNC_ENGINE_EVENT_CONNECTED:
		engine->obj_connects = engine->obj_connects | (0x1 << position);
		break;
	case OSYNC_ENGINE_EVENT_CONNECT_DONE:
		engine->obj_connect_done = engine->obj_connect_done | (0x1 << position);
		break;
	case OSYNC_ENGINE_EVENT_ERROR:
		/* ObjEngine don't emit this signal. To determine which actual event fail,
			 the ObjEngine emits regular event type and pass an OSyncError object.
			 See _osync_engine_generate_event() and _osync_engine_get_obj_engine_error(). */
		break;
	case OSYNC_ENGINE_EVENT_READ:
		engine->obj_get_changes = engine->obj_get_changes | (0x1 << position);
		break;
	case OSYNC_ENGINE_EVENT_PREPARED_MAP:
		engine->obj_prepared_map = engine->obj_prepared_map | (0x1 << position);
		break;
	case OSYNC_ENGINE_EVENT_MAPPED:
		engine->obj_mapped = engine->obj_mapped | (0x1 << position);
		break;
	case OSYNC_ENGINE_EVENT_END_CONFLICTS:
		engine->obj_solved = engine->obj_solved | (0x1 << position);
		break;
	case OSYNC_ENGINE_EVENT_MULTIPLIED:
		engine->obj_multiplied = engine->obj_multiplied | (0x1 << position);
		break;
	case OSYNC_ENGINE_EVENT_PREPARED_WRITE:
		engine->obj_prepared_write = engine->obj_prepared_write | (0x1 << position);
		break;
	case OSYNC_ENGINE_EVENT_WRITTEN:
		engine->obj_written = engine->obj_written | (0x1 << position);
		break;
	case OSYNC_ENGINE_EVENT_SYNC_DONE:
		engine->obj_sync_done = engine->obj_sync_done | (0x1 << position);
		break;
	case OSYNC_ENGINE_EVENT_DISCONNECTED:
		engine->obj_disconnects = engine->obj_disconnects | (0x1 << position);
		break;
	case OSYNC_ENGINE_EVENT_SUCCESSFUL:
	case OSYNC_ENGINE_EVENT_PREV_UNCLEAN:
		break;
	}
}

static void _osync_engine_generate_event(OSyncEngine *engine, OSyncEngineEvent event)
{
	engine->lastevent = event;

	switch (event) {
	case OSYNC_ENGINE_EVENT_CONNECTED:
		_osync_engine_generate_connected_event(engine);
		break;
	case OSYNC_ENGINE_EVENT_CONNECT_DONE:
		_osync_engine_generate_connect_done_event(engine);
		break;
	case OSYNC_ENGINE_EVENT_READ:
		_osync_engine_generate_get_changes_event(engine);
		break;
	case OSYNC_ENGINE_EVENT_PREPARED_MAP:
		_osync_engine_generate_prepared_map(engine);
		break;
	case OSYNC_ENGINE_EVENT_MAPPED:
		_osync_engine_generate_mapped_event(engine);
		break;
	case OSYNC_ENGINE_EVENT_END_CONFLICTS:
		_osync_engine_generate_end_conflicts_event(engine);
		break;
	case OSYNC_ENGINE_EVENT_MULTIPLIED:
		_osync_engine_generate_multiplied_event(engine);
		break;
	case OSYNC_ENGINE_EVENT_PREPARED_WRITE:
		_osync_engine_generate_prepared_write_event(engine);
		break;
	case OSYNC_ENGINE_EVENT_WRITTEN:
		_osync_engine_generate_written_event(engine);
		break;
	case OSYNC_ENGINE_EVENT_SYNC_DONE:
		_osync_engine_generate_sync_done_event(engine);
		break;
	case OSYNC_ENGINE_EVENT_DISCONNECTED:
		_osync_engine_generate_disconnected_event(engine);
		break;
	case OSYNC_ENGINE_EVENT_ERROR:
	case OSYNC_ENGINE_EVENT_SUCCESSFUL:
	case OSYNC_ENGINE_EVENT_PREV_UNCLEAN:
		break;
	}
}

static void _osync_engine_event_callback(OSyncObjEngine *objengine, OSyncEngineEvent event, OSyncError *error, void *userdata)
{
	OSyncEngine *engine = userdata;
	int position = 0;
	osync_trace(TRACE_ENTRY, "%s(%p, %s, %p, %p)", __func__, objengine, osync_engine_get_eventstr(event), error, userdata);

	position = _osync_engine_get_objengine_position(engine, objengine);
	
	if (error)
		_osync_engine_get_objengine_error(engine, objengine, position, error);
	else
		_osync_engine_get_objengine_event(engine, objengine, position, event);

	_osync_engine_generate_event(engine, event);

	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void _osync_engine_discover_callback(OSyncClientProxy *proxy, void *userdata, OSyncError *error)
{
	OSyncEngine *engine = userdata;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, proxy, userdata, error);
	
	if (error) {
		osync_engine_set_error(engine, error);
		osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_ERROR, NULL, error);
	} else {
		osync_status_update_member(engine, osync_client_proxy_get_member(proxy), OSYNC_CLIENT_EVENT_DISCOVERED, NULL, NULL);
	}
	
	g_mutex_lock(engine->syncing_mutex);
	g_cond_signal(engine->syncing);
	g_mutex_unlock(engine->syncing_mutex);
			
	osync_trace(TRACE_EXIT, "%s", __func__);
}


osync_bool osync_engine_initialize_formats(OSyncEngine *engine, OSyncError **error)
{
	engine->formatenv = osync_format_env_new(error);
	if (!engine->formatenv)
		goto error;
	
	if (!osync_format_env_load_plugins(engine->formatenv, engine->format_dir, error))
		goto error_free;
	
	/* XXX The internal formats XXX */
	_osync_engine_set_internal_format(engine, "contact", osync_format_env_find_objformat(engine->formatenv, "xmlformat-contact"));
	_osync_engine_set_internal_format(engine, "event", osync_format_env_find_objformat(engine->formatenv, "xmlformat-event"));
	_osync_engine_set_internal_format(engine, "todo", osync_format_env_find_objformat(engine->formatenv, "xmlformat-todo"));
	_osync_engine_set_internal_format(engine, "note", osync_format_env_find_objformat(engine->formatenv, "xmlformat-note"));
	
	return TRUE;

 error_free:
	osync_format_env_unref(engine->formatenv);
	engine->formatenv = NULL;

 error:
	return FALSE;
}


osync_bool osync_engine_initialize(OSyncEngine *engine, OSyncError **error)
{
	osync_bool prev_sync_unclean = FALSE, first_sync = FALSE;
	OSyncGroup *group = NULL;
	int i = 0, num = 0;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, engine, error);

	if (engine->state != OSYNC_ENGINE_STATE_UNINITIALIZED) {
		osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "This engine was not uninitialized: %i", engine->state);
		goto error_no_state_reset;
	}

	engine->state = OSYNC_ENGINE_STATE_START_INIT;
	group = engine->group;

	if (osync_group_num_members(group) < 2) {
		//Not enough members!
		osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "You only configured %i members, but at least 2 are needed", osync_group_num_members(group));
		goto error;
	}
	
	if (osync_group_num_objtypes(engine->group) == 0) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "No synchronizable objtype");
		goto error;
	}

	/* Check if this is the first Synchronization.
	 * On the first synchronization a Slow-Sync is required for all (ObjType)Sinks!
	 */
	if (osync_group_get_last_synchronization(engine->group) == 0) {
		osync_trace(TRACE_INTERNAL, "Last Sync timestamp is 0. First Sync!");
		first_sync = TRUE;
	}
	
	switch (osync_group_lock(group)) {
	case OSYNC_LOCKED:
		osync_error_set(error, OSYNC_ERROR_LOCKED, "Group is locked");
		goto error;
	case OSYNC_LOCK_STALE:
		osync_trace(TRACE_INTERNAL, "Detected stale lock file. Slow-syncing");
		osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_PREV_UNCLEAN, NULL);
		prev_sync_unclean = TRUE;
		break;
	case OSYNC_LOCK_OK:
		break;
	}

	if (!osync_engine_initialize_formats(engine, error))
		goto error;
	
	osync_trace(TRACE_INTERNAL, "Running the main loop");
	if (!_osync_engine_start(engine, error))
		goto error_finalize;
		
	osync_trace(TRACE_INTERNAL, "Spawning clients");
	for (i = 0; i < osync_group_num_members(group); i++) {
		OSyncMember *member = osync_group_nth_member(group, i);
		if (!_osync_engine_initialize_member(engine, member, error))
			goto error_finalize;
	}
	
	/* Lets see which objtypes are synchronizable in this group */
	num = osync_group_num_objtypes(engine->group);
	if (num == 0) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "No synchronizable objtype");
		goto error;
	}

	for (i = 0; i < num; i++) {
		const char *objtype = osync_group_nth_objtype(engine->group, i);
		OSyncObjEngine *objengine = NULL;

		/* Respect if the object type is disabled */
		if (!osync_group_objtype_enabled(engine->group, objtype))
			continue;

		objengine = osync_obj_engine_new(engine, objtype, engine->formatenv, error);
		if (!objengine)
			goto error;

		osync_obj_engine_set_callback(objengine, _osync_engine_event_callback, engine);
		engine->object_engines = osync_list_append(engine->object_engines, objengine);

		/* If previous sync was unclean, then trigger SlowSync for all ObjEngines.
		 * Also trigger SlowSync if this is the first synchronization. */
		if (prev_sync_unclean || first_sync)
			osync_obj_engine_set_slowsync(objengine, TRUE);
	}

	engine->state = OSYNC_ENGINE_STATE_INITIALIZED;

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
	
 error_finalize:
	osync_engine_finalize(engine, NULL);
	osync_group_unlock(engine->group);
 error:
	engine->state = OSYNC_ENGINE_STATE_UNINITIALIZED;
 error_no_state_reset:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_engine_finalize(OSyncEngine *engine, OSyncError **error)
{
	OSyncClientProxy *proxy = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, engine, error);

	if (engine->state != OSYNC_ENGINE_STATE_START_INIT && engine->state != OSYNC_ENGINE_STATE_INITIALIZED) {
		osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "This engine was not in state initialized: %i", engine->state);
		goto error;
	}
	
	engine->state = OSYNC_ENGINE_STATE_UNINITIALIZED;

	while (engine->object_engines) {
		OSyncObjEngine *objengine = engine->object_engines->data;
		osync_obj_engine_unref(objengine);
		engine->object_engines = osync_list_remove(engine->object_engines, engine->object_engines->data);
	}
	
	while (engine->proxies) {
		proxy = engine->proxies->data;
		if (!_osync_engine_finalize_member(engine, proxy, error))
			goto error;
	}
	
	_osync_engine_stop(engine);
	
	if (engine->formatenv) {
		osync_format_env_unref(engine->formatenv);
		engine->formatenv = NULL;
	}
	
	if (engine->pluginenv) {
		osync_plugin_env_unref(engine->pluginenv);
		engine->pluginenv = NULL;
	}
	
	/* Store group modificiations (i.e. last_sync timestamp) */
	if (!osync_group_save(engine->group, error))
		goto error;

	if (!engine->error)
		osync_group_unlock(engine->group);

	osync_error_unref(&(engine->error));
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
	
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

void osync_engine_command(OSyncEngine *engine, OSyncEngineCommand *command)
{
	OSyncList *o = NULL;
	OSyncList *p = NULL;
	OSyncError *locerror = NULL;
	OSyncClientProxy *proxy = NULL;

	osync_assert(engine);
	osync_assert(command);
			
	osync_trace(TRACE_ENTRY, "%s(%p, %p:%s)", __func__, engine, command, osync_engine_get_cmdstr(command->cmd));
	
	switch (command->cmd) {
	case OSYNC_ENGINE_COMMAND_CONNECT:

		/* We first tell all object engines to connect */
		for (o = engine->object_engines; o; o = o->next) {
			OSyncObjEngine *objengine = o->data;

			if (!osync_obj_engine_initialize(objengine, &locerror))
				goto error;

			if (!osync_obj_engine_command(objengine, OSYNC_ENGINE_COMMAND_CONNECT, &locerror))
				goto error;
		}
			
		/* Then we connect the main sinks */
		for (o = engine->proxies; o; o = o->next) {
			OSyncClientProxy *proxy = o->data;
			if (!osync_client_proxy_connect(proxy, _osync_engine_connect_callback, engine, NULL, FALSE, &locerror))
				goto error;
		}
		break;
	case OSYNC_ENGINE_COMMAND_CONNECT_DONE:
	case OSYNC_ENGINE_COMMAND_READ:
	case OSYNC_ENGINE_COMMAND_PREPARE_MAP:
	case OSYNC_ENGINE_COMMAND_MAP:
		break;
	case OSYNC_ENGINE_COMMAND_END_CONFLICTS:
		for (o = engine->object_engines; o; o = o->next) {
			OSyncObjEngine *objengine = o->data;

			if (!osync_obj_engine_command(objengine, OSYNC_ENGINE_COMMAND_END_CONFLICTS, &locerror))
				goto error;
		}
		break;
	case OSYNC_ENGINE_COMMAND_MULTIPLY:
		/* Now that we have mapped everything, we multiply the changes */
		for (o = engine->object_engines; o; o = o->next) {
			OSyncObjEngine *objengine = o->data;
			if (!osync_obj_engine_command(objengine, OSYNC_ENGINE_COMMAND_MULTIPLY, &locerror))
				goto error;
		}
		break;
	case OSYNC_ENGINE_COMMAND_PREPARE_WRITE:
		/* Now that we have multiplied the change, we prepare the write event. */
		for (o = engine->object_engines; o; o = o->next) {
			OSyncObjEngine *objengine = o->data;
			if (!osync_obj_engine_command(objengine, OSYNC_ENGINE_COMMAND_PREPARE_WRITE, &locerror))
				goto error;
		}
		break;
	case OSYNC_ENGINE_COMMAND_WRITE:
	case OSYNC_ENGINE_COMMAND_DISCONNECT:
	case OSYNC_ENGINE_COMMAND_SYNC_DONE:
		break;
	case OSYNC_ENGINE_COMMAND_SOLVE:
		switch (command->solve_type) {
		case OSYNC_ENGINE_SOLVE_CHOOSE:
			if (!osync_mapping_engine_solve(command->mapping_engine, command->master, &locerror))
				goto error;
			break;
		case OSYNC_ENGINE_SOLVE_DUPLICATE:
			if (!osync_mapping_engine_duplicate(command->mapping_engine, &locerror))
				goto error;
			break;
		case OSYNC_ENGINE_SOLVE_IGNORE:
			if (!osync_mapping_engine_ignore(command->mapping_engine, &locerror))
				goto error;
			break;
		case OSYNC_ENGINE_SOLVE_USE_LATEST:
			if (!osync_mapping_engine_use_latest(command->mapping_engine, &locerror))
				goto error;
			break;
		}

		break;
	case OSYNC_ENGINE_COMMAND_DISCOVER:
		for (p = engine->proxies; p; p = p->next) {
			proxy = p->data;
			if (osync_client_proxy_get_member(proxy) == command->member)
				break;
			proxy = NULL;
		}
			
		if (!proxy) {
			osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "Unable to find member");
			goto error;
		}
		
		if (!osync_client_proxy_discover(proxy, _osync_engine_discover_callback, engine, &locerror))
			goto error;
		
		break;
	case OSYNC_ENGINE_COMMAND_ABORT:
		/* For nwo Command Aborting is just trigger ENGINE_EVENT_ERROR.
			 Which is basically just calling the disconnect functions and not setting
			 the synchrnoization as a successful one. */
		if (!osync_engine_has_error(engine)) {
			osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "Synchronization got aborted!");
			osync_engine_set_error(engine, locerror);
			osync_error_unref(&locerror);
		}

		osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_ERROR, engine->error);

		osync_engine_event(engine, OSYNC_ENGINE_EVENT_ERROR);
		break;

	}
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
	
 error:
	osync_engine_set_error(engine, locerror);

	g_mutex_lock(engine->syncing_mutex);
	g_cond_signal(engine->syncing);
	g_mutex_unlock(engine->syncing_mutex);

	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(&locerror));
}

void osync_engine_event(OSyncEngine *engine, OSyncEngineEvent event)
{
	OSyncList *o = NULL;
	OSyncError *locerror = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %s)", __func__, engine, osync_engine_get_eventstr(event));
	osync_assert(engine);
	

	switch (event) {
	case OSYNC_ENGINE_EVENT_CONNECTED:
		/* Now that we are connected, we call the connect_done sink-functions */
		for (o = engine->object_engines; o; o = o->next) {
			OSyncObjEngine *objengine = o->data;
			if (!osync_obj_engine_command(objengine, OSYNC_ENGINE_COMMAND_CONNECT_DONE, &locerror))
				goto error;
		}
			
		/* Now we send connect_done to the main sink */
		for (o = engine->proxies; o; o = o->next) {
			OSyncClientProxy *proxy = o->data;
			if (!osync_client_proxy_connect_done(proxy, _osync_engine_connect_done_callback, engine, NULL, &locerror))
				goto error;
		}

		break;
	case OSYNC_ENGINE_EVENT_CONNECT_DONE:
		/* Now that we are connected and send the connect_done singal, we read the changes */
		for (o = engine->object_engines; o; o = o->next) {
			OSyncObjEngine *objengine = o->data;
			if (!osync_obj_engine_command(objengine, OSYNC_ENGINE_COMMAND_READ, &locerror))
				goto error;
		}
			
		/* Now we read the main sink */
		for (o = engine->proxies; o; o = o->next) {
			OSyncClientProxy *proxy = o->data;
			if (!osync_client_proxy_get_changes(proxy, _osync_engine_get_changes_callback, engine, NULL, FALSE, &locerror))
				goto error;
		}

		break;
	case OSYNC_ENGINE_EVENT_READ:
		/* Now that we have read everything, we prepare for mapping the changes */
		for (o = engine->object_engines; o; o = o->next) {
			OSyncObjEngine *objengine = o->data;
			if (!osync_obj_engine_command(objengine, OSYNC_ENGINE_COMMAND_PREPARE_MAP, &locerror))
				goto error;
		}

		break;
	case OSYNC_ENGINE_EVENT_PREPARED_MAP:
		/* Now that we have read everything, we map the changes */
		for (o = engine->object_engines; o; o = o->next) {
			OSyncObjEngine *objengine = o->data;
			if (!osync_obj_engine_command(objengine, OSYNC_ENGINE_COMMAND_MAP, &locerror))
				goto error;
		}

		break;
	case OSYNC_ENGINE_EVENT_MAPPED:
		/* No proxies involved in this.
		 * So this get called by osync_engine_command_queue() to queue
		 * an aynchronous. This is also required to handle the mapping
		 * resolution from the conflict callback.
		 */
		break;
	case OSYNC_ENGINE_EVENT_END_CONFLICTS:
		/* No proxies involved in this.
		 * So this get called by osync_engine_command_queue() to queue
		 * an aynchronous.
		 */
		break;
	case OSYNC_ENGINE_EVENT_MULTIPLIED:
		/* Now that we have multiplied the changes, we write the changes */
		for (o = engine->object_engines; o; o = o->next) {
			OSyncObjEngine *objengine = o->data;
			if (!osync_obj_engine_command(objengine, OSYNC_ENGINE_COMMAND_PREPARE_WRITE, &locerror))
				goto error;
		}

		break;
	case OSYNC_ENGINE_EVENT_PREPARED_WRITE:
		/* Now that we have prepared the write event, we finally write the changes */
		for (o = engine->object_engines; o; o = o->next) {
			OSyncObjEngine *objengine = o->data;
			if (!osync_obj_engine_command(objengine, OSYNC_ENGINE_COMMAND_WRITE, &locerror))
				goto error;
		}

		/* Now we write the main sink */
		for (o = engine->proxies; o; o = o->next) {
			OSyncClientProxy *proxy = o->data;
			if (!osync_client_proxy_committed_all(proxy, _osync_engine_written_callback, engine, NULL, &locerror))
				goto error;
		}

		break;
	case OSYNC_ENGINE_EVENT_WRITTEN:
		/* Lets call sync done */
		for (o = engine->object_engines; o; o = o->next) {
			OSyncObjEngine *objengine = o->data;
			if (!osync_obj_engine_command(objengine, OSYNC_ENGINE_COMMAND_SYNC_DONE, &locerror))
				goto error;
		}
			
		/* Now we call sync done on the main sink */
		for (o = engine->proxies; o; o = o->next) {
			OSyncClientProxy *proxy = o->data;
			if (!osync_client_proxy_sync_done(proxy, _osync_engine_sync_done_callback, engine, NULL, &locerror))
				goto error;
		}

		break;
	case OSYNC_ENGINE_EVENT_ERROR:
		osync_trace(TRACE_ERROR, "Engine aborting due to an error: %s", osync_error_print(&(engine->error)));
		/* Fall through! - To emit disconnect commands for clean connection termination, in error condition */
	case OSYNC_ENGINE_EVENT_SYNC_DONE:

		if (engine->disconnecting) {
			osync_trace(TRACE_INTERNAL, "Already disconnecting!");
			break;
		}

		engine->disconnecting = TRUE;

		/* Store the timestamp of the last synchronization */
		osync_group_set_last_synchronization(engine->group, time(NULL)); 

		/* Lets disconnect */
		for (o = engine->object_engines; o; o = o->next) {
			OSyncObjEngine *objengine = o->data;
			if (!osync_obj_engine_command(objengine, OSYNC_ENGINE_COMMAND_DISCONNECT, &locerror))
				goto error;
		}

		/* Now we disconnect the main sink */
		for (o = engine->proxies; o; o = o->next) {
			OSyncClientProxy *proxy = o->data;

			if (!_osync_engine_is_proxy_connected(engine, proxy)) {
				_osync_engine_disconnect_callback(proxy, engine, NULL);
				continue;
			}

			if (!osync_client_proxy_disconnect(proxy, _osync_engine_disconnect_callback, engine, NULL, &locerror))
				goto error;
		}
			
		if (!engine->error)
			osync_status_update_engine(engine, OSYNC_ENGINE_EVENT_SUCCESSFUL, NULL);

		break;
	case OSYNC_ENGINE_EVENT_DISCONNECTED:

		for (o = engine->object_engines; o; o = o->next) {
			OSyncObjEngine *objengine = o->data;
			osync_obj_engine_finalize(objengine);
		}

		engine->disconnecting = FALSE;

		engine->proxy_connects = 0;
		engine->proxy_connect_done = 0;
		engine->proxy_disconnects = 0;
		engine->proxy_get_changes = 0;
		engine->proxy_written = 0;
		engine->proxy_errors = 0;
		engine->proxy_sync_done = 0;
			
		engine->obj_errors = 0;
		engine->obj_connects = 0;
		engine->obj_connect_done = 0;
		engine->obj_disconnects = 0;
		engine->obj_get_changes = 0;
		engine->obj_mapped = 0;
		engine->obj_solved = 0;
		engine->obj_multiplied = 0;
		engine->obj_prepared_write = 0;
		engine->obj_written = 0;
		engine->obj_sync_done = 0;
			
		g_mutex_lock(engine->syncing_mutex);
		g_cond_signal(engine->syncing);
		g_mutex_unlock(engine->syncing_mutex);
		break;
	case OSYNC_ENGINE_EVENT_SUCCESSFUL:
	case OSYNC_ENGINE_EVENT_PREV_UNCLEAN:
		break;
	}
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
	
 error:
	osync_engine_set_error(engine, locerror);

	g_mutex_lock(engine->syncing_mutex);
	g_cond_signal(engine->syncing);
	g_mutex_unlock(engine->syncing_mutex);

	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(&locerror));
}

osync_bool osync_engine_synchronize(OSyncEngine *engine, OSyncError **error)
{
	OSyncEngineCommand *cmd = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, engine, error);
	osync_assert(engine);

	if (engine->error) {
		osync_error_set(error, OSYNC_ERROR_INITIALIZATION, "Can't initialize the engine, it's still affected by an error."); 
		goto error;
	}

	if (engine->state != OSYNC_ENGINE_STATE_INITIALIZED) {
		osync_error_set(error, OSYNC_ERROR_INITIALIZATION, "This engine was not in state initialized.");
		goto error;
	}

	cmd = osync_try_malloc0(sizeof(OSyncEngineCommand), error);
	if (!cmd)
		goto error;
	cmd->cmd = OSYNC_ENGINE_COMMAND_CONNECT;
	
	g_async_queue_push(engine->command_queue, cmd);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_engine_synchronize_and_block(OSyncEngine *engine, OSyncError **error)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, engine, error);
	
	g_mutex_lock(engine->syncing_mutex);
	
	if (!osync_engine_synchronize(engine, error)) {
		g_mutex_unlock(engine->syncing_mutex);
		goto error;
	}

	g_cond_wait(engine->syncing, engine->syncing_mutex);
	g_mutex_unlock(engine->syncing_mutex);
	
	if (engine->error) {
		char *msg = osync_error_print_stack(&(engine->error));
		osync_trace(TRACE_ERROR, "error while synchronizing: %s", msg);
		osync_free(msg);
		osync_error_set_from_error(error, &(engine->error));
		goto error;
	}
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_engine_wait_sync_end(OSyncEngine *engine, OSyncError **error)
{
	g_mutex_lock(engine->syncing_mutex);
	g_cond_wait(engine->syncing, engine->syncing_mutex);
	g_mutex_unlock(engine->syncing_mutex);
	
	if (engine->error) {
		osync_error_set_from_error(error, &(engine->error));
		osync_error_unref(&(engine->error));
		engine->error = NULL;
		return FALSE;
	}
	return TRUE;
}

osync_bool osync_engine_discover(OSyncEngine *engine, OSyncMember *member, OSyncError **error)
{
	OSyncClientProxy *proxy = NULL;
	OSyncEngineCommand *cmd = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, engine, member, error);
	osync_assert(engine);
	
	if (engine->state == OSYNC_ENGINE_STATE_INITIALIZED) {
		osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "This engine was in state initialized: %i", engine->state);
		goto error;
	}
	
	if (!_osync_engine_start(engine, error))
		goto error;
	
	engine->state = OSYNC_ENGINE_STATE_INITIALIZED;

	/* Initialize formats before members! 
	 * Since we check if the formats claimed by the members are available */
	if (!osync_engine_initialize_formats(engine, error))
		goto error;
	
	proxy = _osync_engine_initialize_member(engine, member, error);
	if (!proxy)
		goto error;

	cmd = osync_try_malloc0(sizeof(OSyncEngineCommand), error);
	if (!cmd)
		goto error;

	/* Flush all object types of member before discovering.
		 Otherwise "old" object types get discovered again. */
	osync_member_flush_objtypes(member);

	cmd->cmd = OSYNC_ENGINE_COMMAND_DISCOVER;
	cmd->member = member;
	
	g_async_queue_push(engine->command_queue, cmd);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_engine_discover_and_block(OSyncEngine *engine, OSyncMember *member, OSyncError **error)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, engine, member, error);
	
	g_mutex_lock(engine->syncing_mutex);
	
	if (!osync_engine_discover(engine, member, error)) {
		g_mutex_unlock(engine->syncing_mutex);
		goto error_finalize;
	}
	
	g_cond_wait(engine->syncing, engine->syncing_mutex);
	g_mutex_unlock(engine->syncing_mutex);
	
	if (!osync_engine_finalize(engine, error))
		goto error;
	
	if (engine->error) {
		osync_error_set_from_error(error, &(engine->error));
		osync_error_unref(&(engine->error));
		engine->error = NULL;
		goto error;
	}
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_finalize:
	osync_engine_finalize(engine, NULL);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

unsigned int osync_engine_num_proxies(OSyncEngine *engine)
{
	osync_return_val_if_fail(engine, 0);
	return osync_list_length(engine->proxies);
}

OSyncClientProxy *osync_engine_nth_proxy(OSyncEngine *engine, unsigned int nth)
{
	osync_return_val_if_fail(engine, NULL);
	return osync_list_nth_data(engine->proxies, nth);
}

OSyncClientProxy *osync_engine_find_proxy(OSyncEngine *engine, OSyncMember *member)
{
	OSyncList *p = NULL;
	OSyncClientProxy *proxy = NULL;
	
	osync_return_val_if_fail(engine, NULL);
	
	for (p = engine->proxies; p; p = p->next) {
		proxy = p->data;
		if (osync_client_proxy_get_member(proxy) == member)
			return proxy;
	}
	
	return NULL;
}

unsigned int osync_engine_num_objengines(OSyncEngine *engine)
{
	osync_return_val_if_fail(engine, 0);
	return osync_list_length(engine->object_engines);
}

OSyncObjEngine *osync_engine_nth_objengine(OSyncEngine *engine, unsigned int nth)
{
	osync_return_val_if_fail(engine, NULL);
	return osync_list_nth_data(engine->object_engines, nth);
}

OSyncObjEngine *osync_engine_find_objengine(OSyncEngine *engine, const char *objtype)
{
	OSyncList *p = NULL;
	OSyncObjEngine *objengine = NULL;
	
	osync_return_val_if_fail(engine, NULL);

	for (p = engine->object_engines; p; p = p->next) {
		objengine = p->data;
		if (!strcmp(osync_obj_engine_get_objtype(objengine), objtype))
			return objengine;
	}
	
	return NULL;
}

void osync_engine_set_conflict_callback(OSyncEngine *engine, osync_conflict_cb callback, void *user_data)
{
	engine->conflict_callback = callback;
	engine->conflict_userdata = user_data;
}

void osync_engine_set_multiply_callback(OSyncEngine *engine, osync_multiply_cb callback, void *user_data)
{
	engine->multiply_callback = callback;
	engine->multiply_userdata = user_data;
}

void osync_engine_set_changestatus_callback(OSyncEngine *engine, osync_status_change_cb callback, void *user_data)
{
	engine->changestat_callback = callback;
	engine->changestat_userdata = user_data;
}

void osync_engine_set_mappingstatus_callback(OSyncEngine *engine, osync_status_mapping_cb callback, void *user_data)
{
	engine->mapstat_callback = callback;
	engine->mapstat_userdata = user_data;
}

void osync_engine_set_enginestatus_callback(OSyncEngine *engine, osync_status_engine_cb callback, void *user_data)
{
	engine->engstat_callback = callback;
	engine->engstat_userdata = user_data;
}

void osync_engine_set_memberstatus_callback(OSyncEngine *engine, osync_status_member_cb callback, void *user_data)
{
	engine->mebstat_callback = callback;
	engine->mebstat_userdata = user_data;
}

osync_bool osync_engine_abort(OSyncEngine *engine, OSyncError **error)
{
	OSyncError *locerror = NULL;
	OSyncEngineCommand *pending_command, *cmd;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, engine, error);

	if (engine->state != OSYNC_ENGINE_STATE_INITIALIZED) {
		osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "This engine was not in state initialized: %i", engine->state);
		goto error;
	}

	osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "Synchronization got aborted by user!");
	osync_engine_set_error(engine, locerror);
	osync_error_unref(&locerror);

	cmd = osync_try_malloc0(sizeof(OSyncEngineCommand), error);
	if (!cmd)
		goto error;
	
	cmd->cmd = OSYNC_ENGINE_COMMAND_ABORT;
	
	/* Lock the engine command queue ... */
	g_async_queue_lock(engine->command_queue);

	/* ...and flush all pending commands.
		 To make sure the abort command will be the next and last command. */
	while ((pending_command = g_async_queue_try_pop_unlocked(engine->command_queue)))
		osync_free(pending_command);

	/* Push the abort command on the empty queue. */
	g_async_queue_push_unlocked(engine->command_queue, cmd);

	/* Done. Unlock the command queue again. */
	g_async_queue_unlock(engine->command_queue);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error:	
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_engine_continue(OSyncEngine *engine, OSyncError **error)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, engine, error);

	if (engine->state != OSYNC_ENGINE_STATE_INITIALIZED) {
		osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "This engine was not in state initialized: %i", engine->state);
		goto error;
	}

	osync_engine_event(engine, engine->lastevent);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error:	
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_engine_repair(OSyncEngine *engine, OSyncError **error)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, engine, error);

	if (engine->error)
		osync_error_unref(&(engine->error));

	osync_trace(TRACE_EXIT, "%s: Engine got repaired!", __func__);
	return TRUE;
}

osync_bool osync_engine_queue_command(OSyncEngine *engine, OSyncEngineCmd cmdid, OSyncError **error)
{
	OSyncEngineCommand *cmd;
	osync_trace(TRACE_ENTRY, "%s(%p, %s, %p)", __func__, engine, osync_engine_get_cmdstr(cmdid),  error);
	
	cmd = osync_try_malloc0(sizeof(OSyncEngineCommand), error);
	if (!cmd) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
		return FALSE;
	}
	
	cmd->cmd = cmdid;
	
	g_async_queue_push(engine->command_queue, cmd);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
}

const char *osync_engine_get_cmdstr(OSyncEngineCmd cmd)
{
	const char *cmdstr = "UNKNOWN";

	switch (cmd) {
		case OSYNC_ENGINE_COMMAND_CONNECT:
			cmdstr = "CONNECTED";
			break;
		case OSYNC_ENGINE_COMMAND_CONNECT_DONE:
			cmdstr = "CONNECT_DONE";
			break;
		case OSYNC_ENGINE_COMMAND_READ:
			cmdstr = "READ";
			break;
		case OSYNC_ENGINE_COMMAND_WRITE:
			cmdstr = "WRITE";
			break;
		case OSYNC_ENGINE_COMMAND_SYNC_DONE: 
			cmdstr = "SYNC_DONE";
			break;
		case OSYNC_ENGINE_COMMAND_DISCONNECT:
			cmdstr = "DISCONNECT";
			break;
		case OSYNC_ENGINE_COMMAND_SOLVE:
			cmdstr = "SOLVE";
			break;
		case OSYNC_ENGINE_COMMAND_DISCOVER:
			cmdstr = "DISCOVER";
			break;
		case OSYNC_ENGINE_COMMAND_ABORT:
			cmdstr = "ABORT";
			break;
		case OSYNC_ENGINE_COMMAND_MAP:
			cmdstr = "MAP";
			break;
		case OSYNC_ENGINE_COMMAND_MULTIPLY:
			cmdstr = "MULTIPLY";
			break;
		case OSYNC_ENGINE_COMMAND_END_CONFLICTS:
			cmdstr = "END_CONFLICTS";
			break;
		case OSYNC_ENGINE_COMMAND_PREPARE_WRITE:
			cmdstr = "PREPARE_WRITE";
			break;
		case OSYNC_ENGINE_COMMAND_PREPARE_MAP:
			cmdstr = "PREPARE_MAP";
			break;
	}

	return cmdstr;
}

const char *osync_engine_get_eventstr(OSyncEngineEvent event)
{
	const char *eventstr = "UNKNOWN";

	switch (event) {
		case OSYNC_ENGINE_EVENT_CONNECTED:
			eventstr = "CONNECTED";
			break;
		case OSYNC_ENGINE_EVENT_CONNECT_DONE:
			eventstr = "CONNECT_DONE";
			break;
		case OSYNC_ENGINE_EVENT_ERROR:
			eventstr = "ERROR";
			break;
		case OSYNC_ENGINE_EVENT_READ:
			eventstr = "READ";
			break;
		case OSYNC_ENGINE_EVENT_WRITTEN:
			eventstr = "WRITTEN";
			break;
		case OSYNC_ENGINE_EVENT_SYNC_DONE:
			eventstr = "SYNC_DONE";
			break;
		case OSYNC_ENGINE_EVENT_DISCONNECTED:
			eventstr = "DISCONNECTED";
			break;
		case OSYNC_ENGINE_EVENT_SUCCESSFUL:
			eventstr = "SUCCESSFUL";
			break;
		case OSYNC_ENGINE_EVENT_END_CONFLICTS:
			eventstr = "END_CONFLICTS";
			break;
		case OSYNC_ENGINE_EVENT_PREV_UNCLEAN:
			eventstr = "PREV_UNCLEAN";
			break;
		case OSYNC_ENGINE_EVENT_MAPPED:
			eventstr = "MAPPED";
			break;
		case OSYNC_ENGINE_EVENT_MULTIPLIED:
			eventstr = "MULTIPLIED";
			break;
		case OSYNC_ENGINE_EVENT_PREPARED_WRITE:
			eventstr = "PREPARE_WRITE";
			break;
		case OSYNC_ENGINE_EVENT_PREPARED_MAP:
			eventstr = "PREPARE_MAP";
			break;
	}

	return eventstr;
}

#ifdef OPENSYNC_UNITTESTS
void osync_engine_set_schemadir(OSyncEngine *engine, const char *schema_dir)
{
	osync_assert(engine);
	osync_assert(schema_dir);

	if (engine->schema_dir)
		osync_free(engine->schema_dir);

	engine->schema_dir = osync_strdup(schema_dir); 
}
#endif /* OPENSYNC_UNITTESTS */

