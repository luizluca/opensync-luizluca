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
#include "plugin/opensync_objtype_sink_internals.h"
#include "plugin/opensync_plugin_config_internals.h"

#include "opensync-capabilities.h"
#include "opensync-group.h"
#include "opensync-format.h"
#include "opensync_member_internals.h"
#include "opensync_member_private.h"

#include "capabilities/opensync-capabilities_internals.h"

#include "common/opensync_xml_internals.h"

void osync_member_parse_timeout(xmlNode *cur, OSyncObjTypeSink *sink)
{
	osync_assert(sink);

	while (cur != NULL) {
		char *str = (char*)xmlNodeGetContent(cur);
		if (str) {
			if (!xmlStrcmp(cur->name, (const xmlChar *)"connect")) {
				osync_objtype_sink_set_connect_timeout(sink, atoi(str));
			} else if (!xmlStrcmp(cur->name, (const xmlChar *)"disconnect")) {
				osync_objtype_sink_set_disconnect_timeout(sink, atoi(str));
			} else if (!xmlStrcmp(cur->name, (const xmlChar *)"get_changes")) {
				osync_objtype_sink_set_getchanges_timeout(sink, atoi(str));
			} else if (!xmlStrcmp(cur->name, (const xmlChar *)"commit")) {
				osync_objtype_sink_set_commit_timeout(sink, atoi(str));
			} else if (!xmlStrcmp(cur->name, (const xmlChar *)"committed_all")) {
				osync_objtype_sink_set_committedall_timeout(sink, atoi(str));
			} else if (!xmlStrcmp(cur->name, (const xmlChar *)"sync_done")) {
				osync_objtype_sink_set_syncdone_timeout(sink, atoi(str));
			} else if (!xmlStrcmp(cur->name, (const xmlChar *)"read")) {
				osync_objtype_sink_set_read_timeout(sink, atoi(str));
			}
			osync_xml_free(str);
		}
		cur = cur->next;
	}
}

static OSyncObjTypeSink *osync_member_parse_objtype(xmlNode *cur, OSyncError **error)
{
	OSyncObjTypeSink *sink = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, cur, error);

	sink = osync_objtype_sink_new(NULL, error);
	if (!sink) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
		return NULL;
	}
	
	while (cur != NULL) {
		char *str = (char*)xmlNodeGetContent(cur);
		if (str) {
			if (!xmlStrcmp(cur->name, (const xmlChar *)"name")) {
				osync_objtype_sink_set_name(sink, str);
			} else if (!xmlStrcmp(cur->name, (const xmlChar *)"preferred_format")) {
				osync_objtype_sink_set_preferred_format(sink, str);
			} else if (!xmlStrcmp(cur->name, (const xmlChar *)"enabled")) {
				osync_objtype_sink_set_enabled(sink, atoi(str));
			} else if (!xmlStrcmp(cur->name, (const xmlChar *)"read")) {
				osync_objtype_sink_set_read(sink, atoi(str));
			} else if (!xmlStrcmp(cur->name, (const xmlChar *)"getchanges")) {
				osync_objtype_sink_set_getchanges(sink, atoi(str));
			} else if (!xmlStrcmp(cur->name, (const xmlChar *)"objformat")) {
				xmlChar *str_name = osync_xml_find_node(cur, "name");
				xmlChar *str_config = osync_xml_find_node(cur, "config");
				OSyncObjFormatSink *format_sink = osync_objformat_sink_new((char *)str_name, error);
				if (!format_sink)
					return NULL;

				osync_objformat_sink_set_config(format_sink, (char *)str_config);
				osync_objtype_sink_add_objformat_sink(sink, format_sink);
				osync_objformat_sink_unref(format_sink);

				osync_xml_free(str_name);
				osync_xml_free(str_config);
			} else if (!xmlStrcmp(cur->name, (const xmlChar *)"timeout")) {
				osync_member_parse_timeout(cur->xmlChildrenNode, sink);
			}
			osync_xml_free(str);
		}
		cur = cur->next;
	}
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, sink);
	return sink;
}

#ifdef OPENSYNC_UNITTESTS
void osync_member_set_schemadir(OSyncMember *member, const char *schemadir)
{
	osync_assert(member);
	osync_assert(schemadir);

	if (member->schemadir)
		osync_free(member->schemadir);

	member->schemadir = osync_strdup(schemadir); 
}
#endif /* OPENSYNC_UNITTESTS */

