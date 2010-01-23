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

#include "opensync-data.h"

#include "opensync-ipc.h"
#include "ipc/opensync_message_internals.h"
#include "ipc/opensync_serializer_internals.h"
#include "ipc/opensync_queue_internals.h"
#include "plugin/opensync_objtype_sink_internals.h"

#include "opensync-capabilities.h"
#include "capabilities/opensync_capabilities_internals.h"

#include "opensync-group.h"
#include "opensync-plugin.h"
#include "opensync-format.h"

#include "opensync-version.h"
#include "version/opensync_version_internals.h"

#include "opensync-client.h"
#include "opensync_client_internals.h"

#ifndef _WIN32
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#endif /* _WIN32 */

#ifdef _WIN32
/* Historical signals specified by POSIX. */
#define SIGKILL 9	/* Kill (cannot be blocked, caught, or ignored). */
#define SIGTERM 15	/* can be caught and interpreted or ignored by the process */
typedef int pid_t;
#endif //_WIN32

#include "opensync_client_proxy_internals.h"
#include "opensync_client_proxy_private.h"

typedef struct callContext {
	OSyncClientProxy *proxy;
	
	initialize_cb init_callback;
	void *init_callback_data;
	
	finalize_cb fin_callback;
	void *fin_callback_data;
	
	discover_cb discover_callback;
	void *discover_callback_data;
	
	connect_cb connect_callback;
	void *connect_callback_data;

	connect_done_cb connect_done_callback;
	void *connect_done_callback_data;
	
	disconnect_cb disconnect_callback;
	void *disconnect_callback_data;

	read_cb read_callback;
	void *read_callback_data;
	
	get_changes_cb get_changes_callback;
	void *get_changes_callback_data;
	
	commit_change_cb commit_change_callback;
	void *commit_change_callback_data;
	
	committed_all_cb committed_all_callback;
	void *committed_all_callback_data;
	
	sync_done_cb sync_done_callback;
	void *sync_done_callback_data;
} callContext;

//portable kill pid helper
/*static int _osync_kill(pid_t pid, int sig) 
	{
	#ifndef _WIN32
	return kill(pid, sig);
	#else //_WIN32
	int ret = -1;
	DWORD dwExitCode = 0;

	HANDLE hProc = OpenProcess(1, 0, pid);
	GenerateConsoleCtrlEvent(CTRL_C_EVENT, pid);
	WaitForSingleObject(hProc, 3000);
	GetExitCodeProcess(hProc, &dwExitCode);
	
	if(dwExitCode != STILL_ACTIVE) {
	ret = 0;
	goto end;
	}
	if (sig == SIGKILL) {
	if (TerminateProcess(hProc, 0))
	ret = 0;
	else
	ret = -1;
	}
	end:
	CloseHandle(hProc);
	return (ret);
	#endif //_WIN32
	}*/

/*static char *_osync_client_pid_filename(OSyncClientProxy *proxy)
	{
	return g_strdup_printf("%s%cosplugin.pid", proxy->path, G_DIR_SEPARATOR);
	}*/

/*static osync_bool osync_client_remove_pidfile(OSyncClientProxy *proxy, OSyncError **error)
	{
	char *pidpath = _osync_client_pid_filename(proxy);

	if (unlink(pidpath) < 0) {
	osync_error_set(error, OSYNC_ERROR_GENERIC, "Couldn't remove pid file: %s", g_strerror(errno));
	g_free(pidpath);
	return FALSE;
	}
	
	g_free(pidpath);
	return TRUE;
	}*/

/*static osync_bool _osync_client_create_pidfile(OSyncClientProxy *proxy, OSyncError **error)
	{
	char *pidpath = _osync_client_pid_filename(proxy);
	char *pidstr = g_strdup_printf("%ld", (long)proxy->child_pid);

	if (!osync_file_write(pidpath, pidstr, strlen(pidstr), 0644, error)) {
	g_free(pidstr);
	g_free(pidpath);
	return FALSE;
	}

	g_free(pidstr);
	g_free(pidpath);
	return TRUE;
	}*/

/*static osync_bool _osync_client_kill_old_osplugin(OSyncClientProxy *proxy, OSyncError **error)
	{
	osync_bool ret = FALSE;

	char *pidstr;
	unsigned int pidlen;
	pid_t pid;

	char *pidpath = _osync_client_pid_filename(proxy);

	// Simply returns if there is no PID file
	if (!g_file_test(pidpath, G_FILE_TEST_EXISTS)) {
	ret = TRUE;
	goto out_free_path;
	}

	if (!osync_file_read(pidpath, &pidstr, &pidlen, error))
	goto out_free_path;

	pid = atol(pidstr);
	if (!pid)
	goto out_free_str;

	osync_trace(TRACE_INTERNAL, "Killing old osplugin process. PID: %ld", (long)pid);

	if (_osync_kill(pid, SIGTERM) < 0) {
	osync_trace(TRACE_INTERNAL, "Error killing old osplugin: %s. Stale pid file?", g_strerror(errno));
	// Don't return failure if kill() failed, because it may be a stale pid file
	}

	int count = 0;
	while (osync_queue_is_alive(proxy->outgoing)) {
	if (count++ > 10) {
	osync_trace(TRACE_INTERNAL, "Killing old osplugin process with SIGKILL");
	_osync_kill(pid, SIGKILL);
	break;
	}
	osync_trace(TRACE_INTERNAL, "Waiting for other side to terminate");
	// FIXME: Magic numbers are evil
	g_usleep(500000);
	}

	if (unlink(pidpath) < 0) {
	osync_error_set(error, OSYNC_ERROR_GENERIC, "Couldn't erase PID file: %s", g_strerror(errno));
	goto out_free_str;
	}

	// Success
	ret = TRUE;

	out_free_str:
	g_free(pidstr);
	out_free_path:
	g_free(pidpath);
	//out:
	return ret;
	}*/


/** This function takes care of the messages received on the outgoing (sending)
 * queue. The only messages we can receive there, are HUPs or ERRORs. */
static void _osync_client_proxy_hup_handler(OSyncMessage *message, void *user_data)
{
	OSyncClientProxy *proxy = user_data;
	OSyncError *error = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);

	osync_trace(TRACE_INTERNAL, "client received command %i on sending queue", osync_message_get_command(message));

	if ( (osync_message_get_command(message) == OSYNC_MESSAGE_QUEUE_ERROR) /* Treat an error as a disconnect */
	     || (osync_message_get_command(message) == OSYNC_MESSAGE_QUEUE_HUP) ) {
		/* The remote side disconnected. So we can now disconnect as well and then
		 * shutdown */
		if (!osync_queue_disconnect(proxy->outgoing, &error))
			osync_error_unref(&error);
		
		if (!osync_queue_disconnect(proxy->incoming, &error))
			osync_error_unref(&error);
		
	} else {
		/* This should never ever happen */
		osync_trace(TRACE_ERROR, "received neither a hup, nor a error on a sending queue...");
	}

	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
}

