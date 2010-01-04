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
 
#ifndef OPENSYNC_CAPABILITY_PRIVATE_H_
#define OPENSYNC_CAPABILITY_PRIVATE_H_

/**
 * @defgroup OSyncCapabilityPrivateAPI OpenSync Capability Private
 * @ingroup OSyncCapabilitiesPrivate
 * @brief Private part of OpenSync Capability
 */

/*@{*/

/**
 * @brief Represents a Capability object
 */
struct OSyncCapability {
	/** CapabilityObjType parent object */
	OSyncCapabilitiesObjType *capobjtype;
	/** DisplayName */
	char *displayname;
	/** MaxOccurs */
	unsigned int maxoccurs;
	/** Max */
	unsigned int max;
	/** Min */
	unsigned int min;
	/** Name */
	char *name;
	/** Parameter */
	OSyncList *parameters; /* OSyncCapabilityParameter * list */
	/** Type */
	OSyncCapabilityType type;
	/** ValEnum */
	OSyncList *valenum; /* char* */
	/** Value */
	char *value;

	/** Childs */
	OSyncList *childs; /* OSYncCapability * list */


	/** Reference counting */
	int ref_count;
};

struct OSyncCapabilityParameter {
	/** DisplayName */
	char *displayname;
	/** Name */
	char *name;
	/** Type */
	OSyncCapabilityType type;
	/** ValEnum */
	OSyncList *valenum; /* char* */
	/** Value */
	char *value;

	/** Reference counting */
	int ref_count;
};


/*@}*/

#endif /* OPENSYNC_CAPABILITY_PRIVATE_H_ */

