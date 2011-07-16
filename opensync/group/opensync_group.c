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

#include "opensync-format.h"

#include "format/opensync_filter_internals.h"
#include "common/opensync_xml_internals.h"
#include "db/opensync_db_internals.h"

#include "opensync-group.h"
#include "opensync_group_internals.h"
#include "opensync_group_private.h"
#include "opensync_member_internals.h"

#ifdef OPENSYNC_UNITTESTS
#include "opensync_member_internals.h"
#endif /* OPENSYNC_UNITTESTS */

#ifndef _WIN32
#include <sys/file.h>

#ifndef HAVE_FLOCK
#define LOCK_SH 1
#define LOCK_EX 2
#define LOCK_NB 4
#define LOCK_UN 8

static int flock(int fd, int operation)
{
	struct flock flock;

	switch (operation & ~LOCK_NB) {
	case LOCK_SH:
		flock.l_type = F_RDLCK;
		break;
	case LOCK_EX:
		flock.l_type = F_WRLCK;
		break;
	case LOCK_UN:
		flock.l_type = F_UNLCK;
		break;
	default:
		errno = EINVAL;
		return -1;
	}

	flock.l_whence = 0;
	flock.l_start = 0;
	flock.l_len = 0;

	return fcntl(fd, (operation & LOCK_NB) ? F_SETLK : F_SETLKW, &flock);
}
#endif //NOT_HAVE_FLOCK
#else  //not defined _WIN32
#include <io.h> /* For close() */
#endif //not defined _WIN32


static osync_memberid osync_group_create_member_id(OSyncGroup *group)
{
	char *filename = NULL;
	osync_memberid i = 0;
	
	do {
		i++;
		if (filename)
			osync_free(filename);
		filename = osync_strdup_printf("%s%c%i", group->configdir, G_DIR_SEPARATOR, i);
	} while (g_file_test(filename, G_FILE_TEST_EXISTS));
	
	osync_free(filename);
	return i;
}

static osync_bool osync_group_load_members(OSyncGroup *group, const char *path, OSyncError **error)
{	
	GDir *dir = NULL;
	GError *gerror = NULL;
	char *member_path = NULL;
	char *filename = NULL;
	OSyncMember *member = NULL;
	const gchar *de = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %s, %p)", __func__, group, path, error);

	dir = g_dir_open(path, 0, &gerror);
	if (!dir) {
		osync_error_set(error, OSYNC_ERROR_IO_ERROR, "Unable to open group configdir %s", gerror->message);
		g_error_free (gerror);
		goto error;
	}

	while ((de = g_dir_read_name(dir))) {
		filename = osync_strdup_printf ("%s%c%s%csyncmember.conf", osync_group_get_configdir(group), G_DIR_SEPARATOR, de, G_DIR_SEPARATOR);
		if (!g_file_test(filename, G_FILE_TEST_IS_REGULAR)) {
			osync_free(filename);
			continue;
		}
		osync_free(filename);
		
		member = osync_member_new(error);
		if (!member)
			goto error_close;

#ifdef OPENSYNC_UNITTESTS
		if (group->schemadir)
			osync_member_set_schemadir(member, group->schemadir);
#endif /* OPENSYNC_UNITTESTS */
		
		member_path = osync_strdup_printf ("%s%c%s", osync_group_get_configdir(group), G_DIR_SEPARATOR, de);
		if (!osync_member_load(member, member_path, error)) {
			osync_free(member_path);
			goto error_free_member;
		}
		osync_free(member_path);
		
		osync_group_add_member(group, member);
		osync_member_unref(member);
	}
	g_dir_close(dir);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_free_member:
	osync_member_unref(member);
 error_close:
	g_dir_close(dir);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %p", __func__, osync_error_print(error));
	return FALSE;
}

static void osync_group_build_list(gpointer key, gpointer value, gpointer user_data)
{
	if (GPOINTER_TO_INT(value) >= 2) {
		GList **l = user_data;
		*l = g_list_append(*l, key);
	}
}

static void osync_group_build_mixed_list(gpointer key, gpointer value, gpointer user_data)
{
	OSyncList **l = user_data;
	*l = osync_list_append(*l, key);
}

