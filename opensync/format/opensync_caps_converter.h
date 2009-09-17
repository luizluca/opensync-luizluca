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

#ifndef _OPENSYNC_CAPS_CONVERTER_H_
#define _OPENSYNC_CAPS_CONVERTER_H_

/**
 * @defgroup OSyncConverterAPI OpenSync Converter
 * @ingroup OSyncFormat
 * @brief Functions for creating and managing object format converters
 *
 */
/*@{*/

typedef osync_bool (* OSyncCapsConvertFunc) (OSyncCapabilities *oldcaps, OSyncCapabilities **newcaps, const char *config, void *userdata, OSyncError **error);
typedef void * (* OSyncCapsConverterInitializeFunc) (const char *config, OSyncError **error);
typedef osync_bool (* OSyncCapsConverterFinalizeFunc) (void *userdata, OSyncError **error);

/**
 * @brief Creates a new converter
 * @param type the type of converter
 * @param sourceformat the source format for the converter
 * @param targetformat the target format for the converter
 * @param convert_func the converter function
 * @param error Pointer to an error struct
 * @returns The pointer to the newly allocated converter or NULL in case of error
 */
OSYNC_EXPORT OSyncCapsConverter *osync_caps_converter_new(const char *sourceformat, const char *targetformat, OSyncCapsConvertFunc convert_func, OSyncError **error);

/** @brief Increase the reference count on a converter
 *
 * @param converter Pointer to the converter
 *
 */
OSYNC_EXPORT OSyncCapsConverter *osync_caps_converter_ref(OSyncCapsConverter *converter);

/** @brief Decrease the reference count on a converter
 *
 * @param converter Pointer to the converter
 *
 */
OSYNC_EXPORT void osync_caps_converter_unref(OSyncCapsConverter *converter);

/**
 * @brief Returns the source format of a converter
 * @param converter Pointer to the converter
 * @returns The source format of the specified converter
 */
OSYNC_EXPORT const char *osync_caps_converter_get_sourceformat(OSyncCapsConverter *converter);

/**
 * @brief Returns the target format of a converter
 * @param converter Pointer to the converter
 * @returns The target format of the specified converter
 */
OSYNC_EXPORT const char *osync_caps_converter_get_targetformat(OSyncCapsConverter *converter);

/**
 * @brief Invokes converter for OSyncCapabilities object with passed format converter configuration
 *
 * @param converter Pointer to the converter
 * @param caps Pointer to OSyncCapabilitiesobject which should be converted
 * @param config Format converter configuration
 * @param error Pointer to an error struct
 * @returns TRUE on successful conversion, FALSE on error
 */
OSYNC_EXPORT osync_bool osync_caps_converter_invoke(OSyncCapsConverter *converter, OSyncCapabilities **caps, const char *config, OSyncError **error);

/**
 * @brief Sets the initialize function of a converter
 * @param converter Pointer to the converter
 * @param initialize_func Pointer to the initialize function
 */
OSYNC_EXPORT void osync_caps_converter_set_initialize_func(OSyncCapsConverter *converter, OSyncCapsConverterInitializeFunc initialize_func);

/**
 * @brief Sets the finalize function of a converter
 * @param converter Pointer to the converter
 * @param finalize_func Pointer to the finalize function
 */
OSYNC_EXPORT void osync_caps_converter_set_finalize_func(OSyncCapsConverter *converter, OSyncCapsConverterFinalizeFunc finalize_func);

/**
 * @brief Invokes initialize function of a converter
 *
 * @param converter Pointer to the converter which should be initialized
 * @param config configuration
 * @param error Pointer to an error struct
 *
 * @todo config is not used currently. Should be used in the future to pass a directory for xml format schema location
 */
OSYNC_EXPORT void osync_caps_converter_initialize(OSyncCapsConverter *converter, const char *config, OSyncError **error);

/**
 * @brief Invokes finalize function of a converter
 *
 * @param converter Pointer to the converter which should be finalized
 * @param error Pointer to an error struct
 * @returns TRUE on success, FALSE otherwise
 */
OSYNC_EXPORT osync_bool osync_caps_converter_finalize(OSyncCapsConverter *converter, OSyncError **error);

/*@}*/

#endif //_OPENSYNC_CAPS_CONVERTER_H_
