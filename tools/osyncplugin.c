/*
 * osyncplugin - swiss-knife for testing OpenSync synchronization plugins 
 * Copyright (C) 2008  Daniel Gollub <gollub@b1-systems.de>
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include <glib.h>

#include <opensync/opensync.h>
#include <opensync/opensync-ipc.h>
#include <opensync/opensync-data.h>
#include <opensync/opensync-plugin.h>
#include <opensync/opensync-format.h>

char *pluginpath = NULL;
char *formatpath = NULL;
char *pluginname = NULL;
char *configfile = NULL;
char *configdir = NULL;
char *syncgroup = NULL;
osync_bool pluginlist = FALSE;
osync_bool connect_requested_slowsync = FALSE;
GList *sinks = NULL;
GList *cmdlist = NULL;
GList *changesList = NULL;
GMainContext *ctx = NULL;
OSyncPlugin *plugin = NULL;
OSyncPluginEnv *plugin_env = NULL;
OSyncFormatEnv *format_env = NULL;
OSyncPluginInfo *plugin_info = NULL;

typedef enum {
	CMD_EMPTY,
	CMD_INITIALIZE,
	CMD_FINALIZE,
	CMD_CONNECT,
	CMD_DISCONNECT,
	CMD_SYNC,
	CMD_SLOWSYNC,
	CMD_FASTSYNC,
	CMD_COMMIT,
	CMD_COMMITTEDALL,
	CMD_READ,
	CMD_WRITE,
	CMD_SYNCDONE,
	CMD_DISCOVER
} Cmd;

typedef struct _Command {
	Cmd cmd;
	char *arg;
	GCond *cond;
	GMutex *mutex;
	OSyncObjTypeSink *sink;
	osync_bool done;
} Command;

typedef enum {
	/* regular sync - plugin decides for slow or fast sync */
	SYNCTYPE_NORMAL,
	/* force fast sync */
	SYNCTYPE_FORCE_FASTSYNC,
	/* foce slow sync */
	SYNCTYPE_FORCE_SLOWSYNC
} SyncType;

/*
 * Argument handling 
 */
static Command *new_command(Cmd cmd, const char *arg) {

	Command *newcommand = malloc(sizeof(Command));
	if (!newcommand) {
		perror("Can't allocate new command");
		exit(errno);
	}

	memset(newcommand, 0, sizeof(Command));

	newcommand->cmd = cmd;
	if (arg)
		newcommand->arg = strdup(arg);

	newcommand->mutex = g_mutex_new();
	newcommand->cond = g_cond_new();

	cmdlist = g_list_append(cmdlist, newcommand);

	return newcommand;
}

static void free_command(Command **cmd) {
	assert(*cmd);

	if ((*cmd)->arg)
		free((*cmd)->arg);
	
	free(*cmd);
	*cmd = NULL;
}

static void usage(const char *name)
{
	/* TODO: improve usage output */
	fprintf(stderr, "Usage: %s\n", name);
	
	fprintf (stderr, "Configuration options:\n");
	fprintf (stderr, "[--config] \tSet config file\n");
	fprintf (stderr, "[--configdir] \tSet different config directory. Default: ~./opensync\n");
	fprintf (stderr, "[--syncgroup] \tSet the name of the sync group. Default: osyncplugin\n");
	
	fprintf (stderr, "Plugin options:\n");
	fprintf (stderr, "[--plugin] \tSet plugin\n");
	fprintf (stderr, "[--pluginpath] \t\n");
	fprintf (stderr, "[--pluginlist] \tShow list of plugins\n");
	
	fprintf (stderr, "Format options:\n");
	fprintf (stderr, "[--formatpath] \t\n");
	
	fprintf (stderr, "Command options:\n");
	fprintf (stderr, "[--initialize] \t\n");
	fprintf (stderr, "[--connect] \t\n");
	fprintf (stderr, "[--disconnect] \t\n");
	fprintf (stderr, "[--finalize] \t\n");
	fprintf (stderr, "[--slowsync] \t\n");
	fprintf (stderr, "[--sync] \t\n");
	fprintf (stderr, "[--fastsync] \t\n");
	fprintf (stderr, "[--syncdone] \t\n");
	fprintf (stderr, "[--committedall] \t\n");
	fprintf (stderr, "[--commit] \t\n");
	fprintf (stderr, "[--write] \t\n");
	fprintf (stderr, "[--read] \t\n");
	fprintf (stderr, "[--empty] \t\n");

	exit(1);
}

