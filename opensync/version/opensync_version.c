/*
 * libopensync - A synchronization framework
 * Copyright (C) 2006  NetNix Finland Ltd <netnix@netnix.fi>
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
 * Author: Daniel Friedrich <daniel.friedrich@opensync.org>
 * 
 */
 
#include "opensync.h"
#include "opensync_internals.h"

#include "opensync-capabilities.h"
#include "capabilities/opensync-capabilities_internals.h"

#include "opensync-version.h"
#include "opensync-version_internals.h"
#include "opensync_version_private.h"

static int _osync_version_match(char *pattern, char* string, OSyncError **error)
{
	osync_assert(pattern);
	osync_assert(string);
	if(!strlen(pattern)) 
		return 1;
	/* Only newer versions of glib support regular expressions */
	/* On Windows the glib regular expressions are required    */
#if GLIB_MAJOR_VERSION < 3 && GLIB_MINOR_VERSION < 14
	regex_t *preg = osync_try_malloc0(sizeof(regex_t), error);
	if(!preg)
		goto error;
	
	int ret = regcomp(preg, pattern, 0);
	
	char *errbuf;
	size_t errbuf_size;
	if(ret) {
		errbuf_size = regerror(ret, preg, NULL, 0);
		errbuf = osync_try_malloc0(errbuf_size, error);
		if(!errbuf)
			goto error_and_free;
		regerror(ret, preg, errbuf, errbuf_size);
		osync_error_set(error, OSYNC_ERROR_GENERIC, "%s", errbuf);
		osync_free(errbuf);
		goto error_and_free;
	}
	
	ret = regexec(preg, string, 0, NULL, 0);
	regfree(preg);
	osync_free(preg);

	if(ret != 0) { 
		if(ret == REG_NOMATCH)
			return 0;
		errbuf_size = regerror(ret, preg, NULL, 0);
		errbuf = osync_try_malloc0(errbuf_size, error);
		if(!errbuf)
			goto error;
		regerror(ret, preg, errbuf, errbuf_size);
		osync_error_set(error, OSYNC_ERROR_GENERIC, "%s", errbuf);
		osync_free(errbuf);
		goto error;
	}
	return 1;

 error_and_free:	
	regfree(preg);
	osync_free(preg);
 error:
	return -1;

#else /* GLIB_MAJOR_VERSION < 3 && GLIB_MINOR_VERSION < 14 */
	return g_regex_match_simple(pattern, string, 0, 0);
#endif
}

OSyncList *osync_version_load_from_descriptions(OSyncError **error, const char *descriptiondir, const char *schemadir)
{
	GDir *dir = NULL;
	GError *gerror = NULL;
	const char *descpath = descriptiondir ? descriptiondir : OPENSYNC_DESCRIPTIONSDIR; 
	const char *schemapath = schemadir ? schemadir : OPENSYNC_SCHEMASDIR; 
	char *filename = NULL;
	const gchar *de = NULL;
	OSyncList *versions = NULL;
	OSyncVersion *version = NULL;
	xmlDocPtr doc;
	xmlNodePtr root;
	xmlNodePtr cur;
	xmlNodePtr child;
	
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, error);
	
	dir = g_dir_open(descpath, 0, &gerror);
	if (!dir) {
		/* If description directory doesn't exist (e.g. unittests), just ignore this. */
		osync_trace(TRACE_EXIT, "Unable to open directory %s: %s", descpath, gerror->message);
		g_error_free(gerror);
		return NULL;
	}
	
	while ((de = g_dir_read_name(dir))) {
		char *schemafilepath = NULL;
		osync_bool res;

		filename = osync_strdup_printf ("%s%c%s", descpath, G_DIR_SEPARATOR, de);
		
		if (!g_file_test(filename, G_FILE_TEST_IS_REGULAR) || !g_pattern_match_simple("*.xml", filename)) {
			osync_free(filename);
			continue;
		}
		
		doc = xmlReadFile(filename, NULL, XML_PARSE_NOBLANKS);
		if(!doc) {
			osync_free(filename);
			continue;
		}
		
		osync_free(filename);
		
		root = xmlDocGetRootElement(doc);
		if(!root || !xmlStrEqual(root->name, BAD_CAST "versions")) {
			osync_xml_free_doc(doc);
			continue;
		}

		schemafilepath = osync_strdup_printf("%s%c%s", schemapath, G_DIR_SEPARATOR, "descriptions.xsd");
		res = osync_xml_validate_document(doc, schemafilepath);
		osync_free(schemafilepath);

		if(res == FALSE) {
			osync_xml_free_doc(doc);
			continue;
		}
		
		cur = root->children;
		for(; cur != NULL; cur = cur->next) {
		
			version = osync_version_new(error);
			if(!version) {
				OSyncList *cur = NULL;
				osync_xml_free_doc(doc);
				cur = osync_list_first(versions);
				while(cur) {
					osync_version_unref(cur->data);
					cur = cur->next;	
				}
				goto error;
			}
				
			child = cur->children;
			osync_version_set_plugin(version, (const char *)osync_xml_node_get_content(child));
			child = child->next;
			osync_version_set_priority(version, (const char *)osync_xml_node_get_content(child));
			child = child->next;
			osync_version_set_vendor(version, (const char *)osync_xml_node_get_content(child));
			child = child->next;
			osync_version_set_modelversion(version, (const char *)osync_xml_node_get_content(child));
			child = child->next;
			osync_version_set_firmwareversion(version, (const char *)osync_xml_node_get_content(child));
			child = child->next;
			osync_version_set_softwareversion(version, (const char *)osync_xml_node_get_content(child));
			child = child->next;
			osync_version_set_hardwareversion(version, (const char *)osync_xml_node_get_content(child));
			child = child->next;
			osync_version_set_identifier(version, (const char *)osync_xml_node_get_content(child));
			
			versions = osync_list_append(versions, version);
		}
		
		osync_xml_free_doc(doc);
	}
	
	g_dir_close(dir);
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, versions);
	return versions;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

