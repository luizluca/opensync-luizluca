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

#ifndef _OPENSYNC_UPDATER_H_
#define _OPENSYNC_UPDATER_H_

/**
 * @defgroup OSyncUpdaterAPI OpenSync Updater 
 * @ingroup OSyncPublic
 * @brief OpenSync configuration update facilities
 *
 * The OpenSync Updater applies the required changes to group/member/plugin 
 * configuration when the user upgrades to a newer version of OpenSync, using
 * XSLT stylesheets (libxslt / libexslt). To make configuration updates work
 * we just have to place some XSLT stylesheets into the OPENSYNC_UPDATESDIR.
 * 
 * Naming of update stylesheets looks like this:
 * 
 * - syncmember-$memberapi.xsl
 * - syncgroup-$groupapi.xsl
 * - $plugin-$pluginapi.xsl
 * 
 * This Updater interface should be used by any "rich" OpenSync frontend.
 * Frontends should call osync_updater_action_required() for each group. If 
 * the return value is TRUE, the user should be notified that an update of 
 * the configuration is required. Later the update process gets started by
 * calling:
 * 
 * - (non blocking) osync_updater_process()
 * - (blocking) osync_updater_process_and_block()
 * 
 * The update process works like this:
 * 
 * -# Lock group
 * -# Check version of group configuration
 * -# Create group backup
 * -# Process Member configurations
 * -# Process Group configuration
 * -# Unlock group
 * 
 * If the update process fails the backup gets restored. To avoid any
 * data loss, the backup will not be deleted an can be found in
 * ~/.opensync/groupX.bak(.bak.bak.bak.bak)
 *
 */
/*@{*/

typedef struct OSyncUpdaterStatus OSyncUpdaterStatus; 
typedef struct OSyncUpdater OSyncUpdater;

typedef void (* osync_updater_cb) (OSyncUpdater *updater, OSyncUpdaterStatus *status);

/** @brief Registers a new updater
 *
 * @param group Pointer to the OSyncGroup to update
 * @param error Pointer to an error struct
 * @returns the newly created updater
 */
OSYNC_EXPORT OSyncUpdater *osync_updater_new(OSyncGroup *group, OSyncError **error);

/** @brief Decrease the reference count on a updater 
 * 
 * @param updater Pointer to the updater 
 * 
 */
OSYNC_EXPORT void osync_updater_unref(OSyncUpdater *updater);

/** @brief Increase the reference count on an updater
 * 
 * @param updater Pointer to the updater
 * 
 */
OSYNC_EXPORT OSyncUpdater *osync_updater_ref(OSyncUpdater *updater);


/** @brief Register OSyncUpdater callback
 * 
 * @param updater Pointer to the updater 
 * @param callback The callback function to be called on updater events 
 * 
 */
OSYNC_EXPORT void osync_updater_set_callback(OSyncUpdater *updater, osync_updater_cb callback);


/** @brief Check if an update is required for the group
 * 
 * @param updater Pointer to the updater 
 * @returns TRUE if update is required on that group, FALSE if no update is required.
 * 
 */
OSYNC_EXPORT osync_bool osync_updater_action_required(OSyncUpdater *updater);


/** @brief Process update on the group. This function is not blocking. 
 * 
 * @param updater Pointer to the updater 
 * @param error Pointer to OSyncError
 * @returns TRUE if the updater process started successfully, FALSE on error.
 * 
 */
OSYNC_EXPORT osync_bool osync_updater_process(OSyncUpdater *updater, OSyncError **error);

/** @brief Process update on the group. This function is blocking. 
 * 
 * @param updater Pointer to the updater 
 * @param error Pointer to OSyncError
 * @returns TRUE if the updater process was successful, FALSE on error.
 * 
 */
OSYNC_EXPORT osync_bool osync_updater_process_and_block(OSyncUpdater *updater, OSyncError **error);


/** @brief Set path of Updates directory, which inlcudes the update stylesheets. 
 * 
 * @param updater Pointer to the updater 
 * @param path Path to updates directory 
 * 
 */
OSYNC_EXPORT void osync_updater_set_updates_directory(OSyncUpdater *updater, const char *path);

/** @brief Get path of Updates directory. 
 * 
 * @param updater Pointer to the updater 
 * @returns Path of Updates directory.
 * 
 */
OSYNC_EXPORT const char *osync_updater_get_updates_directory(OSyncUpdater *updater);

/*@}*/

#endif /* _OPENSYNC_UPDATER_H_ */
