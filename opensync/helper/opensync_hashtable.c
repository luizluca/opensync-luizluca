/*
 * libopensync - A synchronization framework
 * Copyright (C) 2004-2005  Armin Bauer <armin.bauer@opensync.org>
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

#include "opensync.h"
#include "opensync_internals.h"
#include "opensync_hashtable_internals.h"
#include "opensync_hashtable_private.h"

#include "opensync-data.h"
#include "opensync-helper.h"
#include "opensync-db.h"

/* start private api */

static osync_bool osync_hashtable_create(OSyncHashTable *table, OSyncError **error)
{
	char *query = NULL;
	osync_assert(table);
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, table, error);

	query = osync_strdup_printf("CREATE TABLE %s (id INTEGER PRIMARY KEY, uid VARCHAR UNIQUE, hash VARCHAR)", table->name);
	if (!osync_db_query(table->dbhandle, query, error)) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
		osync_free(query);
		return FALSE;
	}

	osync_free(query);
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
}

#if !GLIB_CHECK_VERSION(2,12,0)
static gboolean remove_entry(gpointer key, gpointer val, gpointer data)
{
	return TRUE;
}
#endif

static void osync_hashtable_reset_reports(OSyncHashTable *table)
{
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, table);
	osync_assert(table);
	osync_assert(table->dbhandle);

	/* Only free the internal hashtable of reported entries.
	   Don't flush the real database. */
#if GLIB_CHECK_VERSION(2,12,0)
	g_hash_table_remove_all(table->reported_entries);
#else
	g_hash_table_foreach_remove(table->reported_entries, remove_entry, NULL);
#endif

	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void osync_hashtable_reset(OSyncHashTable *table)
{
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, table);
	osync_assert(table);
	osync_assert(table->dbhandle);

	/* Only free the internal hashtable of reported entries.
	   Don't flush the real database. */
#if GLIB_CHECK_VERSION(2,12,0)
	g_hash_table_remove_all(table->db_entries);
#else
	g_hash_table_foreach_remove(table->db_entries, remove_entry, NULL);
#endif

	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void osync_hashtable_report(OSyncHashTable *table, OSyncChange *change)
{
	char *uid = NULL;
	osync_assert(table);
	osync_assert(table->dbhandle);
	osync_assert(change);

	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, table, change);

	/* uid get freed when hashtable get's destroyed */
	uid = osync_strdup(osync_change_get_uid(change));

	g_hash_table_insert(table->reported_entries, uid, GINT_TO_POINTER(1));

	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void _osync_hashtable_prepare_insert_query(const char *uid, const char *hash, void *user_data)
{
	OSyncHashTable *table = user_data;

	char *escaped_uid = osync_db_sql_escape(uid);
	char *escaped_hash = osync_db_sql_escape(hash);

	g_string_append_printf(table->query,
	                       "REPLACE INTO %s ('uid', 'hash') VALUES('%s', '%s');",
	                       table->name, escaped_uid, escaped_hash);

	osync_free(escaped_uid);
	osync_free(escaped_hash);
}

/* end private api */

OSyncHashTable *osync_hashtable_new(const char *path, const char *objtype, OSyncError **error)
{
	OSyncHashTable *table = NULL;
	int ret = 0;
	osync_trace(TRACE_ENTRY, "%s(%s, %p)", __func__, path, error);

	table = osync_try_malloc0(sizeof(OSyncHashTable), error);
	if (!table)
		goto error;

	table->ref_count = 1;


	table->reported_entries = g_hash_table_new_full(g_str_hash, g_str_equal, osync_free, NULL);
	table->db_entries = g_hash_table_new_full(g_str_hash, g_str_equal, osync_free, osync_free);

	table->dbhandle = osync_db_new(error);
	if (!table->dbhandle)
		goto error_and_free_db;

	if (!osync_db_open(table->dbhandle, path, error))
		goto error_and_free;

	table->name = osync_strdup_printf(OSYNC_HASHTABLE_DB_PREFIX"%s", objtype);

	ret = osync_db_table_exists(table->dbhandle, table->name, error);
	/* greater then 0 means evrything is O.k. */

	if (ret < 0)
		goto error;
	else if (ret == 0)
		/* if ret == 0 then table does not exist yet. contiune and create one. */
		if (!osync_hashtable_create(table, error))
			goto error;


	osync_trace(TRACE_EXIT, "%s: %p", __func__, table);
	return table;

 error_and_free_db:
	/* TODO OSyncDB reference counting / free */
	osync_free(table->dbhandle);
 error_and_free:
	osync_free(table);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;

}

