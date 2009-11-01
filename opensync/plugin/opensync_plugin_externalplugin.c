/*
 * libopensync - A synchronization framework
 * Copyright (C) 2009  Henrik Kaare Poulsen
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

#include "opensync.h"
#include "opensync_internals.h"

#include "opensync-plugin.h"
#include "opensync_plugin_externalplugin_private.h"

OSyncPluginExternalPlugin *osync_plugin_externalplugin_new(OSyncError **error)
{
	OSyncPluginExternalPlugin *externalplugin = osync_try_malloc0(sizeof(OSyncPluginExternalPlugin), error);
	if (!externalplugin)
		return NULL;

	externalplugin->ref_count = 1;

	return externalplugin;
}

OSyncPluginExternalPlugin *osync_plugin_externalplugin_ref(OSyncPluginExternalPlugin *externalplugin)
{
	osync_assert(externalplugin);
	
	g_atomic_int_inc(&(externalplugin->ref_count));

	return externalplugin;
}

void osync_plugin_externalplugin_unref(OSyncPluginExternalPlugin *externalplugin)
{
	osync_assert(externalplugin);
	
	if (g_atomic_int_dec_and_test(&(externalplugin->ref_count))) {
		if (externalplugin->external_command)
			osync_free(externalplugin->external_command);
		osync_free(externalplugin);
	}
}


const char *osync_plugin_externalplugin_get_external_command(OSyncPluginExternalPlugin *externalplugin)
{
	osync_assert(externalplugin);
	return externalplugin->external_command;
}

void osync_plugin_externalplugin_set_external_command(OSyncPluginExternalPlugin *externalplugin, const char *external_command)
{
	osync_assert(externalplugin);

	if (externalplugin->external_command)
		osync_free(externalplugin->external_command);

	externalplugin->external_command = osync_strdup(external_command);
}


