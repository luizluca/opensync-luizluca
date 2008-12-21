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
 
#ifndef OPENSYNC_OBJ_ENGINE_INTERNALS_H_
#define OPENSYNC_OBJ_ENGINE_INTERNALS_H_

struct OSyncObjEngine {
	int ref_count;
	OSyncEngine *parent;

	/** The object type of this Object Engine **/
	char *objtype;

	/** Status of Slow Sync **/
	osync_bool slowsync;

	OSyncArchive *archive;
	
	OSyncMappingTable *mapping_table;
	GList *mapping_engines;
	
	GList *sink_engines;
	OSyncError *error;
	OSyncFormatEnv *formatenv;
	
	int sink_errors;
	int sink_connects;
	int sink_connect_done;
	int sink_disconnects;
	int sink_get_changes;
	int sink_sync_done;
	int sink_written;
	
	OSyncObjEngineEventCallback callback;
	void *callback_userdata;
	
	GList *conflicts;

	/** Written status of Object Engine. - TODO: Is this still needed?! **/
	osync_bool written;
};

OSyncMappingEngine *_osync_obj_engine_create_mapping_engine(OSyncObjEngine *engine, OSyncError **error);

#endif /*OPENSYNC_OBJ_ENGINE_INTERNALS_H_*/
