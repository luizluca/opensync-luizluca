/*
 * libopensync - A synchronization framework
 * Copyright (C) 2004-2005  Armin Bauer <armin.bauer@opensync.org>
 * Copyright (C) 2008  Daniel Gollub <dgollub@suse.de>
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

#ifndef OPENSYNC_HASHTABLE_H_
#define OPENSYNC_HASHTABLE_H_

#include <opensync/opensync_list.h>

typedef void (*OSyncHashtableForEach) (const char *uid, const char *hash, void *user_data);

/**
 * @defgroup OSyncHashtableAPI OpenSync Hashtables
 * @ingroup OSyncPublic
 * @brief A Hashtable can be used to detect changes
 * 
 * Hashtables can be used to detect changes since the last invocation. They do this
 * by keeping track of all reported uids and the hashes of the objects.
 * 
 * A hash is a string that changes when an object is updated or when the content of
 * the object changes. So hashes can either be a real hash like an MD5, or something 
 * like a timestamp. The only important thing is that the hash changes when the item
 * gets updated.
 * 
 * The hashtable is created from a .db file using the osync_hashtable_new() function.
 *
 * With osync_hashtable_load() the pertinent database gets read and loads all hashtable
 * entries into memory.
 * 
 * Now you can query and alter the hashtable in memory. You can ask if a item has changed 
 * by doing:
 *
 * - osync_hashtable_get_changetype() 
 *   To get the changetype of a certain OSyncChange object. Don't forget to update the hash for 
 *   the change in advance. Update your OSyncChange with this detect changetype with
 *   osync_change_set_changetype()
 *
 * - osync_hashtable_update_change()
 *   When the changetype got updated for the OSyncChange object, update the hash entry with
 *   calling osync_hashtable_update_change(). Call this function even if the entry has changetype
 *   unmodified. Otherwise the hashtable will report this entry later as deleted.
 *  
 * - osync_hashtable_get_deleted()
 *   Once all available changes got reported call osync_hashtable_get_deleted() to get an OSyncList
 *   of changes which got deleted since last sync. Entries get determined as deleted if they
 *   got not reported as osync_hashtable_update_change(), independent of the changetype.
 *
 * - osync_hashtable_save()
 *   For performance reason the hashtable in memory got only stored persistence with calling
 *   osync_hashtable_save(). Call this function everytime when the synchronization finished.
 *   This is usually inside the sync_done() function.
 * 
 * After you are finished using the hashtable, call:
 * - osync_hashtable_unref()
 * 
 * The hashtable works like this:
 * 
 * First the items are reported with a certain uid or hash. If the uid does not yet
 * exist in the database it is reported as ADDED. If the uid exists and the hash is different
 * it is reported as MODIFIED. If the uid exists but the hash is the same it means that the
 * object is UNMODIFIED.
 * 
 * To be able to report deleted objects the hashtables keeps track of the uids you reported.
 * After you are done with asking the hashtable for changes you can ask it for deleted objects.
 * All items that are in the hashtable but where not reported by you have to be DELETED.
 * 
 */
/*@{*/

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
OSYNC_EXPORT OSyncHashTable *osync_hashtable_new(const char *path, const char *objtype, OSyncError **error);

/** @brief Increase the reference count of a hashtable object.
 *
 * @param table The hashtable to increase the reference count
 * @returns Pointer to increased hashtable object
 */
OSYNC_EXPORT OSyncHashTable *osync_hashtable_ref(OSyncHashTable *table);

/** @brief Decrease the reference count of a hastable object. 
 *
 * The object will be freed if the reference count reaches zero. 
 * 
 * @param table The hashtable to decrease the reference count 
 * 
 */
OSYNC_EXPORT void osync_hashtable_unref(OSyncHashTable *table);


/** @brief Loads the data in a hashtable from disk
 * 
 * This function must be called to load the hashtable before attempting
 * to use it (eg. in your plugin objtype sink initialization code).
 *
 * @param table The hashtable to load from
 * @param error Pointer to an error struct
 * @returns TRUE on success or FALSE if an error occurred.
 * 
 */
OSYNC_EXPORT osync_bool osync_hashtable_load(OSyncHashTable *table, OSyncError **error);

/** @brief Saves the data in a hashtable to disk
 * 
 * Call this function in your syncdone() plugin sink function.
 *
 * @param table The hashtable to save
 * @param error Pointer to an error struct
 * @returns TRUE on success or FALSE if an error occurred.
 * 
 */
OSYNC_EXPORT osync_bool osync_hashtable_save(OSyncHashTable *table, OSyncError **error);


/** @brief Prepares the hashtable for a slowsync and flush the entire hashtable
 * 
 * This function should be called to prepare the hashtable for a slowsync.
 * The entire database, which stores the values of the hashtable beyond the
 * synchronization, will be flushed.
 * 
 * @param table The hashtable
 * @param error An error struct
 * @returns TRUE on success, or FALSE if an error occurred.
 * 
 */
OSYNC_EXPORT osync_bool osync_hashtable_slowsync(OSyncHashTable *table, OSyncError **error);


/** @brief Get the number of entries
 * 
 * @param table The hashtable
 * @returns the number of entries in the hashtable
 * 
 */
OSYNC_EXPORT unsigned int osync_hashtable_num_entries(OSyncHashTable *table);

/** @brief Call a function over all of the entries
 * 
 * @param table The hashtable to operate on
 * @param func The function to call for each entry
 * @param user_data Pointer to custom data to send to the function
 * 
 */
OSYNC_EXPORT void osync_hashtable_foreach(OSyncHashTable *table, OSyncHashtableForEach func, void *user_data);


/** @brief Update an entry from a change
 * 
 * Updates the entry in the hashtable. Use this even if the change entry
 * is unmodified! Usually this function get called in get_changes(). In some
 * rare cases this get even called inside of the commit() plugin functions,
 * to update the UID inside the hashtable of a changed entry.
 * 
 * @param table The hashtable
 * @param change The change object for the entry
 */
OSYNC_EXPORT void osync_hashtable_update_change(OSyncHashTable *table, OSyncChange *change);

//OSYNC_EXPORT void osync_hashtable_report(OSyncHashTable *table, OSyncChange *change);
//OSYNC_EXPORT void osync_hashtable_reset_reports(OSyncHashTable *table);


/** @brief Get a list of uids which are marked as deleted 
 * 
 * @param table The hashtable
 * @returns OSyncList containing UIDs of deleted entries. Caller is responsible for freeing the ist,
 *          not the content, with osync_list_free() .
 * 
 */
OSYNC_EXPORT OSyncList *osync_hashtable_get_deleted(OSyncHashTable *table);

/** @brief Gets the changetype for the given OSyncChange object, by comparing the hashes
 *         of the hashtable and OSyncChange object.
 * 
 * This function does not report the change so if you only use this function then
 * the object will get reported as deleted! Please use osync_hashtable_update_change() for reporting
 * a change.
 * 
 * @param table The hashtable
 * @param change The change object for the entry
 * @returns The changetype
 * 
 */
OSYNC_EXPORT OSyncChangeType osync_hashtable_get_changetype(OSyncHashTable *table, OSyncChange *change);

/** @brief Get the hash of an entry
 * 
 * @param table The hashtable
 * @param uid the uid of the entry to find
 * @returns the hash for the entry, or NULL if not found
 * 
 */
OSYNC_EXPORT const char *osync_hashtable_get_hash(OSyncHashTable *table, const char *uid);

/*@}*/

#endif /* OPENSYNC_HASHTABLE_H_ */
