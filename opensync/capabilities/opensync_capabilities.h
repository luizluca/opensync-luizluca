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

#ifndef OPENSYNC_CAPABILITIES_H_
#define OPENSYNC_CAPABILITIES_H_


/**
 * @defgroup OSyncCapabilitiesAPI OpenSync Capabilities
 * @ingroup OSyncMerger
 * @brief The public part of the OSyncCapabilities
 * 
 */
/*@{*/

/**
 * @brief Creates a new capabilities object
 * @param error The error which will hold the info in case of an error
 * @return The pointer to the newly allocated capabilities object or NULL in case of error
 */
OSYNC_EXPORT OSyncCapabilities *osync_capabilities_new(OSyncError **error);

/**
 * @brief Creates a new capabilities object from an xml document. 
 * @param buffer The pointer to the xml document
 * @param size The size of the xml document
 * @param error The error which will hold the info in case of an error
 * @return The pointer to the newly allocated capabilities object or NULL in case of error
 */
OSYNC_EXPORT OSyncCapabilities *osync_capabilities_parse(const char *buffer, unsigned int size, OSyncError **error);

/**
 * @brief Increments the reference counter
 * @param capabilities The pointer to a capabilities object
 */
OSYNC_EXPORT OSyncCapabilities *osync_capabilities_ref(OSyncCapabilities *capabilities);

/**
 * @brief Decrement the reference counter. The object will 
 *  be freed if the reference count reaches zero.
 * @param capabilities The pointer to a capabilities object
 */
OSYNC_EXPORT void osync_capabilities_unref(OSyncCapabilities *capabilities);


/**
 * @brief Get the first capability for a given objtype from the capabilities
 * @param capabilities The pointer to a capabilities object
 * @param objtype The name of the objtype (e.g.: contact)
 * @return The first capability for a given objtype from the capabilities
 */
OSYNC_EXPORT OSyncCapability *osync_capabilities_get_first(OSyncCapabilities *capabilities, const char *objtype);

/**
 * @brief Dump the capabilities into memory.
 * @param capabilities The pointer to a capabilities object 
 * @param buffer The pointer to the buffer which will hold the xml document
 * @param size The pointer to the buffer which will hold the size of the xml document
 * @return The xml document and the size of it. It's up to the caller to free
 *  the buffer. Always it return TRUE.
 */
OSYNC_EXPORT osync_bool osync_capabilities_assemble(OSyncCapabilities *capabilities, char **buffer, int *size);

/*@}*/

#endif /*OPENSYNC_CAPABILITIES_H_*/
