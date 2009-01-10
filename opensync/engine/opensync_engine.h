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
 * @defgroup OSyncEngine OpenSync Engine Module
 * @ingroup OSyncPublic
 * @defgroup OSyncEngineAPI OpenSync Engine
 * @ingroup OSyncEngine
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
	OSYNC_ENGINE_COMMAND_MAP,
	/* Multiply all reported changes to all peers */
	OSYNC_ENGINE_COMMAND_MULTIPLY,
	/* Check if all conflicts got solved */
	OSYNC_ENGINE_COMMAND_END_CONFLICTS,
	/* Prepare write to peers */
	OSYNC_ENGINE_COMMAND_PREPARE_WRITE,
	/* Prepare mapping of changes */
	OSYNC_ENGINE_COMMAND_PREPARE_MAP
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
	OSYNC_ENGINE_STATE_MAPPING,
	/** Multiplying all reported changes to all peers */ 
	OSYNC_ENGINE_STATE_MULTIPLYING,
	/** Solving conflicts */
	OSYNC_ENGINE_STATE_SOLVING
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
	OSYNC_ENGINE_EVENT_MAPPED,
	/** Multiplying of all reported changes is completed */
	OSYNC_ENGINE_EVENT_MULTIPLIED,
	/** Engine completed with preparing a write */
	OSYNC_ENGINE_EVENT_PREPARED_WRITE,
	/** Engien completed with preparing for the mapping */
	OSYNC_ENGINE_EVENT_PREPARED_MAP
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

/** @brief This will create a new engine for the given group
 *
 * This will create a new engine for the given group
 *
 * @param group A pointer to the group, for which you want to create a new engine
 * @param error A pointer to a error struct
 * @returns Pointer to a newly allocated OSyncEngine on success, NULL otherwise
 *
 */
OSYNC_EXPORT OSyncEngine *osync_engine_new(OSyncGroup *group, OSyncError **error);
OSYNC_EXPORT OSyncEngine *osync_engine_ref(OSyncEngine *engine);
OSYNC_EXPORT void osync_engine_unref(OSyncEngine *engine);

OSYNC_EXPORT osync_bool osync_engine_initialize(OSyncEngine *engine, OSyncError **error);
OSYNC_EXPORT osync_bool osync_engine_finalize(OSyncEngine *engine, OSyncError **error);

/** @brief Starts to synchronize the given OSyncEngine
 *
 * This function synchronizes a given engine. The Engine has to be created
 * from a OSyncGroup before by using osync_engine_new(). This function will not block
 *
 * @param engine A pointer to the engine, which will be used to sync
 * @param error A pointer to a error struct
 * @returns TRUE on success, FALSE otherwise. Check the error on FALSE. Note that this just says if the sync has been started successfully, not if the sync itself was successful
 *
 */
OSYNC_EXPORT osync_bool osync_engine_synchronize(OSyncEngine *engine, OSyncError **error);

/** @brief This function will synchronize once and block until the sync has finished
 *
 * This can be used to sync a group and wait for the synchronization end. DO NOT USE
 * osync_engine_wait_sync_end for this as this might introduce a race condition.
 *
 * @param engine A pointer to the engine, which to sync and wait for the sync end
 * @param member A pointer to the member, which to discover
 * @param error A pointer to a error struct
 * @returns TRUE on success, FALSE otherwise.
 *
 */
OSYNC_EXPORT osync_bool osync_engine_synchronize_and_block(OSyncEngine *engine, OSyncError **error);


/** @brief This function will block until a synchronization has ended
 *
 * This can be used to wait until the synchronization has ended. Note that this function will always
 * block until 1 sync has ended. It can be used before the sync has started, to wait for one auto-sync
 * to end
 *
 * @param engine A pointer to the engine, for which to wait for the sync end
 * @param error Return location for the error if the sync was not successful
 * @returns TRUE on success, FALSE otherwise.
 */
OSYNC_EXPORT osync_bool osync_engine_wait_sync_end(OSyncEngine *engine, OSyncError **error);

/** @brief This function will discover the capabilities of a member
 *
 * This function discover a member of a given engine. The Engine has to be created
 * from a OSyncGroup before by using osync_engine_new(). This function will not block
 * The Engine MUST NOT be initialized by osync_engine_initilize(), but MUST finalized with
 * osync_engine_finalize().
 *
 * FIXME: Automatically finalize the engine after discovery of member is finished. This
 *        is needed by the frontend to allow easy use of non-blocking discovery.
 *
 * @param engine A pointer to the engine, which to discover the member and wait for the discover end
 * @param member A pointer to the member, which to discover
 * @param error A pointer to a error struct
 * @returns TRUE on success, FALSE otherwise.
 *
 */