static void parse_args(int argc, char **argv) {

	int i;
	char *arg;

	for (i=1; i < argc; i++) {

		arg = argv[i];

		if (!strcmp(arg, "--config") || !strcmp(arg, "-c")) {
			if (!configfile)
				configfile = strdup(argv[i+1]);

			i++;
			continue;
		} else if (!strcmp(arg, "--configdir") || !strcmp(arg, "-C")) {
			if (!configdir)
				configdir = strdup(argv[i+1]);

			i++;
			continue;
		} else if (!strcmp(arg, "--syncgroup")) {
			if (!syncgroup)
				syncgroup = strdup(argv[i+1]);
			i++;
			continue;
		} else if (!strcmp(arg, "--plugin") || !strcmp(arg, "-p")) {
			if (!pluginname)
				pluginname = strdup(argv[i+1]);

			i++;
			continue;
		} else if (!strcmp(arg, "--pluginpath") || !strcmp(arg, "-P")) {
			if (!pluginpath)
				pluginpath = strdup(argv[i+1]);

			i++;
			continue;
		} else if (!strcmp(arg, "--pluginlist") || !strcmp(arg, "-L")) {
			pluginlist= TRUE;

			i++;
			continue;
		} else if (!strcmp(arg, "--formatpath") || !strcmp(arg, "-F")) {
			if (!formatpath)
				formatpath = strdup(argv[i+1]);

			i++;
			continue;
		} else if (!strcmp(arg, "--initialize")) {
			new_command(CMD_INITIALIZE, NULL);
			continue;
		} else if (!strcmp(arg, "--connect")) {
			if (!argv[i+1] || *argv[i+1] == '-')
				new_command(CMD_CONNECT, NULL);
			else
				new_command(CMD_CONNECT, argv[++i]);

			continue;
		} else if (!strcmp(arg, "--disconnect")) {
			if (!argv[i+1] || *argv[i+1] == '-')
				new_command(CMD_DISCONNECT, NULL);
			else
				new_command(CMD_DISCONNECT, argv[++i]);

			continue;
		} else if (!strcmp(arg, "--finalize")) {
			new_command(CMD_FINALIZE, NULL);
			continue;
		} else if (!strcmp(arg, "--slowsync")) {
			if (!argv[i+1] || *argv[i+1] == '-')
				new_command(CMD_SLOWSYNC, NULL);
			else
				new_command(CMD_SLOWSYNC, argv[++i]);

			continue;
		} else if (!strcmp(arg, "--sync")) {
			if (!argv[i+1] || *argv[i+1] == '-')
				new_command(CMD_SYNC, NULL);
			else
				new_command(CMD_SYNC, argv[++i]);

			continue;
		} else if (!strcmp(arg, "--fastsync")) {
			if (!argv[i+1] || *argv[i+1] == '-')
				new_command(CMD_FASTSYNC, NULL);
			else
				new_command(CMD_FASTSYNC, argv[++i]);

			continue;
		} else if (!strcmp(arg, "--syncdone")) {
			if (!argv[i+1] || *argv[i+1] == '-')
				new_command(CMD_SYNCDONE, NULL);
			else
				new_command(CMD_SYNCDONE, argv[++i]);

			continue;
		} else if (!strcmp(arg, "--committedall")) {
			if (!argv[i+1] || *argv[i+1] == '-')
				new_command(CMD_COMMITTEDALL, NULL);
			else
				new_command(CMD_COMMITTEDALL, argv[++i]);

			continue;
		} else if (!strcmp(arg, "--commit")) {
			if (!argv[i+1] || *argv[i+1] == '-')
				new_command(CMD_COMMIT, NULL);
			else
				new_command(CMD_COMMIT, argv[++i]);

			continue;
		} else if (!strcmp(arg, "--write")) {
			if (!argv[i+1] || *argv[i+1] == '-')
				new_command(CMD_WRITE, NULL);
			else
				new_command(CMD_WRITE, argv[++i]);

			continue;
		} else if (!strcmp(arg, "--read")) {
			if (!argv[i+1] || *argv[i+1] == '-')
				new_command(CMD_READ, NULL);
			else
				new_command(CMD_READ, argv[++i]);

			continue;
		} else if (!strcmp(arg, "--discover")) {
			if (!argv[i+1] || *argv[i+1] == '-')
				new_command(CMD_DISCOVER, NULL);
			else
				new_command(CMD_DISCOVER, argv[++i]);

			continue;
		} else if (!strcmp(arg, "--empty")) {
			if (!argv[i+1] || *argv[i+1] == '-')
				new_command(CMD_EMPTY, NULL);
			else
				new_command(CMD_EMPTY, argv[++i]);

			continue;
		} else {
			fprintf(stderr, "Unknown argument: %s\n", argv[i]);
			usage(argv[0]);
		}
	}
	
	if (pluginlist)
		return;

	if (!cmdlist)
		fprintf(stderr, "No command set.\n");

	if (!pluginname)
		fprintf(stderr, "No plugin set.\n");

	if (!configdir)
		fprintf(stderr, "No working/configuraiton directory set.\n");

	if (!pluginname || !cmdlist || !configdir)
		usage(argv[0]);
}

