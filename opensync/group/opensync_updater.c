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

#include "config.h"

#include <libxml/xmlIO.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#include <libxslt/xslt.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxslt/xsltInternals.h>

#include <libexslt/exslt.h>

#include "opensync.h"
#include "opensync_internals.h"

#include "opensync-group.h"

#include "common/opensync_xml_internals.h"
#include "common/opensync_file.h"
#include "common/opensync_file_internals.h"

#include "opensync_updater.h"
#include "opensync_updater_private.h"
#include "opensync_updater_internals.h"

#include "opensync_group_internals.h"

static void osync_updater_set_error(OSyncUpdater *updater, OSyncError *error)
{
	osync_assert(updater);
	if (updater->error) {
		osync_error_stack(&error, &updater->error);
		osync_error_unref(&updater->error);
	}
	
	updater->error = error;
	if (error)
		osync_error_ref(&error);
}

static osync_bool osync_updater_stylesheet_process(OSyncUpdater *updater, const char *config, const char *stylesheet, OSyncError **error)
{
	xmlDocPtr result = NULL, doc = NULL, style = NULL;
	xsltTransformContextPtr ctxt = NULL;
	xsltStylesheetPtr cur = NULL;

	osync_assert(updater);
	osync_assert(config);
	osync_assert(stylesheet);
	osync_trace(TRACE_ENTRY, "%s(%p, %s, %s, %p)", __func__, updater, config, stylesheet, error);

	/* Register EXSLT extension.
		 Required for exslt:node-set($rtf) */
	exsltRegisterAll();

	style = xmlReadFile(stylesheet, NULL, XSLT_PARSE_OPTIONS);
	if (!style) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Could not read update stylesheet file: %s", stylesheet);
		goto error;
	}

	doc = xmlReadFile(config, NULL, XSLT_PARSE_OPTIONS);
	if (!doc) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Could not read update stylesheet file: %s", config);
		goto error;
	}

	cur = xsltParseStylesheetDoc(style);
	if (!cur || cur->errors) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Could not load update stylesheet.");
		goto error;
	}

	ctxt = xsltNewTransformContext(cur, doc);
	if (!ctxt) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Failed creating the XSLT context. Run out of memory?");
		goto error;
	}

	result = xsltApplyStylesheetUser(cur, doc, NULL, NULL, NULL, ctxt);
	osync_xml_free_doc(doc);

	if (!result || ctxt->state != XSLT_STATE_OK) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Applying the stylesheet failed.");
		goto error;
	}

	xsltSaveResultToFilename(config, result, cur, 0);

	osync_xml_free_doc(result);
	xsltFreeStylesheet(cur);

	/* FIXME: Review; This got freed by xsltFreeStylesheet?!
		 osync_xml_free_doc(style);
	*/

	xsltFreeTransformContext(ctxt);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
 error:

	if (cur)
		xsltFreeStylesheet(cur);

	/* FIXME: Review; This got freed by xsltFreeStylesheet?!
		 if (style)
		 osync_xml_free_doc(style);
	*/

	if (result)
		osync_xml_free_doc(result);

	if (ctxt)
		xsltFreeTransformContext(ctxt);

	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool osync_updater_process_plugin_config(OSyncUpdater *updater, OSyncMember *member, OSyncError **error)
{
	const char *configdir;
	char *plugin_config = NULL;
	char *update_stylesheet = NULL;

	osync_assert(updater);
	osync_assert(member);
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, updater, member, error);

	/* Member configuration directory */
	configdir = osync_member_get_configdir(member);
	if (!configdir) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Could not find member configuration directory.");
		goto error;
	}

	plugin_config = osync_strdup_printf("%s%c%s.conf", 
	                                    configdir, G_DIR_SEPARATOR,
	                                    osync_member_get_pluginname(member));

	if (!plugin_config) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Could not get enough memory to build plugin configuration path.");
		goto error;
	}

	/* Plugin configuration is optional. So tolerate if none is present.
		 If so - nothing to do. Skip the update process. */
	if (!g_file_test(plugin_config, G_FILE_TEST_IS_REGULAR))
		goto end;

	update_stylesheet = osync_strdup_printf("%s%c%s-%u%s", updater->updatesdir, 
	                                        G_DIR_SEPARATOR, osync_member_get_pluginname(member),
	                                        updater->plugin_version, OSYNC_UPDATER_SUFFIX);

	if (!update_stylesheet) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Could not get enough memory to build plugin update stylesheet path.");
		goto error;
	}

	if (!g_file_test(update_stylesheet, G_FILE_TEST_IS_REGULAR)) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Could not find update stylesheet for \"%s\" plugin in directory: %s", osync_member_get_pluginname(member), updater->updatesdir);
		goto error;
	}

	/* Perform plugin update */
	if (!osync_updater_stylesheet_process(updater, plugin_config, update_stylesheet, error))
		goto error;

	osync_free(update_stylesheet);
 end:
	osync_free(plugin_config);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
 error:	
	if (plugin_config)
		osync_free(plugin_config);

	if (update_stylesheet)
		osync_free(update_stylesheet);

	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}


