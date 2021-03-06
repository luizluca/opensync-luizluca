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

#include "opensync-data.h"
#include "opensync-format.h"
#include "opensync_filter_internals.h"
#include "opensync_filter_private.h"

OSyncFilter *osync_filter_new(const char *objtype, OSyncFilterAction action, OSyncError **error)
{
	OSyncFilter *filter = NULL;
	osync_trace(TRACE_ENTRY, "%s(%s, %i, %p)", __func__, objtype, action, error);
	
	filter = osync_try_malloc0(sizeof(OSyncFilter), error);
	if (!filter)
		goto error;
	
	filter->objtype = osync_strdup(objtype);
	filter->action = action;
	filter->ref_count = 1;
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, filter);
	return filter;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

OSyncFilter *osync_filter_new_custom(OSyncCustomFilter *custom_filter, const char *config, OSyncFilterAction action, OSyncError **error)
{
	OSyncFilter *filter = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %s, %i, %p)", __func__, custom_filter, config, action, error);
	
	filter = osync_try_malloc0(sizeof(OSyncFilter), error);
	if (!filter)
		goto error;
	
	filter->custom_filter = custom_filter;
	osync_custom_filter_ref(custom_filter);
	
	filter->config = osync_strdup(config);
	filter->action = action;
	filter->ref_count = 1;
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, filter);
	return filter;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

OSyncFilter *osync_filter_ref(OSyncFilter *filter)
{
	osync_assert(filter);
	
	g_atomic_int_inc(&(filter->ref_count));

	return filter;
}

void osync_filter_unref(OSyncFilter *filter)
{
	osync_assert(filter);
	
	if (g_atomic_int_dec_and_test(&(filter->ref_count))) {
		if (filter->objtype)
			osync_free(filter->objtype);
		
		if (filter->config)
			osync_free(filter->config);
		
		osync_free(filter);
	}
}

void osync_filter_set_config(OSyncFilter *filter, const char *config)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %s)", __func__, filter, config);
	
	osync_assert(filter);
	if (filter->config)
		osync_free(filter->config);
	filter->config = osync_strdup(config);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

const char *osync_filter_get_config(OSyncFilter *filter)
{
	osync_assert(filter);
	return filter->config;
}

const char *osync_filter_get_objtype(OSyncFilter *filter)
{
	osync_assert(filter);
	return filter->objtype;
}

OSyncFilterAction osync_filter_invoke(OSyncFilter *filter, OSyncData *data)
{
	osync_assert(filter);
	osync_assert(data);
	
	/* If our objtype doesnt match we return ignore */
	if (strcmp(filter->objtype, osync_data_get_objtype(data)))
		return OSYNC_FILTER_IGNORE;
	
	/* If this filter doesn't use a custom filter function, we return
	 * the action specified */
	if (!filter->custom_filter)
		return filter->action;
	
	/* If this filter uses a custom filter function, we invoke the
	 * custom filter */
	if (osync_custom_filter_invoke(filter->custom_filter, data, filter->config)) {
		/* It does match. so we return the filter action */
		return filter->action;
	}
	
	/* It does not match. So we return an ignore */
	return OSYNC_FILTER_IGNORE;	
}

OSyncCustomFilter *osync_custom_filter_new(const char *objtype, const char *objformat, const char *name, OSyncFilterFunction hook, OSyncError **error)
{
	OSyncCustomFilter *filter = NULL;
	osync_trace(TRACE_ENTRY, "%s(%s, %s, %s, %p, %p)", __func__, objtype, objformat, name, hook, error);
	
	filter = osync_try_malloc0(sizeof(OSyncCustomFilter), error);
	if (!filter)
		goto error;
	
	filter->objtype = osync_strdup(objtype);
	filter->objformat = osync_strdup(objformat);
	filter->name = osync_strdup(name);
	filter->hook = hook;
	filter->ref_count = 1;
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, filter);
	return filter;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

OSyncCustomFilter *osync_custom_filter_ref(OSyncCustomFilter *filter)
{
	osync_assert(filter);
	
	g_atomic_int_inc(&(filter->ref_count));

	return filter;
}

void osync_custom_filter_unref(OSyncCustomFilter *filter)
{
	osync_assert(filter);
	
	if (g_atomic_int_dec_and_test(&(filter->ref_count))) {
		if (filter->objtype)
			osync_free(filter->objtype);
		
		if (filter->objformat)
			osync_free(filter->objformat);
		
		if (filter->name)
			osync_free(filter->name);
		
		osync_free(filter);
	}
}

osync_bool osync_custom_filter_invoke(OSyncCustomFilter *filter, OSyncData *data, const char *config)
{
	osync_assert(filter);
	osync_assert(data);
	
	/* If our objtype doesnt match we return */
	if (strcmp(filter->objtype, osync_data_get_objtype(data)))
		return FALSE;
	
	/* If our objformat doesnt match we return */
	if (strcmp(filter->objformat, osync_objformat_get_name(osync_data_get_objformat(data))))
		return FALSE;
	
	/* We now check if the filter matches the data */
	return filter->hook(data, config);
}

