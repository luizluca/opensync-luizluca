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

#include "opensync-helper.h"
#include "db/opensync_db_internals.h"

#include "opensync_sink_state_db_internals.h"
#include "opensync_sink_state_db_private.h"

static osync_bool osync_sink_states_table_create(
			OSyncSinkStateDB *sinkStateDB,
			OSyncError **error)
{
	char *query = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, sinkStateDB, error);

	/* TODO: How portable to other databes is UNIQUE? */
	query = osync_strdup("CREATE TABLE tbl_sink_states (id INTEGER PRIMARY KEY, key VARCHAR UNIQUE, value VARCHAR, objtype VARCHAR)");

	if (!osync_db_query(sinkStateDB->db, query, error)) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
		osync_free(query);
		return FALSE;
	}

	osync_free(query);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
}

OSyncSinkStateDB *osync_sink_state_db_new(
			const char *filename,
			const char *objtype,
			OSyncError **error)
{
	OSyncSinkStateDB *sinkStateDB = NULL;
	int ret = 0;
	osync_trace(TRACE_ENTRY, "%s(%s, %s, %p)", __func__, __NULLSTR(filename), __NULLSTR(objtype), error);

	sinkStateDB = osync_try_malloc0(sizeof(OSyncSinkStateDB), error);
	if (!sinkStateDB)
		goto error;

	sinkStateDB->ref_count = 1;

	/* Could be NULL, which means object type neutral
	 * or data for the main-sink.
	 */
	if (objtype)
		sinkStateDB->objtype = osync_strdup(objtype);

	sinkStateDB->db = osync_db_new(error);
	if (!sinkStateDB->db)
		goto error;

	if (!osync_db_open(sinkStateDB->db, filename, error)) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
		goto error;
	}

	ret = osync_db_table_exists(sinkStateDB->db, "tbl_sink_states", error);
	if (ret > 0) {
		osync_trace(TRACE_EXIT, "%s: %p", __func__, sinkStateDB->db);
		return sinkStateDB;
		/* error if ret == -1 */
	} else if (ret < 0) {
		goto error;
	}

	/* ret equal 0 means table does not exist yet. continue and create one. */
	if (!osync_sink_states_table_create(sinkStateDB, error))
		goto error;

	osync_trace(TRACE_EXIT, "%s: %p", __func__, sinkStateDB);
	return sinkStateDB;

 error:
	if (sinkStateDB && sinkStateDB->db)
		osync_free(sinkStateDB->db);
	if (sinkStateDB)
		osync_sink_state_db_unref(sinkStateDB);
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

OSyncSinkStateDB *osync_sink_state_db_ref(OSyncSinkStateDB *sinkStateDB)
{
	osync_return_val_if_fail(sinkStateDB, NULL);
	
	g_atomic_int_inc(&(sinkStateDB->ref_count));

	return sinkStateDB;
}

void osync_sink_state_db_unref(OSyncSinkStateDB *sinkStateDB)
{
	osync_return_if_fail(sinkStateDB);
	
	if (g_atomic_int_dec_and_test(&(sinkStateDB->ref_count))) {

		if (!osync_db_close(sinkStateDB->db, NULL))
			osync_trace(TRACE_INTERNAL, "Can't close database");

		if (sinkStateDB->objtype)
			osync_free(sinkStateDB->objtype);

		osync_free(sinkStateDB->db);

		osync_free(sinkStateDB);
	}
}

char *osync_sink_state_get(
		OSyncSinkStateDB *sinkStateDB,
		const char *key,
		OSyncError **error)
{
	char *value = NULL;
	char *query = NULL, *escaped_key;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, sinkStateDB, error);
	osync_assert(sinkStateDB);
	osync_assert(sinkStateDB->db);
	osync_assert(key);

	escaped_key = osync_db_sql_escape(key);
	query = osync_strdup_printf("SELECT value FROM tbl_sink_states WHERE key='%s' AND objtype='%s'",
			escaped_key, sinkStateDB->objtype ? sinkStateDB->objtype : "");
	osync_free(escaped_key);
	value = osync_db_query_single_string(sinkStateDB->db, query, error);
	osync_free(query);

	if (osync_error_is_set(error))
		goto error;

	if (!value)
		value = osync_strdup("");

	osync_trace(TRACE_EXIT, "%s: %s", __func__, value);
	return value;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

osync_bool osync_sink_state_set(
		OSyncSinkStateDB *sinkStateDB,
		const char *key,
		const char *value,
		OSyncError **error)
{
	char *escaped_value = NULL, *escaped_key = NULL;
	char *query = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %s, %p)", __func__, sinkStateDB, __NULLSTR(value), error);
	osync_assert(sinkStateDB);
	osync_assert(sinkStateDB->db);
	osync_assert(key);
	osync_assert(value);

	escaped_key = osync_db_sql_escape(key);
	escaped_value = osync_db_sql_escape(value);
	query = osync_strdup_printf("REPLACE INTO tbl_sink_states (objtype, key, value) VALUES('%s', '%s', '%s')",
			sinkStateDB->objtype ? sinkStateDB->objtype : "", escaped_key, escaped_value);
	osync_free(escaped_value);
	osync_free(escaped_key);

	if (!osync_db_query(sinkStateDB->db, query, error))
		goto error;

	osync_free(query);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_sink_state_equal(
		OSyncSinkStateDB *sinkStateDB,
		const char *key,
		const char *value,
		osync_bool *same,
		OSyncError **error)
{
	char *old_value = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %s, %s, %p, %p)", __func__, sinkStateDB, __NULLSTR(key), __NULLSTR(value), same, error);
	osync_assert(sinkStateDB);
	osync_assert(key);
	osync_assert(value);

	old_value = osync_sink_state_get(sinkStateDB, key, error);
	if (!old_value)
		goto error;

	if (!strcmp(old_value, value))
		*same = TRUE;
	else
		*same = FALSE;

	osync_free(old_value);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

