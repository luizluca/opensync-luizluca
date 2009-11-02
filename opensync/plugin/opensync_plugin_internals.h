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

#ifndef _OPENSYNC_PLUGIN_INTERNALS_H_
#define _OPENSYNC_PLUGIN_INTERNALS_H_

/* TODO: Removed from 0.40 Public API. Untested and uncomplete. */
typedef osync_bool (* usable_fn) (OSyncError **);

/**
 * @defgroup OSyncPluginInternalsAPI OpenSync Plugin Internals
 * @ingroup OSyncPluginPrivate
 * @brief Internal functions to manage plugins
 * 
 */

/*@{*/

/** @brief Get default external command of the plugin
 *
 * The default external command is set in the plugin external configuration (XML) file
 * 
 * @param plugin The plugin to check
 * @returns external command as const-char* string if a command is set, NULL otherwise.
 * 
 */
OSYNC_TEST_EXPORT const char *osync_plugin_get_external_command(OSyncPlugin *plugin);

/** @brief Get default external command of the plugin
 *
 * External command as const-char* string get substitued with the pipe-path.
 * For this, one %s is required, which will be substriuted with the pipe-path.
 *
 * Example: /usr/local/bin/foobar --pipe %s
 *
 * @param plugin The plugin to check
 * @param external_command External command as const-char* string like-printf with %s which get substitured with pipe-path
 * 
 */
OSYNC_TEST_EXPORT void osync_plugin_set_external_command(OSyncPlugin *plugin, const char *external_command);

/** @brief Checks if a plugin is available and usable
 * 
 * @param plugin The plugin to check
 * @param error If the return was FALSE, will contain information on why the plugin is not available
 * @returns TRUE if the plugin was found and is usable, FALSE otherwise
 * 
 */
OSYNC_TEST_EXPORT osync_bool osync_plugin_is_usable(OSyncPlugin *plugin, OSyncError **error);

/** @brief Set timeout interval for plugin "usable" function
 * 
 * @param plugin The plugin to check
 * @param timeout Timeout value 
 * 
 */
OSYNC_TEST_EXPORT void osync_plugin_set_useable_timeout(OSyncPlugin *plugin, unsigned int timeout);

/** @brief Get timeout interval for plugin "usable" function
 * 
 * @param plugin The plugin to check
 * @return Timeout value
 * 
 */
OSYNC_TEST_EXPORT unsigned int osync_plugin_get_useable_timeout(OSyncPlugin *plugin);

/** @brief Get timeout interval for plugin initialization 
 * 
 * @param plugin The plugin to check
 * @return Timeout value
 * 
 */
OSYNC_TEST_EXPORT unsigned int osync_plugin_get_initialize_timeout(OSyncPlugin *plugin);

/** @brief Get timeout interval for plugin finalization
 * 
 * @param plugin The plugin to check
 * @return Timeout value
 * 
 */
OSYNC_TEST_EXPORT unsigned int osync_plugin_get_finalize_timeout(OSyncPlugin *plugin);

/** @brief Get timeout interval for plugin discovery
 * 
 * @param plugin The plugin to get discvoery timeout 
 * @return Timeout value 
 * 
 */
OSYNC_TEST_EXPORT unsigned int osync_plugin_get_discover_timeout(OSyncPlugin *plugin);

/*@}*/

#endif /* _OPENSYNC_PLUGIN_INTERNALS_H_ */

