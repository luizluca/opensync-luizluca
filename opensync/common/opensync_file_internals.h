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

#ifndef _OPENSYNC_FILE_INTERNALS_H
#define _OPENSYNC_FILE_INTERNALS_H

/**
 * @defgroup OSyncFileInternalAPI OpenSync File 
 * @ingroup OSyncCommonPrivate
 * @brief Functions for handling common file operations 
 */

/*@{*/

/** @brief Removes a directory recursively
 * 
 * Removes a directory recursively. This is an 
 * internal function for portability. 
 * 
 * @param dirname Directory which will be deleted
 * @returns 0 if successful, -1 otherwise
 * 
 */

int osync_remove_directory_recursively(const char *dirname);

/*@} */

#endif /* _OPENSYNC_FILE_INTERNALS_H */

