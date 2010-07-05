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

#include "opensync-plugin.h"
#include "plugin/opensync_plugin_internals.h"
#include "plugin/opensync_objtype_sink_internals.h"
#include "plugin/opensync_plugin_info_internals.h"
#include "plugin/opensync_context_internals.h"

#include "opensync-helper.h"
#include "helper/opensync_hashtable_internals.h"

#include "opensync-ipc.h"
#include "ipc/opensync_serializer_internals.h"
#include "ipc/opensync_message_internals.h"
#include "ipc/opensync_queue_internals.h"

#include "opensync-format.h"

#include "opensync-capabilities.h"
#include "capabilities/opensync_capabilities_internals.h"
#include "opensync-version.h"
#include "version/opensync_version_internals.h"

#include "opensync-client.h"
#include "opensync_client_internals.h"
#include "opensync_client_private.h"

#ifdef OPENSYNC_UNITTESTS
#include "plugin/opensync_plugin_info_private.h"	/* FIXME: access directly private header */
#endif

typedef struct callContext {
	OSyncClient *client;
	OSyncMessage *message;
	OSyncChange *change;
} callContext;

static OSyncContext *_create_context(OSyncClient *client, OSyncMessage *message, OSyncContextCallbackFn callback, OSyncChange *change, OSyncError **error)
{
	OSyncContext *context = NULL;
	callContext *baton = NULL;
	context = osync_context_new(error);
	if (!context)
		goto error;
	
	baton = osync_try_malloc0(sizeof(callContext), error);
	if (!baton)
		goto error_free_context;
	
	baton->client = client;
	osync_client_ref(baton->client);
	
	baton->message = message;
	osync_message_ref(message);
	
	baton->change = change;
	if (baton->change)
		osync_change_ref(baton->change);
	
	osync_context_set_callback(context, callback, baton);
	return context;
	
 error_free_context:
	osync_context_unref(context);
 error:
	return FALSE;
}

static void _free_baton(callContext *baton)
{
	osync_client_unref(baton->client);
	osync_message_unref(baton->message);
	
	if (baton->change)
		osync_change_unref(baton->change);
	
	osync_free(baton);
}

static void _osync_client_slowsync_callback(void *data)
{
	OSyncObjTypeSink *sink = (OSyncObjTypeSink *) data;
	osync_objtype_sink_set_slowsync(sink, TRUE);
}


static void _osync_client_connect_callback(void *data, OSyncError *error)
{
	OSyncError *locerror = NULL;
	callContext *baton = NULL;
	OSyncMessage *message = NULL;
	OSyncClient *client = NULL;
	char *objtype = NULL;
	OSyncObjTypeSink *sink = NULL;
	osync_bool slowsync = FALSE;
	OSyncMessage *reply = NULL;

	baton = data;
	message = baton->message;
	client = baton->client;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, data, error);
	if (!osync_message_read_string(message, &objtype, &error))
		goto error;

	if (objtype) {
		// objtype sink (e.g. event, contact, ...)
		sink = osync_plugin_info_find_objtype(client->plugin_info, objtype);
		
		if (!sink) {
			osync_error_set(&locerror, OSYNC_ERROR_GENERIC, "Unable to find sink for %s", objtype);
			osync_free(objtype);
			goto error;
		}
		
		osync_free(objtype);
	} else {
		// main sink
		sink = osync_plugin_info_get_sink(client->plugin_info);
	}
	slowsync = osync_objtype_sink_get_slowsync(sink);
	osync_trace(TRACE_INTERNAL, "%s: slowsync %i", __func__, slowsync);

	if (!osync_error_is_set(&error)) {
		reply = osync_message_new_reply(message, &locerror);
		if (!reply)
			goto error;

		//Send connect specific reply data
		if (!osync_message_write_int(reply, slowsync, &error))
			goto error_free_message;

	} else {
		reply = osync_message_new_errorreply(message, error, &locerror);
	}
	if (!reply)
		goto error;
	osync_trace(TRACE_INTERNAL, "Reply id %lli", osync_message_get_id(reply));

	_free_baton(baton);
	
	if (!osync_queue_send_message(client->outgoing, NULL, reply, &locerror))
		goto error_free_message;
	
	osync_message_unref(reply);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
	
 error_free_message:
	osync_message_unref(reply);
 error:
	_free_baton(baton);
	osync_client_error_shutdown(client, locerror);
	osync_error_unref(&locerror);
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
}

static void _osync_client_connect_done_callback(void *data, OSyncError *error)
{
	OSyncError *locerror = NULL;
	callContext *baton = NULL;
	OSyncMessage *message = NULL;
	OSyncClient *client = NULL;
	OSyncMessage *reply = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, data, error);
	baton = data;

	message = baton->message;
	client = baton->client;

	if (!osync_error_is_set(&error)) {
		reply = osync_message_new_reply(message, &locerror);
	} else {
		reply = osync_message_new_errorreply(message, error, &locerror);
	}
	if (!reply)
		goto error;
	osync_trace(TRACE_INTERNAL, "Reply id %lli", osync_message_get_id(reply));

	_free_baton(baton);
	
	if (!osync_queue_send_message(client->outgoing, NULL, reply, &locerror))
		goto error_free_message;
	
	osync_message_unref(reply);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
	
 error_free_message:
	osync_message_unref(reply);
 error:
	_free_baton(baton);
	osync_client_error_shutdown(client, locerror);
	osync_error_unref(&locerror);
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
}

static void _osync_client_disconnect_callback(void *data, OSyncError *error)
{
	OSyncError *locerror = NULL;
	callContext *baton = NULL;
	OSyncMessage *message = NULL;
	OSyncClient *client = NULL;
	OSyncMessage *reply = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, data, error);

	baton = data;
	message = baton->message;
	client = baton->client;

	if (!osync_error_is_set(&error)) {
		reply = osync_message_new_reply(message, &locerror);
	} else {
		reply = osync_message_new_errorreply(message, error, &locerror);
	}
	if (!reply)
		goto error;
	osync_trace(TRACE_INTERNAL, "Reply id %lli", osync_message_get_id(reply));

	_free_baton(baton);
	
	if (!osync_queue_send_message(client->outgoing, NULL, reply, &locerror))
		goto error_free_message;
	
	osync_message_unref(reply);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
	
 error_free_message:
	osync_message_unref(reply);
 error:
	_free_baton(baton);
	osync_client_error_shutdown(client, locerror);
	osync_error_unref(&locerror);
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
}

