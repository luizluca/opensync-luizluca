/*
 * libopensync - A synchronization framework
 * Copyright (C) 2004-2005  Armin Bauer <armin.bauer@opensync.org>
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
 */

#ifndef _OPENSYNC_MEMBER_INTERNALS_H_
#define _OPENSYNC_MEMBER_INTERNALS_H_

/**
 * @defgroup OSyncMemberInternalsAPI OpenSync Member Internals
 * @ingroup OSyncGroupPrivate
 * @brief The private part of the OSyncMember
 * 
 */
/*@{*/

/** @brief A member of a group which represent a single device */
struct OSyncMember {
	osync_memberid id;
	char *configdir;
	
	OSyncPluginConfig *config;
	
	//OSyncGroup *group;

	char *name;
	char *pluginname;

	GHashTable *alternative_objtype_table;
	
	GList *objtypes; /* OSyncObjTypeSink */
	OSyncObjTypeSink *main_sink;

	//For the filters
	GList *filters;
	int ref_count;
	
	OSyncCapabilities *capabilities;

#ifdef OPENSYNC_UNITTESTS
	/** Modify schema directory for unittesting */
	char *schemadir;

	/** Modify default configuration direcotry for unittesting */
	char *default_configdir;
#endif /* OPENSYNC_UNITTESTS */
};

/** @brief Get list of all OSyncObjFormatSink elements
 * 
 * @param member Pointer to member 
 * @returns Pointer to OSyncList with element of OSyncObjformatSink, or NULL
 * 
 */
OSYNC_TEST_EXPORT OSyncList *osync_member_get_objformat_sinks_all(OSyncMember *member);

/** @brief Get names of all object formats of this member
 * 
 * @param member Pointer to member 
 * @returns Pointer to OSyncList with strings of object formats, or NULL
 * 
 */
OSYNC_TEST_EXPORT OSyncList *osync_member_get_all_objformats(OSyncMember *member);

/** @brief Check if a member supports, with the help of various conversions,
 *  a specific format
 *
 *  To determine if a target format is supported by this member, a loaded
 *  OSyncFormatEnv is required.
 * 
 * @param member Pointer to member 
 * @param formatenv Pointer to a loaded OSyncFormatEnv for conversion path building
 * @param targetformat The target format to test
 * @returns Source OSyncObjFormat which can convert to targetformat, NULL if not supported
 * 
 */
OSYNC_TEST_EXPORT OSyncObjFormat *osync_member_support_targetformat(OSyncMember *member, OSyncFormatEnv *formatenv, OSyncObjFormat *targetformat);

/** @brief Get the alternative Object Type to a provided Object Type
 *
 * The alternative Object Type is the Object Type which could be handled by
 * this member due to format-conversion and mixed-objtype syncing.
 * 
 * @param member Pointer to member 
 * @param orig_objtype The original object type to look for an alternative for
 * @returns The name of the alternative object type or NULL if there is no alternative for this member
 * 
 */
OSYNC_TEST_EXPORT const char *osync_member_get_alternative_objtype(OSyncMember *member, const char *orig_objtype);

/** @brief Add an alternative object type for this member
 *
 * @param member Pointer to member 
 * @param native_objtype The native object type of this member which can reprsent the alternative
 * @param alternative_objtype The alternative object type t add
 * 
 */
OSYNC_TEST_EXPORT void osync_member_add_alternative_objtype(OSyncMember *member, const char *native_objtype, const char *alternative_objtype);

#ifdef OPENSYNC_UNITTESTS
/** @brief Set the schemadir for configuration validation to a custom directory.
 *  This is actually only inteded for UNITTESTS to run tests without 
 *  having OpenSync installed.
 * 
 * @param member Pointer to member 
 * @param schemadir Custom schemadir path
 * 
 */
OSYNC_TEST_EXPORT void osync_member_set_schemadir(OSyncMember *member, const char *schemadir);


/** @brief Set the default configdir to a custom directory.
 *  This is actually only inteded for UNITTESTS to run tests without
 *  having OpenSync installed.
 *
 * @param group Pointer to group
 * @param schemadir Custom "default" configdir path
 *
 */
OSYNC_TEST_EXPORT void osync_member_set_default_configdir(OSyncMember *member, const char *default_configdir);
#endif

/** @brief Get the number of supported object types of this member
 * 
 * @param member The member pointer
 * @returns Number of supported object type of this member
 * 
 */
OSYNC_TEST_EXPORT int osync_member_num_objtypes(OSyncMember *member);

/** @brief Get the name of the nth supported object type of this member
 * 
 * @param member The member pointer
 * @param nth The nth position of the list of supported object types of this member
 * @returns Name of the nth supported object type
 * 
 */
OSYNC_TEST_EXPORT const char *osync_member_nth_objtype(OSyncMember *member, int nth);

/**
 * @brief Checks if the capabilities are already cached 
 * @param member The member which should be tested for cached capabilities
 * @return TRUE if the capabilities for this member are cached otherwise FALSE
 */
OSYNC_TEST_EXPORT osync_bool osync_member_has_capabilities(OSyncMember *member);

/**
 * @brief Load the cached capabilities of a member. The cache capabilities is stored as
 *        "capabilities.xml" in the member directory. This function should be only used
 *        internal. To get the current capabilities of a member please use:
 *        osync_member_get_capabilities()
 *
 * @param member The pointer to a member object
 * @param error The error which will hold the info in case of an error
 * @return The objtype of the xmlformat
 */
OSYNC_TEST_EXPORT OSyncCapabilities* osync_member_load_capabilities(OSyncMember *member, OSyncError** error);

/**
 * @brief Save the capabilities of a member. The capabilities get cached in the member directory
 *        as "capabilities.xml". This function should be only used internal. To set member
 *        capabilities, please use:
 *        osync_member_set_capabilities()
 *
 * @param member The pointer to a member object
 * @param capabilities The pointer to a capabilities object
 * @param error The error which will hold the info in case of an error
 * @return TRUE on success otherwise FALSE
 */
OSYNC_TEST_EXPORT osync_bool osync_member_save_capabilities(OSyncMember *member, OSyncCapabilities* capabilities, OSyncError** error);

/*@}*/

#endif /* _OPENSYNC_MEMBER_INTERNALS_H_ */
