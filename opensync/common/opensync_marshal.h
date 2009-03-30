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

#ifndef _OPENSYNC_MARSHAL_H
#define _OPENSYNC_MARSHAL_H

/**
 * @defgroup OSyncCommon OpenSync Common Module
 * @ingroup OSyncPublic
 * @defgroup OSyncMarshalAPI OpenSync Marshal
 * @ingroup OSyncCommon
 * @brief Functions for serializing/marshaling data 
 * 
 */

/*@{*/

/** @brief Creates a new marshal object
 * 
 * @param error Pointer to a error-struct
 * @returns Pointer to a newly allocated marshal object
 * 
 */
OSYNC_EXPORT OSyncMarshal *osync_marshal_new(OSyncError **error);


/** @brief Creates a new marshal object with a reserved size
 * 
 * @param size Size of the marshal object
 * @param error Pointer to a error-struct
 * @returns Pointer to a newly allocated marshal object
 * 
 */
OSYNC_EXPORT OSyncMarshal *osync_marshal_sized_new(unsigned int size, OSyncError **error);

/** @brief Increase the reference count of the marshal object 
 * 
 * @param marshal The marshal object
 * @returns The referenced marshal pointer
 * 
 */
OSYNC_EXPORT OSyncMarshal *osync_marshal_ref(OSyncMarshal *marshal);

/** @brief Decrease the reference count of the marshal object 
 * 
 * @param marshal The marshal 
 * 
 */
OSYNC_EXPORT void osync_marshal_unref(OSyncMarshal *marshal);

/** @brief Get marshal size of supplied marshal object
 * 
 * @param marshal The marshal object
 * @returns The marshal size of supplied marshal
 * 
 */
OSYNC_EXPORT unsigned int osync_marshal_get_marshal_size(OSyncMarshal *marshal);

/** @brief Set marshal size for supplied marshal object
 * 
 * @param marshal The marshal object
 * @param size The size of the marshal to set
 * 
 */
OSYNC_EXPORT void osync_marshal_set_marshal_size(OSyncMarshal *marshal, unsigned int size);

/** @brief Get the buffer/content of the marshal object
 * 
 * @param marshal The marshal object
 * @param data Pointer to data 
 * @param size Size of the data
 * 
 */
OSYNC_EXPORT void osync_marshal_get_buffer(OSyncMarshal *marshal, char **data, unsigned int *size);

/** @brief Appends an integer value to serialized buffer
 * 
 * @param marshal The marshal object
 * @param value The integer value to append
 */
OSYNC_EXPORT void osync_marshal_write_int(OSyncMarshal *marshal, int value);

/** @brief Appends an unsigned integer value to serialized buffer
 * 
 * @param marshal The marshal object
 * @param value The integer value to append
 */
OSYNC_EXPORT void osync_marshal_write_uint(OSyncMarshal *marshal, unsigned int value);

/** @brief Appends a long long integer value to serialized buffer
 * 
 * @param marshal The marshal object
 * @param value The long long integer value to append
 */
OSYNC_EXPORT void osync_marshal_write_long_long_int(OSyncMarshal *marshal, long long int value);

/** @brief Appends a string to serialized buffer
 * 
 * @param marshal The marshal object
 * @param value The string to append
 */
OSYNC_EXPORT void osync_marshal_write_string(OSyncMarshal *marshal, const char *value);

/** @brief Appends data with a specific length to the serialized buffer
 *
 * This data should be completely serialized. This is only for internal use,
 * since this function doesn't append the size/end of the appended data.
 * 
 * @param marshal The marshal object
 * @param value The data to append
 * @param size Size of corresponding data parameter
 */
OSYNC_EXPORT void osync_marshal_write_data(OSyncMarshal *marshal, const void *value, unsigned int size);

/** @brief Appends data with a specific length to the serialized buffer,
 * plus the length of the data to determine the end.
 *
 * @param marshal The marshal object
 * @param value The data to append
 * @param size Size of corresponding data parameter
 */
OSYNC_EXPORT void osync_marshal_write_buffer(OSyncMarshal *marshal, const void *value, unsigned int size);

/** @brief Read serialized integer from marshal buffer. This increments the read
 * position of the marshal buffer.
 *
 * @param marshal The marshal object
 * @param value Reference to store the integer value 
 */
OSYNC_EXPORT void osync_marshal_read_int(OSyncMarshal *marshal, int *value);

/** @brief Read serialized unsigned integer from marshal buffer. This increments the read
 * position of the marshal buffer.
 *
 * @param marshal The marshal object
 * @param value Reference to store the integer value 
 */
OSYNC_EXPORT void osync_marshal_read_uint(OSyncMarshal *marshal, unsigned int *value);

/** @brief Read serialized long long integer from marshal buffer. This increments the read
 * position of the marshal buffer.
 *
 * @param marshal The marshal object
 * @param value Reference to store the long long integer value 
 */
OSYNC_EXPORT void osync_marshal_read_long_long_int(OSyncMarshal *marshal, long long int *value);

/** @brief Read serialized string from marshal buffer. This increments the read
 * position of the marshal buffer. Caller is responsible for freeing the duplicated
 * string.
 *
 * @param marshal The marshal object
 * @param value Reference to store the pointer to the newly allocated string 
 */
OSYNC_EXPORT void osync_marshal_read_string(OSyncMarshal *marshal, char **value);

/** @brief Read specific size of serialized data from marshal buffer. This increments 
 * the read position of the marshal buffer. Caller is responsible for freeing the 
 * duplicate data.
 *
 * @param marshal The marshal object
 * @param value Reference to store the pointer to the newly allocated data 
 * @param size Size of data
 */
OSYNC_EXPORT void osync_marshal_read_data(OSyncMarshal *marshal, void *value, unsigned int size);

/** @brief Read serialized const data from marshal buffer. This increments the read
 * position of the marshal buffer.
 *
 * @param marshal The marshal object
 * @param value Reference to store the data pointer 
 * @param size The size of data
 */
OSYNC_EXPORT void osync_marshal_read_const_data(OSyncMarshal *marshal, void **value, unsigned int size);

/** @brief Read serialized const string from marshal buffer. This increments the read
 * position of the marshal buffer.
 *
 * @param marshal The marshal object
 * @param value Reference to store the string pointer 
 */
OSYNC_EXPORT void osync_marshal_read_const_string(OSyncMarshal *marshal, const char **value);

/** @brief Read serialized data from marshal buffer. This increments the read
 * position of the marshal buffer. Caller is responsible for freeing the duplicated
 * data.
 *
 * @param marshal The marshal object
 * @param value Reference to store the pointer to the newly allocated data 
 * @param size Size of data
 */
OSYNC_EXPORT void osync_marshal_read_buffer(OSyncMarshal *marshal, void **value, unsigned int *size);

/*@}*/

#endif /* _OPENSYNC_MARSHAL_H */

