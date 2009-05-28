/*
 * file-sync - A plugin for the opensync framework
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

#include "mock_sync.h"
#include "mock_format.h"

#include "opensync/plugin/opensync_plugin_info_private.h"	/* FIXME: access direclty private header */

static osync_bool mock_get_error(long long int memberid, const char *domain)
{
	const char *env = g_getenv(domain);

	if (!env)
		return FALSE;

	int num = atoi(env);
	int mask = 1 << (memberid - 1);
	if (num & mask) {
		char *chancestr = g_strdup_printf("%s_PROB", domain);
		const char *chance = g_getenv(chancestr);
		g_free(chancestr);
		if (!chance)
				return TRUE;
		int prob = atoi(chance);
		if (prob >= g_random_int_range(0, 100))
			return TRUE;
	}
	return FALSE;
}

static char *mock_generate_hash(struct stat *buf)
{
	return g_strdup_printf("%i-%i", (int)buf->st_mtime, (int)buf->st_ctime);
}

static void mock_connect(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *data)
{
	osync_bool state_match;
	OSyncSinkStateDB *state_db = osync_objtype_sink_get_state_db(sink); 
	MockDir *dir = data;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %p)", __func__, sink, info, ctx, data);
	
	if (mock_get_error(info->memberid, "CONNECT_ERROR")) {
		osync_context_report_error(ctx, OSYNC_ERROR_EXPECTED, "Triggering CONNECT_ERROR error");
		osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, "Triggering CONNECT_ERROR error");
		return;
	}

	if (mock_get_error(info->memberid, "CONNECT_TIMEOUT")) {
		/* Don't report context back ... let it timeout! */
		osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, "Triggering CONNECT_TIMEOUT error");
		return;
	}

	if (mock_get_error(info->memberid, "CONNECT_SLOWSYNC"))
		osync_context_report_slowsync(ctx);

	/* Skip Objtype related stuff like hashtable and state db */
	if (mock_get_error(info->memberid, "MAINSINK_CONNECT"))
		goto end;


	/* From this line MockDir *dir is required, for ObjTypeSink specific stuff: #821 */
	osync_assert(dir);
	dir->committed_all = TRUE;

	/* Sanity check for connect_done - reset it to FALSE.
	 * To make sure it get reseted to TRUE before get_changes().
	 */
	dir->connect_done = FALSE;

	osync_assert_msg(osync_sink_state_equal(state_db, "path", dir->path, &state_match, NULL), "Not expected to fail");

	if (!state_match)
		osync_context_report_slowsync(ctx);

	osync_assert(g_file_test(dir->path, G_FILE_TEST_IS_DIR));

end:	
	osync_context_report_success(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
}

