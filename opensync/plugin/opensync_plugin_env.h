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

#ifndef _OPENSYNC_PLUGIN_ENV_H_
#define _OPENSYNC_PLUGIN_ENV_H_

/**
 * @defgroup OSyncPublic OpenSync Public API
 * @brief The public API of opensync
 * 
 * This gives you an insight in the public API of opensync.
 * 
 */

/**
 * @defgroup OSyncPluginEnvAPI OpenSync Plugin Environment
 * @ingroup OSyncPlugin
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
OSYNC_EXPORT OSyncPluginEnv *osync_plugin_env_new(OSyncError **error);

/** @brief Increases the reference counton an opensync plugin environment
 * 
 * The reference count on the OSyncPluginEnv is incremented. When the
 * reference is no longer needed it should be removed with 
 * osync_plugin_env_unref
 * 
 * @returns The environment passed in
 * 
 */
OSYNC_EXPORT OSyncPluginEnv *osync_plugin_env_ref(OSyncPluginEnv *env);

/** @brief Removes a reference to an OSyncPluginEnv
 * 
 * Decrements the reference count on an osync plugin environment.  If
 * the reference count reaches zero the environment is freed and all
 * resources are unreferenced
 * 
 * @param env Pointer to the environment to unreference
 * 
 */
OSYNC_EXPORT void osync_plugin_env_unref(OSyncPluginEnv *env);

/** @brief Loads the sync modules from a given directory
 * 
 * Loads all sync modules from a directory into a osync environment
 * 
 * @param env Pointer to a OSyncPluginEnv environment
 * @param path The path where to look for plugins
 * @param error Pointer to a error struct to return a error
 * @returns TRUE on success, FALSE otherwise
 * 
 */
OSYNC_EXPORT osync_bool osync_plugin_env_load(OSyncPluginEnv *env, const char *path, OSyncError **error);


/** @brief Register plugin to plugin environment 
 * 
 * @param env Pointer to a plugin environment
 * @param plugin Pointer to plugin which should get added to environment
 * @param error Pointer to a error struct to return a error
 * @returns TRUE on success, FALSE otherwise
 * 
 */
OSYNC_EXPORT osync_bool osync_plugin_env_register_plugin(OSyncPluginEnv *env, OSyncPlugin *plugin, OSyncError **error);

/** @brief Finds the plugin with the given name
 * 
 * Finds the plugin with the given name
 * 
 * @param env Pointer to a OSyncPluginEnv environment
 * @param name The name to search for
 * @returns The plugin or NULL if not found
 * 
 */
OSYNC_EXPORT OSyncPlugin *osync_plugin_env_find_plugin(OSyncPluginEnv *env, const char *name);

/* @brief Get all plugins which are registered in the Plugin Env
 * 
 * Please be aware that the returned list has to be freed with 
 * osync_list_free. If it isn't freed there will be a memory leak.
 * 
 * @param env Pointer to a OSyncPluginEnv
 * @return A shallow copy of the internal plugin list
 */
OSYNC_EXPORT OSyncList *osync_plugin_env_get_plugins(OSyncPluginEnv *env);

/*@}*/

#endif /* _OPENSYNC_PLUGIN_ENV_H_ */

