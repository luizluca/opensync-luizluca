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

#include "opensync_message_internals.h"

#include "opensync-data.h"
#include "opensync-format.h"
#include "format/opensync_objformat_internals.h"

#include "opensync-plugin.h"
#include "plugin/opensync_objtype_sink_internals.h"

#include "client/opensync_client_internals.h"
#include "client/opensync_client_proxy_internals.h"

#include "opensync-group.h"

#include "opensync_serializer.h"
#include "opensync_serializer_internals.h"

osync_bool osync_marshal_data(OSyncMessage *message, OSyncData *data, OSyncError **error)
{
	OSyncObjFormat *objformat = NULL;
	char *input_data = NULL;
	unsigned int input_size = 0;

	/* Order:
	 *
	 * format
	 * objtype
	 * size
	 * data */

	osync_assert(message);
	osync_assert(data);

	/* Find the format */
	objformat = osync_data_get_objformat(data);

	/* Write the format and objtype first */
	osync_message_write_string(message, osync_objformat_get_name(objformat), error);
	osync_message_write_string(message, osync_data_get_objtype(data), error);

	if (osync_error_is_set(error))
		goto error;

	/* Now we get the pointer to the data */
	osync_data_get_data(data, &input_data, &input_size);

	if (input_size > 0) {
		if (!osync_message_write_int(message, 1, error))
			goto error;

		/* If the format must be marshalled, we call the marshal function
		 * and the send the marshalled data. Otherwise we send the unmarshalled data */
		if (osync_objformat_must_marshal(objformat) == TRUE) {
			OSyncMarshal *marshal = osync_message_get_marshal(message);
			if (!osync_objformat_marshal(objformat, input_data, input_size, marshal, error))
				goto error;
		} else {
			/* If the format is a plain format, then we have to add
			 * one byte for \0 to the input_size. This extra byte will
			 * be removed by the osync_demarshal_data funciton.
			 */
			input_size++;
			if (!osync_message_write_buffer(message, input_data, input_size, error))
				goto error;
		}
	} else {
		if (!osync_message_write_int(message, 0, error))
			goto error;
	}

	return TRUE;

 error:
	return FALSE;
}

osync_bool osync_demarshal_data(OSyncMessage *message, OSyncData **data, OSyncFormatEnv *env, OSyncError **error)
{
	char *objformat = NULL;
	char *objtype = NULL;
	OSyncObjFormat *format = NULL;
	unsigned int input_size = 0;
	char *input_data = NULL;
	int has_data = 0;

	osync_assert(message);
	osync_assert(env);

	/* Order:
	 *
	 * format
	 * objtype
	 * size
	 * data */

	/* Get the objtype and format */
	osync_message_read_string(message, &objformat, error);
	osync_message_read_string(message, &objtype, error);

	if (osync_error_is_set(error))
		goto error;

	/* Search for the format */
	format = osync_format_env_find_objformat(env, objformat);
	if (!format) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to find objformat %s", objformat);
		goto error;
	}

	if (!osync_message_read_int(message, &has_data, error))
		goto error;

	if (has_data) {
		if (osync_objformat_must_marshal(format) == TRUE) {
			OSyncMarshal *marshal = osync_message_get_marshal(message);
			if (!osync_objformat_demarshal(format, marshal, &input_data, &input_size, error))
				goto error;
		} else {
			if (!osync_message_read_buffer(message, (void *)&input_data, &input_size, error))
				goto error;

			/* If the format is a plain, then we have to remove
			 * one from the input_size, since once one was added by
			 * osync_marshall_data() for trailing newline.
			 */
			input_size--;
		}
	}

	osync_trace(TRACE_INTERNAL, "Data is: %p, %i", input_data, input_size);

	*data = osync_data_new(input_data, input_size, format, error);
	if (!*data)
		goto error;

	osync_data_set_objtype(*data, objtype);
	osync_free(objtype);
	osync_free(objformat);

	return TRUE;

 error:
	osync_free(objformat);
	osync_free(objtype);
	return FALSE;
}

osync_bool osync_marshal_change(OSyncMessage *message, OSyncChange *change, OSyncError **error)
{
	OSyncData *data = NULL;
	osync_assert(message);
	osync_assert(change);

	/* Order:
	 *
	 * uid
	 * hash
	 * changetype
	 * data */

	osync_message_write_string(message, osync_change_get_uid(change), error);
	osync_message_write_string(message, osync_change_get_hash(change), error);
	osync_message_write_int(message, osync_change_get_changetype(change), error);

	if (osync_error_is_set(error))
		goto error;

	data = osync_change_get_data(change);
	if (!osync_marshal_data(message, data, error))
		goto error;

	return TRUE;

 error:
	return FALSE;
}

osync_bool osync_demarshal_change(OSyncMessage *message, OSyncChange **change, OSyncFormatEnv *env, OSyncError **error)
{
	char *uid = NULL;
	char *hash = NULL;
	int change_type = OSYNC_CHANGE_TYPE_UNKNOWN;
	OSyncData *data = NULL;

	osync_assert(message);
	osync_assert(env);

	/* Order:
	 *
	 * uid
	 * hash
	 * changetype
	 * data */

	osync_message_read_string(message, &uid, error);
	osync_message_read_string(message, &hash, error);
	osync_message_read_int(message, &change_type, error);
	if (osync_error_is_set(error))
		goto error_free;

	if (!osync_demarshal_data(message, &data, env, error))
		goto error_free;

	*change = osync_change_new(error);
	if (!*change)
		goto error_free;

	osync_change_set_uid(*change, uid);
	osync_free(uid);

	osync_change_set_hash(*change, hash);
	osync_free(hash);

	osync_change_set_changetype(*change, change_type);

	osync_change_set_data(*change, data);
	osync_data_unref(data);

	return TRUE;

 error_free:
	osync_free(uid);
	osync_free(hash);
	if (data)
		osync_data_unref(data);

	return FALSE;
}

osync_bool osync_marshal_objformat_sink(OSyncMessage *message, OSyncObjFormatSink *sink, OSyncError **error)
{
	/* Order:
	 *
	 * objformat name
	 * objformat sink config
	 */

	const char *objformat_name = osync_objformat_sink_get_objformat(sink);
	const char *objformat_sink_config = osync_objformat_sink_get_config(sink);

	osync_message_write_string(message, objformat_name, error);
	osync_message_write_string(message, objformat_sink_config, error);

	if (osync_error_is_set(error))
		goto error;

	return TRUE;

error:
	return FALSE;
}

