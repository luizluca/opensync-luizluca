/*
 * libopensync - A synchronization framework
 * Copyright (C) 2004-2005  Armin Bauer <armin.bauer@opensync.org>
 * Copyright (C) 2008       Daniel Gollub <dgollub@suse.de>
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

#ifndef OPENSYNC_HASHTABLE_PRIVATE_H_
#define OPENSYNC_HASHTABLE_PRIVATE_H_

/**
 * @defgroup OSyncHashtablePrivateAPI OpenSync Hashtables Private
 * @ingroup OSyncHelperPrivate
 * @brief A Hashtable can be used to detect changes
 */

/*@{*/

/** @brief Creates the database table for the hashtable
 *
 * @param table The hashtable
 * @param error An error struct
 * @returns TRUE on success, or FALSE if an error occurred.
 *
 */
static osync_bool osync_hashtable_create(OSyncHashTable *table, OSyncError **error);

#if !GLIB_CHECK_VERSION(2,12,0)
/**
 * @brief g_hash_table_foreach_remove foreach function
 */
static gboolean remove_entry(gpointer key, gpointer val, gpointer data);
#endif

/**
 * @brief Makes a hashtable forget reported entries
 *
 * You can ask the hashtable to detect the changes. In the end you can
 * ask the hashtable for all items that have been deleted since the last sync.
 * For this the hashtable maintains a internal table of items you already reported and
 * reports the items it didn't see yet as deleted.
 * This function resets the internal table so it start to report deleted items again
 *
 * @param table The hashtable
 *
 */
static void osync_hashtable_reset_reports(OSyncHashTable *table);

/**
 * @brief Makes hashtable in memory forget, not the persistent one
 *
 * @param table The hashtable
 *
 */
static void osync_hashtable_reset(OSyncHashTable *table);

/**
 * @brief Report a item
 *
 * When you use this function the item is marked as reported, so it will not get
 * listed as deleted. Use this function if there are problems accessing an object for
 * example so that the object does not get reported as deleted accidentally.
 *
 * @param table The hashtable
 * @param change The change to report
 *
 */
static void osync_hashtable_report(OSyncHashTable *table, OSyncChange *change);

/**
 * @todo write documentation
 */
static void _osync_hashtable_prepare_insert_query(const char *uid, const char *hash, void *user_data);

/*@}*/

#endif /* OPENSYNC_HASHTABLE_PRIVATE_H_ */