/*
 * Plugin Commands
 */

static osync_bool init(OSyncError **error) {
	OSyncPluginConfig *config;
	assert(!plugin);
	assert(!plugin_env);

	if (!(plugin_env = osync_plugin_env_new(error)))
		goto error;

	if (!(format_env = osync_format_env_new(error)))
		goto error_free_pluginenv;

	if (!osync_format_env_load_plugins(format_env, formatpath, error))
		goto error_free_formatenv;

	if (osync_error_is_set(error)) {
		fprintf(stderr, "WARNING! Some plugins couldn't get loaded in "
				"format plugin environment: %s\n", osync_error_print(error));
		osync_error_unref(error);
	}

	if (!osync_plugin_env_load(plugin_env, pluginpath, error))
		goto error_free_pluginenv;

	if (osync_error_is_set(error)) {
		fprintf(stderr, "WARNING! Some plugins couldn't get loaded in "
				"plugin environment: %s\n", osync_error_print(error));
		osync_error_unref(error);
	}

	if (!(plugin = osync_plugin_env_find_plugin(plugin_env, pluginname))) {
		osync_error_set(error, OSYNC_ERROR_PLUGIN_NOT_FOUND, "Plugin not found: \"%s\"", pluginname);
		goto error_free_pluginenv;
	}

	if (!(plugin_info = osync_plugin_info_new(error)))
		goto error_free_pluginenv;

	config = osync_plugin_config_new(error);
	if (!config)
		goto error_free_plugininfo;

	if (osync_plugin_get_config_type(plugin) != OSYNC_PLUGIN_NO_CONFIGURATION && configfile) {
		OSyncList *r = NULL;
		if (!osync_plugin_config_file_load(config, configfile, error))
			goto error_free_pluginconfig;

		osync_plugin_info_set_config(plugin_info, config);

		/** Redudant(aka. stolen) code from opensync/client/opensync_client.c */
		/* Enable active sinks */

		if (config)
			r = osync_plugin_config_get_resources(config);

		for (; r; r = r->next) {
			OSyncPluginResource *res = r->data;
			OSyncObjTypeSink *sink;

			const char *objtype = osync_plugin_resource_get_objtype(res); 
			OSyncList *o = NULL;
			/* Check for ObjType sink */
			if (!(sink = osync_plugin_info_find_objtype(plugin_info, objtype))) {
				sink = osync_objtype_sink_new(objtype, error);
				if (!sink)
					goto error_free_pluginconfig;

				osync_plugin_info_add_objtype(plugin_info, sink);
			}
			OSyncList *objformats = osync_plugin_resource_get_objformat_sinks(res);
			for ( o = objformats; o; o = o->next) {
				OSyncObjFormatSink *format_sink = (OSyncObjFormatSink *) o->data; 
				osync_objtype_sink_add_objformat_sink(sink, format_sink);
			}
			osync_list_free(objformats);
		}

		osync_plugin_config_unref(config);

	}

	if (!configfile && osync_plugin_get_config_type(plugin) == OSYNC_PLUGIN_NEEDS_CONFIGURATION) {
		osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "Plugin \"%s\" requires configuration!", pluginname); 
		goto error_free_pluginconfig;
	}

	assert(!ctx);
	ctx = g_main_context_new();

	osync_plugin_info_set_configdir(plugin_info, configdir);
	osync_plugin_info_set_loop(plugin_info, ctx);
	osync_plugin_info_set_format_env(plugin_info, format_env);
	osync_plugin_info_set_groupname(plugin_info, syncgroup);

	return TRUE;

	/*
		error_free_loop:
		g_main_context_unref(ctx);
	*/
 error_free_pluginconfig:
	osync_plugin_config_unref(config);
 error_free_plugininfo:
	osync_plugin_info_unref(plugin_info);
 error_free_formatenv:
	osync_format_env_unref(format_env);
	format_env = NULL;
 error_free_pluginenv:
	osync_plugin_env_unref(plugin_env);
	plugin_env = NULL;
 error:	
	return FALSE;
}

static void *plugin_initialize(OSyncError **error)
{
	OSyncList *list;
	OSyncList *objtypesinks = NULL;
	void *plugin_data;
	osync_bool couldinit;
	couldinit = osync_plugin_initialize(plugin, &(plugin_data), plugin_info, error);
	if (!couldinit)
		goto error;

	objtypesinks = osync_plugin_info_get_objtype_sinks(plugin_info);
	list = objtypesinks;
	while(list) {
		OSyncObjTypeSink *sink = (OSyncObjTypeSink*)list->data;
		if (!osync_objtype_sink_open_state_db(sink, plugin_info, error)) 
			goto error;

		if (!osync_objtype_sink_load_hashtable(sink, plugin_info, error))
			goto error;
		
		list = list->next;
	}
	osync_list_free(objtypesinks);

	return plugin_data;
error:
	osync_list_free(objtypesinks);
	return NULL;
}