/*@}*/

OSyncMember *osync_member_new(OSyncError **error)
{
	OSyncMember *member = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, error);
	
	member = osync_try_malloc0(sizeof(OSyncMember), error);
	if (!member)
		goto error;

	member->ref_count = 1;

	member->alternative_objtype_table = g_hash_table_new_full(g_str_hash, g_str_equal, osync_free, osync_free);
	if (!member->alternative_objtype_table) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "No memory left");
		goto error;
	}
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, member);
	return member;
	
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %p", __func__, osync_error_print(error));
	return NULL;
}

OSyncMember *osync_member_ref(OSyncMember *member)
{
	osync_assert(member);
	
	g_atomic_int_inc(&(member->ref_count));

	return member;
}

void osync_member_unref(OSyncMember *member)
{
	osync_assert(member);
		
	if (g_atomic_int_dec_and_test(&(member->ref_count))) {

		if (member->pluginname)
			osync_free(member->pluginname);

		if (member->name)
			osync_free(member->name);

		if (member->configdir)
			osync_free(member->configdir);
		
		if (member->config)
			osync_plugin_config_unref(member->config);
			
		if (member->capabilities)
			osync_capabilities_unref(member->capabilities);

		if (member->alternative_objtype_table)
			g_hash_table_destroy(member->alternative_objtype_table);
			
		osync_member_flush_objtypes(member);

#ifdef OPENSYNC_UNITTESTS
		if (member->schemadir)
			osync_free(member->schemadir);

		if (member->default_configdir)
			osync_free(member->default_configdir);
#endif /* OPENSYNC_UNITTESTS */


		osync_free(member);
	}
}

const char *osync_member_get_pluginname(OSyncMember *member)
{
	osync_assert(member);
	return member->pluginname;
}

void osync_member_set_pluginname(OSyncMember *member, const char *pluginname)
{
	osync_assert(member);
	if (member->pluginname)
		osync_free(member->pluginname);
	member->pluginname = osync_strdup(pluginname);
}

const char *osync_member_get_name(OSyncMember *member)
{
	osync_assert(member);
	return member->name;
}

void osync_member_set_name(OSyncMember *member, const char *name)
{
	osync_assert(member);
	if (member->name)
		osync_free(member->name);
	member->name = osync_strdup(name);
}

const char *osync_member_get_configdir(OSyncMember *member)
{
	osync_assert(member);
	return member->configdir;
}

void osync_member_set_configdir(OSyncMember *member, const char *configdir)
{
	osync_assert(member);
	if (member->configdir)
		osync_free(member->configdir);
	member->configdir = osync_strdup(configdir);
}


#ifdef OPENSYNC_UNITTESTS
void osync_member_set_default_configdir(OSyncMember *member, const char *default_configdir)
{
	osync_assert(member);
	if (member->default_configdir)
		osync_free(member->default_configdir);
	member->default_configdir = osync_strdup(default_configdir);
}
#endif

OSyncPluginConfig *osync_member_get_config_or_default(OSyncMember *member, OSyncError **error)
{
	char *filename = NULL;
	OSyncPluginConfig *config = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, member, error);
	g_assert(member);

	if (member->config) {
		osync_trace(TRACE_EXIT, "%s: Configdata already in memory", __func__);
		return member->config;
	}

	filename = osync_strdup_printf("%s"G_DIR_SEPARATOR_S"%s.conf", member->configdir, member->pluginname);

	config = osync_plugin_config_new(error);
	if (!config)
		goto error;
	
	osync_trace(TRACE_INTERNAL, "Reading %s", filename);
	if (!g_file_test(filename, G_FILE_TEST_IS_REGULAR)) {
		osync_free(filename);
#ifdef OPENSYNC_UNITTESTS
		filename = osync_strdup_printf("%s"G_DIR_SEPARATOR_S"%s",
				member->default_configdir ? member->default_configdir : osync_plugin_get_default_configdir(),
				member->pluginname);
#else
		filename = osync_strdup_printf("%s%s",
			osync_plugin_get_default_configdir(),
			member->pluginname);
#endif		
		osync_trace(TRACE_INTERNAL, "Reading default %s", filename);
	}

