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
 * @defgroup OSyncPluginEnvPrivateAPI OpenSync Environment Internals
 * @ingroup OSyncPrivate
 * @brief The private API of the opensync environment
 * 
 */
/*@{*/

/*! @brief Loads a module into the plugin environment 
 * 
 * @param env Pointer to a plugin environment
 * @param filename Module filename, as full path, to load
 * @param error Pointer to error-struct
 * @returns TRUE on success, FALSE otherwise
 * 
 */
osync_bool osync_plugin_env_load_module(OSyncPluginEnv *env, const char *filename, OSyncError **error);

/*! @brief Checks if plugin is usable 
 * 
 * @param env Pointer to a OSyncPluginEnv environment
 * @param pluginname The name of the plugin
 * @param error Pointer to error-struct
 * @returns TRUE if plugin is usable, FALSE otherwise 
 * 
 */
OSYNC_TEST_EXPORT osync_bool osync_plugin_env_plugin_is_usable(OSyncPluginEnv *env, const char *pluginname, OSyncError **error);

/*@}*/

#endif /* _OPENSYNC_PLUGIN_ENV_INTERNALS_H_ */