static void finalize_plugin(void **plugin_data)
{

	if (!*plugin_data)
		return;

	osync_plugin_finalize(plugin, *plugin_data);
	*plugin_data = NULL;
}


static OSyncObjTypeSink *find_sink(const char *objtype, OSyncError **error)
{
	OSyncObjTypeSink *sink = NULL;
	assert(objtype);

	sink = osync_plugin_info_find_objtype(plugin_info, objtype);
	if (!sink) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to find sink for %s", objtype);
		return NULL;
	}

	return sink;
}

static OSyncObjTypeSink *get_main_sink()
{
	return osync_plugin_info_get_main_sink(plugin_info);
}

static const char *_osyncplugin_changetype_str(OSyncChange *change)
{
	const char *type;
	assert(change);

	switch (osync_change_get_changetype(change)) {
	case OSYNC_CHANGE_TYPE_ADDED:
		type = "ADDED";
		break;
	case OSYNC_CHANGE_TYPE_UNMODIFIED:
		type = "UNMODIFIED";
		break;
	case OSYNC_CHANGE_TYPE_DELETED:
		type = "DELETED";
		break;
	case OSYNC_CHANGE_TYPE_MODIFIED:
		type = "MODIFIED";
		break;
	case OSYNC_CHANGE_TYPE_UNKNOWN:
	default:
		type = "UNKNOWN";
		break;
	}

	return type;
}

//typedef void (* OSyncContextChangeFn) (OSyncChange *, void *);
static void _osyncplugin_ctx_change_callback(OSyncChange *change, void *user_data)
{
	Command *cmd = (Command *) user_data;		
	OSyncObjTypeSink *sink = cmd->sink;

	printf("GETCHANGES:\t%s\t%s\t%s", 
	       _osyncplugin_changetype_str(change), 
	       osync_objtype_sink_get_name(sink),	
	       osync_change_get_uid(change));

	if (osync_change_get_objformat(change))
		printf("\t%s", osync_objformat_get_name(osync_change_get_objformat(change)));

	printf("\n");

	osync_change_ref(change);
	changesList = g_list_append(changesList, change);
}

//typedef void (* OSyncContextCallbackFn)(void *, OSyncError *);
static void _osyncplugin_ctx_callback_getchanges(void *user_data, OSyncError *error)
{
	Command *cmd = (Command *) user_data;		
	OSyncObjTypeSink *sink = cmd->sink;

	if (error)
		fprintf(stderr, "Sink \"%s\": %s\n", osync_objtype_sink_get_name(sink), osync_error_print(&error));

	g_mutex_lock(cmd->mutex);
	g_cond_signal(cmd->cond);
	cmd->done = TRUE;
	g_mutex_unlock(cmd->mutex);

}

static osync_bool get_changes_sink(Command *cmd, OSyncObjTypeSink *sink, SyncType type, OSyncError **error)
{
	OSyncContext *context = NULL;
	osync_bool slowsync = FALSE;

	assert(sink);

	switch (type) {
	case SYNCTYPE_NORMAL:
		slowsync = connect_requested_slowsync;
		break;
	case SYNCTYPE_FORCE_FASTSYNC:
		slowsync = FALSE;
		break;
	case SYNCTYPE_FORCE_SLOWSYNC:
		slowsync = TRUE;
		break;
	}

	context = osync_context_new(error);
	if (!context)
		goto error;

	osync_context_set_changes_callback(context, _osyncplugin_ctx_change_callback);
	osync_context_set_callback(context, _osyncplugin_ctx_callback_getchanges, cmd);

	osync_plugin_info_set_sink(plugin_info, sink);

	osync_objtype_sink_get_changes(sink, plugin_info, slowsync, context);

	osync_context_unref(context);


	return TRUE;

 error:
	return FALSE;
}

static osync_bool get_changes(Command *cmd, SyncType type, OSyncError **error)
{
	OSyncObjTypeSink *sink = NULL;
	OSyncList *list;
	OSyncList *objtypesinks = NULL;
	const char *objtype = cmd->arg;

	if (objtype) {
		sink = find_sink(objtype, error);
		if (!sink)
			goto error;

		cmd->sink = sink;
		if (!get_changes_sink(cmd, sink, type, error))
			goto error;

	} else {
		/* all available objtypes */
		objtypesinks = osync_plugin_info_get_objtype_sinks(plugin_info);
		list = objtypesinks;
		while(list) {
			sink = (OSyncObjTypeSink*)list->data;

			cmd->sink = sink;
			if (!get_changes_sink(cmd, sink, type, error))
				goto error;
			list = list->next;
		}
		/* last but not least - the main sink */
		if (get_main_sink())
			if (!get_changes_sink(cmd, get_main_sink(), type, error))
				goto error;
		
		osync_list_free(objtypesinks);
	}

	return TRUE;

 error:
	osync_list_free(objtypesinks);
	return FALSE;
}

