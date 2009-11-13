/*
 * libopensync - A synchronization framework
 * Copyright (C) 2006  Armin Bauer <armin.bauer@opensync.org>
 * Copyright (C) 2006  NetNix Finland Ltd <netnix@netnix.fi>
 * Copyright (C) 2008  Michael Bell <michael.bell@opensync.org>
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
 * Author: Armin Bauer <armin.bauer@opensync.org>
 * Author: Daniel Friedrich <daniel.friedrich@opensync.org>
 * 
 */
 
#include "opensync.h"
#include "opensync_internals.h"

#include "opensync_archive_private.h"
#include "opensync_archive_internals.h"
#include "db/opensync_db_internals.h"

static osync_bool osync_archive_create_changes(OSyncDB *db, const char *objtype, OSyncError **error)
{
	int ret = 0;
	const char *query = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %s, %p)", __func__, db, objtype, error); 

	osync_assert(db);
	osync_assert(objtype);
	osync_assert(strlen(objtype) <= 64);

	ret = osync_db_table_exists(db, "tbl_changes", error);

	/* error if ret -1 */
	if (ret < 0)
		goto error;

	/* if ret is != 0 then the table already exist */
	if (ret) {
		osync_trace(TRACE_EXIT, "%s", __func__);
		return TRUE;
	}

	/* The primary key is conceptually (objtype, id).  SQLite3 will only AUTOINCREMENT
	 * a single integer primary key so the primary key restricted to being id.  To
	 * enforce the entity-relationship model all queries for a single item should be:
	 * WHERE objtype='%s' AND id=%lli
	 */ 
	query = "CREATE TABLE tbl_changes (objtype VARCHAR(64) NOT NULL, id INTEGER PRIMARY KEY AUTOINCREMENT, uid VARCHAR NOT NULL, memberid INTEGER NOT NULL, mappingid INTEGER NOT NULL, objengine VARCHAR(64) NOT NULL )";
	if (!osync_db_query(db, query, error)) {
		goto error;
	}

	osync_trace(TRACE_EXIT, "%s: created table.", __func__);
	return TRUE;

 error:	
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool osync_archive_create_changelog(OSyncDB *db, const char *objtype, OSyncError **error)
{
	int ret = 0;
	const char *query = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %s, %p)", __func__, db, objtype, error); 

	osync_assert(db);
	osync_assert(objtype);
	osync_assert(strlen(objtype) <= 64);

	ret = osync_db_table_exists(db, "tbl_changelog", error);

	/* error if ret -1 */
	if (ret < 0)
		goto error;

	/* if ret != 0 table does not exist. continue and create it */
	if (ret) {
		osync_trace(TRACE_EXIT, "%s", __func__);
		return TRUE;
	}

	query = "CREATE TABLE tbl_changelog (objtype VARCHAR(64) NOT NULL, memberid INTEGER NOT NULL, mappingid INTEGER NOT NULL, changetype INTEGER NOT NULL, PRIMARY KEY (objtype, memberid, mappingid) )";
	if (!osync_db_query(db, query, error)) {
		goto error;
	}

	osync_trace(TRACE_EXIT, "%s: created table.", __func__);
	return TRUE;

 error:	
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool osync_archive_create(OSyncDB *db, const char *objtype, OSyncError **error)
{
	int ret = 0;
	const char *query = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %s, %p)", __func__, db, objtype, error); 

	osync_assert(db);
	osync_assert(objtype);
	osync_assert(strlen(objtype) <= 64);

	ret = osync_db_table_exists(db, "tbl_archive", error);

	/* error if ret -1 */
	if (ret < 0)
		goto error;

	/* if ret != 0 table does not exist. continue and create it */
	if (ret) {
		osync_trace(TRACE_EXIT, "%s", __func__); 
		return TRUE;
	}


	query = "CREATE TABLE tbl_archive (objtype VARCHAR(64), mappingid INTEGER, data BLOB, PRIMARY KEY (objtype, mappingid) )";
	if (!osync_db_query(db, query, error)) {
		goto error;
	}

	osync_trace(TRACE_EXIT, "%s: created table.", __func__); 
	return TRUE;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