OSyncVersion *osync_version_new(OSyncError **error)
{
	OSyncVersion *version = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, error);
	
	version = osync_try_malloc0(sizeof(OSyncVersion), error);
	if(!version) {
		osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
		return NULL;
	}
	
	version->ref_count = 1;
	version->plugin = osync_strdup("");
	version->priority = osync_strdup("");
	version->vendor = osync_strdup("");
	version->modelversion = osync_strdup("");
	version->firmwareversion = osync_strdup("");
	version->softwareversion = osync_strdup("");
	version->hardwareversion = osync_strdup("");
	version->identifier = osync_strdup("");
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, version);
	return version;
}

OSyncVersion *osync_version_ref(OSyncVersion *version)
{
	osync_assert(version);
	
	g_atomic_int_inc(&(version->ref_count));

	return version;
}

void osync_version_unref(OSyncVersion *version)
{
	osync_assert(version);
			
	if (g_atomic_int_dec_and_test(&(version->ref_count))) {

		if(version->plugin)
			osync_free(version->plugin);
		if(version->priority)
			osync_free(version->priority);
		if(version->vendor)
			osync_free(version->vendor);
		if(version->modelversion)
			osync_free(version->modelversion);
		if(version->firmwareversion)
			osync_free(version->firmwareversion);
		if(version->softwareversion)
			osync_free(version->softwareversion);
		if(version->hardwareversion)
			osync_free(version->hardwareversion);
		if(version->identifier)
			osync_free(version->identifier);

		osync_free(version);
	}
}

char *osync_version_get_plugin(OSyncVersion *version)
{
	return version->plugin;
}

char *osync_version_get_priority(OSyncVersion *version)
{
	return version->priority;
}

char *osync_version_get_vendor(OSyncVersion *version)
{
	return version->vendor;
}

char *osync_version_get_modelversion(OSyncVersion *version)
{
	return version->modelversion;
}

char *osync_version_get_firmwareversion(OSyncVersion *version)
{
	return version->firmwareversion;
}

char *osync_version_get_softwareversion(OSyncVersion *version)
{
	return version->softwareversion;
}

char *osync_version_get_hardwareversion(OSyncVersion *version)
{
	return version->hardwareversion;
}

char *osync_version_get_identifier(OSyncVersion *version)
{
	return version->identifier;
}

void osync_version_set_plugin(OSyncVersion *version, const char *plugin)
{
	if(version->plugin)
		osync_free(version->plugin);
	if(!plugin)
		version->plugin = osync_strdup("");
	else
		version->plugin = osync_strdup(plugin);
}

void osync_version_set_priority(OSyncVersion *version, const char *priority)
{
	if(version->priority)
		osync_free(version->priority);
	if(!priority)
		version->priority = osync_strdup("");
	else
		version->priority = osync_strdup(priority);
}

void osync_version_set_vendor(OSyncVersion *version, const char *vendor)
{
	if(version->vendor)
		osync_free(version->vendor);
	if(!vendor)
		version->vendor = osync_strdup("");
	else
		version->vendor = osync_strdup(vendor);
}

