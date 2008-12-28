/*
 * libopensync - A synchronization engine for the opensync framework
 * Copyright (C) 2004-2005  Armin Bauer <armin.bauer@opensync.org>
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
 
#ifndef OPENSYNC_ENGINE_H_
#define OPENSYNC_ENGINE_H_

/**
 * @defgroup OSyncEngine OpenSync Engine 
 * @ingroup OSyncPublic
 * @brief Interface to start and control a synchronization
 *
 * The OpenSync Engine provide the interfaces to start a synchronization and
 * to control various synchronization options. A synchronization process can
 * be started in a synchronous (blocking) and asynchronous way.
 * 
 * Callback interfaces allow to dynamically influence the synchronization process.
 * e.g. conflict callback, ...
 *
 * Other callbacks give frequent updates of the synchronization process, as well
 * as error message.
 *
 */

/*@{*/

/**
 * @brief Engine Commands
 **/

typedef enum {
	/** Initiate connect phase */
	OSYNC_ENGINE_COMMAND_CONNECT = 1,
	/** Finalize connect phase */
	OSYNC_ENGINE_COMMAND_CONNECT_DONE,
	/** Read/Get latest changes or all records, depending on synchronization type */
	OSYNC_ENGINE_COMMAND_READ,
	/** Write/Commit changes */	
	OSYNC_ENGINE_COMMAND_WRITE,
	/** Finalize this synchronization */
	OSYNC_ENGINE_COMMAND_SYNC_DONE, 
	/* Disconnect */
	OSYNC_ENGINE_COMMAND_DISCONNECT,
	/* Solve conflict(s) */
	OSYNC_ENGINE_COMMAND_SOLVE,
	/* Discover resources and capabilities */
	OSYNC_ENGINE_COMMAND_DISCOVER,
	/* Abort the currently running synchronization process */ 
	OSYNC_ENGINE_COMMAND_ABORT,
	/* Map all reported changes */
	OSYNC_ENGINE_COMMAND_MAP
} OSyncEngineCmd;


/**
 * @brief Engine Status
 **/
typedef enum {
	/** Uninitialized */
	OSYNC_ENGINE_STATE_UNINITIALIZED,
	/** Initialized */
	OSYNC_ENGINE_STATE_INITIALIZED,
	/** Waiting for synchronization request by a peer */
	OSYNC_ENGINE_STATE_WAITING,
	/** Connecting the peers */
	OSYNC_ENGINE_STATE_CONNECTING,
	/** Reading latest changes or all records, depending on synchronization type */
	OSYNC_ENGINE_STATE_READING,
	/** Writing changes to peers */
	OSYNC_ENGINE_STATE_WRITING,
	/** Disconnecting the peers */
	OSYNC_ENGINE_STATE_DISCONNECTING,
	/** Creating mapping between the different reported records */
	OSYNC_ENGINE_STATE_MAPPING
} OSyncEngineState;

/**
 * @brief Completed Engine Event
 **/
typedef enum {
	/** Initial connection phase of all peers are done */
	OSYNC_ENGINE_EVENT_CONNECTED = 1,
	/** Connection phase to all all peers is completed */
	OSYNC_ENGINE_EVENT_CONNECT_DONE,
	/** Error */
	OSYNC_ENGINE_EVENT_ERROR,
	/** Read latest changes or all records, depending on synchronization type */
	OSYNC_ENGINE_EVENT_READ,
	/** All changes got written */
	OSYNC_ENGINE_EVENT_WRITTEN,
	/** Synchronization process got finalized */
	OSYNC_ENGINE_EVENT_SYNC_DONE,
	/** All peers got disconnected */
	OSYNC_ENGINE_EVENT_DISCONNECTED,
	/** Synchronization process was successful */
	OSYNC_ENGINE_EVENT_SUCCESSFUL,
	/** All conflicts got solved */
	OSYNC_ENGINE_EVENT_END_CONFLICTS,
	/** Previous synchronization process was unclean */
	OSYNC_ENGINE_EVENT_PREV_UNCLEAN,
	/** All reported records got mapped */
	OSYNC_ENGINE_EVENT_MAPPED
} OSyncEngineEvent;

typedef enum {
	OSYNC_CLIENT_EVENT_CONNECTED = 1,
	OSYNC_CLIENT_EVENT_CONNECT_DONE,
	OSYNC_CLIENT_EVENT_ERROR,
	OSYNC_CLIENT_EVENT_READ,
	OSYNC_CLIENT_EVENT_WRITTEN,
	OSYNC_CLIENT_EVENT_SYNC_DONE,
	OSYNC_CLIENT_EVENT_DISCONNECTED,
	OSYNC_CLIENT_EVENT_DISCOVERED
} OSyncMemberEvent;

typedef enum {
	OSYNC_CHANGE_EVENT_READ = 1,
	OSYNC_CHANGE_EVENT_WRITTEN = 2,
	OSYNC_CHANGE_EVENT_ERROR = 3
} OSyncChangeEvent;

typedef enum {
	OSYNC_MAPPING_EVENT_SOLVED = 1,
	//OSYNC_MAPPING_EVENT_WRITTEN = 2,
	OSYNC_MAPPING_EVENT_ERROR = 3
} OSyncMappingEvent;


