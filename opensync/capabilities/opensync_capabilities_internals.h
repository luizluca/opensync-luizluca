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

typedef struct OSyncCapabilitiesObjType OSyncCapabilitiesObjType;

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
 * @brief Load a capabilities object from a prepackaged file 
 * @param file The name of the file
 * @param error The error which will hold the info in case of an error
 * @return The pointer to the newly allocated capabilities object or NULL in case of error
 */
OSyncCapabilities *osync_capabilities_load(const char *file, OSyncError **error);

/**
 * @brief Checks if the capabilities are already cached 
 * @param member The member which should be tested for cached capabilities
 * @return TRUE if the capabilities for this member are cached otherwise FALSE
 */
osync_bool osync_capabilities_member_has_capabilities(OSyncMember *member);

/**
 * @brief Get the cached capabilities of a member. The cache capabilities is stored as
 *        "capabilities.xml" in the member directory. This function should be only used
 *        internal. To get the current capabilities of a member please use:
 *        osync_member_get_capabilities()
 *
 * @param member The pointer to a member object
 * @param error The error which will hold the info in case of an error
 * @return The objtype of the xmlformat
 */
OSyncCapabilities* osync_capabilities_member_get_capabilities(OSyncMember *member, OSyncError** error);

/**
 * @brief Set the capabilities of a member. The capabilities get cached in the member directory
 *        as "capabilities.xml". This function should be only used internal. To set member
 *        capabilities, please use:
 *        osync_member_set_capabilities()
 *
 * @param member The pointer to a member object
 * @param capabilities The pointer to a capabilities object
 * @param error The error which will hold the info in case of an error
 * @return TRUE on success otherwise FALSE
 */
osync_bool osync_capabilities_member_set_capabilities(OSyncMember *member, OSyncCapabilities* capabilities, OSyncError** error);

/*@}*/

#endif /*OPENSYNC_CAPABILITIES_INTERNAL_H_*/
