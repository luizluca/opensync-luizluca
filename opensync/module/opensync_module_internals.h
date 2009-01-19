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

#ifndef _OPENSYNC_MODULE_INTERNALS_H_
#define _OPENSYNC_MODULE_INTERNALS_H_

/**
 * @defgroup OSyncModulePrivate OpenSync Module Module Private
 * @ingroup OSyncPrivate
 * @defgroup OSyncModuleInternalAPI OpenSync Module Internals
 * @ingroup OSyncModulePrivate
 * @brief Internals of OpenSync Module
 */

/*@{*/

/** 
 * @brief Represent a synchronzation plugin
 */
struct OSyncModule {
	/** Pointer to the Module */
	GModule *module;
	/** Directory where the module is loaded from */
	char *path;

	/** Reference Counter */
	int ref_count;
};

/**
 * @brief Creates a new Module
 * @param error An OpenSync Error
 * @return The new OSyncModule
 */
OSYNC_TEST_EXPORT OSyncModule *osync_module_new(OSyncError **error);

/** 
 * @brief Increase the reference count on a module
 *
 * When storing a reference to an OSyncModule the reference count
 * on the module must be manually incremented
 * @code
 * format->module = osync_module_ref(module);
 * @endcode
 *
 * @param module pointer to module
 * @returns the passed module
 *
 */
OSYNC_TEST_EXPORT OSyncModule *osync_module_ref(OSyncModule *module);

/** @brief Used to reduce the reference count on a module
 * 
 * Decreases the reference count on a module.  If the reference count reaches
 * zero the module is freed.
 * 
 * @param module Pointer to the module
 * 
 */
OSYNC_TEST_EXPORT void osync_module_unref(OSyncModule *module);

/**
 * @brief Loads a module
 * 
 * @param module Module that should be loaded
 * @param path Where to find this module
 * @param error Pointer to an error struct
 * @return TRUE on success, FALSE otherwise
 * 
 */
OSYNC_TEST_EXPORT osync_bool osync_module_load(OSyncModule *module, const char *path, OSyncError **error);

/**
 * @brief Closes a module
 * 
 * @param module The module to unload
 * 
 */
OSYNC_TEST_EXPORT void osync_module_unload(OSyncModule *module);

/**
 * @brief Calls the get_sync_info function of a module (Sync Plugin)
 * @param module Plugin which contains the function
 * @param env Plugin Environment that is passed to get_sync_info function of the plugin
 * @param error An OpenSync Error
 * @return TRUE if successful
 */
OSYNC_TEST_EXPORT osync_bool osync_module_get_sync_info(OSyncModule *module, OSyncPluginEnv *env, OSyncError **error);

/**
 * @brief Calls the get_format_info function of a module (Format Plugin)
 * @param module Plugin which contains the function
 * @param env Format Environment that is passed to get_format_info function of the plugin
 * @param error An OpenSync Error
 * @return TRUE if successful
 */
OSYNC_TEST_EXPORT osync_bool osync_module_get_format_info(OSyncModule *module, OSyncFormatEnv *env, OSyncError **error);

/**
 * @brief Calls the get_conversion_info function of a module (Format Plugin)
 * @param module Plugin which contains the function
 * @param env Format Environment that is passed to get_format_info function of the plugin
 * @param error An OpenSync Error
 * @return TRUE if successful
 */
OSYNC_TEST_EXPORT osync_bool osync_module_get_conversion_info(OSyncModule *module, OSyncFormatEnv *env, OSyncError **error);

/**
 * @brief Calls the get_version function of a module
 * @param module Plugin which contains the function
 * @return The version which is provided by the module
 */
OSYNC_TEST_EXPORT int osync_module_get_version(OSyncModule *module);

/**
 * @brief Compares version which is supported by the plugin with OPENSYNC_PLUGINVERSION
 * osync_module_check calls osync_get_version and compares it with OPENSYNC_PLUGINVERSION.
 * @param module Plugin to compare
 * @param error An OpenSync Error
 * @return TRUE if the passed version of the plugin matches OPENSYNC_PLUGINVERSION
 */
OSYNC_TEST_EXPORT osync_bool osync_module_check(OSyncModule *module, OSyncError **error);

/**
 * @brief Used to look up a symbol on the plugin
 * 
 * Looks up and returns a function
 * 
 * @param module Pointer to the module
 * @param name The name of the function to look up
 * @param error Pointer to an error struct
 * @return Pointer to the function
 * 
 */
OSYNC_TEST_EXPORT void *osync_module_get_function(OSyncModule *module, const char *name, OSyncError **error);

/*@}*/

#endif //_OPENSYNC_MODULE_INTERNALS_H_
