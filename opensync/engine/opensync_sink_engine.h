/*
 * libopensync- A synchronization engine for the opensync framework
 * Copyright (C) 2004-2005  Armin Bauer <armin.bauer@opensync.org>
 * Copyright (C) 2006-2009  Daniel Gollub <gollub@b1-systems.de> 
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
 
#ifndef OPENSYNC_SINK_ENGINE_H_
#define OPENSYNC_SINK_ENGINE_H_

/*! @brief Get list of OSyncMappingEntryEngines of the OSyncSinkEngine
 *
 * @param engine Pointer to an OSyncSinkEngine
 * @returns List of OSyncMappingEntryEngines-elements or NULL if there are no Mapping Entry Engines. 
 */
OSYNC_EXPORT const OSyncList *osync_sink_engine_get_mapping_entry_engines(OSyncSinkEngine *engine);

/*! @brief Get member of the OSyncSinkEngine
 *
 * @param engine Pointer to an OSyncSinkEngine
 * @returns Pointer to Member of OSyncSinkEngine 
 */
OSYNC_EXPORT OSyncMember *osync_sink_engine_get_member(OSyncSinkEngine *engine);

#endif /* OPENSYNC_SINK_ENGINE_H_ */