static void _osync_client_proxy_init_handler(OSyncMessage *message, void *user_data)
{
	callContext *ctx = user_data;
	OSyncClientProxy *proxy = ctx->proxy;
	OSyncError *error = NULL;
	OSyncError *locerror = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);
	
	if (osync_message_get_cmd(message) == OSYNC_MESSAGE_REPLY) {

		if (!osync_demarshal_objtype_sinks(message, proxy, &locerror))
			goto error;

		ctx->init_callback(proxy, ctx->init_callback_data, NULL);
	} else if (osync_message_get_cmd(message) == OSYNC_MESSAGE_ERRORREPLY) {

		if (!osync_demarshal_error(message, &error, &locerror))
			goto error;

		ctx->init_callback(proxy, ctx->init_callback_data, error);
		osync_error_unref(&error);
	} else {
		osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "Unexpected reply");
		goto error;
	}
	
	osync_free(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
	
 error:
	ctx->init_callback(proxy, ctx->init_callback_data, locerror);
	osync_free(ctx);
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(&locerror));
	osync_error_unref(&locerror);
	return;
}

static void _osync_client_proxy_fin_handler(OSyncMessage *message, void *user_data)
{
	callContext *ctx = user_data;
	OSyncClientProxy *proxy = ctx->proxy;
	OSyncError *error = NULL;
	OSyncError *locerror = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);
	
	if (osync_message_get_cmd(message) == OSYNC_MESSAGE_REPLY) {
		ctx->fin_callback(proxy, ctx->fin_callback_data, NULL);
	} else if (osync_message_get_cmd(message) == OSYNC_MESSAGE_ERRORREPLY) {

		if (osync_demarshal_error(message, &error, &locerror))
			goto error;

		ctx->fin_callback(proxy, ctx->fin_callback_data, error);
		osync_error_unref(&error);
	} else {
		osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "Unexpected reply");
		goto error;
	}
	
	osync_free(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
	
 error:
	ctx->fin_callback(proxy, ctx->fin_callback_data, locerror);
	osync_free(ctx);
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(&locerror));
	osync_error_unref(&locerror);
	return;
}

static osync_bool _osync_client_proxy_read_discover_message(OSyncClientProxy *proxy, OSyncMessage *message, OSyncError **error)
{
	
	char* str = NULL;
	int sent_version = 0;
	int sent_capabilities = 0;
	OSyncVersion *version = NULL;
	OSyncCapabilities *capabilities = NULL;
	OSyncCapabilities *version_cap = NULL; 
	OSyncMember *member = osync_client_proxy_get_member(proxy);


	/* Merger - Set the capabilities */
	if (!osync_message_read_int(message, &sent_version, error))
		goto error;

	if (sent_version) {
		version = osync_version_new(error);
		if (!version)
			goto error;
		
		osync_message_read_string(message, &str, error);
		osync_version_set_plugin(version, str);
		osync_free(str);
		osync_message_read_string(message, &str, error);
		osync_version_set_priority(version, str);
		osync_free(str);
		osync_message_read_string(message, &str, error);
		osync_version_set_vendor(version, str);
		osync_free(str);
		osync_message_read_string(message, &str, error);
		osync_version_set_modelversion(version, str);
		osync_free(str);
		osync_message_read_string(message, &str, error);
		osync_version_set_firmwareversion(version, str);
		osync_free(str);
		osync_message_read_string(message, &str, error);
		osync_version_set_softwareversion(version, str);
		osync_free(str);
		osync_message_read_string(message, &str, error);
		osync_version_set_hardwareversion(version, str);
		osync_free(str);
		osync_message_read_string(message, &str, error);
		osync_version_set_identifier(version, str);
		osync_free(str);	

		if (osync_error_is_set(error))
			goto error_free_version;
	}
			
	if (!osync_message_read_int(message, &sent_capabilities, error))
		goto error;

	if (sent_capabilities) {
		if (!osync_message_read_string(message, &str, error))
			goto error_free_version;

		capabilities = osync_capabilities_parse(str, strlen(str), error);
		osync_free(str);
		if (!capabilities)
			goto error_free_version;
	}
	
	/* we set the capabilities for the member only if they are not set yet */
	if (member && osync_member_get_capabilities(member) == NULL) {
		osync_trace(TRACE_INTERNAL, "No capabilities set for the member right now. version: %p capabilities: %p\n", version, capabilities);

		/* we take our own capabilities rather then from the client */ 
		if (version)
			version_cap = osync_version_find_capabilities(version, error);

		if (*error)
			goto error_free_capabilities;

		if (version_cap) {
			if (capabilities)
				osync_capabilities_unref(capabilities);
			capabilities = version_cap;
		}

		if (capabilities) {
			if (!osync_member_set_capabilities(member, capabilities, error))
				goto error_free_capabilities; 

			osync_capabilities_unref(capabilities);
		}
	}

	if (version)
		osync_version_unref(version);

	return TRUE;

error_free_capabilities:
	if (capabilities)
		osync_capabilities_unref(capabilities);
error_free_version:
	if (version)
		osync_version_unref(version);

error:	

	return FALSE;
}

static void _osync_client_proxy_discover_handler(OSyncMessage *message, void *user_data)
{
	callContext *ctx = user_data;
	OSyncClientProxy *proxy = ctx->proxy;
	OSyncError *error = NULL;
	OSyncError *locerror = NULL;
	unsigned int i = 0;
	OSyncMember *member = osync_client_proxy_get_member(proxy);
	unsigned int num_res = 0;
	OSyncPluginConfig *config = NULL;
	OSyncPluginResource *resource = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);
	
	if (osync_message_get_cmd(message) == OSYNC_MESSAGE_REPLY) {
		
		osync_message_read_int(message, &proxy->has_main_sink, &locerror);
		

		if (osync_error_is_set(&locerror))
			goto error;
		
		if (!osync_demarshal_objtype_sinks(message, proxy, &locerror))
			goto error;

		if (!_osync_client_proxy_read_discover_message(proxy, message, &locerror))
			goto error;
	 
		/* Store stuff in member configuration */
		if (member) {

			/* Demarshal discovered resources */
			config = osync_member_get_config(member, &locerror);
			if (!config)
				goto error;

			osync_plugin_config_flush_resources(config);

			if (!osync_message_read_uint(message, &num_res, &locerror))
				goto error;

			for (i=0; i < num_res; i++) {
				if (!osync_demarshal_pluginresource(message, &resource, &locerror))
					goto error;

				osync_plugin_config_add_resource(config, resource);
				osync_plugin_resource_unref(resource);
			}

			if (!osync_member_save(member, &locerror))
				goto error;
		}

		ctx->discover_callback(proxy, ctx->discover_callback_data, NULL);
	} else if (osync_message_get_cmd(message) == OSYNC_MESSAGE_ERRORREPLY) {

		if (!osync_demarshal_error(message, &error, &locerror))
			goto error;

		ctx->discover_callback(proxy, ctx->discover_callback_data, error);
		osync_error_unref(&error);
	} else {
		osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "Unexpected reply");
		goto error;
	}
	
	osync_free(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;

 error:
	ctx->discover_callback(proxy, ctx->discover_callback_data, locerror);
	osync_free(ctx);
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(&locerror));
	osync_error_unref(&locerror);
	return;
}

