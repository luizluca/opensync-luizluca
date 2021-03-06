/*
 * libopensync - A synchronization framework
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

#ifndef _OPENSYNC_OBJTYPE_SINK_H_
#define _OPENSYNC_OBJTYPE_SINK_H_

/**
 * @defgroup OSyncObjTypeSinkAPI OpenSync Object Type Sink
 * @ingroup OSyncPlugin
 * @brief Functions to register and manage object type sinks
 * 
 */
/*@{*/

/**
 * @brief Callback function to connect a sink
 * 
 * This function is optional and set through osync_objtype_sink_set_connect_func.
 * Use this function to set up a connection to your device, protocol, ...
 * It could be used to establish Object Type specific connection. If the connection establishment 
 * is an Object Type  neutral task (e.g. Bluetooth, USB, ...) and needs to be done only once,
 * then the Main Sink should be given a connect function.
 * Often this function is also used to determine if a slow-sync is necessary
 * 
 * @param sink Pointer to the Sink which corresponds to the functions
 * @param info PluginInfo pointer e.g. to get all formats
 * @param ctx Context to report success or failure
 * @param data Pointer to the data which is passed in osync_objtype_sink_set_userdata
 */
typedef void (* OSyncSinkConnectFn) (OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *data);

/**
 * @brief Callback function to disconnect a sink
 * 
 * This function is optional and set through osync_objtype_sink_set_disconnect_func.
 * Use this function to tear down an established connection
 * 
 * @param sink Pointer to the Sink which corresponds to the functions
 * @param info PluginInfo pointer e.g. to get all formats
 * @param ctx Context to report success or failure
 * @param data Pointer to the data which is passed in osync_objtype_sink_set_userdata
 */
typedef void (* OSyncSinkDisconnectFn) (OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *data);

/**
 * @brief Callback function to read all changes from a sink
 * 
 * This function is required and set through osync_objtype_sink_set_get_changes_func.
 * Use this function to read all changes from your device/plugin.
 * The Get Changes function gets called by the OpenSync Framework to request changes
 * since last synchronization or all entries of the Object Type specific resource.
 * The later case MUST be performed when a Slow Sync got requested by the OpenSync
 * Synchronization framework.On a Slow Sync the changetype (for all changes) is 
 * OSYNC_CHANGE_TYPE_ADDED. On a regular (fast sync) synchronization it's up to the
 * Get Changes function to determine the changetype. Every change has to be reported
 * with osync_context_report_change(). If an error occurs the function must reply on the
 * context with osync_context_report_error() and stop/leave this function ASAP. If the
 * protocol, application or device isn't able to report changed entries since last sync,
 * the OpenSync Helper Hashtable should be used to determine the Changetype of the entry.
 * 
 * @param sink Pointer to the Sink which corresponds to the functions
 * @param info PluginInfo pointer e.g. to get all formats
 * @param ctx Context to report success or failure and all changes
 * @param slow_sync TRUE if a slow sync is requested
 * @param data Pointer to the data which is passed in osync_objtype_sink_set_userdata
 */
typedef void (* OSyncSinkGetChangesFn) (OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, osync_bool slow_sync, void *data);

/**
 * @brief Callback function to commit a change to a sink
 * 
 * This function is required and set through osync_objtype_sink_set_commit_func.
 * It gets called with a single OSyncChange object, which MUST be committed
 * to the application or device. If the commit failed the context MUST be replied with
 * osync_context_report_error(). On success the context get replied with osync_context_success().
 * If a hashtable is already involved in the get_changes function, then the Commit function
 * should also update the hashtable for the entries which get committed.
 * 
 * @param sink Pointer to the Sink which corresponds to the functions
 * @param info PluginInfo pointer e.g. to get all formats
 * @param ctx Context to report success or failure
 * @param change Entry to commit
 * @param data Pointer to the data which is passed in osync_objtype_sink_set_userdata
 */
typedef void (* OSyncSinkCommitFn) (OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, OSyncChange *change, void *data);

