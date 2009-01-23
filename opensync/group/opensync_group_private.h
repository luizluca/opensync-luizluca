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

/**
 * @defgroup OSyncGroupPrivate OpenSync Group Module Private
 * @ingroup OSyncPrivate
 * @defgroup OSyncGroupPrivateAPI OpenSync Group Private
 * @ingroup OSyncGroupPrivate
 * @brief The private API of opensync
 *
 * This gives you an insight in the private API of opensync group.
 *
 */
/*@{*/

/** @brief Creates a new unique member if in this group
 *
 * @param group The group
 * @returns A new unique member id
 *
 */
static long long int osync_group_create_member_id(OSyncGroup *group);

/** @brief Loads all members of a group
 *
 * Loads all members of a group
 *
 * @param group The group
 * @param path The path from which to load the members
 * @param error Pointer to an error struct
 * @returns True if the members were loaded successfully, FALSE otherwise
 *
 */
static osync_bool osync_group_load_members(OSyncGroup *group, const char *path, OSyncError **error);

static void osync_group_build_list(gpointer key, gpointer value, gpointer user_data);

/** @brief Get list of supported object types of the group
 *
 * @param group The group
 * @returns List of supported object types
 *
 */
static GList *osync_group_get_supported_objtypes(OSyncGroup *group);
/*@}*/
