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

#ifndef _OPENSYNC_FORMAT_ENV_H_
#define _OPENSYNC_FORMAT_ENV_H_

/**
 * @defgroup OSyncPublic OpenSync Public API
 * @brief The public API of opensync
 * 
 * This gives you an insight in the public API of opensync.
 * 
 */

/**
 * @defgroup OSyncFormatEnvAPI OpenSync Format Environment
 * @ingroup OSyncFormat
 * @brief The public API of the OpenSync Format Environment
 * 
 */
/*@{*/

/** @brief This will create a new OpenSync Format Environment
 * 
 * The format environment will hold all information about format plugins.
 * 
 * @returns A pointer to a newly allocated format environment. NULL on error.
 * 
 */
OSYNC_EXPORT OSyncFormatEnv *osync_format_env_new(OSyncError **error);

/*! @brief Increases the reference counton an opensync plugin environment
 * 
 * When storing a reference to an OSyncFormatEnv the reference count must be
 * manually incremented. When the reference is no longer needed it should
 * be removed with  osync_plugin_env_unref
 * 
 * @param env Pointer to the environment to reference
 * @returns The environment passed in
 * 
 */
OSYNC_EXPORT OSyncFormatEnv *osync_format_env_ref(OSyncFormatEnv *env);

/** @brief Decreases the reference count on an OSyncFormatEnv
 * 
 * Decrements the reference count on an osync format environment.  If
 * the reference count reaches zero the environment is freed and all
 * resources are unreferenced
 * 
 * @param env Pointer to the environment
 * 
 */
OSYNC_EXPORT void osync_format_env_unref(OSyncFormatEnv *env);

/** @brief Loads all format and conversion plugins
 * 
 * This command will load all plugins for the conversion system.
 * If you dont change the path before it will load the plugins
 * from the default location
 * 
 * @param env The format environment
 * @param path The path to load from or NULL if to load from default path
 * @param error The location to return a error to
 * @returns TRUE if successful, FALSE otherwise
 * 
 */
OSYNC_EXPORT osync_bool osync_format_env_load_plugins(OSyncFormatEnv *env, const char *path, OSyncError **error);

/** @brief Register Object Format to the Format Environment 
 * 
 * @param env Pointer to the environment
 * @param format Pointer ot the Object Format which sould be registred
 * 
 */
OSYNC_EXPORT void osync_format_env_register_objformat(OSyncFormatEnv *env, OSyncObjFormat *format);

/** @brief Finds the object format with the given name
 * 
 * @param env Pointer to the environment
 * @param name Name of the format type to find
 * @returns The object format, or NULL if not found
 * 
 */
OSYNC_EXPORT OSyncObjFormat *osync_format_env_find_objformat(OSyncFormatEnv *env, const char *name);

/** @brief Returns the number of available object formats
 * 
 * @param env The format environment
 * @returns The number of object formats
 * 
 */
OSYNC_EXPORT unsigned int osync_format_env_num_objformats(OSyncFormatEnv *env);

/** @brief Gets the nth object format
 * 
 * @param env The format environment
 * @param nth The position of the object format to retrieve
 * @returns The object format
 * 
 */
OSYNC_EXPORT OSyncObjFormat *osync_format_env_nth_objformat(OSyncFormatEnv *env, unsigned int nth);

/** @brief Registers Format Converter or Detector to Format Environment
 * 
 * @param env The format environment
 * @param converter Pointer of the Format Converter or Detector
 */
OSYNC_EXPORT void osync_format_env_register_converter(OSyncFormatEnv *env, OSyncFormatConverter *converter);

/** @brief Finds first converter with the given source and target format
 * 
 * @param env Pointer to the environment
 * @param sourceformat The source format
 * @param targetformat The target format
 * @returns The converter, or NULL if not found
 * 
 */
OSYNC_EXPORT OSyncFormatConverter *osync_format_env_find_converter(OSyncFormatEnv *env, OSyncObjFormat *sourceformat, OSyncObjFormat *targetformat);

/** @brief Returns a list of all converters with the given source and target format
 * 
 * The returned list must be freed with osync_list_free().  The ref count on each
 * OSyncFormatConverter is not increased.
 * 
 * @param env Pointer to the environment
 * @param sourceformat The source format
 * @param targetformat The target format
 * @returns List of OSyncFormatConverter, or NULL if none found
 * 
 */
OSYNC_EXPORT OSyncList *osync_format_env_find_converters(OSyncFormatEnv *env, OSyncObjFormat *sourceformat, OSyncObjFormat *targetformat);

/** @brief Returns the number of available converters
 * 
 * @param env The format environment
 * @returns The number of converters
 * 
 */
