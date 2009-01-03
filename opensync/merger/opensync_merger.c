/*
 * libopensync - A synchronization framework
 * Copyright (C) 2006  NetNix Finland Ltd <netnix@netnix.fi>
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
 * Author: Daniel Friedrich <daniel.friedrich@opensync.org>
 * 
 */
 
#include "opensync.h"
#include "opensync_internals.h"

#include "opensync-xmlformat.h"
#include "opensync-merger.h"

#include "opensync_capabilities_internals.h"
#include "opensync_merger_internals.h"

#include "opensync_merger_private.h"


OSyncMerger *osync_merger_new(OSyncCapabilities *capabilities, OSyncError **error)
{
	OSyncMerger *merger = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, capabilities, error);
	osync_assert(capabilities);
	
	merger = osync_try_malloc0(sizeof(OSyncMerger), error);
	if(!merger) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
		return NULL;
	}
	
	merger->ref_count = 1;
	osync_capabilities_ref(capabilities);
	merger->capabilities = capabilities;
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, merger);
	return merger;
}

OSyncMerger *osync_merger_ref(OSyncMerger *merger)
{
	osync_assert(merger);
	
	g_atomic_int_inc(&(merger->ref_count));

	return merger;
}

void osync_merger_unref(OSyncMerger *merger)
{
	osync_assert(merger);
			
	if (g_atomic_int_dec_and_test(&(merger->ref_count))) {
		osync_capabilities_unref(merger->capabilities);
		g_free(merger);
	}
}