static void _osync_client_proxy_connect_handler(OSyncMessage *message, void *user_data)
{
	int slowsync;
	callContext *ctx = user_data;
	OSyncClientProxy *proxy = ctx->proxy;
	OSyncError *error = NULL;
	OSyncError *locerror = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);
	
	if (osync_message_get_cmd(message) == OSYNC_MESSAGE_REPLY) {

		if (!osync_message_read_int(message, &slowsync, &locerror))
			goto error;

		ctx->connect_callback(proxy, ctx->connect_callback_data, slowsync, NULL);
	} else if (osync_message_get_cmd(message) == OSYNC_MESSAGE_ERRORREPLY) {

		if (!osync_demarshal_error(message, &error, &locerror))
			goto error;

		ctx->connect_callback(proxy, ctx->connect_callback_data, FALSE, error);
		osync_error_unref(&error);
	} else {
		osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "Unexpected reply");
		goto error;
	}
	
	osync_free(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
	
 error:
	ctx->connect_callback(proxy, ctx->connect_callback_data, FALSE, locerror);
	osync_free(ctx);
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(&locerror));
	osync_error_unref(&locerror);
	return;
}

static void _osync_client_proxy_connect_done_handler(OSyncMessage *message, void *user_data)
{
	callContext *ctx = user_data;
	OSyncClientProxy *proxy = ctx->proxy;
	OSyncError *error = NULL;
	OSyncError *locerror = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);
	
	if (osync_message_get_cmd(message) == OSYNC_MESSAGE_REPLY) {
		ctx->connect_done_callback(proxy, ctx->connect_done_callback_data, NULL);
	} else if (osync_message_get_cmd(message) == OSYNC_MESSAGE_ERRORREPLY) {

		if (!osync_demarshal_error(message, &error, &locerror))
			goto error;

		ctx->connect_done_callback(proxy, ctx->connect_done_callback_data, error);
		osync_error_unref(&error);
	} else {
		osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "Unexpected reply");
		goto error;
	}
	
	osync_free(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
	
 error:
	ctx->connect_done_callback(proxy, ctx->connect_done_callback_data, locerror);
	osync_free(ctx);
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(&locerror));
	osync_error_unref(&locerror);
	return;
}

static void _osync_client_proxy_disconnect_handler(OSyncMessage *message, void *user_data)
{
	callContext *ctx = user_data;
	OSyncClientProxy *proxy = ctx->proxy;
	OSyncError *error = NULL;
	OSyncError *locerror = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);
	
	if (osync_message_get_cmd(message) == OSYNC_MESSAGE_REPLY) {
		ctx->disconnect_callback(proxy, ctx->disconnect_callback_data, NULL);
	} else if (osync_message_get_cmd(message) == OSYNC_MESSAGE_ERRORREPLY) {

		if (!osync_demarshal_error(message, &error, &locerror))
			goto error;

		ctx->disconnect_callback(proxy, ctx->disconnect_callback_data, error);
		osync_error_unref(&error);
	} else {
		osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "Unexpected reply");
		goto error;
	}
	
	osync_free(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
	
 error:
	ctx->disconnect_callback(proxy, ctx->disconnect_callback_data, locerror);
	osync_free(ctx);
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(&locerror));
	osync_error_unref(&locerror);
	return;
}

static void _osync_client_proxy_read_handler(OSyncMessage *message, void *user_data)
{
	callContext *ctx = user_data;
	OSyncClientProxy *proxy = ctx->proxy;
	OSyncError *error = NULL;
	OSyncError *locerror = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);
	
	if (osync_message_get_cmd(message) == OSYNC_MESSAGE_REPLY) {
		ctx->read_callback(proxy, ctx->read_callback_data, NULL);
	} else if (osync_message_get_cmd(message) == OSYNC_MESSAGE_ERRORREPLY) {
		
		if (!osync_demarshal_error(message, &error, &locerror))
			goto error;

		ctx->read_callback(proxy, ctx->read_callback_data, error);
		osync_error_unref(&error);
	} else {
		osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "Unexpected reply");
		goto error;
	}
	
	osync_free(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
	
 error:
	ctx->read_callback(proxy, ctx->read_callback_data, locerror);
	osync_free(ctx);
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(&locerror));
	osync_error_unref(&locerror);
	return;
}

static void _osync_client_proxy_get_changes_handler(OSyncMessage *message, void *user_data)
{
	callContext *ctx = user_data;
	OSyncClientProxy *proxy = ctx->proxy;
	OSyncError *error = NULL;
	OSyncError *locerror = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);
	
	if (osync_message_get_cmd(message) == OSYNC_MESSAGE_REPLY) {
		ctx->get_changes_callback(proxy, ctx->get_changes_callback_data, NULL);

		if (osync_message_get_message_size(message)) {
			if (!_osync_client_proxy_read_discover_message(proxy, message, &locerror))
				goto error;
		}

	} else if (osync_message_get_cmd(message) == OSYNC_MESSAGE_ERRORREPLY) {

		if (!osync_demarshal_error(message, &error, &locerror))
			goto error;

		ctx->get_changes_callback(proxy, ctx->get_changes_callback_data, error);
		osync_client_proxy_set_error(proxy, error);
		osync_error_unref(&error);
	} else {
		osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "Unexpected reply");
		goto error;
	}
	
	osync_free(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
	
 error:
	ctx->get_changes_callback(proxy, ctx->get_changes_callback_data, locerror);
	osync_free(ctx);
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(&locerror));
	osync_error_unref(&locerror);
	return;
}

static void _osync_client_proxy_commit_change_handler(OSyncMessage *message, void *user_data)
{
	callContext *ctx = user_data;
	OSyncClientProxy *proxy = ctx->proxy;
	OSyncError *error = NULL;
	OSyncError *locerror = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);
	
	if (osync_message_get_cmd(message) == OSYNC_MESSAGE_REPLY) {
		char *uid = NULL;

		if (!osync_message_read_string(message, &uid, &locerror))
			goto error;

		ctx->commit_change_callback(proxy, ctx->commit_change_callback_data, uid, NULL);
		osync_free(uid);
	} else if (osync_message_get_cmd(message) == OSYNC_MESSAGE_ERRORREPLY) {

		if (!osync_demarshal_error(message, &error, &locerror))
			goto error;

		ctx->commit_change_callback(proxy, ctx->commit_change_callback_data, NULL, error);
		osync_error_unref(&error);
	} else {
		osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "Unexpected reply");
		goto error;
	}
	
	osync_free(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
	
 error:
	ctx->commit_change_callback(proxy, ctx->commit_change_callback_data, NULL, locerror);
	osync_free(ctx);
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(&locerror));
	osync_error_unref(&locerror);
	return;
}

static void _osync_client_proxy_committed_all_handler(OSyncMessage *message, void *user_data)
{
	callContext *ctx = user_data;
	OSyncClientProxy *proxy = ctx->proxy;
	OSyncError *error = NULL;
	OSyncError *locerror = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);
	
	if (osync_message_get_cmd(message) == OSYNC_MESSAGE_REPLY) {
		ctx->committed_all_callback(proxy, ctx->committed_all_callback_data, NULL);
	} else if (osync_message_get_cmd(message) == OSYNC_MESSAGE_ERRORREPLY) {

		if (!osync_demarshal_error(message, &error, &locerror))
			goto error;

		ctx->committed_all_callback(proxy, ctx->committed_all_callback_data, error);
		osync_error_unref(&error);
	} else {
		osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "Unexpected reply");
		goto error;
	}
	
	osync_free(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
	
 error:
	ctx->committed_all_callback(proxy, ctx->committed_all_callback_data, locerror);
	osync_free(ctx);
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(&locerror));
	osync_error_unref(&locerror);
	return;
}

