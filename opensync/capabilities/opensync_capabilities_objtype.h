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
 
#ifndef OPENSYNC_CAPABILITIES_OBJTYPE_H_
#define OPENSYNC_CAPABILITIES_OBJTYPE_H_


/**
 * @defgroup OSyncCapabilitiesObjTypeAPI OpenSync Capabilities Objtypes
 * @ingroup OSyncCapabilitiesObjTypePrivate
 * @brief The private part of the OSyncCapabilitiesObjType
 * 
 */
/*@{*/


/**
 * @brief Creates a new capabilities object type object
 * @param error The error which will hold the info in case of an error
 * @return The pointer to the newly allocated capabilities object type object or NULL in case of error
 */

OSYNC_EXPORT OSyncCapabilitiesObjType *osync_capabilities_objtype_new(OSyncCapabilities *capabilities, const char *objtype, OSyncError **error);

OSYNC_EXPORT OSyncCapabilitiesObjType *osync_capabilities_objtype_ref(OSyncCapabilitiesObjType *capsobjtype);
OSYNC_EXPORT void osync_capabilities_objtype_unref(OSyncCapabilitiesObjType *capsobjtype);

OSYNC_EXPORT OSyncList *osync_capabilities_objtype_get_caps(OSyncCapabilitiesObjType *capsobjtype);

/*@}*/

#endif /*OPENSYNC_CAPABILITIES_OBJTYPE_H_*/
