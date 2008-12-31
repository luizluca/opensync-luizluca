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
 
#ifndef OPENSYNC_ENGINE_PRIVATE_H_
#define OPENSYNC_ENGINE_PRIVATE_H_

/**
 * @defgroup OSyncEnginePrivate OpenSync Engine Module Private
 * @ingroup OSyncPrivate
 * @defgroup OSyncEnginePrivateAPI OpenSync Engine Private
 * @ingroup OSyncEnginePrivate
 */

/*@{*/

typedef enum {
	OSYNC_ENGINE_SOLVE_DUPLICATE,
	OSYNC_ENGINE_SOLVE_CHOOSE,
	OSYNC_ENGINE_SOLVE_IGNORE,
	OSYNC_ENGINE_SOLVE_USE_LATEST
} OSyncEngineSolveType;

typedef struct OSyncEngineCommand {
	OSyncEngineCmd cmd;
	OSyncMappingEngine *mapping_engine;
	OSyncChange *master;
	OSyncEngineSolveType solve_type;
	OSyncMember *member;
} OSyncEngineCommand;

struct OSyncEngine {
	int ref_count;
	/** The opensync group **/
	OSyncGroup *group;
	OSyncArchive *archive;
	
	char *engine_path;
	char *plugin_dir;
	char *format_dir;
#ifdef OPENSYNC_UNITTESTS
	char *schema_dir;
#endif /* OPENSYNC_UNITTESTS */	
	OSyncFormatEnv *formatenv;
	OSyncPluginEnv *pluginenv;
	
	OSyncEngineState state;
	
	osync_conflict_cb conflict_callback;
	void *conflict_userdata;

	osync_multiply_cb multiply_callback;
	void *multiply_userdata;
	
	osync_status_change_cb changestat_callback;
	void *changestat_userdata;
	
	osync_status_member_cb mebstat_callback;
	void *mebstat_userdata;
	
	osync_status_engine_cb engstat_callback;
	void *engstat_userdata;
	
	osync_status_mapping_cb mapstat_callback;
	void *mapstat_userdata;
	
	/** The g_main_loop of this engine **/
	OSyncThread *thread;
	GMainContext *context;
	
	GAsyncQueue *command_queue;
	GSourceFuncs *command_functions;
	GSource *command_source;
	
	GCond* syncing;
	GMutex* syncing_mutex;
	
	GCond* started;
	GMutex* started_mutex;
	
	/** proxies contains a list of all OSyncClientProxy objects **/
	GList *proxies;

	/** object_engines contains a list of all OSyncObjEngine objects **/
	GList *object_engines;

	osync_bool man_dispatch;
	osync_bool allow_sync_alert;
	
	OSyncError *error;

	/** disconnecting status, if TRUE engine is already busy with disconnecting **/
	osync_bool disconnecting;
	
	int proxy_connects;
	int proxy_connect_done;
	int proxy_disconnects;
	int proxy_get_changes;
	int proxy_written;
	int proxy_sync_done;
	int proxy_errors;
	
	int obj_errors;
	int obj_connects;
	int obj_connect_done;
	int obj_disconnects;
	int obj_get_changes;
	int obj_mapped;
	int obj_solved;
	int obj_multiplied;
	int obj_written;
	int obj_sync_done;
	
	osync_bool busy;
	
	GHashTable *internalFormats;
	GHashTable *internalSchemas;
	/** converter_paths contains a hash of all OSyncFormatConverterPath objects **/
	GHashTable *converterPathes;
};

/**
 * @brief Set error for OSyncEngine.
 * 
 * Stack error message. If an error is already set, the error get stacked.
 *
 * @param engine Pointer of OSyncEngine 
 * @param error Pointer to OSyncEror struct to add
 */
void osync_engine_set_error(OSyncEngine *engine, OSyncError *error);

/** @brief Initialize format environment and "intenral formats" for the engine
 *
 * FIXME: Drop internal schema initilization once xmlformat plugin does this in the fomrat-init function.
 *
 * @param engine A pointer to the engine, which to initialize the formatenv.
 * @param error A pointer to a error struct
 * @returns TRUE on success, FALSE otherwise.
 *
 */
static osync_bool _osync_engine_initialize_formats(OSyncEngine *engine, OSyncError **error);

/*@}*/

#endif /* OPENSYNC_ENGINE_PRIVATE_H_ */