static void mock_connect_done(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, osync_bool slow_sync, void *data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %i, %p)", __func__, sink, info, ctx, slow_sync, data);
	MockDir *dir = data;

	dir->connect_done_slowsync = slow_sync;

	if (mock_get_error(info->memberid, "CONNECT_DONE_ERROR")) {
		osync_context_report_error(ctx, OSYNC_ERROR_EXPECTED, "Triggering CONNECT_DONE_ERROR error");
		return;
	}
	if (mock_get_error(info->memberid, "CONNECT_DONE_TIMEOUT"))
		return;

	/* Validate connect_done() call before get_changes(),
	 * and after connect().
	 */
	osync_assert(!dir->connect_done);
	dir->connect_done = TRUE;
	
	osync_context_report_success(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void mock_mainsink_disconnect(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %p)", __func__, sink, info, ctx, data);

	mock_env *env = data;
	
	osync_assert(data);
	
	GList *o = env->directories;
	for (; o; o = o->next) {
		MockDir *sink_dir = o->data;
		if (!g_getenv("NO_COMMITTED_ALL_CHECK"))
				osync_assert(sink_dir->committed_all == TRUE);
		sink_dir->committed_all = FALSE;
	}

	if (mock_get_error(info->memberid, "DISCONNECT_ERROR")) {
		osync_context_report_error(ctx, OSYNC_ERROR_EXPECTED, "Triggering DISCONNECT_ERROR error");
		return;
	}
	if (mock_get_error(info->memberid, "DISCONNECT_TIMEOUT"))
		return;

	osync_context_report_success(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void mock_disconnect(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %p)", __func__, sink, info, ctx, data);

	MockDir *dir = data;

	osync_assert(dir);
	
	if (!g_getenv("NO_COMMITTED_ALL_CHECK"))
		osync_assert(dir->committed_all == TRUE);
	dir->committed_all = FALSE;


	if (mock_get_error(info->memberid, "DISCONNECT_ERROR")) {
		osync_context_report_error(ctx, OSYNC_ERROR_EXPECTED, "Triggering DISCONNECT_ERROR error");
		return;
	}
	if (mock_get_error(info->memberid, "DISCONNECT_TIMEOUT"))
		return;

	osync_context_report_success(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}


//typedef void (* OSyncSinkWriteFn) 
//typedef void (* OSyncSinkCommittedAllFn) (void *data, OSyncPluginInfo *info, OSyncContext *ctx);


static void mock_read(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, OSyncChange *change, void *data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %p, %p)", __func__, sink, info, ctx, change, data);
	MockDir *dir = data;
	OSyncError *error = NULL;

	char *filename = g_strdup_printf("%s/%s", dir->path, osync_change_get_uid(change));
	
	OSyncFileFormat *file = osync_try_malloc0(sizeof(OSyncFileFormat), &error);
	osync_assert(file);
	file->path = g_strdup(osync_change_get_uid(change));
	
	struct stat filestats;
	stat(filename, &filestats);
	file->userid = filestats.st_uid;
	file->groupid = filestats.st_gid;
	file->mode = filestats.st_mode;
	file->last_mod = filestats.st_mtime;
			
	osync_assert(osync_file_read(filename, &(file->data), &(file->size), &error));

	OSyncData *odata = osync_data_new((char *)file, sizeof(OSyncFileFormat), dir->objformat, &error);
	osync_assert(odata);

	osync_trace(TRACE_INTERNAL, "odata: %p", odata);
	
	osync_data_set_objtype(odata, osync_objtype_sink_get_name(sink));
	osync_change_set_data(change, odata);
	osync_data_unref(odata);
	
	osync_context_report_success(ctx);
	
	g_free(filename);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
}

static osync_bool mock_write(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, OSyncChange *change, void *data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %p, %p)", __func__, sink, info, ctx, change, data);
	MockDir *dir = data;
	OSyncError *error = NULL;
	OSyncData *odata = NULL;
	char *buffer = NULL;
	unsigned int size = 0;
	
	char *filename = g_strdup_printf ("%s/%s", dir->path, osync_change_get_uid(change));
			
	switch (osync_change_get_changetype(change)) {
		case OSYNC_CHANGE_TYPE_DELETED:
			osync_assert(remove(filename) == 0);
			break;
		case OSYNC_CHANGE_TYPE_ADDED:
			osync_assert(!g_file_test(filename, G_FILE_TEST_EXISTS));
			/* No break. Continue below */
		case OSYNC_CHANGE_TYPE_MODIFIED:
			//FIXME add ownership for file-sync
			odata = osync_change_get_data(change);
			g_assert(odata);
			osync_data_get_data(odata, &buffer, &size);
			g_assert(buffer);
			g_assert(size == sizeof(OSyncFileFormat));
			
			OSyncFileFormat *file = (OSyncFileFormat *)buffer;
			
			osync_assert(osync_file_write(filename, file->data, file->size, file->mode, &error));
			break;
		case OSYNC_CHANGE_TYPE_UNMODIFIED:
			fail("Unmodified in a change function?!");
			break;
		case OSYNC_CHANGE_TYPE_UNKNOWN:
			fail("Unknown Change Type");
			break;
	}
	
	g_free(filename);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
}