/**
 * @brief Callback function to which got called after all changes got committed
 * 
 * This function is optional and set through osync_objtype_sink_set_comittedall_func.
 * It is called after all changes got committed to the sink even if an error appeared while
 * committing. If a function is required that is called after all changes to all members/peers
 * are committed please use OSyncSinkSyncDoneFn
 * 
 * @param sink Pointer to the Sink which corresponds to the functions
 * @param info PluginInfo pointer e.g. to get all formats
 * @param ctx Context to report success or failure
 * @param data Pointer to the data which is passed in osync_objtype_sink_set_userdata
 */
typedef void (* OSyncSinkCommittedAllFn) (OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *data);

/**
 * @brief Callback function to read the data of a change
 * 
 * The read sink function is used to support the "ignore" conflict resolution.
 * The read() function requires that the protocol/interface is able to query the native datastore for single entries.
 * This function is optional set through osync_objtype_sink_set_read_func.
 * 
 * @param sink Pointer to the Sink which corresponds to the functions
 * @param info PluginInfo pointer e.g. to get all formats
 * @param ctx Context to report success or failure and the change
 * @param change Change that should be read again
 * @param data Pointer to the data which is passed in osync_objtype_sink_set_userdata
 */
typedef void (* OSyncSinkReadFn) (OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, OSyncChange *change, void *data);

/**
 * @brief Callback function to which got called after sync is finished by all members/peers
 * 
 * This function could be used to store or clean up plugin specific data after a sync process.
 * It is optional and set through osync_objtype_sink_set_sync_done_func.
 * 
 * @param sink Pointer to the Sink which corresponds to the functions
 * @param info PluginInfo pointer e.g. to get all formats
 * @param ctx Context to report success or failure
 * @param data Pointer to the data which is passed in osync_objtype_sink_set_userdata
 */
typedef void (* OSyncSinkSyncDoneFn) (OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *data);

/**
 * @brief Callback function which got called after all members are connected
 * 
 * Called after all peers have connected.  At this point the slow sync status of
 * all peers is known. If your plugin must connect differently depending on the
 * slow sync status of the group (e.g. syncml) then you may perform that connection here.
 * This function is optional and set through osync_objtype_sink_set_connect_done_func.
 * 
 * @param sink Pointer to the Sink which corresponds to the functions
 * @param info PluginInfo pointer e.g. to get all formats
 * @param ctx Context to report success or failure
 * @param slow_sync TRUE if a slow sync is requested
 * @param data Pointer to the data which is passed in osync_objtype_sink_set_userdata
 */
typedef void (* OSyncSinkConnectDoneFn) (OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, osync_bool slow_sync, void *data);

/** @brief Creates a new main sink
 *
 * Main sink is objtype neutral and should be used for object type
 * neutral actions. Actions like connecting/disconnecting which could
 * be a object type neutral - e.g. connecting to a device via bluetooth.
 * Object type specific example would be a connection to different 
 * object type specific databases/resources.
 *
 * The main sink is not limited to the connect and disconnect functions however. 
 * 
 * @param error Pointer to an error struct
 * @returns the newly created main sink
 */
OSYNC_EXPORT OSyncObjTypeSink *osync_objtype_main_sink_new(OSyncError **error);

/** @brief Creates a new sink for an object type
 *
 * @param objtype The name of the object type for the sink
 * @param error Pointer to an error struct
 * @returns the newly created objtype specific sink
 */
OSYNC_EXPORT OSyncObjTypeSink *osync_objtype_sink_new(const char *objtype, OSyncError **error);

/** @brief Increase the reference count on a sink
 * 
 * @param sink Pointer to the sink
 * 
 */
OSYNC_EXPORT OSyncObjTypeSink *osync_objtype_sink_ref(OSyncObjTypeSink *sink);

/** @brief Decrease the reference count on a sink
 * 
 * @param sink Pointer to the sink
 * 
 */
OSYNC_EXPORT void osync_objtype_sink_unref(OSyncObjTypeSink *sink);

/** @brief Request a state database for this Sink 
 *
 * If for this sink an anchor/state is required, this needs to be requested by this
 * function. If anchor/state gets enabled/requested inside the plugin, the framework
 * will take care about preparing the anchor/state.
 * The created anchor/state can be accessed
 * by using the function osync_objtype_sink_get_state_db()
 *
 * By default no anchor is requested/enabled.
 *
 * @param sink Pointer to the sink
 * @param enable Flag to enable, disable state database.
 * 
 */