static osync_bool _osync_client_handle_capabilities_message(OSyncClient *client, OSyncMessage *reply, OSyncError **error)
{

	OSyncVersion *version = NULL;
	OSyncCapabilities *capabilities = NULL;
	char* buffer = NULL;
	unsigned int size = 0;

	version = osync_plugin_info_get_version(client->plugin_info);
	if (version) {
		osync_message_write_int(reply, 1, error);
		osync_message_write_string(reply, osync_version_get_plugin(version), error);
		osync_message_write_string(reply, osync_version_get_priority(version), error);
		osync_message_write_string(reply, osync_version_get_vendor(version), error);
		osync_message_write_string(reply, osync_version_get_modelversion(version), error);
		osync_message_write_string(reply, osync_version_get_firmwareversion(version), error);
		osync_message_write_string(reply, osync_version_get_softwareversion(version), error);
		osync_message_write_string(reply, osync_version_get_hardwareversion(version), error);
		osync_message_write_string(reply, osync_version_get_identifier(version), error);
	} else {
		osync_message_write_int(reply, 0, error);
	}

	if (osync_error_is_set(error))
		goto error;
	
	/* Report detected capabilities */
	capabilities = osync_plugin_info_get_capabilities(client->plugin_info);
	if (capabilities) {
		if (!osync_message_write_int(reply, 1, error))
			goto error;

		if (!osync_capabilities_assemble(capabilities, &buffer, &size, error))
			goto error;

		if (!osync_message_write_string(reply, buffer, error))
			goto error;

		g_free(buffer);
	} else {
		if (!osync_message_write_int(reply, 0, error))
			goto error;
	}

	return TRUE;

error:
	return FALSE;
}

static void _osync_client_get_changes_callback(void *data, OSyncError *error)
{
	OSyncError *locerror = NULL;
	callContext *baton = NULL;
	OSyncMessage *message = NULL;
	OSyncClient *client = NULL;
	OSyncMessage *reply = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, data, error);

	baton = data;
	message = baton->message;
	client = baton->client;

	if (!osync_error_is_set(&error)) {
		reply = osync_message_new_reply(message, &locerror);
		//Send get_changes specific reply data
	} else {
		reply = osync_message_new_errorreply(message, error, &locerror);
	}
	if (!reply)
		goto error;
	osync_trace(TRACE_INTERNAL, "Reply id %lli", osync_message_get_id(reply));


	/* Handle capabilities if those got discovered during the sync .... */
	if (!_osync_client_handle_capabilities_message(client, reply, &locerror))
		goto error_free_message;

	_free_baton(baton);
	
	if (!osync_queue_send_message(client->outgoing, NULL, reply, &locerror))
		goto error_free_message;
	
	osync_message_unref(reply);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
	
 error_free_message:
	osync_message_unref(reply);
 error:
	_free_baton(baton);
	osync_client_error_shutdown(client, locerror);
	osync_error_unref(&locerror);
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
}

static void _osync_client_change_callback(OSyncChange *change, void *data)
{
	callContext *baton = NULL;
	OSyncError *locerror = NULL;
	OSyncClient *client = NULL;
	OSyncMessage *message = NULL;

	baton = data;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, change, data);
	
	client = baton->client;
	message = osync_message_new(OSYNC_MESSAGE_NEW_CHANGE, 0, &locerror);
	if (!message)
		goto error;

	if (!osync_marshal_change(message, change, &locerror))
		goto error_free_message;

	if (!osync_queue_send_message(client->outgoing, NULL, message, &locerror))
		goto error_free_message;
	
	osync_message_unref(message);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
	
 error_free_message:
	osync_message_unref(message);
 error:
	_free_baton(baton);
	osync_client_error_shutdown(client, locerror);
	osync_error_unref(&locerror);
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
}

static void _osync_client_uid_update_callback(const char *olduid, const char *newuid, OSyncObjTypeSink *sink, void *data)
{
	callContext *baton = NULL;
	OSyncError *locerror = NULL;
	OSyncClient *client = NULL;
	OSyncMessage *message = NULL;
	OSyncHashTable *hashtable = NULL;
	const char *objtype = NULL;

	client = data;
	osync_trace(TRACE_ENTRY, "%s(%s, %s, %p, %p)", __func__, __NULLSTR(olduid), __NULLSTR(newuid), sink, data);

	objtype = osync_objtype_sink_get_name(sink);

	/* We don't expect main-sink here */
	osync_assert(objtype);


	/* Update Hashtable, if available, first. */
	if ((hashtable = osync_objtype_sink_get_hashtable(sink))) {

		if (!osync_hashtable_update_uid(hashtable, olduid, newuid, &locerror))
			goto error;

	}
	
	message = osync_message_new(OSYNC_MESSAGE_MAPPING_CHANGED, 0, &locerror);
	if (!message)
		goto error;

	/** objtype */
	if (!osync_message_write_string(message, objtype, &locerror))
		goto error_free_message;

	/** olduid */
	if (!osync_message_write_string(message, olduid, &locerror))
		goto error_free_message;

	/** newuid */
	if (!osync_message_write_string(message, newuid, &locerror))
		goto error_free_message;

	if (!osync_queue_send_message(client->outgoing, NULL, message, &locerror))
		goto error_free_message;
	
	osync_message_unref(message);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
	
 error_free_message:
	osync_message_unref(message);
 error:
	_free_baton(baton);
	osync_client_error_shutdown(client, locerror);
	osync_error_unref(&locerror);
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
}

static void _osync_client_ignored_conflict_callback(OSyncChange *change, void *data)
{
	callContext *baton = NULL;
	OSyncError *locerror = NULL;
	OSyncClient *client = NULL;
	OSyncMessage *message = NULL;

	baton = data;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, change, data);
	
	client = baton->client;
	message = osync_message_new(OSYNC_MESSAGE_READ_CHANGE, 0, &locerror);
	if (!message)
		goto error;

	if (!osync_marshal_change(message, change, &locerror))
		goto error_free_message;

	if (!osync_queue_send_message(client->outgoing, NULL, message, &locerror))
		goto error_free_message;
	
	osync_message_unref(message);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
	
 error_free_message:
	osync_message_unref(message);
 error:
	_free_baton(baton);
	osync_client_error_shutdown(client, locerror);
	osync_error_unref(&locerror);
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
}

static void _osync_client_read_callback(void *data, OSyncError *error)
{
	OSyncError *locerror = NULL;
	callContext *baton = NULL;
	OSyncMessage *message = NULL;
	OSyncClient *client = NULL;
	OSyncMessage *reply = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, data, error);
	baton = data;

	message = baton->message;
	client = baton->client;

	osync_trace(TRACE_INTERNAL, "ignored chnaged: %p", baton->change);

	if (!osync_error_is_set(&error)) {
		reply = osync_message_new_reply(message, &locerror);
		if (!reply)
			goto error;

		//Send get_changes specific reply data
		if (!osync_message_write_string(reply, osync_change_get_uid(baton->change), &locerror))
			goto error;
	} else {
		reply = osync_message_new_errorreply(message, error, &locerror);
	}
	if (!reply)
		goto error;
	osync_trace(TRACE_INTERNAL, "Reply id %lli", osync_message_get_id(reply));


	if (!osync_queue_send_message(client->outgoing, NULL, reply, &locerror))
		goto error_free_message;

	_osync_client_ignored_conflict_callback(baton->change, baton);

	_free_baton(baton);

	osync_message_unref(reply);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
	
 error_free_message:
	osync_message_unref(reply);
 error:
	_free_baton(baton);
	osync_client_error_shutdown(client, locerror);
	osync_error_unref(&locerror);
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
}

