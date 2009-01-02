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

#ifndef _OPENSYNC_SUPPORT_H
#define _OPENSYNC_SUPPORT_H

/* FIXME: Drop opensync-support.h with 0.39 release */
#include "opensync-common.h"
#include "opensync-debug.h"

OPENSYNC_BEGIN_DECLS

/**
 * @defgroup OSyncSupportAPI OpenSync Support Module 
 * @ingroup OSyncPublic
 */
/*@{*/

/*! @brief Returns the version of opensync
 * 
 * Returns a string identifying the major and minor version
 * of opensync (something like "0.11")
 * 
 * @returns String with version
 * 
 */
OSYNC_EXPORT const char *osync_get_version(void);


/*@} */

OPENSYNC_END_DECLS

#endif /* _OPENSYNC_SUPPORT_H */

