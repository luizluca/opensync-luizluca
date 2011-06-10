/*
 * external_demo.c - example implementation of an userspace process
 *                   which acts as external OpenSync plugin 
 *
 * Copyright (C) 2009  Daniel Gollub <gollub@b1-systems.de>
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

#include <opensync/opensync.h>
#include <opensync/opensync-client.h>
#include <opensync/opensync-data.h>
#include <opensync/opensync-format.h>
#include <opensync/opensync-plugin.h>
#include <opensync/opensync-helper.h>
#include <opensync/opensync-version.h>

static void get_changes(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, osync_bool slow_sync, void *data)
{
	printf("[EXTERNAL-DEMO]: %s slow_sync: %s\n", __func__, slow_sync ? "yes" : "no");
	
	/** XXX: here you get your changes and report them via osync_context_report_change() */

	osync_context_report_success(ctx);
}

static void initialize(OSyncPlugin *plugin, OSyncPluginInfo *info, OSyncError **error)
{
	OSyncList *sinks, *s;
	OSyncPluginConfig *config;

	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, info, error);

	printf("[EXTERNAL-DEMO]: %s\n", __func__);

	/*
	 * get the config
	 */
	config = osync_plugin_info_get_config(info);
	if (!config) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to get config.");
		goto error;
	}

	sinks = osync_plugin_info_get_objtype_sinks(info);
	for (s = sinks; s; s = s->next) {
		OSyncObjTypeSink *sink = s->data;

		/* Here you register all your objtype sink functions ... */
		osync_objtype_sink_set_get_changes_func(sink, get_changes);
		/* You can also add sink function for
		 * - connect()
		 * - disconnect()
		 * - commit()
		 * - sync_done()
		 * - ...
		 */
	}

	osync_trace(TRACE_EXIT, "%s: %p", __func__, NULL);
	return;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
}

static void finalize(void *data)
{
	printf("[EXTERNAL-DEMO]: %s\n", __func__);
}

static osync_bool discover(OSyncPluginInfo *info, void *data, OSyncError **error)
{

	printf("[EXTERNAL-DEMO]: %s\n", __func__);
	OSyncList *s, *sinks = osync_plugin_info_get_objtype_sinks(info);
	for (s = sinks; s; s = s->next) {
		OSyncObjTypeSink *sink = (OSyncObjTypeSink *) s->data;
		osync_objtype_sink_set_available(sink, TRUE);
	}
	osync_list_free(sinks);
	
	OSyncVersion *version = osync_version_new(error);
	osync_version_set_plugin(version, "external-demo");
	osync_plugin_info_set_version(info, version);
	osync_version_unref(version);

	return TRUE;
}

int main(int argc, char **argv)
{
	const char *pipe_path;
	OSyncError *error = NULL;
	OSyncClient *client = NULL;
	OSyncPlugin *plugin = NULL;

	if (!argv[1]) {
		fprintf(stderr, "pipe path is missing!\n");
		return 1;
	}

	pipe_path = argv[1];


	/** Plugin **/
	plugin = osync_plugin_new(&error);
	if (!plugin)
		goto error;

	osync_plugin_set_initialize(plugin, initialize);
	osync_plugin_set_finalize(plugin, finalize);
	osync_plugin_set_discover(plugin, discover);


	/** Client */
	client = osync_client_new(&error);
	if (!client)
		goto error;

	osync_client_set_pipe_path(client, pipe_path);
	osync_client_set_plugin(client, plugin);


	printf("[EXTERNAL-DEMO]: %s OSyncPlugin:%p OSyncClient:%p\n", __func__, plugin, client);
	printf("[EXTERNAL-DEMO]: Starting (blocking) OSyncClient ...\n");

	if (!osync_client_run_and_block(client, &error))
		goto error;

	printf("[EXTERNAL-DEMO]: OSyncClient completed.");

	osync_client_unref(client);

	return 0;

error:
	fprintf(stderr, "[EXTERNAL-DEMO] Error: %s\n", osync_error_print(&error));
	osync_error_unref(&error);
	return 1;
}