static void _osync_client_commit_change_callback(void *data, OSyncError *error)
{
	OSyncError *locerror = NULL;
	callContext *baton = NULL;
	OSyncMessage *message = NULL;
	OSyncClient *client = NULL;
	OSyncMessage *reply = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, data, error);
	baton = data;

	message = baton->message;
	client = baton->client;

	if (!osync_error_is_set(&error)) {
		reply = osync_message_new_reply(message, &locerror);
		if (!reply)
			goto error;

		//Send get_changes specific reply data
		if (!osync_message_write_string(reply, osync_change_get_uid(baton->change), &locerror))
			goto error_free_message;
	} else {
		reply = osync_message_new_errorreply(message, error, &locerror);
	}
	if (!reply)
		goto error;
	osync_trace(TRACE_INTERNAL, "Reply id %lli", osync_message_get_id(reply));

	_free_baton(baton);
	
	if (!osync_queue_send_message(client->outgoing, NULL, reply, &locerror))
		goto error_free_message;
	
	osync_message_unref(reply);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
	
 error_free_message:
	osync_message_unref(reply);
 error:
	_free_baton(baton);
	osync_client_error_shutdown(client, locerror);
	osync_error_unref(&locerror);
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
}

static void _osync_client_committed_all_callback(void *data, OSyncError *error)
{
	OSyncError *locerror = NULL;
	callContext *baton = NULL;
	OSyncMessage *message = NULL;
	OSyncClient *client = NULL;
	OSyncMessage *reply = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, data, error);
	baton = data;

	message = baton->message;
	client = baton->client;

	if (!osync_error_is_set(&error)) {
		reply = osync_message_new_reply(message, &locerror);
		//Send get_changes specific reply data
	} else {
		reply = osync_message_new_errorreply(message, error, &locerror);
	}
	if (!reply)
		goto error;
	osync_trace(TRACE_INTERNAL, "Reply id %lli", osync_message_get_id(reply));

	_free_baton(baton);
	
	if (!osync_queue_send_message(client->outgoing, NULL, reply, &locerror))
		goto error_free_message;
	
	osync_message_unref(reply);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
	
 error_free_message:
	osync_message_unref(reply);
 error:
	_free_baton(baton);
	osync_client_error_shutdown(client, locerror);
	osync_error_unref(&locerror);
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
}

static void _osync_client_sync_done_callback(void *data, OSyncError *error)
{
	OSyncError *locerror = NULL;
	callContext *baton = NULL;
	OSyncMessage *message = NULL;
	OSyncClient *client = NULL;
	OSyncMessage *reply = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, data, error);
	baton = data;

	message = baton->message;
	client = baton->client;

	if (!osync_error_is_set(&error)) {
		reply = osync_message_new_reply(message, &locerror);
		//Send get_changes specific reply data
	} else {
		reply = osync_message_new_errorreply(message, error, &locerror);
	}
	if (!reply)
		goto error;
	osync_trace(TRACE_INTERNAL, "Reply id %lli", osync_message_get_id(reply));

	_free_baton(baton);
	
	if (!osync_queue_send_message(client->outgoing, NULL, reply, &locerror))
		goto error_free_message;
	
	osync_message_unref(reply);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
	
 error_free_message:
	osync_message_unref(reply);
 error:
	_free_baton(baton);
	osync_client_error_shutdown(client, locerror);
	osync_error_unref(&locerror);
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
}

static osync_bool _osync_client_handle_initialize(OSyncClient *client, OSyncMessage *message, OSyncError **error)
{
	OSyncMessage *reply = NULL;
	char *enginepipe = NULL;
	char *pluginname = NULL;
	char *plugindir = NULL;
	char *groupname = NULL;
	char *configdir = NULL;
	char *formatdir = NULL;
	int haspluginconfig = 0;
	OSyncPluginConfig *config = NULL;
	OSyncQueue *outgoing = NULL;
	OSyncList *r = NULL;
	OSyncPluginResource *res = NULL;
	OSyncObjTypeSink *sink = NULL, *main_sink = NULL;
	const char *objtype = NULL;
	const char *preferred_format = NULL;
	OSyncList *o = NULL;
	OSyncList *objtypesinks = NULL;
	OSyncList *objformats = NULL;
	OSyncList *list;
	OSyncObjFormatSink *format_sink = NULL;
	osync_bool couldinit;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, client, message, error);
	
	osync_message_read_string(message, &enginepipe, error);
	osync_message_read_string(message, &formatdir, error);
	osync_message_read_string(message, &plugindir, error);
	osync_message_read_string(message, &pluginname, error);
	osync_message_read_string(message, &groupname, error);
	osync_message_read_string(message, &configdir, error);
	osync_message_read_int(message, &haspluginconfig, error);

	if (osync_error_is_set(error))
		goto error;

	if (haspluginconfig && !osync_demarshal_pluginconfig(message, &config, error))
		goto error;
	
	osync_trace(TRACE_INTERNAL, "enginepipe %s, formatdir %s, plugindir %s, pluginname %s", enginepipe, formatdir, plugindir, pluginname);
		
	/* First we connect the engine pipe if necessary*/
	if (enginepipe) {
		outgoing = osync_queue_new(enginepipe, error);
		if (!outgoing)
			goto error;
		
		osync_trace(TRACE_INTERNAL, "connecting to engine");
		
		if (!osync_queue_connect(outgoing, OSYNC_QUEUE_SENDER, error)) {
			osync_queue_unref(outgoing);
			goto error;
		}
		
		if (!osync_client_set_outgoing_queue(client, outgoing, error))
			goto error;

		osync_queue_cross_link(client->incoming, client->outgoing);
		osync_queue_unref(outgoing);
		osync_trace(TRACE_INTERNAL, "done connecting to engine");
	}
	
	if (!client->plugin) {
		client->plugin_env = osync_plugin_env_new(error);
		if (!client->plugin_env)
			goto error;
		
		if (!osync_plugin_env_load(client->plugin_env, plugindir, error))
			goto error;
	
		client->plugin = osync_plugin_env_find_plugin(client->plugin_env, pluginname);
		if (!client->plugin) {
			osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to find plugin %s", pluginname);
			goto error;
		}
		osync_plugin_ref(client->plugin);
	}
	
	client->format_env = osync_format_env_new(error);
	if (!client->format_env)
		goto error;
	
	if (!osync_format_env_load_plugins(client->format_env, formatdir, error))
		goto error;
	
	client->plugin_info = osync_plugin_info_new(error);
	if (!client->plugin_info)
		goto error;
	
	osync_plugin_info_set_configdir(client->plugin_info, configdir);
	osync_plugin_info_set_loop(client->plugin_info, client->context);
	osync_plugin_info_set_format_env(client->plugin_info, client->format_env);
	osync_plugin_info_set_groupname(client->plugin_info, groupname);

	if (config)
		osync_plugin_info_set_config(client->plugin_info, config);
	
