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

/* FIXME: Drop opensync-support.h with 0.39 release */
#include "opensync-common.h"

OPENSYNC_BEGIN_DECLS

/**
 * @defgroup OSyncDebugAPI OpenSync Debugging 
 * @ingroup OSyncDebug
 * @brief Functions for debugging
 */

/*@{*/

/*! @brief The type of the trace
 *
 */
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


/*! @brief Reset the indentation of the trace function. 
 *
 * Use this function after when process got forked. It's up to the forked
 * process (child) to reset the indent.
 * 
 */
OSYNC_EXPORT void osync_trace_reset_indent(void);

/*! @brief Used for tracing the application
 * 
 * use this function to trace calls. The call graph will be saved into
 * the file that is given in the OSYNC_TRACE environment variable
 * 
 * @param type The type of the trace
 * @param message The message to save
 * 
 */
OSYNC_EXPORT void osync_trace(OSyncTraceType type, const char *message, ...);

/*! @brief Disable tracing
 *
 */
OSYNC_EXPORT void osync_trace_disable(void);

/*! @brief Enable tracing
 *
 */
OSYNC_EXPORT void osync_trace_enable(void);

/*! @brief Is tracing enabled?
 *
 * @returns TRUE if tracing is enabled, FALSE if disabled.
 */
OSYNC_EXPORT osync_bool osync_trace_is_enabled(void);

/*@} */

/**
 * @defgroup OSyncSupportAPI OpenSync Support Module 
 * @ingroup OSyncPublic
 */
/*@{*/

#define __NULLSTR(x) x ? x : "(NULL)"

/*! @brief Returns the version of opensync
 * 
 * Returns a string identifying the major and minor version
 * of opensync (something like "0.11")
 * 
 * @returns String with version
 * 
 */
OSYNC_EXPORT const char *osync_get_version(void);

/*! @brief Safely tries to malloc memory
 * 
 * Tries to malloc memory but returns an error in an OOM situation instead
 * of aborting
 * 
 * @param size The size in bytes to malloc
 * @param error The error which will hold the info in case of an error
 * @returns A pointer to the new memory or NULL in case of error, needs to be released by osync_free()
 * 
 */
OSYNC_EXPORT void *osync_try_malloc0(unsigned int size, OSyncError **error);

/*! @brief Frees memory
 * 
 * Frees memory allocated by osync_try_malloc0() and others osync_* functions.
 * 
 * @param ptr Pointer to allocated memory which should get freed
 * 
 */
OSYNC_EXPORT void osync_free(void *ptr);

/*! @brief String replace
 * 
 * @param input Input string to work on
 * @param delimiter Delimiter
 * @param replacement Replacement
 * @returns Replaced/Modified string result 
 * 
 */
OSYNC_EXPORT char *osync_strreplace(const char *input, const char *delimiter, const char *replacement);

/*! @brief Duplicates a string 
 * 
 * Duplicates a string, ending with terminating-zero: \0
 * 
 * @param str The pointer of the string to duplicate
 * @returns The duplicate string, caller is responsible for freeing.
 * 
 */
OSYNC_EXPORT char *osync_strdup(const char *str);

/*! @brief Duplicates a formated string 
 * 
 * Duplicates a formated string, ending with terminating-zero: \0
 * 
 * @param str The pointer of the string to duplicate
 * @param args 
 * @returns The duplicate string, caller is responsible for freeing.
 * 
 */
OSYNC_EXPORT char *osync_strdup_printf(const char *format, ...);

/*! @brief Creates a random string
 * 
 * Creates a random string of given length or less
 * 
 * @param maxlength The maximum length of the string
 * @returns The random string, caller is repsonsible for freeing with osync_free().
 *          NULL on OOM situation.
 * 
 */
OSYNC_EXPORT char *osync_rand_str(int maxlength);

/*@} */

OPENSYNC_END_DECLS

#endif /* _OPENSYNC_SUPPORT_H */

