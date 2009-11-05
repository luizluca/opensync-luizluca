/*
 * libopensync - A synchronization framework
 * Copyright (C) 2006  Armin Bauer <armin.bauer@opensync.org>
 * Copyright (C) 2006  NetNix Finland Ltd <netnix@netnix.fi>
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
 * Author: Armin Bauer <armin.bauer@opensync.org>
 * Author: Daniel Friedrich <daniel.friedrich@opensync.org>
 * 
 */

#ifndef OPENSYNC_ARCHIVE_INTERNALS_H_
#define OPENSYNC_ARCHIVE_INTERNALS_H_

/**
 * @defgroup OSyncArchiveInternalAPI OpenSync Archive Internals
 * @ingroup OSyncArchivePrivate
 * @brief The internal part of the OSyncArchive API
 * 
 */
/*@{*/

/**
 * @brief Creates a new archive object
 * @param filename the full path to the archive database file
 * @param error Pointer to an error struct
 * @return The pointer to the newly allocated archive object or NULL in case of error
 */
OSYNC_TEST_EXPORT OSyncArchive *osync_archive_new(const char *filename, OSyncError **error);

/**
 * @brief Increments the reference counter
 * @param archive The pointer to an archive object
 */
OSYNC_TEST_EXPORT OSyncArchive *osync_archive_ref(OSyncArchive *archive);

/**
 * @brief Decrement the reference counter. The archive object will 
 *  be freed if there is no more reference to it.
 * @param archive The pointer to an archive object
 */
OSYNC_TEST_EXPORT void osync_archive_unref(OSyncArchive *archive);

/**
 * @brief Loads all changes from group archive for a certain object type.
 *
 * @param archive The group archive
 * @param objtype Requested object type 
 * @param ids List to store the archive (database) ids of each entry
 * @param uids List to store uids of each entry
 * @param mappingids List to store mappingids for each entry
 * @param memberids List to store member IDs for each entry 
 * @param error Pointer to an error struct
 * @return TRUE on when all changes successfully loaded otherwise FALSE
 */
OSYNC_TEST_EXPORT osync_bool osync_archive_load_changes(OSyncArchive *archive, const char *objtype, OSyncList **ids, OSyncList **uids, OSyncList **mappingids, OSyncList **memberids, OSyncError **error);


/**
 * @brief Stores data of an entry in the group archive database (blob).
 *
 * @param archive The group archive
 * @param id Archive (database) id of entry to save.
 * @param objtype The object type of the entry
 * @param data The data to store 
 * @param size Total size of data 
 * @param error Pointer to an error struct
 * @return Returns TRUE on success otherwise FALSE
 */
OSYNC_TEST_EXPORT osync_bool osync_archive_save_data(OSyncArchive *archive, long long int id, const char *objtype, const char *data, unsigned int size, OSyncError **error);

/**
 * @brief Loads data of an entry which is stored in the group archive database (blob).
 *
 * @param archive The group archive
 * @param uid UID of requestd entry
 * @param objtype The objtype type of the entry
 * @param data Pointer to store the requested data 
 * @param size Pointer to store the size of requested data
 * @param error Pointer to an error struct
 * @return Returns 0 if no data is present else 1. On error -1.
 */ 
OSYNC_TEST_EXPORT int osync_archive_load_data(OSyncArchive *archive, const char *uid, const char *objtype, char **data, unsigned int *size, OSyncError **error);

/**
 * @brief Saves an entry in the group archive. 
 *
 * @param archive The group archive
 * @param id Archive (database) id of entry to update (if it already exists), specify 0 to add a new entry.
 * @param uid Reported UID of entry
 * @param objtype Reported object type of entry
 * @param mappingid Mapped ID of entry 
 * @param memberid ID of member which reported entry 
 * @param objengine Object Engine which handles the change
 * @param error Pointer to an error struct
 * @return Returns number of entries in archive group database. 0 on error. 
 */
OSYNC_TEST_EXPORT long long int osync_archive_save_change(OSyncArchive *archive, long long int id, const char *uid, const char *objtype, long long int mappingid, long long int memberid, const char *objengine, OSyncError **error);

/**
 * @brief Deletes an entry from a group archive.
 *
 * @param archive The group archive
 * @param id Archive (database) id of entry to be deleted
 * @param objtype The object type of the entry
 * @param error Pointer to an error struct
 * @return TRUE if the specified change was deleted successfully, otherwise FALSE
 */
osync_bool osync_archive_delete_change(OSyncArchive *archive, long long int id, const char *objtype, OSyncError **error);

/**
 * @brief Delete all changes from group archive for a certain object type.
 *
 * @param archive The group archive
 * @param objtype Reported object type of entry
 * @param error Pointer to an error struct
 * @return Returns TRUE on success, FALSE otherwise 
 */
osync_bool osync_archive_flush_changes(OSyncArchive *archive, const char *objtype, OSyncError **error);

/**
 * @brief Loads all conficting changes which were ignored in the previous sync. 
 *
 * @param archive The group archive
 * @param objtype Requested object type 
 * @param mappingsids List to store the archive (database) ids of each entry
 * @param changetypes List to store the changetypes for each entry
 * @param error Pointer to an error struct
 * @return TRUE on when all changes successfully loaded otherwise FALSE
 */
osync_bool osync_archive_load_ignored_conflicts(OSyncArchive *archive, const char *objtype, OSyncList **mappingsids, OSyncList **changetypes, OSyncError **error);

/**
 * @brief Saves an entry in the ignored conflict list.
 *
 * @param archive The group archive
 * @param objtype Reported object type of entry
 * @param mappingid Mapping Entry ID of entry 
 * @param changetype Changetype of entry 
 * @param error Pointer to an error struct
 * @return Returns TRUE on success, FALSE otherwise 
 */
osync_bool osync_archive_save_ignored_conflict(OSyncArchive *archive, const char *objtype, long long int mappingid, OSyncChangeType changetype, OSyncError **error);

/**
 * @brief Deletes all ignored conflict entries of the changelog with the objtype.
 *
 * @param archive The group archive
 * @param objtype Reported object type of entry
 * @param error Pointer to an error struct
 * @return Returns TRUE on success, FALSE otherwise 
 */
osync_bool osync_archive_flush_ignored_conflict(OSyncArchive *archive, const char *objtype, OSyncError **error);

osync_bool osync_archive_get_mixed_objengines(OSyncArchive *archive, const char *objengine, OSyncList **objengines, OSyncError **error);


osync_bool osync_archive_update_change_uid(OSyncArchive *archive, const char *olduid, const char *newuid, long long int memberid, const char *objengine, OSyncError **error);

/*@}*/

#endif /*OPENSYNC_ARCHIVE_INTERNALS_H_*/
