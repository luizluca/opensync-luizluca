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

#ifndef _OPENSYNC_PLUGIN_H_
#define _OPENSYNC_PLUGIN_H_

/**
 * @defgroup OSyncPlugin OpenSync Plugin Module
 * @ingroup OSyncPublic
 * @defgroup OSyncPluginAPI OpenSync Plugin
 * @ingroup OSyncPlugin
 * @brief The public part of the OSyncPlugin
 * 
 * Functions to register and manage plugins
 */

/*@{*/

/**
 * @brief Prototype of the plugin initialize function
 * 
 * In this function a plugin should initialize and allocate 
 * all plugin specific data which is required in the other plugin
 * functions. This data could be e.g. connections, directories, ...
 * @param plugin
 * @param info the OSyncPluginInfo
 * @param error An OSyncError struct that should be used to set an error
 * @return The plugin specific data that is passed to the other plugin functions
 */
typedef void * (* initialize_fn) (OSyncPlugin *plugin, OSyncPluginInfo *info, OSyncError **error);

/**
 * @brief Prototype of the plugin finalize function
 * 
 * This plugin function is called to give a plugin the possibility
 * to free all allocated data.
 * @param plugin_data Plugin specific data that was returned by the initialize function
 */
typedef void (* finalize_fn) (void * plugin_data);

/**
 * @brief Prototype of the plugin discovery function
 * 
 * TODO Add detailed description
 * @param info the OSyncPluginInfo
 * @param plugin_data Plugin specific data that was returned in the plugin initialize function
 * @param error An OSyncError struct that should be used to set an error
 * @return TRUE if discovery was successful
 */
typedef osync_bool (* discover_fn) (OSyncPluginInfo *info, void * plugin_data, OSyncError **error);

/** @brief Gives information about wether the plugin
 * has to be configured or not
 **/
typedef enum {
	/** Plugin has no configuration options */
	OSYNC_PLUGIN_NO_CONFIGURATION = 0,
	/** Plugin can be configured, but will accept the default config in the initialize function */
	OSYNC_PLUGIN_OPTIONAL_CONFIGURATION = 1,
	/** Plugin must be configured to run correctly */
	OSYNC_PLUGIN_NEEDS_CONFIGURATION = 2
} OSyncPluginConfigurationType;

/** @brief Registers a new plugin
 *
 * This function creates a new OSyncPlugin object, that
 * can be used to register a new plugin dynamically. This
 * can be used by a module to register multiple plugins,
 * instead of using the get_info() function which allows
 * registering only one plugin.
 *
 * @param error Pointer to an error struct
 * @returns the newly registered plugin
 */
OSYNC_EXPORT OSyncPlugin *osync_plugin_new(OSyncError **error);

/** @brief Decrease the reference count on a plugin
 * 
 * @param plugin Pointer to the plugin
 * 
 */
OSYNC_EXPORT void osync_plugin_unref(OSyncPlugin *plugin);

/** @brief Increase the reference count on a plugin
 * 
 * @param plugin Pointer to the plugin
 * 
 */
OSYNC_EXPORT OSyncPlugin *osync_plugin_ref(OSyncPlugin *plugin);


/** @brief Returns the name of a plugin
 * 
 * @param plugin Pointer to the plugin
 * @returns Name of the plugin
 * 
 */
OSYNC_EXPORT const char *osync_plugin_get_name(OSyncPlugin *plugin);

/** @brief Sets the name of a plugin
 * 

 * This name is mostly used for internal configuration and isn't visible to the
 * user (at least not in rich OpenSync frontends). The name should be less then
 * 15 characters and should be one word (no spaces).
 *
 * @param plugin Pointer to the plugin
 * @param name the name to set
 * 
 */
OSYNC_EXPORT void osync_plugin_set_name(OSyncPlugin *plugin, const char *name);


/** @brief Returns the long name of a plugin
 * 
 * @param plugin Pointer to the plugin
 * @returns Long name of the plugin
 * 
 */
OSYNC_EXPORT const char *osync_plugin_get_longname(OSyncPlugin *plugin);

/** @brief Sets the long name of a plugin
 * 
 * The longname of the plugin is the name visible to a regular user to choose the
 * correct plugin from a list of available plugins. Use the description field to
 * describe the plugin in more detail. Don't include the term "Plugin" in the
 * longname. Example: "Palm Device"
 *
 * @param plugin Pointer to the plugin
 * @param longname the long name to set
 * 
 */
OSYNC_EXPORT void osync_plugin_set_longname(OSyncPlugin *plugin, const char *longname);


/** @brief Returns whether or not the plugin requires configuration
 *
 * @param plugin Pointer to the plugin
 * @returns The configuration requirement type of the plugin
 */
OSYNC_EXPORT OSyncPluginConfigurationType osync_plugin_get_config_type(OSyncPlugin *plugin);

/** @brief Sets whether or not the plugin requires configuration
 *
 * @param plugin Pointer to the plugin
 * @param type The configuration requirement type of the plugin
 */
OSYNC_EXPORT void osync_plugin_set_config_type(OSyncPlugin *plugin, OSyncPluginConfigurationType type);


