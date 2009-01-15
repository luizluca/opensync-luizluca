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

#ifndef _OPENSYNC_FILE_H
#define _OPENSYNC_FILE_H

/**
 * @defgroup OSyncFileAPI OpenSync File interface 
 * @ingroup OSyncCommon
 * @brief Functions for handling common file operations 
 */

/*@{*/

/*! @brief Writes data to a file
 * 
 * Writes data to a file
 * 
 * @param filename Where to save the data
 * @param data Pointer to the data
 * @param size Size of the data
 * @param mode The mode to set on the file
 * @param error Pointer to a error struct
 * @returns TRUE if successful, FALSE otherwise
 * 
 */
OSYNC_EXPORT osync_bool osync_file_write(const char *filename, const char *data, unsigned int size, int mode, OSyncError **error);

/*! @brief Reads a file
 * 
 * Reads a file
 * 
 * @param filename Where to read the data from
 * @param data Pointer to the data
 * @param size Size of the data
 * @param error Pointer to a error struct
 * @returns TRUE if successful, FALSE otherwise
 * 
 */
OSYNC_EXPORT osync_bool osync_file_read(const char *filename, char **data, unsigned int *size, OSyncError **error);

/*! @brief Removes a directory recursively
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

#endif /* _OPENSYNC_FILE_H */