static void _osync_client_proxy_sync_done_handler(OSyncMessage *message, void *user_data)
{
	callContext *ctx = user_data;
	OSyncClientProxy *proxy = ctx->proxy;
	OSyncError *error = NULL;
	OSyncError *locerror = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);
	
	if (osync_message_get_cmd(message) == OSYNC_MESSAGE_REPLY) {
		ctx->sync_done_callback(proxy, ctx->sync_done_callback_data, NULL);
	} else if (osync_message_get_cmd(message) == OSYNC_MESSAGE_ERRORREPLY) {

		if (!osync_demarshal_error(message, &error, &locerror))
			goto error;

		ctx->sync_done_callback(proxy, ctx->sync_done_callback_data, error);
		osync_error_unref(&error);
	} else {
		osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "Unexpected reply");
		goto error;
	}
	
	osync_free(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
	
 error:
	ctx->sync_done_callback(proxy, ctx->sync_done_callback_data, locerror);
	osync_free(ctx);
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(&locerror));
	osync_error_unref(&locerror);
	return;
}

static void _osync_client_proxy_message_handler(OSyncMessage *message, void *user_data)
{
	OSyncClientProxy *proxy = user_data;
	OSyncError *error = NULL;
	OSyncChange *change = NULL;
	char *objtype = NULL, *olduid = NULL, *newuid = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);
	
	osync_trace(TRACE_INTERNAL, "proxy received command %i", osync_message_get_command(message));
	switch (osync_message_get_command(message)) {
	case OSYNC_MESSAGE_NEW_CHANGE:
	case OSYNC_MESSAGE_READ_CHANGE:

		osync_assert(proxy->change_callback);

		if (proxy->error) {
			osync_trace(TRACE_INTERNAL, "WARNING: Proxy error taintend! Ignoring incoming changes!");
			break;
		}
			
		if (!osync_demarshal_change(message, &change, proxy->formatenv, &error))
			goto error;
			
		proxy->change_callback(proxy, proxy->change_callback_data, change);
			
		osync_change_unref(change);
		break;

	case OSYNC_MESSAGE_MAPPING_CHANGED:

		osync_assert(proxy->uid_update_callback);

		if (proxy->error) {
			osync_trace(TRACE_INTERNAL, "WARNING: Proxy error taintend! Ignoring incoming changes!");
			break;
		}

		if (!osync_message_read_string(message, &objtype, &error))
			goto error;

		if (!osync_message_read_string(message, &olduid, &error))
			goto error;

		if (!osync_message_read_string(message, &newuid, &error))
			goto error;
			
		proxy->uid_update_callback(proxy, proxy->uid_update_callback_data, objtype, olduid, newuid);

		osync_free(objtype);
		osync_free(olduid);
		osync_free(newuid);

		objtype = NULL;
		olduid = NULL;
		newuid = NULL;

		break;

	default:
		break;
	}
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;

error:
	if (objtype)
		osync_free(objtype);

	if (olduid)
		osync_free(olduid);

	if (newuid)
		osync_free(newuid);

	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(&error));
	osync_error_unref(&error);
}

OSyncClientProxy *osync_client_proxy_new(OSyncFormatEnv *formatenv, OSyncMember *member, OSyncError **error)
{
	OSyncClientProxy *proxy = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, formatenv, member, error);

	osync_assert(formatenv);
	
	proxy = osync_try_malloc0(sizeof(OSyncClientProxy), error);
	if (!proxy)
		goto error;
	proxy->ref_count = 1;
	proxy->type = OSYNC_START_TYPE_UNKNOWN;
	proxy->formatenv = osync_format_env_ref(formatenv);
	
	/* TODO: Is member optional parameter? */
	if (member) {
		proxy->member = member;
		osync_member_ref(member);
	}
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, proxy);
	return proxy;
	
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

OSyncClientProxy *osync_client_proxy_ref(OSyncClientProxy *proxy)
{
	osync_assert(proxy);
	
	g_atomic_int_inc(&(proxy->ref_count));

	return proxy;
}

void osync_client_proxy_unref(OSyncClientProxy *proxy)
{
	OSyncObjTypeSink *sink = NULL;
	osync_assert(proxy);
	
	if (g_atomic_int_dec_and_test(&(proxy->ref_count))) {
		if (proxy->path)
			g_free(proxy->path);
	
		if (proxy->member)
			osync_member_unref(proxy->member);
		
		while (proxy->objtypes) {
			sink = proxy->objtypes->data;
			osync_objtype_sink_unref(sink);
			proxy->objtypes = g_list_remove(proxy->objtypes, sink);
		}
		
		if (proxy->context)
			g_main_context_unref(proxy->context);

		if (proxy->formatenv)
			osync_format_env_unref(proxy->formatenv);

		if (proxy->error)
			osync_error_unref(&proxy->error);
		
		osync_free(proxy);
	}
}

void osync_client_proxy_set_context(OSyncClientProxy *proxy, GMainContext *ctx)
{
	osync_assert(proxy);
	proxy->context = ctx;
	if (ctx)
		g_main_context_ref(ctx);
}


void osync_client_proxy_set_change_callback(OSyncClientProxy *proxy, change_cb cb, void *userdata)
{
	osync_assert(proxy);
	
	proxy->change_callback = cb;
	proxy->change_callback_data = userdata;
}

void osync_client_proxy_set_uid_update_callback(OSyncClientProxy *proxy, uid_update_cb cb, void *userdata)
{
	osync_assert(proxy);
	
	proxy->uid_update_callback = cb;
	proxy->uid_update_callback_data = userdata;
}

OSyncMember *osync_client_proxy_get_member(OSyncClientProxy *proxy)
{
	osync_assert(proxy);
	return proxy->member;
}

