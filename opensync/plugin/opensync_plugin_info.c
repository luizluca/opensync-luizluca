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

#include "opensync-capabilities.h"
#include "opensync-format.h"
#include "opensync-plugin.h"
#include "opensync_plugin_info_private.h"

#include "opensync-version.h"

OSyncPluginInfo *osync_plugin_info_new(OSyncError **error)
{
	OSyncPluginInfo *info = osync_try_malloc0(sizeof(OSyncPluginInfo), error);
	if (!info)
		return NULL;
	
	info->ref_count = 1;
	
	return info;
}

OSyncPluginInfo *osync_plugin_info_ref(OSyncPluginInfo *info)
{
	osync_assert(info);
	
	g_atomic_int_inc(&(info->ref_count));

	return info;
}

void osync_plugin_info_unref(OSyncPluginInfo *info)
{
	osync_assert(info);
	
	if (g_atomic_int_dec_and_test(&(info->ref_count))) {
		if (info->config)
			osync_plugin_config_unref(info->config);
		
		if (info->configdir)
			osync_free(info->configdir);
		
		if (info->groupname)
			osync_free(info->groupname);
		
		while (info->objtype_sinks) {
			OSyncObjTypeSink *sink = info->objtype_sinks->data;
			osync_objtype_sink_unref(sink);
			info->objtype_sinks = osync_list_remove(info->objtype_sinks, sink);
		}
		
		if (info->main_sink)
			osync_objtype_sink_unref(info->main_sink);
			
		if (info->version)
			osync_version_unref(info->version);
			
		if (info->capabilities)
			osync_capabilities_unref(info->capabilities);

		if (info->formatenv)
			osync_format_env_unref(info->formatenv);
		
		osync_free(info);
	}
}

void osync_plugin_info_set_loop(OSyncPluginInfo *info, void *loop)
{
	osync_assert(info);
	info->loop = loop;
}

void *osync_plugin_info_get_loop(OSyncPluginInfo *info)
{
	osync_assert(info);
	return info->loop;
}

void osync_plugin_info_set_config(OSyncPluginInfo *info, OSyncPluginConfig *config)
{
	osync_assert(info);
	osync_assert(config);

	if (info->config)
		osync_plugin_config_unref(config);

	osync_plugin_config_ref(config);
	info->config = config;
}

OSyncPluginConfig *osync_plugin_info_get_config(OSyncPluginInfo *info)
{
	osync_assert(info);
	return info->config;
}

void osync_plugin_info_set_configdir(OSyncPluginInfo *info, const char *configdir)
{
	osync_assert(info);
	if (info->configdir)
		osync_free(info->configdir);
	info->configdir = osync_strdup(configdir);
}

const char *osync_plugin_info_get_configdir(OSyncPluginInfo *info)
{
	osync_assert(info);
	return info->configdir;
}

void osync_plugin_info_set_groupname(OSyncPluginInfo *info, const char *groupname)
{
	osync_assert(info);
	if (info->groupname)
		osync_free(info->groupname);
	info->groupname = osync_strdup(groupname);
}

const char *osync_plugin_info_get_groupname(OSyncPluginInfo *info)
{
	osync_assert(info);
	return info->groupname;
}

OSyncObjTypeSink *osync_plugin_info_find_objtype(OSyncPluginInfo *info, const char *name)
{
	OSyncList *p;
	OSyncObjTypeSink *sink = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %s)", __func__, info, name);
	osync_assert(info);

	for (p = info->objtype_sinks; p; p = p->next) {
		sink = p->data;
		if (g_ascii_strcasecmp(osync_objtype_sink_get_name(sink), name) == 0)
			goto done;
	}
	
	osync_trace(TRACE_EXIT, "%s: NULL", __func__);
	return NULL;

 done:
	osync_trace(TRACE_EXIT, "%s: %p", __func__, sink);
	return sink;

}

void osync_plugin_info_add_objtype(OSyncPluginInfo *info, OSyncObjTypeSink *sink)
{
	osync_assert(info);
	info->objtype_sinks = osync_list_append(info->objtype_sinks, sink);
	osync_objtype_sink_ref(sink);
}

OSyncList *osync_plugin_info_get_objtype_sinks(OSyncPluginInfo *info) {
	osync_assert(info);
	return osync_list_copy(info->objtype_sinks);
}

OSyncObjTypeSink *osync_plugin_info_get_main_sink(OSyncPluginInfo *info)
{
	osync_assert(info);
	return info->main_sink;
}

void osync_plugin_info_set_main_sink(OSyncPluginInfo *info, OSyncObjTypeSink *sink)
{
	osync_assert(info);
	osync_assert(sink);
	info->main_sink = sink;
	osync_objtype_sink_ref(sink);
}

OSyncObjTypeSink *osync_plugin_info_get_sink(OSyncPluginInfo *info)
{
	osync_assert(info);
	return info->current_sink;
}

void osync_plugin_info_set_sink(OSyncPluginInfo *info, OSyncObjTypeSink *sink)
{
	osync_assert(info);
	osync_assert(sink);
	info->current_sink = sink;
}

OSyncFormatEnv *osync_plugin_info_get_format_env(OSyncPluginInfo *info)
{
	osync_assert(info);
	return info->formatenv;
}

void osync_plugin_info_set_format_env(OSyncPluginInfo *info, OSyncFormatEnv *env)
{
	osync_assert(info);
	osync_assert(env);
	if (info->formatenv)
		osync_format_env_unref(info->formatenv);

	info->formatenv = osync_format_env_ref(env);
}

void osync_plugin_info_set_version(OSyncPluginInfo *info, OSyncVersion *version)
{
	osync_assert(info);
	osync_assert(version);
	
	if(info->version)
		osync_version_unref(info->version);
	
	osync_version_ref(version);
	info->version = version;
}

OSyncVersion *osync_plugin_info_get_version(OSyncPluginInfo *info)
{
	osync_assert(info);
	return info->version;
}

void osync_plugin_info_set_capabilities(OSyncPluginInfo *info, OSyncCapabilities *capabilities)
{
	osync_assert(info);
	osync_assert(capabilities);
	
	if(info->capabilities)
		osync_capabilities_unref(info->capabilities);
	
	osync_capabilities_ref(capabilities);
	info->capabilities = capabilities;
}

OSyncCapabilities *osync_plugin_info_get_capabilities(OSyncPluginInfo *info)
{
	osync_assert(info);
	return info->capabilities;
}
