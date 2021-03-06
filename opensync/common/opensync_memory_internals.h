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
 
#ifndef _OPENSYNC_MEMORY_INTERNALS_H
#define _OPENSYNC_MEMORY_INTERNALS_H

/** @brief Bit counting
 * 
 * MIT HAKMEM Count, Bit counting in constant time and memory. 
 * 
 * result can't be greater than 32 on 32 Bit systems or 64 on 64 Bit systems
 * @param u unsigned integer value to count bits
 * @returns The bit counting result 
 * 
 */
unsigned char osync_bitcount(unsigned int u);

#endif /* _OPENSYNC_MEMORY_INTERNALS_H */

