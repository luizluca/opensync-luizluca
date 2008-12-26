/*
 * libosengine - A synchronization engine for the opensync framework
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
 
#ifndef OPENSYNC_MAPPING_ENTRY_ENGINE_INTERNALS_H_
#define OPENSYNC_MAPPING_ENTRY_ENGINE_INTERNALS_H_

#include "opensync_sink_engine_internals.h"

/* @TODO: move struct to private header */
struct OSyncMappingEntryEngine {
	int ref_count;
	OSyncSinkEngine *sink_engine;
	osync_bool dirty;
	OSyncChange *change;
	OSyncObjEngine *objengine;
	OSyncMappingEngine *mapping_engine;
	OSyncMappingEntry *entry;
};

/**
 * @defgroup OSyncMappingEntryEnginePrivateAPI OpenSync Mapping Entry Engine 
 * @ingroup OSyncEnginePrivate
 * @brief Private functions to handle Mapping Entries 
 * 
 */
/*@{*/

/*! @brief Register a new Mapping Entry Engine
 *
 * @param entry Pointer to an OSyncMappingEntry
 * @param mapping_engine Pointer to an OSyncMappingEngine
 * @param sink_engine Pointer to an OSyncSinkEngine
 * @param error Pointer to an error struct
 * @returns the newly registered Mapping Entry Engine, NULL on error
 */
OSyncMappingEntryEngine *osync_entry_engine_new(OSyncMappingEntry *entry, OSyncMappingEngine *mapping_engine, OSyncSinkEngine *sink_engine, OSyncObjEngine *objengine, OSyncError **error);

/*! @brief Increase the reference count on a Mapping Entry Engine 
 * 
 * @param plugin Pointer to the Mapping Entry Engine
 * 
 */
OSyncMappingEntryEngine *osync_entry_engine_ref(OSyncMappingEntryEngine *engine);

/*! @brief Decrease the reference count on a Mapping Entry Engine 
 * 
 * @param plugin Pointer to the Mapping Entry Engine
 * 
 */
void osync_entry_engine_unref(OSyncMappingEntryEngine *engine);

/*! @brief Tries to match OSyncMappingEntryEngine with a OSyncChange.
 *
 *  Based on the UID of the OSyncChange and the Mapping Entry of the
 *  OSyncMappingEntryEngine, this functions tries to match the two objects.
 *
 *  If the Mapping Entry of the OSyncMappingEntryEngine is not yet assigned to
 *  a real record (i.e. UID not set) the match fails.
 *
 * @param engine Pointer to an OSyncMappingEntryEngine with a Mapping Entry 
 * @param change Pointer to an OSyncChange with UID set
 * @returns TRUE if engine and change matches, FALSE otherwise. 
 */
osync_bool osync_entry_engine_matches(OSyncMappingEntryEngine *engine, OSyncChange *change);

/*! @brief Get the pointer of the assinged OSyncChange object 
 *
 * @param engine Pointer to an OSyncMappingEntryEngine
 * @returns Pointer to assigned OSyncChange, or NULL if no change is set
 */
OSyncChange *osync_entry_engine_get_change(OSyncMappingEntryEngine *engine);

/*! @brief Update the OSyncChange object in Mapping Entry Engine 
 *
 * Mapping Entry Engine get flagged as not sync in the Mapping Table, when
 * change get updated.
 *
 * @param engine Pointer to an OSyncMappingEntryEngine
 * @param change Pointer to new change to assign or NULL to unset the current change
 */
void osync_entry_engine_update(OSyncMappingEntryEngine *engine, OSyncChange *change);

/*! @brief Set the dirty flag for OSyncMappingEntryEngine 
 *
 * If dirty flag for OSyncMappingEntryEngine get set, the entry gets handled
 * in the engine WRITE section.
 *
 * @param engine Pointer to an OSyncMappingEntryEngine
 * @param change Pointer to new change to assign or NULL to unset the current change
 */
void osync_entry_engine_set_dirty(OSyncMappingEntryEngine *engine, osync_bool dirty);

/*@}*/

#endif /* OPENSYNC_MAPPING_ENTRY_ENGINE_INTERNALS_H_ */