static GList *osync_group_get_supported_objtypes(OSyncGroup *group)
{
	OSyncList *m = NULL;
	GList *ret = NULL;
	GHashTable *table = g_hash_table_new(g_str_hash, g_str_equal);
		
	int i;
		
	/* Loop over all members... */
	for (m = group->members; m; m = m->next) {
		OSyncMember *member = m->data;
		int num_member = osync_member_num_objtypes(member);
		/* ... and get the objtype from each of the members. */
		for (i = 0; i < num_member; i++) {
			const char *objtype = osync_member_nth_objtype(member, i);
			if (objtype != NULL) {
				int num = 0;
				/* For each objtype, add 1 to the hashtable. */
				num = GPOINTER_TO_INT(g_hash_table_lookup(table, objtype));
				g_hash_table_replace(table, (char *)objtype, GINT_TO_POINTER(num + 1));
			}
		}
	}
	
	g_hash_table_foreach(table, osync_group_build_list, &ret);
	g_hash_table_destroy(table);
	return ret;
}

OSyncList *osync_group_get_supported_objtypes_mixed(OSyncGroup *group, OSyncFormatEnv *formatenv)
{

	OSyncList *m;
	OSyncList *ret = NULL;
	OSyncList *t, *targetformats = osync_group_get_objformats(group);

	GHashTable *table = g_hash_table_new(g_str_hash, g_str_equal);


	for (m=group->members; m; m = m->next) {
		OSyncMember *member = m->data;
		int i, num_member = osync_member_num_objtypes(member);
		/* ... and get the objtype from each of the members. */
		for (i = 0; i < num_member; i++) {
			const char *objtype = osync_member_nth_objtype(member, i);
			osync_assert(objtype);
			g_hash_table_replace(table, (char *)objtype, NULL);
		}

		for (t=targetformats; t; t = t->next) {
			const char *targetformat_name = t->data;
			OSyncObjFormat *targetformat = osync_format_env_find_objformat(formatenv, targetformat_name);
			OSyncObjFormat *sourceformat;
			const char *source_objtype;
			const char *target_objtype = osync_objformat_get_objtype(targetformat);

			osync_assert(target_objtype);

			/* If targetformat is not supported, skip it */
			sourceformat = osync_member_support_targetformat(member, formatenv, targetformat);
			if (!sourceformat)
				continue;

			source_objtype = osync_objformat_get_objtype(sourceformat);
			osync_member_add_alternative_objtype(member, source_objtype, target_objtype);

			/* For each objtype, add 1 to the hashtable. */
			g_hash_table_replace(table, (char *)target_objtype, NULL);

		}
	}

	g_hash_table_foreach(table, osync_group_build_mixed_list, &ret);
	g_hash_table_destroy(table);
	osync_list_free(targetformats);

	return ret;
}

OSyncList *osync_group_get_objformats(OSyncGroup *group)
{
	OSyncList *m = NULL;
	OSyncList *list = NULL;

	/* Loop over all members... */
	for (m = group->members; m; m = m->next) {
		OSyncMember *member = m->data;
		OSyncList *objformats = osync_member_get_all_objformats(member);

		list = osync_list_concat(list, objformats);

	}

	return list;
}

#ifdef OPENSYNC_UNITTESTS

void osync_group_set_schemadir(OSyncGroup *group, const char *schemadir)
{
	osync_assert(group);
	osync_assert(schemadir);

	if (group->schemadir)
		osync_free(group->schemadir);

	group->schemadir = osync_strdup(schemadir); 
}

#endif /* OPENSYNC_UNITTESTS */

OSyncGroup *osync_group_new(OSyncError **error)
{
	OSyncGroup *group = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, error);
	
	group = osync_try_malloc0(sizeof(OSyncGroup), error);
	if (!group)
		goto error;
	group->ref_count = 1;

	/* By default Merger is enabled */
	group->merger_enabled = TRUE;

	/* By default Converter is enabled */
	group->converter_enabled = TRUE;
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, group);
	return group;
	
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %p", __func__, osync_error_print(error));
	return NULL;
}

OSyncGroup *osync_group_ref(OSyncGroup *group)
{
	osync_assert(group);
	
	g_atomic_int_inc(&(group->ref_count));

	return group;
}

