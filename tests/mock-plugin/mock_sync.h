/*
 * file-sync - A plugin for the opensync framework
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

#ifndef _MOCK_PLUGIN_H
#define _MOCK_PLUGIN_H

#include <stdlib.h>
#include <sys/stat.h>
#include <stdio.h>
#include <glib.h>
#include <string.h>

#include <opensync/opensync.h>
#include <opensync/opensync-format.h>
#include <opensync/opensync-plugin.h>
#include <opensync/opensync-data.h>
#include <opensync/opensync-helper.h>
#include <opensync/opensync-version.h>

#include "config.h"
#include "support.h"

typedef struct mock_env {
	GList *directories;

	OSyncMember *member;

	int num_connect;
	int num_disconnect;
	int num_get_changes;
	int num_commit_changes;
	int num_sync_done;

	int main_connect;
	int main_disconnect;
	int main_get_changes;
	int main_sync_done;

	OSyncObjTypeSink *mainsink;

	OSyncContext *ctx[10];
} mock_env;

typedef struct MockDir {
	OSyncObjFormat *objformat;
	OSyncPluginResource *res;
	GDir *dir;
	const char *path;
	OSyncHashTable *hashtable;
	mock_env *env;
	osync_bool committed_all;
	osync_bool connect_done;
	osync_bool connect_done_slowsync;
} MockDir;

#endif //_MOCK_PLUGIN_H