OSYNC_EXPORT void osync_objtype_sink_enable_state_db(OSyncObjTypeSink *sink, osync_bool enable);

/** @brief Get the pointer to the sink OSyncSinkStateDB
 *
 * This state database is sink specific and can store persistent, sink specific states.
 * Originally designed to detect if a certain value changed since last
 * synchronization on the peer. E.g. to decided if a slow-sync is requried
 * or not.
 * 
 * @param sink Pointer to the sink
 * @returns Pointer to the requested OSyncSinkStateDB,
 *          or NULL if no state datbase is requested
 * 
 */
OSYNC_EXPORT OSyncSinkStateDB *osync_objtype_sink_get_state_db(OSyncObjTypeSink *sink);

/** @brief Request a hashtable for this Sink 
 *
 * If for this sink a hashtable is required, this needs to be requested by this
 * function. If hashtable gets enabled/requested inside the plugin, the framework
 * will take care about preparing the hashtable. The created hashtable can be accessed
 * by using the function osync_objtype_sink_get_hashtable()
 *
 * By default no hashtable is requested/enabled.
 *
 * @param sink Pointer to the sink
 * @param enable Flag to enable, disbable hashtable.
 * 
 */
OSYNC_EXPORT void osync_objtype_sink_enable_hashtable(OSyncObjTypeSink *sink, osync_bool enable);

/** @brief Get the pointer to the sink OSyncHashTable
 *
 * This Hashtable is sink specific and can store persistent hash values of changes.
 * Designed to help to determine the change type of an entry.
 * 
 * @param sink Pointer to the sink
 * @returns the name of the object type of the specified sink
 * 
 */
OSYNC_EXPORT OSyncHashTable *osync_objtype_sink_get_hashtable(OSyncObjTypeSink *sink);


/** @brief Return the name of the object type of a sink
 * 
 * @param sink Pointer to the sink
 * @returns the name of the object type of the specified sink
 * 
 */
OSYNC_EXPORT const char *osync_objtype_sink_get_name(OSyncObjTypeSink *sink);


/** @brief Set the object type of a sink
 * 
 * @param sink Pointer to the sink
 * @param name the name of the object type to set
 * 
 */
OSYNC_EXPORT void osync_objtype_sink_set_name(OSyncObjTypeSink *sink, const char *name);


/** @brief Return the preferred format for the conversion 
 * 
 * @param sink Pointer to the sink
 * @returns the name of the preferred format
 * 
 */
OSYNC_EXPORT const char *osync_objtype_sink_get_preferred_format(OSyncObjTypeSink *sink);

/** @brief Set the preferred format for the conversion
 * 
 * @param sink Pointer to the sink
 * @param preferred_format the name of the preferred format to set
 * 
 */
OSYNC_EXPORT void osync_objtype_sink_set_preferred_format(OSyncObjTypeSink *sink, const char *preferred_format);

/**
 * @brief Returns a OSyncList that contains the OSyncObjFormatSinks of this objtype sink
 * 
 * Please be aware that the returned list has to be freed with 
 * osync_list_free. If it isn't freed there will be a memory leak.
 * 
 * @param sink A pointer to a OSyncObjTypeSink
 * @return A shallow copy of the internal list of OSyncObjFormatSinks
 */
OSYNC_EXPORT OSyncList *osync_objtype_sink_get_objformat_sinks(OSyncObjTypeSink *sink);

/** @brief Finds the objformat sink for the corresponding objformat 
 * 
 * @param sink Pointer to the sink
 * @param objformat the objformat to look for the corresponding objformat sink
 * @returns Pointer to the corresponding objformat sink if found, NULL otherwise 
 * 
 */
OSYNC_EXPORT OSyncObjFormatSink *osync_objtype_sink_find_objformat_sink(OSyncObjTypeSink *sink, OSyncObjFormat *objformat);

/** @brief Adds an object format sink to the sink
 * 
 * @param sink Pointer to the sink
 * @param objformatsink The object format sink to add 
 * 
 */
OSYNC_EXPORT void osync_objtype_sink_add_objformat_sink(OSyncObjTypeSink *sink, OSyncObjFormatSink *objformatsink);

