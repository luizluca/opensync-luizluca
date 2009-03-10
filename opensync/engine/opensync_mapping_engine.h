/*
 * libopensync - A synchronization framework
 * Copyright (C) 2004-2006  Armin Bauer <armin.bauer@desscon.com>
 * Copyright (C) 2007       Daniel Gollub <dgollub@suse.de> 
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
 
#ifndef OPENSYNC_MAPPING_ENGINE_H_
#define OPENSYNC_MAPPING_ENGINE_H_

OSYNC_EXPORT unsigned int osync_mapping_engine_num_changes(OSyncMappingEngine *engine);

/** @brief Search for the nth entry in the mapping
 *
 * @param engine A pointer to the mapping engine
 * @param nth The value of the position
 * @returns The pointer to the nth change. NULL if there isn't enough entries in the mapping.
 */
OSYNC_EXPORT OSyncChange *osync_mapping_engine_nth_change(OSyncMappingEngine *engine, unsigned int nth);

/** @brief Search in the mapping for the change of the member.
 *
 * @param engine A pointer to the mapping engine
 * @param memberid The member id of the request change.
 * @returns The pointer to the change of the member. NULL if member doesn't have an entry in this mapping.
 */
OSYNC_EXPORT OSyncChange *osync_mapping_engine_member_change(OSyncMappingEngine *engine, int memberid);

OSYNC_EXPORT OSyncMember *osync_mapping_engine_change_find_member(OSyncMappingEngine *engine, OSyncChange *change);

OSYNC_EXPORT osync_bool osync_mapping_engine_supports_ignore(OSyncMappingEngine *engine);
OSYNC_EXPORT osync_bool osync_mapping_engine_supports_use_latest(OSyncMappingEngine *engine);

OSYNC_EXPORT osync_bool osync_mapping_engine_solve(OSyncMappingEngine *engine, OSyncChange *change, OSyncError **error);
OSYNC_EXPORT osync_bool osync_mapping_engine_ignore(OSyncMappingEngine *engine, OSyncError **error);
OSYNC_EXPORT osync_bool osync_mapping_engine_use_latest(OSyncMappingEngine *engine, OSyncError **error);

/** @brief Solves the conflict by duplicating the conflicting entries
 *
 * @param engine The engine
 * @param dupe_mapping The conflicting mapping to duplicate
 *
 */
OSYNC_EXPORT osync_bool osync_mapping_engine_duplicate(OSyncMappingEngine *existingMapping, OSyncError **error);

#endif /*OPENSYNC_MAPPING_ENGINE_H_*/