/** Report files on a directory
 *
 * NOTE: If 'dir' is non-empty it MUST start it a slash. This is just
 * to make easier concatenation of the paths, and we can just concatenate
 * fsinfo->path and subdir to get the complete path.
 *
 * @param dir The fsinfo->path subdirectory that should be reported. Use
 *            an empty string to report files on fsinfo->path. Should
 *            start with a slash. See note above.
 *
 */
static void mock_report_dir(MockDir *directory, const char *subdir, OSyncContext *ctx, OSyncPluginInfo *info, OSyncObjTypeSink *sink)
{
	GError *gerror = NULL;
	const char *de = NULL;
	char *path = NULL;
	GDir *dir = NULL;
	OSyncError *error = NULL;
	OSyncList *sorted_dir_list = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %s, %p, %p)", __func__, directory, subdir, ctx, sink);

	OSyncHashTable *hashtable = osync_objtype_sink_get_hashtable(sink);
	OSyncFormatEnv *formatenv = osync_plugin_info_get_format_env(info);
	osync_assert(formatenv);

	path = g_build_filename(directory->path, subdir, NULL);
	osync_trace(TRACE_INTERNAL, "path %s", path);
	
	dir = g_dir_open(path, 0, &gerror);
	osync_assert(dir);

	while((de = g_dir_read_name(dir))) {
		sorted_dir_list = osync_list_insert_sorted(sorted_dir_list,
			g_strdup(de), (OSyncCompareFunc)g_strcmp0);
	}

	g_dir_close(dir);

	while(sorted_dir_list) {
		de = sorted_dir_list->data;
		char *filename = g_build_filename(path, de, NULL);
		char *relative_filename = NULL;
		if (!subdir)
			relative_filename = g_strdup(de);
		else
			relative_filename = g_build_filename(subdir, de, NULL);
		g_free(sorted_dir_list->data);
		sorted_dir_list = osync_list_remove(sorted_dir_list, sorted_dir_list->data);

		osync_trace(TRACE_INTERNAL, "path2 %s %s", filename, relative_filename);
		
		if (g_file_test(filename, G_FILE_TEST_IS_REGULAR)) {
			
			struct stat buf;
			stat(filename, &buf);
			char *hash = mock_generate_hash(&buf);
			
			/* Report normal files */
			OSyncChange *change = osync_change_new(&error);
			osync_assert(change);
			
			osync_change_set_uid(change, relative_filename);

			osync_change_set_hash(change, hash);
			g_free(hash);

			OSyncChangeType type = osync_hashtable_get_changetype(hashtable, change);
			
			osync_change_set_changetype(change, type);
			osync_hashtable_update_change(hashtable, change);

			if (type == OSYNC_CHANGE_TYPE_UNMODIFIED) {
				g_free(filename);
				g_free(relative_filename);
				osync_change_unref(change);
				continue;
			}

			OSyncFileFormat *file = osync_try_malloc0(sizeof(OSyncFileFormat), &error);
			osync_assert(file);

			file->path = g_strdup(relative_filename);
			
			OSyncData *odata = NULL;

			if (!mock_get_error(info->memberid, "ONLY_INFO")) {
				osync_assert(osync_file_read(filename, &(file->data), &(file->size), &error));

				if (mock_get_error(info->memberid, "SLOW_REPORT"))
					g_usleep(1*G_USEC_PER_SEC);
				
				odata = osync_data_new((char *)file, sizeof(OSyncFileFormat), directory->objformat, &error);
				osync_assert(odata);


				osync_change_set_data(change, odata);

			}
			
			osync_data_set_objtype(odata, osync_objtype_sink_get_name(sink));
			osync_data_unref(odata);
	
			osync_context_report_change(ctx, change);

			osync_change_unref(change);
		}

		g_free(filename);
		g_free(relative_filename);

	}

	g_free(path);
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void mock_get_changes(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, osync_bool slow_sync, void *data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %i, %p)", __func__, sink, info, ctx, slow_sync, data);
	MockDir *dir = data;
	OSyncError *error = NULL;
	OSyncHashTable *hashtable = osync_objtype_sink_get_hashtable(sink);

	osync_assert(dir->committed_all == TRUE);
	dir->committed_all = FALSE;

	osync_assert(dir->connect_done == TRUE);
	dir->connect_done = FALSE;

	/* Validate that connect_doene and get_changes slow_sync
	 * is the same. To avoid mix-up of a "late slow-sync".
	 */
	if (!mock_get_error(info->memberid, "MAINSINK_CONNECT"))
		osync_assert(dir->connect_done_slowsync == slow_sync);

	if (mock_get_error(info->memberid, "GET_CHANGES_ERROR")) {
		osync_context_report_error(ctx, OSYNC_ERROR_EXPECTED, "Triggering GET_CHANGES_ERROR error");
		osync_trace(TRACE_EXIT_ERROR, "%s - Triggering GET_CHANGES error", __func__);
		return;
	}

	if (mock_get_error(info->memberid, "GET_CHANGES_TIMEOUT")) {
		osync_trace(TRACE_EXIT, "%s - Triggering GET_CHANGES_TIMEOUT (without context report!)", __func__);
		return;
	}

	if (mock_get_error(info->memberid, "GET_CHANGES_TIMEOUT2"))
		g_usleep(8*G_USEC_PER_SEC);
		
	if (slow_sync) {
		osync_trace(TRACE_INTERNAL, "Slow sync requested");
		osync_assert(osync_hashtable_slowsync(hashtable, &error));
	}
	
	osync_trace(TRACE_INTERNAL, "get_changes for %s", osync_objtype_sink_get_name(sink));

	mock_report_dir(dir, NULL, ctx, info, sink);
	
	OSyncList *u, *uids = osync_hashtable_get_deleted(hashtable);
	for (u = uids; u; u = u->next) {
		OSyncChange *change = osync_change_new(&error);
		osync_assert(change);

		const char *uid = u->data;
		
		osync_change_set_uid(change, uid);
		osync_change_set_changetype(change, OSYNC_CHANGE_TYPE_DELETED);
		
		OSyncData *odata = osync_data_new(NULL, 0, dir->objformat, &error);
		osync_assert(odata);
		
		osync_data_set_objtype(odata, osync_objtype_sink_get_name(sink));
		osync_change_set_data(change, odata);
		osync_data_unref(odata);
		
		osync_context_report_change(ctx, change);
		
		osync_hashtable_update_change(hashtable, change);
	
		osync_change_unref(change);
	}
	
	osync_context_report_success(ctx);
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void mock_commit_change(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, OSyncChange *change, void *data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %p, %p)", __func__, sink, info, ctx, change, data);
	MockDir *dir = data;
	OSyncHashTable *hashtable = osync_objtype_sink_get_hashtable(sink);
	
	char *filename = NULL;

	osync_assert(dir->committed_all == FALSE);

	if (mock_get_error(info->memberid, "COMMIT_ERROR")) {
		osync_context_report_error(ctx, OSYNC_ERROR_EXPECTED, "Triggering COMMIT_ERROR error");
		return;
	}
	if (mock_get_error(info->memberid, "COMMIT_TIMEOUT")) {
		osync_trace(TRACE_EXIT_ERROR, "COMMIT_TIMEOUT (mock-sync)!");
		return;
	}
	
	if (!mock_write(sink, info, ctx, change, data)) {
		osync_trace(TRACE_EXIT_ERROR, "%s", __func__);
		return;
	}
	
	filename = g_strdup_printf ("%s/%s", dir->path, osync_change_get_uid(change));
	char *hash = NULL;
	
	if (osync_change_get_changetype(change) != OSYNC_CHANGE_TYPE_DELETED) {
		struct stat buf;
		stat(filename, &buf);
		hash = mock_generate_hash(&buf);
		osync_change_set_hash(change, hash);
		g_free(hash);
	}
	g_free(filename);


	osync_hashtable_update_change(hashtable, change);
	
	osync_context_report_success(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void mock_committed_all(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *context, void *data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %p)", __func__, sink, info, context, data);
	MockDir *dir = data;

	osync_assert(dir->committed_all == FALSE);
	dir->committed_all = TRUE;

	if (mock_get_error(info->memberid, "COMMITTED_ALL_ERROR")) {
		osync_context_report_error(context, OSYNC_ERROR_EXPECTED, "Triggering COMMITTED_ALL_ERROR error");
		osync_trace(TRACE_EXIT_ERROR, "%s: Reporting error", __func__);
		return;
	}

	osync_context_report_success(context);

	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void mock_sync_done(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %p)", __func__, sink, info, ctx, data);
	OSyncSinkStateDB *state_db = osync_objtype_sink_get_state_db(sink);
	MockDir *dir = data;

	if (mock_get_error(info->memberid, "SYNC_DONE_ERROR")) {
		osync_context_report_error(ctx, OSYNC_ERROR_EXPECTED, "Triggering SYNC_DONE_ERROR error");
		return;
	}
	if (mock_get_error(info->memberid, "SYNC_DONE_TIMEOUT"))
		return;
	
	osync_assert_msg(osync_sink_state_set(state_db, "path", dir->path, NULL), "Not expected to fail!");
	
	osync_context_report_success(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

/* In initialize, we get the config for the plugin. Here we also must register
 * all _possible_ objtype sinks. */
static void *mock_initialize(OSyncPlugin *plugin, OSyncPluginInfo *info, OSyncError **error)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, plugin, info, error);

	if (mock_get_error(info->memberid, "INIT_NULL_NOERROR")) {
		osync_trace(TRACE_EXIT, "%s: %s", __func__, "Everything is fine. I don't need plugin userdata.");
		return NULL;
	}

	if (mock_get_error(info->memberid, "INIT_NULL")) {
		osync_error_set(error, OSYNC_ERROR_EXPECTED, "Triggering INIT_NULL error");
		osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
		return NULL;
	}

	mock_env *env = osync_try_malloc0(sizeof(mock_env), error);
	osync_assert(env);

	OSyncFormatEnv *formatenv = osync_plugin_info_get_format_env(info);
	osync_assert(formatenv);
	
	OSyncPluginConfig *config = osync_plugin_info_get_config(info);
	osync_assert(config);
	
	if (mock_get_error(info->memberid, "MAINSINK_CONNECT")) {
		env->mainsink = osync_objtype_main_sink_new(error);

		osync_objtype_sink_set_connect_func(env->mainsink, mock_connect);
		osync_objtype_sink_set_disconnect_func(env->mainsink, mock_mainsink_disconnect);

		osync_objtype_sink_set_userdata(env->mainsink, env);

		osync_plugin_info_set_main_sink(info, env->mainsink);
	}
	
	/* Now we register the objtypes that we can sync. This plugin is special. It can
	 * synchronize any objtype we configure it to sync and where a conversion
	 * path to the file format can be found */
	OSyncList *objtypesinks = osync_plugin_info_get_objtype_sinks(info);
	OSyncList *list = NULL;
	for (list = objtypesinks;list; list = list->next) {
		MockDir *dir = osync_try_malloc0(sizeof(MockDir), error);
		osync_assert(dir);

		dir->committed_all = TRUE;

		OSyncObjTypeSink *sink = (OSyncObjTypeSink*)list->data;
		osync_assert(sink);

		const char *objtype = osync_objtype_sink_get_name(sink);
		dir->res = osync_plugin_config_find_active_resource(config, objtype);
		dir->path = osync_plugin_resource_get_path(dir->res);
		osync_assert(dir->path);

		OSyncList *format_sinks = osync_plugin_resource_get_objformat_sinks(dir->res);
		osync_assert(osync_list_length(format_sinks) == 1);
		OSyncObjFormatSink *format_sink = osync_list_nth_data(format_sinks, 0);
		const char *objformat_str = osync_objformat_sink_get_objformat(format_sink);
		osync_assert(objformat_str);
		dir->objformat = osync_format_env_find_objformat(formatenv, objformat_str);
		osync_assert(dir->objformat);
		osync_objformat_ref(dir->objformat);

		osync_list_free(format_sinks);
		/*
		const char *objformat = osync_objformat_get_name(dir->objformat); 
		OSyncObjFormatSink *format_sink = osync_objformat_sink_new(objformat, error);
		if (!format_sink)
			return NULL;

		osync_objtype_sink_add_objformat_sink(sink, format_sink);
		*/

		/* Sanity check for connect_done */
		dir->connect_done = TRUE;
		
		if (!mock_get_error(info->memberid, "MAINSINK_CONNECT")) {
			osync_objtype_sink_set_connect_func(sink, mock_connect);
			osync_objtype_sink_set_connect_done_func(sink, mock_connect_done);
			osync_objtype_sink_set_disconnect_func(sink, mock_disconnect);
		}

		osync_objtype_sink_set_get_changes_func(sink, mock_get_changes);

		osync_objtype_sink_set_committed_all_func(sink, mock_committed_all);
		osync_objtype_sink_set_commit_func(sink, mock_commit_change);
		osync_objtype_sink_set_read_func(sink, mock_read);
		osync_objtype_sink_set_sync_done_func(sink, mock_sync_done);
		
		/* We pass the MockDir object to the sink, so we dont have to look it up
		 * again once the functions are called */
		osync_objtype_sink_set_userdata(sink, dir);

		/* Request an Anchor */
		osync_objtype_sink_enable_state_db(sink, TRUE);

		/* Request an Hashtable */
		osync_objtype_sink_enable_hashtable(sink, TRUE);

		//Lets reduce the timeouts a bit so the checks work faster
		osync_objtype_sink_set_connect_timeout(sink, 2);
		osync_objtype_sink_set_getchanges_timeout(sink, 2);
		osync_objtype_sink_set_commit_timeout(sink, 4);
		osync_objtype_sink_set_committedall_timeout(sink, 4);
		osync_objtype_sink_set_syncdone_timeout(sink, 2);
		osync_objtype_sink_set_disconnect_timeout(sink, 2);

		osync_objtype_sink_set_read_timeout(sink, 2);


/* XXX No testcase is currently using this at all! */
#if 0		
	
		if (g_getenv("NO_TIMEOUTS")) {

			/* XXX Timeout value of wouldn't work out, since
			   the Sink object would fallback to the default timeout value:

			 sink->timeout.connect ? sink->timeout.connect : OSYNC_SINK_TIMEOUT_CONNECT;

			 Really needed?!
			 */

			osync_objtype_sink_set_connect_timeout(sink, 0);
			osync_objtype_sink_set_getchanges_timeout(sink, 0);
			osync_objtype_sink_set_commit_timeout(sink, 0);
			osync_objtype_sink_set_committedall_timeout(sink, 0);
			osync_objtype_sink_set_syncdone_timeout(sink, 0);
			osync_objtype_sink_set_disconnect_timeout(sink, 0);

			osync_objtype_sink_set_read_timeout(sink, 0);
		}
		
		/* What is meant by this?! Maybe OSyncPlugin.useable?! Not used at all...
		if (g_getenv("IS_AVAILABLE"))
			info->functions.is_available = mock_is_available;
		*/

#endif
		env->directories = g_list_append(env->directories, dir);
	}
	osync_list_free(objtypesinks);
	osync_trace(TRACE_EXIT, "%s: %p", __func__, env);
	return (void *)env;
}

