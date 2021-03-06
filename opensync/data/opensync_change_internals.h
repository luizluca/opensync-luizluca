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

#ifndef _OPENSYNC_CHANGE_INTERNALS_H_
#define _OPENSYNC_CHANGE_INTERNALS_H_

/**
 * @defgroup OSyncChangeInternalAPI OpenSync Change Internals
 * @ingroup OSyncDataPrivate
 */

/*@{*/

/*! @brief Clone a change object
 *
 *  @param source The change object to clone
 *  @param error An error struct
 *  @returns a copy of the specified change object, or NULL if an error occured
 *
 */
OSyncChange *osync_change_clone(OSyncChange *source, OSyncError **error);

/*! @brief Duplicates the uid of a change
 * 
 * This will call the duplicate function of a format.
 * This is used if a uid is not unique.
 * 
 * @param change The change to duplicate
 * @param dirty Reference which stores value if change still needs to be multiplied (dirty)
 * @param error An error struct
 * @returns TRUE if the uid was duplicated successfully, FALSE otherwise.
 * 
 */
osync_bool osync_change_duplicate(OSyncChange *change, osync_bool *dirty, OSyncError **error);

/*@}*/

#endif /*_OPENSYNC_CHANGE_INTERNALS_H_*/
