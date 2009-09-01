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
 
#ifndef OPENSYNC_CAPABILITIES_INTERNALS_H_
#define OPENSYNC_CAPABILITIES_INTERNALS_H_


/**
 * @defgroup OSyncCapabilitiesInternalAPI OpenSync Capabilities Internals
 * @ingroup OSyncCapabilitiesPrivate
 * @brief The private part of the OSyncCapabilities
 * 
 */
/*@{*/

/**
 * @brief Sort all the capabilities of every objtype of the capabilities object. This function has to
 *  be called after a capability was added to the capabilities.
 * @param capabilities The pointer to a capabilities object
 */
OSYNC_TEST_EXPORT void osync_capabilities_sort(OSyncCapabilities *capabilities);

/**
 * @brief Save a capabilities object in a file 
 * @param capabilities The pointer to a capabilities object
 * @param file The name of the file
 * @param error The error which will hold the info in case of an error
 * @return TRUE on success, FALSE otherwise 
 */
OSYNC_TEST_EXPORT osync_bool osync_capabilities_save(OSyncCapabilities *capabilities, const char *file, OSyncError **error);

/**
 * @brief Load a capabilities object from a file 
 * @param file The name of the file
 * @param error The error which will hold the info in case of an error
 * @return The pointer to the newly allocated capabilities object or NULL in case of error
 */
OSYNC_TEST_EXPORT OSyncCapabilities *osync_capabilities_load(const char *file, OSyncError **error);

/**
 * @brief Load a capabilities object from a prepackaged file 
 *
 *  The location of the prepackaged file is: OPENSYNC_CAPABILITIESDIR/$file
 *
 * @param file The name of the file
 * @param error The error which will hold the info in case of an error
 * @return The pointer to the newly allocated capabilities object or NULL in case of error
 */
OSYNC_TEST_EXPORT OSyncCapabilities *osync_capabilities_load_identifier(const char *file, OSyncError **error);

/**
 * @brief Dump the capabilities into memory.
 * @param capabilities The pointer to a capabilities object 
 * @param error The error which will hold the info in case of an error
 * @return The xml document and the size of it. It's up to the caller to free
 *  the buffer. Always it return TRUE.
 */
OSYNC_TEST_EXPORT osync_bool osync_capabilities_assemble(OSyncCapabilities *capabilities, char **buffer, unsigned int *size, OSyncError **error);

/**
 * @brief Creates a new capabilities object from an xml document. 
 * @param buffer The pointer to the xml document
 * @param size The size of the xml document
 * @param error The error which will hold the info in case of an error
 * @return The pointer to the newly allocated capabilities object or NULL in case of error
 */
OSYNC_TEST_EXPORT OSyncCapabilities *osync_capabilities_parse(const char *buffer, unsigned int size, OSyncError **error);

/*@}*/


OSYNC_TEST_EXPORT void osync_capabilities_add_objtype(OSyncCapabilities *capabilities, OSyncCapabilitiesObjType *capsobjtype);

OSYNC_TEST_EXPORT void osync_capabilities_set_format(OSyncCapabilities *capabilities, const char *capsformat);

OSYNC_TEST_EXPORT OSyncCapabilityParameter *osync_capability_parameter_new(OSyncError **error);
OSYNC_TEST_EXPORT OSyncCapabilityParameter *osync_capability_parameter_ref(OSyncCapabilityParameter *capparam);
OSYNC_TEST_EXPORT void osync_capability_parameter_unref(OSyncCapabilityParameter *capparam);

#endif /*OPENSYNC_CAPABILITIES_INTERNAL_H_*/
