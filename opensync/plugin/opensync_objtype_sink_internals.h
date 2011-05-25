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

#ifndef _OPENSYNC_OBJTYPE_SINK_INTERNALS_H_
#define _OPENSYNC_OBJTYPE_SINK_INTERNALS_H_

#include "opensync_objtype_sink.h"

/**
 * @defgroup OSyncObjTypeSinkInternalAPI OpenSync Object Type Sink Internals
 * @ingroup OSyncPluginPrivate
 * @brief Internal functions for managing object type sinks
 * 
 */

/*@{*/

typedef struct OSyncObjTypeSinkFunctions {
	OSyncSinkConnectFn connect;
	OSyncSinkDisconnectFn disconnect;
	OSyncSinkGetChangesFn get_changes;
	OSyncSinkCommitFn commit;
	OSyncSinkCommittedAllFn committed_all;
	OSyncSinkReadFn read;
	OSyncSinkSyncDoneFn sync_done;
	OSyncSinkConnectDoneFn connect_done;
} OSyncObjTypeSinkFunctions;

/** @brief Check if sink has a state database request. 
 *
 * @param sink Pointer to the sink
 * @returns TRUE if the sink has a state database request, FALSE otherwise
 */
osync_bool osync_objtype_sink_has_state_db(OSyncObjTypeSink *sink);

/** @brief Set the OSyncSinkStateDB for this sink
 *
 * This state database is sink specific and can store persistent, sink specific states.
 * Originally designed to detect if a certain value changed since last
 * synchronization on the peer. E.g. to decided if a slow-sync is requried
 * or not.
 * 
 * @param sink Pointer to the sink
 * @param state_db Pointer to the state database object
 * 
 */
void osync_objtype_sink_set_state_db(
		OSyncObjTypeSink *sink,
		OSyncSinkStateDB *state_db);

/** @brief Check if sink has a hashtable request. 
 *
 * @param sink Pointer to the sink
 * @returns TRUE if the sink has a hashtable request, FALSE otherwise
 */
osync_bool osync_objtype_sink_has_hashtable(OSyncObjTypeSink *sink);

/** @brief Set the OSyncHashTable for this sink
 *
 * This Hashtable is sink specific and can store persistent hash values of changes.
 * 
 * @param sink Pointer to the sink
 * @param hashtable Pointer to the Hashtable object
 * 
 */
void osync_objtype_sink_set_hashtable(OSyncObjTypeSink *sink, OSyncHashTable *hashtable);

/** @brief Checks if sink has a read single entries function (read)
 *
 * @param sink Pointer to the sink
 * @returns TRUE if the sink has a read single entries function (read), FALSE otherwise
 */
osync_bool osync_objtype_sink_get_function_read(OSyncObjTypeSink *sink);

/** @brief Sets the status of the read sink function
 *
 * @param sink Pointer to sink
 * @param read TRUE if the sink has a read function, FALSE otherwise
 */
void osync_objtype_sink_set_function_read(OSyncObjTypeSink *sink, osync_bool read);


/** @brief Checks if sink has a get latest changes function (get_changes)
 *
 * @param sink Pointer to the sink
 * @returns TRUE if the sink has a get latest changes function (get_changes), FALSE otherwise
 */
osync_bool osync_objtype_sink_get_function_getchanges(OSyncObjTypeSink *sink);

/** @brief Sets the status of the get_changes sink function
 *
 * @param sink Pointer to sink
 * @param getchanges TRUE if the sink has a get_changes function, FALSE otherwise
 */
void osync_objtype_sink_set_function_getchanges(OSyncObjTypeSink *sink, osync_bool getchanges);

/** @brief Get the current or default connect timeout in seconds 
 * 
 * @param sink Pointer to the sink
 * @return The timeout in seconds 
 * 
 */
unsigned int osync_objtype_sink_get_connect_timeout_or_default(OSyncObjTypeSink *sink);

/** @brief Get the current connect timeout in seconds 
 * 
 * @param sink Pointer to the sink
 * @return The timeout in seconds 
 * 
 */
unsigned int osync_objtype_sink_get_connect_timeout(OSyncObjTypeSink *sink);


/** @brief Get the current or default disconnect timeout in seconds 
 * 
 * @param sink Pointer to the sink
 * @return The timeout in seconds 
 * 
 */
unsigned int osync_objtype_sink_get_disconnect_timeout_or_default(OSyncObjTypeSink *sink);

/** @brief Get the current disconnect timeout in seconds 
 * 
 * @param sink Pointer to the sink
 * @return The timeout in seconds 
 * 
 */
unsigned int osync_objtype_sink_get_disconnect_timeout(OSyncObjTypeSink *sink);


/** @brief Get the current or default getchanges timeout in seconds 
 * 
 * @param sink Pointer to the sink
 * @return The timeout in seconds 
 * 
 */
unsigned int osync_objtype_sink_get_getchanges_timeout_or_default(OSyncObjTypeSink *sink);

