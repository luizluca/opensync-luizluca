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

#ifndef OPENSYNC_ERROR_H_
#define OPENSYNC_ERROR_H_

#include <stdarg.h>

OPENSYNC_BEGIN_DECLS

/**
 * @defgroup OSyncCommon OpenSync Common Module
 * @ingroup OSyncPublic
 * @defgroup OSyncErrorAPI OpenSync Error
 * @ingroup OSyncCommon
 */
 
/*@{*/

 /*
 * @brief Defines the possible error types
 */
typedef enum {
	OSYNC_NO_ERROR = 0,
	OSYNC_ERROR_GENERIC = 1,
	OSYNC_ERROR_IO_ERROR = 2,
	OSYNC_ERROR_NOT_SUPPORTED = 3,
	OSYNC_ERROR_TIMEOUT = 4,
	OSYNC_ERROR_DISCONNECTED = 5,
	OSYNC_ERROR_FILE_NOT_FOUND = 6,
	OSYNC_ERROR_EXISTS = 7,
	OSYNC_ERROR_CONVERT = 8,
	OSYNC_ERROR_MISCONFIGURATION = 9,
	OSYNC_ERROR_INITIALIZATION = 10,
	OSYNC_ERROR_PARAMETER = 11,
	OSYNC_ERROR_EXPECTED = 12,
	OSYNC_ERROR_NO_CONNECTION = 13,
	OSYNC_ERROR_TEMPORARY = 14,
	OSYNC_ERROR_LOCKED = 15,
	OSYNC_ERROR_PLUGIN_NOT_FOUND = 16
} OSyncErrorType;

/** @brief Increase the reference count of the error object 
 * 
 * @param error The error object 
 * @returns The referenced error pointer
 * 
 */
OSYNC_EXPORT OSyncError **osync_error_ref(OSyncError **error);

/** @brief Decrease the reference count of the error object
 * 
 * @param error The error object 
 * 
 */
OSYNC_EXPORT void osync_error_unref(OSyncError **error);

/** @brief Checks if the error is set
 * 
 * @param error A pointer to a error struct to check
 * @returns TRUE if the error is set, FALSE otherwise
 * 
 */
OSYNC_EXPORT osync_bool osync_error_is_set (OSyncError **error);

/** @brief Sets the error
 * 
 * You can use this function to set the error to the given type and message
 * 
 * @param error A pointer to a error struct to set
 * @param type The Error type to set
 * @param format The message
 * 
 */
OSYNC_EXPORT void osync_error_set(OSyncError **error, OSyncErrorType type, const char *format, ...);

/** @brief This will return a string describing the type of the error
 * 
 * @param error A pointer to a error struct
 * @returns The description, NULL on error
 * 
 */
OSYNC_EXPORT const char *osync_error_get_name(OSyncError **error);

/** @brief Duplicates the error into the target
 * 
 * 
 * @param target The target error to update
 * @param source The source error which to duplicate
 * 
 */
OSYNC_EXPORT void osync_error_set_from_error(OSyncError **target, OSyncError **source);

/** @brief Returns the message of the error
 * 
 * @param error The error to print
 * @returns The message of the error or NULL if no error
 * 
 */
OSYNC_EXPORT const char *osync_error_print(OSyncError **error);

/** @brief Returns the entired error stack as single string 
 * 
 * @param error The error stack to print
 * @returns The message of the error or NULL if no error
 * 
 */
OSYNC_EXPORT char *osync_error_print_stack(OSyncError **error);

/** @brief Stack error on another error object 
 * 
 * Use this function to stack all errors to describe the root cause of an error 
 * 
 * @param parent A pointer to a error which gets the child stacked 
 * @param child A pointer to a error to which get stacked on parent error
 * 
 */
OSYNC_EXPORT void osync_error_stack(OSyncError **parent, OSyncError **child);

/** @brief Get stacked child of an error object 
 * 
 * Use this function to read an error stack 
 * 
 * @param parent A pointer to a error stack 
 * 
 */
OSYNC_EXPORT OSyncError *osync_error_get_child(OSyncError **parent);

/** @brief Returns the type of the error
 * 
 * @param error The error
 * @returns The type of the error or OSYNC_NO_ERROR if no error
 * 
 */
OSYNC_EXPORT OSyncErrorType osync_error_get_type(OSyncError **error);

/** @brief Sets the type of an error
 * 
 * @param error A pointer to a error struct to set
 * @param type The Error type to set
 * 
 */
OSYNC_EXPORT void osync_error_set_type(OSyncError **error, OSyncErrorType type);

/** @brief Sets a error from a va_list
 * 
 * @param error A pointer to a error struct
 * @param type The type to set
 * @param format The message
 * @param args The arguments to the message
 * 
 */
OSYNC_EXPORT void osync_error_set_vargs(OSyncError **error, OSyncErrorType type, const char *format, va_list args);

/*@}*/

OPENSYNC_END_DECLS

#endif //OPENSYNC_ERROR_H_
