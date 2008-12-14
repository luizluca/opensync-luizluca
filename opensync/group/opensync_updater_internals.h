/*
 * libopensync - A synchronization framework
 * Copyright (C) 2008 Daniel Gollub <dgollub@suse.de> 
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

#ifndef _OPENSYNC_UPDATER_INTERNALS_H
#define _OPENSYNC_UPDATER_INTERNALS_H 

/**
 * @defgroup OSyncPrivateUpdaterAPI OpenSync Updater Internals
 * @ingroup OSyncPrivate
 * @brief OpenSync update facilities
 * 
 */
/*@{*/

/** @brief Change version of group configuration. Only for testing.
 *
 * @param updater Pointer to the OSyncUpdater 
 * @param major Major Version number to set
 */
OSYNC_TEST_EXPORT void osync_updater_set_group_version(OSyncUpdater *updater, int major);

/** @brief Change version of member configuration. Only for testing.
 *
 * @param updater Pointer to the OSyncUpdater 
 * @param major Major Version number to set
 */
OSYNC_TEST_EXPORT void osync_updater_set_member_version(OSyncUpdater *updater, int major);

/** @brief Change version of plugin configuration. Only for testing.
 *
 * @param updater Pointer to the OSyncUpdater 
 * @param major Major Version number to set
 */
OSYNC_TEST_EXPORT void osync_updater_set_plugin_version(OSyncUpdater *updater, int major);

/*@}*/

#endif /*  _OPENSYNC_UPDATER_INTERNALS_H */

