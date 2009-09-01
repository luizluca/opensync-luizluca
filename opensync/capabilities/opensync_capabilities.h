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
 * @defgroup OSyncCapabilities OpenSync Capabilities Module
 * @ingroup OSyncPublic
 * @defgroup OSyncCapabilitiesAPI OpenSync Capabilities
 * @ingroup OSyncCapabilities
 * @brief The public part of the OSyncCapabilities
 * 
 */
/*@{*/

/**
 * @brief Creates a new capabilities object
 * @param capsformat Name of the used capabilities format
 * @param error The error which will hold the info in case of an error
 * @return The pointer to the newly allocated capabilities object or NULL in case of error
 */
OSYNC_EXPORT OSyncCapabilities *osync_capabilities_new(const char *capsformat, OSyncError **error);

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
 * @brief Get the first capabilitiesobjtype for a given objtype from the capabilities
 * @param capabilities The pointer to a capabilities object
 * @param objtype The name of the objtype (e.g.: contact)
 * @return The capabilitiesobjtype for a given objtype from the capabilities
 */
OSYNC_EXPORT OSyncCapabilitiesObjType *osync_capabilities_get_objtype(OSyncCapabilities *capabilities, const char *objtype);

OSYNC_EXPORT const char *osync_capabilities_get_format(OSyncCapabilities *capabilities);

/*@}*/

#endif /*OPENSYNC_CAPABILITIES_H_*/
