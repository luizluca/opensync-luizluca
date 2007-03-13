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

#ifndef _OPENSYNC_SUPPORT_H
#define _OPENSYNC_SUPPORT_H

/*! @ingroup OSyncDebugAPI
 * @brief The type of the trace */
typedef enum {
	/** Used when entering a function. This will indent the callgraph */
	TRACE_ENTRY,
	/** Used when exiting a function. This will unindent the callgraph */
	TRACE_EXIT,
	/** Used for traces inside a function. Does not indent. */
	TRACE_INTERNAL,
	/** Used for traces with sensitive content inside a function. Does not indent. */
	TRACE_SENSITIVE,
	/** Used when exiting a function with a error. This will unindent the callgraph */
	TRACE_EXIT_ERROR,
	/** Used to log a general error. This will not unindent the callgraph */
	TRACE_ERROR
} OSyncTraceType;

OSYNC_EXPORT char *osync_rand_str(int maxlength);
OSYNC_EXPORT char *osync_print_binary(const unsigned char *data, int len);

OSYNC_EXPORT void osync_trace_reset_indent(void);
OSYNC_EXPORT void osync_trace(OSyncTraceType type, const char *message, ...);
OSYNC_EXPORT void osync_trace_disable(void);
OSYNC_EXPORT void osync_trace_enable(void);

OSYNC_EXPORT osync_bool osync_file_write(const char *filename, const char *data, unsigned int size, int mode, OSyncError **error);
OSYNC_EXPORT osync_bool osync_file_read(const char *filename, char **data, unsigned int *size, OSyncError **error);
OSYNC_EXPORT const char *osync_get_version(void);
OSYNC_EXPORT void *osync_try_malloc0(unsigned int size, OSyncError **error);

char *osync_strreplace(const char *input, const char *delimiter, const char *replacement);

#include "opensync_list.h"

#endif /*_OPENSYNC_SUPPORT_H*/
