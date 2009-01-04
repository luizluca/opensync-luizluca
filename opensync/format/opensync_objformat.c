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
#include "opensync_objformat_internals.h"
#include "opensync_objformat_private.h"


OSyncObjFormat *osync_objformat_new(const char *name, const char *objtype_name, OSyncError **error)
{
	OSyncObjFormat *format = NULL;
	osync_trace(TRACE_ENTRY, "%s(%s, %s, %p)", __func__, name, objtype_name, error);
	
	format = osync_try_malloc0(sizeof(OSyncObjFormat), error);
	if (!format)
		return FALSE;
	
	format->name = g_strdup(name);
	format->objtype_name = g_strdup(objtype_name);
	format->ref_count = 1;
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, format);
	return format;
}

OSyncObjFormat *osync_objformat_ref(OSyncObjFormat *format)
{
	osync_return_val_if_fail(format, NULL);
	
	g_atomic_int_inc(&(format->ref_count));

	return format;
}

void osync_objformat_unref(OSyncObjFormat *format)
{
	osync_return_if_fail(format);
	
	if (g_atomic_int_dec_and_test(&(format->ref_count))) {
		if (format->name)
			g_free(format->name);
			
		if (format->objtype_name)
			g_free(format->objtype_name);

		g_free(format);
	}
}

const char *osync_objformat_get_name(OSyncObjFormat *format)
{
	osync_return_val_if_fail(format, NULL);
	return format->name;
}

const char *osync_objformat_get_objtype(OSyncObjFormat *format)
{
	osync_return_val_if_fail(format, NULL);
	return format->objtype_name;
}

osync_bool osync_objformat_is_equal(OSyncObjFormat *leftformat, OSyncObjFormat *rightformat)
{
	osync_return_val_if_fail(leftformat, FALSE);
	osync_return_val_if_fail(rightformat, FALSE);
	
	return (!strcmp(leftformat->name, rightformat->name)) ? TRUE : FALSE;
}

void osync_objformat_set_initialize_func(OSyncObjFormat *format, OSyncFormatInitializeFunc initialize_func)
{
	osync_return_if_fail(format);
	format->initialize_func = initialize_func;
}

osync_bool osync_objformat_initialize(OSyncObjFormat *format, OSyncError **error)
{
	osync_assert(format);
	
	/* Just return, if no initialize_func is registered */
	osync_return_val_if_fail(format->initialize_func, TRUE);

	format->user_data = format->initialize_func(error);

	if (osync_error_is_set(error))
		return FALSE;

	return TRUE;
}

void osync_objformat_set_finalize_func(OSyncObjFormat *format, OSyncFormatFinalizeFunc finalize_func)
{
	osync_return_if_fail(format);
	format->finalize_func = finalize_func;
}

void osync_objformat_finalize(OSyncObjFormat *format)
{
	osync_return_if_fail(format);
	osync_return_if_fail(format->finalize_func);
	format->finalize_func(format->user_data);
}

void osync_objformat_set_compare_func(OSyncObjFormat *format, OSyncFormatCompareFunc cmp_func)
{
	osync_return_if_fail(format);
	format->cmp_func = cmp_func;
}

OSyncConvCmpResult osync_objformat_compare(OSyncObjFormat *format, const char *leftdata, unsigned int leftsize, const char *rightdata, unsigned int rightsize)
{
	osync_return_val_if_fail(format, OSYNC_CONV_DATA_UNKNOWN);
	osync_return_val_if_fail(format->cmp_func, OSYNC_CONV_DATA_UNKNOWN);
	return format->cmp_func(leftdata, leftsize, rightdata, rightsize);
}

void osync_objformat_set_destroy_func(OSyncObjFormat *format, OSyncFormatDestroyFunc destroy_func)
{
	osync_return_if_fail(format);
	format->destroy_func = destroy_func;
}

void osync_objformat_destroy(OSyncObjFormat *format, char *data, unsigned int size)
{
	osync_return_if_fail(format);
	
	if (!format->destroy_func) {
		osync_trace(TRACE_INTERNAL, "Format %s don't have a destroy function. Possible memory leak", format->name);
		return;
	}
	
	format->destroy_func(data, size);
}

void osync_objformat_set_copy_func(OSyncObjFormat *format, OSyncFormatCopyFunc copy_func)
{
	osync_return_if_fail(format);
	format->copy_func = copy_func;
}

osync_bool osync_objformat_copy(OSyncObjFormat *format, const char *indata, unsigned int insize, char **outdata, unsigned int *outsize, OSyncError **error)
{
	osync_assert(format);
	osync_assert(indata);
	osync_assert(outdata);

	if (!format->copy_func) {
		osync_trace(TRACE_INTERNAL, "We cannot copy the change, falling back to memcpy");
		*outdata = osync_try_malloc0(sizeof(char) * insize, error);
		if (!*outdata)
			return FALSE;
			
		memcpy(*outdata, indata, insize);
		*outsize = insize;
	} else {
		if (!format->copy_func(indata, insize, outdata, outsize, error)) {
			osync_error_set(error, OSYNC_ERROR_GENERIC, "Something went wrong during copying");
			return FALSE;
		}
	}
	return TRUE;
}

