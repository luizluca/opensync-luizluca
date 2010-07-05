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
 
#ifndef _OPENSYNC_MAPPING_H_
#define _OPENSYNC_MAPPING_H_

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
OSYNC_EXPORT osync_mappingid osync_mapping_get_id(OSyncMapping *mapping);

/**
 * @brief Set the ID of a mapping object
 * @param mapping Pointer to a mapping object
 * @param id the mapping ID to set
 */
OSYNC_EXPORT void osync_mapping_set_id(OSyncMapping *mapping, osync_mappingid id);

/**
 * @brief Returns a OSyncList that contains the OSyncMappingEntries of this mapping
 * 
 * Please be aware that the returned list has to be freed with 
 * osync_list_free. If it isn't freed there will be a memory leak.
 * 
 * @param mapping A pointer to a OSyncMapping
 * @return A shallow copy of the internal list of OSyncMappingEntries
 */
OSYNC_EXPORT OSyncList *osync_mapping_get_entries(OSyncMapping *mapping);

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
OSYNC_EXPORT OSyncMappingEntry *osync_mapping_find_entry_by_member_id(OSyncMapping *mapping, osync_memberid memberid);

/*@}*/

#endif /* _OPENSYNC_MAPPING_H_*/
