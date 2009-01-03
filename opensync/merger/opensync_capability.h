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
 
#ifndef OPENSYNC_CAPABILITY_H_
#define OPENSYNC_CAPABILITY_H_

/**
 * @defgroup OSyncCapabilityAPI OpenSync Capability
 * @ingroup OSyncMerger
 * @brief The public part of the OSyncCapability
 * 
 */
/*@{*/

/**
 * @brief Creates a new capability object which will be added to the end of capabilities of the capabilities object.
 *  The returned object will be freed with the capabilities object.
 * @param capabilities The pointer to a capabilities object
 * @param objtype The name of the objtype (e.g.: contact)
 * @param name The name of the capability
 * @param error The error which will hold the info in case of an error
 * @return The pointer to the newly allocated capability object or NULL in case of error
 */
OSYNC_EXPORT OSyncCapability *osync_capability_new(OSyncCapabilities *capabilities, const char *objtype, const char *name, OSyncError **error);


/**
 * @brief Get the name of the capability
 * @param capability The pointer to a capability object
 * @return The name of the capability
 */
OSYNC_EXPORT const char *osync_capability_get_name(OSyncCapability *capability);

/**
 * @brief Get the next capability
 * @param capability The pointer to a capability object
 * @return The pointer to the next capability or NULL if there is no more capability
 */
OSYNC_EXPORT OSyncCapability *osync_capability_get_next(OSyncCapability *capability);

/**
 * @brief Get the previous capability
 * @param capability The pointer to a capability object
 * @return The pointer to the previous capability or NULL if there is no more capability
 */
OSYNC_EXPORT OSyncCapability *osync_capability_get_prev(OSyncCapability *capability);

/**
 * @brief Get the child capability
 * @param capability The pointer to a capability object
 * @return The pointer to the child capability or NULL if there is no more capability
 */
OSYNC_EXPORT OSyncCapability *osync_capability_get_child(OSyncCapability *capability);

/**
 * @brief Get the parent capability
 * @param capability The pointer to a capability object
 * @return The pointer to the parent capability or NULL if there is no more capability
 */
OSYNC_EXPORT OSyncCapability *osync_capability_get_parent(OSyncCapability *capability);

/**
 * @brief Check if the capability has a key
 * @param capability The pointer to a capability object
 * @return TRUE if the capability has a key otherwise FALSE
 */
OSYNC_EXPORT osync_bool osync_capability_has_key(OSyncCapability *capability);

/**
 * @brief Get the count of keys of a capability
 * @param capability The pointer to a capability object
 * @return The count of keys of the capability
 */
OSYNC_EXPORT int osync_capability_get_key_count(OSyncCapability *capability);

/**
 * @brief Get the name of the nth key of a capability
 * @param capability The pointer to a capability object
 * @param nth The number of the key
 * @return The name of the nth key of the capability or NULL in case of error
 */
OSYNC_EXPORT const char *osync_capability_get_nth_key(OSyncCapability *capability, int nth);

/**
 * @brief Add a key to a capability
 * @param capability The pointer to a capability object
 * @param name The name of the key
 */
OSYNC_EXPORT void osync_capability_add_key(OSyncCapability *capability, const char *name);

/*@}*/

#endif /*OPENSYNC_CAPABILITY_H_*/