static void mock_finalize(void *data)
{
	mock_env *env = data;

	while (env->directories) {
		MockDir *dir = env->directories->data;

		osync_plugin_resource_unref(dir->res);
		osync_objformat_unref(dir->objformat);

		env->directories = g_list_remove(env->directories, dir);
		g_free(dir);
	}

	if (env->mainsink)
		osync_objtype_sink_unref(env->mainsink);


	g_free(env);
}

/* Here we actually tell opensync which sinks are available. For this plugin, we
 * go through the list of directories and enable all, since all have been configured */
static osync_bool mock_discover(OSyncPluginInfo *info, void *data, OSyncError **error)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, error);
	
	OSyncPluginConfig *config = osync_plugin_info_get_config(info);
	osync_assert_msg(config, "No OSyncPluginConfig set for mock_discover!");

	/*
	OSyncFormatEnv *formatenv = osync_plugin_info_get_format_env(info);
	osync_assert_msg(config, "No OSyncFormatEnv set for mock_discover!");
	*/

	OSyncList *r = osync_plugin_config_get_resources(config);
	for (; r; r = r->next) {
		OSyncPluginResource *res = r->data;
		OSyncObjTypeSink *sink;

		const char *objtype = osync_plugin_resource_get_objtype(res);
		/* Check for ObjType sink */
		if ((sink = osync_plugin_info_find_objtype(info, objtype)))
			osync_objtype_sink_set_available(sink, TRUE);
 	}

	
	OSyncVersion *version = osync_version_new(error);
	osync_version_set_plugin(version, "mock-sync");
	//osync_version_set_vendor(version, "version");
	//osync_version_set_modelversion(version, "version");
	//osync_version_set_firmwareversion(version, "firmwareversion");
	//osync_version_set_softwareversion(version, "softwareversion");
	//osync_version_set_hardwareversion(version, "hardwareversion");
	osync_plugin_info_set_version(info, version);
	osync_version_unref(version);

	/* we can set here the capabilities, but for the file-sync
	 * plugin they are static and shipped with opensync */


	if (mock_get_error(info->memberid, "MOCK_DISCOVER_ERROR")) {
		osync_error_set(error, OSYNC_ERROR_EXPECTED, "MOCK_DISCOVER_ERROR on purpose!");
		osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
		return FALSE;
	}

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
}