void osync_group_unref(OSyncGroup *group)
{
	osync_assert(group);
		
	if (g_atomic_int_dec_and_test(&(group->ref_count))) {
		
		while (group->members)
			osync_group_remove_member(group, group->members->data);
		
		if (group->name)
			osync_free(group->name);
		
		if (group->configdir)
			osync_free(group->configdir);
			
#ifdef OPENSYNC_UNITTESTS
		if (group->schemadir)
			osync_free(group->schemadir);
#endif /* OPENSYNC_UNITTESTS */

		osync_free(group);
	}
}

OSyncLockState osync_group_lock(OSyncGroup *group, OSyncError **error)
{
	char *lockfile = NULL;
	osync_bool exists = FALSE;
	osync_bool locked = FALSE;
	
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, group);
	osync_assert(group);
	
	if (!group->configdir) {
		osync_trace(TRACE_EXIT, "%s: OSYNC_LOCK_OK: No configdir", __func__);
		return OSYNC_LOCK_OK;
	}
	
	if (group->lock_fd) {
		osync_trace(TRACE_EXIT, "%s: OSYNC_LOCKED, lock_fd existed", __func__);
		return OSYNC_LOCKED;
	}
	
	lockfile = osync_strdup_printf("%s%clock", group->configdir, G_DIR_SEPARATOR);

	if (g_file_test(lockfile, G_FILE_TEST_EXISTS)) {
		osync_trace(TRACE_INTERNAL, "locking group: file exists");
		exists = TRUE;
	}

	if ((group->lock_fd = g_open(lockfile, O_CREAT | O_WRONLY, 00700)) == -1) {
		group->lock_fd = 0;
		osync_free(lockfile);
		osync_error_set(error, OSYNC_ERROR_IO_ERROR, "Unable to open: %s", g_strerror(errno) );
		goto error;
	} else {
#ifndef _WIN32
		/* Set FD_CLOEXEC flags for the lock file descriptor. We don't want the
		 * subprocesses created by plugins or the engine to keep holding the lock
		 */
		int oldflags = fcntl(group->lock_fd, F_GETFD);
		if (oldflags == -1) {
			osync_error_set(error, OSYNC_ERROR_IO_ERROR, "Unable to get fd flags");
			goto error;
		}

		if (fcntl(group->lock_fd, F_SETFD, oldflags|FD_CLOEXEC) == -1) {
			osync_error_set(error, OSYNC_ERROR_IO_ERROR, "Unable to set fd flags");
			goto error;
		}

		if (flock(group->lock_fd, LOCK_EX | LOCK_NB) == -1) {
			if (errno == EWOULDBLOCK) {
				osync_trace(TRACE_INTERNAL, "locking group: is locked2");
				locked = TRUE;
				close(group->lock_fd);
				group->lock_fd = 0;
			} else {
				osync_error_set(error, OSYNC_ERROR_IO_ERROR, "error setting lock: %s", g_strerror(errno));
				osync_trace(TRACE_INTERNAL, "%s", osync_error_print(error));
			}
		} else
#else /* _WIN32 */
			/* Windows cannot delete files which are open. When doing the backup, the lock file */
			/* would be open */
			close(group->lock_fd); 
#endif
		osync_trace(TRACE_INTERNAL, "Successfully locked");
	} /* g_open */
	
	osync_free(lockfile);
	
	if (exists) {
		if (locked) {
			osync_trace(TRACE_EXIT, "%s: OSYNC_LOCKED", __func__);
			return OSYNC_LOCKED;
		} else {
			osync_trace(TRACE_EXIT, "%s: OSYNC_LOCK_STALE", __func__);
			return OSYNC_LOCK_STALE;
		}
	}
	
	osync_trace(TRACE_EXIT, "%s: OSYNC_LOCK_OK", __func__);
	return OSYNC_LOCK_OK;
error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__,  osync_error_print(error));
	return OSYNC_LOCK_STALE;
}

void osync_group_unlock(OSyncGroup *group)
{
	char *lockfile = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, group);
	osync_assert(group);
	
	if (!group->configdir) {
		osync_trace(TRACE_EXIT, "%s: No configdir", __func__);
		return;
	}
	
	if (!group->lock_fd) {
		osync_trace(TRACE_EXIT, "%s: You have to lock the group before unlocking", __func__);
		return;
	}
		