static osync_bool osync_updater_process_member_config(OSyncUpdater *updater, OSyncMember *member, OSyncError **error)
{
	const char *configdir;
	char *member_config = NULL;
	char *update_stylesheet = NULL;

	osync_assert(updater);
	osync_assert(member);
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, updater, member, error);

	/* Member configuration directory */
	configdir = osync_member_get_configdir(member);
	if (!configdir) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Couldnot find member configuration directory.");
		goto error;
	}

	member_config = osync_strdup_printf("%s%csyncmember.conf", configdir, G_DIR_SEPARATOR);
	if (!member_config) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Could not get enough memory to build member configuration path.");
		goto error;
	}

	if (!g_file_test(member_config, G_FILE_TEST_IS_REGULAR)) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Could not find member configuration file %s", member_config);
		goto error;
	}

	update_stylesheet = osync_strdup_printf("%s%c%s-%u%s", updater->updatesdir, 
	                                        G_DIR_SEPARATOR, "syncmember",
	                                        updater->member_version, OSYNC_UPDATER_SUFFIX);

	if (!update_stylesheet) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Could not get enough memory to build member configuration update stylesheet path.");
		goto error;
	}

	if (!g_file_test(update_stylesheet, G_FILE_TEST_IS_REGULAR)) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Could not find update stylesheet for member configuration in directory: %s", updater->updatesdir);
		goto error;
	}

	/* Perform member update */
	if (!osync_updater_stylesheet_process(updater, member_config, update_stylesheet, error))
		goto error;

	osync_free(member_config);
	osync_free(update_stylesheet);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
 error:	
	if (member_config)
		osync_free(member_config);

	if (update_stylesheet)
		osync_free(update_stylesheet);

	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool osync_updater_process_member(OSyncUpdater *updater, unsigned int nthmember, OSyncError **error)
{
	OSyncMember *member = NULL;
	osync_assert(updater);
	osync_trace(TRACE_ENTRY, "%s(%p, %u, %p)", __func__, updater, nthmember, error);

	osync_trace(TRACE_INTERNAL, "%s: %s", __func__, osync_error_print(error));
	member = osync_group_nth_member(updater->group, nthmember);
	if (!member) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Could not find #%i member in this group.", nthmember);
		goto error;
	}

	/* Member configuration */
	if (!osync_updater_process_member_config(updater, member, error))
		goto error;

	/* Plugin configuration */
	if (!osync_updater_process_plugin_config(updater, member, error))
		goto error;

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
 error:	
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool osync_updater_process_group(OSyncUpdater *updater, OSyncError **error)
{
	const char *configdir;
	char *group_config = NULL;
	char *update_stylesheet = NULL;

	osync_assert(updater);
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, updater, error);

	/* Group configuration directory */
	configdir = osync_group_get_configdir(updater->group);
	if (!configdir) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Could not find group configuration directory.");
		goto error;
	}

	group_config = osync_strdup_printf("%s%c%s", configdir, G_DIR_SEPARATOR, "syncgroup.conf");
	if (!group_config) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Could not get enough memory to build group configuration path.");
		goto error;
	}

	if (!g_file_test(group_config, G_FILE_TEST_IS_REGULAR)) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Could not find group configuration file: %s", group_config);
		goto error;
	}

	update_stylesheet = osync_strdup_printf("%s%c%s-%u%s", updater->updatesdir, 
	                                        G_DIR_SEPARATOR, "syncgroup",
	                                        updater->group_version, OSYNC_UPDATER_SUFFIX);

	if (!update_stylesheet) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Could not get enough memory to build group update stylesheet path.");
		goto error;
	}

	if (!g_file_test(update_stylesheet, G_FILE_TEST_IS_REGULAR)) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Could not find update stylesheet for group configuration in directory: %s", updater->updatesdir);
		goto error;
	}

	/* Perform group update */
	if (!osync_updater_stylesheet_process(updater, group_config, update_stylesheet, error))
		goto error;

	osync_free(group_config);
	osync_free(update_stylesheet);


	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
 error:

	if (group_config)
		osync_free(group_config);

	if (update_stylesheet)
		osync_free(update_stylesheet);

	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static char *osync_updater_create_backup(OSyncUpdater *updater, OSyncError **error)
{
	GDir *dir = NULL, *member_dir = NULL;
	GError *gerror = NULL;
	const char *de = NULL, *member_de = NULL;
	char *backup_groupdir = NULL;
	struct stat orig_stat;
	const char *orig_groupdir = NULL;
	char *config, *copy_config, *content, *member_path;
	gsize length;


	osync_assert(updater);
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, updater, error);

	orig_groupdir = osync_group_get_configdir(updater->group);

	/* Sanity check if group got loaded and has a configdir set */
	if (!orig_groupdir) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "No group configuration directory set. Could not create Backup. Aborting.");
		goto error;
	}

	backup_groupdir = osync_strdup_printf("%s.bak", orig_groupdir);

	/* #1 Move original group directory */
	while (g_file_test(backup_groupdir, G_FILE_TEST_IS_DIR)) {
		char *tmp = backup_groupdir;
		backup_groupdir = osync_strdup_printf("%s.bak", tmp);
		osync_free(tmp);
	}

	if (g_stat(orig_groupdir, &orig_stat) < 0) {
		g_set_error(&gerror, G_FILE_ERROR, g_file_error_from_errno(errno), "%s", orig_groupdir);
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Could not get information about group directory: %s", gerror->message);
		g_error_free(gerror);
		goto error;
	}

	if (g_rename(orig_groupdir, backup_groupdir) < 0) {
		g_set_error(&gerror, G_FILE_ERROR, g_file_error_from_errno(errno), "%s", orig_groupdir);
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Could not backup group directory: %s", gerror->message);
		g_error_free(gerror);
		goto error;
	}

	if (g_mkdir(orig_groupdir, orig_stat.st_mode) < 0) { 
		g_set_error(&gerror, G_FILE_ERROR, g_file_error_from_errno(errno), "%s", orig_groupdir);
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Could not create new group directory: %s", gerror->message);
		g_error_free(gerror);
		goto error;
	}

	/* #2 Copy group and member configurations */
	dir = g_dir_open(backup_groupdir, 0, &gerror);
	if (!dir) {
		osync_error_set(error, OSYNC_ERROR_IO_ERROR, "Unable to open group configdir %s", gerror->message);
		g_error_free(gerror);
		goto error;
	}

	while ((de = g_dir_read_name(dir))) {
		member_path = osync_strdup_printf("%s%c%s%c", backup_groupdir, G_DIR_SEPARATOR, de, G_DIR_SEPARATOR);
		if (!g_file_test(member_path, G_FILE_TEST_IS_DIR)) {
			osync_free(member_path);
			continue;
		}

		if (g_stat(member_path, &orig_stat) < 0) {
			g_set_error(&gerror, G_FILE_ERROR, g_file_error_from_errno(errno), "%s", member_path);
			osync_error_set(error, OSYNC_ERROR_GENERIC, "Could not get information about member directory: %s", gerror->message);
			g_error_free(gerror);
			osync_free(member_path);
			goto error;
		}

		member_dir = g_dir_open(member_path, 0, &gerror);
		osync_free(member_path);

		if (!member_dir) {
			osync_error_set(error, OSYNC_ERROR_IO_ERROR, "Unable to open member configdir %s", gerror->message);
			g_error_free(gerror);
			goto error;
		}

		member_path = osync_strdup_printf("%s%c%s%c", orig_groupdir, G_DIR_SEPARATOR, de, G_DIR_SEPARATOR);
		if (g_mkdir(member_path, orig_stat.st_mode) < 0) { 
			g_set_error(&gerror, G_FILE_ERROR, g_file_error_from_errno(errno), "%s", member_path);
			osync_error_set(error, OSYNC_ERROR_GENERIC, "Could not create new member directory: %s", gerror->message);
			g_error_free(gerror);
			osync_free(member_path);
			goto error;
		}
		osync_free(member_path);

		while ((member_de = g_dir_read_name(member_dir))) {
			config = osync_strdup_printf("%s%c%s%c%s", backup_groupdir, G_DIR_SEPARATOR, de, G_DIR_SEPARATOR, member_de);
			
			/* Only copy files ending with ".conf" */
			//			if (!g_file_test(config, G_FILE_TEST_IS_REGULAR) || !sscanf(config, "%*s.conf")) {
			if (!g_file_test(config, G_FILE_TEST_IS_REGULAR) || !g_pattern_match_simple("*.conf", config)) {
				osync_free(config);
				continue;
			}

			if (!g_file_get_contents(config, &content, &length, &gerror)) {
				osync_error_set(error, OSYNC_ERROR_IO_ERROR, "Failed reading configfile: %s", gerror->message);
				g_error_free(gerror);
				osync_free(config);
				goto error;
			}
			osync_free(config);

			copy_config = osync_strdup_printf("%s%c%s%c%s", orig_groupdir, G_DIR_SEPARATOR, de, G_DIR_SEPARATOR, member_de);

			if (!g_file_set_contents(copy_config, content, length, &gerror)) {
				osync_error_set(error, OSYNC_ERROR_IO_ERROR, "Failed writing configfile: %s", gerror->message);
				g_error_free(gerror);
				osync_free(copy_config);
				g_free(content);
				goto error;
			}
			osync_free(copy_config);
			g_free(content);

		}

		g_dir_close(member_dir);

	}
	g_dir_close(dir);


	config = osync_strdup_printf("%s%c%s", backup_groupdir, G_DIR_SEPARATOR, "syncgroup.conf");

	if (!g_file_get_contents(config, &content, &length, &gerror)) {
		osync_error_set(error, OSYNC_ERROR_IO_ERROR, "Failed reading group configfile: %s", gerror->message);
		g_error_free(gerror);
		osync_free(config);
		goto error;
	}
	osync_free(config);

	copy_config = osync_strdup_printf("%s%c%s", orig_groupdir, G_DIR_SEPARATOR, "syncgroup.conf");

	if (!g_file_set_contents(copy_config, content, length, &gerror)) {
		osync_error_set(error, OSYNC_ERROR_IO_ERROR, "Failed writing group configfile: %s", gerror->message);
		g_error_free(gerror);
		osync_free(copy_config);
		g_free(content);
		goto error;
	}
	osync_free(copy_config);
	g_free(content);

	osync_trace(TRACE_EXIT, "%s: %s", __func__, backup_groupdir);
	return backup_groupdir;
 error:
	if (member_dir)
		g_dir_close(member_dir);

	if (backup_groupdir)
		osync_free(backup_groupdir);

	if (dir)
		g_dir_close(dir);

	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;

}

