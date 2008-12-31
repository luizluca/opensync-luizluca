/*
 * libopensync - A synchronization engine for the opensync framework
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
 
#ifndef OPENSYNC_ENGINE_INTERNALS_H_
#define OPENSYNC_ENGINE_INTERNALS_H_

/**
 * @defgroup OSyncEngineInternalAPI OpenSync Engine Internals
 * @ingroup OSyncEnginePrivate
 * @brief Internals of OpenSync Engine
 */

osync_bool osync_engine_check_get_changes(OSyncEngine *engine);

void osync_engine_event(OSyncEngine *engine, OSyncEngineEvent event);

OSyncClientProxy *osync_engine_find_proxy(OSyncEngine *engine, OSyncMember *member);

OSyncArchive *osync_engine_get_archive(OSyncEngine *engine);
OSYNC_TEST_EXPORT OSyncGroup *osync_engine_get_group(OSyncEngine *engine);

OSyncObjEngine *osync_engine_nth_objengine(OSyncEngine *engine, int nth);
int osync_engine_num_objengine(OSyncEngine *engine);

OSyncClientProxy *osync_engine_nth_proxy(OSyncEngine *engine, int nth);
int osync_engine_num_proxies(OSyncEngine *engine);

/**
 * @brief Get "human readable" string of OSyncEngineCmd enum 
 *
 * @param cmd OSyncEngineCmd enum value 
 * @returns String of corresponding  OSyncEngineCmd
 */
const char *osync_engine_get_cmdstr(OSyncEngineCmd cmd);

/**
 * @brief Get "human readable" string of OSyncEngineEvent enum 
 *
 * @param event OSyncEngineEvent enum value 
 * @returns String of corresponding  OSyncEngineEvent
 */
const char *osync_engine_get_eventstr(OSyncEngineEvent event);

/**
 * @brief Check if engine has an error. 
 *
 * @param engine Pointer of OSyncEngine 
 * @returns TURE if engine has an error, FALSE otherwise 
 */
OSYNC_TEST_EXPORT osync_bool osync_engine_has_error(OSyncEngine *engine);

/**
 * @brief Queue an engine command 
 *
 * This function is useful to queue asynchronous engine commands.
 *
 * @param engine Pointer of OSyncEngine 
 * @param cmdid ID of Engine Command (OSyncEngineCmd) to queue
 * @param error Pointer to an error struct which stores information about the failed queuing
 * @returns TURE if command got queued successful, FALSE otherwise 
 */
osync_bool osync_engine_queue_command(OSyncEngine *engine, OSyncEngineCmd cmdid, OSyncError **error);

OSYNC_TEST_EXPORT void osync_engine_set_formatdir(OSyncEngine *engine, const char *dir);
OSYNC_TEST_EXPORT void osync_engine_set_plugindir(OSyncEngine *engine, const char *dir);

/** @brief Set the schemadir for schema validation to a custom directory.
 *  This is actually only inteded for UNITTESTS to run tests without
 *  having OpenSync installed.
 *
 * @param engine Pointer to engine
 * @param schemadir Custom schemadir path
 *
 */
OSYNC_TEST_EXPORT void osync_engine_set_schemadir(OSyncEngine *engine, const char *schema_dir);

/*@}*/

#endif /*OPENSYNC_ENGINE_INTERNALS_H_*/