OSyncArchive *osync_archive_new(const char *filename, OSyncError **error)
{
	OSyncArchive *archive = NULL;
	osync_trace(TRACE_ENTRY, "%s(%s, %p)", __func__, filename, error);
	osync_assert(filename);
	
	archive = osync_try_malloc0(sizeof(OSyncArchive), error);
	if (!archive)
		goto error;

	archive->ref_count = 1;
	
	archive->db = osync_db_new(error);
	if (!archive->db)
		goto error_and_free;

	if (!osync_db_open(archive->db, filename, error)) {
		osync_free(archive->db);
		goto error_and_free;
	}

	osync_trace(TRACE_EXIT, "%s: %p", __func__, archive);
	return archive;

 error_and_free:	
	osync_free(archive);

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

OSyncArchive *osync_archive_ref(OSyncArchive *archive)
{
	osync_assert(archive);
	
	g_atomic_int_inc(&(archive->ref_count));

	return archive;
}

void osync_archive_unref(OSyncArchive *archive)
{
	osync_assert(archive);
	
	if (g_atomic_int_dec_and_test(&(archive->ref_count))) {
		osync_trace(TRACE_ENTRY, "%s(%p)", __func__, archive);
		
		if (archive->db) {
			if (!osync_db_close(archive->db, NULL))	
				osync_trace(TRACE_INTERNAL, "Can't close database");
		}
		
		osync_free(archive->db);
		osync_free(archive);

		osync_trace(TRACE_EXIT, "%s", __func__);
	}
}

osync_bool osync_archive_save_data(OSyncArchive *archive, long long int id, const char *objtype, const char *data, unsigned int size, OSyncError **error)
{
	char *query = NULL;
	char *escaped_objtype = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %lli, %s, %p, %u, %p)", __func__, archive, id, objtype, data, size, error);
	osync_assert(archive);
	osync_assert(data);
	osync_assert(size);

	if (!osync_archive_create(archive->db, objtype, error))
		goto error;

	// FIXME: Avoid subselect - this query needs up to 0.5s
	escaped_objtype = osync_db_sql_escape(objtype);
	query = osync_strdup_printf("REPLACE INTO tbl_archive (objtype, mappingid, data) VALUES('%s', %lli, ?)", escaped_objtype, id);
	osync_free(escaped_objtype);
	escaped_objtype = NULL;
	
	if (!osync_db_bind_blob(archive->db, query, data, size, error)) {
		osync_free(query);
		goto error;
	}

	osync_free(query);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

int osync_archive_load_data(OSyncArchive *archive, const char *uid, const char *objtype, char **data, unsigned int *size, OSyncError **error)
{
	char *escaped_uid = NULL;
	char *escaped_objtype = NULL;
	char *query = NULL;
	int ret = 0;

	osync_trace(TRACE_ENTRY, "%s(%p, %s, %s, %p, %p, %p)", __func__, archive, uid, objtype, data, size, error);
	osync_assert(archive);
	osync_assert(uid);
	osync_assert(data);
	osync_assert(size);

	if (!osync_archive_create(archive->db, objtype, error))
		goto error;

	escaped_uid = osync_db_sql_escape(uid);
	escaped_objtype = osync_db_sql_escape(objtype);
	query = osync_strdup_printf("SELECT data FROM tbl_archive WHERE objtype='%s' AND mappingid=(SELECT mappingid FROM tbl_changes WHERE objtype='%s' AND uid='%s' LIMIT 1)", escaped_objtype, escaped_objtype, escaped_uid);
	osync_free(escaped_objtype);

	escaped_objtype = NULL;

	ret = osync_db_get_blob(archive->db, query, data, size, error);

	osync_free(query);
	osync_free(escaped_uid);

	if (ret < 0) {
		goto error;
	} else if (ret == 0) {
		osync_trace(TRACE_EXIT, "%s: no data stored in archive.", __func__); 
		return 0;
	}

	osync_trace(TRACE_EXIT, "%s", __func__);
	return 1;
	
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return -1;
}

long long int osync_archive_save_change(OSyncArchive *archive, long long int id, const char *uid, const char *objtype, long long int mappingid, long long int memberid, const char *objengine, OSyncError **error)
{
	char *query = NULL;
	char *escaped_uid = NULL;
	char *escaped_objtype = NULL;
	char *escaped_objengine = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %lli, %s, %s, %lli, %lli, %p, %p)", __func__, archive, id, uid, objtype, mappingid, memberid, __NULLSTR(objengine), error);
	osync_assert(archive);
	osync_assert(uid);
	osync_assert(objtype);
	osync_assert(objengine);

	if (!osync_archive_create_changes(archive->db, objtype, error))
		goto error;

	escaped_uid = osync_db_sql_escape(uid);
	escaped_objtype = osync_db_sql_escape(objtype);
	escaped_objengine = osync_db_sql_escape(objengine);

	if (!id) {
		query = osync_strdup_printf("INSERT INTO tbl_changes (objtype, uid, mappingid, memberid, objengine) VALUES('%s', '%s', '%lli', '%lli', '%s')", escaped_objtype, escaped_uid, mappingid, memberid, objengine);
	} else {
		query = osync_strdup_printf("UPDATE tbl_changes SET uid='%s', mappingid='%lli', memberid='%lli', objengine='%s' WHERE objtype='%s' AND id=%lli", escaped_uid, mappingid, memberid, escaped_objengine, escaped_objtype, id);
	}
	osync_free(escaped_objengine);
	osync_free(escaped_objtype);
	osync_free(escaped_uid);
	escaped_objengine = NULL;
	escaped_objtype = NULL;
	escaped_uid = NULL;
	
	if (!osync_db_query(archive->db, query, error)) {
		osync_free(query);
		goto error;
	}

	osync_free(query);
	
	if (!id)
		id = osync_db_last_rowid(archive->db);
	
	osync_trace(TRACE_EXIT, "%s: %lli", __func__, id);
	return id;
	
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return 0;
}

