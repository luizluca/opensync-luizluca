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

#include "opensync.h"
#include "opensync_internals.h"

#include "opensync-plugin.h"
#include "opensync_plugin_internals.h"
#include "opensync_plugin_private.h"

OSyncPlugin *osync_plugin_new(OSyncError **error)
{
	OSyncPlugin *plugin = osync_try_malloc0(sizeof(OSyncPlugin), error);
	if (!plugin)
		return NULL;
	
	plugin->config_type = OSYNC_PLUGIN_NEEDS_CONFIGURATION;
	plugin->start_type = OSYNC_START_TYPE_THREAD;
	plugin->ref_count = 1;

	plugin->timeout.initialize = OSYNC_PLUGIN_TIMEOUT_INITIALIZE;
	plugin->timeout.finalize = OSYNC_PLUGIN_TIMEOUT_FINALIZE;
	plugin->timeout.discover = OSYNC_PLUGIN_TIMEOUT_DISCOVER;
	plugin->timeout.useable = OSYNC_PLUGIN_TIMEOUT_USEABLE;
	
	return plugin;
}

OSyncPlugin *osync_plugin_ref(OSyncPlugin *plugin)
{
	osync_assert(plugin);
	
	g_atomic_int_inc(&(plugin->ref_count));

	return plugin;
}

void osync_plugin_unref(OSyncPlugin *plugin)
{
	osync_assert(plugin);
	
	if (g_atomic_int_dec_and_test(&(plugin->ref_count))) {
		if (plugin->name)
			osync_free(plugin->name);
			
		if (plugin->longname)
			osync_free(plugin->longname);
			
		if (plugin->description)
			osync_free(plugin->description);

		if (plugin->external_command)
			osync_free(plugin->external_command);
			
		osync_free(plugin);
	}
}

const char *osync_plugin_get_name(OSyncPlugin *plugin)
{
	osync_assert(plugin);
	return plugin->name;
}

void osync_plugin_set_name(OSyncPlugin *plugin, const char *name)
{
	osync_assert(plugin);
	if (plugin->name)
		osync_free(plugin->name);
	plugin->name = osync_strdup(name);
}

const char *osync_plugin_get_longname(OSyncPlugin *plugin)
{
	osync_assert(plugin);
	return plugin->longname;
}

void osync_plugin_set_longname(OSyncPlugin *plugin, const char *longname)
{
	osync_assert(plugin);
	if (plugin->longname)
		osync_free(plugin->longname);
	plugin->longname = osync_strdup(longname);
}

const char *osync_plugin_get_description(OSyncPlugin *plugin)
{
	osync_assert(plugin);
	return plugin->description;
}

void osync_plugin_set_description(OSyncPlugin *plugin, const char *description)
{
	osync_assert(plugin);
	if (plugin->description)
		osync_free(plugin->description);
	plugin->description = osync_strdup(description);
}

const char *osync_plugin_get_external_command(OSyncPlugin *plugin)
{
	osync_assert(plugin);
	return plugin->external_command;
}

void osync_plugin_set_external_command(OSyncPlugin *plugin, const char *external_command)
{
	osync_assert(plugin);
	if (plugin->external_command)
		osync_free(plugin->external_command);
	plugin->external_command = osync_strdup(external_command);
}

void *osync_plugin_get_data(OSyncPlugin *plugin)
{
	osync_assert(plugin);
	return plugin->plugin_data;
}

void osync_plugin_set_data(OSyncPlugin *plugin, void *data)
{
	osync_assert(plugin);
	plugin->plugin_data = data;
}

OSyncPluginConfigurationType osync_plugin_get_config_type(OSyncPlugin *plugin)
{
	osync_assert(plugin);
	return plugin->config_type;
}

void osync_plugin_set_config_type(OSyncPlugin *plugin, OSyncPluginConfigurationType config_type)
{
	osync_assert(plugin);
	plugin->config_type = config_type;
}

