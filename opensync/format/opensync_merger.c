/*
 * libopensync - A synchronization framework
 * Copyright (C) 2009  Bjoern Ricks <bjoern.ricks@googlemail.com>
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

#include "opensync_merger.h"
#include "opensync_merger_private.h"

OSyncMerger *osync_merger_new(const char *objformat, const char *capsformat, OSyncError **error)
{
	OSyncMerger *merger = NULL;
	osync_trace(TRACE_ENTRY, "%s(%s, %s, %p)", __func__, objformat, capsformat, error);
	
	merger = osync_try_malloc0(sizeof(OSyncMerger), error);
	if (!merger) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
		return NULL;
	}
	merger->ref_count = 1;
	merger->objformat = osync_strdup(objformat);
	merger->capsformat = osync_strdup(capsformat);
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, merger);
	return merger;
}

OSyncMerger *osync_merger_ref(OSyncMerger *merger)
{
	osync_return_val_if_fail(merger, NULL);
	
	g_atomic_int_inc(&(merger->ref_count));

	return merger;
}

void osync_merger_unref(OSyncMerger *merger)
{
	osync_return_if_fail(merger);
	
	if (g_atomic_int_dec_and_test(&(merger->ref_count))) {
		if (merger->objformat) {
			osync_free(merger->objformat);
		}
			
		if (merger->capsformat) {
			osync_free(merger->capsformat);
		}

		osync_free(merger);
	}
}

void osync_merger_set_initialize_func(OSyncMerger *merger, OSyncMergerInitializeFunc initialize_func)
{
	osync_return_if_fail(merger);
	merger->initialize_func = initialize_func;
}

void osync_merger_set_finalize_func(OSyncMerger *merger, OSyncMergerFinalizeFunc finalize_func)
{
	osync_return_if_fail(merger);
	merger->finalize_func = finalize_func;
}

void osync_merger_set_merge_func(OSyncMerger *merger, OSyncMergerMergeFunc merge_func)
{
	osync_return_if_fail(merger);
	merger->merge_func = merge_func;
}

void osync_merger_set_demerge_func(OSyncMerger *merger, OSyncMergerDemergeFunc demerge_func)
{
	osync_return_if_fail(merger);
	merger->demerge_func = demerge_func;
}