osync_bool osync_client_proxy_spawn(OSyncClientProxy *proxy, OSyncStartType type, const char *path, const char* external_command, OSyncError **error)
{
	OSyncQueue *read1 = NULL;
	OSyncQueue *read2 = NULL;
	OSyncQueue *write1 = NULL;
	OSyncQueue *write2 = NULL;
	pid_t cpid = 0;
	char *readfd = NULL;
	char *writefd = NULL;
	char *name = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %i, %s, %s, %p)", __func__, proxy, type, __NULLSTR(path), __NULLSTR(external_command), error);
	osync_assert(proxy);
	osync_assert(type != OSYNC_START_TYPE_UNKNOWN);
		
	proxy->type = type;
	
	if (type != OSYNC_START_TYPE_EXTERNAL) {
		/* Now we either spawn a new process, or we create a new thread */
		if (type == OSYNC_START_TYPE_THREAD) {
			// First, create the pipe from the engine to the client
			if (!osync_queue_new_threadcom(&read1, &write1, error))
				goto error;
			// Then the pipe from the client to the engine
			if (!osync_queue_new_threadcom(&read2, &write2, error))
				goto error_free_pipe1;
			proxy->outgoing = osync_queue_ref(write1);
			proxy->incoming = osync_queue_ref(read2);

			proxy->client = osync_client_new(error);
			if (!proxy->client)
				goto error_free_pipe2;
			
			/* We now connect to our incoming queue */
			if (!osync_queue_connect(read1, OSYNC_QUEUE_RECEIVER, error))
				goto error_free_pipe2;
			
			/* and the to the outgoing queue */
			if (!osync_queue_connect(write2, OSYNC_QUEUE_SENDER, error))
				goto error_free_pipe2;
			
			
			if (!osync_client_set_incoming_queue(proxy->client, read1, error))
				goto error_free_pipe2;

			if (!osync_client_set_outgoing_queue(proxy->client, write2, error))
				goto error_free_pipe2;

			osync_queue_cross_link(read1, write2);
			
			if (!osync_client_run(proxy->client, error))
				goto error_free_pipe2;
		} else {
			if (!osync_queue_new_pipes(&read1, &write1, error))
				goto error;
			if (!osync_queue_new_pipes(&read2, &write2, error))
				goto error_free_pipe1;
			proxy->outgoing = osync_queue_ref(write1);
			proxy->incoming = osync_queue_ref(read2);

			/* First lets see if the old plugin exists, and kill it if it does */
			//if (!_osync_client_kill_old_osplugin(proxy, error))
			//	goto error;

			//if (!osync_queue_exists(proxy->outgoing) || !osync_queue_is_alive(proxy->outgoing)) {
			if (!proxy->outgoing || !osync_queue_exists(proxy->outgoing) || !osync_queue_is_alive(proxy->outgoing)) {
#ifndef _WIN32
				cpid = fork();
				if (cpid == 0) {
					osync_trace_reset_indent();
					
					/* close the read and write ends of the pipes */
					osync_queue_disconnect(write1, error);
					osync_queue_disconnect(read2, error);
						
					osync_trace(TRACE_INTERNAL, "About to exec osplugin");
					//char *memberstring = g_strdup_printf("%lli", osync_member_get_id(proxy->member));
					//execlp("osplugin", "osplugin", osync_group_get_configdir(osync_member_get_group(osync_proxy_get_member(proxy)), memberstring, NULL);
					readfd = osync_strdup_printf("%i", osync_queue_get_fd(read1));
					writefd = osync_strdup_printf("%i", osync_queue_get_fd(write2));
					execlp(OSPLUGIN, "osplugin", "-f", readfd, writefd, NULL);

					if (errno == ENOENT) {
						osync_trace(TRACE_INTERNAL, "Unable to find osplugin. Trying local path.");
						//execlp("osplugin", "osplugin", osync_group_get_configdir(osync_member_get_group(osync_proxy_get_member(proxy)), memberstring, NULL);
						execlp("./osplugin", "osplugin", "-f", readfd, writefd, NULL);
					}
											
					osync_trace(TRACE_INTERNAL, "%s", strerror(errno));
					osync_trace(TRACE_INTERNAL, "Unable to execute osplugin.");
					exit(1);
				} else {
					/* close the read and write ends of the pipes */
					osync_queue_disconnect(write2, error);
					osync_queue_disconnect(read1, error);
				}
	
				proxy->child_pid = cpid;
			
				//while (!osync_queue_exists(proxy->outgoing)) {
				//	osync_trace(TRACE_INTERNAL, "Waiting for other side to create fifo");
				//	g_usleep(500000);
				//}
			
				osync_trace(TRACE_INTERNAL, "Queue was created");
#endif //_WIN32
			}
	
			//if (proxy->child_pid) {
			//	if (!_osync_client_create_pidfile(proxy, error))
			//		goto error;
			//}
		}
		
		osync_queue_unref(read1);
		osync_queue_unref(write1);
		osync_queue_unref(read2);
		osync_queue_unref(write2);

		/* We now connect to our incoming queue */
		if (!osync_queue_connect(proxy->incoming, OSYNC_QUEUE_RECEIVER, error))
			goto error;
			
		/* and the to the outgoing queue */
		if (!osync_queue_connect(proxy->outgoing, OSYNC_QUEUE_SENDER, error))
			goto error;
	} else {
		name = osync_strdup_printf("%s%cpluginpipe", path, G_DIR_SEPARATOR);
	
		if (external_command) {
			char *command = osync_strdup_printf(external_command, name);
			osync_trace(TRACE_INTERNAL, "g_spawn_command_line_async(%s)", command);
			GError *gerror = NULL;
			gboolean f = g_spawn_command_line_async(command, &gerror);
			if (!f) {
				osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to g_spawn_command_line_async(%s): %s", command, gerror->message);
				g_error_free (gerror);
				osync_free(command);
				goto error;
			}
			osync_free(command);
		}


		proxy->outgoing = osync_queue_new(name, error);
		osync_free(name);
		if (!proxy->outgoing)
			goto error;
		
		name = osync_strdup_printf("%s%cenginepipe", path, G_DIR_SEPARATOR);
		proxy->incoming = osync_queue_new(name, error);
		osync_free(name);
		if (!proxy->incoming)
			goto error;
			
		if (!osync_queue_create(proxy->outgoing, error))
			goto error;
			
		if (!osync_queue_create(proxy->incoming, error))
			goto error;
			
		/* and the to the outgoing queue */
		if (!osync_queue_connect(proxy->outgoing, OSYNC_QUEUE_SENDER, error))
			goto error;
	}
	
	osync_queue_set_message_handler(proxy->incoming, _osync_client_proxy_message_handler, proxy);
	if (!osync_queue_setup_with_gmainloop(proxy->incoming, proxy->context, error))
		goto error;
	
	osync_queue_set_message_handler(proxy->outgoing, _osync_client_proxy_hup_handler, proxy);
	if (!osync_queue_setup_with_gmainloop(proxy->outgoing, proxy->context, error))
		goto error;
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
	
 error_free_pipe2:
	osync_queue_unref(read2);
	osync_queue_unref(write2);
 error_free_pipe1:
	osync_queue_unref(read1);
	osync_queue_unref(write1);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_client_proxy_shutdown(OSyncClientProxy *proxy, OSyncError **error)
{
	OSyncMessage *message = NULL;
	int status = 0;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, proxy, error);
	
	/* We first disconnect our reading queue. This will generate a HUP
	 * on the remote side */
	if (!osync_queue_disconnect(proxy->incoming, error))
		goto error;
	
	/* We now wait for the HUP on our sending queue */
	message = osync_queue_get_message(proxy->outgoing);
	if (osync_message_get_command(message) != OSYNC_MESSAGE_QUEUE_HUP) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Disconnected, but received no HUP");
		osync_message_unref(message);
		goto error;
	}
	osync_message_unref(message);
	
	/* After we received the HUP, we can disconnect */
	if (!osync_queue_disconnect(proxy->outgoing, error))
		goto error;
			
	if (proxy->type == OSYNC_START_TYPE_THREAD) {
#ifndef OPENSYNC_PREVENT_CLIENT_SHUTDOWN
		osync_client_shutdown(proxy->client);
#endif
		osync_client_unref(proxy->client);
	} else if (proxy->type == OSYNC_START_TYPE_PROCESS) {
		if (proxy->child_pid) {
#ifndef _WIN32
			if (waitpid(proxy->child_pid, &status, 0) == -1) {
				osync_error_set(error, OSYNC_ERROR_GENERIC, "Error waiting for osplugin process: %s", g_strerror(errno));
				goto error;
			}

			if (!WIFEXITED(status))
				osync_trace(TRACE_INTERNAL, "Child has exited abnormally");
			else if (WEXITSTATUS(status) != 0)
				osync_trace(TRACE_INTERNAL, "Child has returned non-zero exit status (%d)", WEXITSTATUS(status));
#endif //_WIN32

			//if (!osync_client_remove_pidfile(client, error))
			//	goto error;
		}
		
		/* First lets see if the old plugin exists, and kill it if it does */
		//if (!_osync_client_kill_old_osplugin(proxy, error))
		//	goto error;
	}
			
	osync_queue_unref(proxy->incoming);
	osync_queue_unref(proxy->outgoing);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
	
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool osync_client_proxy_check_resource(OSyncClientProxy *proxy, OSyncPluginResource *resource, OSyncError **error)
{
	OSyncList *format_sinks = osync_plugin_resource_get_objformat_sinks(resource);
	OSyncList *o = NULL;
	
	/* OSyncPluginConfig should fail on validiation if format sinks are missing for any resource */
	osync_assert(format_sinks);

	for (o = format_sinks; o; o = o->next) {
		OSyncObjFormatSink *format_sink = o->data;	
		const char *objformat_name = osync_objformat_sink_get_objformat(format_sink);

		if (!osync_format_env_find_objformat(proxy->formatenv, objformat_name)) {
			osync_error_set(error, OSYNC_ERROR_PLUGIN_NOT_FOUND, "Plugin for format \"%s\" not found.", objformat_name);
			osync_list_free(format_sinks);
			return FALSE;
		}
	}

	osync_list_free(format_sinks);
	return TRUE;
}

