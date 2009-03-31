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

#ifndef OPENSYNC_SINK_STATE_DB_INTERNALS_H_
#define OPENSYNC_SINK_STATE_DB_INTERNALS_H_

/**
 * @defgroup OSyncSinkStateDatabaseInternalAPI OpenSync Sink State Database Internals
 * @ingroup OSyncHelperPrivate
 * @brief Internal functions to deal with anchors and other sink related state informations
 */

/**
 * @brief Create an sink state database
 *
 * @param filename the full path to the database file to create
 * @param objtype Object Type to associate this anchor or state, NULL for main-sink. 
 * @param error Pointer to an error struct
 * @returns a pointer to the new database, NULL on error
 *
 */
OSyncSinkStateDB *osync_sink_state_db_new(
			const char *filename,
			const char *objtype,
			OSyncError **error);

/**
 * @brief Increase the reference count on a database 
 *
 * @param sinkStateDB Pointer to the database
 *
 */
OSyncSinkStateDB *osync_sink_state_db_ref(OSyncSinkStateDB *sinkStateDB);

/**
 * @brief Decrease the reference count on a database 
 *
 * @param sinkStateDB Pointer to the database
 *
 */
void osync_sink_state_db_unref(OSyncSinkStateDB *sinkStateDB);

/*@}*/

#endif /* OPENSYNC_SINK_STATE_DB_INTERNALS_H_ */
