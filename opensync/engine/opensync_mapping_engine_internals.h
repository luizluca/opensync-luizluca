/*
 * libosengine - A synchronization engine for the opensync framework
 * Copyright (C) 2004-2005  Armin Bauer <armin.bauer@opensync.org>
 * Copyright (C) 2007       Daniel Gollub <gollub@b1-systems.de> 
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
 
#ifndef OPENSYNC_MAPPING_ENGINE_INTERNALS_H_
#define OPENSYNC_MAPPING_ENGINE_INTERNALS_H_

#include "opensync_mapping_entry_engine_internals.h"

/**
 * @defgroup OSyncMappingEngineInternalAPI OpenSync Mapping Engine Internals
 * @ingroup OSyncEnginePrivate
 * @brief The internal part of the OSyncMappingEngine API
 * 
 */
/*@{*/

struct OSyncMappingEngine {
	int ref_count;
	OSyncMapping *mapping;
	OSyncMappingEntryEngine *master;
	OSyncList *entries; /* OSyncMappingEntryEngine */
	OSyncObjEngine *parent;
	osync_bool conflict;
	osync_bool synced;
};

OSyncMappingEngine *osync_mapping_engine_new(OSyncObjEngine *parent, OSyncMapping *mapping, OSyncError **error);
OSyncMappingEngine *osync_mapping_engine_ref(OSyncMappingEngine *engine);
void osync_mapping_engine_unref(OSyncMappingEngine *engine);

osync_bool osync_mapping_engine_multiply(OSyncMappingEngine *engine, OSyncError **error);

/**
 * @brief Checks for conflicts in the Mapping Engine and calls conflict
 *        callbacks if required.
 *
 * On a conflict the conflict callbacks get called in a syncrhounous way
 * and might block. But the application could also delay the conflict
 * resolution and solve it later.
 *
 * If this functions fails, by returning FALSE, synchronization got aborted
 * during the conflict resolution.
 *
 * @param engine Pointer to the Mapping Engine struct 
 * @return Returns TRUE on success, FALSE if conflict resolution got aborted
 */
osync_bool osync_mapping_engine_check_conflict(OSyncMappingEngine *engine);

OSyncMappingEntryEngine *osync_mapping_engine_get_entry(OSyncMappingEngine *engine, OSyncSinkEngine *sinkengine);

OSYNC_TEST_EXPORT unsigned int osync_mapping_engine_num_changes(OSyncMappingEngine *engine);

/** @brief Search for the nth entry in the mapping
 *
 * @param engine A pointer to the mapping engine
 * @param nth The value of the position
 * @returns The pointer to the nth change. NULL if there isn't enough entries in the mapping.
 */
OSYNC_TEST_EXPORT OSyncChange *osync_mapping_engine_nth_change(OSyncMappingEngine *engine, unsigned int nth);

/*@}*/

#endif /* OPENSYNC_MAPPING_ENGINE_INTERNALS_H_ */