osync_bool osync_archive_delete_change(OSyncArchive *archive, long long int id, const char *objtype, OSyncError **error)
{
	char *query = NULL;
	char *escaped_objtype = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %lli, %s, %p)", __func__, archive, id, objtype, error);
	osync_assert(archive);
	osync_assert(objtype);

	if (!osync_archive_create_changes(archive->db, objtype, error))
		goto error;

	escaped_objtype = osync_db_sql_escape(objtype);
	query = osync_strdup_printf("DELETE FROM tbl_changes WHERE objtype='%s' AND id=%lli", escaped_objtype, id);
	osync_free(escaped_objtype);
	escaped_objtype = NULL;
	if (!osync_db_query(archive->db, query, error)) {
		osync_free(query);
		goto error;
	}

	osync_free(query);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error:	
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_archive_load_changes(OSyncArchive *archive, const char *objtype, OSyncList **ids, OSyncList **uids, OSyncList **mappingids, OSyncList **memberids, OSyncError **error)
{
	char *query = NULL;
	char *escaped_objtype = NULL;
	OSyncList *result = NULL, *row = NULL, *column = NULL;
	long long int id = 0, mappingid = 0, memberid = 0;
	const char *uid = NULL;
	const char *value_str = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %s, %p, %p, %p, %p, %p)", __func__, archive, objtype, ids, uids, mappingids, memberids, error);

	osync_assert(archive);
	osync_assert(objtype);
	osync_assert(ids);
	osync_assert(uids);
	osync_assert(mappingids);
	osync_assert(memberids);

	*ids = NULL;
	*uids = NULL;
	*mappingids = NULL;
	*memberids = NULL;

	if (!osync_archive_create_changes(archive->db, objtype, error))
		goto error;

	escaped_objtype = osync_db_sql_escape(objtype);
	query = osync_strdup_printf("SELECT id, uid, mappingid, memberid FROM tbl_changes WHERE objtype='%s' ORDER BY mappingid", escaped_objtype);
	osync_free(escaped_objtype);
	escaped_objtype = NULL;
	result = osync_db_query_table(archive->db, query, error);

	osync_free(query);
	
	/* Check for error of osync_db_query_table() call. */
	if (osync_error_is_set(error))
		goto error;

	for (row = result; row; row = row->next) { 
		column = row->data;

		value_str = osync_list_nth_data(column, 0);
		if (!value_str) {
			osync_error_set(error, OSYNC_ERROR_GENERIC, "Database table tbl_changes corrupt, id is NULL");
			goto error;
		}
		id = g_ascii_strtoull(value_str, NULL, 0);

		uid = osync_list_nth_data(column, 1);
		if (!uid) {
			osync_error_set(error, OSYNC_ERROR_GENERIC, "Database table tbl_changes corrupt, uid is NULL");
			goto error;
		}

		value_str = osync_list_nth_data(column, 2);
		if (!value_str) {
			osync_error_set(error, OSYNC_ERROR_GENERIC, "Database table tbl_changes corrupt, mappingid is NULL");
			goto error;
		}
		mappingid = g_ascii_strtoull(value_str, NULL, 0);

		value_str = osync_list_nth_data(column, 3);
		if (!value_str) {
			osync_error_set(error, OSYNC_ERROR_GENERIC, "Database table tbl_changes corrupt, memberid is NULL");
			goto error;
		}
		memberid = g_ascii_strtoull(value_str, NULL, 0);
		
		*ids = osync_list_append((*ids), GINT_TO_POINTER((int)id));
		*uids = osync_list_append((*uids), osync_strdup(uid));
		*mappingids = osync_list_append((*mappingids), GINT_TO_POINTER((int)mappingid));
		*memberids = osync_list_append((*memberids), GINT_TO_POINTER((int)memberid));
		
		osync_trace(TRACE_INTERNAL, "Loaded change with uid %s, mappingid %lli from member %lli", uid, mappingid, memberid);
	}

	osync_db_free_list(result);	

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;	
}

