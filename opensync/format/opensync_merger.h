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

#ifndef OPENSYNC_MERGER_H_
#define OPENSYNC_MERGER_H_

/**
 * @defgroup OSyncMergerAPI OpenSync Merger
 * @ingroup OSyncFormat
 * @brief Functions for merging formats
 *
 */
/*@{*/

typedef void * (* OSyncMergerInitializeFunc) (OSyncError **error);
typedef void (* OSyncMergerFinalizeFunc) (void *user_data);

OSYNC_EXPORT OSyncMerger *osync_merger_new(const char *objformat, const char *capsformat, OSyncError **error);

OSYNC_EXPORT OSyncMerger *osync_merger_ref(OSyncMerger *merger);

OSYNC_EXPORT void osync_merger_unref(OSyncMerger *merger);

OSYNC_EXPORT void osync_merger_set_initialize_func(OSyncMerger *merger, OSyncMergerInitializeFunc initialize_func);

OSYNC_EXPORT void osync_merger_set_finalize_func(OSyncMerger *merger, OSyncMergerFinalizeFunc finalize_func);

/*@}*/

#endif /* OPENSYNC_MERGER_H_ */
