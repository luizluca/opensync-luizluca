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
 
#ifndef OPENSYNC_CAPABILITY_INTERNALS_H_
#define OPENSYNC_CAPABILITY_INTERNALS_H_

/**
 * @defgroup OSyncCapabilityInternalAPI OpenSync Capability Internals
 * @ingroup OSyncCapabilitiesPrivate
 * @brief The internal part of the OpenSync Capability
 * 
 */
/*@{*/

OSyncCapabilityParameter *osync_capability_param_new(OSyncError **error);
OSyncCapabilityParameter *osync_capability_param_ref(OSyncCapabilityParameter *capparam);
void osync_capability_param_unref(OSyncCapabilityParameter *capparam);

/**
 * @brief Creates a new capability object which will be added to the end of capabilities of the capabilities object.
 *  The returned object is owned by the OSyncCapabilitiesObjType objtype and
 *  will be unref'd with it.  You still need to unref the returned pointer,
 *  though.
 * @param objtype The pointer to a capabilitiesobjtype object
 * @param node The node must be already inserted at the end of childs of the objtype xml node
 * @param error The error which will hold the info in case of an error
 * @return The pointer to the newly allocated capability object or NULL in case of error
 */
OSyncCapability *osync_capability_parse_and_add(OSyncCapabilitiesObjType *objtype, xmlNodePtr node, OSyncError **error);


/* TODO - Doxygen */
osync_bool osync_capability_assemble(OSyncCapability *cap, xmlNodePtr node, OSyncError **error);

/**
 * @brief Compare the names of two capabilities (for stdlib)
 *
 * 1st and 2nd pointer get dereferneced, to fit the needs of e.g. qsort()
 *
 * @param capability1 The pointer to a capability object
 * @param capability2 The pointer to a capability object
 */
int osync_capability_compare_stdlib(const void *capability1, const void *capability2);

/**
 * @brief Compare the names of two capabilities
 * @param capability1 The pointer to a capability object
 * @param capability2 The pointer to a capability object
 */
int osync_capability_compare(const void *capability1, const void *capability2);

/*@}*/

#endif /*OPENSYNC_CAPABILITY_INTERNALS_H_*/