osync_bool osync_archive_flush_changes(OSyncArchive *archive, const char *objtype, OSyncError **error)
{
	char *query = NULL;
	char *escaped_objtype = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %s, %p)", __func__, archive, objtype, error);
	osync_assert(archive);
	osync_assert(objtype);

	if (!osync_archive_create_changes(archive->db, objtype, error))
		goto error;
	
	escaped_objtype = osync_db_sql_escape(objtype);
	query = osync_strdup_printf("DELETE FROM tbl_changes WHERE objtype='%s'", escaped_objtype);
	osync_free(escaped_objtype);
	escaped_objtype = NULL;
	
	if (!osync_db_query(archive->db, query, error)) {
		osync_free(query);
		goto error;
	}

	osync_free(query);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
	
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_archive_load_ignored_conflicts(OSyncArchive *archive, const char *objtype, OSyncList **memberids, OSyncList **mappingids, OSyncList **changetypes, OSyncError **error)
{
	OSyncList *result = NULL, *row = NULL, *column = NULL;
	char *query = NULL;
	char *escaped_objtype = NULL;
	long long int mappingid = 0, memberid = 0;
	int changetype = 0;

	osync_trace(TRACE_ENTRY, "%s(%p, %s, %p, %p)", __func__, archive, objtype, mappingids, error);

	osync_assert(archive);
	osync_assert(objtype);
	osync_assert(memberids);
	osync_assert(mappingids);
	osync_assert(changetypes);

	if (!osync_archive_create_changelog(archive->db, objtype, error))
		goto error;

	escaped_objtype = osync_db_sql_escape(objtype);
	query = osync_strdup_printf("SELECT memberid, mappingid, changetype FROM tbl_changelog WHERE objtype='%s' ORDER BY mappingid", escaped_objtype);
	osync_free(escaped_objtype);
	escaped_objtype = NULL;
	result = osync_db_query_table(archive->db, query, error);

	osync_free(query);
	
	/* Check for error of osync_db_query_table() call. */
	if (osync_error_is_set(error))
		goto error;

	for (row = result; row; row = row->next) { 
		column = row->data;

		memberid = g_ascii_strtoull(osync_list_nth_data(column, 0), NULL, 0);
		mappingid = atoi(osync_list_nth_data(column, 1));
		changetype = atoi(osync_list_nth_data(column, 2));
		
		*memberids = osync_list_append((*memberids), GINT_TO_POINTER((int)memberid));
		*mappingids = osync_list_append((*mappingids), GINT_TO_POINTER((int)mappingid));
		*changetypes = osync_list_append((*changetypes), GINT_TO_POINTER((int)changetype));
		
		osync_trace(TRACE_INTERNAL, "Loaded ignored mapping with mappingid %lli", mappingid);
	}

	osync_db_free_list(result);	

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;	
}

