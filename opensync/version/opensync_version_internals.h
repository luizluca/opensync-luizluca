/*
 * libopensync - A synchronization framework
 * Copyright (C) 2006  NetNix Finland Ltd <netnix@netnix.fi>
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
 * Author: Daniel Friedrich <daniel.friedrich@opensync.org>
 * 
 */
 
#ifndef OPENSYNC_VERSION_INTERNALS_H_
#define OPENSYNC_VERSION_INTERNALS_H_


/**
 * @defgroup OSyncVersionInternalAPI OpenSync Version Internals
 * @ingroup OSyncVersionPrivate
 * @brief The internal part of the OSyncVersion API representing a Version Object
 *
 */
/*@{*/


/**
 *  @brief Searching for capabilities for specified OSyncVersion object.
 * 
 *  @param version Pointer to OSyncVersion object
 *  @param error Pointer to error-struct
 *  @returns Pointer to OSyncCapabilties object, NULL if none capabilities are found
 */
OSyncCapabilities *osync_version_find_capabilities(OSyncVersion *version, OSyncError **error);

/**
 * @brief Loads description from the default description directory and returns an OSyncList
 * with OSyncVersions objects.
 *
 * @param error Pointer to error-struct
 * @returns List of OSyncVersion objects, NULL if none found
 */
OSyncList *osync_version_load_from_default_descriptions(OSyncError **error);

/**
 * @brief Loads description from a specified directory and returns an OSyncList
 * with OSyncVersions objects.
 *
 * This internal function allows to adjust the description and schema directories.
 *
 * @param error Pointer to error-struct
 * @param descriptiondir Path to description directory
 * @param schemadir Path to XML schema directory
 * @returns List of OSyncVersion objects, NULL if none found
 */
OSYNC_TEST_EXPORT OSyncList *osync_version_load_from_descriptions(OSyncError **error, const char *descriptiondir, const char *schemadir);

/**
 * @brief Get Plugin Name
 * @param version The pointer to a version object
 * @returns Plugin Name or NULL
 */
/* FIXME: char* to const char* */
char *osync_version_get_plugin(OSyncVersion *version);

/**
 * @brief Get Priority
 * @param version The pointer to a version object
 * @returns Priority or NULL
 */
char *osync_version_get_priority(OSyncVersion *version);

/**
 * @brief Get Vendor
 * @param version The pointer to a version object
 * @returns Vendor or NULL
 */
char *osync_version_get_vendor(OSyncVersion *version);

/**
 * @brief Get Model Version
 * @param version The pointer to a version object
 * @returns Model Version or NULL
 */
char *osync_version_get_modelversion(OSyncVersion *version);

/**
 * @brief Get Firmware Version
 * @param version The pointer to a version object
 * @returns Firmware Version or NULL
 */
char *osync_version_get_firmwareversion(OSyncVersion *version);

/**
 * @brief Get Software Version
 * @param version The pointer to a version object
 * @returns Software Version or NULL
 */
char *osync_version_get_softwareversion(OSyncVersion *version);

/**
 * @brief Get Hardware Version
 * @param version The pointer to a version object
 * @returns Hardware Version or NULL
 */
char *osync_version_get_hardwareversion(OSyncVersion *version);

/**
 * @brief Get Identifier
 * @param version The pointer to a version object
 * @returns Identifier or NULL
 */
char *osync_version_get_identifier(OSyncVersion *version);

/**
 * @brief Matches the version object with a pattern object and returns the priority
 * of the pattern if it matches the original version object.
 *
 * @param pattern The pointer to a version object which acts as pattern
 * @param version The version (original) object supplied to find a fitting version pattern
 * @param error Pointer to a error-struct
 * @returns Priority of matching pattern, -1 on error
 */
OSYNC_TEST_EXPORT int osync_version_matches(OSyncVersion *pattern, OSyncVersion *version, OSyncError **error);
/*@}*/

#endif /* OPENSYNC_VERSION_INTERNALS_H_ */

