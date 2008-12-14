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

#ifndef _OPENSYNC_GROUP_ENV_H_
#define _OPENSYNC_GROUP_ENV_H_

/**
 * @defgroup OSyncPublic OpenSync Public API
 * @brief The public API of opensync
 * 
 * This gives you an insight in the public API of opensync.
 * 
 */

/**
 * @defgroup OSyncGroupEnvAPI OpenSync Environment
 * @ingroup OSyncPublic
 * @brief The public API of the opensync environment
 * 
 */
/*@{*/


/** @brief This will create a new opensync environment
 * 
 * The environment will hold all information about plugins, groups etc
 * 
 * @returns A pointer to a newly allocated environment. NULL on error.
 * 
 */
OSYNC_EXPORT OSyncGroupEnv *osync_group_env_new(OSyncError **error);

/** @brief Frees a osync environment
 * 
 * Frees a osync environment and all resources.
 * 
 * @param env Pointer to the environment to free
 * 
 */
OSYNC_EXPORT void osync_group_env_free(OSyncGroupEnv *env);


/** @brief Loads the plugins from a given directory
 * 
 * Loads all plugins from a directory into a osync environment.
 * The directory must exist prior to opening.
 * 
 * @param env Pointer to a OSyncGroupEnv environment
 * @param path The path where to look for groups
 * @param error Pointer to a error struct to return a error
 * @returns TRUE on success, FALSE otherwise
 * 
 */
OSYNC_EXPORT osync_bool osync_group_env_load_groups(OSyncGroupEnv *env, const char *p, OSyncError **error);

/** @brief Finds the group with the given name
 * 
 * Finds the group with the given name
 * 
 * @param env Pointer to a OSyncGroupEnv environment
 * @param name Name of the group to search
 * @returns Pointer to group. NULL if not found
 * 
 */
OSYNC_EXPORT OSyncGroup *osync_group_env_find_group(OSyncGroupEnv *env, const char *name);

/** @brief Adds the given group to the environment.
 * 
 * Adds the given group to the environment.
 * 
 * @param env Pointer to a OSyncGroupEnv environment
 * @param group The group to add. The group must have a name.
 * @param error Pointer to a error struct to return a error
 * @returns FALSE if group with the same name already exists. 
 * 
 */
OSYNC_EXPORT osync_bool osync_group_env_add_group(OSyncGroupEnv *env, OSyncGroup *group, OSyncError **error);

/** @brief Removes the given group from the enviroment
 * 
 * Removes the given group from the environment
 * 
 * @param env Pointer to a OSyncGroupEnv environment
 * @param group The group to add
 * 
 */
OSYNC_EXPORT void osync_group_env_remove_group(OSyncGroupEnv *env, OSyncGroup *group);

/** @brief Counts the groups in the environment
 * 
 * Returns the number of groups
 * 
 * @param env Pointer to a OSyncGroupEnv environment
 * @returns Number of groups
 * 
 */
OSYNC_EXPORT int osync_group_env_num_groups(OSyncGroupEnv *env);

/** @brief Returns the nth group
 * 
 * Returns the nth groups from the environment
 * 
 * @param env Pointer to a OSyncGroupEnv environment
 * @param nth Which group to return
 * @returns Pointer to the group
 * 
 */
OSYNC_EXPORT OSyncGroup *osync_group_env_nth_group(OSyncGroupEnv *env, int nth);

/*@}*/

#endif //_OPENSYNC_GROUP_ENV_H_
