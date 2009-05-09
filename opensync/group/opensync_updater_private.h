/*
 * libopensync - A synchronization framework
 * Copyright (C) 2008 Daniel Gollub <gollub@b1-systems.de> 
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

#ifndef _OPENSYNC_UPDATER_PRIVATE_H
#define _OPENSYNC_UPDATER_PRIVATE_H 

/**
 * @defgroup OSyncUpdaterprivateAPI OpenSync Updater Private
 * @ingroup OSyncGroupPrivate
 * @brief OpenSync update facilities
 * 
 */
/*@{*/

#define OSYNC_UPDATER_SUFFIX ".xsl"

/** @brief Represents an update event */
typedef enum {
	OSYNC_UPDATER_UNKOWN,
	OSYNC_UPDATER_PROCESSING_MEMBER_CONFIG,
	OSYNC_UPDATER_PROCESSING_MEMBER_DATABASE,
	OSYNC_UPDATER_PROCESSING_GROUP_CONFIG,
	OSYNC_UPDATER_PROCESSING_GROUP_DATABASE,
	OSYNC_UPDATER_NUM
} OSyncUpdaterEvent; 

/** @brief Represents an OpenSync Updater */
struct OSyncUpdater {
	/* reference counter for OSyncUpdater */
	int ref_count;

	/* Error stack for errors during update process */
	OSyncError *error;

	/* The OSyncGroup which are handled */
	OSyncGroup *group;

	/* Updates Directory (by default: OPENSYNC_UPDATESDIR) */
	char *updatesdir;

	/* OSyncUpdater status callback */
	osync_updater_cb status_callback;

	GCond *updating;
	GMutex *updating_mutex;

	OSyncThread *thread;
	GMainContext *context;

	int member_version;
	int group_version;
	int plugin_version;

};

/** @brief Represents the status of an update */
struct OSyncUpdaterStatus {
	/** The type of the status update */
	OSyncUpdaterEvent type;
	/** The member for which the status update is, on NULL it's about the group */
	OSyncMember *member;
	/** If the status was a error, this error will be set */
	OSyncError *error;
};

/** @brief Stacks errors on top of the Updater error stack.
 *
 * @param updater Pointer to the OSyncUpdater 
 * @param error Pointer to OSyncError which get stacked
 * @returns return code of thread as pointer 
 */
static void osync_updater_set_error(OSyncUpdater *updater, OSyncError *error);

/** @brief Apply stylesheet on configuration.
 *
 * @param updater Pointer to the OSyncUpdater 
 * @param config Path to configuration which gets processed 
 * @param stylesheet Path to stylesheet which gets invoked 
 * @param error Pointer to OSyncError which get stacked
 * @returns TRUE on success, FALSE on error. 
 */
static osync_bool osync_updater_stylesheet_process(OSyncUpdater *updater, const char *config, const char *stylesheet, OSyncError **error);

/** @brief Process update on plugin configuration of a certain member.
 *
 * @param updater Pointer to the OSyncUpdater 
 * @param member Pointer to OSyncMember who gets updated
 * @param error Pointer to OSyncError which get stacked
 * @returns TRUE on success, FALSE on error. 
 */
static osync_bool osync_updater_process_plugin_config(OSyncUpdater *updater, OSyncMember *member, OSyncError **error);

/** @brief Process update on member configuration.
 *
 * @param updater Pointer to the OSyncUpdater 
 * @param member Pointer to OSyncMember who gets updated
 * @param error Pointer to OSyncError which get stacked
 * @returns TRUE on success, FALSE on error. 
 */
static osync_bool osync_updater_process_member_config(OSyncUpdater *updater, OSyncMember *member, OSyncError **error);

/** @brief Process update on member. 
 *
 * @param updater Pointer to the OSyncUpdater 
 * @param nthmember Nth member in the group, who gets updated
 * @param error Pointer to OSyncError which get stacked
 * @returns TRUE on success, FALSE on error. 
 */
static osync_bool osync_updater_process_member(OSyncUpdater *updater, unsigned int nthmember, OSyncError **error);

/** @brief Process update on group configuration.
 *
 * @param updater Pointer to the OSyncUpdater 
 * @param error Pointer to OSyncError which get stacked
 * @returns TRUE on success, FALSE on error. 
 */
static osync_bool osync_updater_process_group(OSyncUpdater *updater, OSyncError **error);

/** @brief Create group backup.
 *
 * The entire group directory got moved to to groupN.bak.
 * If already backups present further .bak prefixes got appended.
 *
 * @param updater Pointer to the OSyncUpdater 
 * @param error Pointer to OSyncError which get stacked
 * @returns Path of the backup, NULL on error
 */
static char *osync_updater_create_backup(OSyncUpdater *updater, OSyncError **error);

/** @brief Restore group backup.
 *
 * This function is intended to get called if update process failed.
 *
 * backup_pathgets used as backup directory, if set. Otherwise it restores the 
 * latest backup of the group, if several backups are present.
 *
 * @param updater Pointer to the OSyncUpdater 
 * @param backup_path Path to prefered backup
 * @returns TRUE on success, FALSE on error. 
 */
static osync_bool osync_updater_restore_backup(OSyncUpdater *updater, const char *backup_path);

/** @brief Updater thread function 
 *
 *  #1 Lock group
 *  #2 Check version of group configuration
 *  #3 Create group backup
 *  #4 Process Member configurations
 *  #5 Process Group configuration
 *  #6 Unlock group
 *
 *  On any error (expect locked group) restore backup and unlock group.
 *
 * @param userdata Pointer to the OSyncUpdater 
 * @returns return code of thread as pointer 
 */
static void *osync_updater_run(void *userdata);

/*@}*/

#endif /*  _OPENSYNC_UPDATER_PRIVATE_H */

