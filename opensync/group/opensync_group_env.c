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
 
#include "opensync.h"
#include "opensync_internals.h"

#include "opensync-group.h"
#include "opensync_group_env_internals.h"
#include "opensync_group_env_private.h"

static long long int osync_group_env_create_group_id(OSyncGroupEnv *env)
{
	char *filename = NULL;
	long long int i = 0;
	do {
		i++;
		if (filename)
			osync_free(filename);
		filename = osync_strdup_printf("%s%cgroup%lli", env->groupsdir, G_DIR_SEPARATOR, i);
	} while (g_file_test(filename, G_FILE_TEST_EXISTS));
	osync_free(filename);
	return i;
}

OSyncGroupEnv *osync_group_env_new(OSyncError **error)
{
	OSyncGroupEnv *env = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, error);
	
	env = osync_try_malloc0(sizeof(OSyncGroupEnv), error);
	if (!env) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
		return NULL;
	}
	env->ref_count = 1;

	osync_trace(TRACE_EXIT, "%s: %p", __func__, env);
	return env;

}


OSyncGroupEnv *osync_group_env_ref(OSyncGroupEnv *env)
{
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, env);
	osync_assert(env);

	g_atomic_int_inc(&(env->ref_count));
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, env);
	return env;
}

void osync_group_env_unref(OSyncGroupEnv *env)
{
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, env);
	osync_assert(env);

	if (g_atomic_int_dec_and_test(&(env->ref_count))) {
		if (env->groupsdir)
			osync_free(env->groupsdir);
	
		/* Free the groups */
		while (env->groups) {
			osync_group_unref(env->groups->data);
			env->groups = osync_list_remove(env->groups, env->groups->data);
		}
	
		osync_free(env);
	}

	osync_trace(TRACE_EXIT, "%s", __func__);
}

/* Returns const char* containing the result of XDG_CONFIG_HOME
 * according to the spec.  It is the caller's responsibility to
 * free the returned string.
 *
 * If HOME does not exist, then the current user's home directory
 * is used.  In any case, ".config" is always in the generated
 * default directory name, so it is safe to use non-dot directory
 * names to build on.
 *
 * On fatal error, NULL is returned.
 */
static char *osync_xdg_get_config_home(void)
{
	/* http://standards.freedesktop.org/basedir-spec/latest/ar01s03.html */

	/* XDG_CONFIG_HOME is the base directory to use instead of HOME.
	   If it does not exist, or if the value is an empty string,
	   then use the default: $HOME/.config */

	const char *configdir = g_getenv("XDG_CONFIG_HOME");
	if (configdir != NULL && strlen(configdir) != 0)
		return osync_strdup(configdir);

	/* Use $HOME instead of passwd home, in case someone runs
	 * with OpenSync with sudo. The behaviour of sudo might
	 * differ on different systems, depending on the sudoers
	 * configuration. For more details see ticket #751
	 */
	const char *homedir = g_getenv("HOME");
	if (homedir == NULL) {
		homedir = g_get_home_dir();
		if (homedir == NULL)
			return NULL;
	}

	return osync_strdup_printf("%s%c.config", homedir, G_DIR_SEPARATOR);
}

/* The XDG_CONFIG_HOME spec states that any new directories are created
 * with mode 0700.  This function is a simple wrapper for
 * g_mkdir_with_parents() that enforces this.
 */
static int osync_xdg_make_config_dir(const char *full_path)
{
	return g_mkdir_with_parents(full_path, 0700);
}