osync_bool osync_demarshal_objformat_sink(OSyncMessage *message, OSyncObjFormatSink **sink, OSyncError **error)
{
	char *objformat_name = NULL;
	char *objformat_sink_config = NULL;

	osync_assert(message);

	/* Order:
	 *
	 * objformat name
	 * objformat sink config
	 */

	/* Get the objtype and format */
	if (!osync_message_read_string(message, &objformat_name, error))
		goto error;

	*sink = osync_objformat_sink_new(objformat_name, error);
	osync_free(objformat_name);
	if (!*sink)
		goto error;

	if (!osync_message_read_string(message, &objformat_sink_config, error))
		goto error;

	osync_objformat_sink_set_config(*sink, objformat_sink_config);
	osync_free(objformat_sink_config);

	return TRUE;

 error:
	osync_trace(TRACE_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_marshal_objtype_sink(OSyncMessage *message, OSyncObjTypeSink *sink, OSyncError **error)
{
	int i = 0;
	int num = 0;

	osync_assert(message);
	osync_assert(sink);

	/* Order:
	 *
	 * name
	 * read function (bool)
	 * get_changes function (bool)
	 * preferred_format (string)
	 * number of format sinks
	 * format sink list (format sinks)
	 * enabled (int)
	 * timeout connect (int)
	 * timeout disconnect (int)
	 * timeout get_changes (int)
	 * timeout commit (int)
	 * timeout committed_all (int)
	 * timeout sync_done (int)
	 * timeout read (int)
	 *
	 */

	num = osync_objtype_sink_num_objformat_sinks(sink);
	osync_message_write_string(message, osync_objtype_sink_get_name(sink), error);

	osync_message_write_int(message, osync_objtype_sink_get_function_read(sink), error);
	osync_message_write_int(message, osync_objtype_sink_get_function_getchanges(sink), error);

	osync_message_write_string(message, osync_objtype_sink_get_preferred_format(sink), error);

	osync_message_write_int(message, num, error);

	if (osync_error_is_set(error))
		goto error;

	for (i = 0; i < num; i++) {
		OSyncObjFormatSink *formatsink = osync_objtype_sink_nth_objformat_sink(sink, i);
		if (!osync_marshal_objformat_sink(message, formatsink, error))
			goto error;
	}

	/* enabled */
	osync_message_write_int(message, osync_objtype_sink_is_enabled(sink), error);

	/* slowsync */
	osync_message_write_int(message, osync_objtype_sink_get_slowsync(sink), error);

	/* timeouts */
	osync_message_write_int(message, osync_objtype_sink_get_connect_timeout(sink), error);
	osync_message_write_int(message, osync_objtype_sink_get_disconnect_timeout(sink), error);

	osync_message_write_int(message, osync_objtype_sink_get_getchanges_timeout(sink), error);
	osync_message_write_int(message, osync_objtype_sink_get_commit_timeout(sink), error);
	osync_message_write_int(message, osync_objtype_sink_get_committedall_timeout(sink), error);
	osync_message_write_int(message, osync_objtype_sink_get_syncdone_timeout(sink), error);

	osync_message_write_int(message, osync_objtype_sink_get_read_timeout(sink), error);

	if (osync_error_is_set(error))
		goto error;


	return TRUE;

 error:
	return FALSE;
}

osync_bool osync_demarshal_objtype_sink(OSyncMessage *message, OSyncObjTypeSink **sink, OSyncError **error)
{
	char *name = NULL;
	char *preferred_format = NULL;
	int num_formats = 0;
	int enabled = 0, timeout = 0, slowsync = 0;
	int read = 0, get_changes = 0;
	int i = 0;

	osync_assert(message);

	/* Order:
	 *
	 * name
	 * read function (bool)
	 * get_changes function (bool)
	 * preferred_format (string)
	 * number of format sinks
	 * format sink list (format sinks)
	 * enabled (int)
	 * slowsync (bool)
	 * timeout connect (int)
	 * timeout disconnect (int)
	 * timeout get_changes (int)
	 * timeout commit (int)
	 * timeout committed_all (int)
	 * timeout sync_done (int)
	 * timeout read (int)
	 *
	 */

	*sink = osync_objtype_sink_new(NULL, error);
	if (!*sink)
		goto error;

	if (!osync_message_read_string(message, &name, error))
		goto free;

	osync_objtype_sink_set_name(*sink, name);
	osync_free(name);

	if (!osync_message_read_int(message, &read, error))
		goto free;

	osync_objtype_sink_set_function_read(*sink, read);

	if (!osync_message_read_int(message, &get_changes, error))
		goto free;

	osync_objtype_sink_set_function_getchanges(*sink, get_changes);

	if (!osync_message_read_string(message, &preferred_format, error))
		goto free;

	osync_objtype_sink_set_preferred_format(*sink, preferred_format);
	osync_free(preferred_format);

	if (!osync_message_read_int(message, &num_formats, error))
		goto free;

	for (i = 0; i < num_formats; i++) {
		OSyncObjFormatSink *formatsink;
		if (!osync_demarshal_objformat_sink(message, &formatsink, error))
			goto free;

		osync_objtype_sink_add_objformat_sink(*sink, formatsink);
		osync_objformat_sink_unref(formatsink);
	}

	/* enabled */
	if (!osync_message_read_int(message, &enabled, error))
		goto free;

	osync_objtype_sink_set_enabled(*sink, enabled);

	/* slowsync */
	if (!osync_message_read_int(message, &slowsync, error))
		goto free;

	osync_objtype_sink_set_slowsync(*sink, slowsync);

	/* timeouts */
	if (!osync_message_read_int(message, &timeout, error))
		goto free;

	osync_objtype_sink_set_connect_timeout(*sink, timeout);

	if (!osync_message_read_int(message, &timeout, error))
		goto free;

	osync_objtype_sink_set_disconnect_timeout(*sink, timeout);

	if (!osync_message_read_int(message, &timeout, error))
		goto free;

	osync_objtype_sink_set_getchanges_timeout(*sink, timeout);

	if (!osync_message_read_int(message, &timeout, error))
		goto free;

	osync_objtype_sink_set_commit_timeout(*sink, timeout);

	if (!osync_message_read_int(message, &timeout, error))
		goto free;

	osync_objtype_sink_set_committedall_timeout(*sink, timeout);

	if (!osync_message_read_int(message, &timeout, error))
		goto free;

	osync_objtype_sink_set_syncdone_timeout(*sink, timeout);

	if (!osync_message_read_int(message, &timeout, error))
		goto free;

	osync_objtype_sink_set_read_timeout(*sink, timeout);

	return TRUE;
 free:
	osync_objtype_sink_unref(*sink);
 error:
	return FALSE;
}

osync_bool osync_marshal_objtype_sinks(OSyncMessage *reply, OSyncClient *client, osync_bool only_available, OSyncError **error)
{

	unsigned int avail = 0;
	OSyncPluginInfo *plugin_info = osync_client_get_plugin_info(client);
	OSyncList *list, *objtypesinks = osync_plugin_info_get_objtype_sinks(plugin_info);
	OSyncObjTypeSink *sink;

	list = objtypesinks;
	while(list) {
		sink = (OSyncObjTypeSink*)list->data;
		if (!only_available || osync_objtype_sink_is_available(sink)) {
			avail++;
		}
		list = list->next;
	}

	if (!osync_message_write_uint(reply, avail, error))
		goto error;

	list = objtypesinks;
	while(list) {
		sink = (OSyncObjTypeSink*)list->data;
		if (!only_available || osync_objtype_sink_is_available(sink)) {
			if (!osync_marshal_objtype_sink(reply, sink, error))
				goto error;
		}
		list = list->next;
	}
	osync_list_free(objtypesinks);

	return TRUE;

error:
	osync_list_free(objtypesinks);
	return FALSE;
}

osync_bool osync_demarshal_objtype_sinks(OSyncMessage *message, OSyncClientProxy *proxy, OSyncError **error)
{
	OSyncObjTypeSink *sink, *proxy_sink, *member_sink;
	unsigned int i, num_sinks;
	OSyncMember *member;

	if (!osync_message_read_uint(message, &num_sinks, error))
		goto error;

	osync_trace(TRACE_INTERNAL, "num objs?: %u", num_sinks);

	for (i = 0; i < num_sinks; i++) {
		if (!osync_demarshal_objtype_sink(message, &sink, error))
			goto error;

		/* Update the sink if there already exists one */
		if ((proxy_sink = osync_client_proxy_find_objtype_sink(proxy, osync_objtype_sink_get_name(sink)))) {
			osync_bool func_read = osync_objtype_sink_get_function_read(sink);
			osync_bool func_getchanges = osync_objtype_sink_get_function_getchanges(sink);

			osync_objtype_sink_set_function_read(proxy_sink, func_read);
			osync_objtype_sink_set_function_getchanges(proxy_sink, func_getchanges);
		} else {
			osync_client_proxy_add_objtype_sink(proxy, sink);
		}

		if ((member = osync_client_proxy_get_member(proxy)) &&
			!(member_sink = osync_member_find_objtype_sink(member, osync_objtype_sink_get_name(sink)))) {
			osync_member_add_objtype_sink(member, sink);
		}

		osync_objtype_sink_unref(sink);
	}

	return TRUE;

error:
	return FALSE;
}

osync_bool osync_marshal_error(OSyncMessage *message, OSyncError *marshal_error, OSyncError **error)
{
	osync_assert(message);

	if (marshal_error) {
		const char *msg = NULL;
		osync_message_write_int(message, 1, error);
		osync_message_write_int(message, osync_error_get_type(&marshal_error), error);
		msg = osync_error_print(&marshal_error);
		osync_message_write_string(message, msg, error);
	} else {
		osync_message_write_int(message, 0, error);
	}

	if (osync_error_is_set(error))
		goto error;

	return TRUE;

error:
	return FALSE;
}

osync_bool osync_demarshal_error(OSyncMessage *message, OSyncError **marshal_error, OSyncError **error)
{
	int hasError = 0;
	osync_assert(message);

	if (!osync_message_read_int(message, &hasError, error))
		goto error;

	if (hasError) {
		char *msg = NULL;
		int error_type = OSYNC_NO_ERROR;

		osync_message_read_int(message, &error_type, error);
		if (error_type <= OSYNC_NO_ERROR || error_type > OSYNC_ERROR_MAX) {
			/* odd... this almost seems like an error in itself,
			 * so let's log it just in case
			 */
			osync_trace(TRACE_ERROR, "%s: demarshaled invalid error type: %d", __func__, error_type);
		}

		osync_message_read_string(message, &msg, error);

		osync_error_set(marshal_error, (OSyncErrorType)error_type, "%s", msg);
		osync_free(msg);
	}

	if (osync_error_is_set(error))
		goto error;

	return TRUE;

error:
	return FALSE;
}

osync_bool osync_marshal_pluginconnection(OSyncMessage *message, OSyncPluginConnection *conn, OSyncError **error)
{
	OSyncPluginConnectionType type;
	osync_assert(message);
	osync_assert(conn);

	/* Order:
	 *
	 * type (int)
	 *
	 * (following are depending on type)
	 *
	 * bt_address (char*)
	 * bt_sdpuuid (char*)
	 * bt_channel (uint)
	 *
	 * usb_vendorid (char *)
	 * usb_productid (char *)
	 * usb_interface (uint)
	 *
	 * net_address (char*)
	 * net_port (uint)
	 * net_protocol (char*)
	 * net_dnssd (char*)
	 *
	 * serial_speed (uint)
	 * serial_devicenode (char*)
	 *
	 * irda_service (char *)
	 */

	type = osync_plugin_connection_get_type(conn);
	if (!osync_message_write_int(message, type, error))
		goto error;

	switch(type) {
	case OSYNC_PLUGIN_CONNECTION_BLUETOOTH:
		osync_message_write_string(message, osync_plugin_connection_bt_get_addr(conn), error);
		osync_message_write_string(message, osync_plugin_connection_bt_get_sdpuuid(conn), error);
		osync_message_write_uint(message, osync_plugin_connection_bt_get_channel(conn), error);
		break;
	case OSYNC_PLUGIN_CONNECTION_USB:
		osync_message_write_string(message, osync_plugin_connection_usb_get_vendorid(conn), error);
		osync_message_write_string(message, osync_plugin_connection_usb_get_productid(conn), error);
		osync_message_write_uint(message, osync_plugin_connection_usb_get_interface(conn), error);
		break;
	case OSYNC_PLUGIN_CONNECTION_NETWORK:
		osync_message_write_string(message, osync_plugin_connection_net_get_address(conn), error);
		osync_message_write_uint(message, osync_plugin_connection_net_get_port(conn), error);
		osync_message_write_string(message, osync_plugin_connection_net_get_protocol(conn), error);
		osync_message_write_string(message, osync_plugin_connection_net_get_dnssd(conn), error);
		break;
	case OSYNC_PLUGIN_CONNECTION_SERIAL:
		osync_message_write_uint(message, osync_plugin_connection_serial_get_speed(conn), error);
		osync_message_write_string(message, osync_plugin_connection_serial_get_devicenode(conn), error);
		break;
	case OSYNC_PLUGIN_CONNECTION_IRDA:
		osync_message_write_string(message, osync_plugin_connection_irda_get_service(conn), error);
		break;
	case OSYNC_PLUGIN_CONNECTION_UNKNOWN:
		break;
	}

	if (osync_error_is_set(error))
		goto error;

	return TRUE;

error:
	return FALSE;
}

osync_bool osync_demarshal_pluginconnection(OSyncMessage *message, OSyncPluginConnection **conn, OSyncError **error)
{
	int type;

	char *bt_address, *bt_sdpuuid;
	unsigned int bt_channel;

	char *usb_vendorid, *usb_productid;
	unsigned int usb_interface;

	unsigned int net_port;
	char *net_address, *net_protocol, *net_dnssd;

	unsigned int serial_speed;
	char *serial_devicenode;

	char *irda_service;

	/* Order:
	 *
	 * type (int)
	 *
	 * (following are depending on type)
	 *
	 * bt_address (char*)
	 * bt_sdpuuid (char*)
	 * bt_channel (uint)
	 *
	 * usb_vendorid (uint)
	 * usb_productid (uint)
	 * usb_interface (uint)
	 *
	 * net_address (char*)
	 * net_port (uint)
	 * net_protocol (char*)
	 * net_dnssd (char*)
	 *
	 * serial_speed (uint)
	 * serial_devicenode (char*)
	 *
	 * irda_service (char *)
	 */

	if (!osync_message_read_int(message, &type, error))
		goto error;

	*conn = osync_plugin_connection_new(error);
	if (!*conn)
		goto error;

	osync_plugin_connection_set_type(*conn, type);

	switch(type) {
	case OSYNC_PLUGIN_CONNECTION_BLUETOOTH:
		osync_message_read_string(message, &bt_address, error);
		osync_plugin_connection_bt_set_addr(*conn, bt_address);

		osync_message_read_string(message, &bt_sdpuuid, error);
		osync_plugin_connection_bt_set_sdpuuid(*conn, bt_sdpuuid);

		osync_message_read_uint(message, &bt_channel, error);
		osync_plugin_connection_bt_set_channel(*conn, bt_channel);

		osync_free(bt_address);
		osync_free(bt_sdpuuid);
		break;
	case OSYNC_PLUGIN_CONNECTION_USB:
		osync_message_read_string(message, &usb_vendorid, error);
		osync_plugin_connection_usb_set_vendorid(*conn, usb_vendorid);

		osync_message_read_string(message, &usb_productid, error);
		osync_plugin_connection_usb_set_productid(*conn, usb_productid);

		osync_message_read_uint(message, &usb_interface, error);
		osync_plugin_connection_usb_set_interface(*conn, usb_interface);
		break;
	case OSYNC_PLUGIN_CONNECTION_NETWORK:
		osync_message_read_string(message, &net_address, error);
		osync_plugin_connection_net_set_address(*conn, net_address);

		osync_message_read_uint(message, &net_port, error);
		osync_plugin_connection_net_set_port(*conn, net_port);

		osync_message_read_string(message, &net_protocol, error);
		osync_plugin_connection_net_set_protocol(*conn, net_protocol);

		osync_message_read_string(message, &net_dnssd, error);
		osync_plugin_connection_net_set_dnssd(*conn, net_dnssd);

		osync_free(net_address);
		osync_free(net_protocol);
		osync_free(net_dnssd);
		break;
	case OSYNC_PLUGIN_CONNECTION_SERIAL:
		osync_message_read_uint(message, &serial_speed, error);
		osync_plugin_connection_serial_set_speed(*conn, serial_speed);

		osync_message_read_string(message, &serial_devicenode, error);
		osync_plugin_connection_serial_set_devicenode(*conn, serial_devicenode);

		osync_free(serial_devicenode);
		break;
	case OSYNC_PLUGIN_CONNECTION_IRDA:
		osync_message_read_string(message, &irda_service, error);
		osync_plugin_connection_serial_set_devicenode(*conn, irda_service);

		osync_free(irda_service);
		break;
	case OSYNC_PLUGIN_CONNECTION_UNKNOWN:
		break;
	}

	if (osync_error_is_set(error))
		goto error;

	return TRUE;

error:
	return FALSE;
}

#define MARSHAL_OBJFORMATSINK_CONFIG (1 << 1)

osync_bool osync_marshal_objformatsink(OSyncMessage *message, OSyncObjFormatSink *sink, OSyncError **error)
{
	unsigned int available_settings = 0;
	const char *config = NULL;
	const char *name = NULL;

	osync_assert(message);
	osync_assert(sink);

	/* Order:
	 *
	 * name (string)
	 *
	 * available_settings (uint)
	 *
	 * (optional)
	 * config (string)
	 */

	config = osync_objformat_sink_get_config(sink);
	name = osync_objformat_sink_get_objformat(sink);

	osync_assert(name);
	if (!osync_message_write_string(message, name, error))
		goto error;

	if (config)
		available_settings |= MARSHAL_OBJFORMATSINK_CONFIG;

	if (!osync_message_write_uint(message, available_settings, error))
		goto error;

	if (config)
		if (!osync_message_write_string(message, config, error))
			goto error;

	return TRUE;

error:
	return FALSE;
}

osync_bool osync_demarshal_objformatsink(OSyncMessage *message, OSyncObjFormatSink **sink, OSyncError **error)
{
	char *name = NULL;
	char *config = NULL;
	unsigned int available_settings = 0;

	/* make sure we start with nothing, to handle errors easier */
	*sink = NULL;

	osync_assert(message);

	/* Order:
	 *
	 * name (string)
	 *
	 * available_settings (uint)
	 *
	 * (optional)
	 * config (string)
	 */

	if (!osync_message_read_string(message, &name, error))
		goto error;

	osync_assert(name);

	*sink = osync_objformat_sink_new(name, error);
	if (!*sink)
		goto error;

	osync_free(name);

	if (!osync_message_read_uint(message, &available_settings, error))
		goto error;

	if (available_settings & MARSHAL_OBJFORMATSINK_CONFIG) {

		if (!osync_message_read_string(message, &config, error))
			goto error;

		osync_objformat_sink_set_config(*sink, config);
		osync_free(config);
	}

	return TRUE;

 error:
	if (name)
		osync_free(name);
	if (config)
		osync_free(config);
	if (*sink)
		osync_objformat_sink_unref(*sink);
	return FALSE;
}


#define MARSHAL_PLUGINADVANCEDOPTION_PARAM_DISPLAYNAME (1 << 1)

osync_bool osync_marshal_pluginadvancedoption_param(OSyncMessage *message, OSyncPluginAdvancedOptionParameter *param, OSyncError **error)
{
	unsigned int available_subconfigs = 0;
	const char *displayname = NULL;
	const char *name = NULL;
	unsigned int type = 0;
	const char *value = NULL;
	unsigned int num_valenum = 0;
	OSyncList *valenum = NULL, *v = NULL;

	osync_assert(message);
	osync_assert(param);

	/* Order:
	 *
	 * available_subconfigs
	 *
	 * displayname (string) (optional)
	 * name (string)
	 * type (uint)
	 * value (string)
	 * num_valenum (uint)
	 * valenum (string list)
	 */

	displayname = osync_plugin_advancedoption_param_get_displayname(param);
	if (displayname)
		available_subconfigs |= MARSHAL_PLUGINADVANCEDOPTION_PARAM_DISPLAYNAME;

	if (!osync_message_write_uint(message, available_subconfigs, error))
		goto error;

	if (!osync_message_write_string(message, displayname, error))
		goto error;

	name = osync_plugin_advancedoption_param_get_name(param);
	if (!osync_message_write_string(message, name, error))
		goto error;

	type = osync_plugin_advancedoption_param_get_type(param);
	if (!osync_message_write_uint(message, type, error))
		goto error;

	value = osync_plugin_advancedoption_param_get_value(param);
	if (!osync_message_write_string(message, value, error))
		goto error;

	valenum = osync_plugin_advancedoption_param_get_valenums(param);
	num_valenum = osync_list_length(valenum);
	if (!osync_message_write_uint(message, num_valenum, error))
		goto error;

	for (v = valenum; v; v = v->next) {
		value = v->data;
		if (!osync_message_write_string(message, value, error))
			goto error;
	}
	osync_list_free(valenum);

	return TRUE;

error:
	return FALSE;
}

#define MARSHAL_PLUGINADVANCEDOPTION_DISPLAYNAME (1 << 1)
#define MARSHAL_PLUGINADVANCEDOPTION_MAXOCCURS   (1 << 2)
#define MARSHAL_PLUGINADVANCEDOPTION_MAXSIZE     (1 << 3)

osync_bool osync_marshal_pluginadvancedoption(OSyncMessage *message, OSyncPluginAdvancedOption *opt, OSyncError **error)
{
	unsigned int available_subconfigs = 0;
	const char *displayname = NULL;
	unsigned int maxoccurs = 0;
	unsigned int maxsize = 0;
	const char *name = NULL;
	unsigned int type = 0;
	const char *value = NULL;
	unsigned int num_parameters = 0;
	OSyncList *parameters = NULL, *p = NULL;
	const char *param = NULL;
	unsigned int num_valenum = 0;
	OSyncList *valenum = NULL, *v = NULL;

	osync_assert(message);
	osync_assert(opt);

	/* Order:
	 *
	 * available_subconfigs
	 *
	 * displayname (string) (optional)
	 * maxoccurs (uint)     (optional)
	 * maxsize (uint)       (optional)
	 * name (string)
	 * type (uint)
	 * value (string)
	 * num_parameters (uint)
	 * parameters (OSyncPluginAdvancedOptionParameter list)
	 * num_valenum (uint)
	 * valenum (string list)
	 */

	displayname = osync_plugin_advancedoption_get_displayname(opt);
	if (displayname)
		available_subconfigs |= MARSHAL_PLUGINADVANCEDOPTION_DISPLAYNAME;

	maxoccurs = osync_plugin_advancedoption_get_maxoccurs(opt);
	if (maxoccurs)
		available_subconfigs |= MARSHAL_PLUGINADVANCEDOPTION_MAXOCCURS;

	maxsize = osync_plugin_advancedoption_get_max(opt);
	if (maxsize)
		available_subconfigs |= MARSHAL_PLUGINADVANCEDOPTION_MAXSIZE;

	osync_message_write_uint(message, available_subconfigs, error);

	if (displayname)
		osync_message_write_string(message, displayname, error);

	if (maxoccurs)
		osync_message_write_uint(message, maxoccurs, error);

	if (maxsize)
		osync_message_write_uint(message, maxsize, error);

	name = osync_plugin_advancedoption_get_name(opt);
	osync_message_write_string(message, name, error);

	type = osync_plugin_advancedoption_get_type(opt);
	osync_message_write_uint(message, type, error);

	value = osync_plugin_advancedoption_get_value(opt);
	osync_message_write_string(message, value, error);

	parameters = osync_plugin_advancedoption_get_parameters(opt);
	num_parameters = osync_list_length(parameters);
	osync_message_write_uint(message, num_parameters, error);

	if (osync_error_is_set(error))
		goto error;

	for ( p = parameters; p; p = p->next) {
		param = p->data;
		if (!osync_message_write_string(message, param, error))
			goto error;
	}
	osync_list_free(parameters);

	valenum = osync_plugin_advancedoption_get_valenums(opt);
	num_valenum = osync_list_length(valenum);
	if (!osync_message_write_uint(message, num_valenum, error))
		goto error;

	for (v = valenum; v; v=v->next) {
		value = v->data;
		if (!osync_message_write_string(message, value, error))
			goto error;
	}
	osync_list_free(valenum);

	return TRUE;

error:
	return FALSE;
}

osync_bool osync_demarshal_pluginadvancedoption_param(OSyncMessage *message, OSyncPluginAdvancedOptionParameter **param, OSyncError **error)
{
	char *displayname = NULL;
	char *name = NULL;
	char *value = NULL;
	unsigned int type;
	unsigned int num_valenum;
	unsigned int i;

	/* Order:
	 *
	 * displayname (string)     (optional)
	 * name (string)
	 * type (uint)
	 * value (string)
	 * num_valenum (uint)
	 * valenum (string list)
	 */

	*param = osync_plugin_advancedoption_param_new(error);
	if (!*param)
		goto error;

	if (!osync_message_read_string(message, &displayname, error))
		goto error;

	osync_plugin_advancedoption_param_set_name(*param, displayname);
	osync_free(displayname);

	if (!osync_message_read_string(message, &name, error))
		goto error;

	osync_plugin_advancedoption_param_set_name(*param, name);
	osync_free(name);

	if (!osync_message_read_uint(message, &type, error))
		goto error;

	osync_plugin_advancedoption_param_set_type(*param, type);

	if (!osync_message_read_string(message, &value, error))
		goto error;

	osync_plugin_advancedoption_param_set_value(*param, value);
	osync_free(value);

	if (!osync_message_read_uint(message, &num_valenum, error))
		goto error;

	for (i=0; i < num_valenum; i++) {

		if (!osync_message_read_string(message, &value, error))
			goto error;

		osync_plugin_advancedoption_param_add_valenum(*param, value);
		osync_free(value);
	}

	return TRUE;
error:
	return FALSE;
}

osync_bool osync_demarshal_pluginadvancedoption(OSyncMessage *message, OSyncPluginAdvancedOption **opt, OSyncError **error)
{
	/* Order:
	 *
	 * available_subconfigs
	 *
	 * displayname (string) (optional)
	 * maxoccurs (uint)     (optional)
	 * maxsize (uint)       (optional)
	 * name (string)
	 * type (uint)
	 * value (string)
	 * num_parameters (uint)
	 * parameters (OSyncPluginAdvancedOptionParameter list)
	 * num_valenum (uint)
	 * valenum (string list)
	 */

	unsigned int available_subconfigs = 0;
	char *displayname = NULL;
	char *name = NULL;
	char *value = NULL;
	unsigned int maxoccurs;
	unsigned int maxsize;
	unsigned int type;
	unsigned int num_parameters;
	unsigned int num_valenum;
	unsigned int i;

	*opt = osync_plugin_advancedoption_new(error);
	if (!*opt)
		goto error;

	if (!osync_message_read_uint(message, &available_subconfigs, error))
		goto error;

	if (available_subconfigs & MARSHAL_PLUGINADVANCEDOPTION_DISPLAYNAME) {
		if (!osync_message_read_string(message, &displayname, error))
			goto error;

		osync_plugin_advancedoption_set_name(*opt, displayname);
		osync_free(displayname);
	}

	if (available_subconfigs & MARSHAL_PLUGINADVANCEDOPTION_MAXOCCURS) {
		if (!osync_message_read_uint(message, &maxoccurs, error))
			goto error;

		osync_plugin_advancedoption_set_maxoccurs(*opt, maxoccurs);
	}


	if (available_subconfigs & MARSHAL_PLUGINADVANCEDOPTION_MAXSIZE) {
		if (!osync_message_read_uint(message, &maxsize, error))
			goto error;

		osync_plugin_advancedoption_set_max(*opt, maxsize);
	}

	if (!osync_message_read_string(message, &name, error))
		goto error;

	osync_plugin_advancedoption_set_name(*opt, name);
	osync_free(name);

	if (!osync_message_read_uint(message, &type, error))
		goto error;

	osync_plugin_advancedoption_set_type(*opt, type);

	if (!osync_message_read_string(message, &value, error))
		goto error;

	osync_plugin_advancedoption_set_value(*opt, value);
	osync_free(value);

	if (!osync_message_read_uint(message, &num_parameters, error))
		goto error;

	for (i=0; i < num_parameters; i++) {
		OSyncPluginAdvancedOptionParameter *param;
		if (!osync_demarshal_pluginadvancedoption_param(message, &param, error))
			goto error;

		osync_plugin_advancedoption_add_parameter(*opt, param);
		osync_plugin_advancedoption_param_unref(param);
	}

	if (!osync_message_read_uint(message, &num_valenum, error))
		goto error;

	for (i=0; i < num_valenum; i++) {
		if (!osync_message_read_string(message, &value, error))
			goto error;

		osync_plugin_advancedoption_add_valenum(*opt, value);
		osync_free(value);
	}

	return TRUE;

error:
	return FALSE;
}

#define MARSHAL_PLUGINLOCALIZATION_ENCODING     (1 << 1)
#define MARSHAL_PLUGINLOCALIZATION_TIMEZONE     (1 << 2)
#define MARSHAL_PLUGINLOCALIZATION_LANGUAGE     (1 << 3)

osync_bool osync_marshal_pluginlocalization(OSyncMessage *message, OSyncPluginLocalization *local, OSyncError **error)
{
	unsigned int available_fields = 0;
	const char *encoding = NULL;
	const char *timezone = NULL;
	const char *language = NULL;

	osync_assert(message);
	osync_assert(local);

	/*
	 * Order:
	 *
	 * available_fields
	 *
	 * encoding (string) (optional)
	 * timezone (string) (optional)
	 * language (string) (optional)
	 */

	encoding = osync_plugin_localization_get_encoding(local);
	timezone = osync_plugin_localization_get_timezone(local);
	language = osync_plugin_localization_get_language(local);

	if (encoding)
		available_fields |= MARSHAL_PLUGINLOCALIZATION_ENCODING;

	if (timezone)
		available_fields |= MARSHAL_PLUGINLOCALIZATION_TIMEZONE;

	if (language)
		available_fields |= MARSHAL_PLUGINLOCALIZATION_LANGUAGE;

	osync_message_write_uint(message, available_fields, error);

	if (encoding)
		osync_message_write_string(message, encoding, error);

	if (timezone)
		osync_message_write_string(message, timezone, error);

	if (language)
		osync_message_write_string(message, language, error);

	if (osync_error_is_set(error))
		goto error;

	return TRUE;

error:
	return FALSE;
}

osync_bool osync_demarshal_pluginlocalization(OSyncMessage *message, OSyncPluginLocalization **local, OSyncError **error)
{
	unsigned int available_fields = 0;
	char *encoding = NULL;
	char *timezone = NULL;
	char *language = NULL;

	osync_assert(message);
	osync_assert(local);

	*local = osync_plugin_localization_new(error);
	if (!*local)
		goto error;

	if (!osync_message_read_uint(message, &available_fields, error))
		goto error;

	if (available_fields & MARSHAL_PLUGINLOCALIZATION_ENCODING) {
		if (!osync_message_read_string(message, &encoding, error))
			goto error;

		osync_plugin_localization_set_encoding(*local, encoding);
		osync_free(encoding);
	}

	if (available_fields & MARSHAL_PLUGINLOCALIZATION_TIMEZONE) {
		if (!osync_message_read_string(message, &timezone, error))
				goto error;

		osync_plugin_localization_set_encoding(*local, timezone);
		osync_free(timezone);
	}


	if (available_fields & MARSHAL_PLUGINLOCALIZATION_LANGUAGE) {
		if (!osync_message_read_string(message, &language, error))
			goto error;

		osync_plugin_localization_set_encoding(*local, language);
		osync_free(language);
	}

	return TRUE;

 error:
	return FALSE;
}

#define MARSHAL_PLUGINAUTEHNTICATION_USERNAME  (1 << 0)
#define MARSHAL_PLUGINAUTEHNTICATION_PASSWORD  (1 << 1)
#define MARSHAL_PLUGINAUTEHNTICATION_REFERENCE (1 << 2)

osync_bool osync_marshal_pluginauthentication(OSyncMessage *message, OSyncPluginAuthentication *auth, OSyncError **error)
{
	unsigned int available_fields = 0;
	const char *username = NULL;
	const char *password = NULL;
	const char *reference = NULL;

	osync_assert(message);
	osync_assert(auth);

	/*
	 * Order:
	 *
	 * available_fields
	 *
	 * username  (string)
	 * password  (string) (optional)
	 * reference (string) (optional)
	 *
	 */

	if (osync_plugin_authentication_option_is_supported(auth, OSYNC_PLUGIN_AUTHENTICATION_USERNAME))
		username = osync_plugin_authentication_get_username(auth);

	if (osync_plugin_authentication_option_is_supported(auth, OSYNC_PLUGIN_AUTHENTICATION_PASSWORD))
		password = osync_plugin_authentication_get_password(auth);

	if (osync_plugin_authentication_option_is_supported(auth, OSYNC_PLUGIN_AUTHENTICATION_REFERENCE))
		reference = osync_plugin_authentication_get_reference(auth);

	osync_assert(username);

	if (username)
		available_fields |= MARSHAL_PLUGINAUTEHNTICATION_USERNAME;

	if (password)
		available_fields |= MARSHAL_PLUGINAUTEHNTICATION_PASSWORD;

	if (reference)
		available_fields |= MARSHAL_PLUGINAUTEHNTICATION_REFERENCE;

	osync_message_write_uint(message, available_fields, error);

	if (username)
		osync_message_write_string(message, username, error);

	if (password)
		osync_message_write_string(message, password, error);

	if (reference)
		osync_message_write_string(message, reference, error);

	if (osync_error_is_set(error))
		goto error;

	return TRUE;

error:
	return FALSE;
}

osync_bool osync_demarshal_pluginauthentication(OSyncMessage *message, OSyncPluginAuthentication **auth, OSyncError **error)
{
	unsigned int available_fields = 0;
	char *username = NULL;
	char *password = NULL;
	char *reference = NULL;

	// Supported options by default : none
	OSyncPluginAuthenticationOptionSupportedFlags supported_options = 0;

	osync_assert(message);

	/*
	 * Order:
	 *
	 * available_fields
	 *
	 * username  (string)
	 * password  (string) (optional)
	 * reference (string) (optional)
	 *
	 */

	*auth = osync_plugin_authentication_new(error);
	if (!*auth)
		goto error;

	if (!osync_message_read_uint(message, &available_fields, error))
		goto error;

	if (available_fields & MARSHAL_PLUGINAUTEHNTICATION_USERNAME) {
		if (!osync_message_read_string(message, &username, error))
			goto error;

		supported_options |= OSYNC_PLUGIN_AUTHENTICATION_USERNAME;
		osync_plugin_authentication_set_username(*auth, username);
		osync_free(username);
	}

	if (available_fields & MARSHAL_PLUGINAUTEHNTICATION_PASSWORD) {
		if (!osync_message_read_string(message, &password, error))
			goto error;

		supported_options |= OSYNC_PLUGIN_AUTHENTICATION_PASSWORD;
		osync_plugin_authentication_set_password(*auth, password);
		osync_free(password);
	}

	if (available_fields & MARSHAL_PLUGINAUTEHNTICATION_REFERENCE) {
		if (!osync_message_read_string(message, &reference, error))
			goto error;

		supported_options |= OSYNC_PLUGIN_AUTHENTICATION_REFERENCE;
		osync_plugin_authentication_set_reference(*auth, reference);
		osync_free(reference);
	}

	osync_plugin_authentication_option_set_supported(*auth, supported_options);

	return TRUE;
error:
	return FALSE;

}

#define MARSHAL_PLUGINRESOURCE_NAME (1 << 1)
#define MARSHAL_PLUGINRESOURCE_MIME (1 << 2)
#define MARSHAL_PLUGINRESOURCE_PATH (1 << 3)
#define MARSHAL_PLUGINRESOURCE_URL  (1 << 4)
#define MARSHAL_PLUGINRESOURCE_PREFERRED_FORMAT  (1 << 5)

osync_bool osync_marshal_pluginresource(OSyncMessage *message, OSyncPluginResource *res, OSyncError **error)
{
	unsigned int available_settings = 0;
	const char *preferred_format = NULL;
	const char *name = NULL;
	const char *mime = NULL;
	const char *objtype = NULL;
	const char *path = NULL;
	const char *url = NULL;
	OSyncList *sinks = NULL;
	unsigned int num_sinks = 0;
	OSyncList *s = NULL;

	osync_assert(message);
	osync_assert(res);

	/* Order:
	 *
	 * enabled (int)
	 * objtype (string)
	 * num_sinks (uint)
	 * sinks (OSyncObjFormatSink)
	 *
	 * available_settings (uint)
	 *
	 * (optional)
	 * name (string)
	 * mime (string)
	 * path (string)
	 * url (string)
	 */
	preferred_format = osync_plugin_resource_get_preferred_format(res);
	name = osync_plugin_resource_get_name(res);
	mime = osync_plugin_resource_get_mime(res);
	objtype = osync_plugin_resource_get_objtype(res);
	path = osync_plugin_resource_get_path(res);
	url = osync_plugin_resource_get_url(res);

	/* enabled */
	osync_message_write_int(message, osync_plugin_resource_is_enabled(res), error);

	/* objtype */
	osync_assert(objtype);
	osync_message_write_string(message, objtype, error);

	/* num_sinks */
	sinks = osync_plugin_resource_get_objformat_sinks(res);
	num_sinks = osync_list_length(sinks);
	osync_message_write_uint(message, num_sinks, error);

	if (osync_error_is_set(error))
		goto error;

	/* format sinks */
	for (s = sinks; s; s = s->next) {
		OSyncObjFormatSink *sink = s->data;
		if (!osync_marshal_objformatsink(message, sink, error))
			goto error;
	}
	osync_list_free(sinks);

	/** optional fields */

	if (preferred_format)
		available_settings |= MARSHAL_PLUGINRESOURCE_PREFERRED_FORMAT;

	if (name)
		available_settings |= MARSHAL_PLUGINRESOURCE_NAME;

	if (mime)
		available_settings |= MARSHAL_PLUGINRESOURCE_MIME;

	if (path)
		available_settings |= MARSHAL_PLUGINRESOURCE_PATH;

	if (url)
		available_settings |= MARSHAL_PLUGINRESOURCE_URL;

	osync_message_write_uint(message, available_settings, error);

	if (preferred_format)
		osync_message_write_string(message, preferred_format, error);

	if (name)
		osync_message_write_string(message, name, error);

	if (mime)
		osync_message_write_string(message, mime, error);

	if (path)
		osync_message_write_string(message, path, error);

	if (url)
		osync_message_write_string(message, url, error);

	if (osync_error_is_set(error))
		goto error;

	return TRUE;

 error:
	return FALSE;
}

osync_bool osync_demarshal_pluginresource(OSyncMessage *message, OSyncPluginResource **res, OSyncError **error)
{
	/* Order:
	 *
	 * enabled (int)
	 * objtype (string)
	 * num_sinks (uint)
	 * sinks (OSyncObjFormatSink)
	 *
	 * available_settings (uint)
	 *
	 * (optional)
	 * preferred_format (string)
	 * name (string)
	 * mime (string)
	 * path (string)
	 * url (string)
	 */

	int enabled;
	char *objtype = NULL;
	unsigned int i, num_sinks;
	unsigned int available_settings;
	char *preferred_format = NULL;
	char *name = NULL;
	char *mime = NULL;
	char *path = NULL;
	char *url = NULL;

	*res = osync_plugin_resource_new(error);
	if (!*res)
		goto error;


	if (!osync_message_read_int(message, &enabled, error))
		goto error;

	osync_plugin_resource_enable(*res, enabled);

	if (!osync_message_read_string(message, &objtype, error))
		goto error;

	osync_plugin_resource_set_objtype(*res, objtype);
	osync_free(objtype);

	if (!osync_message_read_uint(message, &num_sinks, error))
		goto error;

	for (i=0; i < num_sinks; i++) {
		OSyncObjFormatSink *sink = NULL;
		if (!osync_demarshal_objformatsink(message, &sink, error))
			goto error;

		osync_plugin_resource_add_objformat_sink(*res, sink);
		osync_objformat_sink_unref(sink);
	}

	if (!osync_message_read_uint(message, &available_settings, error))
		goto error;

	if (available_settings & MARSHAL_PLUGINRESOURCE_PREFERRED_FORMAT) {
		if (!osync_message_read_string(message, &preferred_format, error))
			goto error;

		osync_plugin_resource_set_preferred_format(*res, preferred_format);
		osync_free(preferred_format);
	}

	if (available_settings & MARSHAL_PLUGINRESOURCE_NAME) {
		if (!osync_message_read_string(message, &name, error))
			goto error;

		osync_plugin_resource_set_name(*res, name);
		osync_free(name);
	}

	if (available_settings & MARSHAL_PLUGINRESOURCE_MIME) {
		if (!osync_message_read_string(message, &mime, error))
			goto error;

		osync_plugin_resource_set_mime(*res, mime);
		osync_free(mime);
	}

	if (available_settings & MARSHAL_PLUGINRESOURCE_PATH) {
		if (!osync_message_read_string(message, &path, error))
			goto error;

		osync_plugin_resource_set_path(*res, path);
		osync_free(path);
	}

	if (available_settings & MARSHAL_PLUGINRESOURCE_URL) {
		if (!osync_message_read_string(message, &url, error))
			goto error;

		osync_plugin_resource_set_url(*res, url);
		osync_free(url);
	}

	return TRUE;

 error:
	return FALSE;
}

#define MARSHAL_PLUGINCONFIG_CONNECTION		(1 << 1)
#define MARSHAL_PLUGINCONFIG_AUTHENTICATION	(1 << 2)
#define MARSHAL_PLUGINCONFIG_LOCALIZATION	(1 << 3)

osync_bool osync_marshal_pluginconfig(OSyncMessage *message, OSyncPluginConfig *config, OSyncError **error)
{
	unsigned int available_subconfigs = 0;
	OSyncPluginConnection *conn = NULL;
	OSyncPluginAuthentication *auth = NULL;
	OSyncPluginLocalization *local = NULL;
	OSyncList *r = NULL;
	OSyncList *aos = NULL;
	OSyncList *options = NULL;

	osync_assert(message);
	osync_assert(config);

	/* Order:
	 *
	 * $available subconfigs
	 *
	 * $connection         (optional)
	 * $authentication     (optional)
	 * $localization       (optional)
	 * $num_resources
	 * $resources
	 * $num_advancedoptions
	 * $advancedoptions
	 */

	conn = osync_plugin_config_get_connection(config);
	auth = osync_plugin_config_get_authentication(config);
	local = osync_plugin_config_get_localization(config);

	if (conn)
		available_subconfigs |= MARSHAL_PLUGINCONFIG_CONNECTION;

	if (auth)
		available_subconfigs |= MARSHAL_PLUGINCONFIG_AUTHENTICATION;

	if (local)
		available_subconfigs |= MARSHAL_PLUGINCONFIG_LOCALIZATION;

	if (!osync_message_write_uint(message, available_subconfigs, error))
		goto error;

	if (conn && !osync_marshal_pluginconnection(message, conn, error))
		goto error;

	if (auth && !osync_marshal_pluginauthentication(message, auth, error))
		goto error;

	if (local && !osync_marshal_pluginlocalization(message, local, error))
		goto error;

	r = osync_plugin_config_get_resources(config);
	if (!osync_message_write_uint(message, osync_list_length(r), error))
		goto error;

	for (; r; r = r->next) {
		OSyncPluginResource *res = r->data;
		if (!osync_marshal_pluginresource(message, res, error))
			goto error;
	}

	options = osync_plugin_config_get_advancedoptions(config);
	if (!osync_message_write_uint(message, osync_list_length(options), error))
		goto error;

	for (aos = options; aos; aos = aos->next) {
		OSyncPluginAdvancedOption *opt = aos->data;
		if (!osync_marshal_pluginadvancedoption(message, opt, error)) {
			osync_list_free(options);
			goto error;
		}
	}
	osync_list_free(options);

	return TRUE;

 error:
	return FALSE;
}

osync_bool osync_demarshal_pluginconfig(OSyncMessage *message, OSyncPluginConfig **config, OSyncError **error)
{
	/* Order:
	 *
	 * $available subconfigs
	 *
	 * $connection            (optional)
	 * $authentication        (optional)
	 * $localization          (optional)
	 * $num_resources
	 * $resources
	 * $num_advancedoptions
	 * $options

	 */

	OSyncPluginConnection *conn = NULL;
	OSyncPluginAuthentication *auth = NULL;
	OSyncPluginLocalization *local = NULL;
	unsigned int available_subconfigs = 0;
	unsigned int i, num_resources = 0, num_advancedoptions = 0;

	*config = osync_plugin_config_new(error);
	if (!*config)
		goto error;

	/* available subconfigs */
	if (!osync_message_read_uint(message, &available_subconfigs, error))
		goto error;

	/* Connection */
	if (available_subconfigs & MARSHAL_PLUGINCONFIG_CONNECTION) {
		if (!osync_demarshal_pluginconnection(message, &conn, error))
			goto error_free_config;

		osync_plugin_config_set_connection(*config, conn);
		osync_plugin_connection_unref(conn);
	}

	/* Authentication */
	if (available_subconfigs & MARSHAL_PLUGINCONFIG_AUTHENTICATION) {
		if (!osync_demarshal_pluginauthentication(message, &auth, error))
			goto error_free_config;

		osync_plugin_config_set_authentication(*config, auth);
		osync_plugin_authentication_unref(auth);
	}

	/* Localization */
	if (available_subconfigs & MARSHAL_PLUGINCONFIG_LOCALIZATION) {
		if (!osync_demarshal_pluginlocalization(message, &local, error))
			goto error_free_config;

		osync_plugin_config_set_localization(*config, local);
		osync_plugin_localization_unref(local);
	}

	if (!osync_message_read_uint(message, &num_resources, error))
		goto error;

	/* number of resources */
	for (i = 0; i < num_resources; i++) {
		OSyncPluginResource *res = NULL;
		if (!osync_demarshal_pluginresource(message, &res, error))
			goto error_free_config;

		osync_plugin_config_add_resource(*config, res);
		osync_plugin_resource_unref(res);

	}

	if (!osync_message_read_uint(message, &num_advancedoptions, error))
		goto error;

	/* number of advancedoptions */
	for (i=0; i < num_advancedoptions; i++) {
		OSyncPluginAdvancedOption *opt;
		if (!osync_demarshal_pluginadvancedoption(message, &opt, error))
			goto error_free_config;

		osync_plugin_config_add_advancedoption(*config, opt);
		osync_plugin_advancedoption_unref(opt);
	}

	return TRUE;

 error_free_config:
	if (*config)
		osync_plugin_config_unref(*config);
 error:
	return FALSE;
}

