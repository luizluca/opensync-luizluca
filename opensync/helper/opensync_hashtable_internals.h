/*
 * libopensync - A synchronization framework
 * Copyright (C) 2004-2005  Armin Bauer <armin.bauer@opensync.org>
 * Copyright (C) 2008       Daniel Gollub <gollub@b1-systems.de>
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

#ifndef _OPENSYNC_HASHTABLE_INTERNALS_H_
#define _OPENSYNC_HASHTABLE_INTERNALS_H_

/**
 * @defgroup OSyncHashtableInternalAPI OpenSync Hashtable Internals
 * @ingroup OSyncHelperPrivate
 * @brief Internals of OpenSync Hashtable
 */

#define OSYNC_HASHTABLE_DB_PREFIX "tbl_hash_"

/** @brief Represent a hashtable which can be used to check if changes have been modifed or deleted */
struct OSyncHashTable {
	int ref_count;
	OSyncDB *dbhandle;

	GHashTable *reported_entries;

	GHashTable *db_entries;

	char *name;

	/* Only to build transaction queries */
	GString *query;
};

/** @brief Creates a hashtable access object
 *
 * Hashtables can be used to detect what has been changed since
 * the last sync.
 *
 * @param path the full path and file name of the hashtable .db file to load from or create
 * @param objtype the object type of the hashtable
 * @param error Pointer to an error struct
 * @returns A new hashtable, or NULL if an error occurred.
 *
 */
OSYNC_TEST_EXPORT OSyncHashTable *osync_hashtable_new(const char *path, const char *objtype, OSyncError **error);

/** @brief Increase the reference count of a hashtable object.
 *
 * @param table The hashtable to increase the reference count
 * @returns Pointer to increased hashtable object
 */
OSYNC_TEST_EXPORT OSyncHashTable *osync_hashtable_ref(OSyncHashTable *table);

/** @brief Decrease the reference count of a hastable object.
 *
 * The object will be freed if the reference count reaches zero.
 *
 * @param table The hashtable to decrease the reference count
 *
 */
OSYNC_TEST_EXPORT void osync_hashtable_unref(OSyncHashTable *table);

/** @brief Loads the hashtable
 *
 * This function must be called to load the hashtable before attempting
 * to use it (eg. in your plugin objtype sink initialization code).
 *
 * @param table The hashtable to load from
 * @param error Pointer to an error struct
 * @returns TRUE on success or FALSE if an error occurred.
 *
 */
OSYNC_TEST_EXPORT osync_bool osync_hashtable_load(OSyncHashTable *table, OSyncError **error);

/** @brief Saves the data in a hashtable
 *
 * Call this function in your syncdone() plugin sink function.
 *
 * @param table The hashtable to save
 * @param error Pointer to an error struct
 * @returns TRUE on success or FALSE if an error occurred.
 *
 */
OSYNC_TEST_EXPORT osync_bool osync_hashtable_save(OSyncHashTable *table, OSyncError **error);

/** @brief Get the number of entries
 *
 * @param table The hashtable
 * @returns the number of entries in the hashtable
 *
 */
OSYNC_TEST_EXPORT unsigned int osync_hashtable_num_entries(OSyncHashTable *table);


OSYNC_TEST_EXPORT osync_bool osync_hashtable_update_uid(OSyncHashTable *table, const char *olduid, const char *newuid, OSyncError **error);

#endif /*_OPENSYNC_HASHTABLE_INTERNALS_H_*/