osync_bool osync_archive_save_ignored_conflict(OSyncArchive *archive, const char *objtype, long long int memberid, long long int mappingid, OSyncChangeType changetype, OSyncError **error)
{
	char *query = NULL;
	char *escaped_objtype = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %s, %lli, %lli, %p)", __func__, archive, objtype, memberid, mappingid, error);

	osync_assert(archive);
	osync_assert(objtype);

	if (!osync_archive_create_changelog(archive->db, objtype, error))
		goto error;
	
	escaped_objtype = osync_db_sql_escape(objtype);
	query = osync_strdup_printf("INSERT INTO tbl_changelog (objtype, memberid, mappingid, changetype) VALUES('%s', '%lli', '%lli', '%i')", escaped_objtype, memberid, mappingid, changetype);
	osync_free(escaped_objtype);
	escaped_objtype = NULL;
	
	if (!osync_db_query(archive->db, query, error)) {
		osync_free(query);
		goto error;
	}

	osync_free(query);
	
	osync_trace(TRACE_EXIT, "%s: %lli", __func__, mappingid);
	return TRUE;
	
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_archive_flush_ignored_conflict(OSyncArchive *archive, const char *objtype, OSyncError **error)
{
	char *query = NULL;
	char *escaped_objtype = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %s, %p)", __func__, archive, objtype, error);
	osync_assert(archive);
	osync_assert(objtype);

	if (!osync_archive_create_changelog(archive->db, objtype, error))
		goto error;
	
	escaped_objtype = osync_db_sql_escape(objtype);
	query = osync_strdup_printf("DELETE FROM tbl_changelog WHERE objtype='%s'", escaped_objtype);
	osync_free(escaped_objtype);
	escaped_objtype = NULL;
	
	if (!osync_db_query(archive->db, query, error)) {
		osync_free(query);
		goto error;
	}

	osync_free(query);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
	
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_archive_get_mixed_objengines(OSyncArchive *archive, const char *objengine, OSyncList **objengines, OSyncError **error)
{
	char *query = NULL, *escaped_objengine, *objengine_name;
	OSyncList *result, *row, *column;

	osync_assert(archive);
	osync_assert(objengine);
	osync_assert(objengines);

	escaped_objengine = osync_db_sql_escape(objengine);

	query = osync_strdup_printf("SELECT DISTINCT(b.objengine) FROM tbl_changes, tbl_changes as a, tbl_changes as b "
			"WHERE a.mappingid == b.mappingid AND a.objengine == '%s';", escaped_objengine);
	result = osync_db_query_table(archive->db, query, error);
	osync_free(query);
	
	/* Check for error of osync_db_query_table() call. */
	if (osync_error_is_set(error))
		goto error;

	for (row = result; row; row = row->next) { 
		column = row->data;

		objengine_name = osync_list_nth_data(column, 0);
		if (!objengine_name) {
			osync_error_set(error, OSYNC_ERROR_GENERIC, "Database table tbl_changes corrupt. Couldn't query for mixed object engines.");
			goto error;
		}
		
		*objengines = osync_list_append((*objengines), osync_strdup(objengine_name));
	}

	osync_db_free_list(result);	


	return TRUE;
error:
	return FALSE;
}

osync_bool osync_archive_update_change_uid(OSyncArchive *archive, const char *olduid, const char *newuid, long long int memberid, const char *objengine, OSyncError **error)
{
	char *query = NULL;
	char *escaped_newuid = NULL;
	char *escaped_olduid = NULL;
	char *escaped_objengine = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %s, %s, %lli, %s, %p)", __func__, archive, __NULLSTR(olduid), __NULLSTR(newuid), memberid, __NULLSTR(objengine), error);
	osync_assert(archive);
	osync_assert(olduid);
	osync_assert(newuid);
	osync_assert(objengine);

	escaped_newuid = osync_db_sql_escape(newuid);
	escaped_olduid = osync_db_sql_escape(olduid);
	escaped_objengine = osync_db_sql_escape(objengine);

	query = osync_strdup_printf("UPDATE tbl_changes SET uid='%s' WHERE objengine='%s' AND memberid=%lli AND uid='%s'", escaped_newuid, escaped_objengine, memberid, escaped_olduid);

	osync_free(escaped_objengine);
	osync_free(escaped_olduid);
	osync_free(escaped_newuid);
	escaped_objengine = NULL;
	escaped_olduid = NULL;
	escaped_newuid = NULL;
	
	if (!osync_db_query(archive->db, query, error)) {
		osync_free(query);
		goto error;
	}

	osync_free(query);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
	
error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}


