/*
 * libopensync - A synchronization framework
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

#include "opensync-format.h"
#include "opensync-helper.h"
#include "opensync-plugin.h"

#include "opensync/helper/opensync_sink_state_db_internals.h"
#include "opensync/helper/opensync_hashtable_internals.h"

#include "opensync_plugin_info_internals.h" /* due to osync_plugin_info_get_configdir() */

#include "opensync_objtype_sink.h"
#include "opensync_objtype_sink_internals.h"
#include "opensync_objtype_sink_private.h"

OSyncObjTypeSink *osync_objtype_main_sink_new(OSyncError **error)
{
	return osync_objtype_sink_new(NULL, error);
}

OSyncObjTypeSink *osync_objtype_sink_new(const char *objtype, OSyncError **error)
{
	OSyncObjTypeSink *sink = osync_try_malloc0(sizeof(OSyncObjTypeSink), error);
	if (!sink)
		return FALSE;
	
	sink->objtype = osync_strdup(objtype);
	sink->ref_count = 1;

	sink->preferred_format = NULL;
	
	sink->read = TRUE;
	sink->getchanges = TRUE;
	sink->write = TRUE;

	sink->enabled = TRUE;

	memset(&sink->timeout, 0, sizeof(sink->timeout));

	return sink;
}

OSyncObjTypeSink *osync_objtype_sink_ref(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	
	g_atomic_int_inc(&(sink->ref_count));

	return sink;
}

void osync_objtype_sink_unref(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	
	if (g_atomic_int_dec_and_test(&(sink->ref_count))) {
		while (sink->objformatsinks) {
			osync_objformat_sink_unref(sink->objformatsinks->data);
			sink->objformatsinks = osync_list_remove(sink->objformatsinks, sink->objformatsinks->data);
		}

		if (sink->state_db)
			osync_sink_state_db_unref(sink->state_db);

		if (sink->hashtable)
			osync_hashtable_unref(sink->hashtable);

		if (sink->preferred_format)
			osync_free(sink->preferred_format);

		if (sink->objtype)
			osync_free(sink->objtype);
		
		osync_free(sink);
	}
}

void osync_objtype_sink_enable_state_db(OSyncObjTypeSink *sink, osync_bool enable)
{
	osync_return_if_fail(sink);
	sink->state_db_requested = enable;
}

osync_bool osync_objtype_sink_has_state_db(OSyncObjTypeSink *sink)
{
	osync_return_val_if_fail(sink, FALSE);
	return sink->state_db_requested; 
}

OSyncSinkStateDB *osync_objtype_sink_get_state_db(OSyncObjTypeSink *sink)
{
	osync_return_val_if_fail(sink, NULL);
	return sink->state_db;
}

void osync_objtype_sink_set_state_db(OSyncObjTypeSink *sink, OSyncSinkStateDB *state_db)
{
	osync_return_if_fail(sink);
	if (sink->state_db)
		osync_sink_state_db_unref(sink->state_db);

	sink->state_db = osync_sink_state_db_ref(state_db);
}

void osync_objtype_sink_enable_hashtable(OSyncObjTypeSink *sink, osync_bool enable)
{
	osync_return_if_fail(sink);
	sink->hashtable_requested = enable;
}

osync_bool osync_objtype_sink_has_hashtable(OSyncObjTypeSink *sink)
{
	osync_return_val_if_fail(sink, FALSE);
	return sink->hashtable_requested; 
}

OSyncHashTable *osync_objtype_sink_get_hashtable(OSyncObjTypeSink *sink)
{
	osync_return_val_if_fail(sink, NULL);
	return sink->hashtable;
}

void osync_objtype_sink_set_hashtable(OSyncObjTypeSink *sink, OSyncHashTable *hashtable)
{
	osync_return_if_fail(sink);
	if (sink->hashtable)
		osync_hashtable_unref(sink->hashtable);

	sink->hashtable = osync_hashtable_ref(hashtable);
}

const char *osync_objtype_sink_get_name(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return sink->objtype;
}

void osync_objtype_sink_set_name(OSyncObjTypeSink *sink, const char *name)
{
	osync_assert(sink);
	if (sink->objtype)
		osync_free(sink->objtype);
	sink->objtype = osync_strdup(name);
}

const char *osync_objtype_sink_get_preferred_format(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return sink->preferred_format;
}

void osync_objtype_sink_set_preferred_format(OSyncObjTypeSink *sink, const char *preferred_format)
{
	osync_assert(sink);
	if (sink->preferred_format)
		osync_free(sink->preferred_format);
	sink->preferred_format = osync_strdup(preferred_format);
}