void osync_client_proxy_add_objtype_sink(OSyncClientProxy *proxy, OSyncObjTypeSink *sink)
{
	osync_return_if_fail(proxy);
	osync_return_if_fail(sink);

	osync_objtype_sink_ref(sink);
	proxy->objtypes = g_list_append(proxy->objtypes, sink);
}

osync_bool osync_client_proxy_initialize(OSyncClientProxy *proxy, initialize_cb callback, void *userdata, const char *formatdir, const char *plugindir, const char *plugin, const char *groupname, const char *configdir, OSyncPluginConfig *config, OSyncError **error)
{
	callContext *ctx = NULL;
	int haspluginconfig = config ? TRUE : FALSE;
	OSyncMessage *message = NULL;
#ifdef OPENSYNC_UNITTESTS
	long long int memberid = 0;
#endif
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %s, %s, %s, %s, %s, %p, %p)", __func__, proxy, callback, userdata, __NULLSTR(formatdir), __NULLSTR(plugindir), __NULLSTR(plugin), __NULLSTR(groupname), __NULLSTR(configdir), config, error);
	osync_assert(proxy);
	

	ctx = osync_try_malloc0(sizeof(callContext), error);
	if (!ctx)
		goto error;
	
	ctx->proxy = proxy;
	ctx->init_callback = callback;
	ctx->init_callback_data = userdata;
	
	message = osync_message_new(OSYNC_MESSAGE_INITIALIZE, 0, error);
	if (!message)
		goto error;
	
	osync_message_write_string(message, osync_queue_get_path(proxy->incoming), error);
	osync_message_write_string(message, formatdir, error);
	osync_message_write_string(message, plugindir, error);
	osync_message_write_string(message, plugin, error);
	osync_message_write_string(message, groupname, error);
	osync_message_write_string(message, configdir, error);
	osync_message_write_int(message, haspluginconfig, error);

	if (osync_error_is_set(error))
		goto error;

	if (haspluginconfig && !osync_marshal_pluginconfig(message, config, error))
		goto error;

	if (haspluginconfig) {
		OSyncList *r = osync_plugin_config_get_resources(config);
		for (; r; r = r->next) {
			OSyncPluginResource *res = r->data;
			const char *objtype = NULL;
			OSyncObjTypeSink *sink = NULL;


			if (!osync_plugin_resource_is_enabled(res))
				continue;

			if (!osync_client_proxy_check_resource(proxy, res, error))
				goto error;

			objtype = osync_plugin_resource_get_objtype(res);
			sink = osync_client_proxy_find_objtype_sink(proxy, objtype);
			/* TODO: In discovery phase *sink COULD be NULL. Review if this is correct behavior. */
			if (sink) {
				osync_client_proxy_add_objtype_sink(proxy, sink);
			}
		}
	}

#ifdef OPENSYNC_UNITTESTS
	// Introduced (only) for testing/debugging purpose (mock-sync)

	if (proxy->member)
		memberid = osync_member_get_id(proxy->member);

	osync_message_write_long_long_int(message, memberid, error);
#endif	
	
	osync_message_set_handler(message, _osync_client_proxy_init_handler, ctx);
	
	if (!osync_queue_send_message_with_timeout(proxy->outgoing, proxy->incoming, message, proxy->timeout.initialize, error))
		goto error_free_message;
	
	osync_message_unref(message);
	
	if (proxy->type == OSYNC_START_TYPE_EXTERNAL) {
		/* We now connect to our incoming queue */
		if (!osync_queue_connect(proxy->incoming, OSYNC_QUEUE_RECEIVER, error))
			goto error;
	}
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_free_message:
	osync_message_unref(message);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

unsigned int osync_client_proxy_get_initialize_timeout(OSyncClientProxy *proxy)
{
	osync_assert(proxy);
	return proxy->timeout.initialize;
}

void osync_client_proxy_set_initialize_timeout(OSyncClientProxy *proxy, unsigned int timeout)
{
	osync_assert(proxy);
	proxy->timeout.initialize = timeout;
}

osync_bool osync_client_proxy_finalize(OSyncClientProxy *proxy, finalize_cb callback, void *userdata, OSyncError **error)
{
	callContext *ctx = NULL;
	OSyncMessage *message = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %p)", __func__, proxy, callback, userdata, error);
	
	ctx = osync_try_malloc0(sizeof(callContext), error);
	if (!ctx)
		goto error;
	
	ctx->proxy = proxy;
	ctx->fin_callback = callback;
	ctx->fin_callback_data = userdata;
	
	message = osync_message_new(OSYNC_MESSAGE_FINALIZE, 0, error);
	if (!message)
		goto error;

	osync_message_set_handler(message, _osync_client_proxy_fin_handler, ctx);
	
	if (!osync_queue_send_message_with_timeout(proxy->outgoing, proxy->incoming, message, proxy->timeout.finalize, error))
		goto error_free_message;

	osync_message_unref(message);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_free_message:
	osync_message_unref(message);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

unsigned int osync_client_proxy_get_finalize_timeout(OSyncClientProxy *proxy)
{
	osync_assert(proxy);
	return proxy->timeout.finalize;
}

void osync_client_proxy_set_finalize_timeout(OSyncClientProxy *proxy, unsigned int timeout)
{
	osync_assert(proxy);
	proxy->timeout.finalize = timeout;
}

osync_bool osync_client_proxy_discover(OSyncClientProxy *proxy, discover_cb callback, void *userdata, OSyncError **error)
{
	callContext *ctx = NULL;
	OSyncMessage *message = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %p)", __func__, proxy, callback, userdata, error);
	
	ctx = osync_try_malloc0(sizeof(callContext), error);
	if (!ctx)
		goto error;
	
	ctx->proxy = proxy;
	ctx->discover_callback = callback;
	ctx->discover_callback_data = userdata;
	
	message = osync_message_new(OSYNC_MESSAGE_DISCOVER, 0, error);
	if (!message)
		goto error;

	osync_message_set_handler(message, _osync_client_proxy_discover_handler, ctx);
	
	if (!osync_queue_send_message_with_timeout(proxy->outgoing, proxy->incoming, message, proxy->timeout.discover, error))
		goto error_free_message;
	
	osync_message_unref(message);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_free_message:
	osync_message_unref(message);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

