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

#ifndef _OPENSYNC_GROUP_H_
#define _OPENSYNC_GROUP_H_

/**
 * @defgroup OSyncGroup OpenSync Group Module
 * @ingroup OSyncPublic
 * @defgroup OSyncGroupAPI OpenSync Groups
 * @ingroup OSyncGroup
 * @brief A groups represent several device or application that should be synchronized
 * 
 */
/*@{*/

/** @brief The lock state of a group
 */
typedef enum {
	OSYNC_LOCK_OK,
	OSYNC_LOCKED,
	OSYNC_LOCK_STALE
} OSyncLockState;

typedef enum {
	OSYNC_CONFLICT_RESOLUTION_UNKNOWN,
	OSYNC_CONFLICT_RESOLUTION_DUPLICATE,
	OSYNC_CONFLICT_RESOLUTION_IGNORE,
	OSYNC_CONFLICT_RESOLUTION_NEWER,
	OSYNC_CONFLICT_RESOLUTION_SELECT
} OSyncConflictResolution;


/** @brief Creates a new group for the given environment
 * 
 * Creates a newly allocated group
 * 
 * @param error Pointer to an error struct
 * @returns Pointer to a new group
 * 
 */
OSYNC_EXPORT OSyncGroup *osync_group_new(OSyncError **error);

/** @brief Increase the reference count of the group
 * 
 * @param group The group
 * @returns The referenced group pointer
 * 
 */
OSYNC_EXPORT OSyncGroup *osync_group_ref(OSyncGroup *group);

/** @brief Decrease the reference count of the group
 * 
 * @param group The group
 * 
 */
OSYNC_EXPORT void osync_group_unref(OSyncGroup *group);


/** @brief Locks a group
 * 
 * Tries to acquire a lock for the given group.
 * 
 * If the lock was successfully acquired, OSYNC_LOCK_OK will
 * be returned.
 * 
 * If the lock was acquired, but a old lock file was detected,
 * OSYNC_LOCK_STALE will be returned. Use this to detect if the
 * last sync of this group was successful, or if this something crashed.
 * If you get this answer you should perform a slow-sync
 * 
 * If the group is locked, OSYNC_LOCKED is returned
 * 
 * @param group The group
 * @returns if the lockfile was acquired
 * 
 */
OSYNC_EXPORT OSyncLockState osync_group_lock(OSyncGroup *group);

/** @brief Unlocks a group
 * 
 * @param group The group to unlock
 * 
 */
OSYNC_EXPORT void osync_group_unlock(OSyncGroup *group);


/** @brief Sets the name for the group
 * 
 * Sets the name for a group
 * 
 * @param group The group
 * @param name The name to set
 * 
 */
OSYNC_EXPORT void osync_group_set_name(OSyncGroup *group, const char *name);

/** @brief Returns the name of a group
 * 
 * Returns the name of a group
 * 
 * @param group The group
 * @returns Name of the group
 * 
 */
OSYNC_EXPORT const char *osync_group_get_name(OSyncGroup *group);


/** @brief Saves the group to disc
 * 
 * Saves the group to disc possibly creating the configdirectory
 * 
 * @param group The group
 * @param error Pointer to an error struct
 * @returns TRUE on success, FALSE otherwise
 * 
 */
OSYNC_EXPORT osync_bool osync_group_save(OSyncGroup *group, OSyncError **error);

/** @brief Deletes a group from disc
 * 
 * Deletes to group directories
 * 
 * @param group The group
 * @param error Pointer to an error struct
 * @returns TRUE on success, FALSE otherwise
 * 
 */
OSYNC_EXPORT osync_bool osync_group_delete(OSyncGroup *group, OSyncError **error);

/** @brief Reset all databases of a group (anchor, hashtable and archive) 
 * 
 * @param group The group
 * @param error Pointer to an error struct
 * @returns TRUE on success, FALSE otherwise
 * 
 */
OSYNC_EXPORT osync_bool osync_group_reset(OSyncGroup *group, OSyncError **error);

/** @brief Loads a group from a directory
 * 
 * Loads a group from a directory
 * 
 * @param group The group object to load into
 * @param path The path to the config directory of the group
 * @param error Pointer to an error struct
 * @returns TRUE on success, FALSE otherwise
 * 
 */
OSYNC_EXPORT osync_bool osync_group_load(OSyncGroup *group, const char *path, OSyncError **error);


/** @brief Appends a member to the group
 * 
 * Appends a member to the group
 * 
 * @param group The group to which to append
 * @param member The member to append
 * 
 */
OSYNC_EXPORT void osync_group_add_member(OSyncGroup *group, OSyncMember *member);

/** @brief Removes a member from the group
 * 
 * @param group The group from which to remove
 * @param member The member to remove
 * 
 */
OSYNC_EXPORT void osync_group_remove_member(OSyncGroup *group, OSyncMember *member);

/** @brief Searches for a member by its id
 * 
 * @param group The group in which to search
 * @param id The id of the member
 * @returns The member, or NULL if not found
 * 
 */
OSYNC_EXPORT OSyncMember *osync_group_find_member(OSyncGroup *group, int id);

/** @brief Returns the nth member of the group
 * 
 * Returns a pointer to the nth member of the group
 * 
 * @param group The group
 * @param nth Which member to return
 * @returns Pointer to the member
 * 
 */
OSYNC_EXPORT OSyncMember *osync_group_nth_member(OSyncGroup *group, int nth);

/** @brief Counts the members of the group
 * 
 * Returns the number of members in the group
 * 
 * @param group The group
 * @returns Number of members
 * 
 */