unsigned int osync_objtype_sink_num_objformat_sinks(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return osync_list_length(sink->objformatsinks);
}

OSyncObjFormatSink *osync_objtype_sink_nth_objformat_sink(OSyncObjTypeSink *sink, unsigned int nth)
{
	osync_assert(sink);
	return osync_list_nth_data(sink->objformatsinks, nth);
}

OSyncObjFormatSink *osync_objtype_sink_find_objformat_sink(OSyncObjTypeSink *sink, OSyncObjFormat *objformat)
{
	OSyncList *f = NULL;
	osync_assert(sink);
	osync_assert(objformat);

	f = sink->objformatsinks;
	for (; f; f = f->next) {
		OSyncObjFormatSink *formatsink = f->data;
		const char *objformat_name = osync_objformat_get_name(objformat);
		if (!strcmp(osync_objformat_sink_get_objformat(formatsink), objformat_name))
			return formatsink;
	}
	return NULL;
}

OSyncList *osync_objtype_sink_get_objformat_sinks(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return osync_list_copy(sink->objformatsinks);
}

void osync_objtype_sink_add_objformat_sink(OSyncObjTypeSink *sink, OSyncObjFormatSink *objformatsink)
{
	osync_assert(sink);
	osync_assert(objformatsink);

	if (!osync_list_find(sink->objformatsinks, objformatsink)) {
		sink->objformatsinks = osync_list_append(sink->objformatsinks, objformatsink);
		osync_objformat_sink_ref(objformatsink);
	}
}

void osync_objtype_sink_remove_objformat_sink(OSyncObjTypeSink *sink, OSyncObjFormatSink *objformatsink)
{
	osync_assert(sink);
	osync_assert(objformatsink);

	sink->objformatsinks = osync_list_remove(sink->objformatsinks, objformatsink);
	osync_objformat_sink_unref(objformatsink);
}

osync_bool osync_objtype_sink_get_function_read(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return sink->func_read;
}

void osync_objtype_sink_set_function_read(OSyncObjTypeSink *sink, osync_bool read)
{
	osync_assert(sink);
	sink->func_read = read;
}

osync_bool osync_objtype_sink_get_function_getchanges(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return sink->func_getchanges;
}

void osync_objtype_sink_set_function_getchanges(OSyncObjTypeSink *sink, osync_bool getchanges)
{
	osync_assert(sink);
	sink->func_getchanges = getchanges;
}

void *osync_objtype_sink_get_userdata(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return sink->userdata;
}

void osync_objtype_sink_set_userdata(OSyncObjTypeSink *sink, void *userdata)
{
	osync_return_if_fail(sink);
	sink->userdata = userdata;
}

void osync_objtype_sink_get_changes(OSyncObjTypeSink *sink, OSyncPluginInfo *info, osync_bool slow_sync, OSyncContext *ctx)
{
	OSyncObjTypeSinkFunctions functions;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, sink, info, ctx);
	osync_assert(sink);
	osync_assert(ctx);
	
	functions = sink->functions;
	if (sink->objtype && !functions.get_changes) {
		osync_context_report_error(ctx, OSYNC_ERROR_GENERIC, "No get_changeinfo function was given");
		osync_trace(TRACE_EXIT_ERROR, "%s: No get_changes function was given", __func__);
		return;
	} else if (!functions.get_changes) {
		osync_context_report_success(ctx);
	} else {
		functions.get_changes(sink, info, ctx, slow_sync, osync_objtype_sink_get_userdata(sink));
	}

	osync_trace(TRACE_EXIT, "%s", __func__);
}

void osync_objtype_sink_read_change(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncChange *change, OSyncContext *ctx)
{
	OSyncObjTypeSinkFunctions functions;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %p)", __func__, sink, info, change, ctx);
	osync_assert(sink);
	osync_assert(ctx);
	osync_assert(change);
	
	functions = sink->functions;


	if (sink->objtype && !functions.read) {
		osync_context_report_error(ctx, OSYNC_ERROR_GENERIC, "No read function was given");
		osync_trace(TRACE_EXIT_ERROR, "%s: No read function was given", __func__);
		return;
	} else if (!functions.read) {
		osync_context_report_success(ctx);
	} else {
		functions.read(sink, info, ctx, change,  osync_objtype_sink_get_userdata(sink));
	}

	osync_trace(TRACE_EXIT, "%s", __func__);
}

