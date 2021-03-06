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

#ifndef OPENSYNC_OBJTYPE_SINK_PRIVATE_H_
#define OPENSYNC_OBJTYPE_SINK_PRIVATE_H_

/**
 * @defgroup OSyncObjTypeSinkPrivateAPI OpenSync Object Type Sink Private
 * @ingroup OSyncPluginPrivate
 */

/*@{*/

#define OSYNC_SINK_TIMEOUT_TRANSPORT 	30
#define OSYNC_SINK_TIMEOUT_SINGLEIO	30
#define OSYNC_SINK_TIMEOUT_BATCHIO	1800

/* FIXME: OSYNC_SINK_TIMEOUT_COMMIT, OSYNC_SINK_TIMEOUT_READ,
   are set to OSYNC_SINK_TIMEOUT_BATCHIO
   to workaround a timeout issue with the async client/proxy
   communication and the current timeout handling.

   All async i/o call (message) recieved by the client get processed in
   a sequence. Each event gets appended at the end of the queue
   and sits there; timeout handlers keeps already ticking!

   Commits which got added at the end of a long queue of events,
   doesn't get processed in time and get killed by the timeout handler.

   TODO: Fix timeout handler and start ticking when SINGLEIO get processed.
   Reset  OSYNC_SINK_TIMEOUT_COMMIT, OSYNC_SINK_TIMEOUT_READ and other SINGLEIO
   actions to SINGLEIO timeout values by default.
 
 */

#define OSYNC_SINK_TIMEOUT_CONNECT 	OSYNC_SINK_TIMEOUT_TRANSPORT 
#define OSYNC_SINK_TIMEOUT_CONNECTDONE	OSYNC_SINK_TIMEOUT_SINGLEIO
#define OSYNC_SINK_TIMEOUT_DISCONNECT 	OSYNC_SINK_TIMEOUT_TRANSPORT 
#define OSYNC_SINK_TIMEOUT_GETCHANGES	OSYNC_SINK_TIMEOUT_BATCHIO
#define OSYNC_SINK_TIMEOUT_COMMIT	OSYNC_SINK_TIMEOUT_BATCHIO /* FIXME */
#define OSYNC_SINK_TIMEOUT_COMMITTEDALL	OSYNC_SINK_TIMEOUT_SINGLEIO
#define OSYNC_SINK_TIMEOUT_SYNCDONE	OSYNC_SINK_TIMEOUT_SINGLEIO
#define OSYNC_SINK_TIMEOUT_READ		OSYNC_SINK_TIMEOUT_BATCHIO /* FIXME */

typedef struct OSyncObjTypeSinkFunctionTimeouts {
	unsigned int connect;
	unsigned int connect_done;
	unsigned int disconnect;
	unsigned int get_changes;
	unsigned int commit;
	unsigned int committed_all;
	unsigned int sync_done;
	unsigned int read;
} OSyncObjTypeSinkFunctionTimeouts;

struct OSyncObjTypeSink {
	/** The sink anchor if requested by the plugin */
	OSyncSinkStateDB *state_db;

	/** Flag if anchor is requested by plugin */
	osync_bool state_db_requested;

	/** The sink hashtable if requested by the plugin */
	OSyncHashTable *hashtable;

	/** Flag if hashtable is requested by plugin */
	osync_bool hashtable_requested;

	/** The preferred step or target format for the conversion path of this sink */
	char *preferred_format;
	/** The format which can be synchronized by this sink */
	OSyncList *objformatsinks;

	/** The functions to be called */
	OSyncObjTypeSinkFunctions functions;
	void *userdata;

	/** The timeout values of the sink functions */
	OSyncObjTypeSinkFunctionTimeouts timeout;
	
	/** The objtype type of this sink. In case of a main sink this is always NULL. */
	char *objtype;

	/** The status if this sink is allowed to write (commit) */
	osync_bool write;

	/** The status if this sink is allowed to read (single entries) */
	osync_bool read;
	/** The status if this sink has a read function (single entries) */
	osync_bool func_read;

	/** The status if this sink is allowed to get_changes (latest changed entries) */
	osync_bool getchanges; 
	/** The status if this sink has a get_changes function (latest changed entries) */
	osync_bool func_getchanges; 

	/** The status of this sink */
	osync_bool enabled;

	/** The request status of a slow-sync of this sink */
	osync_bool slowsync;

	/** Referce counting */
	int ref_count;

	osync_bool available;
};

/*@}*/

#endif /* OPENSYNC_OBJTYPE_SINK_PRIVATE_H_ */

