/*
 * libopensync - A synchronization framework
 * Copyright (C) 2004-2006  Armin Bauer <armin.bauer@desscon.com>
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
 
#ifndef OPENSYNC_MAPPING_ENTRY_H_
#define OPENSYNC_MAPPING_ENTRY_H_

/**
 * @defgroup OSyncMappingEntryAPI OpenSync Mapping Entry
 * @ingroup OSyncPublic
 * @brief Functions for managing member uids in mappings
 *
 * Mapping entries (member-specific components of a mapping) have several associated IDs:
 * 
 * - The member ID - uniquely identifies the member
 * - The entry ID - uniquely identifies the mapping table entry
 * - The uid - the unique identifier of the item/entry in the member's system
 * 
 * @see OSyncMappingTableAPI, OSyncMappingAPI
 */
/*@{*/

/**
 * @brief Creates a new mapping entry object
 * @param error Pointer to an error struct
 * @return The pointer to the newly allocated mapping entry object or NULL in case of error
 */
OSYNC_EXPORT OSyncMappingEntry *osync_mapping_entry_new(OSyncError **error);

/**
 * @brief Increments the reference counter
 * @param entry Pointer to a mapping entry object
 */
OSYNC_EXPORT OSyncMappingEntry *osync_mapping_entry_ref(OSyncMappingEntry *entry);

/**
 * @brief Decrement the reference counter. The mapping table object will 
 *  be freed if the reference count reaches zero.
 * @param entry Pointer to a mapping entry object
 */
OSYNC_EXPORT void osync_mapping_entry_unref(OSyncMappingEntry *entry);


/**
 * @brief Determine if a mapping entry matches a change
 * @param entry Pointer to a mapping entry object
 * @param change Pointer to a change object
 * @returns TRUE if the change and mapping entry match up, FALSE otherwise.
 */
OSYNC_EXPORT osync_bool osync_mapping_entry_matches(OSyncMappingEntry *entry, OSyncChange *change);

/*
OSYNC_EXPORT void osync_mapping_entry_update(OSyncMappingEntry *entry, OSyncChange *change);
OSYNC_EXPORT OSyncChange *osync_mapping_entry_get_change(OSyncMappingEntry *entry);

OSYNC_EXPORT osync_bool osync_mapping_entry_is_dirty(OSyncMappingEntry *entry);
OSYNC_EXPORT void osync_mapping_entry_set_dirty(OSyncMappingEntry *entry, osync_bool dirty);
 */

/**
 * @brief Set the unique ID of the mapping
 * 
 * This uid is the unique identifier of the item/entry in the member's system.
 * 
 * @param entry Pointer to a mapping entry object
 * @param uid the member's uid for the entry
 */
OSYNC_EXPORT void osync_mapping_entry_set_uid(OSyncMappingEntry *entry, const char *uid);

/**
 * @brief Get the unique ID of the mapping
 * 
 * This uid is the unique identifier of the item/entry in the member's system.
 * 
 * @param entry Pointer to a mapping entry object
 * @returns the member's uid for the entry
 */
OSYNC_EXPORT const char *osync_mapping_entry_get_uid(OSyncMappingEntry *entry);

/**
 * @brief Get the member ID of the entry
 * @param entry Pointer to a mapping entry object
 * @returns the entry's member ID
 */
OSYNC_EXPORT long long int osync_mapping_entry_get_member_id(OSyncMappingEntry *entry);

/**
 * @brief Set the member ID of the entry
 * @param entry Pointer to a mapping entry object
 * @param id the member ID to set
 */
OSYNC_EXPORT void osync_mapping_entry_set_member_id(OSyncMappingEntry *entry, long long int id);


/**
 * @brief Get the ID of the entry
 * @param entry Pointer to a mapping entry object
 * @returns the entry's ID
 */
OSYNC_EXPORT long long int osync_mapping_entry_get_id(OSyncMappingEntry *entry);

/**
 * @brief Set the ID of the entry
 * @param entry Pointer to a mapping entry object
 * @param id the ID to set
 */
OSYNC_EXPORT void osync_mapping_entry_set_id(OSyncMappingEntry *entry, long long int id);

/*@}*/

#endif /*OPENSYNC_MAPPING_ENTRY_H_*/