/** @brief Returns start type of plugin 
 *
 * @param plugin Pointer to the plugin
 * @returns The start type of the plugin
 */
OSYNC_EXPORT OSyncStartType osync_plugin_get_start_type(OSyncPlugin *plugin);

/** @brief Sets the start type of the plugin 
 *
 * @param plugin Pointer to the plugin
 * @param type The start type of the plugin
 */
OSYNC_EXPORT void osync_plugin_set_start_type(OSyncPlugin *plugin, OSyncStartType type);


/** @brief Returns the description of a plugin
 * 
 * @param plugin Pointer to the plugin
 * @returns Description of the plugin
 * 
 */
OSYNC_EXPORT const char *osync_plugin_get_description(OSyncPlugin *plugin);

/** @brief Sets the description of a plugin
 * 
 * The description should help the user to choose the correct plugin if there
 * are several plugins with similar names. The description should probably be no
 * more than 200 characters.  The term "Plugin" might confuse regular user,
 * avoid it.
 *
 * @param plugin Pointer to the plugin
 * @param description the description to set
 * 
 */
OSYNC_EXPORT void osync_plugin_set_description(OSyncPlugin *plugin, const char *description);


/** @brief Sets the initialize function for a plugin
 *
 * The initialize function of a plugin sets up sinks for the plugin as well
 * as other plugin-wide structures.
 *
 * @param plugin Pointer to the plugin
 * @param init The initialize function for the plugin
 */
OSYNC_EXPORT void osync_plugin_set_initialize_func(OSyncPlugin *plugin, initialize_fn init);

/** @brief Sets the finalize function for a plugin
 *
 * The finalize function of a plugin frees any plugin-wide structures
 * that were created in the initialize function.
 *
 * @param plugin Pointer to the plugin
 * @param fin The finalize function for the plugin
 */
OSYNC_EXPORT void osync_plugin_set_finalize_func(OSyncPlugin *plugin, finalize_fn fin);

/** @brief Sets the optional discover function for a plugin
 *
 * The discover function of a plugin can be used to specify which 
 * of the sinks in the plugin are currently available, and to declare
 * the compatible device versions for the plugin. It can also
 * be used to set the plugin's capabilities.
 *
 * The discover function is optional.
 *
 * @param plugin Pointer to the plugin
 * @param discover The discover function for the plugin
 */
OSYNC_EXPORT void osync_plugin_set_discover_func(OSyncPlugin *plugin, discover_fn discover);


/** @brief Returns the plugin_info data, set by the plugin
 *
 * @param plugin Pointer to the plugin
 * @returns The void pointer set on plugin->info.plugin_data
 */
OSYNC_EXPORT void *osync_plugin_get_data(OSyncPlugin *plugin);

/** @brief Set the plugin_info data for the plugin object
 *
 * @param plugin Pointer to the plugin
 * @param data Pointer to data which should get set 
 */
OSYNC_EXPORT void osync_plugin_set_data(OSyncPlugin *plugin, void *data);

/** @brief Set timeout interval for plugin discovery
 * 
 * @param plugin The plugin to check
 * @param timeout Timeout value 
 * 
 */
OSYNC_EXPORT void osync_plugin_set_discover_timeout(OSyncPlugin *plugin, unsigned int timeout);

/** @brief Set timeout interval for plugin initialization 
 * 
 * @param plugin The plugin to check
 * @param timeout Timeout value 
 * 
 */
OSYNC_EXPORT void osync_plugin_set_initialize_timeout(OSyncPlugin *plugin, unsigned int timeout);

/** @brief Set timeout interval for plugin finalization 
 * 
 * @param plugin The plugin to check
 * @param timeout Timeout value 
 * 
 */
OSYNC_EXPORT void osync_plugin_set_finalize_timeout(OSyncPlugin *plugin, unsigned int timeout);

/** @brief Initialize a Plugin 
 *
 * @param plugin Pointer to the plugin
 * @param plugin_data Pointer to store the data that is returned by the plugin
 * @param info Pointer to OSyncPluginInfo which describes the plugin 
 * @param error Pointer to error-struct
 * @return TREU if successful, FALSE otherwise
 */
OSYNC_EXPORT osync_bool osync_plugin_initialize(OSyncPlugin *plugin, void **plugin_data, OSyncPluginInfo *info, OSyncError **error);

/** @brief Finalize a Plugin 
 *
 * @param plugin Pointer to the plugin
 * @param data Pointer to userdata which got returned by plugin initialize function
 */
OSYNC_EXPORT void osync_plugin_finalize(OSyncPlugin *plugin, void *data);

/** @brief Call plugin discovery
 *
 * @param plugin Pointer to the plugin
 * @param data Pointer to userdata which got returned by plugin initialize function
 * @param info Pointer to OSyncPluginInfo which describes the plugin 
 * @param error Pointer to error-struct
 * @return TRUE on success, FALSE otherwise 
 */
OSYNC_EXPORT osync_bool osync_plugin_discover(OSyncPlugin *plugin, void *data, OSyncPluginInfo *info, OSyncError **error);

/*@}*/

#endif /* OPENSYNC_PLUGIN_H_ */

