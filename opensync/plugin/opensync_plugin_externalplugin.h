/*
 * libopensync - A synchronization framework
 * Copyright (C) 2009  Henrik Kaare Poulsen
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

#ifndef _OPENSYNC_PLUGIN_EXTERNALPLUGIN_H_
#define _OPENSYNC_PLUGIN_EXTERNALPLUGIN_H_

/**
 * @defgroup OSyncPluginExternalPluginAPI OpenSync Plugin ExternalPlugin
 * @ingroup OSyncPlugin
 * @brief Functions for configuring external plugin 
 * 
 */
/*@{*/


/** @brief Create a new OSyncPluginExternalPlugin object
 *
 * @param error Pointer to an error struct
 * @returns the newly created object, or NULL in case of an error.
 *
 */
OSYNC_EXPORT OSyncPluginExternalPlugin *osync_plugin_externalplugin_new(OSyncError **error);

/** @brief Decrease the reference count on an OSyncPluginExternalPlugin object
 * 
 * @param external_plugin Pointer to the OSyncPluginExternalPlugin object
 * 
 */
OSYNC_EXPORT void osync_plugin_externalplugin_unref(OSyncPluginExternalPlugin *external_plugin);

/** @brief Increase the reference count on an OSyncPluginExternalPlugin object
 * 
 * @param external_plugin Pointer to the OSyncPluginExternalPlugin object
 * @returns The OSyncPluginExternalPlugin object passed in
 * 
 */
OSYNC_EXPORT OSyncPluginExternalPlugin *osync_plugin_externalplugin_ref(OSyncPluginExternalPlugin *external_plugin);


/** @brief Get the command to start the external plugin process
 *
 * @param external_plugin Pointer to the OSyncPluginExternalPlugin object
 * @returns the external command or NULL if not set
 *
 */
OSYNC_EXPORT const char *osync_plugin_externalplugin_get_external_command(OSyncPluginExternalPlugin *external_plugin);

/** @brief Set the command to start the external plugin process
 *
 * If the plugin is of type OSYNC_START_TYPE_EXTERNAL, an external command can be executed by OpenSync.
 * The external_command should be a string in printf format, with one %s.
 * Before the command is executed, a variant of printf will be called
 * to replace the %s with the path to the plugin pipe.
 * The resulting command will be exectued with the glib function
 * g_spawn_command_line_async.
 * 
 * Example: "thunderbird -mozilla-sync %s"
 *
 * @param external_plugin Pointer to the OSyncPluginExternalPlugin object
 * @param external_command The external command to set
 *
 */
OSYNC_EXPORT void osync_plugin_externalplugin_set_external_command(OSyncPluginExternalPlugin *external_plugin, const char *external_command);



/*@}*/

#endif /*_OPENSYNC_PLUGIN_EXTERNALPLUGIN_H_*/