#ifdef OPENSYNC_UNITTESTS
	if (member->schemadir)
		osync_plugin_config_set_schemadir(config, member->schemadir);
#endif

	if (!osync_plugin_config_file_load(config, filename, error))
		goto error_free_config;
		
	osync_member_set_config(member, config);
	osync_plugin_config_unref(config);

	osync_free(filename);

	osync_trace(TRACE_EXIT, "%s: Read default config", __func__);
	return config;

 error_free_config:
	osync_plugin_config_unref(config);
 error:
	osync_free(filename);
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

osync_bool osync_member_has_config(OSyncMember *member)
{
	osync_assert(member);
	return member->config ? TRUE : FALSE;
}

OSyncPluginConfig *osync_member_get_config(OSyncMember *member, OSyncError **error)
{
	char *filename = NULL;
	OSyncPluginConfig *config = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, member, error);
	osync_assert(member);

	if (member->config) {
		osync_trace(TRACE_EXIT, "%s: Configdata already in memory", __func__);
		return member->config;
	}
	
	filename = osync_strdup_printf("%s%c%s.conf", member->configdir, G_DIR_SEPARATOR, member->pluginname);
	osync_trace(TRACE_INTERNAL, "Reading config from: %s", filename);
	
	if (!g_file_test(filename, G_FILE_TEST_IS_REGULAR)) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Plugin is not configured");
		goto error;
	}

	config = osync_plugin_config_new(error);
	if (!config)
		goto error;

#ifdef OPENSYNC_UNITTESTS
	if (member->schemadir)
		osync_plugin_config_set_schemadir(config, member->schemadir);
#endif

	if (!osync_plugin_config_file_load(config, filename, error))
		goto error_free_config;

	osync_free(filename);

	osync_member_set_config(member, config);
	osync_plugin_config_unref(config);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return config;

 error_free_config:
	osync_plugin_config_unref(config);
 error:
	osync_free(filename);
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

void osync_member_set_config(OSyncMember *member, OSyncPluginConfig *config)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, member, config);
	g_assert(member);
	
	if (member->config)
		osync_plugin_config_unref(member->config);

	member->config = osync_plugin_config_ref(config);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

osync_bool osync_member_load(OSyncMember *member, const char *path, OSyncError **error)
{	xmlDocPtr doc;
	xmlNodePtr cur;
	char *filename = NULL;
	char *basename = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %s, %p)", __func__, member, path, error);

	filename = osync_strdup_printf ("%s%csyncmember.conf", path, G_DIR_SEPARATOR);
	
	basename = g_path_get_basename(path);
	member->id = atoi(basename);
	g_free(basename);
	osync_member_set_configdir(member, path);
	
	if (!osync_xml_open_file(&doc, &cur, filename, "syncmember", error)) {
		osync_free(filename);
		goto error;
	}
	osync_free(filename);

	while (cur != NULL) {
		char *str = (char*)xmlNodeGetContent(cur);
		if (str) {
			if (!xmlStrcmp(cur->name, (const xmlChar *)"pluginname")) {
				member->pluginname = osync_strdup(str);
			} else if (!xmlStrcmp(cur->name, BAD_CAST "name")) {
				osync_member_set_name(member, str);
			} else if (!xmlStrcmp(cur->name, (const xmlChar *)"objtype")) {
				OSyncObjTypeSink *sink = osync_member_parse_objtype(cur->xmlChildrenNode, error);
				if (!sink)
					goto error_free_doc;

				osync_member_add_objtype_sink(member, sink);
				osync_objtype_sink_unref(sink);
			} else if (!xmlStrcmp(cur->name, (const xmlChar *)"timeout")) {
				/* Sink Function Timeout settings for main sink */
				if (!member->main_sink)
					member->main_sink = osync_objtype_main_sink_new(error);

				if (!member->main_sink) {
					osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
					goto error_free_doc;
				}

				osync_member_parse_timeout(cur->xmlChildrenNode, member->main_sink);
			}

			osync_xml_free(str);
		}
		cur = cur->next;
	}
	osync_xml_free_doc(doc);

	if (osync_member_has_capabilities(member)) {
		OSyncCapabilities* capabilities = osync_member_load_capabilities(member, error);
		if (!capabilities)
			goto error;
		if (!osync_member_set_capabilities(member, capabilities, error))
			goto error;
		osync_capabilities_unref(capabilities);
	}
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
	
 error_free_doc:
	osync_xml_free_doc(doc);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool _osync_member_save_sink_add_timeout(xmlNode *cur, const char *timeoutname, unsigned int timeout, OSyncError **error)
{
	char *str = NULL;
	/* Skip if no custom timeout got set */
	if (!timeout)
		return TRUE;

	str = osync_strdup_printf("%u", timeout);
	xmlNewChild(cur, NULL, (xmlChar*)timeoutname, BAD_CAST str);
	osync_free(str);

	return TRUE;
}

