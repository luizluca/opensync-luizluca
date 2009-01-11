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
 
#ifndef OPENSYNC_OBJ_ENGINE_INTERNALS_H_
#define OPENSYNC_OBJ_ENGINE_INTERNALS_H_

struct OSyncObjEngine {
	/** Reference counting */
	int ref_count;
	
	/** Pointer to (parent) OSyncEngine */
	OSyncEngine *parent;

	/** The object type of this Object Engine **/
	char *objtype;

	/** Status of Slow Sync **/
	osync_bool slowsync;

	/** Pointer to assinged OSyncArchive */
	OSyncArchive *archive;
	
	/** Pointer to assinged OSyncMappingTable */
	OSyncMappingTable *mapping_table;

	/** List of OSyncMappingEngine-elemenets */
	GList *mapping_engines;
	
	/** List of OSyncSinkEngine-elements */
	GList *sink_engines;

	/** Pointer to OSyncObjEngine assinged error struct */
	OSyncError *error;
	/** Pointer to format enviornment */
	OSyncFormatEnv *formatenv;
	
	/** Total number of Sink Engine errors */
	int sink_errors;
	/** Total number of initiated connections */
	int sink_connects;
	/** Total number of finalized connections */
	int sink_connect_done;
	/** Total number of disconnects */
	int sink_disconnects;
	/** Total number of get latest changes/all recores calls */
	int sink_get_changes;
	/** Total number of completed synchronization */
	int sink_sync_done;
	/** Total number of completed write-/commit-phases */
	int sink_written;
	
	/** Callback to give feedback to (parent) OSyncEngine */ 
	OSyncObjEngineEventCallback callback;
	/** Userdata poitner of callback */
	void *callback_userdata;
	
	/** List of OSyncMappingEngine-elements with pending conflicts */
	GList *conflicts;

	/** Written status of Object Engine. - TODO: Is this still needed?! **/
	osync_bool written;

	/** Conflicts already solved */
	osync_bool conflicts_solved;

	/** Total number of "dummy" OSyncSinkEngine-elements */
	unsigned int dummies;
};

OSyncMappingEngine *_osync_obj_engine_create_mapping_engine(OSyncObjEngine *engine, OSyncError **error);

/*! @brief Get number of "active" Sink Engines (excluded "dummy" Sink Engines)
 *         in ObjEngine
 *
 * This function should be used to determine the number of "active" Sink
 * Engines. Original purpose of this function is to determine the number of
 * Sink Engine to wait for. Excluded are all Sink Engines which have the
 * attribute "dummy" set.
 *
 * @param engine Pointer to OSyncObjEngine
 * @returns Number of "active" Sink Engines
 */
unsigned int osync_obj_engine_num_active_sinkengines(OSyncObjEngine *engine);

/*! @brief Get total number of OSyncMappingEngines of this OSyncObjEngine
 *
 * @param engine Pointer to OSyncObjEngine
 * @returns Total number of "active" and "dummy" Sink Engines
 */
unsigned int osync_obj_engine_num_mapping_engines(OSyncObjEngine *engine);

/*! @brief Prepare the OSyncObjEngine for writing
 *
 * This function prepare the write process, by demerging and converting if
 * required/configured.
 *
 * @param engine Pointer to OSyncObjEngine to prepare
 * @param engine Pointer to error struct, which get set on any error
 * @returns TRUE on success, FALSE otherwise
 */
osync_bool osync_obj_engine_prepare_write(OSyncObjEngine *engine, OSyncError **error);

#endif /* OPENSYNC_OBJ_ENGINE_INTERNALS_H_ */

