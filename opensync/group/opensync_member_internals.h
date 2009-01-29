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
	long long int id;
	char *configdir;
	
	OSyncPluginConfig *config;
	
	//OSyncGroup *group;

	char *name;
	char *pluginname;
	
	GList *objtypes; /* OSyncObjTypeSink */
	OSyncObjTypeSink *main_sink;

	//For the filters
	GList *filters;
	int ref_count;
	
	OSyncCapabilities *capabilities;

#ifdef OPENSYNC_UNITTESTS
	char *schemadir;
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
 * @returns TRUE if member supports with the given OSyncFormatEnv the targetformat, FALSE otherwise
 * 
 */
OSYNC_TEST_EXPORT osync_bool osync_member_support_targetformat(OSyncMember *member, OSyncFormatEnv *formatenv, OSyncObjFormat *targetformat);

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
#endif

/*@}*/

#endif /* _OPENSYNC_MEMBER_INTERNALS_H_ */
