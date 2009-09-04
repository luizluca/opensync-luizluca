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

#ifndef OPENSYNC_MERGER_PRIVATE_H_
#define OPENSYNC_MERGER_PRIVATE_H_

/**
 * @defgroup OSyncMergerrPrivateAPI OpenSync Merger Private
 * @ingroup OSyncFormatPrivate
 * @brief Private part for merging formats
 * 
 */
/*@{*/

struct OSyncMerger {
	char *objformat;
	char *capsformat;
	int ref_count;
	OSyncMergerInitializeFunc initialize_func;
	OSyncMergerFinalizeFunc finalize_func;
	OSyncMergerMergeFunc merge_func;
	OSyncMergerDemergeFunc demerge_func;
	void *user_data;
};

/*@}*/

#endif /* OPENSYNC_MERGER_PRIVATE_H_ */
