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

#ifndef _OPENSYNC_MARSHAL_PRIVATE_H
#define _OPENSYNC_MARSHAL_PRIVATE_H

/**
 * @defgroup OSyncCommonPrivate OpenSync Common Module Private
 * @ingroup OSyncPrivate
 * @defgroup OSyncMarshalPrivateAPI OpenSync Marshal Private 
 * @ingroup OSyncMarshalPrivate
 * @brief Serialized buffer 
 * 
 */

/*@{*/
/*! @brief A OSyncMarshal object 
 * 
 */
struct OSyncMarshal {
	/** Reference counting */
	int  ref_count;
	/** The pointer to the internal **/
	GByteArray *buffer;
	/** The current read position **/
	int buffer_read_pos;
};

/*@}*/

#endif /* _OPENSYNC_MARSHAL_PRIVATE_H */