static osync_bool osync_updater_restore_backup(OSyncUpdater *updater, const char *backup_path)
{
	OSyncError *error = NULL;
	GError *gerror = NULL;
	char *backup_groupdir = NULL;
	const char *groupdir = NULL;

	osync_assert(updater);
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, updater);

	groupdir = osync_group_get_configdir(updater->group);
	/* Sanity check if group got loaded and a configdir is present */
	if (!groupdir) {
		osync_error_set(&error, OSYNC_ERROR_GENERIC, "No group configuration directory set. Could not restore Backup.");
		goto error;
	}

	backup_groupdir = backup_path ? osync_strdup(backup_path) : osync_strdup_printf("%s.bak", groupdir);

	/* Search for latest backup if no certain backup_path was given.

		 The latest backups might look like this:
		 ~/.opensync/group1.bak.bak.bak.bak.bak.bak.bak */
	while (!backup_path && !g_file_test(backup_groupdir, G_FILE_TEST_IS_DIR)) {
		char *tmp = backup_groupdir;
		backup_groupdir = osync_strdup_printf("%s.bak", tmp);
		osync_free(tmp);
	}
	if (osync_remove_directory_recursively(groupdir) < 0) {
		g_set_error(&gerror, G_FILE_ERROR, g_file_error_from_errno(errno), "%s", groupdir);
		osync_error_set(&error, OSYNC_ERROR_GENERIC, "Could not remove current group directory: %s", gerror->message);
		g_error_free(gerror);
		goto error;
	}

	if (g_rename(backup_groupdir, groupdir) < 0) {
		g_set_error(&gerror, G_FILE_ERROR, g_file_error_from_errno(errno), "%s", backup_groupdir);
		osync_error_set(&error, OSYNC_ERROR_GENERIC, "Could not restore backup group directory: %s", gerror->message);
		g_error_free(gerror);
		goto error;
	}

	osync_free(backup_groupdir);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
 error:
	osync_free(backup_groupdir);
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(&error));
	osync_updater_set_error(updater, error);
	return FALSE;

}

