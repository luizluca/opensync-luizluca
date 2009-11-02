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

#ifndef _OPENSYNC_PLUGIN_ENV_INTERNALS_H_
#define _OPENSYNC_PLUGIN_ENV_INTERNALS_H_

/**
 * @defgroup OSyncPluginEnvInternalAPI OpenSync Plugin Environment Internals
 * @ingroup OSyncPluginPrivate
 * @brief The internal API of the opensync plugin environment
 * 
 */

/*@{*/

#define OSYNC_EXTERNAL_PLUGIN_CONFIG_SCHEMA "external_plugin_config.xsd"

/** @brief Loads a module into the plugin environment 
 * 
 * @param env Pointer to a plugin environment
 * @param filename Module filename, as full path, to load
 * @param error Pointer to error-struct
 * @returns TRUE on success, FALSE otherwise
 * 
 */
osync_bool osync_plugin_env_load_module(OSyncPluginEnv *env, const char *filename, OSyncError **error);

/** @brief Loads a configuration file for an external module into the plugin environment 
 * 
 * @param env Pointer to a plugin environment
 * @param filename Config filename, as full path, to load
 * @param error Pointer to error-struct
 * @returns TRUE on success, FALSE otherwise
 * 
 */
osync_bool osync_plugin_env_load_module_xml(OSyncPluginEnv *env, const char *filename, OSyncError **error);

/** @brief Checks if plugin is usable 
 * 
 * @param env Pointer to a OSyncPluginEnv environment
 * @param pluginname The name of the plugin
 * @param error Pointer to error-struct
 * @returns TRUE if plugin is usable, FALSE otherwise 
 * 
 */
OSYNC_TEST_EXPORT osync_bool osync_plugin_env_plugin_is_usable(OSyncPluginEnv *env, const char *pluginname, OSyncError **error);

/** @brief Returns the number of loaded plugins
 * 
 * TODO: This function isn't necessary anymore and can possibly be removed
 * 
 * Returns the number of loaded plugins. 0 if used before initialization
 * 
 * @param env Pointer to a OSyncPluginEnv environment
 * @returns Number of plugins
 * 
 */
OSYNC_TEST_EXPORT unsigned int osync_plugin_env_num_plugins(OSyncPluginEnv *env);


#ifdef OPENSYNC_UNITTESTING
/** @brief Change the schema path to a non-default directory
 * 
 * XXX: This is only intended for unittesting.
 * 
 * @param env Pointer to a OSyncPluginEnv environment
 * @param schemapath Path to the non-default schema-path 
 * 
 */
OSYNC_TEST_EXPORT void osync_plugin_env_set_schemapath(OSyncPluginEnv *env, const char *schemapth);
#endif /* OPENSYNC_UNITTESTING */

/*@}*/

#endif /* _OPENSYNC_PLUGIN_ENV_INTERNALS_H_ */

