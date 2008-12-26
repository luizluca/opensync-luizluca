/*
 * libosengine - A synchronization engine for the opensync framework
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
 
#ifndef OPENSYNC_SINK_ENGINE_INTERNALS_H_
#define OPENSYNC_SINK_ENGINE_INTERNALS_H_

/*! @brief OSyncSinkEngine struct members 
 *
 **/
typedef struct OSyncSinkEngine {
	/** Reference counting */
	int ref_count;
	/** Position of OSyncSinkEngine in OSyncObjEngine */
	int position;
	/** Pointer to assinged OSyncClientProxy */
	OSyncClientProxy *proxy;
	/** Pointer to assinged OSyncObjEngine */
	OSyncObjEngine *engine;
	/** List of assinged OSyncMappingEntry elements */
	GList *entries;
	/** List of assinged OSyncMappingEntry elemebts, but unmapped (no counter-entry) */
	GList *unmapped;
	/** "dummy" attribute, when set no proxy functions of OSyncSinkEngine get called */
	osync_bool dummy_sink;
} OSyncSinkEngine;

OSyncSinkEngine *osync_sink_engine_new(int position, OSyncClientProxy *proxy, OSyncObjEngine *objengine, OSyncError **error);
OSyncSinkEngine *osync_sink_engine_ref(OSyncSinkEngine *engine);
void osync_sink_engine_unref(OSyncSinkEngine *engine);
osync_bool osync_sink_engine_is_connected(OSyncSinkEngine *engine);


/*! @brief Set "dummy" attribute for OSyncSinkEngine
 *
 * If the "dummy" attribute set, then OSyncSinkEngine will not call any proxy
 * function inside the OSyncObjEngine. The original purpose of this is to have
 * a OSyncSinkEngine for "peers" which make use of mixed-ObjTypes. For this
 * reason primarly the lists of OSyncSinkEngine (i.e. entries and unmapped
 * entries) get used for mixed-ObjType mappings.
 *
 * @param engine Pointer to an OSyncSinkEngine which should act as "dummy"
 * @param isdummy TRUE or FALSE to set the "dummy" attribute 
 */
void osync_sink_engine_set_dummy(OSyncSinkEngine *engine, osync_bool isdummy);

/*! @brief Get state of "dummy" attribute of OSyncSinkEngine 
 *
 * @param engine Pointer to an OSyncSinkEngine which should act as "dummy"
 * @returns TRUE if engine got set as "dummy", FALSE otherwise
 */
osync_bool osync_sink_engine_is_dummy(OSyncSinkEngine *engine);

#endif /*OPENSYNC_SINK_ENGINE_INTERNALS_H_*/