OSyncHashTable *osync_hashtable_ref(OSyncHashTable *table)
{
	osync_assert(table);

	g_atomic_int_inc(&(table->ref_count));

	return table;
}

void osync_hashtable_unref(OSyncHashTable *table)
{
	osync_assert(table);

	if (g_atomic_int_dec_and_test(&(table->ref_count))) {
		OSyncError *error = NULL;
		osync_trace(TRACE_ENTRY, "%s(%p)", __func__, table);

		if (!osync_db_close(table->dbhandle, &error)) {
			osync_trace(TRACE_ERROR, "Couldn't close database: %s", osync_error_print(&error));
			osync_error_unref(&error);
		}

		g_hash_table_destroy(table->reported_entries);
		g_hash_table_destroy(table->db_entries);

		osync_free(table->name);
		/* TODO OSyncDB reference counting / free */
		osync_free(table->dbhandle);
		osync_free(table);

		osync_trace(TRACE_EXIT, "%s", __func__);
	}

}

osync_bool osync_hashtable_load(OSyncHashTable *table, OSyncError **error)
{
	char *query;
	OSyncList *row, *result;

	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, table, error);

	query = osync_strdup_printf("SELECT uid, hash FROM %s", table->name);
	result = osync_db_query_table(table->dbhandle, query, error);
	osync_free(query);

	/* If result is NULL, this means no entries - just check for error. */
	if (osync_error_is_set(error))
		goto error;

	for (row = result; row; row = row->next) {
		OSyncList *column = row->data;

		char *uid =  osync_strdup(osync_list_nth_data(column, 0));
		char *hash = osync_strdup(osync_list_nth_data(column, 1));

		g_hash_table_insert(table->db_entries, uid, hash);
	}
	osync_list_free(result);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_hashtable_save(OSyncHashTable *table, OSyncError **error)
{
	osync_bool ret;
	char *query = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, table, error);

	/* Should only be used by this function */
	osync_assert(!table->query);

	table->query = g_string_new("BEGIN TRANSACTION;");
	g_string_append_printf(table->query, "DELETE FROM %s;", table->name);

	osync_hashtable_foreach(table, _osync_hashtable_prepare_insert_query, table);

	table->query = g_string_append(table->query, "COMMIT TRANSACTION;");

	query = g_string_free(table->query, FALSE);
	ret = osync_db_query(table->dbhandle, query, error);
	g_free(query);

	table->query = NULL;

	if (!ret)
		goto error;

	osync_hashtable_reset_reports(table);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}


osync_bool osync_hashtable_slowsync(OSyncHashTable *table, OSyncError **error)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, table, error);
	osync_assert(table);
	osync_assert(table->dbhandle);

	/* Reset persistent hashtable in database */
	if (!osync_db_reset_table(table->dbhandle, table->name, error))
		goto error;

	/* Reset hashtable in memory */
	osync_hashtable_reset(table);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;

}