/** @brief Removes an object format from the sink
 * 
 * @param sink Pointer to the sink
 * @param objformatsink the object format sink to remove
 * 
 */
OSYNC_EXPORT void osync_objtype_sink_remove_objformat_sink(OSyncObjTypeSink *sink, OSyncObjFormatSink *objformatsink);

/** @brief Checks if a sink is enabled
 * 
 * @param sink Pointer to the sink
 * @returns TRUE if the sink is enabled, FALSE otherwise
 * 
 */
OSYNC_EXPORT osync_bool osync_objtype_sink_is_enabled(OSyncObjTypeSink *sink);

/** @brief Sets the enabled/disabled state of a sink
 * 
 * @param sink Pointer to the sink
 * @param enabled TRUE if the sink is enabled, FALSE otherwise
 * 
 */
OSYNC_EXPORT void osync_objtype_sink_set_enabled(OSyncObjTypeSink *sink, osync_bool enabled);


/** @brief Checks if a sink is available
 * 
 * @param sink Pointer to the sink
 * @returns TRUE if the sink is available, FALSE otherwise
 * 
 */
OSYNC_EXPORT osync_bool osync_objtype_sink_is_available(OSyncObjTypeSink *sink);

/** @brief Sets the available state of a sink
 * 
 * @param sink Pointer to the sink
 * @param available TRUE if the sink is available, FALSE otherwise
 * 
 */
OSYNC_EXPORT void osync_objtype_sink_set_available(OSyncObjTypeSink *sink, osync_bool available);


/** @brief Checks if sink is allowed to write (commit)
 *
 * If the sink is not allowed to write, then no changes will be commited to
 * the sink.
 *
 * @param sink Pointer to the sink
 * @returns TRUE if the sink is allowed to write (commit), FALSE otherwise
 */
OSYNC_EXPORT osync_bool osync_objtype_sink_get_write(OSyncObjTypeSink *sink);

/** @brief Sets the write status of the sink (commit)
 *
 * See osync_objtype_sink_get_write()
 *
 * @param sink Pointer to sink
 * @param write TRUE if the sink is allowed to write changes (commit), FALSE otherwise
 *
 */
OSYNC_EXPORT void osync_objtype_sink_set_write(OSyncObjTypeSink *sink, osync_bool write);


/** @brief Checks if sink is allowed to get latest changes 
 *
 * @param sink Pointer to the sink
 * @returns TRUE if the sink is allowed to get latest changed entries, FALSE otherwise
 *
 */
OSYNC_EXPORT osync_bool osync_objtype_sink_get_getchanges(OSyncObjTypeSink *sink);

/** @brief Sets the get latest changes status of the sink (get_change)
 *
 * See osync_objtype_sink_get_getchanges()
 *
 * @param sink Pointer to sink
 * @param getchanges Set TRUE if the sink is allowed to get latest changes, FALSE otherwise
 *
 */
OSYNC_EXPORT void osync_objtype_sink_set_getchanges(OSyncObjTypeSink *sink, osync_bool getchanges);


/** @brief Checks if sink is allowed to read single entries
 *
 * "Read" means to request a single entry and does not mean to get the
 * latest changes since last sink. See osync_objtype_sink_get_getchanges().
 * The read function explicitly means to read a single entry without triggering
 * a full sync. This is used for example to check if a conflict between entries
 * could be ignored. Ignoring conflicts is only possible if the sink is allowed to
 * read this conflicting entries on the next sync without triggering a SlowSync.
 *
 * @param sink Pointer to the sink
 * @returns TRUE if the sink is allowed to read single entries, FALSE otherwise
 *
 */
OSYNC_EXPORT osync_bool osync_objtype_sink_get_read(OSyncObjTypeSink *sink);

/** @brief Sets the (single) read status of a sink 
 *
 * See osync_objtype_sink_get_read()
 *
 * @param sink Pointer to the sink
 * @param read TRUE if the sink is able to read (single entries), FALSE otherwise
 *
 */
OSYNC_EXPORT void osync_objtype_sink_set_read(OSyncObjTypeSink *sink, osync_bool read);