osync_bool osync_group_env_load_groups(OSyncGroupEnv *env, const char *path, OSyncError **error)
{	
	GDir *dir = NULL;
	GError *gerror = NULL;
	char *filename = NULL;
	const gchar *de = NULL;
	OSyncGroup *group = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %s, %p)", __func__, env, __NULLSTR(path), error);
	
	/* Create the correct path and test it */
	if (!path) {

		char *homedir = osync_xdg_get_config_home();
		if (!homedir)
			return FALSE;

		env->groupsdir = osync_strdup_printf("%s%copensync%c0.40", homedir, G_DIR_SEPARATOR, G_DIR_SEPARATOR);
		osync_trace(TRACE_INTERNAL, "Default home dir: %s", env->groupsdir);
		osync_free(homedir);
		
		if (!g_file_test(env->groupsdir, G_FILE_TEST_EXISTS)) {
			if (osync_xdg_make_config_dir(env->groupsdir) < 0) {
				osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to create group directory at %s: %s", env->groupsdir, g_strerror(errno));
				goto error_free_path;
			}
			osync_trace(TRACE_INTERNAL, "Created groups configdir %s\n", env->groupsdir);
		}
	} else {
		if (!g_path_is_absolute(path)) {
			env->groupsdir = osync_strdup_printf("%s%c%s", g_get_current_dir(), G_DIR_SEPARATOR, path);
		} else {
			env->groupsdir = osync_strdup(path);
		}
	}
	
	if (!g_file_test(env->groupsdir, G_FILE_TEST_IS_DIR)) {
		osync_error_set(error, OSYNC_ERROR_INITIALIZATION, "%s is not dir", env->groupsdir);
		goto error_free_path;
	}
	
	/* Open the directory */
	dir = g_dir_open(env->groupsdir, 0, &gerror);
	if (!dir) {
		osync_error_set(error, OSYNC_ERROR_IO_ERROR, "Unable to open main configdir %s: %s", env->groupsdir, gerror->message);
		g_error_free (gerror);
		goto error_close_dir;
	}
	
	while ((de = g_dir_read_name(dir))) {
		filename = osync_strdup_printf ("%s%c%s", env->groupsdir, G_DIR_SEPARATOR, de);
		
		if (!g_file_test(filename, G_FILE_TEST_IS_DIR) || !g_pattern_match_simple("group*", de)) {
			osync_free(filename);
			continue;
		}
		
		/* Try to open the confdir*/
		group = osync_group_new(error);
		if (!group) {
			osync_free(filename);
			goto error_close_dir;
		}
		
		if (!osync_group_load(group, filename, error)) {
			osync_free(filename);
			osync_group_unref(group);
			goto error_close_dir;
		}
		
		osync_group_env_add_group(env, group, error);
		osync_group_unref(group);
		
		osync_free(filename);
	}
	g_dir_close(dir);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_close_dir:
	g_dir_close(dir);
 error_free_path:
	osync_free(env->groupsdir);
	env->groupsdir = NULL;
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

OSyncGroup *osync_group_env_find_group(OSyncGroupEnv *env, const char *name)
{
	OSyncList *g = NULL;
	osync_assert(env);
	osync_assert(name);
	
	for (g = env->groups; g; g = g->next) {
		OSyncGroup *group = g->data;
		if (g_ascii_strcasecmp(osync_group_get_name(group), name) == 0)
			return group;
	}
	
	return NULL;
}

osync_bool osync_group_env_add_group(OSyncGroupEnv *env, OSyncGroup *group, OSyncError **error)
{
	const char *group_name = NULL;
	osync_assert(env);
	osync_assert(group);

	group_name = osync_group_get_name(group);
	/* Fail if no group name is already set. The Group name must be set in
		 advanced to check if a group with the same name already exists. */
	if (!group_name) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Group has no name. The Group can't be added to the environment without name.");
		return FALSE;
	}

	/* Check if the group already exist. Fail if there is already a group with the same name */
	if (osync_group_env_find_group(env, group_name)) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Group \"%s\" already exists.", osync_group_get_name(group));
		return FALSE;
	}
	
	if (!osync_group_get_configdir(group)) {
		char *configdir = osync_strdup_printf("%s%cgroup%lli", env->groupsdir, G_DIR_SEPARATOR, osync_group_env_create_group_id(env));
		osync_group_set_configdir(group, configdir);
		osync_free(configdir);
	}
	
	env->groups = osync_list_append(env->groups, group);
	osync_group_ref(group);

	return TRUE;
}

void osync_group_env_remove_group(OSyncGroupEnv *env, OSyncGroup *group)
{
	osync_assert(env);
	osync_assert(group);
	
	env->groups = osync_list_remove(env->groups, group);
	osync_group_unref(group);
}

int osync_group_env_num_groups(OSyncGroupEnv *env)
{
	osync_assert(env);
	return osync_list_length(env->groups);
}

OSyncGroup *osync_group_env_nth_group(OSyncGroupEnv *env, int nth)
{
	osync_assert(env);
	return (OSyncGroup *)osync_list_nth_data(env->groups, nth);
}

OSyncList *osync_group_env_get_groups(OSyncGroupEnv *env) {
	return osync_list_copy(env->groups);
}