static void *osync_updater_run(void *userdata)
{
	OSyncUpdater *updater = (OSyncUpdater *) userdata;
	OSyncError *error = NULL;
	int i, num_members;
	char *backup_dir = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, userdata);

	/* #1 Lock group */
	if (osync_group_lock(updater->group) == OSYNC_LOCKED) {
		osync_error_set(&error, OSYNC_ERROR_GENERIC, "Group is locked. Can not process update on this group.");
		goto error;
	}

	/* #2 Check version of group configuration */
	if (!osync_updater_action_required(updater)) {
		osync_error_set(&error, OSYNC_ERROR_GENERIC, "No update required for this group. Aborting Update.");
		goto error_and_unlock;
	}

	/* #3 Create backup of group, before performing any update! */
	if (!(backup_dir = osync_updater_create_backup(updater, &error)))
		goto error_and_unlock;

	/* #4 Process Member configurations */
	num_members = osync_group_num_members(updater->group);
	for (i=0; i < num_members; i++) {
		if (!osync_updater_process_member(updater, i, &error))
			goto error_and_restore;

		/* TODO: Emit updater member X signal */
	}

	/* #5 Process Group configuration */
	if (!osync_updater_process_group(updater, &error))
		goto error_and_restore;

	/* #6 Unlock group */
	osync_group_unlock(updater->group);

	g_mutex_lock(updater->updating_mutex);
	g_cond_signal(updater->updating);
	g_mutex_unlock(updater->updating_mutex);

	osync_free(backup_dir);

	osync_trace(TRACE_EXIT, "%s", __func__);
	osync_thread_exit(updater->thread, 0);
	return 0;

 error_and_restore:
	osync_updater_restore_backup(updater, backup_dir);
 error_and_unlock:
	osync_group_unlock(updater->group);
 error:

	if (backup_dir)
		osync_free(backup_dir);

	/* TODO: Emit error signal */
	osync_updater_set_error(updater, error);

	g_mutex_lock(updater->updating_mutex);
	g_cond_signal(updater->updating);
	g_mutex_unlock(updater->updating_mutex);

	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(&error));

	osync_thread_exit(updater->thread, 0);
	return 0;
}