osync_bool get_sync_info(OSyncPluginEnv *env, OSyncError **error)
{
	OSyncPlugin *plugin = osync_plugin_new(error);
	osync_assert(plugin);
	
	osync_plugin_set_name(plugin, "mock-sync");
	osync_plugin_set_longname(plugin, "Mock Synchronization Plugin");
	osync_plugin_set_description(plugin, "Plugin to synchronize files on the local filesystem");
	
	osync_plugin_set_initialize(plugin, mock_initialize);
	osync_plugin_set_finalize(plugin, mock_finalize);
	osync_plugin_set_discover(plugin, mock_discover);
	
	osync_plugin_env_register_plugin(env, plugin);
	osync_plugin_unref(plugin);

	plugin = osync_plugin_new(error);
	osync_assert(plugin);
	
	osync_plugin_set_name(plugin, "mock-sync-external");
	osync_plugin_set_longname(plugin, "Mock Synchronization Plugin with Start Type External");
	osync_plugin_set_description(plugin, "Plugin to synchronize files on the local filesystem for unit tests");
	osync_plugin_set_start_type(plugin, OSYNC_START_TYPE_EXTERNAL);
	
	osync_plugin_set_initialize(plugin, mock_initialize);
	osync_plugin_set_finalize(plugin, mock_finalize);
	osync_plugin_set_discover(plugin, mock_discover);
	
	osync_plugin_env_register_plugin(env, plugin);
	osync_plugin_unref(plugin);

	
	return TRUE;
}

int get_version(void)
{
	return 1;
}