#ifdef OPENSYNC_UNITTESTS
	{
		osync_memberid memberid;
		if (!osync_message_read_int(message, &memberid, error)) // Introduced (only) for testing/debugging purpose (mock-sync)
			goto error;

		client->plugin_info->memberid = memberid;
	}
#endif	

	/* Enable active sinks */

	if (config)
		r = osync_plugin_config_get_resources(config);

	for (; r; r = r->next) {
		res = r->data;

		/* Don't add disabled Resources to PluginInfo objtypes list: #817 */
		if (!osync_plugin_resource_is_enabled(res))
			continue;

		objtype = osync_plugin_resource_get_objtype(res); 
		/* Check for ObjType sink */
		if (!(sink = osync_plugin_info_find_objtype(client->plugin_info, objtype))) {
			sink = osync_objtype_sink_new(objtype, error);
			if (!sink)
				goto error_finalize;

			osync_plugin_info_add_objtype(client->plugin_info, sink);
			osync_objtype_sink_unref(sink);
		} else {
			osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "Duplicate sink objtype \"%s\" configured in plugin %s", objtype, pluginname);
			goto error;
		}

		preferred_format = osync_plugin_resource_get_preferred_format(res);
		osync_objtype_sink_set_preferred_format(sink, preferred_format); 
		objformats = osync_plugin_resource_get_objformat_sinks(res);
		for (o = objformats; o; o = o->next) {
			format_sink = (OSyncObjFormatSink *) o->data; 
			osync_objtype_sink_add_objformat_sink(sink, format_sink);
		}
		osync_list_free(objformats);
	}
	
	couldinit = osync_plugin_initialize(client->plugin, &(client->plugin_data), client->plugin_info, error);
	if (!couldinit) {
		if (!osync_error_is_set(error))
			osync_error_set(error, OSYNC_ERROR_GENERIC, "Plugin \"%s\" failed to initialize but gave no reason", pluginname);
		goto error;
	}

	objtypesinks = osync_plugin_info_get_objtype_sinks(client->plugin_info);
	list = objtypesinks;
	while (list) {
		sink = (OSyncObjTypeSink*)list->data;
		if (!osync_objtype_sink_open_state_db(sink, client->plugin_info, error)) {
			goto error_finalize;
		}

		if (!osync_objtype_sink_load_hashtable(sink, client->plugin_info, error)) {
			goto error_finalize;
		}
		list = list->next;
	}
	osync_list_free(objtypesinks);

	main_sink = osync_plugin_info_get_main_sink(client->plugin_info);
	if (main_sink) {
		if (!osync_objtype_sink_open_state_db(main_sink,
					client->plugin_info, error))
			goto error_finalize;

	}

	reply = osync_message_new_reply(message, error);
	if (!reply)
		goto error_finalize;

	/* Marshal available ObjType sinks */
	if (!osync_marshal_objtype_sinks(reply, client, FALSE, error))
		goto error_free_message;

	if (!osync_queue_send_message(client->outgoing, NULL, reply, error))
		goto error_free_message;
	
	osync_message_unref(reply);
		
	osync_free(enginepipe);
	osync_free(pluginname);
	osync_free(configdir);
	osync_free(plugindir);
	osync_free(groupname);
	osync_free(formatdir);
	
	if (config)
		osync_plugin_config_unref(config);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
	
 error_free_message:
	osync_message_unref(reply);
 error_finalize:
	osync_list_free(objtypesinks);
	osync_plugin_finalize(client->plugin, client->plugin_data);
 error:
	osync_free(enginepipe);
	osync_free(pluginname);
	osync_free(configdir);
	osync_free(plugindir);
	osync_free(groupname);
	osync_free(formatdir);

	if (config)
		osync_plugin_config_unref(config);

	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool _osync_client_handle_finalize(OSyncClient *client, OSyncMessage *message, OSyncError **error)
{
	OSyncMessage *reply = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, client, message, error);
	
	if (client->plugin) {
		if (client->plugin_data)
			osync_plugin_finalize(client->plugin, client->plugin_data);
		
		osync_plugin_unref(client->plugin);
		client->plugin = NULL;
	}
	
	if (client->plugin_env) {
		osync_plugin_env_unref(client->plugin_env);
		client->plugin_env = NULL;
	}
	
	if (client->plugin_info) {
		osync_plugin_info_unref(client->plugin_info);
		client->plugin_info = NULL;
	}
	
	if (client->format_env) {
		osync_format_env_unref(client->format_env);
		client->format_env = NULL;
	}
	
	if (!client->outgoing) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "No outgoing queue yet");
		goto error;
	}
	
	reply = osync_message_new_reply(message, NULL);
	if (!reply)
		goto error;

	if (!osync_queue_send_message(client->outgoing, NULL, reply, NULL))
		goto error_free_message;
	
	osync_message_unref(reply);
		
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
	
 error_free_message:
	osync_message_unref(reply);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}