void osync_updater_set_group_version(OSyncUpdater *updater, int major)
{
	osync_assert(updater);
	updater->group_version = major;
}

void osync_updater_set_member_version(OSyncUpdater *updater, int major)
{
	osync_assert(updater);
	updater->member_version = major;
}

void osync_updater_set_plugin_version(OSyncUpdater *updater, int major)
{
	osync_assert(updater);
	updater->plugin_version = major;
}

OSyncUpdater *osync_updater_new(OSyncGroup *group, OSyncError **error)
{
	OSyncUpdater *updater = osync_try_malloc0(sizeof(OSyncUpdater), error);
	osync_assert(group);

	if (!updater)
		return NULL;

	updater->ref_count = 1;
	updater->group = group;

	updater->updatesdir = osync_strdup(OPENSYNC_UPDATESDIR);

	updater->updating_mutex = g_mutex_new();
	updater->updating = g_cond_new();

	updater->member_version = OSYNC_MEMBER_MAJOR_VERSION;
	updater->group_version = OSYNC_GROUP_MAJOR_VERSION;
	updater->plugin_version = OSYNC_PLUGIN_MAJOR_VERSION;
	
	return updater;
}

OSyncUpdater *osync_updater_ref(OSyncUpdater *updater)
{
	osync_assert(updater);
	
	g_atomic_int_inc(&(updater->ref_count));

	return updater;
}

