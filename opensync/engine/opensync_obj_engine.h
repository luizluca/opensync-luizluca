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
 
#ifndef OPENSYNC_OBJ_ENGINE_H_
#define OPENSYNC_OBJ_ENGINE_H_

typedef void (* OSyncObjEngineEventCallback) (OSyncObjEngine *engine, OSyncEngineEvent event, OSyncError *error, void *userdata);

OSYNC_EXPORT OSyncObjEngine *osync_obj_engine_new(OSyncEngine *engine, const char *objtype, OSyncFormatEnv *formatenv, OSyncError **error);
OSYNC_EXPORT OSyncObjEngine *osync_obj_engine_ref(OSyncObjEngine *engine);
OSYNC_EXPORT void osync_obj_engine_unref(OSyncObjEngine *engine);

OSYNC_EXPORT osync_bool osync_obj_engine_initialize(OSyncObjEngine *engine, OSyncError **error);
OSYNC_EXPORT void osync_obj_engine_finalize(OSyncObjEngine *engine);

OSYNC_EXPORT const char *osync_obj_engine_get_objtype(OSyncObjEngine *engine);

OSYNC_EXPORT void osync_obj_engine_set_slowsync(OSyncObjEngine *engine, osync_bool slowsync);
OSYNC_EXPORT osync_bool osync_obj_engine_get_slowsync(OSyncObjEngine *engine);

OSYNC_EXPORT void osync_obj_engine_event(OSyncObjEngine *objengine, OSyncEngineEvent event, OSyncError *error);
OSYNC_EXPORT osync_bool osync_obj_engine_command(OSyncObjEngine *engine, OSyncEngineCmd cmd, OSyncError **error);
OSYNC_EXPORT void osync_obj_engine_set_callback(OSyncObjEngine *engine, OSyncObjEngineEventCallback callback, void *userdata);
OSYNC_EXPORT osync_bool osync_obj_engine_receive_change(OSyncObjEngine *objengine, OSyncClientProxy *proxy, OSyncChange *change, OSyncError **error);

OSYNC_EXPORT void osync_obj_engine_set_error(OSyncObjEngine *engine, OSyncError *error);

/** @brief Get list of OSyncMappingEntryEngines of the OSyncObjEngine 
 * for a specific member
 *
 * @param engine Pointer to an OSyncObjEngine
 * @param member Pointer to OSyncMember to get OSyncMappingEntryEngine list from
 * @returns List of OSyncMappingEntryEngines-elements or NULL if there are no Mapping Entry Engines. 
 */
OSYNC_EXPORT const OSyncList *osync_obj_engine_get_mapping_entry_engines_of_member(OSyncObjEngine *engine, OSyncMember *member);

/**
 * @brief Returns a OSyncList of all active sinkengines
 * 
 * Please be aware that the returned list has to be freed with 
 * osync_list_free. If it isn't freed there will be a memory leak.
 * 
 * @param A pointer to a OSyncObjEngine
 * @return a shallow copy of active OSyncSinkEngines
 */
OSYNC_EXPORT OSyncList *osync_obj_engine_get_sinkengines(OSyncObjEngine *engine);

/**
 * @brief Returns a OSyncList of all members
 * 
 * Please be aware that the returned list has to be freed with 
 * osync_list_free. If it isn't freed there will be a memory leak.
 * 
 * @param A pointer to a OSyncObjEngine
 * @return a shallow copy of the internal OSyncMembers
 */
OSYNC_EXPORT OSyncList *osync_obj_engine_get_members(OSyncObjEngine *engine);

#endif /* OPENSYNC_OBJ_ENGINE_H_ */

