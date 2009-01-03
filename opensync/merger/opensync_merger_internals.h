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

#ifndef OPENSYNC_MERGER_INTERNALS_H_
#define OPENSYNC_MERGER_INTERNALS_H_


/**
 * @defgroup OSyncMerger OpenSync Merger Module
 * @ingroup OSyncPublic
 * @defgroup OSyncMergerAPI OpenSync Merger
 * @ingroup OSyncMerger
 * @brief The public part of the OSyncMerger
 * 
 */
/*@{*/

typedef struct OSyncMerger OSyncMerger;

/**
 * @brief Creates a new merger object
 * @param capabilities Pointer to capabilities which should be taken in account by the Merger
 * @param error The error which will hold the info in case of an error
 * @return The pointer to the newly allocated merger object or NULL in case of error
 */
OSYNC_TEST_EXPORT OSyncMerger *osync_merger_new(OSyncCapabilities *capabilities, OSyncError **error);

/**
 * @brief Increments the reference counter
 * @param merger The pointer to a merger object
 */
OSYNC_TEST_EXPORT OSyncMerger *osync_merger_ref(OSyncMerger *merger);

/**
 * @brief Decrement the reference counter. The merger object will 
 *  be freed if there is no more reference to it.
 * @param merger The pointer to a merger object
 */
OSYNC_TEST_EXPORT void osync_merger_unref(OSyncMerger *merger);

/*@}*/

#endif /*OPENSYNC_MERGER_INTERNALS_H_*/