unsigned int osync_client_proxy_get_discover_timeout(OSyncClientProxy *proxy)
{
	osync_assert(proxy);
	return proxy->timeout.discover;
}

void osync_client_proxy_set_discover_timeout(OSyncClientProxy *proxy, unsigned int timeout)
{
	osync_assert(proxy);
	proxy->timeout.discover = timeout;
}

unsigned int osync_client_proxy_num_objtypes(OSyncClientProxy *proxy)
{
	osync_assert(proxy);
	return g_list_length(proxy->objtypes);
}

OSyncObjTypeSink *osync_client_proxy_nth_objtype(OSyncClientProxy *proxy, unsigned int nth)
{
	osync_assert(proxy);
	return g_list_nth_data(proxy->objtypes, nth);
}

OSyncObjTypeSink *osync_client_proxy_find_objtype_sink(OSyncClientProxy *proxy, const char *objtype)
{
	GList *o = NULL;
	OSyncObjTypeSink *sink = NULL;

	osync_assert(proxy);

	for (o = proxy->objtypes; o; o = o->next) {
		sink = o->data;
		if (!objtype && !osync_objtype_sink_get_name(sink))
			return sink;

		if (objtype && !strcmp(osync_objtype_sink_get_name(sink), objtype))
			return sink;
	}

	if (objtype && proxy->member)
		return osync_member_find_objtype_sink(proxy->member, objtype);
	else if (!objtype && proxy->member)
		return osync_member_get_main_sink(proxy->member);

	return NULL;
}