void osync_objformat_set_duplicate_func(OSyncObjFormat *format, OSyncFormatDuplicateFunc dupe_func)
{
	osync_return_if_fail(format);
	format->duplicate_func = dupe_func;
}

osync_bool osync_objformat_duplicate(OSyncObjFormat *format, const char *uid, const char *input, unsigned int insize, char **newuid, char **output, unsigned int *outsize, osync_bool *dirty, OSyncError **error)
{
	osync_assert(format);

	if (!format->duplicate_func) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "No duplicate function set");
		return FALSE;
	}

	return format->duplicate_func(uid, input, insize, newuid, output, outsize, dirty, error);
}

void osync_objformat_set_create_func(OSyncObjFormat *format, OSyncFormatCreateFunc create_func)
{
	osync_return_if_fail(format);
	format->create_func = create_func;
}

void osync_objformat_create(OSyncObjFormat *format, char **data, unsigned int *size)
{
	osync_return_if_fail(format);
	osync_return_if_fail(format->create_func);

	format->create_func(data, size);
}

void osync_objformat_set_print_func(OSyncObjFormat *format, OSyncFormatPrintFunc print_func)
{
	osync_return_if_fail(format);
	format->print_func = print_func;
}

char *osync_objformat_print(OSyncObjFormat *format, const char *data, unsigned int size)
{
	osync_return_val_if_fail(format, NULL);
	
	if (!format->print_func)
		return g_strndup(data, size);
	
	return format->print_func(data, size);
}

void osync_objformat_set_revision_func(OSyncObjFormat *format, OSyncFormatRevisionFunc revision_func)
{
	osync_return_if_fail(format);
	format->revision_func = revision_func;
}

time_t osync_objformat_get_revision(OSyncObjFormat *format, const char *data, unsigned int size, OSyncError **error)
{
	osync_assert(format);
	osync_assert(data);
	
	if (!format->revision_func) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "No revision function set");
		return -1;
	}
	
	return format->revision_func(data, size, error);
}

void osync_objformat_set_marshal_func(OSyncObjFormat *format, OSyncFormatMarshalFunc marshal_func)
{
	osync_return_if_fail(format);
	format->marshal_func = marshal_func;
}

osync_bool osync_objformat_must_marshal(OSyncObjFormat *format)
{
	osync_assert(format);
	return format->marshal_func ? TRUE : FALSE;
}

osync_bool osync_objformat_marshal(OSyncObjFormat *format, const char *input, unsigned int inpsize, OSyncMessage *message, OSyncError **error)
{
	osync_assert(format);
	osync_return_val_if_fail(format->marshal_func, TRUE);
	return format->marshal_func(input, inpsize, message, error);
}

void osync_objformat_set_demarshal_func(OSyncObjFormat *format, OSyncFormatDemarshalFunc demarshal_func)
{
	osync_return_if_fail(format);
	format->demarshal_func = demarshal_func;
}

osync_bool osync_objformat_demarshal(OSyncObjFormat *format, OSyncMessage *message, char **output, unsigned int *outpsize, OSyncError **error)
{
	osync_assert(format);
	osync_return_val_if_fail(format->demarshal_func, TRUE);
	return format->demarshal_func(message, output, outpsize, error);
}

void osync_objformat_set_validate_func(OSyncObjFormat *format, OSyncFormatValidateFunc validate_func)
{
	osync_return_if_fail(format);
	format->validate_func = validate_func;
}

osync_bool osync_objformat_validate(OSyncObjFormat *format, const char *data, unsigned int size, OSyncError **error)
{
	osync_assert(format);
	osync_return_val_if_fail(format->validate_func, TRUE);
	return format->validate_func(data, size, format->user_data, error);
}

osync_bool osync_objformat_must_validate(OSyncObjFormat *format)
{
	osync_assert(format);
	return format->validate_func ? TRUE : FALSE;
}

void osync_objformat_set_merge_func(OSyncObjFormat *format, OSyncFormatMergeFunc merge_func)
{
	osync_return_if_fail(format);
	format->merge_func = merge_func;
}

osync_bool osync_objformat_merge(OSyncObjFormat *format,
		const char *input, unsigned int inpsize,
		char **output, unsigned int *outpsize,
		const char *entire, unsigned int entsize,
		OSyncCapabilities *caps, OSyncError **error)
{
	osync_assert(format);
	osync_return_val_if_fail(format->merge_func, TRUE);
	return format->merge_func(input, inpsize, output, outpsize, entire, entsize, caps, error);
}

void osync_objformat_set_demerge_func(OSyncObjFormat *format, OSyncFormatDemergeFunc demerge_func)
{
	osync_return_if_fail(format);
	format->demerge_func = demerge_func;
}

osync_bool osync_objformat_demerge(OSyncObjFormat *format,
		const char *input, unsigned int inpsize,
		char **output, unsigned int *outpsize,
		OSyncCapabilities *caps, OSyncError **error)
{
	osync_assert(format);
	osync_return_val_if_fail(format->demerge_func, TRUE);
	return format->demerge_func(input, inpsize, output, outpsize, caps, error);
}


osync_bool osync_objformat_has_merger(OSyncObjFormat *format)
{
	osync_return_val_if_fail(format, FALSE);
	return (format->demerge_func && format->merge_func) ? TRUE : FALSE;
}