/** @brief Get the current getchanges timeout in seconds 
 * 
 * @param sink Pointer to the sink
 * @return The timeout in seconds 
 * 
 */
unsigned int osync_objtype_sink_get_getchanges_timeout(OSyncObjTypeSink *sink);


/** @brief Get the current or default commit timeout in seconds 
 * 
 * @param sink Pointer to the sink
 * @return The timeout in seconds 
 * 
 */
unsigned int osync_objtype_sink_get_commit_timeout_or_default(OSyncObjTypeSink *sink);

/** @brief Get the current commit timeout in seconds 
 * 
 * @param sink Pointer to the sink
 * @return The timeout in seconds 
 * 
 */
unsigned int osync_objtype_sink_get_commit_timeout(OSyncObjTypeSink *sink);

/** @brief Get the current or default committedall timeout in seconds 
 * 
 * @param sink Pointer to the sink
 * @return The timeout in seconds 
 * 
 */
unsigned int osync_objtype_sink_get_committedall_timeout_or_default(OSyncObjTypeSink *sink);

/** @brief Get the current committedall timeout in seconds 
 * 
 * @param sink Pointer to the sink
 * @return The timeout in seconds 
 * 
 */
unsigned int osync_objtype_sink_get_committedall_timeout(OSyncObjTypeSink *sink);


/** @brief Get the current or default syncdone timeout in seconds 
 * 
 * @param sink Pointer to the sink
 * @return The timeout in seconds 
 * 
 */
unsigned int osync_objtype_sink_get_syncdone_timeout_or_default(OSyncObjTypeSink *sink);

/** @brief Get the current syncdone timeout in seconds 
 * 
 * @param sink Pointer to the sink
 * @return The timeout in seconds 
 * 
 */
unsigned int osync_objtype_sink_get_syncdone_timeout(OSyncObjTypeSink *sink);

/** @brief Get the current or default connectdone timeout in seconds 
 * 
 * @param sink Pointer to the sink
 * @return The timeout in seconds 
 * 
 */
unsigned int osync_objtype_sink_get_connectdone_timeout_or_default(OSyncObjTypeSink *sink);

/** @brief Get the current connectdone timeout in seconds 
 * 
 * @param sink Pointer to the sink
 * @return The timeout in seconds 
 * 
 */
unsigned int osync_objtype_sink_get_connectdone_timeout(OSyncObjTypeSink *sink);

/** @brief Get the current or default read timeout in seconds 
 * 
 * @param sink Pointer to the sink
 * @return The timeout in seconds 
 * 
 */
unsigned int osync_objtype_sink_get_read_timeout_or_default(OSyncObjTypeSink *sink);

/** @brief Get the current read timeout in seconds 
 * 
 * @param sink Pointer to the sink
 * @return The timeout in seconds 
 * 
 */
unsigned int osync_objtype_sink_get_read_timeout(OSyncObjTypeSink *sink);

/** @brief Checks if slow-sync has been requested
 * 
 * When slow-sync is requested, OpenSync synchronizes all entries rather than
 * just the changes.
 *
 * If you are using hashtables, you should call this function in your sink's
 * get_changes() function and if slow-sync has been requested, call
 * osync_hashtable_slowsync() on your hashtable. If you don't do this, OpenSync
 * will assume that all entries should be deleted, which is usually not what 
 * the user wants.
 *
 * @param sink Pointer to the sink
 * @returns TRUE if slow-sync has been requested, FALSE for normal sync
 * 
 */
osync_bool osync_objtype_sink_get_slowsync(OSyncObjTypeSink *sink);

/** @brief Sets the slow-sync state of a sink
 * 
 * When slow-sync is requested, OpenSync synchronizes all entries rather than
 * just the changes.
 *
 * Slow-sync should be requested if you know that your device's memory has
 * been erased. If it is appropriate for your device, you can use OpenSync's 
 * anchor system to determine if you should request slow-sync.
 *
 * @param sink Pointer to the sink
 * @param slowsync TRUE to request slow-sync, FALSE for normal sync
 * 
 */
void osync_objtype_sink_set_slowsync(OSyncObjTypeSink *sink, osync_bool slowsync);

/** @brief Returns the number of object formats in the sink
 * 
 * @param sink Pointer to the sink
 * @returns the number of object formats in the sink
 * 
 */
OSYNC_TEST_EXPORT unsigned int osync_objtype_sink_num_objformat_sinks(OSyncObjTypeSink *sink);

/** @brief Returns the nth object format in the sink
 * 
 * @param sink Pointer to the sink
 * @param nth the index of the object format to return
 * @returns the name of the object format at the specified index
 * 
 */
OSYNC_TEST_EXPORT OSyncObjFormatSink *osync_objtype_sink_nth_objformat_sink(OSyncObjTypeSink *sink, unsigned int nth);

/*@}*/

#endif /* _OPENSYNC_SINK_INTERNALS_H_*/