static void _osyncplugin_ctx_callback_slowsync(void *user_data)
{

	OSyncObjTypeSink *sink = (OSyncObjTypeSink *) user_data;

	printf("SlowSync got requested by CONNECT function");
	if (osync_objtype_sink_get_name(sink))
		printf(" for ObjType: \"%s\"", osync_objtype_sink_get_name(sink));

	connect_requested_slowsync = TRUE;

	printf(".\n");
}

static void _osyncplugin_ctx_callback_connect(void *user_data, OSyncError *error)
{
	OSyncError *locerror = NULL;
	Command *cmd = NULL;
	//OSyncObjTypeSink *sink = NULL;

	assert(user_data);

	cmd = (Command *) user_data;		
	// unused, but valid, if sink is needed:
	//sink = cmd->sink;

	if (error) {
		osync_error_set_from_error(&locerror, &error);
		goto error;
	}

	g_mutex_lock(cmd->mutex);
	g_cond_signal(cmd->cond);
	cmd->done = TRUE;
	g_mutex_unlock(cmd->mutex);

	return;
 error:
	fprintf(stderr, "ERROR: %s\n", osync_error_print(&locerror));
	return;
}

static osync_bool connect_sink(Command *cmd, OSyncObjTypeSink *sink, OSyncError **error) {
	OSyncContext *context = NULL;
	assert(sink);
	assert(cmd);

	context = osync_context_new(error);
	if (!context)
		goto error;

	cmd->sink = sink;

	osync_context_set_callback(context, _osyncplugin_ctx_callback_connect, cmd);
	osync_context_set_slowsync_callback(context, _osyncplugin_ctx_callback_slowsync, sink);

	osync_plugin_info_set_sink(plugin_info, sink);

	/* Reset slow-sync state */
	connect_requested_slowsync = FALSE;

	osync_objtype_sink_connect(sink, plugin_info, context);

	osync_context_unref(context);

	return TRUE;

 error:	
	return FALSE;
}

static osync_bool connect_plugin(Command *cmd, OSyncError **error)
{	
	OSyncObjTypeSink *sink = NULL;
	OSyncList *objtypesinks = NULL;
	OSyncList *list;
	const char *objtype = cmd->arg;

	if (objtype) {
		sink = find_sink(objtype, error);
		if (!sink)
			goto error;

		if (!connect_sink(cmd, sink, error))
			goto error;
	} else {
		objtypesinks = osync_plugin_info_get_objtype_sinks(plugin_info);
		list = objtypesinks;
		while(list) {
			sink = (OSyncObjTypeSink*)list->data;

			if (!connect_sink(cmd, sink, error))
				goto error;
			list = list->next;
		}

		/* last but not least - the main sink */
		if (get_main_sink())
			if (!connect_sink(cmd, get_main_sink(), error))
				goto error;
		
		osync_list_free(objtypesinks);
	}

	return TRUE;
 error:
	osync_list_free(objtypesinks);
	return FALSE;
}

static void _osyncplugin_ctx_callback_disconnect(void *user_data, OSyncError *error)
{
	OSyncError *locerror = NULL;
	Command *cmd = NULL;
	OSyncObjTypeSink *sink = NULL;

	assert(user_data);

	cmd = (Command *) user_data;		
	sink = cmd->sink;

	if (error) {
		osync_error_set_from_error(&locerror, &error);
		goto error;
	}

	if (osync_objtype_sink_get_name(sink))
		printf("Sink \"%s\"", osync_objtype_sink_get_name(sink));
	else
		printf("Main Sink");

	printf(" disconnected.\n");

	g_mutex_lock(cmd->mutex);
	g_cond_signal(cmd->cond);
	cmd->done = TRUE;
	g_mutex_unlock(cmd->mutex);

	return;
 error:
	fprintf(stderr, "ERROR: %s\n", osync_error_print(&locerror));
	return;
}

static osync_bool disconnect_sink(Command *cmd, OSyncObjTypeSink *sink, OSyncError **error) {
	OSyncContext *context = osync_context_new(error);
	assert(sink);
	assert(cmd);

	if (!context)
		goto error;

	cmd->sink = sink;

	osync_context_set_callback(context, _osyncplugin_ctx_callback_disconnect, cmd);

	osync_plugin_info_set_sink(plugin_info, sink);

	osync_objtype_sink_disconnect(sink, plugin_info, context);

	osync_context_unref(context);

	return TRUE;

 error:	
	return FALSE;
}

