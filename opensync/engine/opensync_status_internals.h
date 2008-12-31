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
 
#ifndef OPENSYNC_STATUS_INTERNALS_H_
#define OPENSYNC_STATUS_INTERNALS_H_

/**
 * @defgroup OSyncStatusPrivate OpenSync Status Private
 * @ingroup OSyncPrivate
 * @defgroup OSyncStatusInternalAPI OpenSync Status Internals
 * @ingroup OSyncStatusPrivate
 * @brief Internals of OpenSync Status 
 */

/*@{*/

void osync_status_free_member_update(OSyncMemberUpdate *update);
void osync_status_free_engine_update(OSyncEngineUpdate *update);
void osync_status_free_change_update(OSyncChangeUpdate *update);
void osync_status_free_mapping_update(OSyncMappingUpdate *update);

void osync_status_update_member(OSyncEngine *engine, OSyncMember *member, OSyncMemberEvent type, const char *objtype, OSyncError *error);
void osync_status_update_engine(OSyncEngine *engine, OSyncEngineEvent type, OSyncError *error);
void osync_status_update_change(OSyncEngine *engine, OSyncChange *change, OSyncMember *member, OSyncMapping *mapping, OSyncChangeEvent type, OSyncError *error);


void osync_status_update_mapping(OSyncEngine *engine, OSyncMappingEngine *mapping, OSyncMappingEvent type, OSyncError *error);

/**
 * @brief Send conflict status for a mapping
 *
 * This calls the registered callback function for conflicts. If the callback aborts the
 * synchronization, by calling osync_engine_abort() the function fails and returns FALSE.
 *
 * The function will also return with FALSE if the engine is already in error
 * state. The callback function will not get called if the engien is already
 * in an error state.
 *
 * This functions blocks until the registered callback function returns.
 * Usually this means that the conflict got solved. But it's also
 * possible that the frontend "delayed" the conflict handling, and tries to
 * handle the conflict handling in a asynchronous way.
 *
 * @param engine Pointer to OSyncEngine struct
 * @param mapping_engine Pointer to the OSyncMappingEngine struct of the conflict 
 * @returns TRUE if callback function returned without OSyncEngine error, FALSE otherwise
 */
osync_bool osync_status_conflict(OSyncEngine *engine, OSyncMappingEngine *mapping_engine);

/*@}*/

#endif /* OPENSYNC_STATUS_INTERNALS_H_ */