static osync_bool _osync_member_save_sink_timeout(xmlNode *cur, OSyncObjTypeSink *sink, OSyncError **error)
{
	xmlNode *node = xmlNewChild(cur, NULL, (xmlChar*)"timeout", NULL);

	_osync_member_save_sink_add_timeout(node, "connect", osync_objtype_sink_get_connect_timeout(sink), error);
	_osync_member_save_sink_add_timeout(node, "disconnect", osync_objtype_sink_get_disconnect_timeout(sink), error);
	_osync_member_save_sink_add_timeout(node, "get_changes", osync_objtype_sink_get_getchanges_timeout(sink), error);
	_osync_member_save_sink_add_timeout(node, "commit", osync_objtype_sink_get_commit_timeout(sink), error);
	_osync_member_save_sink_add_timeout(node, "committed_all", osync_objtype_sink_get_committedall_timeout(sink), error);
	_osync_member_save_sink_add_timeout(node, "sync_done", osync_objtype_sink_get_syncdone_timeout(sink), error);
	_osync_member_save_sink_add_timeout(node, "read", osync_objtype_sink_get_read_timeout(sink), error);

	if (!node->children) { 
		xmlUnlinkNode(node);
		xmlFreeNode(node);
	}


	return TRUE;
}

static osync_bool _osync_member_save_sink(xmlDoc *doc, OSyncObjTypeSink *sink, OSyncError **error)
{
	unsigned int i = 0;
	xmlNode *node = NULL;

	/* Write main sink stuff in main node. */
	if (sink && !osync_objtype_sink_get_name(sink)) {
		node = doc->children;
	} else {
		node = xmlNewChild(doc->children, NULL, (xmlChar*)"objtype", NULL);
	}

	xmlNewChild(node, NULL, (xmlChar*)"enabled", osync_objtype_sink_is_enabled(sink) ? (xmlChar*)"1" : (xmlChar*)"0");
	xmlNewChild(node, NULL, (xmlChar*)"read", osync_objtype_sink_get_read(sink) ? (xmlChar*)"1" : (xmlChar*)"0");
	xmlNewChild(node, NULL, (xmlChar*)"getchanges", osync_objtype_sink_get_getchanges(sink) ? (xmlChar*)"1" : (xmlChar*)"0");

	/* Check if sink is a Main Sink, if so skip objtype specific stuff */
	if (sink && !osync_objtype_sink_get_name(sink))
		return TRUE;

	/* Objtype specific settings */
	xmlNewChild(node, NULL, (xmlChar*)"name", (xmlChar*)osync_objtype_sink_get_name(sink));

	xmlNewChild(node, NULL, (xmlChar*)"preferred_format", (xmlChar*)osync_objtype_sink_get_preferred_format(sink));

	for (i = 0; i < osync_objtype_sink_num_objformat_sinks(sink); i++) {
		OSyncObjFormatSink *format_sink = osync_objtype_sink_nth_objformat_sink(sink, i);
		osync_assert(format_sink);
		const char *format = osync_objformat_sink_get_objformat(format_sink);
		const char *format_config = osync_objformat_sink_get_config(format_sink);
		xmlNode *objformat_node = xmlNewChild(node, NULL, (xmlChar*)"objformat", NULL);
		xmlNewChild(objformat_node, NULL, (xmlChar*)"name", (xmlChar*)format);
		xmlNewChild(objformat_node, NULL, (xmlChar*)"config", (xmlChar*)format_config);
	}

	return _osync_member_save_sink_timeout(node, sink, error);
}