void osync_version_set_modelversion(OSyncVersion *version, const char *modelversion)
{
	if(version->modelversion)
		osync_free(version->modelversion);
	if(!modelversion)
		version->modelversion = osync_strdup("");
	else
		version->modelversion = osync_strdup(modelversion);
}

void osync_version_set_firmwareversion(OSyncVersion *version, const char *firmwareversion)
{
	if(version->firmwareversion)
		osync_free(version->firmwareversion);
	if(!firmwareversion)
		version->firmwareversion = osync_strdup("");
	else
		version->firmwareversion = osync_strdup(firmwareversion);
}

void osync_version_set_softwareversion(OSyncVersion *version, const char *softwareversion)
{
	if(version->softwareversion)
		osync_free(version->softwareversion);
	if(!softwareversion)
		version->softwareversion = osync_strdup("");
	else
		version->softwareversion = osync_strdup(softwareversion);
	
}

void osync_version_set_hardwareversion(OSyncVersion *version, const char *hardwareversion)
{
	if(version->hardwareversion)
		osync_free(version->hardwareversion);
	if(!hardwareversion)
		version->hardwareversion = osync_strdup("");
	else
		version->hardwareversion = osync_strdup(hardwareversion);
}

void osync_version_set_identifier(OSyncVersion *version, const char *identifier)
{
	if(version->identifier)
		osync_free(version->identifier);
	if(!identifier)
		version->identifier = osync_strdup("");
	else
		version->identifier = osync_strdup(identifier);
}

int osync_version_matches(OSyncVersion *pattern, OSyncVersion *version, OSyncError **error)
{
	int ret;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, pattern, version, error);

	osync_assert(pattern);
	osync_assert(version);
	
	ret = _osync_version_match(osync_version_get_plugin(pattern), osync_version_get_plugin(version), error);
	if(ret <= 0)
		goto error;
	
	ret = _osync_version_match(osync_version_get_vendor(pattern), osync_version_get_vendor(version), error);
	if(ret <= 0)
		goto error;

	ret = _osync_version_match(osync_version_get_modelversion(pattern), osync_version_get_modelversion(version), error);
	if(ret <= 0)
		goto error;
	
	ret = _osync_version_match(osync_version_get_firmwareversion(pattern), osync_version_get_firmwareversion(version), error);
	if(ret <= 0)
		goto error;
	
	ret = _osync_version_match(osync_version_get_softwareversion(pattern), osync_version_get_softwareversion(version), error);
	if(ret <= 0)
		goto error;
	
	ret = _osync_version_match(osync_version_get_hardwareversion(pattern), osync_version_get_hardwareversion(version), error);
	if(ret <= 0)
		goto error;
	
	ret = atoi(osync_version_get_priority(pattern));

 error:
	if(ret >= 0) {
		osync_trace(TRACE_EXIT, "%s: %i" , __func__, ret);
		return ret;
	}
	osync_trace(TRACE_EXIT_ERROR, "%s: %s" , __func__, osync_error_print(error));
	return -1;
}

OSyncList *osync_version_load_from_default_descriptions(OSyncError **error)
{
	return osync_version_load_from_descriptions(error, NULL, NULL);
}

OSyncCapabilities *osync_version_find_capabilities(OSyncVersion *version, OSyncError **error)
{
	int priority = -1;
	OSyncVersion *winner = NULL;
	OSyncCapabilities *capabilities = NULL;
	OSyncList *versions = NULL;
	OSyncList *cur = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, version, error);
	osync_assert(version);

	versions = osync_version_load_from_default_descriptions(error);
	if (*error) /* versions can be null */
		goto error;

	cur = osync_list_first(versions);
	while(cur) {
		int curpriority = osync_version_matches(cur->data, version, error);
		if (curpriority == -1) {
			if (versions)
				osync_list_free(versions);

			if (winner)
				osync_version_unref(winner);

			goto error;
		}

		if( curpriority > 0 && curpriority > priority) {
			if(winner)
				osync_version_unref(winner);

			winner = cur->data;
			osync_version_ref(winner);
			priority = curpriority;
		}
		osync_version_unref(cur->data);
		cur = cur->next;
	}
	osync_list_free(versions);
	
	/* we found or own capabilities */
	if(priority > 0)
		{
			osync_trace(TRACE_INTERNAL, "Found capabilities file by version: %s ", (const char*)osync_version_get_identifier(winner));

			capabilities = osync_capabilities_load_identifier((const char*)osync_version_get_identifier(winner), error);
			osync_version_unref(winner);

			if (!capabilities)
				goto error;
		}

	osync_trace(TRACE_EXIT, "%s: %p", __func__, capabilities);
	return capabilities;

 error:	

	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

