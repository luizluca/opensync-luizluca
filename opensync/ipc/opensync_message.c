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

#include "common/opensync_marshal.h"
#include "common/opensync_marshal_internals.h"

#include "opensync_serializer_internals.h"

#include "opensync_message_internals.h"
#include "opensync_message_private.h"


OSyncMessage *osync_message_new(OSyncMessageCommand cmd, unsigned int size, OSyncError **error)
{
	OSyncMessage *message = osync_try_malloc0(sizeof(OSyncMessage), error);
	if (!message)
		return NULL;

	message->cmd = cmd;
	message->refCount = 1;

	message->marshal = osync_marshal_sized_new(size, error);
	if (!message->marshal) {
		osync_free(message);
		return NULL;
	}

	return message;
}

OSyncMessage *osync_message_ref(OSyncMessage *message)
{
	g_atomic_int_inc(&(message->refCount));

	return message;
}

void osync_message_unref(OSyncMessage *message)
{
	if (g_atomic_int_dec_and_test(&(message->refCount))) {
		
		osync_marshal_unref(message->marshal);
		
		osync_free(message);
	}
}

void osync_message_set_cmd(OSyncMessage *message, OSyncMessageCommand cmd)
{
	osync_assert(message);
	message->cmd = cmd;
}

OSyncMessageCommand osync_message_get_cmd(OSyncMessage *message)
{
	osync_assert(message);
	return message->cmd;
}

void osync_message_set_timeout(OSyncMessage *message, unsigned int timeout)
{
	osync_assert(message);
	message->timeout = timeout;
}

unsigned int osync_message_get_timeout(OSyncMessage *message)
{
	osync_assert(message);
	return message->timeout;
}

void osync_message_set_id(OSyncMessage *message, osync_messageid id)
{
	osync_assert(message);
	message->id = id;
}

osync_messageid osync_message_get_id(OSyncMessage *message)
{
	osync_assert(message);
	return message->id;
}

OSyncMarshal *osync_message_get_marshal(OSyncMessage *message)
{
	osync_return_val_if_fail(message, NULL);
	return message->marshal;
}

unsigned int osync_message_get_message_size(OSyncMessage *message)
{
	return osync_marshal_get_marshal_size(message->marshal);
}

osync_bool osync_message_set_message_size(OSyncMessage *message, unsigned int size, OSyncError **error)
{
	return osync_marshal_set_marshal_size(message->marshal, size, error);
}

osync_bool osync_message_get_buffer(OSyncMessage *message, char **data, unsigned int *size, OSyncError **error)
{
	return osync_marshal_get_buffer(message->marshal, data, size, error);
}

void osync_message_set_handler(OSyncMessage *message, OSyncMessageHandler handler, void *user_data)
{
	message->user_data = user_data;
	message->callback = handler;
}

OSyncMessageHandler osync_message_get_handler(OSyncMessage *message)
{
	osync_assert(message);
	return message->callback;
}

void *osync_message_get_handler_data(OSyncMessage *message)
{
	osync_assert(message);
	return message->user_data;
}

OSyncMessage *osync_message_new_reply(OSyncMessage *message, OSyncError **error)
{
	OSyncMessage *reply = osync_message_new(OSYNC_MESSAGE_REPLY, 0, error);
	if (!reply)
		return NULL;

	reply->id = message->id;
	return reply;
}

OSyncMessage *osync_message_new_errorreply(OSyncMessage *message, OSyncError *error, OSyncError **loc_error)
{
	OSyncMessage *reply = osync_message_new(OSYNC_MESSAGE_ERRORREPLY, 0, loc_error);
	if (!reply)
		goto error;

	if (!osync_marshal_error(reply, error, loc_error))
		goto error;
	
	if (message)
		reply->id = message->id;
	return reply;

error:
	return NULL;
}

OSyncMessage *osync_message_new_error(OSyncError *error, OSyncError **loc_error)
{
	OSyncMessage *message = osync_message_new(OSYNC_MESSAGE_ERROR, 0, loc_error);
	if (!message)
		goto error;

	if (!osync_marshal_error(message, error, loc_error))
		goto error;
	
	return message;

error:
	return NULL;
}

OSyncMessage *osync_message_new_queue_error(OSyncError *error, OSyncError **loc_error)
{
	OSyncMessage *message = osync_message_new(OSYNC_MESSAGE_QUEUE_ERROR, 0, loc_error);
	if (!message)
		goto error;

	if (!osync_marshal_error(message, error, loc_error))
		goto error;
	
	return message;

error:
	return NULL;
}

osync_bool osync_message_is_error(OSyncMessage *message)
{
	if (message->cmd == OSYNC_MESSAGE_ERRORREPLY)
		return TRUE;
	return FALSE;
}

osync_bool osync_message_is_answered(OSyncMessage *message)
{
	return message->is_answered;
}

void osync_message_set_answered(OSyncMessage *message)
{
	message->is_answered = TRUE;
}

OSyncMessageCommand osync_message_get_command(OSyncMessage *message)
{
	g_assert(message);
	return message->cmd;
}

osync_bool osync_message_write_int(OSyncMessage *message, int value, OSyncError **error)
{
	return osync_marshal_write_int(message->marshal, value, error);
}

osync_bool osync_message_write_uint(OSyncMessage *message, unsigned int value, OSyncError **error)
{
	return osync_marshal_write_uint(message->marshal, value, error);
}

osync_bool osync_message_write_long_long_int(OSyncMessage *message, long long int value, OSyncError **error)
{
	return osync_marshal_write_long_long_int(message->marshal, value, error);
}

