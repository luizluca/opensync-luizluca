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

#include "opensync-capabilities.h"
#include "opensync-data.h"

#include "data/opensync_data_internals.h"

#include "opensync-format.h"
#include "format/opensync_objformat_internals.h"
#include "capabilities/opensync_capabilities_internals.h"

#include "opensync_caps_converter_private.h"
#include "opensync_caps_converter_internals.h"

OSyncCapsConverter *osync_caps_converter_new(const char *sourceformat, const char *targetformat, OSyncCapsConvertFunc convert_func, OSyncError **error)
{
	OSyncCapsConverter *converter = NULL;
	osync_trace(TRACE_ENTRY, "%s(%s, %s, %p, %p)", __func__, __NULLSTR(sourceformat), sourceformat, __NULLSTR(targetformat), targetformat, convert_func, error);
	
	converter = osync_try_malloc0(sizeof(OSyncCapsConverter), error);
	if (!converter) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
		return NULL;
	}
	
	converter->source_capsformat = osync_strdup(sourceformat);
	
	converter->target_capsformat = osync_strdup(targetformat);
	
	converter->convert_func = convert_func;
	converter->initialize_func = NULL;
	converter->finalize_func = NULL;
	converter->userdata = NULL;
	converter->ref_count = 1;
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, converter);
	return converter;
}

OSyncCapsConverter *osync_caps_converter_ref(OSyncCapsConverter *converter)
{
	osync_assert(converter);
	
	g_atomic_int_inc(&(converter->ref_count));

	return converter;
}

void osync_caps_converter_unref(OSyncCapsConverter *converter)
{
	osync_assert(converter);
	
	if (g_atomic_int_dec_and_test(&(converter->ref_count))) {
		if (converter->source_capsformat)
			osync_free(converter->source_capsformat);
			
		if (converter->target_capsformat)
			osync_free(converter->target_capsformat);
		
		osync_free(converter);
	}
}

const char *osync_caps_converter_get_sourceformat(OSyncCapsConverter *converter)
{
	osync_assert(converter);
	return converter->source_capsformat;
}

const char *osync_caps_converter_get_targetformat(OSyncCapsConverter *converter)
{
	osync_assert(converter);
	return converter->target_capsformat;
}

osync_bool osync_caps_converter_invoke(OSyncCapsConverter *converter, OSyncCapabilities **caps, const char *config, OSyncError **error)
{
	OSyncCapabilities *new_caps;
	
	osync_assert(converter);
	osync_assert(caps);
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %s, %p)", __func__, converter, caps, __NULLSTR(config), error);
	osync_trace(TRACE_INTERNAL, "Converter from %s to %s", converter->source_capsformat, converter->target_capsformat);
	
	osync_assert(converter->convert_func);

	/* Invoke the converter */
	if (!converter->convert_func(*caps, &new_caps, config, converter->userdata, error))
		goto error;

	/* Sort capabilities by field-Name */
	osync_capabilities_sort(new_caps);

	/* TODO: check if the old capabilities object is really not leaking. */
	/* osync_capabilities_unref(*caps); */
	*caps = new_caps;

	osync_capabilities_set_format(*caps, converter->target_capsformat);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

void osync_caps_converter_set_initialize_func(OSyncCapsConverter *converter, OSyncCapsConverterInitializeFunc initialize_func)
{
	osync_assert(converter);
	converter->initialize_func = initialize_func;
	
}

void osync_caps_converter_set_finalize_func(OSyncCapsConverter *converter, OSyncCapsConverterFinalizeFunc finalize_func)
{
	osync_assert(converter);
	converter->finalize_func = finalize_func;
}

void osync_caps_converter_initialize(OSyncCapsConverter *converter, const char *config, OSyncError **error) {

	osync_assert(converter);

	if (converter->initialize_func) {
		converter->userdata = converter->initialize_func(config, error);
	}
}

void osync_caps_converter_finalize(OSyncCapsConverter *converter)
{
	osync_assert(converter);

	if (converter->finalize_func) {
		converter->finalize_func(converter->userdata);
	}
}