osync_bool osync_member_save(OSyncMember *member, OSyncError **error)
{
	const char *membername = NULL;
	char *filename = NULL;
	xmlDocPtr doc = NULL;
	char *version_str = NULL;
	GList *o = NULL;
	OSyncCapabilities* capabilities = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, member, error);
	osync_assert(member);
	osync_assert(member->configdir);
	
	if (!g_file_test(member->configdir, G_FILE_TEST_IS_DIR)) {
		/* g_mkdir -> g_mkdir_with_parent: Regression testcase:
		 * member_configdir_deep_path
		 */
		if (g_mkdir_with_parents(member->configdir, 0700)) {
			osync_error_set(error, OSYNC_ERROR_IO_ERROR, "Unable to create directory for member %i\n", member->id);
			goto error;
		}
	}
	
	doc = xmlNewDoc((xmlChar*)"1.0");
	doc->children = xmlNewDocNode(doc, NULL, (xmlChar*)"syncmember", NULL);

	version_str = osync_strdup_printf("%u.%u", OSYNC_MEMBER_MAJOR_VERSION, OSYNC_MEMBER_MINOR_VERSION);
	xmlSetProp(doc->children, (const xmlChar*)"version", (const xmlChar *)version_str);	
	osync_free(version_str);

	//The plugin name
	xmlNewChild(doc->children, NULL, (xmlChar*)"pluginname", (xmlChar*)member->pluginname);

	/* The member name (optional) */
	if ((membername = osync_member_get_name(member)))
		xmlNewChild(doc->children, NULL, BAD_CAST "name", BAD_CAST membername);

	//The main sink
	if (member->main_sink && !_osync_member_save_sink(doc, member->main_sink, error)) {
		osync_xml_free_doc(doc);
		goto error;
	}
	
	//The objtypes
	for (o = member->objtypes; o; o = o->next) {
		OSyncObjTypeSink *sink = o->data;

		if (!_osync_member_save_sink(doc, sink, error)) {
			osync_xml_free_doc(doc);
			goto error;
		}
	}
	
	/* TODO Validate file before storing! */

	//Saving the syncmember.conf
	filename = osync_strdup_printf ("%s%csyncmember.conf", member->configdir, G_DIR_SEPARATOR);
	xmlSaveFormatFile(filename, doc, 1);
	osync_free(filename);

	osync_xml_free_doc(doc);
	
	//Saving the config if it exists
	if (member->config) {
		filename = osync_strdup_printf("%s%c%s.conf", member->configdir, G_DIR_SEPARATOR, member->pluginname);
		if (!osync_plugin_config_file_save(member->config, filename, error)) {
			osync_free(filename);
			goto error;
		}
		
		osync_free(filename);
	}
	
	capabilities = osync_member_get_capabilities(member);
	if (capabilities) {
		if (!osync_member_save_capabilities(member, capabilities, error)) {
			goto error;
		}
	}
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_member_delete(OSyncMember *member, OSyncError **error)
{
	char *delcmd = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, member, error);
	osync_assert(member);
	
	delcmd = osync_strdup_printf("rm -rf %s", member->configdir);
	if (system(delcmd)) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Failed to delete member. command %s failed", delcmd);
		osync_free(delcmd);
		osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
		return FALSE;
	}
	osync_free(delcmd);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
}

osync_memberid osync_member_get_id(OSyncMember *member)
{
	osync_assert(member);
	return member->id;
}

OSyncObjTypeSink *osync_member_find_objtype_sink(OSyncMember *member, const char *objtype)
{
	GList *o;

	osync_assert(member);

	for (o = member->objtypes; o; o = o->next) {
		OSyncObjTypeSink *sink = o->data;
		if (!strcmp(osync_objtype_sink_get_name(sink), objtype)) {
			return sink;
		}
	}
	return NULL;
}

void osync_member_add_objformat(OSyncMember *member, const char *objtype, const char *format, OSyncError **error)
{
	OSyncObjTypeSink *sink = osync_member_find_objtype_sink(member, objtype);
	OSyncObjFormatSink *format_sink = NULL;
	if (!sink) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to find objtype %s", objtype);
		return;
	}
	
	format_sink = osync_objformat_sink_new(format, error);
	if (!format_sink) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to create sink for format %s", format);
		return;
	}
	osync_objtype_sink_add_objformat_sink(sink, format_sink);
	osync_objformat_sink_unref(format_sink);
}

