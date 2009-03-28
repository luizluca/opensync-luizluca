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
 
#ifndef OPENSYNC_VERSION_H_
#define OPENSYNC_VERSION_H_

/**
 * @defgroup OSyncVersion OpenSync Version Module
 * @ingroup OSyncPublic
 * @defgroup OSyncVersionAPI OpenSync Version
 * @ingroup OSyncVersion
 * @brief The public part of the OSyncVersion, representing a Version object
 * 
 */
/*@{*/

/**
 * @brief Creates a new version object
 * @param error The error which will hold the info in case of an error
 * @return The pointer to the newly allocated version object or NULL in case of error
 */
OSYNC_EXPORT OSyncVersion *osync_version_new(OSyncError **error);

/**
 * @brief Increments the reference counter
 * @param version The pointer to a version object
 */
OSYNC_EXPORT OSyncVersion *osync_version_ref(OSyncVersion *version);

/**
 * @brief Decrement the reference counter. The version object will
 *  be freed if there is no more reference to it.
 * @param version The pointer to a version object
 */
OSYNC_EXPORT void osync_version_unref(OSyncVersion *version);

/**
 * @brief Set Plugin Name
 * @param version The pointer to a version object
 * @param plugin Plugin Name
 */
OSYNC_EXPORT void osync_version_set_plugin(OSyncVersion *version, const char *plugin);

/**
 * @brief Set Priority
 * @param version The pointer to a version object
 * @param priority Priority
 */
OSYNC_EXPORT void osync_version_set_priority(OSyncVersion *version, const char *priority);

/**
 * @brief Set Vendor
 * @param version The pointer to a version object
 * @param vendor Vendor
 */
OSYNC_EXPORT void osync_version_set_vendor(OSyncVersion *version, const char *vendor);

/**
 * @brief Set Model Version
 * @param version The pointer to a version object
 * @param modelversion Model Version
 */
OSYNC_EXPORT void osync_version_set_modelversion(OSyncVersion *version, const char *modelversion);

/**
 * @brief Set Firmware Version
 * @param version The pointer to a version object
 * @param firmwareversion Firmware Version
 */
OSYNC_EXPORT void osync_version_set_firmwareversion(OSyncVersion *version, const char *firmwareversion);

/**
 * @brief Set Software Version
 * @param version The pointer to a version object
 * @param softwareversion Software Version
 */
OSYNC_EXPORT void osync_version_set_softwareversion(OSyncVersion *version, const char *softwareversion);

/**
 * @brief Set Hardware Version
 * @param version The pointer to a version object
 * @param hardwareversion Hardware Version
 */
OSYNC_EXPORT void osync_version_set_hardwareversion(OSyncVersion *version, const char *hardwareversion);

/**
 * @brief Set Identifier
 * @param version The pointer to a version object
 * @param identifier Identifier
 */
OSYNC_EXPORT void osync_version_set_identifier(OSyncVersion *version, const char *identifier);

/*@}*/
#endif /*OPENSYNC_VERSION_H_*/

