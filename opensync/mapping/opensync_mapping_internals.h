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
 
#ifndef _OPENSYNC_MAPPING_INTERNALS_H_
#define _OPENSYNC_MAPPING_INTERNALS_H_

/**
 * @defgroup OSyncMappingPrivate OpenSync Mapping Module Private
 * @ingroup OSyncPrivate
 * @defgroup OSyncMappingInternalAPI OpenSync Mapping Internals
 * @ingroup OSyncMappingPrivate
 * @brief Internals of OpenSync Mapping
 */

/*@{*/

/**
 * @brief Entry list of Mappings
 */
struct OSyncMapping {
	/** Reference Counter */
	int ref_count;
	/** ID of the mapping */
	osync_mappingid id;
	/** list of entries */
	OSyncList *entries;
};

/**
 * @brief Get the number of entries in a mapping
 * @param mapping Pointer to a mapping object
 * @returns the number of entries in the mapping
 */
OSYNC_TEST_EXPORT unsigned int osync_mapping_num_entries(OSyncMapping *mapping);

/**
 * @brief Get the nth entry in a mapping
 * @param mapping Pointer to a mapping object
 * @param nth the index of the entry to get
 * @returns the nth entry in the mapping or NULL in case of error
 */
OSYNC_TEST_EXPORT OSyncMappingEntry *osync_mapping_nth_entry(OSyncMapping *mapping, unsigned int nth);


/*@}*/

#endif /* _OPENSYNC_MAPPING_INTERNALS_H_*/