void osync_member_add_objformat_with_config(OSyncMember *member, const char *objtype, const char *format, const char *format_config, OSyncError **error)
{
	OSyncObjTypeSink *sink = osync_member_find_objtype_sink(member, objtype);
	OSyncObjFormatSink *format_sink = NULL;
	if (!sink) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to find objtype %s", objtype);
		return;
	}
	
	format_sink = osync_objformat_sink_new(format, error);
	if (!format_sink) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to create sink for format %s", format);
		return;
	}
	osync_objformat_sink_set_config(format_sink, format_config);
	osync_objtype_sink_add_objformat_sink(sink, format_sink);
	osync_objformat_sink_unref(format_sink);
}

const OSyncList *osync_member_get_objformats(OSyncMember *member, const char *objtype, OSyncError **error)
{
	OSyncObjTypeSink *sink = osync_member_find_objtype_sink(member, objtype);
	if (!sink) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to find objtype %s", objtype);
		return NULL;
	}
	
	return osync_objtype_sink_get_objformat_sinks(sink);
}

OSyncList *osync_member_get_objformat_sinks_all(OSyncMember *member)
{
	GList *o;
	OSyncList *list = NULL;

	for (o = member->objtypes; o; o = o->next) {
		OSyncObjTypeSink *sink = o->data;
		OSyncList *format_sinks = osync_objtype_sink_get_objformat_sinks(sink);

		list = osync_list_concat(list, format_sinks);
	}

	return list;
}

OSyncObjFormat *osync_member_support_targetformat(OSyncMember *member, OSyncFormatEnv *formatenv, OSyncObjFormat *targetformat)
{
	OSyncError *error = NULL;
	GList *o;

	for (o = member->objtypes; o; o = o->next) {
		OSyncObjTypeSink *sink = o->data;
		OSyncList *format_sinks = osync_objtype_sink_get_objformat_sinks(sink);
		OSyncList *fs = NULL;
		for (fs = format_sinks; fs; fs = fs->next) {
			OSyncObjFormatSink *format_sink = fs->data;
			const char *objformat_name = osync_objformat_sink_get_objformat(format_sink);
			OSyncObjFormat *sourceformat = osync_format_env_find_objformat(formatenv, objformat_name);

			OSyncFormatConverterPath *path = osync_format_env_find_path(formatenv, sourceformat, targetformat, &error);
			if (path) {
				/* unref right away, since only using pointer as
				 * success flag */
				osync_converter_path_unref(path);

				osync_list_free(format_sinks);
				return sourceformat;
			}
			else {
				/* TODO error handling */
				/* log it for now? */
				osync_trace(TRACE_ERROR, "%s", osync_error_print(&error));
				osync_error_unref(&error);
			}
		}
		osync_list_free(format_sinks);
	}

	return NULL;
}

const char *osync_member_get_alternative_objtype(OSyncMember *member, const char *orig_objtype)
{
	const char *alternative_objtype = NULL;
		
	alternative_objtype = g_hash_table_lookup(member->alternative_objtype_table, orig_objtype);

	return alternative_objtype;
}

void osync_member_add_alternative_objtype(OSyncMember *member, const char *native_objtype, const char *alternative_objtype)
{
	g_hash_table_insert(member->alternative_objtype_table, osync_strdup(alternative_objtype), osync_strdup(native_objtype));
}

OSyncList *osync_member_get_all_objformats(OSyncMember *member)
{
	OSyncList *o, *format_sinks, *list = NULL;

	format_sinks = osync_member_get_objformat_sinks_all(member);
	for (o = format_sinks; o; o = o->next) {
		OSyncObjFormatSink *format_sink = o->data;
		const char *objformat;
		objformat = osync_objformat_sink_get_objformat(format_sink);

		list = osync_list_prepend(list, (void*)objformat);
	}
	osync_list_free(format_sinks);

	return list;
}

void osync_member_add_objtype_sink(OSyncMember *member, OSyncObjTypeSink *sink)
{
	osync_assert(member);
	osync_assert(sink);

	member->objtypes = g_list_append(member->objtypes, sink);
	osync_objtype_sink_ref(sink);
}

