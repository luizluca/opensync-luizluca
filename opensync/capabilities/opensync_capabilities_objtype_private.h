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

#ifndef OPENSYNC_CAPABILITIES_OBJTYPE_PRIVATE_H_
#define OPENSYNC_CAPABILITIES_OBJTYPE_PRIVATE_H_

/**
 * @defgroup OSyncCapabilitiesPrivate OpenSync Capabilities Module Private
 * @ingroup OSyncPrivate
 * @defgroup OSyncCapabilitiesPrivateAPI OpenSync Capabilities Private
 * @ingroup OSyncCapabilitiesPrivate
 * @brief Private part of OpenSync Capabilities
 */

/*@{*/

/**
 * @brief Represent a CapabilitiesObjType object
 */
struct OSyncCapabilitiesObjType {
	/** The reference counter for this object */
	int ref_count;
	/** Name of ObjType */
	char *name;
	/** Capability List */
	OSyncList *capabilities; /* OSyncCapapbility */

};

/*@}*/

#endif /*OPENSYNC_CAPABILITIES_OBJTYPE_PRIVATE_H_*/
