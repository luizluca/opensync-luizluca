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

#ifndef _OPENSYNC_MARSHAL_INTERNALS_H
#define _OPENSYNC_MARSHAL_INTERNALS_H

/**
 * @defgroup OSyncCommonInternal OpenSync Common Module Internal
 * @ingroup OSyncInternal
 * @defgroup OSyncMarshalInternalAPI OpenSync Marshal Internal 
 * @ingroup OSyncCommonInternal
 * @brief Serialized buffer 
 * 
 */

/*@{*/

/** @brief Appends data with a specific length to the serialized buffer
 *
 * This data should be completely serialized. This is only for internal use,
 * since this function doesn't append the size/end of the appended data.
 * 
 * @param marshal The marshal object
 * @param value The data to append
 * @param size Size of corresponding data parameter
 * @param error Pointer to a error-struct
 */
OSYNC_TEST_EXPORT osync_bool osync_marshal_write_data(OSyncMarshal *marshal, const void *value, unsigned int size, OSyncError **error);

/** @brief Read specific size of serialized data from marshal buffer. This increments 
 * the read position of the marshal buffer. Caller is responsible for freeing the 
 * duplicate data.
 *
 * @param marshal The marshal object
 * @param value Reference to store the pointer to the newly allocated data 
 * @param size Size of data
 * @param error Pointer to a error-struct
 */
OSYNC_TEST_EXPORT osync_bool osync_marshal_read_data(OSyncMarshal *marshal, void *value, unsigned int size, OSyncError **error);

/*@}*/

#endif /* _OPENSYNC_MARSHAL_INTERNALS_H */

