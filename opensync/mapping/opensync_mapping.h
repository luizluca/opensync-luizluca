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
 
#ifndef OPENSYNC_MAPPING_H_
#define OPENSYNC_MAPPING_H_

/**
 * @defgroup OSyncMapping OpenSync Mapping Module
 * @ingroup OSyncPublic
 * @defgroup OSyncMappingAPI OpenSync Mapping
 * @ingroup OSyncMapping
 * @brief Functions for managing mappings between multiple member uids
 * 
 * A mapping links together uids (unique IDs referring to a data item in
 * the member system) in different member systems. Mappings usually 
 * contain two or more entries, one for each member uid in the mapping.
 * 
 * @see OSyncMappingTableAPI, OSyncMappingEntryAPI
 */
/*@{*/

/**
 * @brief Creates a new mapping object
 * @param error Pointer to an error struct
 * @return The pointer to the newly allocated mapping object or NULL in case of error
 */
OSYNC_EXPORT OSyncMapping *osync_mapping_new(OSyncError **error);

/**
 * @brief Increments the reference counter
 * @param mapping Pointer to a mapping object
 */
OSYNC_EXPORT OSyncMapping *osync_mapping_ref(OSyncMapping *mapping);

/**
 * @brief Decrement the reference counter. The mapping table object will 
 *  be freed if the reference count reaches zero.
 * @param mapping Pointer to a mapping object
 */
OSYNC_EXPORT void osync_mapping_unref(OSyncMapping *mapping);


/**
 * @brief Get the ID of a mapping object
 * @param mapping Pointer to a mapping object
 * @returns the mapping ID
 */
OSYNC_EXPORT long long int osync_mapping_get_id(OSyncMapping *mapping);

/**
 * @brief Set the ID of a mapping object
 * @param mapping Pointer to a mapping object
 * @param id the mapping ID to set
 */
OSYNC_EXPORT void osync_mapping_set_id(OSyncMapping *mapping, long long int id);


/**
 * @brief Get the number of entries in a mapping
 * @param mapping Pointer to a mapping object
 * @returns the number of entries in the mapping
 */
OSYNC_EXPORT int osync_mapping_num_entries(OSyncMapping *mapping);

/**
 * @brief Get the nth entry in a mapping
 * @param mapping Pointer to a mapping object
 * @param nth the index of the entry to get
 * @returns the nth entry in the mapping or NULL in case of error
 */
OSYNC_EXPORT OSyncMappingEntry *osync_mapping_nth_entry(OSyncMapping *mapping, int nth);


/**
 * @brief Add an entry to a mapping
 * @param mapping Pointer to a mapping object
 * @param entry Pointer to the mapping entry object to add
 */
OSYNC_EXPORT void osync_mapping_add_entry(OSyncMapping *mapping, OSyncMappingEntry *entry);

/**
 * @brief Remove an entry from a mapping
 * @param mapping Pointer to a mapping object
 * @param entry Pointer to the mapping entry object to remove
 */
OSYNC_EXPORT void osync_mapping_remove_entry(OSyncMapping *mapping, OSyncMappingEntry *entry);


/**
 * @brief Find an entry in a mapping by its member ID
 * @param mapping Pointer to a mapping object
 * @param memberid The ID of the member
 * @returns the entry with the specified member ID or NULL if not found
 */
OSYNC_EXPORT OSyncMappingEntry *osync_mapping_find_entry_by_member_id(OSyncMapping *mapping, long long int memberid);

/*@}*/

#endif /*OPENSYNC_MAPPING_H_*/