OSYNC_EXPORT osync_bool osync_engine_discover(OSyncEngine *engine, OSyncMember *member, OSyncError **error);

/** @brief This function will discover the member and block until the discovery has finished
 *
 * This can be used to discover a member and wait for the discovery end.
 * The engine MUST NOT be initialized or finalized.
 *
 * @param engine A pointer to the engine, which to discover the member and wait for the discover end
 * @param member A pointer to the member, which to discover
 * @param error A pointer to a error struct
 * @returns TRUE on success, FALSE otherwise.
 *
 */
OSYNC_EXPORT osync_bool osync_engine_discover_and_block(OSyncEngine *engine, OSyncMember *member, OSyncError **error);


/** @brief Aborts running synchronization
 *
 * This is aborting the current synchronization while flushing the pending
 * commands in the engine command queue and pushing the abort command on this
 * queue. The abort command will send the disconnect command to the client/plugins.
 * This could be also used within a conflict handler function which aborts the
 * synchronization instead of resolving the conflicts.
 *
 * This will also turn in the engine into error condition.
 * osync_error_has_error() will return TRUE once the abort got requested.
 *
 * FIXME: Currently aborting of the current synchronization is not yet perfect! It
 *        will not preempt already running commands. For example the batch_commit
 *        will not be preempted and the engine will abort after the batch_commit is done.
 *
 * TODO: Review XMPM Benq patches for abort hander. Is sigaction really sane way
 *       to abort? It's very important that the plugins get called with the disconnect
 *       functions, since plugins/devices rely on clean termination of connections.
 *
 * TODO: Introduce plugin abort function for protocol specific abort implementations
 *       (SyncML?, OBEX-based?, ...?)
 *
 * @param engine A pointer to the engine with a running synchronization which gets aborted.
 * @param error A pointer to a error struct
 * @returns TRUE on success, FALSE otherwise.
 *
 */
OSYNC_EXPORT osync_bool osync_engine_abort(OSyncEngine *engine, OSyncError **error);

/** @brief Continue a halted/paused synchronization process. 
 *
 * @param engine A pointer to the engine with a running synchronization which got paused/halted. 
 * @param error A pointer to a error struct
 * @returns TRUE on success, FALSE otherwise.
 *
 */
OSYNC_EXPORT osync_bool osync_engine_continue(OSyncEngine *engine, OSyncError **error);

typedef void (* osync_conflict_cb) (OSyncEngine *, OSyncMappingEngine *, void *);
typedef void (* osync_status_change_cb) (OSyncChangeUpdate *, void *);
typedef void (* osync_status_mapping_cb) (OSyncMappingUpdate *, void *);
typedef void (* osync_multiply_cb) (OSyncEngine *, void *); 
typedef void (* osync_status_member_cb) (OSyncMemberUpdate *, void *);
typedef void (* osync_status_engine_cb) (OSyncEngineUpdate *, void *);

/** @brief This will set the conflict handler for the given engine
 *
 * The conflict handler will be called every time a conflict occurs
 *
 * @param engine A pointer to the engine, for which to set the callback
 * @param callback A pointer to a function which will receive the conflict
 * @param user_data Pointer to some data that will get passed to the callback function as the last argument
 *
 */
OSYNC_EXPORT void osync_engine_set_conflict_callback(OSyncEngine *engine, osync_conflict_cb callback, void *user_data);

/** @brief This will set the multiply handler for the given engine
 *
 * The multiply handler will be called after the engine multiplied all changes.
 * Intention is to summaries the ongoing synchronization process (e.g. What is going to change).
 * If callback is set, then the syncrhonization process is blocked until the callback returned.
 * Callback gets directly called before writing changes to the peers (and before preparing for
 * writing).
 *
 * It's possible to abort the synchronization with osync_engine_abort() within this callback.
 *
 * @param engine A pointer to the engine, for which to set the callback
 * @param callback A pointer to a function which will receive multiply summary
 * @param user_data Pointer to some data that will get passed to the callback function as the last argument
 *
 */
