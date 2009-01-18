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

#ifndef OPENSYNC_ERROR_PRIVATE_H_
#define OPENSYNC_ERROR_PRIVATE_H_

/**
 * @defgroup OSyncCommonPrivate OpenSync Common Module Private
 * @ingroup OSyncPrivate
 * @defgroup OSyncErrorPrivateAPI OpenSync Error Private
 * @ingroup OSyncCommonPrivate
 */

/*@{*/

/** @brief Represent an error
 */
struct OSyncError {
	/** The type of the error that occured */
	OSyncErrorType type;
	/** The message */
	char *message;
	int ref_count;
	OSyncError *child;
};

/** @brief Translate a error type into something human readable
 * 
 * @param type The error type to look up
 * @returns The name of the error type
 * 
 */
static const char *osync_error_name_from_type(OSyncErrorType type);

/*@}*/

#endif //OPENSYNC_ERROR_PRIVATE_H_