OSyncStartType osync_plugin_get_start_type(OSyncPlugin *plugin)
{
	osync_assert(plugin);
	return plugin->start_type;
}

void osync_plugin_set_start_type(OSyncPlugin *plugin, OSyncStartType start_type)
{
	osync_assert(plugin);
	plugin->start_type = start_type;
}

void osync_plugin_set_initialize_func(OSyncPlugin *plugin, initialize_fn init)
{
	osync_assert(plugin);
	plugin->initialize = init;
}

void osync_plugin_set_finalize_func(OSyncPlugin *plugin, finalize_fn fin)
{
	osync_assert(plugin);
	
	plugin->finalize = fin;
}

void osync_plugin_set_discover_func(OSyncPlugin *plugin, discover_fn discover)
{
	osync_assert(plugin);
	plugin->discover = discover;
}

osync_bool osync_plugin_initialize(OSyncPlugin *plugin, void **plugin_data, OSyncPluginInfo *info, OSyncError **error)
{
	osync_assert(plugin);
	osync_assert(plugin_data);
	
	osync_return_val_if_fail_and_set_error(plugin, FALSE, error, OSYNC_ERROR_PARAMETER, "osync_plugin_initialize: plugin is null");
	osync_return_val_if_fail_and_set_error(plugin_data, FALSE, error, OSYNC_ERROR_PARAMETER, "osync_plugin_initialize: plugin_data is null");
	
	/* Just return with FALSE, if no initialize function is registered */ 
	osync_return_val_if_fail_and_set_error(plugin->initialize, FALSE, error, OSYNC_ERROR_INITIALIZATION, "plugin %s has no plugin initialize function", osync_plugin_get_name(plugin) );
	
	void *data = plugin->initialize(plugin, info, error);
	if (osync_error_is_set(error)) { 
		return FALSE;
	}
	*plugin_data = data;
	
	return TRUE;
}

void osync_plugin_finalize(OSyncPlugin *plugin, void *data)
{
	osync_assert(plugin);
	plugin->finalize(data);
}

osync_bool osync_plugin_discover(OSyncPlugin *plugin, void *data, OSyncPluginInfo *info, OSyncError **error)
{
	osync_assert(plugin);
	if (!plugin->discover)
		return TRUE;
		
	return plugin->discover(info, data, error);
}

osync_bool osync_plugin_is_usable(OSyncPlugin *plugin, OSyncError **error)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, plugin, error);
	
	if (plugin->useable && !plugin->useable(error)) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
		return FALSE;
	}
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
}

unsigned int osync_plugin_get_discover_timeout(OSyncPlugin *plugin)
{
	osync_assert(plugin);
	return plugin->timeout.discover;
}

void osync_plugin_set_discover_timeout(OSyncPlugin *plugin, unsigned int timeout)
{
	osync_assert(plugin);
	plugin->timeout.discover = timeout;
}

unsigned int osync_plugin_get_initialize_timeout(OSyncPlugin *plugin)
{
	osync_assert(plugin);
	return plugin->timeout.initialize;
}

void osync_plugin_set_initialize_timeout(OSyncPlugin *plugin, unsigned int timeout)
{
	osync_assert(plugin);
	plugin->timeout.initialize = timeout;
}

unsigned int osync_plugin_get_finalize_timeout(OSyncPlugin *plugin)
{
	osync_assert(plugin);
	return plugin->timeout.finalize;
}

void osync_plugin_set_finalize_timeout(OSyncPlugin *plugin, unsigned int timeout)
{
	osync_assert(plugin);
	plugin->timeout.finalize = timeout;
}

unsigned int osync_plugin_get_useable_timeout(OSyncPlugin *plugin)
{
	osync_assert(plugin);
	return plugin->timeout.useable;
}

void osync_plugin_set_useable_timeout(OSyncPlugin *plugin, unsigned int timeout)
{
	osync_assert(plugin);
	plugin->timeout.useable = timeout;
}