static osync_bool _osync_client_handle_discover(OSyncClient *client, OSyncMessage *message, OSyncError **error)
{
	OSyncMessage *reply = NULL;
	OSyncPluginConfig *config = NULL;
	OSyncList *res = NULL;
	unsigned int num_res = 0;
	OSyncPluginResource *resource = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, client, message, error);

	config = osync_plugin_info_get_config(client->plugin_info);
	res = osync_plugin_config_get_resources(config);
	
	if (!osync_plugin_discover(client->plugin, client->plugin_data, client->plugin_info, error))
		goto error;

	reply = osync_message_new_reply(message, error);
	if (!reply)
		goto error;

	if (osync_plugin_info_get_main_sink(client->plugin_info))
		osync_message_write_int(reply, 1, error);
	else
		osync_message_write_int(reply, 0, error);

	if (osync_error_is_set(error))
		goto error_free_message;

	/* Marshal available ObjType sinks */
	if (!osync_marshal_objtype_sinks(reply, client, TRUE, error))
		goto error_free_message;

	if (!_osync_client_handle_capabilities_message(client, reply, error))
		goto error_free_message;

	/* Report, detected Resources */
	res = osync_plugin_config_get_resources(config);
	num_res = osync_list_length(res);

	if (!osync_message_write_uint(reply, num_res, error))
		goto error;

	for (; res; res = res->next) {
		resource = res->data;
		if (!osync_marshal_pluginresource(reply, resource, error))
			goto error_free_message;
	}

	if (!osync_queue_send_message(client->outgoing, NULL, reply, error))
		goto error_free_message;
	
	osync_message_unref(reply);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
	
 error_free_message:
	osync_message_unref(reply);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool _osync_client_handle_connect(OSyncClient *client, OSyncMessage *message, OSyncError **error)
{
	char *objtype = NULL;
	int slowsync;
	OSyncMessage *reply = NULL;
	OSyncObjTypeSink *sink = NULL;
	OSyncContext *context = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, client, message, error);

	/* The message content is read and the objtype is written back immediately
	 * because the connect callback needs a safe way to determine the correct
	 * sink. The current sink of the plugin info is usually not correct
	 * because the connect functions work asynchronous and the last sink
	 * (usually the main sink) is the current sink.
	 */
	osync_message_read_string(message, &objtype, error);
	osync_message_read_int(message, &slowsync, error);
	osync_message_write_string(message, objtype, error);

	if (osync_error_is_set(error))
		goto error;

	osync_trace(TRACE_INTERNAL, "Searching sink for %s", objtype);
	
	if (objtype) {
		sink = osync_plugin_info_find_objtype(client->plugin_info, objtype);
		
		if (!sink) {
			osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to find sink for %s", objtype);
			osync_free(objtype);
			goto error;
		}
		
		osync_free(objtype);
	} else
		sink = osync_plugin_info_get_main_sink(client->plugin_info);
		
	if (!sink) {
		reply = osync_message_new_reply(message, error);
		if (!reply)
			goto error;
		
		/* SlowSync dummy value for connect reply message handler. */
		if (!osync_message_write_int(reply, FALSE, error))
			goto error_free_reply;

		if (!osync_queue_send_message(client->outgoing, NULL, reply, error))
			goto error_free_reply;
		
		osync_message_unref(reply);
	} else {
               /* set slowsync. but never disable it */
               if (slowsync)
                       osync_objtype_sink_set_slowsync(sink, TRUE);

		context = _create_context(client, message, _osync_client_connect_callback, NULL, error);
		if (!context)
			goto error;

		osync_context_set_slowsync_callback(context, _osync_client_slowsync_callback, sink);

		osync_plugin_info_set_sink(client->plugin_info, sink);
		osync_objtype_sink_connect(sink, client->plugin_info, context);

		osync_context_unref(context);
	}

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_free_reply:
	osync_message_unref(reply);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool _osync_client_handle_connect_done(OSyncClient *client, OSyncMessage *message, OSyncError **error)
{
	char *objtype = NULL;
	int slowsync;
	OSyncMessage *reply = NULL;
	OSyncObjTypeSink *sink = NULL;
	OSyncContext *context = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, client, message, error);
	
	osync_message_read_string(message, &objtype, error);
	osync_message_read_int(message, &slowsync, error);

	if (osync_error_is_set(error))
		goto error;

	osync_trace(TRACE_INTERNAL, "Searching sink for %s (slowsync: %d)", objtype, slowsync);
	
	if (objtype) {
		sink = osync_plugin_info_find_objtype(client->plugin_info, objtype);
		
		if (!sink) {
			osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to find sink for %s", objtype);
			osync_free(objtype);
			goto error;
		}
		
		osync_free(objtype);
	} else
		sink = osync_plugin_info_get_main_sink(client->plugin_info);
		
	if (!sink) {
		reply = osync_message_new_reply(message, error);
		if (!reply)
			goto error;
		
		if (!osync_queue_send_message(client->outgoing, NULL, reply, error))
			goto error_free_reply;
		
		osync_message_unref(reply);
	} else {
		context = _create_context(client, message, _osync_client_connect_done_callback, NULL, error);
		if (!context)
			goto error;

		/* set slowsync (again) if the slow-sync got requested during the connect() call
			 of a member - and not during frontend/engine itself (e.g. anchor mismatch).
			 Never disable a slowsync.*/
		if (slowsync)
			osync_objtype_sink_set_slowsync(sink, TRUE);
		
		osync_plugin_info_set_sink(client->plugin_info, sink);
		osync_objtype_sink_connect_done(sink, client->plugin_info, context);
	
		osync_context_unref(context);
	}
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_free_reply:
	osync_message_unref(reply);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool _osync_client_handle_disconnect(OSyncClient *client, OSyncMessage *message, OSyncError **error)
{
	char *objtype = NULL;
	OSyncMessage *reply = NULL;
	OSyncObjTypeSink *sink = NULL;
	OSyncContext *context = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, client, message, error);
	
	if (!osync_message_read_string(message, &objtype, error))
		goto error;

	osync_trace(TRACE_INTERNAL, "Searching sink for %s", objtype);
	
	if (objtype) {
		sink = osync_plugin_info_find_objtype(client->plugin_info, objtype);
		
		if (!sink) {
			osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to find sink for %s", objtype);
			osync_free(objtype);
			goto error;
		}
		
		osync_free(objtype);
	} else
		sink = osync_plugin_info_get_main_sink(client->plugin_info);
		
	if (!sink) {
		reply = osync_message_new_reply(message, error);
		if (!reply)
			goto error;
		
		if (!osync_queue_send_message(client->outgoing, NULL, reply, error))
			goto error_free_reply;
		
		osync_message_unref(reply);
	} else {
		context = _create_context(client, message, _osync_client_disconnect_callback, NULL, error);
		if (!context)
			goto error;
		
		osync_plugin_info_set_sink(client->plugin_info, sink);
		osync_objtype_sink_disconnect(sink, client->plugin_info, context);
	
		osync_context_unref(context);
	}
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_free_reply:
	osync_message_unref(reply);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool _osync_client_handle_get_changes(OSyncClient *client, OSyncMessage *message, OSyncError **error)
{
	int slowsync;
	char *objtype = NULL;
	OSyncMessage *reply = NULL;
	OSyncObjTypeSink *sink = NULL;
	OSyncContext *context = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, client, message, error);
	
	osync_message_read_string(message, &objtype, error);
	osync_message_read_int(message, &slowsync, error);

	if (osync_error_is_set(error))
		goto error;

	osync_trace(TRACE_INTERNAL, "Searching sink for %s (slowsync: %i)", objtype, slowsync);
	
	if (objtype) {
		sink = osync_plugin_info_find_objtype(client->plugin_info, objtype);
		
		if (!sink) {
			osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to find sink for %s", objtype);
			osync_free(objtype);
			goto error;
		}
		
		osync_free(objtype);
	} else
		sink = osync_plugin_info_get_main_sink(client->plugin_info);
		
	if (!sink) {
		reply = osync_message_new_reply(message, error);
		if (!reply)
			goto error;
		
		if (!osync_queue_send_message(client->outgoing, NULL, reply, error))
			goto error_free_reply;
		
		osync_message_unref(reply);
	} else {
		context = _create_context(client, message, _osync_client_get_changes_callback, NULL, error);
		if (!context)
			goto error;
		osync_context_set_changes_callback(context, _osync_client_change_callback);
		
		osync_plugin_info_set_sink(client->plugin_info, sink);

		osync_objtype_sink_get_changes(sink, client->plugin_info, slowsync, context);
	
		osync_context_unref(context);
	}
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_free_reply:
	osync_message_unref(reply);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool _osync_client_handle_read_change(OSyncClient *client, OSyncMessage *message, OSyncError **error)
{
	const char *objtype = NULL;
	OSyncMessage *reply = NULL;
	OSyncChange *change = NULL;
	OSyncObjTypeSink *sink = NULL;
	OSyncContext *context = NULL;


	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, client, message, error);

	if (!osync_demarshal_change(message, &change, client->format_env, error))
		goto error;
		
	osync_trace(TRACE_INTERNAL, "Change %p", change);

	objtype = osync_data_get_objtype(osync_change_get_data(change));

	if (objtype) {
		sink = osync_plugin_info_find_objtype(client->plugin_info, objtype);
		
		if (!sink) {
			osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to find sink for %s", objtype);
			goto error;
		}
	} else {
		sink = osync_plugin_info_get_main_sink(client->plugin_info);
	}
		
	if (!sink) {
		reply = osync_message_new_reply(message, error);
		if (!reply)
			goto error;
		
		if (!osync_queue_send_message(client->outgoing, NULL, reply, error))
			goto error_free_reply;
		
		osync_message_unref(reply);
	} else {
		context = _create_context(client, message, _osync_client_read_callback, change, error);
		if (!context)
			goto error;
		
		osync_plugin_info_set_sink(client->plugin_info, sink);

		osync_objtype_sink_read_change(sink, client->plugin_info, change, context);
	
		osync_context_unref(context);
	}

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_free_reply:
	osync_message_unref(reply);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}


