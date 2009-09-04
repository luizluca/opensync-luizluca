/*
 * libopensync - A synchronization framework
 * Copyright (C) 2009  Bjoern Ricks <bjoern.ricks@googlemail.com>
 * Copyright (C) 2009  Daniel Gollub <gollub@b1-systems.de>
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

#ifndef OPENSYNC_MERGER_INTERNALS_H_
#define OPENSYNC_MERGER_INTERNALS_H_

/**
 * @defgroup OSyncMergerInternalsAPI OpenSync Merger
 * @ingroup OSyncFormatPrivate
 * @brief Functions for merging formats
 *
 */
/*@{*/

const char *osync_merger_get_objformat(OSyncMerger *merger);
const char *osync_merger_get_capsformat(OSyncMerger *merger);

osync_bool osync_merger_demerge(OSyncMerger *merger, OSyncChange *change, OSyncCapabilities *caps, OSyncError **error);

/*@}*/

#endif /* OPENSYNC_MERGER_INTERNALS_H_ */
