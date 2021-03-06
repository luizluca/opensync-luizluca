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

#ifndef OPENSYNC_CLIENT_PRIVATE_H_
#define OPENSYNC_CLIENT_PRIVATE_H_

struct OSyncClient {
	OSyncQueue *incoming;
	OSyncQueue *outgoing;
	GMainContext *context;
	GMainLoop *syncloop;
	GThread *disconnectThread;
	int ref_count;
	OSyncPlugin *plugin;
	OSyncPluginInfo *plugin_info;
	OSyncPluginEnv *plugin_env;
	OSyncFormatEnv *format_env;
	void *plugin_data;
	OSyncThread *thread;

	/* pipe path for the queue */
	char *pipe_path;
};

#endif /*OPENSYNC_CLIENT_PRIVATE_H_*/