#ifndef _WIN32
	flock(group->lock_fd, LOCK_UN);
	close(group->lock_fd);	
#endif
	group->lock_fd = 0;
	
	lockfile = osync_strdup_printf("%s%clock", group->configdir, G_DIR_SEPARATOR);
	
	g_unlink(lockfile);
	osync_free(lockfile);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

void osync_group_set_name(OSyncGroup *group, const char *name)
{
	g_assert(group);
	if (group->name)
		osync_free(group->name);
	group->name = osync_strdup(name);
}

const char *osync_group_get_name(OSyncGroup *group)
{
	g_assert(group);
	return group->name;
}

osync_bool osync_group_save(OSyncGroup *group, OSyncError **error)
{
	char *filename = NULL;
	unsigned int i;
	xmlDocPtr doc;
	char *tmstr = NULL;
	char *version_str = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, group, error);
	osync_assert(group);
	osync_assert(group->configdir);
	
	osync_trace(TRACE_INTERNAL, "Trying to open configdirectory %s to save group %s", group->configdir, group->name);
	
	if (!g_file_test(group->configdir, G_FILE_TEST_IS_DIR)) {
		osync_trace(TRACE_INTERNAL, "Creating group configdirectory %s", group->configdir);
		if (g_mkdir(group->configdir, 0700)) {
			osync_error_set(error, OSYNC_ERROR_IO_ERROR, "Unable to create directory for group %s\n", group->name);
			goto error;
		}
	}
	
	filename = osync_strdup_printf ("%s%csyncgroup.conf", group->configdir, G_DIR_SEPARATOR);
	osync_trace(TRACE_INTERNAL, "Saving group to file %s", filename);
	
	doc = xmlNewDoc((xmlChar*)"1.0");
	doc->children = xmlNewDocNode(doc, NULL, (xmlChar*)"syncgroup", NULL);

	version_str = osync_strdup_printf("%u.%u", OSYNC_GROUP_MAJOR_VERSION, OSYNC_GROUP_MINOR_VERSION);
	xmlSetProp(doc->children, (const xmlChar*)"version", (const xmlChar *)version_str);	
	osync_free(version_str);
	
	// TODO: reimplement the filter!
	//The filters
	/*GList *f;
		for (f = group->filters; f; f = f->next) {
		OSyncFilter *filter = f->data;
		xmlNodePtr child = xmlNewChild(doc->children, NULL, (xmlChar*)"filter", NULL);
		
		if (filter->sourcememberid) {
		char *sourcememberid = g_strdup_printf("%lli", filter->sourcememberid);
		xmlNewChild(child, NULL, (xmlChar*)"sourcemember", (xmlChar*)sourcememberid);
		g_free(sourcememberid);
		}
		if (filter->destmemberid) {
		char *destmemberid = g_strdup_printf("%lli", filter->destmemberid);
		xmlNewChild(child, NULL, (xmlChar*)"destmember", (xmlChar*)destmemberid);
		g_free(destmemberid);
		}
		if (filter->sourceobjtype)
		xmlNewChild(child, NULL, (xmlChar*)"sourceobjtype", (xmlChar*)filter->sourceobjtype);
		if (filter->destobjtype)
		xmlNewChild(child, NULL, (xmlChar*)"destobjtype", (xmlChar*)filter->destobjtype);
		if (filter->detectobjtype)
		xmlNewChild(child, NULL, (xmlChar*)"detectobjtype", (xmlChar*)filter->detectobjtype);
		if (filter->action) {
		char *action = g_strdup_printf("%i", filter->action);
		xmlNewChild(child, NULL, (xmlChar*)"action", (xmlChar*)action);
		g_free(action);
		}
		if (filter->function_name)
		xmlNewChild(child, NULL, (xmlChar*)"function_name", (xmlChar*)filter->function_name);
		if (filter->config)
		xmlNewChild(child, NULL, (xmlChar*)"config", (xmlChar*)filter->config);
		}*/

	xmlNewChild(doc->children, NULL, (xmlChar*)"groupname", (xmlChar*)group->name);

	tmstr = osync_strdup_printf("%i", (int)group->last_sync);
	xmlNewChild(doc->children, NULL, (xmlChar*)"last_sync", (xmlChar*)tmstr);
	osync_free(tmstr);

	xmlNewChild(doc->children, NULL, (xmlChar*)"merger_enabled", (xmlChar*) (group->merger_enabled ? "true" : "false"));
	xmlNewChild(doc->children, NULL, (xmlChar*)"converter_enabled", (xmlChar*) (group->converter_enabled ? "true" : "false"));


	xmlSaveFormatFile(filename, doc, 1);
	osync_xml_free_doc(doc);
	osync_free(filename);

	for (i = 0; i < osync_group_num_members(group); i++) {
		OSyncMember *member = osync_group_nth_member(group, i);
		if (!osync_member_save(member, error))
			goto error;
	}
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_group_delete(OSyncGroup *group, OSyncError **error)
{
	char *delcmd = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, group, error);
	osync_assert(group);
	
	delcmd = osync_strdup_printf("rm -rf %s", group->configdir);
	if (system(delcmd)) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Failed to delete group. command %s failed", delcmd);
		osync_free(delcmd);
		osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
		return FALSE;
	}
	osync_free(delcmd);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
}