OSYNC_EXPORT int osync_group_num_members(OSyncGroup *group);


/** @brief Returns the configdir for the group
 * 
 * Returns the configdir for the group
 * 
 * @param group The group
 * @returns String with the path of the config directory
 * 
 */
OSYNC_EXPORT const char *osync_group_get_configdir(OSyncGroup *group);

/** @brief Sets the configdir of the group
 * 
 * @param group The group
 * @param directory The new configdir
 * @returns String with the path of the config directory
 * 
 */
OSYNC_EXPORT void osync_group_set_configdir(OSyncGroup *group, const char *directory);


/** @brief Gets the number of object types of the group
 * 
 * @param group The group
 * @returns Number of object types of the group 
 * 
 */
OSYNC_EXPORT int osync_group_num_objtypes(OSyncGroup *group);

/** @brief Gets the nth object type of the group
 * 
 * @param group The group
 * @param nth The nth position of the object type list
 * @returns Name of the nth object type of the group
 * 
 */
OSYNC_EXPORT const char *osync_group_nth_objtype(OSyncGroup *group, int nth);

/** @brief Change the status of an object type in the group.
 * 
 * @param group The group
 * @param objtype Name of the object type
 * @param enabled The status of the object type to set. TRUE enable, FALSE disable objtype. 
 * 
 */
OSYNC_EXPORT void osync_group_set_objtype_enabled(OSyncGroup *group, const char *objtype, osync_bool enabled);

/** @brief Get the status of an object type in the group 
 * 
 * @param group The group
 * @param objtype The name of the object type 
 * @returns TRUE if object type is enabled in this group, otherwise FALSE
 * 
 */
OSYNC_EXPORT int osync_group_objtype_enabled(OSyncGroup *group, const char *objtype);


/** @brief Add a filter to the group 
 * 
 * @param group The group
 * @param filter The filter to add 
 * 
 */
OSYNC_EXPORT void osync_group_add_filter(OSyncGroup *group, OSyncFilter *filter);

/** @brief Remove a filter from the group 
 * 
 * @param group The group
 * @param filter The filter to remove
 * 
 */
OSYNC_EXPORT void osync_group_remove_filter(OSyncGroup *group, OSyncFilter *filter);

/** @brief Get the number of filters registered in a group
 * 
 * @param group The group
 * @returns The number of filters
 * 
 */
OSYNC_EXPORT int osync_group_num_filters(OSyncGroup *group);

/** @brief Gets the nth filter of a group
 * 
 * Note that you should not add or delete filters while
 * iterating over them
 * 
 * @param group The group
 * @param nth Which filter to return
 * @returns The filter or NULL if not found
 * 
 */
OSYNC_EXPORT OSyncFilter *osync_group_nth_filter(OSyncGroup *group, int nth);


/** @brief Sets the last synchronization date of this group
 * 
 * The information will be stored on disc after osync_group_save()
 * 
 * @param group The group in which to save
 * @param last_sync The time info to set
 */
OSYNC_EXPORT void osync_group_set_last_synchronization(OSyncGroup *group, time_t last_sync);

/** @brief Gets the last synchronization date from this group
 * 
 * The information will available on the group after osync_group_load()
 * 
 * @param group The group
 * @return The synchronization info
 */
OSYNC_EXPORT time_t osync_group_get_last_synchronization(OSyncGroup *group);


/** @brief Set fixed conflict resolution for the group for all appearing conflicts 
 * 
 * @param group The group
 * @param res The conflict resolution
 * @param num The Member ID which solves the conflict (winner)
 * 
 */
OSYNC_EXPORT void osync_group_set_conflict_resolution(OSyncGroup *group, OSyncConflictResolution res, int num);

/** @brief Get fixed conflict resolution for the group for all appearing conflicts 
 * 
 * @param group The group
 * @param res Pointer to set conflict resolution value
 * @param num Pointer to set Member ID value which solves the conflict (winner)
 * 
 */
OSYNC_EXPORT void osync_group_get_conflict_resolution(OSyncGroup *group, OSyncConflictResolution *res, int *num);


/** @brief Get group configured status of merger use. 
 * 
 * @param group The group
 * @return TRUE if merger is enabled. FALSE if merger is disabled.
 */
OSYNC_EXPORT osync_bool osync_group_get_merger_enabled(OSyncGroup *group);

/** @brief Configure status of merger use. 
 * 
 * @param group The group
 * @param merger_enabled TRUE enables the merger. FALSE disables the merger. 
 */
OSYNC_EXPORT void osync_group_set_merger_enabled(OSyncGroup *group, osync_bool merger_enabled);


/** @brief Get group configured status of converter use. 
 * 
 * @param group The group
 * @return TRUE if converter is enabled. FALSE if converter is disabled.
 */
OSYNC_EXPORT osync_bool osync_group_get_converter_enabled(OSyncGroup *group);

/** @brief Configure status of converter use. 
 * 
 * @param group The group
 * @param converter_enabled TRUE enables the converter. FALSE disables the converter. 
 */
OSYNC_EXPORT void osync_group_set_converter_enabled(OSyncGroup *group, osync_bool converter_enabled);


/** @brief Check if group configuration is up to date. 
 * 
 * @param group The group
 * @returns TRUE if the group configuration is up to date, FALSE otherwise. 
 */
OSYNC_EXPORT osync_bool osync_group_is_uptodate(OSyncGroup *group);

/*@}*/

#endif /* _OPENSYNC_GROUP_H_ */
