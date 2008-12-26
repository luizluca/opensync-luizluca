/*
 * libosengine - A synchronization engine for the opensync framework
 * Copyright (C) 2004-2005  Armin Bauer <armin.bauer@opensync.org>
 * Copyright (C) 2008       Daniel Gollub <dgollub@suse.de>
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
 
#ifndef OPENSYNC_MAPPING_ENTRY_ENGINE_H_
#define OPENSYNC_MAPPING_ENTRY_ENGINE_H_

/**
 * @defgroup OSyncMappingEntryEngineAPI OpenSync Mapping Entry Engine 
 * @ingroup OSyncEngine
 * @brief Handles Mapping Entries 
 * 
 * Mapping Entry Engine takes care about an entry/record for a specific member
 * (i.e. SinkEngine).
 *
 * After all reported changes got mapped and multiplied, the Mapping Entry
 * Engine object contains of:
 *  - status if the entry needs to get committed (i.e. "dirty" flag)
 *  - member/SinkEngine specific change type
 *  - an own copy of the mapped change
 *  - pointer to the assigned OSyncMappingEntry object
 */
/*@{*/

/*! @brief Get the dirty flag for Mapping Entry Engine 
 *
 * If dirty flag for OSyncMappingEntryEngine is set, the entry gets handled
 * for the assigned peer in the engine WRITE section, e.g. entry gets
 * deleted/added/modified. If not set the entry is unmodified for the assigned
 * peer and doesn't get changed.
 *
 * @param engine Pointer to an OSyncMappingEntryEngine
 * @returns TRUE if dirty flag is set, FALSE otherwise
 */
OSYNC_EXPORT osync_bool osync_entry_engine_is_dirty(OSyncMappingEntryEngine *engine);

/*! @brief Get change type of Mapping Entry Engine for assigned peer 
 *
 * If no entry is assigned to the Mapping Entry Engine the result is
 * OSYNC_CHANGE_TYPE_UNKNOWN. If an entry is assigned but is unmodified the
 * result is OSYNC_CHANGE_TYPE_UNMODIFIED.
 *
 * Usually the change type of modified entries are:
 *  - OSYNC_CHANGE_TYPE_ADDED
 *  - OSYNC_CHANGE_TYPE_MODIFIED
 *  - OSYNC_CHANGE_TYPE_DELETED
 *
 * @param engine Pointer to an OSyncMappingEntryEngine
 * @returns Changetype of change in Mapping Entry Engine 
 */
OSYNC_EXPORT OSyncChangeType osync_entry_engine_get_changetype(OSyncMappingEntryEngine *engine);

/*@}*/

#endif /*OPENSYNC_MAPPING_ENTRY_ENGINE_H_*/