OSYNC_EXPORT void osync_engine_set_multiply_callback(OSyncEngine *engine, osync_multiply_cb callback, void *user_data);

/** @brief This will set the change status handler for the given engine
 *
 * The change status handler will be called every time a new change is received, written etc
 *
 * @param engine A pointer to the engine, for which to set the callback
 * @param function A pointer to a function which will receive the change status
 * @param user_data Pointer to some data that will get passed to the status function as the last argument
 *
 */
OSYNC_EXPORT void osync_engine_set_changestatus_callback(OSyncEngine *engine, osync_status_change_cb callback, void *user_data);

/** @brief This will set the mapping status handler for the given engine
 *
 * The mapping status handler will be called every time a mapping is updated
 *
 * @param engine A pointer to the engine, for which to set the callback
 * @param function A pointer to a function which will receive the mapping status
 * @param user_data Pointer to some data that will get passed to the status function as the last argument
 *
 */
OSYNC_EXPORT void osync_engine_set_mappingstatus_callback(OSyncEngine *engine, osync_status_mapping_cb callback, void *user_data);

/** @brief This will set the engine status handler for the given engine
 *
 * The engine status handler will be called every time the engine is updated (started, stopped etc)
 *
 * @param engine A pointer to the engine, for which to set the callback
 * @param function A pointer to a function which will receive the engine status
 * @param user_data Pointer to some data that will get passed to the status function as the last argument
 *
 */
OSYNC_EXPORT void osync_engine_set_enginestatus_callback(OSyncEngine *engine, osync_status_engine_cb callback, void *user_data);

/** @brief This will set the member status handler for the given engine
 *
 * The member status handler will be called every time a member is updated (connects, disconnects etc)
 *
 * @param engine A pointer to the engine, for which to set the callback
 * @param function A pointer to a function which will receive the member status
 * @param user_data Pointer to some data that will get passed to the status function as the last argument
 *
 */
OSYNC_EXPORT void osync_engine_set_memberstatus_callback(OSyncEngine *engine, osync_status_member_cb callback, void *user_data);

/** @brief Find the Object Engine for a certain Object Type. 
 *
 * @param engine A pointer to the engine
 * @param objtype The string of an Object Type to look for 
 * @returns Pointer of the found OSyncObjEngine, otherwise NULL
 *
 */
OSYNC_EXPORT OSyncObjEngine *osync_engine_find_objengine(OSyncEngine *engine, const char *objtype);

/** @brief Get the nth OSyncObjEngine of the OSyncEngine
 *
 * @param engine A pointer to the engine
 * @param nth The position of the OSyncObjEngine to request
 * @returns Pointer of the nth OSyncObjEngine
 *
 */
OSYNC_EXPORT OSyncObjEngine *osync_engine_nth_objengine(OSyncEngine *engine, unsigned int nth);

/** @brief Get the number of OSyncObjEngine-elements in OSyncEngine 
 *
 * @param engine A pointer to the engine
 * @returns Total number of OSyncObjEngine-elements 
 *
 */
OSYNC_EXPORT unsigned int osync_engine_num_objengines(OSyncEngine *engine);

OSYNC_EXPORT osync_bool osync_engine_mapping_solve(OSyncEngine *engine, OSyncMappingEngine *mapping_engine, OSyncChange *change, OSyncError **error);
OSYNC_EXPORT osync_bool osync_engine_mapping_duplicate(OSyncEngine *engine, OSyncMappingEngine *mapping_engine, OSyncError **error);
OSYNC_EXPORT osync_bool osync_engine_mapping_ignore_conflict(OSyncEngine *engine, OSyncMappingEngine *mapping_engine, OSyncError **error);
OSYNC_EXPORT osync_bool osync_engine_mapping_use_latest(OSyncEngine *engine, OSyncMappingEngine *mapping_engine, OSyncError **error);

/*! @brief Repairs engine from failed synchronization processes. 
 *
 * This needs to get called to repair every failed synchronization process,
 * when same engine should get used. (Without initalizing a new engine).
 * 
 * @param engine A pointer to an already initialized but with an error affected engine
 * @param error A pointer to an empty error struct
 * @returns TRUE if repair process was succesful, FALSE otherwise.
 * 
 */
OSYNC_EXPORT osync_bool osync_engine_repair(OSyncEngine *engine, OSyncError **error);

/*@}*/

#endif /*OPENSYNC_ENGINE_H_*/
