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


/** @brief Get the nth OSyncClientProxy of the OSyncEngine
 *
 * @param engine A pointer to the engine
 * @param nth The position of the OSyncClientProxy to request
 * @returns Pointer of the nth OSyncClientProxy
 *
 */
OSyncClientProxy *osync_engine_nth_proxy(OSyncEngine *engine, unsigned int nth);

/** @brief Get the number of OSyncClientProxy-elements in OSyncEngine 
 *
 * @param engine A pointer to the engine
 * @returns Total number of OSyncClientProxy-elements 
 *
 */
unsigned int osync_engine_num_proxies(OSyncEngine *engine);

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



/** @brief Trace the multiply result of the entire engine. 
 *  
 *  This summaries which changes get synced how to the members.
 *  Very helpful to debug OSyncEngine Multiply issues.
 *
 *  This is NOOP if tracing is disabled.
 *
 * @param engine Pointer to engine
 *
 */
OSYNC_TEST_EXPORT void osync_engine_trace_multiply_summary(OSyncEngine *engine);


/** @brief Reassign entries with a different objtype then the ObjEngine, to the
 *         native ObjEngine. 
 *
 * TODO: This needs serious profiling.
 *  
 * @param engine Pointer to engine
 * @param error Pointer to error-struct which get set on any error
 * @returns TRUE on success, or FALSE on any error
 *
 */
osync_bool osync_engine_handle_mixed_objtypes(OSyncEngine *engine, OSyncError **error);

/** @brief Trigger slow-sync for object engines which have mixed object types synced
 *
 * This function checks if an object engine has requested a slow-sync and if
 * further function need to run a slow-sync. Due to mixed object type syncing.
 * Mixed object type syncing get detected by checking the mapping table if different
 * object engine have written non-native object types.
 *  
 * @param engine Pointer to engine
 * @param error Pointer to error-struct which get set on any error
 * @returns TRUE on success, or FALSE on any error
 *
 */
osync_bool osync_engine_slowsync_for_mixed_objengines(OSyncEngine *engine, OSyncError **error);

/*@}*/

#endif /*OPENSYNC_ENGINE_INTERNALS_H_*/