void osync_hashtable_update_change(OSyncHashTable *table, OSyncChange *change)
{
	const char *uid = NULL;
	const char *hash = NULL;

	osync_assert(table);
	osync_assert(table->dbhandle);
	osync_assert(change);

	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, table, change);

	uid = osync_change_get_uid(change);
	hash = osync_change_get_hash(change);

	osync_assert_msg(uid, "Some plugin forgot to set the UID for the change. Please report this bug.");

	switch (osync_change_get_changetype(change)) {
	case OSYNC_CHANGE_TYPE_DELETED:
		g_hash_table_remove(table->db_entries, uid);
		break;
	case OSYNC_CHANGE_TYPE_UNMODIFIED:
		/* Nothing to do. Just ignore. */
		break;
	case OSYNC_CHANGE_TYPE_UNKNOWN:
		/* Someone violets against the rules of the hashtable API!

		   Changetype needs to get set before calling this function!
		   Even if the change entry got not modified, then the change type
		   should get set at least to OSYNC_CHANGE_TYPE_UNMODIFIED. Otherwise:

		   BOOOOOOOOOOOOOOOOOOOOOM!

		*/
		osync_assert_msg(FALSE, "Got called with unknown changetype. This looks like a plugin makes wrong use of a hashtable. Please, contact the plugin author!");
		break;
	case OSYNC_CHANGE_TYPE_MODIFIED:
		osync_assert_msg(hash, "Some plugin forgot to set the HASH for the change for the changetype MODIFIED. Please report this bug.");
		/* This works even if the UID/key is new to the hashtable */
		g_hash_table_replace(table->db_entries, osync_strdup(uid), osync_strdup(hash));
		break;
	case OSYNC_CHANGE_TYPE_ADDED:
		osync_assert_msg(hash, "Some plugin forgot to set the HASH for the change for the changetype ADDED. Please report this bug.");
		g_hash_table_insert(table->db_entries, osync_strdup(uid), osync_strdup(hash));
		break;
	}

	osync_hashtable_report(table, change);

	osync_trace(TRACE_EXIT, "%s", __func__);
}

struct callback_data {
	OSyncList *deleted_entries;
	OSyncHashTable *table;
};
static void callback_check_deleted(gpointer key, gpointer value, gpointer user_data)
{
	struct callback_data *cbdata = user_data;
	if (!g_hash_table_lookup(cbdata->table->reported_entries, key))
		cbdata->deleted_entries = osync_list_prepend(cbdata->deleted_entries, key);
}
OSyncList *osync_hashtable_get_deleted(OSyncHashTable *table)
{
	struct callback_data cbdata = {NULL, table};
	osync_assert(table);
	osync_assert(table->dbhandle);

	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, table);

	g_hash_table_foreach(table->db_entries, callback_check_deleted, &cbdata);

	osync_trace(TRACE_EXIT, "%s: %p", __func__, cbdata.deleted_entries);
	return cbdata.deleted_entries;
}

OSyncChangeType osync_hashtable_get_changetype(OSyncHashTable *table, OSyncChange *change)
{
	OSyncChangeType retval = OSYNC_CHANGE_TYPE_UNKNOWN;
	const char *uid = NULL;
	const char *newhash = NULL;
	const char *orighash = NULL;

	osync_assert(table);
	osync_assert(table->dbhandle);
	osync_assert(change);

	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, table, change);

	uid = osync_change_get_uid(change);
	newhash = osync_change_get_hash(change);
	orighash = osync_hashtable_get_hash(table, uid);
	if (orighash) {
		if (!strcmp(newhash, orighash))
			retval = OSYNC_CHANGE_TYPE_UNMODIFIED;
		else
			retval = OSYNC_CHANGE_TYPE_MODIFIED;
	} else
		retval = OSYNC_CHANGE_TYPE_ADDED;


	osync_trace(TRACE_EXIT, "%s: %i", __func__, retval);
	return retval;
}

unsigned int osync_hashtable_num_entries(OSyncHashTable *table)
{
	osync_assert(table);
	return g_hash_table_size(table->db_entries);
}

void osync_hashtable_foreach(OSyncHashTable *table, OSyncHashtableForEach func, void *user_data)
{
	osync_assert(table);
	g_hash_table_foreach(table->db_entries, (GHFunc) func, user_data);
}

const char *osync_hashtable_get_hash(OSyncHashTable *table, const char *uid)
{
	osync_assert(table);
	osync_assert(uid);

	return (const char *)  g_hash_table_lookup(table->db_entries, uid);
}

/*@}*/
