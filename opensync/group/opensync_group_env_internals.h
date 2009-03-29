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

#ifndef _OPENSYNC_GROUP_ENV_INTERNALS_H_
#define _OPENSYNC_GROUP_ENV_INTERNALS_H_

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

/**
 * @defgroup OSyncGroupEnvInternalsAPI OpenSync Group Environment Internals
 * @ingroup OSyncGroupPrivate
 * @brief The private part of the OSyncGroupEnv
 * 
 */
/*@{*/
/** @brief Represent a environment of groups that should be synchronized */
struct OSyncGroupEnv {
	/** list of groups */
	OSyncList *groups;
	/** directory to store the groups */
	char *groupsdir;
	
	/** reference counter */
	int ref_count;
};

/** @brief Counts the groups in the environment
 * 
 * Returns the number of groups
 * 
 * @param env Pointer to a OSyncGroupEnv environment
 * @returns Number of groups
 * 
 */
OSYNC_TEST_EXPORT int osync_group_env_num_groups(OSyncGroupEnv *env);

/** @brief Returns the nth group
 * 
 * Returns the nth groups from the environment
 * 
 * @param env Pointer to a OSyncGroupEnv environment
 * @param nth Which group to return
 * @returns Pointer to the group
 * 
 */
OSYNC_TEST_EXPORT OSyncGroup *osync_group_env_nth_group(OSyncGroupEnv *env, int nth);


/*@}*/

#endif /* _OPENSYNC_GROUP_ENV_INTERNALS_H_ */