/** @brief Queries a sink for the changed objects since the last sync
 * 
 * Calls the get_changes function on a sink
 * 
 * @param sink Pointer to the sink
 * @param info Pointer to the plugin info object
 * @param slow_sync Bool to request a Slow Sync if set TRUE
 * @param ctx The sync context
 * 
 */
OSYNC_EXPORT void osync_objtype_sink_get_changes(OSyncObjTypeSink *sink, OSyncPluginInfo *info, osync_bool slow_sync, OSyncContext *ctx);

/** @brief Reads a single object by its uid
 * 
 * Calls the read_change function on the sink
 * 
 * @param sink Pointer to the sink
 * @param info Pointer to the plugin info object
 * @param change The change to read. The change must have the uid set
 * @param ctx The sync context
 * 
 */
OSYNC_EXPORT void osync_objtype_sink_read_change(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncChange *change, OSyncContext *ctx);

/** @brief Connects a sink to its device
 * 
 * Calls the connect function on a sink
 * 
 * @param sink Pointer to the sink
 * @param info Pointer to the plugin info object
 * @param ctx The sync context
 * 
 */
OSYNC_EXPORT void osync_objtype_sink_connect(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx);

/** @brief Disconnects a sink from its device
 * 
 * Calls the disconnect function on a sink
 * 
 * @param sink Pointer to the sink
 * @param info Pointer to the plugin info object
 * @param ctx The sync context
 * 
 */
OSYNC_EXPORT void osync_objtype_sink_disconnect(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx);

/** @brief Tells the sink that the sync was successfully completed
 * 
 * Calls the sync_done function on a sink
 * 
 * @param sink Pointer to the sink
 * @param info Pointer to the plugin info object
 * @param ctx The sync context
 * 
 */
OSYNC_EXPORT void osync_objtype_sink_sync_done(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx);

/** @brief Tells the sink that the connection was successfully completed
 * 
 * Calls the connect_done function on a sink
 * 
 * @param sink Pointer to the sink
 * @param info Pointer to the plugin info object
 * @param ctx The sync context
 * 
 */
OSYNC_EXPORT void osync_objtype_sink_connect_done(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx);

/** @brief Commits a change to the device
 * 
 * Calls the commit_change function on a sink
 * 
 * @param sink Pointer to the sink
 * @param info Pointer to the plugin info object
 * @param change The change to write
 * @param ctx The sync context
 * 
 */
OSYNC_EXPORT void osync_objtype_sink_commit_change(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncChange *change, OSyncContext *ctx);

/** @brief Tells the sink that all changes have been committed
 * 
 * Calls the committed_all function on a sink.
 * 
 * @param sink Pointer to the sink
 * @param info Pointer to the plugin info object
 * @param ctx The sync context
 * 
 */
OSYNC_EXPORT void osync_objtype_sink_committed_all(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx);


/** @brief Sets the connect timeout in seconds for the OSyncObjTypeSink 
 * 
 * @param sink Pointer to the sink
 * @param timeout The timeout in seconds 
 * 
 */
OSYNC_EXPORT void osync_objtype_sink_set_connect_timeout(OSyncObjTypeSink *sink, unsigned int timeout);

/** @brief Sets the disconnect timeout in seconds for the OSyncObjTypeSink 
 * 
 * @param sink Pointer to the sink
 * @param timeout The timeout in seconds 
 * 
 */
OSYNC_EXPORT void osync_objtype_sink_set_disconnect_timeout(OSyncObjTypeSink *sink, unsigned int timeout);

/** @brief Sets the get_changes timeout in seconds for the OSyncObjTypeSink 
 * 
 * @param sink Pointer to the sink
 * @param timeout The timeout in seconds 
 * 
 */
OSYNC_EXPORT void osync_objtype_sink_set_getchanges_timeout(OSyncObjTypeSink *sink, unsigned int timeout);

/** @brief Sets the commit timeout in seconds for the OSyncObjTypeSink 
 * 
 * @param sink Pointer to the sink
 * @param timeout The timeout in seconds 
 * 
 */
OSYNC_EXPORT void osync_objtype_sink_set_commit_timeout(OSyncObjTypeSink *sink, unsigned int timeout);

/** @brief Sets the committedall timeout in seconds for the OSyncObjTypeSink 
 * 
 * @param sink Pointer to the sink
 * @param timeout The timeout in seconds 
 * 
 */
