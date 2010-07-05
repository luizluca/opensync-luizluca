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
 
#ifndef OPENSYNC_MAPPING_TABLE_H_
#define OPENSYNC_MAPPING_TABLE_H_

/**
 * @defgroup OSyncMappingTableAPI OpenSync Mapping Table
 * @ingroup OSyncMapping
 * @brief Functions for managing tables of mappings between member uids
 * 
 * @see OSyncMappingAPI, OSyncMappingEntryAPI
 */
/*@{*/

/**
 * @brief Creates a new mapping table object
 * @param error Pointer to an error struct
 * @return The pointer to the newly allocated mapping table object or NULL in case of error
 */

OSYNC_EXPORT OSyncMappingTable *osync_mapping_table_new(OSyncError **error);

/**
 * @brief Increments the reference counter
 * @param table The pointer to an mapping table object
 */
OSYNC_EXPORT OSyncMappingTable *osync_mapping_table_ref(OSyncMappingTable *table);

/**
 * @brief Decrement the reference counter. The mapping table object will 
 *  be freed if the reference count reaches zero.
 * @param table The pointer to an mapping table object
 */
OSYNC_EXPORT void osync_mapping_table_unref(OSyncMappingTable *table);


/**
 * @brief Loads all mappings from archive for a certain object type.
 *
 * @param table The mapping table object
 * @param archive The archive
 * @param objtype Requested object type 
 * @param error Pointer to an error struct
 * @return TRUE on when all mappings successfully loaded otherwise FALSE
 */ 
OSYNC_EXPORT osync_bool osync_mapping_table_load(OSyncMappingTable *table, OSyncArchive *archive, const char *objtype, OSyncError **error);

/**
 * @brief Delete all mappings from the mapping table and the archive for a certain object type.
 *
 * @param table The mapping table object
 * @param archive The archive
 * @param objtype Reported object type of entry
 * @param error Pointer to an error struct
 * @return Returns TRUE on success, FALSE otherwise 
 */ 
OSYNC_EXPORT osync_bool osync_mapping_table_flush(OSyncMappingTable *table, OSyncArchive *archive, const char *objtype, OSyncError **error);


/**
 * @brief Close the mapping table 
 *
 * @param table The mapping table object
 */ 
OSYNC_EXPORT void osync_mapping_table_close(OSyncMappingTable *table);


/**
 * @brief Search for the mapping object with the mapping id 
 *
 * @param table The mapping table object
 * @param id The mapping id to search for
 * @return Returns Mapping object or NULL if no mapping matched the mapping id
 */ 
OSYNC_EXPORT OSyncMapping *osync_mapping_table_find_mapping(OSyncMappingTable *table, osync_mappingid id);

/**
 * @brief Add a mapping to the mapping table 
 *
 * @param table The mapping table object
 * @param mapping The mapping to add to the mapping table
 */ 
OSYNC_EXPORT void osync_mapping_table_add_mapping(OSyncMappingTable *table, OSyncMapping *mapping);

/**
 * @brief Remove a mapping from the mapping table 
 *
 * @param table The mapping table object
 * @param mapping The mapping to remove from the mapping table
 */ 
OSYNC_EXPORT void osync_mapping_table_remove_mapping(OSyncMappingTable *table, OSyncMapping *mapping);

/**
 * @brief Returns a OSyncList that contains the OSyncMapping of this mapping table
 * 
 * Please be aware that the returned list has to be freed with 
 * osync_list_free. If it isn't freed there will be a memory leak.
 * 
 * @param table A pointer to a OSyncMappingTable
 * @return A shallow copy of the internal list of OSyncMappings
 */
OSYNC_EXPORT OSyncList *osync_mapping_table_get_mappings(OSyncMappingTable *table);

/**
 * @brief Get next free mapping id from mapping table 
 *
 * @param table The mapping table object
 * @return Next free mapping id of mapping table
 */ 
OSYNC_EXPORT osync_mappingid osync_mapping_table_get_next_id(OSyncMappingTable *table);

/*@}*/

#endif /* OPENSYNC_MAPPING_TABLE_H_ */
