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

#ifndef OPENSYNC_ANCHOR_PRIVATE_H_
#define OPENSYNC_ANCHOR_PRIVATE_H_

/**
 * @defgroup OSyncHelperPrivate OpenSync Helper Module Private
 * @ingroup OSyncPrivate
 * @defgroup OSyncAnchorPrivateAPI OpenSync Anchor Private
 * @ingroup OSyncHelperPrivate
 * @brief Internal functions to deal with anchors
 */

/*@{*/

/**
 * @brief OSyncAnchor struct
 */
struct OSyncAnchor {
	/* Reference counting */
	int ref_count;
	/* Pointer to the OSyncDatabase */
	OSyncDB *db;
};

/**
 * @brief Create the anchor table in the specified database
 *
 * @param db Pointer to the database
 * @param error Pointer to an error struct
 * @returns TRUE if the table was created successfully, FALSE otherwise
 *
 */
static osync_bool osync_anchor_db_create(OSyncDB *db, OSyncError **error);

/**
 * @brief Create an anchor database
 *
 * @param filename the full path to the database file to create
 * @param error Pointer to an error struct
 * @returns a pointer to the new database
 *
 */
static OSyncDB *osync_anchor_db_new(const char *filename, OSyncError **error);

/**
 * @brief Close and free an anchor database handle
 *
 * @param db Pointer to the database
 *
 */
static void osync_anchor_db_free(OSyncDB *db);

/**
 * @brief Retrieves the value of an anchor
 *
 * @param db Pointer to the database
 * @param key the key of the anchor to look up
 * @returns the value of the anchor if it was found, otherwise NULL
 *
 */
static char *osync_anchor_db_retrieve(OSyncDB *db, const char *key);

/**
 * @brief Updates the value of an anchor
 *
 * @param db Pointer to the database
 * @param key the key of the anchor to look up
 * @param anchor the new value to set
 *
 */
static void osync_anchor_db_update(OSyncDB *db, const char *key, const char *anchor);

/*@}*/

#endif /* OPENSYNC_ANCHOR_PRIVATE_H_ */