osync_bool osync_message_write_string(OSyncMessage *message, const char *value, OSyncError **error)
{
	return osync_marshal_write_string(message->marshal, value, error);
}

osync_bool osync_message_write_data(OSyncMessage *message, const void *value, unsigned int size, OSyncError **error)
{
	return osync_marshal_write_data(message->marshal, value, size, error);
}

osync_bool osync_message_write_buffer(OSyncMessage *message, const void *value, unsigned int size, OSyncError **error)
{
	return osync_marshal_write_buffer(message->marshal, value, size, error);
}

osync_bool osync_message_read_int(OSyncMessage *message, int *value, OSyncError **error)
{
	return osync_marshal_read_int(message->marshal, value, error);
}

osync_bool osync_message_read_uint(OSyncMessage *message, unsigned int *value, OSyncError **error)
{
	return osync_marshal_read_uint(message->marshal, value, error);
}

osync_bool osync_message_read_long_long_int(OSyncMessage *message, long long int *value, OSyncError **error)
{
	return osync_marshal_read_long_long_int(message->marshal, value, error);
}

/* TODO Change char** to const char ** */
osync_bool osync_message_read_const_string(OSyncMessage *message, const char **value, OSyncError **error)
{
	return osync_marshal_read_const_string(message->marshal, value, error);
}

osync_bool osync_message_read_string(OSyncMessage *message, char **value, OSyncError **error)
{
	return osync_marshal_read_string(message->marshal, value, error);
}

osync_bool osync_message_read_const_data(OSyncMessage *message, void **value, unsigned int size, OSyncError **error)
{
	return osync_marshal_read_const_data(message->marshal, value, size, error);
}

osync_bool osync_message_read_data(OSyncMessage *message, void *value, unsigned int size, OSyncError **error)
{
	return osync_marshal_read_data(message->marshal, value, size, error);
}

osync_bool osync_message_read_buffer(OSyncMessage *message, void **value, unsigned int *size, OSyncError **error)
{
	return osync_marshal_read_buffer(message->marshal, value, size, error);
}

char* osync_message_get_commandstr(OSyncMessage *message)
{
	char* cmdstr = "UNKNOWN";
	
	switch(message->cmd)
		{
		case OSYNC_MESSAGE_NOOP:
			cmdstr = "OSYNC_MESSAGE_NOOP"; break;
		case OSYNC_MESSAGE_CONNECT:
			cmdstr = "OSYNC_MESSAGE_CONNECT"; break;
		case OSYNC_MESSAGE_CONNECT_DONE:
			cmdstr = "OSYNC_MESSAGE_CONNECT_DONE"; break;
		case OSYNC_MESSAGE_DISCONNECT:
			cmdstr = "OSYNC_MESSAGE_DISCONNECT"; break;
		case OSYNC_MESSAGE_GET_CHANGES:
			cmdstr = "OSYNC_MESSAGE_GET_CHANGES"; break;
		case OSYNC_MESSAGE_READ_CHANGE:
			cmdstr = "OSYNC_MESSAGE_READ_CHANGE"; break;
		case OSYNC_MESSAGE_COMMIT_CHANGE:
			cmdstr = "OSYNC_MESSAGE_COMMIT_CHANGE"; break;
		case OSYNC_MESSAGE_COMMITTED_ALL:
			cmdstr = "OSYNC_MESSAGE_COMMITTED_ALL"; break;
		case OSYNC_MESSAGE_SYNC_DONE:
			cmdstr = "OSYNC_MESSAGE_SYNC_DONE"; break;
		case OSYNC_MESSAGE_CALL_PLUGIN:
			cmdstr = "OSYNC_MESSAGE_CALL_PLUGIN"; break;
		case OSYNC_MESSAGE_NEW_CHANGE:
			cmdstr = "OSYNC_MESSAGE_NEW_CHANGE"; break;
		case OSYNC_MESSAGE_REPLY:
			cmdstr = "OSYNC_MESSAGE_REPLY"; break;
		case OSYNC_MESSAGE_ERRORREPLY:
			cmdstr = "OSYNC_MESSAGE_ERRORREPLY"; break;
		case OSYNC_MESSAGE_INITIALIZE:
			cmdstr = "OSYNC_MESSAGE_INITIALIZE"; break;
		case OSYNC_MESSAGE_FINALIZE:
			cmdstr = "OSYNC_MESSAGE_FINALIZE"; break;
		case OSYNC_MESSAGE_DISCOVER:
			cmdstr = "OSYNC_MESSAGE_DISCOVER"; break;
		case OSYNC_MESSAGE_SYNCHRONIZE:
			cmdstr = "OSYNC_MESSAGE_SYNCHRONIZE"; break;
		case OSYNC_MESSAGE_ENGINE_CHANGED:
			cmdstr = "OSYNC_MESSAGE_ENGINE_CHANGED"; break;
		case OSYNC_MESSAGE_MAPPING_CHANGED:
			cmdstr = "OSYNC_MESSAGE_MAPPING_CHANGED"; break;
		case OSYNC_MESSAGE_MAPPINGENTRY_CHANGED:
			cmdstr = "OSYNC_MESSAGE_MAPPINGENTRY_CHANGED"; break;
		case OSYNC_MESSAGE_ERROR:
			cmdstr = "OSYNC_MESSAGE_ERROR"; break;
		case OSYNC_MESSAGE_QUEUE_ERROR:
			cmdstr = "OSYNC_MESSAGE_QUEUE_ERROR"; break;
		case OSYNC_MESSAGE_QUEUE_HUP:
			cmdstr = "OSYNC_MESSAGE_QUEUE_HUP"; break;
		}
	
	return cmdstr;	
}