void osync_objtype_sink_connect(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx)
{
	OSyncObjTypeSinkFunctions functions;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, sink, info, ctx);
	osync_assert(sink);
	osync_assert(ctx);

	functions = sink->functions;
	if (!functions.connect) {
		osync_context_report_success(ctx);
	} else {
		functions.connect(sink, info, ctx, osync_objtype_sink_get_userdata(sink));
	}

	osync_trace(TRACE_EXIT, "%s", __func__);
}

void osync_objtype_sink_disconnect(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx)
{
	OSyncObjTypeSinkFunctions functions;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, sink, info, ctx);
	osync_assert(sink);
	osync_assert(ctx);
	
	functions = sink->functions;
	if (!functions.disconnect) {
		osync_context_report_success(ctx);
	} else {
		functions.disconnect(sink, info, ctx, osync_objtype_sink_get_userdata(sink));
	}

	osync_trace(TRACE_EXIT, "%s", __func__);
}

void osync_objtype_sink_sync_done(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx)
{
	OSyncObjTypeSinkFunctions functions;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, sink, info, ctx);
	osync_assert(sink);
	osync_assert(ctx);
	
	functions = sink->functions;
	if (!functions.sync_done)
		osync_context_report_success(ctx);
	else
		functions.sync_done(sink, info, ctx, osync_objtype_sink_get_userdata(sink));
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

void osync_objtype_sink_connect_done(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx)
{
	OSyncObjTypeSinkFunctions functions;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, sink, info, ctx);
	osync_assert(sink);
	osync_assert(ctx);
	
	functions = sink->functions;
	if (!functions.connect_done)
		osync_context_report_success(ctx);
	else
		functions.connect_done(sink, info, ctx, osync_objtype_sink_get_slowsync(sink), osync_objtype_sink_get_userdata(sink));
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

void osync_objtype_sink_commit_change(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncChange *change, OSyncContext *ctx)
{
	OSyncObjTypeSinkFunctions functions;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %p)", __func__, sink, info, change, ctx);
	g_assert(sink);
	g_assert(change);
	g_assert(ctx);

	functions = sink->functions;

	// Send the change
	if (sink->objtype && !functions.commit) {
		osync_context_report_error(ctx, OSYNC_ERROR_GENERIC, "No commit_change function was given");
		osync_trace(TRACE_EXIT_ERROR, "%s: No commit_change function was given", __func__);
		return;
	} else if (!functions.commit) {
		osync_context_report_success(ctx);
	} else {
		functions.commit(sink, info, ctx, change, osync_objtype_sink_get_userdata(sink));
	}

	osync_trace(TRACE_EXIT, "%s", __func__);
}

void osync_objtype_sink_committed_all(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx)
{
	OSyncObjTypeSinkFunctions functions;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, sink,  info, ctx);
	osync_assert(sink);
	osync_assert(ctx);
	
	functions = sink->functions;
	if (functions.committed_all) {
		functions.committed_all(sink, info, ctx, osync_objtype_sink_get_userdata(sink));
	} else {
		osync_context_report_success(ctx);
	}
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
}

osync_bool osync_objtype_sink_is_enabled(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return sink->enabled;
}

void osync_objtype_sink_set_enabled(OSyncObjTypeSink *sink, osync_bool enabled)
{
	osync_assert(sink);
	sink->enabled = enabled;
}

osync_bool osync_objtype_sink_is_available(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return sink->available;
}

void osync_objtype_sink_set_available(OSyncObjTypeSink *sink, osync_bool available)
{
	osync_assert(sink);
	sink->available = available;
}

osync_bool osync_objtype_sink_get_write(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return sink->write;
}

void osync_objtype_sink_set_write(OSyncObjTypeSink *sink, osync_bool write)
{
	osync_assert(sink);
	sink->write = write;
}

void osync_objtype_sink_set_getchanges(OSyncObjTypeSink *sink, osync_bool getchanges)
{
	osync_assert(sink);
	sink->getchanges = getchanges;
}

osync_bool osync_objtype_sink_get_getchanges(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return sink->getchanges;
}

osync_bool osync_objtype_sink_get_read(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return sink->read;
}

void osync_objtype_sink_set_read(OSyncObjTypeSink *sink, osync_bool read)
{
	osync_assert(sink);
	sink->read = read;
}

osync_bool osync_objtype_sink_get_slowsync(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return sink->slowsync;
}

void osync_objtype_sink_set_slowsync(OSyncObjTypeSink *sink, osync_bool slowsync)
{
	osync_assert(sink);
	osync_trace(TRACE_INTERNAL, "%s: Setting slow-sync of object type \"%s\" to %i", __func__, sink->objtype, slowsync);
	sink->slowsync = slowsync;
}