/**
 * @brief Struct for the member status callback
 */
typedef struct OSyncMemberUpdate {
	/** The type of the status update */
	OSyncMemberEvent type;
	char *objtype;
	/** The member for which the status update is */
	OSyncMember *member;
	/** If the status was a error, this error will be set */
	OSyncError *error;
} OSyncMemberUpdate;

/**
 * @brief Struct for the change status callback
 */
typedef struct OSyncChangeUpdate {
	/** The type of the status update */
	OSyncChangeEvent type;
	/** The change for which the status update is */
	OSyncChange *change;
	/** The id of the member which sent this change */
	OSyncMember *member;
	/** The id of the mapping to which this change belongs if any */
	int mapping_id;
	/** If the status was a error, this error will be set */
	OSyncError *error;
} OSyncChangeUpdate;

/**
 * @brief Struct for the mapping status callback
 */
typedef struct OSyncMappingUpdate {
	/** The type of the status update */
	OSyncMappingEvent type;
	/** If the mapping was already solved, this will have the id if the winning entry */
	long long int winner;
	/** The mapping for which the status update is */
	OSyncMapping *mapping;
	/** If the status was a error, this error will be set */
	OSyncError *error;
} OSyncMappingUpdate;

/**
 * @brief Struct for the engine status callback
 */
typedef struct OSyncEngineUpdate {
	/** The type of the status update */
	OSyncEngineEvent type;
	/** If the status was a error, this error will be set */
	OSyncError *error;
} OSyncEngineUpdate;

OSYNC_EXPORT OSyncEngine *osync_engine_new(OSyncGroup *group, OSyncError **error);
OSYNC_EXPORT OSyncEngine *osync_engine_ref(OSyncEngine *engine);
OSYNC_EXPORT void osync_engine_unref(OSyncEngine *engine);

OSYNC_EXPORT osync_bool osync_engine_initialize(OSyncEngine *engine, OSyncError **error);
OSYNC_EXPORT osync_bool osync_engine_finalize(OSyncEngine *engine, OSyncError **error);

OSYNC_EXPORT osync_bool osync_engine_synchronize(OSyncEngine *engine, OSyncError **error);
OSYNC_EXPORT osync_bool osync_engine_synchronize_and_block(OSyncEngine *engine, OSyncError **error);
OSYNC_EXPORT osync_bool osync_engine_wait_sync_end(OSyncEngine *engine, OSyncError **error);

OSYNC_EXPORT osync_bool osync_engine_discover(OSyncEngine *engine, OSyncMember *member, OSyncError **error);
OSYNC_EXPORT osync_bool osync_engine_discover_and_block(OSyncEngine *engine, OSyncMember *member, OSyncError **error);

OSYNC_EXPORT osync_bool osync_engine_abort(OSyncEngine *engine, OSyncError **error);


typedef void (* osync_conflict_cb) (OSyncEngine *, OSyncMappingEngine *, void *);
typedef void (* osync_status_change_cb) (OSyncChangeUpdate *, void *);
typedef void (* osync_status_mapping_cb) (OSyncMappingUpdate *, void *);
typedef void (* osync_multiply_cb) (OSyncEngine *, void *); 
typedef void (* osync_status_member_cb) (OSyncMemberUpdate *, void *);
typedef void (* osync_status_engine_cb) (OSyncEngineUpdate *, void *);

OSYNC_EXPORT void osync_engine_set_conflict_callback(OSyncEngine *engine, osync_conflict_cb callback, void *user_data);
OSYNC_EXPORT void osync_engine_set_multiply_callback(OSyncEngine *engine, osync_multiply_cb callback, void *user_data);
OSYNC_EXPORT void osync_engine_set_changestatus_callback(OSyncEngine *engine, osync_status_change_cb callback, void *user_data);
OSYNC_EXPORT void osync_engine_set_mappingstatus_callback(OSyncEngine *engine, osync_status_mapping_cb callback, void *user_data);
OSYNC_EXPORT void osync_engine_set_enginestatus_callback(OSyncEngine *engine, osync_status_engine_cb callback, void *user_data);
OSYNC_EXPORT void osync_engine_set_memberstatus_callback(OSyncEngine *engine, osync_status_member_cb callback, void *user_data);

OSYNC_EXPORT OSyncObjEngine *osync_engine_find_objengine(OSyncEngine *engine, const char *objtype);

OSYNC_EXPORT osync_bool osync_engine_mapping_solve(OSyncEngine *engine, OSyncMappingEngine *mapping_engine, OSyncChange *change, OSyncError **error);
OSYNC_EXPORT osync_bool osync_engine_mapping_duplicate(OSyncEngine *engine, OSyncMappingEngine *mapping_engine, OSyncError **error);
OSYNC_EXPORT osync_bool osync_engine_mapping_ignore_conflict(OSyncEngine *engine, OSyncMappingEngine *mapping_engine, OSyncError **error);
OSYNC_EXPORT osync_bool osync_engine_mapping_use_latest(OSyncEngine *engine, OSyncMappingEngine *mapping_engine, OSyncError **error);

/*@}*/

#endif /*OPENSYNC_ENGINE_H_*/
