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
 
#ifndef _OPENSYNC_MAPPING_TABLE_INTERNALS_H_
#define _OPENSYNC_MAPPING_TABLE_INTERNALS_H_

/**
 * @defgroup OSyncMappingTableInternalAPI OpenSync Mapping Table Internals
 * @ingroup OSyncMappingPrivate
 * @brief Internals of OpenSync Mapping Table
 */

/*@{*/

/**
 * @brief List of Mappings
 */
struct OSyncMappingTable {
	/** Reference Counter */
	int ref_count;
	
	/** Contains all mappings */
	OSyncList *mappings;
};

/**
 * @brief Get the number of mappings in the mapping table
 *
 * @param table The mapping table object
 * @return Number of mappings
 */ 
OSYNC_TEST_EXPORT unsigned int osync_mapping_table_num_mappings(OSyncMappingTable *table);

/**
 * @brief Get nth mapping object from mapping table 
 *
 * @param table The mapping table object to count mappings
 * @param nth The position of the mapping object
 * @return The nth mapping object from mapping table or NULL if nth position isn't available 
 */ 
OSYNC_TEST_EXPORT OSyncMapping *osync_mapping_table_nth_mapping(OSyncMappingTable *table, unsigned int nth);

/*@}*/

#endif /* _OPENSYNC_MAPPING_TABLE_INTERNALS_H_ */
