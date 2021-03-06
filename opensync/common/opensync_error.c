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

#include "opensync_error_private.h"

static const char *osync_error_name_from_type(OSyncErrorType type)
{
	switch (type) {
	case OSYNC_NO_ERROR:
		return "NoError";
	case OSYNC_ERROR_GENERIC:
		return "UnknownError";
	case OSYNC_ERROR_IO_ERROR:
		return "IOError";
	case OSYNC_ERROR_NOT_SUPPORTED:
		return "NotSupported";
	case OSYNC_ERROR_TIMEOUT:
		return "Timeout";
	case OSYNC_ERROR_DISCONNECTED:
		return "Disconnected";
	case OSYNC_ERROR_FILE_NOT_FOUND:
		return "FileNotFound";
	default:
		return "UnspecifiedError";
	}
}

void osync_error_set_vargs(OSyncError **error, OSyncErrorType type, const char *format, va_list args)
{
	osync_return_if_fail(error);
	osync_return_if_fail(format);

	// save any existing error, in case it needs to be added as a child
	OSyncError *old_error = *error;

	// create new error object in user's pointer variable
	*error = g_malloc0(sizeof(OSyncError));
	(*error)->message = g_strdup_vprintf(format, args);
	(*error)->type = type;
	(*error)->ref_count = 1;

	// tack on the old error as the child
	(*error)->child = old_error;

	osync_trace(TRACE_ERROR, "%s", (*error)->message);
	
	return;
}

const char *osync_error_get_name(OSyncError **error)
{
	osync_return_val_if_fail(error != NULL, NULL);
	if (!*error)
		return osync_error_name_from_type(OSYNC_NO_ERROR);
	return osync_error_name_from_type((*error)->type);
}

OSyncError **osync_error_ref(OSyncError **error)
{
	if (!osync_error_is_set(error))
		return error;
	
	g_atomic_int_inc(&(*error)->ref_count);

	return error;
}

void osync_error_unref(OSyncError **error)
{
	if (!osync_error_is_set(error))
		return;
		
	if (g_atomic_int_dec_and_test(&(*error)->ref_count)) {
		if ((*error)->message)
			g_free ((*error)->message);
		
		if ((*error)->child)
			osync_error_unref(&((*error)->child));
		
		g_free(*error);
	}
	
	*error = NULL;
}

osync_bool osync_error_is_set (OSyncError **error)
{
	if (!error)
		return FALSE;
		
	if (*error == NULL)
		return FALSE;
	
	if ((*error)->type)
		return TRUE;
		
	return FALSE;
}

OSyncErrorType osync_error_get_type(OSyncError **error)
{
	if (!osync_error_is_set(error))
		return OSYNC_NO_ERROR;
	return (*error)->type;
}

const char *osync_error_print(OSyncError **error)
{
	if (!osync_error_is_set(error))
		return NULL;
	return (*error)->message;
}

char *osync_error_print_stack(OSyncError **error)
{
	char *submessage = NULL;
	char *message = NULL;
	if (!osync_error_is_set(error))
		return NULL;
		
	if ((*error)->child)
		submessage = osync_error_print_stack(&((*error)->child));
	
	if (submessage) {
		message = g_strdup_printf("NEXT ERROR: \"%s\"\n%s", (*error)->message, submessage);
		g_free(submessage);
	} else
		message = g_strdup_printf("ROOT CAUSE: \"%s\"", (*error)->message);
	
	return message;
}

void osync_error_set_from_error(OSyncError **target, OSyncError **source)
{
	if (!target || osync_error_is_set(target))
		return;
	
	if (!osync_error_is_set(source)) {
		*target = NULL;
		return;
	}
	
	*target = *source;
	osync_error_ref(target);
}

void osync_error_set(OSyncError **error, OSyncErrorType type, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	osync_error_set_vargs(error, type, format, args);
	va_end (args);
}

void osync_error_stack(OSyncError **parent, OSyncError **child)
{
	if (!parent || !*parent)
		return;
	
	if (!child || !*child)
		return;

	/* Avoid infinite recursion. */
	if (*parent == *child)
		return;
	
	if ((*parent)->child)
		osync_error_unref(&((*parent)->child));
	
	(*parent)->child = *child;
	osync_error_ref(child);
}

OSyncError *osync_error_get_child(OSyncError **parent)
{
	if (!parent || !*parent)
		return NULL;
	
	return (*parent)->child;
}

void osync_error_set_type(OSyncError **error, OSyncErrorType type)
{
	if (!error)
		return;
	
	(*error)->type = type;
	return;
}