void osync_updater_unref(OSyncUpdater *updater)
{
	osync_assert(updater);
	
	if (g_atomic_int_dec_and_test(&(updater->ref_count))) {

		if (updater->updating)
			g_cond_free(updater->updating);
			
		if (updater->updating_mutex)
			g_mutex_free(updater->updating_mutex);

		if (updater->updatesdir)
			osync_free(updater->updatesdir);
		
		osync_free(updater);
	}
}

void osync_updater_set_callback(OSyncUpdater *updater, osync_updater_cb callback)
{
	osync_assert(updater);
	updater->status_callback = callback;
}

osync_bool osync_updater_action_required(OSyncUpdater *updater)
{
	int i, num_members;
	osync_assert(updater);
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, updater);

	if (!osync_group_is_uptodate(updater->group)) {
		osync_trace(TRACE_EXIT, "%s: The group requires action.", __func__);
		return TRUE;
	}

	/* TODO: Update check magic */
	num_members = osync_group_num_members(updater->group);
	for (i=0; i < num_members; i++) {
		OSyncMember *member = osync_group_nth_member(updater->group, i);
		if (!osync_member_config_is_uptodate(member)) {
			osync_trace(TRACE_EXIT, "%s: #%i Member config requires action.", __func__, i);
			return TRUE;
		}

		if (!osync_member_plugin_is_uptodate(member)) {
			osync_trace(TRACE_EXIT, "%s: #%i Member plugin requires action.", __func__, i);
			return TRUE;
		}
	}

	osync_trace(TRACE_EXIT, "%s: Everything up to date", __func__);
	return FALSE;
}

osync_bool osync_updater_process(OSyncUpdater *updater, OSyncError **error)
{
	osync_assert(updater);
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, updater, error);

	updater->thread = osync_thread_create(osync_updater_run, updater, error);
	if (!updater->thread)
		goto error;

	osync_updater_ref(updater);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;

}

osync_bool osync_updater_process_and_block(OSyncUpdater *updater, OSyncError **error)
{
	osync_assert(updater);
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, updater, error);
	
	g_mutex_lock(updater->updating_mutex);
	
	if (!osync_updater_process(updater, error)) {
		g_mutex_unlock(updater->updating_mutex);
		goto error;
	}

	g_cond_wait(updater->updating, updater->updating_mutex);
	g_mutex_unlock(updater->updating_mutex);
	
	if (updater->error)
		goto error;
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

void osync_updater_set_updates_directory(OSyncUpdater *updater, const char *path)
{
	osync_assert(updater);
	osync_assert(path);

	if (updater->updatesdir)
		osync_free(updater->updatesdir);

	updater->updatesdir = osync_strdup(path);
}

const char *osync_updater_get_updates_directory(OSyncUpdater *updater)
{
	return updater->updatesdir; 
}