osync_bool osync_group_reset(OSyncGroup *group, OSyncError **error)
{
	OSyncDB *db = NULL;
	OSyncList *m = NULL;
	char *path = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, group, error);

	osync_assert(group);

	/* Loop over all members... */
	for (m = group->members; m; m = m->next) {
		OSyncMember *member = m->data;

		/* flush hashtable */
		path = osync_strdup_printf("%s%chashtable.db", osync_member_get_configdir(member), G_DIR_SEPARATOR);
		if (!(db = osync_db_new(error)))
			goto error_and_free;

		if (!osync_db_open(db, path, error))
			goto error_and_free;

		osync_db_reset_full(db, error);

		osync_free(path);

		/* flush state db */ 
		path = osync_strdup_printf("%s%canchor.db", osync_member_get_configdir(member), G_DIR_SEPARATOR);
		if (!(db = osync_db_new(error)))
			goto error_and_free;

		if (!osync_db_open(db, path, error))
			goto error_and_free;

		osync_db_reset_full(db, error);

		osync_free(path);

	}

	path = osync_strdup_printf("%s%carchive.db", osync_group_get_configdir(group), G_DIR_SEPARATOR);
	if (!(db = osync_db_new(error)))
		goto error_and_free;

	if (!osync_db_open(db, path, error))
		goto error_and_free;

	osync_db_reset_full(db, error);

	osync_free(path);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_and_free:
	osync_free(path);	
	//error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_group_load(OSyncGroup *group, const char *path, OSyncError **error)
{
	char *filename = NULL;
	char *real_path = NULL;
	xmlDocPtr doc;
	xmlNodePtr cur;
	//xmlNodePtr filternode;
	
	osync_assert(group);
	osync_assert(path);
	osync_trace(TRACE_ENTRY, "%s(%p, %s, %p)", __func__, group, path, error);
	
	if (!g_path_is_absolute(path)) {
		char *curdir = g_get_current_dir();
		real_path = osync_strdup_printf("%s%c%s", curdir, G_DIR_SEPARATOR, path);
		g_free(curdir);
	} else {
		real_path = osync_strdup(path);
	}
	
	osync_group_set_configdir(group, real_path);
	filename = osync_strdup_printf("%s%csyncgroup.conf", real_path, G_DIR_SEPARATOR);
	osync_free(real_path);
	
	if (!osync_xml_open_file(&doc, &cur, filename, "syncgroup", error)) {
		osync_free(filename);
		goto error;
	}
	osync_free(filename);
	
	while (cur != NULL) {
		char *str = (char*)xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		if (str) {
			if (!xmlStrcmp(cur->name, (const xmlChar *)"groupname"))
				osync_group_set_name(group, str);
	
			if (!xmlStrcmp(cur->name, (const xmlChar *)"last_sync"))
				group->last_sync = (time_t)atoi(str);
			
			//TODO: remove the next 2 lines later
			if (!xmlStrcmp(cur->name, (const xmlChar *)"enable_merger"))
				group->merger_enabled = (!g_ascii_strcasecmp("true", str)) ? TRUE : FALSE;
			//TODO: remove the next 2 lines later
			if (!xmlStrcmp(cur->name, (const xmlChar *)"enable_converter"))
				group->converter_enabled = (!g_ascii_strcasecmp("true", str)) ? TRUE : FALSE;

			if (!xmlStrcmp(cur->name, (const xmlChar *)"merger_enabled"))
				group->merger_enabled = (!g_ascii_strcasecmp("true", str)) ? TRUE : FALSE;

			if (!xmlStrcmp(cur->name, (const xmlChar *)"converter_enabled"))
				group->converter_enabled = (!g_ascii_strcasecmp("true", str)) ? TRUE : FALSE;

			// TODO: reimplement the filter!
			/*if (!xmlStrcmp(cur->name, (const xmlChar *)"filter")) {
				filternode = cur->xmlChildrenNode;
				OSyncFilter *filter = osync_filter_new();
				filter->group = group;
				
				while (filternode != NULL) {
				if (!xmlStrcmp(filternode->name, (const xmlChar *)"sourceobjtype"))
				filter->sourceobjtype = (char*)xmlNodeListGetString(doc, filternode->xmlChildrenNode, 1);
					
				if (!xmlStrcmp(filternode->name, (const xmlChar *)"destobjtype"))
				filter->destobjtype = (char*)xmlNodeListGetString(doc, filternode->xmlChildrenNode, 1);
					
				if (!xmlStrcmp(filternode->name, (const xmlChar *)"detectobjtype"))
				filter->detectobjtype = (char*)xmlNodeListGetString(doc, filternode->xmlChildrenNode, 1);
					
				if (!xmlStrcmp(filternode->name, (const xmlChar *)"config"))
				filter->config = (char*)xmlNodeListGetString(doc, filternode->xmlChildrenNode, 1);
					
				if (!xmlStrcmp(filternode->name, (const xmlChar *)"function_name")) {
				char *str = (char*)xmlNodeListGetString(doc, filternode->xmlChildrenNode, 1);
				if (!str) {
				filternode = filternode->next;
				continue;
				}
				osync_filter_update_hook(filter, group, str);
				osync_xml_free(str);
				}
					
				if (!xmlStrcmp(filternode->name, (const xmlChar *)"sourcemember")) {
				char *str = (char*)xmlNodeListGetString(doc, filternode->xmlChildrenNode, 1);
				if (!str) {
				filternode = filternode->next;
				continue;
				}
				filter->sourcememberid = atoll(str);
				osync_xml_free(str);
				}
					
				if (!xmlStrcmp(filternode->name, (const xmlChar *)"destmember")) {
				char *str = (char*)xmlNodeListGetString(doc, filternode->xmlChildrenNode, 1);
				if (!str) {
				filternode = filternode->next;
				continue;
				}
				filter->destmemberid = atoll(str);
				osync_xml_free(str);
				}
					
				if (!xmlStrcmp(filternode->name, (const xmlChar *)"action")) {
				char *str = (char*)xmlNodeListGetString(doc, filternode->xmlChildrenNode, 1);
				if (!str) {
				filternode = filternode->next;
				continue;
				}
				filter->action = atoi(str);
				osync_xml_free(str);
				}
				filternode = filternode->next;
				}
				osync_filter_register(group, filter);
				}*/
		
			osync_xml_free(str);
		}
		cur = cur->next;
	}
	osync_xml_free_doc(doc);
	
	/* Check for sanity */
	if (!group->name) {
		osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "Loaded a group without a name");
		goto error;
	}
	
	if (!osync_group_load_members(group, group->configdir, error))
		goto error;
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, group);
	return TRUE;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