void osync_objtype_sink_set_connect_timeout(OSyncObjTypeSink *sink, unsigned int timeout)
{
	osync_assert(sink);
	sink->timeout.connect = timeout;
}

unsigned int osync_objtype_sink_get_connect_timeout_or_default(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return sink->timeout.connect ? sink->timeout.connect : OSYNC_SINK_TIMEOUT_CONNECT;
}

unsigned int osync_objtype_sink_get_connect_timeout(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return sink->timeout.connect;
}

void osync_objtype_sink_set_disconnect_timeout(OSyncObjTypeSink *sink, unsigned int timeout)
{
	osync_assert(sink);
	sink->timeout.disconnect = timeout;
}

unsigned int osync_objtype_sink_get_disconnect_timeout_or_default(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return sink->timeout.disconnect ? sink->timeout.disconnect : OSYNC_SINK_TIMEOUT_DISCONNECT;
}

unsigned int osync_objtype_sink_get_disconnect_timeout(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return sink->timeout.disconnect;
}

void osync_objtype_sink_set_getchanges_timeout(OSyncObjTypeSink *sink, unsigned int timeout)
{
	osync_assert(sink);
	sink->timeout.get_changes = timeout;
}

unsigned int osync_objtype_sink_get_getchanges_timeout_or_default(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return sink->timeout.commit ? sink->timeout.get_changes : OSYNC_SINK_TIMEOUT_GETCHANGES;
}

unsigned int osync_objtype_sink_get_getchanges_timeout(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return sink->timeout.get_changes;
}

void osync_objtype_sink_set_commit_timeout(OSyncObjTypeSink *sink, unsigned int timeout)
{
	osync_assert(sink);
	sink->timeout.commit = timeout;
}

unsigned int osync_objtype_sink_get_commit_timeout_or_default(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return sink->timeout.commit ? sink->timeout.commit : OSYNC_SINK_TIMEOUT_COMMIT;
}

unsigned int osync_objtype_sink_get_commit_timeout(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return sink->timeout.commit;
}

void osync_objtype_sink_set_committedall_timeout(OSyncObjTypeSink *sink, unsigned int timeout)
{
	osync_assert(sink);
	sink->timeout.committed_all = timeout;
}

unsigned int osync_objtype_sink_get_committedall_timeout_or_default(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return sink->timeout.committed_all ? sink->timeout.committed_all : OSYNC_SINK_TIMEOUT_COMMITTEDALL;
}

unsigned int osync_objtype_sink_get_committedall_timeout(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return sink->timeout.committed_all;
}

void osync_objtype_sink_set_syncdone_timeout(OSyncObjTypeSink *sink, unsigned int timeout)
{
	osync_assert(sink);
	sink->timeout.sync_done = timeout;
}

unsigned int osync_objtype_sink_get_syncdone_timeout_or_default(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return sink->timeout.sync_done ? sink->timeout.sync_done : OSYNC_SINK_TIMEOUT_SYNCDONE;
}

unsigned int osync_objtype_sink_get_syncdone_timeout(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return sink->timeout.sync_done;
}

void osync_objtype_sink_set_connectdone_timeout(OSyncObjTypeSink *sink, unsigned int timeout)
{
	osync_assert(sink);
	sink->timeout.connect_done = timeout;
}

unsigned int osync_objtype_sink_get_connectdone_timeout_or_default(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return sink->timeout.connect_done ? sink->timeout.connect_done : OSYNC_SINK_TIMEOUT_CONNECTDONE;
}

unsigned int osync_objtype_sink_get_connectdone_timeout(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return sink->timeout.connect_done;
}

void osync_objtype_sink_set_read_timeout(OSyncObjTypeSink *sink, unsigned int timeout)
{
	osync_assert(sink);
	sink->timeout.read = timeout;
}

unsigned int osync_objtype_sink_get_read_timeout_or_default(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return sink->timeout.read ? sink->timeout.read : OSYNC_SINK_TIMEOUT_READ;
}

unsigned int osync_objtype_sink_get_read_timeout(OSyncObjTypeSink *sink)
{
	osync_assert(sink);
	return sink->timeout.read;
}