static osync_bool disconnect(Command *cmd, OSyncError **error)
{
	OSyncList *list;
	OSyncList *objtypesinks = NULL;
	OSyncObjTypeSink *sink = NULL;
	const char *objtype = cmd->arg;

	if (objtype) {
		sink = find_sink(objtype, error);
		if (!sink)
			goto error;

		if (!disconnect_sink(cmd, sink, error))
			goto error;
	} else {
		objtypesinks = osync_plugin_info_get_objtype_sinks(plugin_info);
		list = objtypesinks;
		while(list) {
			sink = (OSyncObjTypeSink*)list->data;

			if (!disconnect_sink(cmd, sink, error))
				goto error;
			list = list->next;
		}

		/* last but not least - the main sink */
		if (get_main_sink())
			if (!disconnect_sink(cmd, get_main_sink(), error))
				goto error;
		
		osync_list_free(objtypesinks);
	}

	return TRUE;
 error:
	osync_list_free(objtypesinks);
	return FALSE;
}

static void _osyncplugin_ctx_callback_commit_change(void *user_data, OSyncError *error)
{
	OSyncError *locerror = NULL;
	Command *cmd = NULL;
	OSyncObjTypeSink *sink = NULL;

	assert(user_data);

	cmd = (Command *) user_data;		
	sink = cmd->sink;

	if (error) {
		osync_error_set_from_error(&locerror, &error);
		goto error;
	}

	g_mutex_lock(cmd->mutex);
	g_cond_signal(cmd->cond);
	cmd->done = TRUE;
	g_mutex_unlock(cmd->mutex);

	return;
 error:
	fprintf(stderr, "ERROR for sink \"%s\": %s\n", osync_objtype_sink_get_name(sink), osync_error_print(&locerror));
	return;

}

static osync_bool commit_sink(Command *cmd, OSyncObjTypeSink *sink, OSyncChange *change, OSyncError **error) {
	OSyncContext *context = NULL;
	assert(sink);
	assert(change);

	context = osync_context_new(error);
	if (!context)
		goto error;

	cmd->sink = sink;

	osync_context_set_callback(context, _osyncplugin_ctx_callback_commit_change, cmd);

	osync_plugin_info_set_sink(plugin_info, sink);

	printf("COMMIT:\t%s\t%s\t%s\n", 
	       _osyncplugin_changetype_str(change), 
	       osync_data_get_objtype(osync_change_get_data(change)), 
	       osync_change_get_uid(change));

	osync_objtype_sink_commit_change(sink, plugin_info, change, context);

	osync_context_unref(context);

	return TRUE;

 error:	
	return FALSE;
}

static osync_bool commit(Command *cmd, OSyncChange *change, OSyncError **error)
{
	OSyncList *list;
	OSyncList *objtypesinks = NULL;
	OSyncObjTypeSink *sink = NULL;
	const char *objtype = cmd->arg;

	assert(change);

	if (objtype) {
		sink = find_sink(objtype, error);
		if (!sink)
			goto error;

		if (!commit_sink(cmd, sink, change, error))
			goto error;
	} else {
		objtypesinks = osync_plugin_info_get_objtype_sinks(plugin_info);
		list = objtypesinks;
		while(list){
			sink = (OSyncObjTypeSink*)list->data;

			if (!commit_sink(cmd, sink, change, error))
				goto error;
			list = list->next;
		}

		/* last but not least - the main sink */
		if (get_main_sink())
			if (!commit_sink(cmd, get_main_sink(), change, error))
				goto error;
		
		osync_list_free(objtypesinks);
	}
	
	return TRUE;
 error:
	osync_list_free(objtypesinks);
	return FALSE;
}

static osync_bool empty(Command *cmd, OSyncError **error)
{
	int i;
	GList *c;
	//const char *objtype = cmd->arg;
	
	/* Perform slowync - if objtype is set for this objtype, otherwise slowsync for ALL */
	if (!get_changes(cmd, SYNCTYPE_FORCE_SLOWSYNC, error))
		goto error;


	for (i=0, c = changesList; c; c = c->next, i++) {
		OSyncChange *change = c->data;
		osync_change_set_changetype(change, OSYNC_CHANGE_TYPE_DELETED);

		if (!commit(cmd, change, error))
			goto error;

	}

	return TRUE;

 error:
	return FALSE;
}

static void _osyncplugin_ctx_callback_syncdone(void *user_data, OSyncError *error)
{
	OSyncError *locerror = NULL;
	Command *cmd = (Command *) user_data;		
	OSyncObjTypeSink *sink = NULL;

	assert(user_data);

	sink = cmd->sink;

	if (error) {
		osync_error_set_from_error(&locerror, &error);
		goto error;
	}

	g_mutex_lock(cmd->mutex);
	g_cond_signal(cmd->cond);
	cmd->done = TRUE;
	g_mutex_unlock(cmd->mutex);

	return;
 error:
	fprintf(stderr, "ERROR for sink \"%s\": %s\n", osync_objtype_sink_get_name(sink), osync_error_print(&locerror));
	return;
}

