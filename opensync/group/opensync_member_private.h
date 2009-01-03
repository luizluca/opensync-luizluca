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

#ifndef _OPENSYNC_MEMBER_PRIVATE_H_
#define _OPENSYNC_MEMBER_PRIVATE_H_

#include <libxml/tree.h>

/**
 * @defgroup OSyncMemberPrivateAPI OpenSync Member Private
 * @ingroup OSyncGroupPrivate
 * @brief The private part of the OSyncMember
 * 
 */
/*@{*/

/** @brief Parser for the "timeout" node in the member configuration
 * 
 * @param cur Pointer to the xmlNode 
 * @param sink Pointer to the OSyncObjTypeSink object
 * 
 */
void osync_member_parse_timeout(xmlNode *cur, OSyncObjTypeSink *sink);

/** @brief Parser for the "objtype" node in the member configuration
 * 
 * @param cur Pointer to the xmlNode 
 * @param error Pointer to a error
 * @returns Object type sink of the parsed configuration. NULL on error.
 * 
 */
static OSyncObjTypeSink *osync_member_parse_objtype(xmlNode *cur, OSyncError **error);

/*@}*/

#endif /* _OPENSYNC_MEMBER_PRIVATE_H_ */
