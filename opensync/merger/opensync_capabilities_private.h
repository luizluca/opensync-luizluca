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

#ifndef OPENSYNC_CAPABILITIES_PRIVATE_H_
#define OPENSYNC_CAPABILITIES_PRIVATE_H_

/**
 * @defgroup OSyncCapabilitiesPrivateAPI OpenSync Capabilities Private
 * @ingroup OSyncMergerPrivate
 * @brief Private part of OpenSync Capabilities
 */

/*@{*/

/**
 * @brief Represent a CapabilitiesObjType object
 */
struct OSyncCapabilitiesObjType {
	/** The pointer to the next objtype */
	OSyncCapabilitiesObjType *next;
	/** The pointer to the root capability */
	OSyncCapability *capability;
	/** The pointer to the first capability */
	OSyncCapability *first_child;
	/** The pointer to the last capability */
	OSyncCapability *last_child;
	/** Counter which holds the number of capabilities for one objtype*/
	int child_count;
	/** The wrapped xml node */
	xmlNodePtr node;	
};

/**
 * @brief Represent a Capabilities object
 */
struct OSyncCapabilities {
	/** The reference counter for this object */
	int ref_count;
	/** The pointer to the first objtype */
	OSyncCapabilitiesObjType *first_objtype;
	/** The pointer to the last objtype */
	OSyncCapabilitiesObjType *last_objtype;
	/** The wrapped xml document */
	xmlDocPtr doc;
};

/**
 * @brief Creates a new capabilitiesobjtype object which will be added to the end of capabilitiesobjtype of the capabilities object.
 *  The returned object will be freed with the capabilities object. 
 * @param capabilities The pointer to a capabilities object
 * @param node The node must be already inserted at the end of childs of the xmlDoc root element
 * @param error The error which will hold the info in case of an error
 * @return The pointer to the newly allocated capabilitiesobjtype object or NULL in case of error
 */
OSyncCapabilitiesObjType *osync_capabilitiesobjtype_new(OSyncCapabilities *capabilities, xmlNodePtr node, OSyncError **error);

/**
 * @brief Get the first capabilitiesobjtype for a given objtype from the capabilities
 * @param capabilities The pointer to a capabilities object
 * @param objtype The name of the objtype (e.g.: contact)
 * @return The capabilitiesobjtype for a given objtype from the capabilities
 */
OSyncCapabilitiesObjType *osync_capabilitiesobjtype_get(OSyncCapabilities *capabilities, const char *objtype);

/*@}*/

#endif /*OPENSYNC_CAPABILITIES_PRIVATE_H_*/