OSYNC_EXPORT int osync_format_env_num_converters(OSyncFormatEnv *env);

/** @brief Gets the nth format converter
 * 
 * @param env The format environment
 * @param nth The position of the format converter to retrieve
 * @returns The format converter
 * 
 */
OSYNC_EXPORT OSyncFormatConverter *osync_format_env_nth_converter(OSyncFormatEnv *env, int nth);

/** @brief Tries to detect the format of the given data object
 * 
 * This will try to detect the format of the specified data object
 * and return it, but not set it.
 * 
 * @param env The conversion environment to use
 * @param data The data object to detect
 * @returns The format on success, NULL otherwise
 * 
 */
OSYNC_EXPORT OSyncObjFormat *osync_format_env_detect_objformat(OSyncFormatEnv *env, OSyncData *data);

/** @brief Tries to detect the encapsulated format of the given data object
 * 
 * This will try to detect the encapsulated format of the specified data object
 * and return it, but not set it. It will try to deencapsulate it, detect it,
 * deencapuslate it again etc until it cannot deencapsulate further.
 * 
 * @param env The conversion environment to use
 * @param input The data object to detect
 * @param detected_format Reference to OSyncObjFormat which get set with detected format, if any got detected
 * @param error The error-return location, get set on any error
 * @returns TRUE when format detection successfully completed, FALSE on any error. 
 * 
 */
OSYNC_EXPORT osync_bool osync_format_env_detect_objformat_full(OSyncFormatEnv *env, OSyncData *input, OSyncObjFormat **detected_format, OSyncError **error);

/** @brief Convert a data object using a specific conversion path
 * 
 * This will convert the specified data object using the specified format
 * conversion path if possible.
 * 
 * @param env The conversion environment to use
 * @param path The conversion path to follow
 * @param data The data object to convert
 * @param error The error-return location
 * @returns TRUE on success, FALSE otherwise
 * 
 */
OSYNC_EXPORT osync_bool osync_format_env_convert(OSyncFormatEnv *env, OSyncFormatConverterPath *path, OSyncData *data, OSyncError **error);

/** @brief Find a conversion path between two formats
 * 
 * This will find a conversion path between two object formats
 * if possible.
 * 
 * @param env The conversion environment to use
 * @param sourceformat The source format to be converted from
 * @param targetformat The target format to be converted to
 * @param error The error-return location
 * @returns The appropriate conversion path, or NULL if an error occurred.
 * 
 */
OSYNC_EXPORT OSyncFormatConverterPath *osync_format_env_find_path(OSyncFormatEnv *env, OSyncObjFormat *sourceformat, OSyncObjFormat *targetformat, OSyncError **error);

/** @brief Find a conversion path from one format to one of a list of formats
 * 
 * @param env The conversion environment to use
 * @param sourceformat The source format to be converted from
 * @param targets List of possible Object Format Sinks
 * @param error The error-return location
 * @returns The appropriate conversion path, or NULL if an error occurred.
 * 
 */
OSYNC_EXPORT OSyncFormatConverterPath *osync_format_env_find_path_formats(OSyncFormatEnv *env, OSyncObjFormat *sourceformat, OSyncList *targets, OSyncError **error);

/** @brief Find a conversion path from the source format to a specific format with help of detectors.
 * 
 * This will find a conversion path between two object formats
 * if possible.
 *
 * @param env The format environment to use
 * @param sourcedata The OSyncData object which should be converted and the detectors will run on
 * @param targetformat The target format to be converted to
 * @param preferred_format Format that should be preferred to build the conversion path
 * @param error The error-return location
 * @return The appropriate conversion path, or NULL if an error occurred.
 * 
 */
OSYNC_EXPORT OSyncFormatConverterPath *osync_format_env_find_path_with_detectors(OSyncFormatEnv *env, OSyncData *sourcedata, OSyncObjFormat *targetformat, const char *preferred_format, OSyncError **error);

/** @brief Find a conversion path from one format to one of a list of formats with the help of detectors
 * 
 * @param env The format environment to use
 * @param sourcedata The OSyncData object which should be converted and the detectors will run on
 * @param targets List of possible Object Format Sinks
 * @param preferred_format Format that should be preferred to build the conversion path
 * @param error The error-return location
 * @returns The appropriate conversion path, or NULL if an error occurred.
 * 
 */
OSYNC_EXPORT OSyncFormatConverterPath *osync_format_env_find_path_formats_with_detectors(OSyncFormatEnv *env, OSyncData *sourcedata, OSyncList *targets, const char *preferred_format, OSyncError **error);

/*@}*/

#endif /* _OPENSYNC_FORMAT_ENV_H_ */