static osync_bool syncdone_sink(Command *cmd, OSyncObjTypeSink *sink, OSyncError **error) {
	OSyncContext *context = NULL;
	assert(sink);
	assert(cmd);

	context = osync_context_new(error);
	if (!context)
		goto error;

	cmd->sink = sink;

	osync_context_set_callback(context, _osyncplugin_ctx_callback_syncdone, cmd);

	osync_plugin_info_set_sink(plugin_info, sink);

	osync_objtype_sink_sync_done(sink, plugin_info, context);

	osync_context_unref(context);

	if (!osync_objtype_sink_save_hashtable(sink, error))
		goto error;

	return TRUE;

 error:	
	return FALSE;
}

static osync_bool syncdone(Command *cmd, OSyncError **error)
{
	
	OSyncList *objtypesinks = NULL;
	OSyncList *list;
	OSyncObjTypeSink *sink = NULL;
	const char *objtype = cmd->arg;

	if (objtype) {
		sink = find_sink(objtype, error);
		if (!sink)
			goto error;

		if (!syncdone_sink(cmd, sink, error))
			goto error;
	} else {
		objtypesinks = osync_plugin_info_get_objtype_sinks(plugin_info);
		list = objtypesinks;
		while(list) {
			sink = (OSyncObjTypeSink*)list->data;

			if (!syncdone_sink(cmd, sink, error))
				goto error;
			
			list = list->next;
		}

		/* last but not least - the main sink */
		if (get_main_sink())
			if (!syncdone_sink(cmd, get_main_sink(), error))
				goto error;
		
		osync_list_free(objtypesinks);
	}

	return TRUE;
 error:
	osync_list_free(objtypesinks);
	return FALSE;
}

static void _osyncplugin_ctx_callback_committedall(void *user_data, OSyncError *error)
{
	OSyncError *locerror = NULL;
	Command *cmd = (Command *) user_data;		

	assert(user_data);

	//OSyncObjTypeSink *sink = cmd->sink;

	if (error) {
		osync_error_set_from_error(&locerror, &error);
		goto error;
	}

	g_mutex_lock(cmd->mutex);
	g_cond_signal(cmd->cond);
	cmd->done = TRUE;
	g_mutex_unlock(cmd->mutex);

	return;
 error:
	fprintf(stderr, "ERROR: %s\n", osync_error_print(&locerror));
	return;
}

static osync_bool committedall_sink(Command *cmd, OSyncObjTypeSink *sink, OSyncError **error) {
	OSyncContext *context = NULL;
	assert(sink);
	assert(cmd);

	context = osync_context_new(error);
	if (!context)
		goto error;
	
	cmd->sink = sink; 

	osync_context_set_callback(context, _osyncplugin_ctx_callback_committedall, cmd);

	osync_plugin_info_set_sink(plugin_info, sink);

	osync_objtype_sink_committed_all(sink, plugin_info, context);

	osync_context_unref(context);

	return TRUE;

 error:	
	return FALSE;
}

static osync_bool committedall(Command *cmd, OSyncError **error)
{
	OSyncList *list;
	OSyncList *objtypesinks = NULL;
	OSyncObjTypeSink *sink = NULL;
	const char *objtype = cmd->arg;

	if (objtype) {
		sink = find_sink(objtype, error);
		if (!sink)
			goto error;

		if (!committedall_sink(cmd, sink, error))
			goto error;
	} else {
		objtypesinks = osync_plugin_info_get_objtype_sinks(plugin_info);
		list = objtypesinks;
		while(list) {
			sink = (OSyncObjTypeSink*)list->data;

			if (!committedall_sink(cmd, sink, error))
				goto error;
			
			list = list->next;
		}

		/* last but not least - the main sink */
		if (get_main_sink())
			if (!committedall_sink(cmd, get_main_sink(), error))
				goto error;
		
		osync_list_free(objtypesinks);
	}

	return TRUE;
 error:
	osync_list_free(objtypesinks);
	return FALSE;
}

/*
 * Sync Flow
 */
