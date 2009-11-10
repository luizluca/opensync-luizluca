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

#ifndef _OPENSYNC_TRACE_H
#define _OPENSYNC_TRACE_H

/**
 * @defgroup OSyncDebug OpenSync Debug Module
 * @ingroup OSyncPublic
 * @defgroup OSyncDebugAPI OpenSync Debug 
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
OSYNC_EXPORT void osync_trace(OSyncTraceType type, const char *message, ...) GCC_FORMAT_CHECK(2, 3);

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

#endif /* _OPENSYNC_TRACE_H */
