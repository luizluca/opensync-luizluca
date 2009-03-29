/*
 * libopensync - A synchronization framework
 * Copyright (C) 2004-2006  Armin Bauer <armin.bauer@desscon.com>
 * Copyright (C) 2008       Daniel Gollub <dgollub@suse.de>
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


#ifndef _OPENSYNC_GROUP_INTERNALS_H_
#define _OPENSYNC_GROUP_INTERNALS_H_

#include "opensync/format/opensync_filter_internals.h"

/**
 * @defgroup OSyncGroupInternalAPI OpenSync Group Internals
 * @ingroup OSyncGroupPrivate
 * @brief The private API of opensync
 *
 * This gives you an insight in the private API of opensync.
 *
 */
/*@{*/

/** @brief Represent a group of members that should be synchronized */
struct OSyncGroup {
	/** The name of the group */
	char *name;
	/** The members of the group */
	OSyncList *members;
	/** The path, where the configuration resides */
	char *configdir;
	/** The last time this group was synchronized successfully */
	time_t last_sync;
	/** The lock file of the group */
	int lock_fd;
	/** The filters of this group */
	GList *filters;
	/** The defined resolution for this group */
	OSyncConflictResolution conflict_resolution;
	/** The winning side if the select resolution is choosen */
	int conflict_winner;
	/** The configured merger status of this group */
	osync_bool merger_enabled;
	/** The configured converter status of this group */
	osync_bool converter_enabled;

#ifdef OPENSYNC_UNITTESTS
	char *schemadir;
#endif /* OPENSYNC_UNITTESTS*/
	int ref_count;
};

/** @brief Sets the last synchronization date of this group
 * 
 * The information will be stored on disc after osync_group_save()
 * 
 * @param group The group in which to save
 * @param last_sync The time info to set
 */
OSYNC_TEST_EXPORT void osync_group_set_last_synchronization(OSyncGroup *group, time_t last_sync);

/** @brief Add a filter to the group 
 * 
 * @param group The group
 * @param filter The filter to add 
 * 
 */
OSYNC_TEST_EXPORT void osync_group_add_filter(OSyncGroup *group, OSyncFilter *filter);

/** @brief Remove a filter from the group 
 * 
 * @param group The group
 * @param filter The filter to remove
 * 
 */
OSYNC_TEST_EXPORT void osync_group_remove_filter(OSyncGroup *group, OSyncFilter *filter);

/** @brief Get the number of filters registered in a group
 * 
 * @param group The group
 * @returns The number of filters
 * 
 */
OSYNC_TEST_EXPORT unsigned int osync_group_num_filters(OSyncGroup *group);

/** @brief Gets the nth filter of a group
 * 
 * Note that you should not add or delete filters while
 * iterating over them
 * 
 * @param group The group
 * @param nth Which filter to return
 * @returns The filter or NULL if not found
 * 
 */
OSYNC_TEST_EXPORT OSyncFilter *osync_group_nth_filter(OSyncGroup *group, unsigned int nth);

/** @brief Gets list of all supported object types in this group, including
 * mixed object types
 *
 * So called "mixed object types" are object types which actually are able
 * to be represeented in each other object type.
 *
 * An example could be:
 *
 * Object Format: vcalendar Object Type: calendar
 * Object Format: vevent10 Object Type: event
 *
 * To detect "mixed object type" all potential conversion path get build.
 * For this is a loaded OSyncFormatEnv required.
 * 
 * @param group The group
 * @param formatenv Pointer to loaded OSyncFormatEnv
 * @returns List of supported object types, including mixed ones
 * 
 */
OSYNC_TEST_EXPORT OSyncList *osync_group_get_supported_objtypes_mixed(OSyncGroup *group, OSyncFormatEnv *formatenv);

/** @brief Get list  of all list of all used OSyncObjFormat elemets
 * 
 * Very complex/expensive!
 *
 * @param group The group
 * @returns The list of ObjFormat elements, or NULL
 */
OSYNC_TEST_EXPORT OSyncList *osync_group_get_objformats(OSyncGroup *group);

#ifdef OPENSYNC_UNITTESTS
/** @brief Set the schemadir for configuration validation to a custom directory.
 *  This is actually only inteded for UNITTESTS to run tests without
 *  having OpenSync installed.
 *
 * @param group Pointer to group
 * @param schemadir Custom schemadir path
 *
 */
OSYNC_TEST_EXPORT void osync_group_set_schemadir(OSyncGroup *group, const char *schemadir);
#endif /* OPENSYNC_UNITTESTS*/

/** @brief Returns the nth member of the group
 * 
 * Returns a pointer to the nth member of the group
 * 
 * @param group The group
 * @param nth Which member to return
 * @returns Pointer to the member
 * 
 */
OSYNC_TEST_EXPORT OSyncMember *osync_group_nth_member(OSyncGroup *group, unsigned int nth);

/** @brief Counts the members of the group
 * 
 * Returns the number of members in the group
 * 
 * @param group The group
 * @returns Number of members
 * 
 */
OSYNC_TEST_EXPORT unsigned int osync_group_num_members(OSyncGroup *group);

/** @brief Gets the number of object types of the group
 * 
 * @param group The group
 * @returns Number of object types of the group 
 * 
 */
OSYNC_TEST_EXPORT unsigned int osync_group_num_objtypes(OSyncGroup *group);

/** @brief Gets the nth object type of the group
 * 
 * @param group The group
 * @param nth The nth position of the object type list
 * @returns Name of the nth object type of the group
 * 
 */
OSYNC_TEST_EXPORT const char *osync_group_nth_objtype(OSyncGroup *group, unsigned int nth);


/*@}*/

#endif /* _OPENSYNC_GROUP_INTERNALS_H_ */

