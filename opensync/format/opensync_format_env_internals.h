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

#ifndef _OPENSYNC_FORMAT_ENV_INTERNALS_H_
#define _OPENSYNC_FORMAT_ENV_INTERNALS_H_

#include "opensync/format/opensync_filter_internals.h"

/**
 * @defgroup OSyncFormatEnvInternalAPI OpenSync Format Environment Internals
 * @ingroup OSyncFormatPrivate
 * @brief The internal API of the OpenSync Format Environment
 * 
 */
/*@{*/

/** A target function for osync_conv_find_path_fn() */
typedef osync_bool (*OSyncPathTargetFn)(const void *data, OSyncObjFormat *fmt);

osync_bool osync_conv_find_path_fmtlist(OSyncFormatEnv *env, OSyncChange *start, GList/*OSyncObjFormat * */ *targets, GList **retlist);

osync_bool osync_conv_convert_fn(OSyncFormatEnv *env, OSyncChange *change, OSyncPathTargetFn target_fn, const void *fndata, const char *extension_name, OSyncError **error);
osync_bool osync_conv_convert_fmtlist(OSyncFormatEnv *env, OSyncChange *change, GList/*OSyncObjFormat * */ *targets);

/** @brief The environment used for conversions
 */
struct OSyncFormatEnv {
	/** A List of formats */
	OSyncList *objformats;
	/** A list of available format converters */
	OSyncList *converters;
	/** A list of available capabilities converters */
	OSyncList *caps_converters;
	/** A list of filter functions */
	OSyncList *custom_filters;
	/** A list of mergers (OSyncMergers *) */
	OSyncList *mergers;
	
	OSyncList *modules;
	GModule *current_module;

	int ref_count;
};

/** @brief search tree for format converters
 */
typedef struct OSyncFormatConverterTree {
	/* The converters that weren't reached yet */
	OSyncList *unused;
	/* The search queue for the Breadth-first search */
	OSyncList *search;
} OSyncFormatConverterTree;

typedef struct OSyncFormatConverterPathVertice {
	/** The format associated with this OSyncFormatConverterPathVertice */
	OSyncObjFormat *format;
	OSyncData *data;

	/** The path of converters taken to this OSyncFormatConverterPathVertice. If this OSyncFormatConverterPathVertice is a target, we will
	 * return this list as the result */
	OSyncList *path;

	unsigned losses;
	unsigned objtype_changes;
	unsigned conversions;
	guint id;
	guint neighbour_id;
	osync_bool preferred;

	int ref_count;

} OSyncFormatConverterPathVertice;

typedef osync_bool (*OSyncTargetLastConverterFn)(const void *data, OSyncFormatConverterTree *tree);

/** @brief Register Filter in Format Environment 
 * 
 * @param env The format environment
 * @param filter Pointer of Custom Filter to register
 * @param error The location to return a error to
 * @returns TRUE if successful, FALSE otherwise
 * 
 */
OSYNC_TEST_EXPORT osync_bool osync_format_env_register_filter(OSyncFormatEnv *env, OSyncCustomFilter *filter, OSyncError **error);

/** @brief Returns the number of available filters
 * 
 * @param env The format environment
 * @returns The number of filters
 * 
 */
OSYNC_TEST_EXPORT unsigned int osync_format_env_num_filters(OSyncFormatEnv *env);

/** @brief Gets the nth filter
 * 
 * @param env The format environment
 * @param nth The position of the filter to retrieve
 * @returns The filter
 * 
 */
OSYNC_TEST_EXPORT OSyncCustomFilter *osync_format_env_nth_filter(OSyncFormatEnv *env, unsigned int nth);


/** @brief Initialize all object formats of the format environment 
 * 
 * @param env The format environment
 * @param error Pointer to an error struct, which get set on any error intialize error 
 * 
 */
OSYNC_TEST_EXPORT void osync_format_env_objformat_initialize(OSyncFormatEnv *env, OSyncError **error);

/** @brief Finalize all object fromats of the format environment 
 * 
 * @param env The format environment
 * 
 */
OSYNC_TEST_EXPORT void osync_format_env_objformat_finalize(OSyncFormatEnv *env);

/** @brief Returns the number of available object formats
 * 
 * @param env The format environment
 * @returns The number of object formats
 * 
 */
OSYNC_TEST_EXPORT unsigned int osync_format_env_num_objformats(OSyncFormatEnv *env);

/** @brief Gets the nth object format
 * 
 * @param env The format environment
 * @param nth The position of the object format to retrieve
 * @returns The object format
 * 
 */
OSYNC_TEST_EXPORT OSyncObjFormat *osync_format_env_nth_objformat(OSyncFormatEnv *env, unsigned int nth);

/** @brief Returns the number of available converters
 * 
 * @param env The format environment
 * @returns The number of converters
 * 
 */
OSYNC_TEST_EXPORT unsigned int osync_format_env_num_converters(OSyncFormatEnv *env);

/** @brief Gets the nth format converter
 * 
 * @param env The format environment
 * @param nth The position of the format converter to retrieve
 * @returns The format converter
 * 
 */
OSYNC_TEST_EXPORT OSyncFormatConverter *osync_format_env_nth_converter(OSyncFormatEnv *env, int nth);

OSYNC_TEST_EXPORT OSyncList *osync_format_env_find_mergers_objformat(OSyncFormatEnv *env, const char *objformat);


/*@}*/

#endif /* _OPENSYNC_FORMAT_ENV_INTERNALS_H_ */

