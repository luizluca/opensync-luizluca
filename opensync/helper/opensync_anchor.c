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

#include "opensync_anchor_internals.h"
#include "opensync_anchor_private.h"

osync_bool osync_anchor_create(OSyncAnchor *anchor, OSyncError **error)
{
	char *query = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, anchor, error);

	/* TODO: How portable to other databes is UNIQUE? */
	query = osync_strdup("CREATE TABLE tbl_anchor (id INTEGER PRIMARY KEY, key VARCHAR UNIQUE, value VARCHAR, objtype VARCHAR)");

	if (!osync_db_query(anchor->db, query, error)) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
		osync_free(query);
		return FALSE;
	}

	osync_free(query);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
}

OSyncAnchor *osync_anchor_new(const char *filename, const char *objtype, OSyncError **error)
{
	OSyncAnchor *anchor = NULL;
	int ret = 0;
	osync_trace(TRACE_ENTRY, "%s(%s, %s, %p)", __func__, __NULLSTR(filename), __NULLSTR(objtype), error);

	anchor = osync_try_malloc0(sizeof(OSyncAnchor), error);
	if (!anchor)
		goto error;

	anchor->ref_count = 1;

	/* Could be NULL, which means object type neutral
	 * or data for the main-sink.
	 */
	if (objtype)
		anchor->objtype = osync_strdup(objtype);

	anchor->db = osync_db_new(error);
	if (!anchor->db)
		goto error_free_anchor;

	if (!osync_db_open(anchor->db, filename, error)) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
		goto error_free_db;
	}

	ret = osync_db_table_exists(anchor->db, "tbl_anchor", error);
	if (ret > 0) {
		osync_trace(TRACE_EXIT, "%s: %p", __func__, anchor->db);
		return anchor;
		/* error if ret == -1 */
	} else if (ret < 0) {
		goto error_free_db;
	}

	/* ret equal 0 means table does not exist yet. continue and create one. */
	if (!osync_anchor_create(anchor, error))
		goto error_free_db;

	osync_trace(TRACE_EXIT, "%s: %p", __func__, anchor);
	return anchor;

 error_free_db:
	osync_free(anchor->db);
 error_free_anchor:
	osync_anchor_unref(anchor);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

OSyncAnchor *osync_anchor_ref(OSyncAnchor *anchor)
{
	osync_return_val_if_fail(anchor, NULL);
	
	g_atomic_int_inc(&(anchor->ref_count));

	return anchor;
}

void osync_anchor_unref(OSyncAnchor *anchor)
{
	osync_return_if_fail(anchor);
	
	if (g_atomic_int_dec_and_test(&(anchor->ref_count))) {

		if (!osync_db_close(anchor->db, NULL))
			osync_trace(TRACE_INTERNAL, "Can't close database");

		if (anchor->objtype)
			osync_free(anchor->objtype);

		osync_free(anchor->db);

		osync_free(anchor);
	}
}

char *osync_anchor_retrieve(OSyncAnchor *anchor, const char *key, OSyncError **error)
{
	char *retanchor = NULL;
	char *query = NULL, *escaped_key;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, anchor, error);
	osync_assert(anchor);
	osync_assert(anchor->db);
	osync_assert(key);

	escaped_key = osync_db_sql_escape(key);
	query = osync_strdup_printf("SELECT value FROM tbl_anchor WHERE key='%s' AND objtype='%s'",
			escaped_key, anchor->objtype ? anchor->objtype : "");
	osync_free(escaped_key);
	retanchor = osync_db_query_single_string(anchor->db, query, error);
	osync_free(query);

	if (osync_error_is_set(error))
		goto error;

	if (!retanchor)
		retanchor = osync_strdup("");

	osync_trace(TRACE_EXIT, "%s: %s", __func__, retanchor);
	return retanchor;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

osync_bool osync_anchor_update(OSyncAnchor *anchor, const char *key, const char *value, OSyncError **error)
{
	char *escaped_value = NULL, *escaped_key = NULL;
	char *query = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %s, %p)", __func__, anchor, __NULLSTR(value), error);
	osync_assert(anchor);
	osync_assert(anchor->db);
	osync_assert(key);
	osync_assert(value);

	escaped_key = osync_db_sql_escape(key);
	escaped_value = osync_db_sql_escape(value);
	query = osync_strdup_printf("REPLACE INTO tbl_anchor (objtype, key, value) VALUES('%s', '%s', '%s')",
			anchor->objtype ? anchor->objtype : "", escaped_key, escaped_value);
	osync_free(escaped_value);
	osync_free(escaped_key);

	if (!osync_db_query(anchor->db, query, error))
		goto error;

	osync_free(query);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_anchor_compare(OSyncAnchor *anchor, const char *key, const char *value, osync_bool *same, OSyncError **error)
{
	char *old_value = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %s, %s, %p, %p)", __func__, anchor, __NULLSTR(key), __NULLSTR(value), same, error);
	osync_assert(anchor);
	osync_assert(key);
	osync_assert(value);

	old_value = osync_anchor_retrieve(anchor, key, error);
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

