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

#ifndef _OPENSYNC_STRING_H
#define _OPENSYNC_STRING_H

/**
 * @defgroup OSyncStringAPI OpenSync String
 * @ingroup OSyncCommon
 * @brief Functions for handling common string operations 
 */

/*@{*/

/** @brief String replace
 * 
 * @param input Input string to work on
 * @param delimiter Delimiter
 * @param replacement Replacement
 * @returns Replaced/Modified string result 
 * 
 */
OSYNC_EXPORT char *osync_strreplace(const char *input, const char *delimiter, const char *replacement);

/** @brief Duplicates a string
 * 
 * Duplicates a string, ending with terminating-zero:
 * 
 * @param str The pointer of the string to duplicate
 * @returns The duplicate string, caller is responsible for freeing.
 * 
 */
OSYNC_EXPORT char *osync_strdup(const char *str);

/** @brief Duplicates a formated string
 * 
 * Duplicates a formated string, ending with terminating-zero:
 * 
 * @param format The format for the output
 * @param ... vararg list
 * @returns The duplicate string, caller is responsible for freeing.
 * 
 */
OSYNC_EXPORT char *osync_strdup_printf(const char *format, ...);

/** @brief Creates a random string
 * 
 * Creates a random string of given length or less
 * 
 * @param maxlength The maximum length of the string
 * @returns The random string, caller is repsonsible for freeing with osync_free().
 *          NULL on OOM situation.
 * 
 */
OSYNC_EXPORT char *osync_rand_str(int maxlength);

/*@} */

#endif /* _OPENSYNC_STRING_H */