osync_bool osync_client_proxy_connect(OSyncClientProxy *proxy, connect_cb callback, void *userdata, const char *objtype, osync_bool slowsync, OSyncError **error)
{
	int timeout = 0;
	callContext *ctx = NULL;
	OSyncObjTypeSink *sink = NULL;
	OSyncMessage *message = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %s, %i, %p)", __func__, proxy, callback, userdata, __NULLSTR(objtype), slowsync, error);
	
	timeout = OSYNC_CLIENT_PROXY_TIMEOUT_CONNECT;

	ctx = osync_try_malloc0(sizeof(callContext), error);
	if (!ctx)
		goto error;
	
	ctx->proxy = proxy;
	ctx->connect_callback = callback;
	ctx->connect_callback_data = userdata;
	
	sink = osync_client_proxy_find_objtype_sink(proxy, objtype);
	if (sink)
		timeout = osync_objtype_sink_get_connect_timeout_or_default(sink); 

	message = osync_message_new(OSYNC_MESSAGE_CONNECT, 0, error);
	if (!message)
		goto error_free_context;
	
	osync_message_set_handler(message, _osync_client_proxy_connect_handler, ctx);

	osync_message_write_string(message, objtype, error);
	osync_message_write_int(message, slowsync, error);
	
	if (osync_error_is_set(error))
		goto error_free_message;
	
	if (!osync_queue_send_message_with_timeout(proxy->outgoing, proxy->incoming, message, timeout, error))
		goto error_free_message;
	
	osync_message_unref(message);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_free_message:
	osync_message_unref(message);
 error_free_context:
	osync_free(ctx);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_client_proxy_connect_done(OSyncClientProxy *proxy, sync_done_cb callback, void *userdata, const char *objtype, osync_bool slowsync, OSyncError **error)
{
	int timeout = 0;
	callContext *ctx = NULL;
	OSyncObjTypeSink *sink = NULL;
	OSyncMessage *message = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %s, %p)", __func__, proxy, callback, userdata, __NULLSTR(objtype), error);
	osync_assert(proxy);

	timeout = OSYNC_CLIENT_PROXY_TIMEOUT_CONNECTDONE;

	ctx = osync_try_malloc0(sizeof(callContext), error);
	if (!ctx)
		goto error;

	sink = osync_client_proxy_find_objtype_sink(proxy, objtype);
	if (sink)
		timeout = osync_objtype_sink_get_connectdone_timeout_or_default(sink); 

	ctx->proxy = proxy;
	ctx->connect_done_callback = callback;
	ctx->connect_done_callback_data = userdata;
	
	message = osync_message_new(OSYNC_MESSAGE_CONNECT_DONE, 0, error);
	if (!message)
		goto error_free_context;
	
	osync_message_set_handler(message, _osync_client_proxy_connect_done_handler, ctx);

	osync_message_write_string(message, objtype, error);
	osync_message_write_int(message, slowsync, error);

	if (osync_error_is_set(error))
		goto error_free_message;
	
	if (!osync_queue_send_message_with_timeout(proxy->outgoing, proxy->incoming, message, timeout, error))
		goto error_free_message;
	
	osync_message_unref(message);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_free_message:
	osync_message_unref(message);
 error_free_context:
	osync_free(ctx);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_client_proxy_disconnect(OSyncClientProxy *proxy, disconnect_cb callback, void *userdata, const char *objtype, OSyncError **error)
{
	int timeout = 0;
	callContext *ctx = NULL;
	OSyncObjTypeSink *sink = NULL;
	OSyncMessage *message = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %s, %p)", __func__, proxy, callback, userdata, __NULLSTR(objtype), error);

	timeout = OSYNC_CLIENT_PROXY_TIMEOUT_DISCONNECT;

	ctx = osync_try_malloc0(sizeof(callContext), error);
	if (!ctx)
		goto error;
	
	sink = osync_client_proxy_find_objtype_sink(proxy, objtype);
	if (sink)
		timeout = osync_objtype_sink_get_disconnect_timeout_or_default(sink); 

	ctx->proxy = proxy;
	ctx->disconnect_callback = callback;
	ctx->disconnect_callback_data = userdata;
	
	message = osync_message_new(OSYNC_MESSAGE_DISCONNECT, 0, error);
	if (!message)
		goto error_free_context;
	
	osync_message_set_handler(message, _osync_client_proxy_disconnect_handler, ctx);

	if (!osync_message_write_string(message, objtype, error))
		goto error_free_message;
	
	if (!osync_queue_send_message_with_timeout(proxy->outgoing, proxy->incoming, message, timeout, error))
		goto error_free_message;
	
	osync_message_unref(message);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_free_message:
	osync_message_unref(message);
 error_free_context:
	osync_free(ctx);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_client_proxy_read(OSyncClientProxy *proxy, read_cb callback, void *userdata, OSyncChange *change, OSyncError **error)
{
	int timeout = 0;
	callContext *ctx = NULL;
	OSyncObjTypeSink *sink = NULL;
	OSyncMessage *message = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %p, %p)", __func__, proxy, callback, userdata, change, error);
	
	timeout = OSYNC_CLIENT_PROXY_TIMEOUT_READ;

	ctx = osync_try_malloc0(sizeof(callContext), error);
	if (!ctx)
		goto error;
	
	sink = osync_client_proxy_find_objtype_sink(proxy, osync_change_get_objtype(change));
	if (sink)
		timeout = osync_objtype_sink_get_read_timeout_or_default(sink); 

	ctx->proxy = proxy;
	ctx->read_callback = callback;
	ctx->read_callback_data = userdata;
	
	message = osync_message_new(OSYNC_MESSAGE_READ_CHANGE, 0, error);
	if (!message)
		goto error_free_context;
	
	osync_message_set_handler(message, _osync_client_proxy_read_handler, ctx);

	if (!osync_marshal_change(message, change, error))
		goto error_free_message;

	if (!osync_queue_send_message_with_timeout(proxy->outgoing, proxy->incoming, message, timeout, error))
		goto error_free_message;

	osync_message_unref(message);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_free_message:
	osync_message_unref(message);
 error_free_context:
	osync_free(ctx);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_client_proxy_get_changes(OSyncClientProxy *proxy, get_changes_cb callback, void *userdata, const char *objtype, osync_bool slowsync, OSyncError **error)
{
	int timeout = 0;
	callContext *ctx = NULL;
	OSyncObjTypeSink *sink = NULL;
	OSyncMessage *message = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %s, %i, %p)", __func__, proxy, callback, userdata, __NULLSTR(objtype), slowsync, error);
	
	timeout = OSYNC_CLIENT_PROXY_TIMEOUT_GETCHANGES;

	ctx = osync_try_malloc0(sizeof(callContext), error);
	if (!ctx)
		goto error;
	
	sink = osync_client_proxy_find_objtype_sink(proxy, objtype);
	if (sink)
		timeout = osync_objtype_sink_get_getchanges_timeout_or_default(sink); 

	ctx->proxy = proxy;
	ctx->get_changes_callback = callback;
	ctx->get_changes_callback_data = userdata;
	
	message = osync_message_new(OSYNC_MESSAGE_GET_CHANGES, 0, error);
	if (!message)
		goto error_free_context;
	
	osync_message_set_handler(message, _osync_client_proxy_get_changes_handler, ctx);

	osync_message_write_string(message, objtype, error);
	osync_message_write_int(message, slowsync, error);

	if (osync_error_is_set(error))
		goto error_free_message;
	
	if (!osync_queue_send_message_with_timeout(proxy->outgoing, proxy->incoming, message, timeout, error))
		goto error_free_message;
	
	osync_message_unref(message);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_free_message:
	osync_message_unref(message);
 error_free_context:
	osync_free(ctx);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_client_proxy_commit_change(OSyncClientProxy *proxy, commit_change_cb callback, void *userdata, OSyncChange *change, OSyncError **error)
{
	int timeout = 0;
	callContext *ctx = NULL;
	OSyncObjTypeSink *sink = NULL;
	OSyncMessage *message = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %p, %p)", __func__, proxy, callback, userdata, change, error);
	osync_assert(proxy);
	osync_assert(change);

	timeout = OSYNC_CLIENT_PROXY_TIMEOUT_COMMIT;

	ctx = osync_try_malloc0(sizeof(callContext), error);
	if (!ctx)
		goto error;

	sink = osync_client_proxy_find_objtype_sink(proxy, osync_change_get_objtype(change));
	if (sink)
		timeout = osync_objtype_sink_get_commit_timeout_or_default(sink); 
	
	ctx->proxy = proxy;
	ctx->commit_change_callback = callback;
	ctx->commit_change_callback_data = userdata;
	
	message = osync_message_new(OSYNC_MESSAGE_COMMIT_CHANGE, 0, error);
	if (!message)
		goto error_free_context;
	
	osync_message_set_handler(message, _osync_client_proxy_commit_change_handler, ctx);

	if (!osync_marshal_change(message, change, error))
		goto error_free_message;
	
	if (!osync_queue_send_message_with_timeout(proxy->outgoing, proxy->incoming, message, timeout, error))
		goto error_free_message;
	
	osync_message_unref(message);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_free_message:
	osync_message_unref(message);
 error_free_context:
	osync_free(ctx);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_client_proxy_committed_all(OSyncClientProxy *proxy, committed_all_cb callback, void *userdata, const char *objtype, OSyncError **error)
{
	int timeout = 0;
	callContext *ctx = NULL;
	OSyncObjTypeSink *sink = NULL;
	OSyncMessage *message = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %s, %p)", __func__, proxy, callback, userdata, __NULLSTR(objtype), error);
	osync_assert(proxy);

	timeout = OSYNC_CLIENT_PROXY_TIMEOUT_COMMITTEDALL;
	
	ctx = osync_try_malloc0(sizeof(callContext), error);
	if (!ctx)
		goto error;
	
	sink = osync_client_proxy_find_objtype_sink(proxy, objtype);
	if (sink)
		timeout = osync_objtype_sink_get_committedall_timeout_or_default(sink); 

	ctx->proxy = proxy;
	ctx->committed_all_callback = callback;
	ctx->committed_all_callback_data = userdata;
	
	message = osync_message_new(OSYNC_MESSAGE_COMMITTED_ALL, 0, error);
	if (!message)
		goto error_free_context;
	
	osync_message_set_handler(message, _osync_client_proxy_committed_all_handler, ctx);

	osync_message_write_string(message, objtype, error);

	if (osync_error_is_set(error))
		goto error;
	
	if (!osync_queue_send_message_with_timeout(proxy->outgoing, proxy->incoming, message, timeout, error))
		goto error_free_message;
	
	osync_message_unref(message);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_free_message:
	osync_message_unref(message);
 error_free_context:
	osync_free(ctx);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_client_proxy_sync_done(OSyncClientProxy *proxy, sync_done_cb callback, void *userdata, const char *objtype, OSyncError **error)
{
	int timeout = 0;
	callContext *ctx = NULL;
	OSyncObjTypeSink *sink = NULL;
	OSyncMessage *message = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %s, %p)", __func__, proxy, callback, userdata, __NULLSTR(objtype), error);
	osync_assert(proxy);

	timeout = OSYNC_CLIENT_PROXY_TIMEOUT_SYNCDONE;

	ctx = osync_try_malloc0(sizeof(callContext), error);
	if (!ctx)
		goto error;

	sink = osync_client_proxy_find_objtype_sink(proxy, objtype);
	if (sink) {
		timeout = osync_objtype_sink_get_syncdone_timeout_or_default(sink); 

		/* Reset the slow-sync state in the client-proxy OSyncObjTypeSink object, since finalize might
		 * not get called in a multi-sync
		 */
		osync_objtype_sink_set_slowsync(sink, FALSE);
	}

	ctx->proxy = proxy;
	ctx->sync_done_callback = callback;
	ctx->sync_done_callback_data = userdata;
	
	message = osync_message_new(OSYNC_MESSAGE_SYNC_DONE, 0, error);
	if (!message)
		goto error_free_context;
	
	osync_message_set_handler(message, _osync_client_proxy_sync_done_handler, ctx);

	if (!osync_message_write_string(message, objtype, error))
		goto error_free_message;
	
	if (!osync_queue_send_message_with_timeout(proxy->outgoing, proxy->incoming, message, timeout, error))
		goto error_free_message;
	
	osync_message_unref(message);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_free_message:
	osync_message_unref(message);
 error_free_context:
	osync_free(ctx);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

void osync_client_proxy_set_error(OSyncClientProxy *proxy, OSyncError *error)
{
	osync_assert(proxy);
	if (proxy->error) {
		osync_error_stack(&error, &proxy->error);
		osync_error_unref(&proxy->error);
	}
	
	proxy->error = error;
	if (error)
		osync_error_ref(&error);
}

