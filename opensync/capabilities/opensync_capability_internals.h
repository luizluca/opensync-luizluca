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

/**
 * @brief Creates a new capability object which will be added to the end of capabilities of the capabilities object.
 *  The returned object will be freed with the capabilities object.
 * @param objtype The pointer to a capabilitiesobjtype object
 * @param node The node must be already inserted at the end of childs of the objtype xml node
 * @param error The error which will hold the info in case of an error
 * @return The pointer to the newly allocated capability object or NULL in case of error
 */
OSyncCapability *osync_capability_new_node(OSyncCapabilitiesObjType *objtype, xmlNodePtr node, OSyncError **error);

/**
 * @brief Multi-level parsing of OSyncCapability based on a given XML Node.
 * 
 * This function create a Capability-tree out of the given xmlNode pointer. This function
 * is calling it self in a recursion and parses all siblings and child of the given xmlNode pointer.
 *
 * @param parent The parent OSyncCapability 
 * @param node The node to start parsing the capabilities. 
 * @param first_child Reference to a pointer to store the first parsed OSyncCapability child 
 * @param last_child Reference to a pointer to store the last parsed OSyncCapability child 
 * @param child_count Reference to unsinged int to count the childs of the capability node
 * @param error The error which will hold the info in case of an error
 * @returns TRUE on success, FALSE on error
 */
osync_bool osync_capability_parse(OSyncCapability *parent, xmlNodePtr node, OSyncCapability **first_child, OSyncCapability **last_child, unsigned int *child_count, OSyncError **error);

/**
 * @brief Frees a capability object 
 * @param capability The pointer to a capability object
 */
void osync_capability_free(OSyncCapability *capability);

/**
 * @brief Compare the names of two capabilities
 * @param capability1 The pointer to a capability object
 * @param capability2 The pointer to a capability object
 */
int osync_capability_compare_stdlib(const void *capability1, const void *capability2);

/*@}*/

#endif /*OPENSYNC_CAPABILITY_INTERNALS_H_*/
