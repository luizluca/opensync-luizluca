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
 
#ifndef OPENSYNC_MAPPING_ENTRY_INTERNALS_H_
#define OPENSYNC_MAPPING_ENTRY_INTERNALS_H_

/**
 * @defgroup OSyncMappingEntryInternalAPI OpenSync Mapping Entry Internals
 * @ingroup OSyncMappingPrivate
 * @brief Internals of OpenSync Mapping Entry
 */

/*@{*/

/**
 * @brief A Mapping Entry
 */
struct OSyncMappingEntry {
	/** Reference Counter */
	int ref_count;
	
	/** unique identifier of the item/entry in the member's system */
	char *uid;
	
	/** uniquely identifies the member */
	osync_memberid member_id;
	/** uniquely identifies the mapping table entry */
	osync_mappingid id;
};

/*@}*/

#endif /*OPENSYNC_MAPPING_ENTRY_INTERNALS_H_*/