osync_bool osync_objtype_sink_open_state_db(OSyncObjTypeSink *sink, OSyncPluginInfo *plugin_info, OSyncError **error)
{
	char *filename;

	osync_assert(sink);

	if (!osync_objtype_sink_has_state_db(sink))
		return TRUE;
	
	/* FIXME: Get rid of file location!
	 * Later with further OSyncDB modifications this should be file-hierarchy indepdendent.
	 * And The first arg should just consists of the Member ID
	 */
	filename = osync_strdup_printf("%s%canchor.db",
			osync_plugin_info_get_configdir(plugin_info),
			G_DIR_SEPARATOR);

	sink->state_db = osync_sink_state_db_new(filename, sink->objtype, error);
	if (!sink->state_db)
		goto error;

	osync_free(filename);

	return TRUE;
error:
	osync_free(filename);
	return FALSE;
}

osync_bool osync_objtype_sink_load_hashtable(OSyncObjTypeSink *sink, OSyncPluginInfo *plugin_info, OSyncError **error)
{
	osync_bool new_hashtable;
	char *hashtablepath;

	osync_assert(sink);

	if (!osync_objtype_sink_has_hashtable(sink))
		return TRUE;

	if (sink->hashtable)
		osync_hashtable_unref(sink->hashtable);
	
	/* FIXME: Get rid of file lcoation!
	 * Later with fruther OSyncDB modifications this should be file-hiarchy indepdendent.
	 * And The first arg should just consists of the Member ID
	 */
	hashtablepath = osync_strdup_printf("%s%chashtable.db",
			osync_plugin_info_get_configdir(plugin_info),
			G_DIR_SEPARATOR);

	sink->hashtable = osync_hashtable_new(hashtablepath, sink->objtype, &new_hashtable, error);
	if (!sink->hashtable)
		goto error;

	if (new_hashtable && osync_objtype_sink_get_slowsync(sink) == FALSE) {
		osync_objtype_sink_set_slowsync(sink, TRUE);
		osync_trace(TRACE_INTERNAL, "SLOWSYNC: new_hashtable got created, maybe it got deleted or malformed?!");
	}

	if (!osync_hashtable_load(sink->hashtable, error))
		goto error_free_hashtable;

	osync_free(hashtablepath);

	return TRUE;

error_free_hashtable:
	osync_hashtable_unref(sink->hashtable);
error:
	osync_free(hashtablepath);
	return FALSE;
}

osync_bool osync_objtype_sink_save_hashtable(OSyncObjTypeSink *sink, OSyncError **error)
{
	osync_assert(sink);

	if (!osync_objtype_sink_has_hashtable(sink))
		return TRUE;

	osync_assert(sink->hashtable);

	if (!osync_hashtable_save(sink->hashtable, error))
		goto error;

	return TRUE;

error:
	return FALSE;
}

void osync_objtype_sink_set_connect_func(OSyncObjTypeSink *sink, OSyncSinkConnectFn connect_func)
{
	osync_return_if_fail(sink);
	sink->functions.connect = connect_func;
}

void osync_objtype_sink_set_connect_done_func(OSyncObjTypeSink *sink, OSyncSinkConnectDoneFn connect_done_func)
{
	osync_return_if_fail(sink);
	sink->functions.connect_done = connect_done_func;
}

void osync_objtype_sink_set_get_changes_func(OSyncObjTypeSink *sink, OSyncSinkGetChangesFn get_changes_func)
{
	osync_return_if_fail(sink);
	sink->functions.get_changes = get_changes_func;

	if (sink->functions.get_changes)
		sink->func_getchanges = TRUE;
}

void osync_objtype_sink_set_commit_func(OSyncObjTypeSink *sink, OSyncSinkCommitFn commit_func)
{
	osync_return_if_fail(sink);
	sink->functions.commit = commit_func;
}

void osync_objtype_sink_set_committed_all_func(OSyncObjTypeSink *sink, OSyncSinkCommittedAllFn committed_all_func)
{
	osync_return_if_fail(sink);
	sink->functions.committed_all = committed_all_func;
}

void osync_objtype_sink_set_read_func(OSyncObjTypeSink *sink, OSyncSinkReadFn read_func)
{
	osync_return_if_fail(sink);
	sink->functions.read = read_func;

	if (sink->functions.read)
		sink->func_read = TRUE;
}

void osync_objtype_sink_set_sync_done_func(OSyncObjTypeSink *sink, OSyncSinkSyncDoneFn sync_done_func)
{
	osync_return_if_fail(sink);
	sink->functions.sync_done = sync_done_func;
}

void osync_objtype_sink_set_disconnect_func(OSyncObjTypeSink *sink, OSyncSinkDisconnectFn disconnect_func)
{
	osync_return_if_fail(sink);
	sink->functions.disconnect = disconnect_func;
}

