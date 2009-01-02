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

#include "config.h"

#include "opensync.h"
#include "opensync_internals.h"

void *osync_try_malloc0(unsigned int size, OSyncError **error)
{
	void *result = NULL;
	
#ifdef OPENSYNC_UNITTESTS 	
	if (!g_getenv("OSYNC_NOMEMORY"))
		result = g_try_malloc(size);
#else		
	result = g_try_malloc(size);
#endif /*OPENSYNC_UNITTESTS*/

	if (!result) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "No memory left");
		return NULL;
	}
	memset(result, 0, size);
	return result;
}

void osync_free(void *ptr)
{
	if (!ptr)
		return;

	g_free(ptr);
}