static osync_bool _osync_client_handle_commit_change(OSyncClient *client, OSyncMessage *message, OSyncError **error)
{
	OSyncChange *change = NULL;
	OSyncData *data = NULL;
	OSyncObjTypeSink *sink = NULL;
	OSyncContext *context = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, client, message, error);
	
	if (!osync_demarshal_change(message, &change, client->format_env, error))
		goto error;
		
	osync_trace(TRACE_INTERNAL, "Change %p", change);
	
	data = osync_change_get_data(change);
	
	osync_trace(TRACE_INTERNAL, "Searching sink for %s", osync_data_get_objtype(data));
	
	sink = osync_plugin_info_find_objtype(client->plugin_info, osync_data_get_objtype(data));
	
	if (!sink) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to find sink for %s", osync_data_get_objtype(data));
		osync_change_unref(change);
		goto error;
	}
		
	context = _create_context(client, message, _osync_client_commit_change_callback, change, error);
	if (!context)
		goto error;
		
	osync_plugin_info_set_sink(client->plugin_info, sink);
	osync_objtype_sink_commit_change(sink, client->plugin_info, change, context);
	
	osync_change_unref(change);
	osync_context_unref(context);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool _osync_client_handle_committed_all(OSyncClient *client, OSyncMessage *message, OSyncError **error)
{
	char *objtype = NULL;
	OSyncMessage *reply = NULL;
	OSyncObjTypeSink *sink = NULL;
	OSyncContext *context = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, client, message, error);

	if (!osync_message_read_string(message, &objtype, error))
		goto error;

	osync_trace(TRACE_INTERNAL, "Searching sink for %s", objtype ? objtype : "(MainSink)");

	if (objtype) {
		sink = osync_plugin_info_find_objtype(client->plugin_info, objtype);
		
		if (!sink) {
			osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to find sink for %s", objtype);
			osync_free(objtype);
			goto error;
		}
		
		osync_free(objtype);
	} else
		sink = osync_plugin_info_get_main_sink(client->plugin_info);
		
	if (!sink) {
		reply = osync_message_new_reply(message, error);
		if (!reply)
			goto error;
		
		if (!osync_queue_send_message(client->outgoing, NULL, reply, error))
			goto error_free_reply;
		
		osync_message_unref(reply);
	} else {
		context = _create_context(client, message, _osync_client_committed_all_callback, NULL, error);
		if (!context)
			goto error;
		
		osync_plugin_info_set_sink(client->plugin_info, sink);
		osync_objtype_sink_committed_all(sink, client->plugin_info, context);
	
		osync_context_unref(context);
	}
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_free_reply:
	osync_message_unref(reply);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool _osync_client_handle_sync_done(OSyncClient *client, OSyncMessage *message, OSyncError **error)
{
	char *objtype = NULL;
	OSyncMessage *reply = NULL;
	OSyncObjTypeSink *sink = NULL;
	OSyncContext *context = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, client, message, error);
	
	if (!osync_message_read_string(message, &objtype, error))
		goto error;

	osync_trace(TRACE_INTERNAL, "Searching sink for %s", objtype);
	
	if (objtype) {
		sink = osync_plugin_info_find_objtype(client->plugin_info, objtype);
		
		if (!sink) {
			osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to find sink for %s", objtype);
			osync_free(objtype);
			goto error;
		}
		
		osync_free(objtype);
	} else
		sink = osync_plugin_info_get_main_sink(client->plugin_info);
		
	if (!sink) {
		reply = osync_message_new_reply(message, error);
		if (!reply)
			goto error;
		
		if (!osync_queue_send_message(client->outgoing, NULL, reply, error))
			goto error_free_reply;
		
		osync_message_unref(reply);
	} else {
		context = _create_context(client, message, _osync_client_sync_done_callback, NULL, error);
		if (!context)
			goto error;
		
		osync_context_set_uid_update_callback(context, _osync_client_uid_update_callback, sink, client);

		osync_plugin_info_set_sink(client->plugin_info, sink);
		osync_objtype_sink_sync_done(sink, client->plugin_info, context);
	
		osync_context_unref(context);

		if (!osync_objtype_sink_save_hashtable(sink, error))
			goto error;

		/* Reset slow-sync state. The sink is here over.
		 * Reset is requried for a multi-sync, without finalization.*/
		osync_objtype_sink_set_slowsync(sink, FALSE);
	}
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_free_reply:
	osync_message_unref(reply);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static void _osync_client_message_handler(OSyncMessage *message, void *user_data)
{
	OSyncClient *client = NULL;
	OSyncError *error = NULL;
	OSyncError *locerror = NULL;
	OSyncMessage *errorreply = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);
	client = user_data;
	osync_trace(TRACE_INTERNAL, "plugin received command %i (%s)", osync_message_get_command(message), osync_message_get_commandstr(message));

	switch (osync_message_get_command(message)) {
	case OSYNC_MESSAGE_NOOP:
	case OSYNC_MESSAGE_REPLY:
	case OSYNC_MESSAGE_ERRORREPLY:
	case OSYNC_MESSAGE_NEW_CHANGE:
	case OSYNC_MESSAGE_SYNCHRONIZE:
	case OSYNC_MESSAGE_ENGINE_CHANGED:
	case OSYNC_MESSAGE_MAPPING_CHANGED:
	case OSYNC_MESSAGE_MAPPINGENTRY_CHANGED:
		//Ignore these. They dont have any meaning to the client
		break;
	case OSYNC_MESSAGE_QUEUE_ERROR:
	case OSYNC_MESSAGE_ERROR:
	case OSYNC_MESSAGE_QUEUE_HUP:
		/* Handle disconnect here */
		break;
			
	case OSYNC_MESSAGE_INITIALIZE:
		if (!_osync_client_handle_initialize(client, message, &error))
			goto error;
		break;
			
	case OSYNC_MESSAGE_FINALIZE:
		if (!_osync_client_handle_finalize(client, message, &error))
			goto error;
		break;
			
	case OSYNC_MESSAGE_DISCOVER:
		if (!_osync_client_handle_discover(client, message, &error))
			goto error;
		break;
			
	case OSYNC_MESSAGE_CONNECT:
		if (!_osync_client_handle_connect(client, message, &error))
			goto error;
		break;

	case OSYNC_MESSAGE_CONNECT_DONE:
		if (!_osync_client_handle_connect_done(client, message, &error))
			goto error;
		break;
	
	case OSYNC_MESSAGE_DISCONNECT:
		if (!_osync_client_handle_disconnect(client, message, &error))
			goto error;
		break;
			
	case OSYNC_MESSAGE_GET_CHANGES:
		if (!_osync_client_handle_get_changes(client, message, &error))
			goto error;
		break;
	
	case OSYNC_MESSAGE_COMMIT_CHANGE:
		if (!_osync_client_handle_commit_change(client, message, &error))
			goto error;
		break;
			
	case OSYNC_MESSAGE_SYNC_DONE:
		if (!_osync_client_handle_sync_done(client, message, &error))
			goto error;
		break;
			
	case OSYNC_MESSAGE_COMMITTED_ALL:
		if (!_osync_client_handle_committed_all(client, message, &error))
			goto error;
		break;
	
	case OSYNC_MESSAGE_READ_CHANGE:
		if (!_osync_client_handle_read_change(client, message, &error))
			goto error;
		break;
	
	case OSYNC_MESSAGE_CALL_PLUGIN:
		/*
			char *function = itm_message_get_data(message, "function");
			void *data = itm_message_get_data(message, "data");
			OSyncError *error = NULL;
			void *replydata = osync_member_call_plugin(client->member, function, data, &error);
	
			if (itm_message_get_data(message, "want_reply")) {
			ITMessage *reply = NULL;
			if (!osync_error_is_set(&error)) {
			reply = itm_message_new_methodreply(client, message);
			itm_message_set_data(message, "reply", replydata);
			} else {
			reply = itm_message_new_errorreply(client, message);
			itm_message_set_error(reply, error);
			}
	
			itm_message_send_reply(reply);
			}
		*/
		break;
	}

	osync_trace(TRACE_EXIT, "%s", __func__);
	return;

 error:;
	if (!client->outgoing) {
		client->thread = NULL;
		osync_client_shutdown(client);
		osync_trace(TRACE_EXIT_ERROR, "%s: Unable to notify parent. no outgoing queue: %s", __func__, osync_error_print(&error));
		osync_error_unref(&error);
		return;
	}

	errorreply = osync_message_new_errorreply(message, error, &locerror);
	if (!errorreply) {
		osync_client_error_shutdown(client, locerror);
		osync_error_unref(&error);
		osync_trace(TRACE_EXIT_ERROR, "%s: Error while sending error: %s", __func__, osync_error_print(&locerror));
		osync_error_unref(&locerror);
		return;
	}

	if (!osync_queue_send_message(client->outgoing, NULL, errorreply, &locerror)) {
		osync_client_error_shutdown(client, locerror);
		osync_error_unref(&error);
		osync_trace(TRACE_EXIT_ERROR, "%s: Error while sending error: %s", __func__, osync_error_print(&locerror));
		osync_error_unref(&locerror);
		return;
	}

	osync_message_unref(errorreply);

	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(&error));
	osync_error_unref(&error);
}