void osync_group_add_member(OSyncGroup *group, OSyncMember *member)
{
	osync_assert(group);
	
	if (!osync_member_get_configdir(member)) {
		member->id = osync_group_create_member_id(group);
		char *configdir = osync_strdup_printf("%s%c%i", group->configdir, G_DIR_SEPARATOR, member->id);
		osync_member_set_configdir(member, configdir);
		osync_free(configdir);
	}
	
	group->members = osync_list_append(group->members, member);
	osync_member_ref(member);
}

void osync_group_remove_member(OSyncGroup *group, OSyncMember *member)
{
	osync_assert(group);
	group->members = osync_list_remove(group->members, member);
	osync_member_unref(member);
}

OSyncMember *osync_group_find_member(OSyncGroup *group, osync_memberid id)
{
	OSyncList *m = NULL;
	for (m = group->members; m; m = m->next) {
		OSyncMember *member = m->data;
		if (osync_member_get_id(member) == id)
			return member;
	}
	return NULL;
}

OSyncMember *osync_group_nth_member(OSyncGroup *group, unsigned int nth)
{
	osync_assert(group);
	return (OSyncMember *)osync_list_nth_data(group->members, nth);
}

unsigned int osync_group_num_members(OSyncGroup *group)
{
	osync_assert(group);
	return osync_list_length(group->members);
}