OSYNC_EXPORT void osync_objtype_sink_set_committedall_timeout(OSyncObjTypeSink *sink, unsigned int timeout);

/** @brief Sets the syncdone timeout in seconds for the OSyncObjTypeSink 
 * 
 * @param sink Pointer to the sink
 * @param timeout The timeout in seconds 
 * 
 */
OSYNC_EXPORT void osync_objtype_sink_set_syncdone_timeout(OSyncObjTypeSink *sink, unsigned int timeout);

/** @brief Sets the read timeout in seconds for the OSyncObjTypeSink 
 * 
 * @param sink Pointer to the sink
 * @param timeout The timeout in seconds 
 * 
 */
OSYNC_EXPORT void osync_objtype_sink_set_read_timeout(OSyncObjTypeSink *sink, unsigned int timeout);

/*! @brief Open the state database for a specific Sink if requested 
 * 
 * Load (i.e. connects) to the Anchor. If no Anchor is requested for this sink
 * this functions just returns TRUE.
 *
 * @param sink Pointer to the sink
 * @param info Pointer to the plugin info object
 * @param error Pointer to error struct, get set on any error
 * @returns TRUE on success, FALSE on any error
 * 
 */
OSYNC_EXPORT osync_bool osync_objtype_sink_open_state_db(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncError **error);

/*! @brief Load the Hashtable for a specific Sink if requested 
 * 
 * Load (i.e. connects) to the Hashtable. If no Hashtable is requested for this sink
 * this functions just returns TRUE.
 *
 * @param sink Pointer to the sink
 * @param info Pointer to the plugin info object
 * @param error Pointer to error struct, get set on any error
 * @returns TRUE on success, FALSE on any error
 * 
 */
OSYNC_EXPORT osync_bool osync_objtype_sink_load_hashtable(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncError **error);

/*! @brief Save the Hashtable for a specific Sink if requested 
 * 
 * Write hashes to the hashtable database. If no Hashtable is requested for this sink
 * this functions just returns TRUE.
 *
 * @param sink Pointer to the sink
 * @param error Pointer to error struct, get set on any error
 * @returns TRUE on success, FALSE on any error
 * 
 */
OSYNC_EXPORT osync_bool osync_objtype_sink_save_hashtable(OSyncObjTypeSink *sink, OSyncError **error);


/** @brief Gets the user data from a sink
 * 
 * Gets the user data from a sink, as previously set by osync_objtype_sink_set_functions()
 *
 * @param sink Pointer to the sink
 * @returns the sink-specific user data
 * 
 */
OSYNC_EXPORT void *osync_objtype_sink_get_userdata(OSyncObjTypeSink *sink);

OSYNC_EXPORT void osync_objtype_sink_set_userdata(OSyncObjTypeSink *sink, void *userdata);

OSYNC_EXPORT void osync_objtype_sink_set_connect_func(OSyncObjTypeSink *sink, OSyncSinkConnectFn connect_func);

OSYNC_EXPORT void osync_objtype_sink_set_get_changes_func(OSyncObjTypeSink *sink, OSyncSinkGetChangesFn get_changes_func);

OSYNC_EXPORT void osync_objtype_sink_set_commit_func(OSyncObjTypeSink *sink, OSyncSinkCommitFn commit_func);

OSYNC_EXPORT void osync_objtype_sink_set_committed_all_func(OSyncObjTypeSink *sink, OSyncSinkCommittedAllFn committed_all_func);

OSYNC_EXPORT void osync_objtype_sink_set_read_func(OSyncObjTypeSink *sink, OSyncSinkReadFn read_func);

OSYNC_EXPORT void osync_objtype_sink_set_sync_done_func(OSyncObjTypeSink *sink, OSyncSinkSyncDoneFn sync_done_func);

OSYNC_EXPORT void osync_objtype_sink_set_connect_done_func(OSyncObjTypeSink *sink, OSyncSinkConnectDoneFn connect_done_func);

OSYNC_EXPORT void osync_objtype_sink_set_disconnect_func(OSyncObjTypeSink *sink, OSyncSinkDisconnectFn disconnect_func);

/*@}*/

#endif /* _OPENSYNC_OBJTYPE_SINK_H_ */