/** This function takes care of the messages received on the outgoing (sending)
 * queue. The only messages we can receive there, are HUPs or ERRORs. */
static void _osync_client_hup_handler(OSyncMessage *message, void *user_data)
{
	OSyncError *error = NULL;
	OSyncClient *client = NULL;
	client = user_data;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);

	osync_trace(TRACE_INTERNAL, "plugin received command %i on sending queue", osync_message_get_command(message));

	if ( (osync_message_get_command(message) == OSYNC_MESSAGE_QUEUE_ERROR)
		|| (osync_message_get_command(message) == OSYNC_MESSAGE_QUEUE_HUP) ) {
		/* The remote side disconnected. So we can now disconnect as well and then
		 * shutdown */
		if (!osync_queue_disconnect(client->outgoing, &error))
			osync_error_unref(&error);
		
		if (!osync_queue_disconnect(client->incoming, &error))
			osync_error_unref(&error);
		
		if (client->syncloop) {
			g_main_loop_quit(client->syncloop);
		}
	} else {
		/* This should never ever happen */
		osync_trace(TRACE_ERROR, "received neither a hup, nor a error on a sending queue...");
	}

	osync_trace(TRACE_EXIT, "%s", __func__);
	return;
}

OSyncClient *osync_client_new(OSyncError **error)
{
	OSyncClient *client = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, error);
	
	client = osync_try_malloc0(sizeof(OSyncClient), error);
	if (!client) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
		return NULL;
	}
	
	client->ref_count = 1;
	client->context = g_main_context_new();
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, client);
	return client;
}

OSyncClient *osync_client_ref(OSyncClient *client)
{
	osync_assert(client);
	
	g_atomic_int_inc(&(client->ref_count));

	return client;
}

void osync_client_unref(OSyncClient *client)
{
	osync_assert(client);
	
	if (g_atomic_int_dec_and_test(&(client->ref_count))) {
		osync_trace(TRACE_ENTRY, "%s(%p)", __func__, client);
		
		if(client->disconnectThread) {
			g_thread_join(client->disconnectThread);
			client->disconnectThread = NULL;
		}

		if (client->incoming) {
			if (osync_queue_is_connected(client->incoming))
				osync_queue_disconnect(client->incoming, NULL);
			osync_queue_remove(client->incoming, NULL);
			osync_queue_unref(client->incoming);
		}
	
		if (client->outgoing) {
			if (osync_queue_is_connected(client->outgoing))
				osync_queue_disconnect(client->outgoing, NULL);
			osync_queue_unref(client->outgoing);
		}
		
		if (client->plugin)
			osync_plugin_unref(client->plugin);
		
		if (client->thread)
			osync_thread_unref(client->thread);
		
		osync_free(client);
		
		osync_trace(TRACE_EXIT, "%s", __func__);
	}
}

void osync_client_set_plugin(OSyncClient *client, OSyncPlugin *plugin)
{
	osync_return_if_fail(client);
	osync_return_if_fail(plugin);

	if (client->plugin)
		osync_plugin_unref(client->plugin);


	client->plugin = osync_plugin_ref(plugin);
}

OSyncPlugin *osync_client_get_plugin(OSyncClient *client)
{
	osync_return_val_if_fail(client, NULL);
	return client->plugin;
}

void osync_client_set_pipe_path(OSyncClient *client, const char *pipe_path)
{
	osync_return_if_fail(client);
	osync_return_if_fail(pipe_path);

	if (client->pipe_path)
		osync_free(client->pipe_path);


	client->pipe_path = osync_strdup(pipe_path);
}

const char *osync_client_get_pipe_path(OSyncClient *client)
{
	osync_return_val_if_fail(client, NULL);
	return client->pipe_path;
}