OSyncList *osync_group_get_members(OSyncGroup *group) {
	return osync_list_copy(group->members);
}

const char *osync_group_get_configdir(OSyncGroup *group)
{
	osync_assert(group);
	return group->configdir;
}

void osync_group_set_configdir(OSyncGroup *group, const char *directory)
{
	osync_assert(group);
	if (group->configdir)
		osync_free(group->configdir);
	group->configdir = osync_strdup(directory);
}

unsigned int osync_group_num_objtypes(OSyncGroup *group)
{
	GList *objs = NULL;
	unsigned int len = 0;
	osync_assert(group);
	objs = osync_group_get_supported_objtypes(group);
	len = g_list_length(objs);
	g_list_free(objs);
	return len;
}

const char *osync_group_nth_objtype(OSyncGroup *group, unsigned int nth)
{
	GList *objs = NULL;
	const char *objtype = NULL;
	osync_assert(group);
	objs = osync_group_get_supported_objtypes(group);
	objtype = g_list_nth_data(objs, nth);
	g_list_free(objs);
	return objtype;
	
}

OSyncList *osync_group_get_objtypes(OSyncGroup *group) {
	GList *list = osync_group_get_supported_objtypes(group);
	OSyncList *new_list = NULL;
	
	if (list) {
		OSyncList *last;

		new_list = osync_list_alloc();
		new_list->data = list->data;
		new_list->prev = NULL;
		last = new_list;
		list = list->next;
		while (list) {
			last->next = osync_list_alloc();
			last->next->prev = last;
			last = last->next;
			last->data = list->data;
			list = list->next;
		}
		last->next = NULL;
	}

	return new_list;
}

void osync_group_set_objtype_enabled(OSyncGroup *group, const char *objtype, osync_bool enabled)
{
	OSyncList *m = NULL;
	osync_assert(group);
	/* Loop over all members... */
	for (m = group->members; m; m = m->next) {
		OSyncMember *member = m->data;
		osync_member_set_objtype_enabled(member, objtype, enabled);
	}
}

osync_bool osync_group_objtype_enabled(OSyncGroup *group, const char *objtype)
{
	OSyncList *m = NULL;
	int enabled = -1;
	
	osync_assert(group);
	
	/* What do to:
	 * 
	 * g -> enabled variable
	 * m = value from member
	 * 
	 *   g  -1 0 1 2
	 * m
	 * -1   -1 0 1 2
	 * 0     0 0 1 1
	 * 1     2 1 1 2
	 * 
	 */
	
	/* Loop over all members... */
	for (m = group->members; m; m = m->next) {
		OSyncMember *member = m->data;
		switch (osync_member_objtype_enabled(member, objtype)) {
		case -1:
			//Do nothing;
			break;
		case 0:
			if (enabled == -1)
				enabled = 0;
			else if (enabled == 2)
				enabled = 1;
			break;
		case 1:
			if (enabled == -1)
				enabled = 2;
			else if (enabled == 0)
				enabled = 1;
			break;
		}
	}
	return (enabled > 0) ? TRUE : FALSE;
}

void osync_group_add_filter(OSyncGroup *group, OSyncFilter *filter)
{
	osync_assert(group);
	group->filters = g_list_append(group->filters, filter);
	osync_filter_ref(filter);
}

void osync_group_remove_filter(OSyncGroup *group, OSyncFilter *filter)
{
	osync_assert(group);
	group->filters = g_list_remove(group->filters, filter);
	osync_filter_unref(filter);
}

