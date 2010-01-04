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
 * @ingroup OSyncCapabilities
 * @brief The public part of the OSyncCapability
 * 
 */
/*@{*/

/** @brief Capability value types
 * 
 **/
typedef enum {
	/** None */
	OSYNC_CAPABILITY_TYPE_NONE = 0,
	/** bool */
	OSYNC_CAPABILITY_TYPE_BOOL,
	/** char */
	OSYNC_CAPABILITY_TYPE_CHAR,
	/** double */
	OSYNC_CAPABILITY_TYPE_DOUBLE,
	/** int */
	OSYNC_CAPABILITY_TYPE_INT,
	/** long */
	OSYNC_CAPABILITY_TYPE_LONG,
	/** long long */
	OSYNC_CAPABILITY_TYPE_LONGLONG,
	/** unsigned int */
	OSYNC_CAPABILITY_TYPE_UINT,
	/** unsigned long */
	OSYNC_CAPABILITY_TYPE_ULONG,
	/** unsigned long long */
	OSYNC_CAPABILITY_TYPE_ULONGLONG,
	/** String (char *) */
	OSYNC_CAPABILITY_TYPE_STRING
} OSyncCapabilityType;

/**
 * @brief Creates a new capability object which will be added to the end of capabilities of the capabilities object.
 *  The returned object will be freed with the capabilities object.
 * @param capobjtype The pointer to a capabilities objtype object
 * @param error The error which will hold the info in case of an error
 * @return The pointer to the newly allocated capability object or NULL in case of error
 */
OSYNC_EXPORT OSyncCapability *osync_capability_new(OSyncCapabilitiesObjType* capobjtype, OSyncError **error);

OSYNC_EXPORT OSyncCapability *osync_capability_ref(OSyncCapability *capability);
OSYNC_EXPORT void osync_capability_unref(OSyncCapability *capability);

/**
 * @brief Get the name of the capability
 * @param capability The pointer to a capability object
 * @return The name of the capability
 */
OSYNC_EXPORT const char *osync_capability_get_name(OSyncCapability *capability);

/**
 * @brief Set the name of the capability
 * @param capability The pointer to a capability object
 * @param name The name of the capability to set 
 */
OSYNC_EXPORT void osync_capability_set_name(OSyncCapability *capability, const char *name);

/**
 * @brief Get the display name of the capability
 * @param capability The pointer to a capability object
 * @return The display name of the capability
 */
OSYNC_EXPORT const char *osync_capability_get_displayname(OSyncCapability *capability);

/**
 * @brief Set the display name of the capability
 * @param capability The pointer to a capability object
 * @param displayname The displayname of the capability to set
 */
OSYNC_EXPORT void osync_capability_set_displayname(OSyncCapability *capability, const char *displayname);

/**
 * @brief Get max occurs of the capability
 * @param capability The pointer to a capability object
 * @return The number of max occurs of the capability 
 */
OSYNC_EXPORT unsigned int osync_capability_get_maxoccurs(OSyncCapability *capability);

/**
 * @brief Set max occurs of the capability
 * @param capability The pointer to a capability object
 * @param maxoccurs The number of max occurs of the capability object
 */
OSYNC_EXPORT void osync_capability_set_maxoccurs(OSyncCapability *capability, unsigned int maxoccurs);

/**
 * @brief Get max length/size of the capability value 
 * @param capability The pointer to a capability object
 * @return The max length/size of the capability value
 */
OSYNC_EXPORT unsigned int osync_capability_get_max(OSyncCapability *capability);

/**
 * @brief Set the max length/size of the capability value
 * @param capability The pointer to a capability object
 * @param max The max lenth/size of the capability value
 */
OSYNC_EXPORT void osync_capability_set_max(OSyncCapability *capability, unsigned int max);

/**
 * @brief Get min length/size of the capability value 
 * @param capability The pointer to a capability object
 * @return The min length/size of the capability value
 */
OSYNC_EXPORT unsigned int osync_capability_get_min(OSyncCapability *capability);

/**
 * @brief Set the min length/size of the capability value
 * @param capability The pointer to a capability object
 * @param min The min lenth/size of the capability value
 */
OSYNC_EXPORT void osync_capability_set_min(OSyncCapability *capability, unsigned int min);

/**
 * @brief Get capability parameter of this capability object
 * @param capability The pointer to a capability object
 * @return Pointer to the capability parameter of this capability object
 */
OSYNC_EXPORT OSyncCapabilityParameter *osync_capability_get_parameter(OSyncCapability *capability) OSYNC_DEPRECATED;

/**
 * @brief Set the capability parameter of the capability object
 * @param capability The pointer to a capability object
 * @param parameter the parameter to set for the capability object
 */
OSYNC_EXPORT void osync_capability_set_parameter(OSyncCapability *capability, OSyncCapabilityParameter *parameter) OSYNC_DEPRECATED;

/**
 * @brief Add a capability parameter to the capability object
 * @param capability The pointer to a capability object
 * @param parameter the parameter to add to the capability object
 */
OSYNC_EXPORT void osync_capability_add_parameter(OSyncCapability *capability, OSyncCapabilityParameter *parameter);

/**
 * @brief Get capability type of this capability object 
 * @param capability The pointer to a capability object
 * @return Type of this capability
 */
OSYNC_EXPORT OSyncCapabilityType osync_capability_get_type(OSyncCapability *capability);

/**
 * @brief Set capability type for capability object
 * @param capability The pointer to a capability object
 * @param type The type to set for the capability object
 */
OSYNC_EXPORT void osync_capability_set_type(OSyncCapability *capability, OSyncCapabilityType type);

/**
 * @brief Get valenum of this capability objects. Contains possible value of
 * this capability object
 *
 * @param capability The pointer to a capability object
 * @return Returns OSyncList with strings (const char*) with possible values
 * of this capability object
 */
OSYNC_EXPORT OSyncList *osync_capability_get_valenums(OSyncCapability *capability); /* const char* list */

/** TODO Doxygen */
OSYNC_EXPORT OSyncList *osync_capability_get_childs(OSyncCapability *capability); /* OSyncCapability* list */
OSYNC_EXPORT void osync_capability_add_child(OSyncCapability *capability, OSyncCapability *child);
OSYNC_EXPORT OSyncCapability *osync_capability_new_child(OSyncCapability *parent, OSyncError **error);

/*@}*/

#endif /*OPENSYNC_CAPABILITY_H_*/
