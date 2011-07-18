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
 
#ifndef OPENSYNC_CAPABILITIES_OBJTYPE_INTERNALS_H_
#define OPENSYNC_CAPABILITIES_OBJTYPE_INTERNALS_H_


/**
 * @defgroup OSyncCapabilitiesObjTypeInternalAPI OpenSync Capabilities Objtype Internals
 * @ingroup OSyncCapabilitiesObjTypePrivate
 * @brief The private part of the OSyncCapabilitiesObjType
 * 
 */
/*@{*/

/* Note: the new/ref/unref functions are internal, since OSyncCapabilities
 *       is intended to own all the memory and free it.  For the user
 *       API to capabilities, for fewer headaches, see opensync_capabilities.h.
 */

/**
 * @brief Creates a new capabilities object type object
 * @param error The error which will hold the info in case of an error
 * @return The pointer to the newly allocated capabilities object type object or NULL in case of error
 */
OSYNC_TEST_EXPORT OSyncCapabilitiesObjType *osync_capabilities_objtype_new(const char *objtype, OSyncError **error);

OSYNC_TEST_EXPORT OSyncCapabilitiesObjType *osync_capabilities_objtype_ref(OSyncCapabilitiesObjType *capsobjtype);
OSYNC_TEST_EXPORT void osync_capabilities_objtype_unref(OSyncCapabilitiesObjType *capsobjtype);

/**
 * @brief Creates a new capabilitiesobjtype object and adds it to the
 *        capabilities object.  The capabilities object owns a ref'd
 *        copy of the object.  The returned object you must still unref()
 *        as usual.
 * @param capabilities The pointer to a capabilities object
 * @param node The ObjType XML node
 * @param error The error which will hold the info in case of an error
 * @return The pointer to the newly allocated capabilitiesobjtype object or NULL in case of error
 */
OSYNC_TEST_EXPORT OSyncCapabilitiesObjType *osync_capabilities_objtype_parse_and_add(OSyncCapabilities *capabilities, xmlNode *node, OSyncError **error);

/**
 * @brief Get name of objtype from capabilities object type object
 * @param capabilities The pointer to a capabilities object type object
 * @return Name as string of the capabilities object type object
 */
OSYNC_TEST_EXPORT const char *osync_capabilities_objtype_get_name(OSyncCapabilitiesObjType *capsobjtype);


OSYNC_TEST_EXPORT osync_bool osync_capabilities_objtype_assemble(OSyncCapabilitiesObjType *capsobjtype, xmlNode *node, OSyncError **error);

OSYNC_TEST_EXPORT void osync_capabilities_objtype_add_capability(OSyncCapabilitiesObjType *capsobjtype, OSyncCapability *capability);

OSYNC_TEST_EXPORT void osync_capabilities_objtype_sort(OSyncCapabilitiesObjType *capsobjtype);

/*@}*/

#endif /*OPENSYNC_CAPABILITIES_OBJTYPE_INTERNAL_H_*/