unsigned int osync_group_num_filters(OSyncGroup *group)
{
	osync_assert(group);
	return g_list_length(group->filters);
}

OSyncFilter *osync_group_nth_filter(OSyncGroup *group, unsigned int nth)
{
	osync_assert(group);
	return g_list_nth_data(group->filters, nth);
}

void osync_group_set_last_synchronization(OSyncGroup *group, time_t last_sync)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %li)", __func__, group, last_sync);
	osync_assert(group);
	
	group->last_sync = last_sync;
	 
	osync_trace(TRACE_EXIT, "%s", __func__);
}

time_t osync_group_get_last_synchronization(OSyncGroup *group)
{
	osync_assert(group);
	return group->last_sync;
}

void osync_group_set_conflict_resolution(OSyncGroup *group, OSyncConflictResolution res, osync_memberid winner)
{
	osync_assert(group);
	group->conflict_resolution = res;
	group->conflict_winner = winner;
}

void osync_group_get_conflict_resolution(OSyncGroup *group, OSyncConflictResolution *res, osync_memberid *winner)
{
	osync_assert(group);
	osync_assert(res);
	osync_assert(winner);
	
	*res = group->conflict_resolution;
	*winner = group->conflict_winner;
}

osync_bool osync_group_get_merger_enabled(OSyncGroup *group)
{
	osync_assert(group);
	
	return group->merger_enabled;
}

void osync_group_set_merger_enabled(OSyncGroup *group, osync_bool merger_enabled)
{
	osync_assert(group);
	
	group->merger_enabled = merger_enabled;
}

osync_bool osync_group_get_converter_enabled(OSyncGroup *group)
{
	osync_assert(group);

	return group->converter_enabled;
}

void osync_group_set_converter_enabled(OSyncGroup *group, osync_bool converter_enabled)
{
	osync_assert(group);

	group->converter_enabled = converter_enabled;
}

osync_bool osync_group_is_uptodate(OSyncGroup *group)
{
	xmlDocPtr doc = NULL;
	xmlNodePtr cur = NULL;
	OSyncError *error = NULL;
	unsigned int version_major;
	unsigned int version_minor;
	xmlChar *version_str = NULL;
	osync_bool uptodate = FALSE;
	char *config = NULL; 
	const char *configdir = NULL;

	osync_assert(group);
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, group);
	
	configdir = osync_group_get_configdir(group);
	if (!configdir){
		osync_trace(TRACE_EXIT, "%s(%p) - No configdir set", __func__, group);
		return FALSE;
	}

	config = osync_strdup_printf("%s%c%s",
	                             configdir,
	                             G_DIR_SEPARATOR, "syncgroup.conf");
	
	/* If syncgroup isn't present, we assume that update is required. */
	if (!osync_xml_open_file(&doc, &cur, config, "syncgroup", &error)) {
		osync_trace(TRACE_ERROR, "%s: %s", __func__, osync_error_print(&error));
		osync_error_unref(&error);
		goto end;
	}

	version_str = xmlGetProp(cur->parent, (const xmlChar *)"version");

	/* No version node, means very outdated version. */
	if (!version_str)
		goto end;

	if (sscanf((const char *) version_str, "%u.%u", &version_major, &version_minor) != 2) {
		/* unparsable version string, can't compare versions,
		   assume update is required */
		osync_trace(TRACE_ERROR, "%s: cannot parse version string: %s", __func__, version_str);
		goto end;
	}

	osync_trace(TRACE_INTERNAL, "Version: %s (current %u.%u required %u.%u)",
	            version_str, version_major, version_minor, 
	            OSYNC_GROUP_MAJOR_VERSION, OSYNC_GROUP_MINOR_VERSION ); 

	if (OSYNC_GROUP_MAJOR_VERSION == version_major 
			&& OSYNC_GROUP_MINOR_VERSION == version_minor)
		uptodate = TRUE;

	osync_xml_free(version_str);
 end:
	osync_free(config);

	if (doc)
		osync_xml_free_doc(doc);

	osync_trace(TRACE_EXIT, "%s(%p)", __func__, group);
	return uptodate;
}
