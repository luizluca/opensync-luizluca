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

#ifndef _OPENSYNC_MEMORY_H
#define _OPENSYNC_MEMORY_H

/**
 * @defgroup OSyncMemoryAPI OpenSync Memory interface 
 * @ingroup OSyncCommon
 * @brief Functions for handling memory operations 
 */

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

#endif /* _OPENSYNC_MEMORY_H */