void osync_member_remove_objtype_sink(OSyncMember *member, OSyncObjTypeSink *sink)
{
	osync_assert(member);
	osync_assert(sink);

	member->objtypes = g_list_remove(member->objtypes, sink);
	osync_objtype_sink_unref(sink);
}

int osync_member_num_objtypes(OSyncMember *member)
{
	osync_assert(member);
	return g_list_length(member->objtypes);
}

const char *osync_member_nth_objtype(OSyncMember *member, int nth)
{
	OSyncObjTypeSink *sink = NULL;
	osync_assert(member);
	sink = g_list_nth_data(member->objtypes, nth);
	return osync_objtype_sink_get_name(sink);
}

OSyncList *osync_member_get_objtypes(OSyncMember *member) {
	GList *list = member->objtypes;
	OSyncList *new_list = NULL;
	OSyncObjTypeSink *sink = NULL;
	
	if (list) {
		OSyncList *last;

		new_list = osync_list_alloc();
		sink = (OSyncObjTypeSink*)list->data;
		new_list->data = (char *)osync_objtype_sink_get_name(sink);
		new_list->prev = NULL;
		last = new_list;
		list = list->next;
		while (list) {
			last->next = osync_list_alloc();
			last->next->prev = last;
			last = last->next;
			sink = (OSyncObjTypeSink*)list->data;
			last->data = (char *)osync_objtype_sink_get_name(sink);
			list = list->next;
		}
		last->next = NULL;
	}

	return new_list;
}

osync_bool osync_member_objtype_enabled(OSyncMember *member, const char *objtype)
{
	OSyncObjTypeSink *sink = NULL;
	osync_assert(member);
	sink = osync_member_find_objtype_sink(member, objtype);
	if (!sink)
		return FALSE;
	return osync_objtype_sink_is_enabled(sink);
}

void osync_member_set_objtype_enabled(OSyncMember *member, const char *objtype, osync_bool enabled)
{
	OSyncObjTypeSink *sink = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %s, %i)", __func__, member, objtype, enabled);
	osync_assert(member);
	
	sink = osync_member_find_objtype_sink(member, objtype);
	if (!sink) {
		osync_trace(TRACE_EXIT, "%s: Unable to find objtype", __func__);
		return;
	}
		
	osync_objtype_sink_set_enabled(sink, enabled);
	osync_trace(TRACE_EXIT, "%s", __func__);
}

OSyncCapabilities *osync_member_get_capabilities(OSyncMember *member)
{
	osync_assert(member);
	return member->capabilities;
}

/* FIXME - why has this bool as return value - there is never return FALSE! */
osync_bool osync_member_set_capabilities(OSyncMember *member, OSyncCapabilities *capabilities, OSyncError **error)
{
	osync_assert(member);
	
	if (member->capabilities)
		osync_capabilities_unref(member->capabilities);

	member->capabilities = capabilities;

	if(capabilities) {
		osync_capabilities_ref(member->capabilities);
	}
	return TRUE;
}

void osync_member_flush_objtypes(OSyncMember *member)
{
	osync_assert(member);

	while (member->objtypes) {
		OSyncObjTypeSink *sink = member->objtypes->data;
		osync_member_remove_objtype_sink(member, sink);
	}

	if (member->main_sink) {
		osync_objtype_sink_unref(member->main_sink);
		member->main_sink = NULL;
	}
}

OSyncObjTypeSink *osync_member_get_main_sink(OSyncMember *member)
{
	osync_assert(member);
	return member->main_sink;
}

