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

#include "opensync_marshal.h"
#include "opensync_marshal_private.h"

OSyncMarshal *osync_marshal_sized_new(unsigned int size, OSyncError **error)
{
	OSyncMarshal *marshal = osync_try_malloc0(sizeof(OSyncMarshal), error);
	if (!marshal)
		return NULL;

	marshal->ref_count = 1;

	if (size > 0)
		marshal->buffer = g_byte_array_sized_new( size );
	else
		marshal->buffer = g_byte_array_new();

	marshal->buffer_read_pos = 0;

	return marshal;
}

OSyncMarshal *osync_marshal_new(OSyncError **error)
{
	return osync_marshal_sized_new(0, error);
}

OSyncMarshal *osync_marshal_ref(OSyncMarshal *marshal)
{
	g_atomic_int_inc(&(marshal->ref_count));

	return marshal;
}

void osync_marshal_unref(OSyncMarshal *marshal)
{
	if (g_atomic_int_dec_and_test(&(marshal->ref_count))) {
		
		g_byte_array_free(marshal->buffer, TRUE);
		
		osync_free(marshal);
	}
}

unsigned int osync_marshal_get_marshal_size(OSyncMarshal *marshal)
{
	osync_assert(marshal);
	return marshal->buffer->len;
}

void osync_marshal_set_marshal_size(OSyncMarshal *marshal, unsigned int size)
{
	osync_assert(marshal);
	marshal->buffer->len = size;
}

void osync_marshal_get_buffer(OSyncMarshal *marshal, char **data, unsigned int *size)
{
	osync_assert(marshal);
	
	if (data)
		*data = (char *)marshal->buffer->data;
	
	if (size)
		*size = marshal->buffer->len;
}

void osync_marshal_write_int(OSyncMarshal *marshal, int value)
{
	g_byte_array_append( marshal->buffer, (unsigned char*)&value, sizeof( int ) );
}

void osync_marshal_write_uint(OSyncMarshal *marshal, unsigned int value)
{
	g_byte_array_append( marshal->buffer, (unsigned char*)&value, sizeof( unsigned int ) );
}

void osync_marshal_write_long_long_int(OSyncMarshal *marshal, long long int value)
{
	g_byte_array_append( marshal->buffer, (unsigned char*)&value, sizeof( long long int ) );
}

void osync_marshal_write_string(OSyncMarshal *marshal, const char *value)
{
	unsigned int length = 0;

	if (value)
		length = strlen(value) + 1;

	g_byte_array_append( marshal->buffer, (unsigned char*)&length, sizeof( unsigned int ) );

	if (value)
		g_byte_array_append( marshal->buffer, (unsigned char*)value, length );
}

void osync_marshal_write_data(OSyncMarshal *marshal, const void *value, unsigned int size)
{
	/* TODO move this to PRIVATE API */
	g_byte_array_append( marshal->buffer, value, size );
}

void osync_marshal_write_buffer(OSyncMarshal *marshal, const void *value, unsigned int size)
{
	/* serialize the length of the data to make it possible to determine the end
	   of this data blob in the serialized blob. This makes demarshaling possible! */
	osync_marshal_write_uint(marshal, size);
	if (size > 0)
		osync_marshal_write_data(marshal, value, size);
}

void osync_marshal_read_int(OSyncMarshal *marshal, int *value)
{
	osync_assert(marshal->buffer->len >= marshal->buffer_read_pos + sizeof(int));
	
	memcpy(value, &(marshal->buffer->data[ marshal->buffer_read_pos ]), sizeof(int));
	marshal->buffer_read_pos += sizeof(int);
}

void osync_marshal_read_uint(OSyncMarshal *marshal, unsigned int *value)
{
	osync_assert(marshal->buffer->len >= marshal->buffer_read_pos + sizeof(unsigned int));
	
	memcpy(value, &(marshal->buffer->data[ marshal->buffer_read_pos ]), sizeof(unsigned int));
	marshal->buffer_read_pos += sizeof(unsigned int);
}

void osync_marshal_read_long_long_int(OSyncMarshal *marshal, long long int *value)
{
	osync_assert(marshal->buffer->len >= marshal->buffer_read_pos + sizeof(long long int));
	
	memcpy(value, &(marshal->buffer->data[ marshal->buffer_read_pos ]), sizeof(long long int));
	marshal->buffer_read_pos += sizeof(long long int);
}

void osync_marshal_read_const_string(OSyncMarshal *marshal, const char **value)
{
	int length = 0;
	osync_marshal_read_int(marshal, &length);

	if (length == -1) {
		*value = NULL;
		return;
	}
	
	osync_assert(marshal->buffer->len >= marshal->buffer_read_pos + length);
	*value = (char *)&(marshal->buffer->data[marshal->buffer_read_pos]);
	marshal->buffer_read_pos += length;
}

void osync_marshal_read_string(OSyncMarshal *marshal, char **value)
{
	unsigned int length = 0;
	osync_marshal_read_uint(marshal, &length);

	if (!length) {
		*value = NULL;
		return;
	}
	
	osync_assert(marshal->buffer->len >= marshal->buffer_read_pos + length);
	
	/* TODO: Error handling? */
	*value = (char*) osync_try_malloc0(length, NULL);
	if (!*value)
		return;

	memcpy(*value, &(marshal->buffer->data[ marshal->buffer_read_pos ]), length );
	marshal->buffer_read_pos += length;
}

void osync_marshal_read_const_data(OSyncMarshal *marshal, void **value, unsigned int size)
{
	osync_assert(marshal->buffer->len >= marshal->buffer_read_pos + size);
	
	*value = &(marshal->buffer->data[marshal->buffer_read_pos]);
	marshal->buffer_read_pos += size;
}

void osync_marshal_read_data(OSyncMarshal *marshal, void *value, unsigned int size)
{
	osync_assert(marshal->buffer->len >= marshal->buffer_read_pos + size);
	
	memcpy(value, &(marshal->buffer->data[ marshal->buffer_read_pos ]), size );
	marshal->buffer_read_pos += size;
}

void osync_marshal_read_buffer(OSyncMarshal *marshal, void **value, unsigned int *size)
{
	/* Now, read the data from the marshal */
	osync_marshal_read_uint(marshal, size);
	
	if (*size > 0) {
		*value = g_malloc0(*size);
		osync_marshal_read_data(marshal, *value, *size);
	}
}