osync_bool osync_client_set_incoming_queue(OSyncClient *client, OSyncQueue *incoming, OSyncError **error)
{
	osync_queue_set_message_handler(incoming, _osync_client_message_handler, client);

	if (!osync_queue_setup_with_gmainloop(incoming, client->context, error))
		goto error;

	client->incoming = osync_queue_ref(incoming);

	osync_queue_set_pending_limit(incoming, OSYNC_QUEUE_PENDING_LIMIT);

	return TRUE;

error:
	return FALSE;
}

osync_bool osync_client_set_outgoing_queue(OSyncClient *client, OSyncQueue *outgoing, OSyncError **error)
{
	osync_queue_set_message_handler(outgoing, _osync_client_hup_handler, client);
	if (!osync_queue_setup_with_gmainloop(outgoing, client->context, error))
		goto error;

	client->outgoing = osync_queue_ref(outgoing);

	return TRUE;
error:
	return FALSE;
}

osync_bool osync_client_run_and_block(OSyncClient *client, OSyncError **error)
{

	if (!osync_client_setup_pipes(client, NULL, error))
		goto error;

	if (!osync_client_connect_pipes(client, error))
		goto error;
	
	client->syncloop = g_main_loop_new(client->context, TRUE);
	g_main_loop_run(client->syncloop);

	return TRUE;

error:
	return FALSE;
}

osync_bool osync_client_run(OSyncClient *client, OSyncError **error)
{
	if (!osync_client_setup_pipes(client, NULL, error))
		goto error;

	if (!osync_client_connect_pipes(client, error))
		goto error;

	client->thread = osync_thread_new(client->context, error);
	if (!client->thread)
		goto error;
		
	osync_thread_start(client->thread);
	
	return TRUE;

error:
	return FALSE;
}

osync_bool osync_client_setup_pipes(OSyncClient *client, const char *pipe_path, OSyncError **error)
{
	OSyncQueue *incoming = NULL;

	const char *path = NULL;

	if (pipe_path) {
		path = pipe_path;
	} else if (client->pipe_path) {
		path = client->pipe_path;
	} else {
		/* If there are no pipes - which is perfectly fine - this is just NOP */
		return TRUE;
	}

	/* Create connection pipes **/
	incoming = osync_queue_new(path, error);
	if (!incoming)
		goto error;
	
	if (!osync_queue_create(incoming, error))
		goto error_free_queue;
	
	if (!osync_client_set_incoming_queue(client, incoming, error))
		goto error_remove_queue;

	osync_queue_unref(incoming);

	return TRUE;

error_remove_queue:
	osync_queue_remove(incoming, NULL);
error_free_queue:
	osync_queue_unref(incoming);
error:
	return FALSE;
}

osync_bool osync_client_connect_pipes(OSyncClient *client, OSyncError **error)
{
	/* Nop pipes are perfeclty fine - this function does NOP in this case */
	if (!client->incoming)
		return TRUE;

	/* Skip if queues already connected - this is different for each plugin start type */
	if (osync_queue_is_connected(client->incoming))
		return TRUE;

	/* We now connect to our incoming queue */
	return osync_queue_connect(client->incoming, OSYNC_QUEUE_RECEIVER, error);
}

static gboolean osyncClientConnectCallback(gpointer data)
{
	OSyncClient *client = NULL;
	client = data;
	osync_trace(TRACE_INTERNAL, "About to connect to the incoming queue");
	

	/* We now connect to our incoming queue */
	if (!osync_client_connect_pipes(client, NULL))
		return TRUE;
	
	return FALSE;
}


osync_bool osync_client_run_external(OSyncClient *client, char *pipe_path, OSyncPlugin *plugin, OSyncError **error)
{
	GSource *source = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %s, %p, %p)", __func__, client, pipe_path, plugin, error);

	if (!osync_client_setup_pipes(client, pipe_path, error))
		goto error;
	
	client->thread = osync_thread_new(client->context, error);
	if (!client->thread)
		goto error;
	
	osync_thread_start(client->thread);
	
	client->plugin = plugin;
	osync_plugin_ref(client->plugin);
	
	source = g_idle_source_new();
	g_source_set_callback(source, osyncClientConnectCallback, client, NULL);
	g_source_attach(source, client->context);
	

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static gboolean osyncClientDisconnectCallback(gpointer data)
{
	OSyncClient *client = data;
	
	/* First, we disconnect our incoming queue. This will generate a HUP on the remote
	 * side. We dont disconnect our outgoing queue yet, since we have to make sure that
	 * all data is read. Only the listener should disconnect a pipe! */
	osync_queue_disconnect(client->incoming, NULL);

	if (client->outgoing) {
		/* We now wait until the other side disconnect our outgoing queue */
		while (osync_queue_is_connected(client->outgoing)) { g_usleep(100); }

		/* Gives some time if anyone wants to grab the HUP message from the outgoing part of the pipe */
		g_usleep(200);	
		/* Now we can safely disconnect our outgoing queue */
		osync_queue_disconnect(client->outgoing, NULL);
	}
	
	return FALSE;
}

/* The aim of this thread is to avoid blocking the all the engine event sources with the osyncClientDisconnectCallback internal loop */
static void client_disconnect_workerthread(gpointer data) 
{
	OSyncClient *client = data;
	GMainContext *context = g_main_context_new();
	GSource *source = NULL;
	OSyncThread *thread = NULL;
	source = g_idle_source_new();
	g_source_set_callback(source, osyncClientDisconnectCallback, client, NULL);
	g_source_attach(source, context);
	thread = osync_thread_new(context, NULL);
	osync_thread_start(thread);

	osync_thread_stop(thread);
	osync_thread_unref(thread);
	thread = NULL;	
	g_source_unref(source);
}

	
void osync_client_disconnect(OSyncClient *client)
{
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, client);
	
	client->disconnectThread = g_thread_create((GThreadFunc)client_disconnect_workerthread, client, TRUE, NULL);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

void osync_client_shutdown(OSyncClient *client)
{
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, client);
	osync_assert(client);
	
	osync_client_disconnect(client);
			
	if (client->syncloop) {
		if (g_main_loop_is_running(client->syncloop)) {

			/* now we can quit the main loop */
			g_main_loop_quit(client->syncloop);
		}
		
		g_main_loop_unref(client->syncloop);
		client->syncloop = NULL;
	} else if (client->thread) {
		osync_thread_stop(client->thread);
		osync_thread_unref(client->thread);
		client->thread = NULL;
	}
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

void osync_client_error_shutdown(OSyncClient *client, OSyncError *error)
{
	OSyncMessage *message = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, client, error);

	message = osync_message_new_error(error, NULL);
	if (message) {
		osync_queue_send_message(client->outgoing, NULL, message, NULL);
		osync_message_unref(message);
	}

	osync_client_shutdown(client);

	osync_trace(TRACE_EXIT, "%s", __func__);
}


OSyncPluginInfo *osync_client_get_plugin_info(OSyncClient *client)
{
	osync_return_val_if_fail(client, NULL);
	return client->plugin_info;
}