osync_bool osync_member_config_is_uptodate(OSyncMember *member)
{
	xmlDocPtr doc;
	xmlNodePtr cur;
	OSyncError *error = NULL;
	unsigned int version_major;
	unsigned int version_minor;
	xmlChar *version_str = NULL;
	osync_bool uptodate = FALSE;
	char *config = NULL;

	osync_assert(member);
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, member);

	config = osync_strdup_printf("%s%c%s",
	                             osync_member_get_configdir(member),
	                             G_DIR_SEPARATOR, "syncmember.conf");
	
	/* If syncmember isn't present, we assume that update is required. */
	if (!osync_xml_open_file(&doc, &cur, config, "syncmember", &error))
		goto end;

	version_str = xmlGetProp(cur->parent, (const xmlChar *)"version");

	/* No version node, means very outdated version. */
	if (!version_str)
		goto end;

	sscanf((const char *) version_str, "%u.%u", &version_major, &version_minor);

	osync_trace(TRACE_INTERNAL, "Version: %s (current %u.%u required %u.%u)",
	            version_str, version_major, version_minor, 
	            OSYNC_MEMBER_MAJOR_VERSION, OSYNC_MEMBER_MINOR_VERSION );

	if (OSYNC_MEMBER_MAJOR_VERSION == version_major 
	    && OSYNC_MEMBER_MINOR_VERSION == version_minor)
		uptodate = TRUE;

	osync_xml_free(version_str);

 end:
	osync_free(config);

	osync_trace(TRACE_EXIT, "%s(%p)", __func__, member);
	return uptodate;
}

osync_bool osync_member_plugin_is_uptodate(OSyncMember *member)
{
	xmlDocPtr doc = NULL;
	xmlNodePtr cur = NULL;
	OSyncError *error = NULL;
	unsigned int version_major;
	unsigned int version_minor;
	xmlChar *version_str = NULL;
	osync_bool uptodate = FALSE;
	char *config = NULL;

	osync_assert(member);
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, member);

	config = osync_strdup_printf("%s%c%s",
	                             osync_member_get_configdir(member),
	                             G_DIR_SEPARATOR, osync_member_get_pluginname(member));
	
	/* If syncmember isn't present, we assume that update is required. */
	if (!osync_xml_open_file(&doc, &cur, config, "plugin", &error))
		goto end;

	version_str = xmlGetProp(cur->parent, (const xmlChar *)"version");

	/* No version node, means very outdated version. */
	if (!version_str)
		goto end;

	sscanf((const char *) version_str, "%u.%u", &version_major, &version_minor);

	osync_trace(TRACE_INTERNAL, "Version: %s (current %u.%u required %u.%u)",
	            version_str, version_major, version_minor, 
	            OSYNC_PLUGIN_MAJOR_VERSION, OSYNC_PLUGIN_MINOR_VERSION ); 

	if (OSYNC_PLUGIN_MAJOR_VERSION == version_major 
			&& OSYNC_PLUGIN_MINOR_VERSION == version_minor)
		uptodate = TRUE;

	osync_xml_free(version_str);

 end:
	osync_free(config);

	if (doc)
		osync_xml_free_doc(doc);

	osync_trace(TRACE_EXIT, "%s(%p)", __func__, member);
	return uptodate;
}

osync_bool osync_member_has_capabilities(OSyncMember *member)
{
	char *filename = NULL;
	gboolean res = FALSE;

	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, member);
	osync_assert(member);
	
	filename = osync_strdup_printf("%s%ccapabilities.xml", osync_member_get_configdir(member), G_DIR_SEPARATOR);
	res = g_file_test(filename, G_FILE_TEST_IS_REGULAR);
	osync_free(filename);
	
	osync_trace(TRACE_EXIT, "%s: %i", __func__, res);
	return res;
}

OSyncCapabilities* osync_member_load_capabilities(OSyncMember *member, OSyncError** error)
{
	char *filename;
	OSyncCapabilities *capabilities;

	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, member, error);
	osync_assert(member);
	
	filename = osync_strdup_printf("%s%ccapabilities.xml", osync_member_get_configdir(member), G_DIR_SEPARATOR);
	capabilities = osync_capabilities_load(filename, error);
	osync_free(filename);

	if (!capabilities)
		goto error;

	osync_trace(TRACE_EXIT, "%s: %p", __func__, capabilities);
	return capabilities;

error:	
	osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
	return NULL;
}

osync_bool osync_member_save_capabilities(OSyncMember *member, OSyncCapabilities* capabilities, OSyncError** error)
{
	char *filename;
	osync_bool res;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, member, capabilities, error);
	osync_assert(member);
	osync_assert(capabilities);
	

	filename = osync_strdup_printf("%s%ccapabilities.xml", osync_member_get_configdir(member), G_DIR_SEPARATOR);
	res = osync_capabilities_save(capabilities, filename, error);
	osync_free(filename);
	if (!res)
		goto error;
	osync_trace(TRACE_EXIT, "%s: %i", __func__, res);
	return res;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
	return FALSE;
}

