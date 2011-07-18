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
 * @defgroup OSyncCapabilitiesPrivate OpenSync Capabilities Module Private
 * @ingroup OSyncPrivate
 * @defgroup OSyncCapabilitiesPrivateAPI OpenSync Capabilities Private
 * @ingroup OSyncCapabilitiesPrivate
 * @brief Private part of OpenSync Capabilities
 */

/*@{*/

/**
 * @brief Represent a Capabilities object
 *
 * This is the top level Capabilities object, which contains a list of
 * lists of capability objects for each objtype.
 *
 * For example, if your device has a set of capabilities, you could call the
 * set "foodevice-caps".  This would be the Capabilities format name.
 * This format name could contain different capability lists for each
 * objtype supported, for example "contact" or "event" or both.
 *
 * The names of the capabilities do not have to match anything else, but
 * if they do not match something opensync knows about, it will need a
 * caps-converter (capabilities converter) to translate your list of
 * capability objects for, say, "contact" into something it knows, say,
 * "xmlformat-contact".
 *
 *   foodevice-caps
 *      contact
 *         name
 *         address1
 *         phonenumber
 *      event
 *         date
 *         location
 *
 * I don't know if multiple objtypes are ever useful, but they are possible,
 * given the 3 structures: OSyncCapabilities, OSyncCapabilitiesObjType,
 * and finally OSyncCapability.
 *
 * Note that OSyncCapability is a hierarchical list, having its own children.
 * It also has a list of OSyncCapabilityParameter objects.
 *
 */
struct OSyncCapabilities {
	/** The reference counter for this object */
	int ref_count;
	/** Capabilities Format */
	char *format;
	/** ObjTypes **/
	OSyncList *objtypes; /* OSyncCapabilitiesObjType */
	/** The wrapped xml document */
	xmlDocPtr doc;
};

/*@}*/

#endif /*OPENSYNC_CAPABILITIES_PRIVATE_H_*/