static osync_bool run_command(Command *cmd, void **plugin_data, OSyncError **error) {

	assert(cmd);

	if (cmd->cmd != CMD_INITIALIZE && *plugin_data == NULL) {
		fprintf(stderr, "WARNING: Got Plugin initialized? plugin_data is NULL.\n");
		goto error;
	}


	switch (cmd->cmd) {
	case CMD_EMPTY:
		if (!empty(cmd, error))
			goto error;
		break;
	case CMD_INITIALIZE:
		if (!(*plugin_data = plugin_initialize(error)))
			goto error;
		break;
	case CMD_FINALIZE:
		finalize_plugin(plugin_data);
		break;
	case CMD_CONNECT:
		if (!connect_plugin(cmd, error))
			goto error;
		break;
	case CMD_DISCONNECT:
		if (!disconnect(cmd, error))
			goto error;
		break;
	case CMD_SLOWSYNC:
		if (!get_changes(cmd, SYNCTYPE_FORCE_SLOWSYNC, error))
			goto error;
		break;
	case CMD_FASTSYNC:
		if (!get_changes(cmd, SYNCTYPE_FORCE_FASTSYNC, error))
			goto error;
		break;
	case CMD_SYNC:
		if (!get_changes(cmd, SYNCTYPE_NORMAL, error))
			goto error;
		break;
	case CMD_COMMIT:
		fprintf(stderr, "COMMIT not yet implemented\n");
		break;
	case CMD_COMMITTEDALL:
		if (!committedall(cmd, error))
			goto error;
		break;
	case CMD_READ:
		fprintf(stderr, "READ not yet implemented\n");
		break;
	case CMD_WRITE:
		fprintf(stderr, "WRITE not yet implemented\n");
		break;
	case CMD_SYNCDONE:
		if (!syncdone(cmd, error))
			goto error;
		break;
	case CMD_DISCOVER:
		fprintf(stderr, "DISCOVER not yet implemented\n");
		break;
	}


	printf("%s: %u - sink: %p\n", __func__, cmd->cmd, cmd->sink);
	switch (cmd->cmd) {
	case CMD_INITIALIZE:
	case CMD_FINALIZE:
	case CMD_DISCOVER:
		break;

	case CMD_EMPTY:
	case CMD_CONNECT:
	case CMD_DISCONNECT:
	case CMD_SLOWSYNC:
	case CMD_FASTSYNC:
	case CMD_SYNC:
	case CMD_COMMIT:
	case CMD_COMMITTEDALL:
	case CMD_READ:
	case CMD_WRITE:
	case CMD_SYNCDONE:
		printf("waiting .....\n");
		if (!cmd->done)
			g_cond_wait(cmd->cond, cmd->mutex);
		printf("DONE\n");
		break;
	}

	return TRUE;

 error:
	return FALSE;
}

static osync_bool plugin_list(OSyncError **error) {
	OSyncList *plugins;
	OSyncList *list;
	
	assert(!plugin_env);

	if (!(plugin_env = osync_plugin_env_new(error)))
		goto error;

	if (!(format_env = osync_format_env_new(error)))
		goto error_free_pluginenv;

	if (!osync_format_env_load_plugins(format_env, formatpath, error))
		goto error_free_formatenv;

	if (!osync_plugin_env_load(plugin_env, pluginpath, error))
		goto error_free_formatenv;

	plugins = osync_plugin_env_get_plugins(plugin_env);
	list = plugins;
	while(list) {
		OSyncPlugin* plugin = (OSyncPlugin*)list->data;
		fprintf (stdout, "Name:        %s\n", osync_plugin_get_name(plugin));
		fprintf (stdout, "Description: %s\n", osync_plugin_get_description(plugin));
		list = list->next;
	}
	osync_list_free(plugins);
	return TRUE;
	
 error_free_formatenv:
	osync_format_env_unref(format_env);
	format_env = NULL;
 error_free_pluginenv:
	osync_plugin_env_unref(plugin_env);
	plugin_env = NULL;
 error:	
	return FALSE;
}

int main(int argc, char **argv) {

	GList *o;
	void *plugin_data = NULL;
	OSyncError *error = NULL;

	if (!g_thread_supported())
		g_thread_init(NULL);

	parse_args(argc, argv);
	/* Set defaults if not set on the command line */
	if (!syncgroup) 
		syncgroup = strdup("osyncplugin");
	
	if (pluginlist) {
		if (!plugin_list(&error))
			goto error;
			
		goto success;
	}

	if (!init(&error))
		goto error;

	for (o=cmdlist; o; o = o->next)
		if (!run_command((Command *) o->data, &plugin_data, &error))
			goto error_disconnect_and_finalize;


 success:
	if (plugin_env)
		osync_plugin_env_unref(plugin_env);

	for (o=cmdlist; o; o = o->next) {
		Command *cmd = o->data;
		free_command(&cmd);
	}


	return EXIT_SUCCESS;

 error_disconnect_and_finalize:
	if (plugin_data)
		disconnect(NULL, NULL);
	//error_finalize:
	finalize_plugin(&plugin_data);
	//error_free_plugin_env:
	if (plugin_env)
		osync_plugin_env_unref(plugin_env);

	for (o=cmdlist; o; o = o->next) {
		Command *cmd = o->data;
		free_command(&cmd);
	}

 error:	
	fprintf(stderr, "Error: %s\n", osync_error_print(&error));
	osync_error_unref(&error);
	return EXIT_FAILURE;
}

