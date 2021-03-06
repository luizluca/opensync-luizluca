/*
 * libopensync - A synchronization framework
 * Copyright (C) 2008  Daniel Gollub <gollub@b1-systems.de>
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
#include "opensync-format.h"

#include "common/opensync_xml_internals.h"

#include "opensync_plugin_advancedoptions_private.h"	/* FIXME: direct access to private header */
#include "opensync_plugin_advancedoptions_internals.h"
#include "opensync_plugin_authentication_private.h"	/* FIXME: direct access to private header */
#include "opensync_plugin_connection_private.h"		/* FIXME: direct access to private header */
#include "opensync_plugin_connection_internals.h"
#include "opensync_plugin_localization_private.h"	/* FIXME: direct access to private header */
#include "opensync_plugin_resource_private.h"		/* FIXME: direct access to private header */
#include "opensync_plugin_externalplugin_private.h"	/* FIXME: direct access to private header */

#include "opensync_plugin_config_private.h"
#include "opensync_plugin_config_internals.h"

static OSyncPluginAdvancedOptionParameter *_osync_plugin_config_parse_advancedoption_param(OSyncPluginAdvancedOption *option, xmlNode *cur, OSyncError **error)
{
	OSyncPluginAdvancedOptionParameter *param = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, option, cur, error);

	param = osync_plugin_advancedoption_param_new(error);
	if (!param)
		goto error;

	for (; cur != NULL; cur = cur->next) {
		char *str = NULL;

		if (cur->type != XML_ELEMENT_NODE)
			continue;

		str = (char*)xmlNodeGetContent(cur);
		if (!str)
			continue;

		if (!xmlStrcmp(cur->name, BAD_CAST "DisplayName"))
			osync_plugin_advancedoption_param_set_displayname(param, str);
		else if (!xmlStrcmp(cur->name, BAD_CAST "Name"))
			osync_plugin_advancedoption_param_set_name(param, str);
		else if (!xmlStrcmp(cur->name, BAD_CAST "Type"))
			osync_plugin_advancedoption_param_set_type(param, 
			                                           osync_plugin_advancedoption_type_string_to_val(str));
		else if (!xmlStrcmp(cur->name, BAD_CAST "ValEnum"))
			osync_plugin_advancedoption_param_add_valenum(param, str);
		else if (!xmlStrcmp(cur->name, BAD_CAST "Value"))
			osync_plugin_advancedoption_param_set_value(param, str);

		osync_xml_free(str);
	}

	osync_trace(TRACE_EXIT, "%s", __func__);
	return param;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

static OSyncPluginAdvancedOption *_osync_plugin_config_parse_advancedoption(OSyncPluginConfig *config, xmlNode *cur, OSyncError **error)
{
	OSyncPluginAdvancedOptionParameter *param = NULL;
	OSyncPluginAdvancedOption *option = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, config, cur, error);

	option = osync_plugin_advancedoption_new(error);
	if (!option)
		goto error;

	for (; cur != NULL; cur = cur->next) {
		char *str = NULL;
		if (cur->type != XML_ELEMENT_NODE)
			continue;

		str = (char*)xmlNodeGetContent(cur);
		if (!str)
			continue;

		if (!xmlStrcmp(cur->name, BAD_CAST "DisplayName"))
			osync_plugin_advancedoption_set_displayname(option, str);
		else if (!xmlStrcmp(cur->name, BAD_CAST "MaxOccurs"))
			osync_plugin_advancedoption_set_maxoccurs(option, atoi(str));
		else if (!xmlStrcmp(cur->name, BAD_CAST "Max"))
			osync_plugin_advancedoption_set_max(option, atoi(str));
		else if (!xmlStrcmp(cur->name, BAD_CAST "Min"))
			osync_plugin_advancedoption_set_min(option, atoi(str));
		else if (!xmlStrcmp(cur->name, BAD_CAST "Name"))
			osync_plugin_advancedoption_set_name(option, str);
		else if (!xmlStrcmp(cur->name, BAD_CAST "Parameter")) {
			if (!(param = _osync_plugin_config_parse_advancedoption_param(option, cur->xmlChildrenNode, error)))
				goto error;

			option->parameters = osync_list_prepend(option->parameters, param);

		} else if (!xmlStrcmp(cur->name, BAD_CAST "Type")) {
			osync_plugin_advancedoption_set_type(option, osync_plugin_advancedoption_type_string_to_val(str));
		} else if (!xmlStrcmp(cur->name, BAD_CAST "ValEnum")) {
			osync_plugin_advancedoption_add_valenum(option, str);
		} else if (!xmlStrcmp(cur->name, BAD_CAST "Value")) {
			osync_plugin_advancedoption_set_value(option, str);
		}

		osync_xml_free(str);
	}

	option->parameters = osync_list_reverse(option->parameters);

	osync_trace(TRACE_EXIT, "%s: %p", __func__, option);
	return option;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

static osync_bool _osync_plugin_config_parse_advancedoptions(OSyncPluginConfig *config, xmlNode *cur, OSyncError **error)
{
	OSyncPluginAdvancedOption *option;

	for (; cur != NULL; cur = cur->next) {

		if (cur->type != XML_ELEMENT_NODE)
			continue;

		if (!xmlStrcmp(cur->name, (const xmlChar *)"AdvancedOption")) {
			if (!(option = _osync_plugin_config_parse_advancedoption(config, cur->xmlChildrenNode, error)))
				goto error;

			config->advancedoptions = osync_list_prepend(config->advancedoptions, option);
		}
	}

	config->advancedoptions = osync_list_reverse(config->advancedoptions);

	return TRUE;

 error:
	return FALSE; 
}

static osync_bool _osync_plugin_config_parse_authentication(OSyncPluginConfig *config, xmlNode *cur, OSyncError **error)
{
	OSyncPluginAuthentication *auth = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, config, cur, error);

	if (cur == NULL) { // don't set auth if Authentication tag is empty
		osync_trace(TRACE_EXIT, "%s", __func__);
		return TRUE;	
	}
	
	auth = osync_plugin_authentication_new(error);
	if (!auth)
		goto error;

	for (; cur != NULL; cur = cur->next) {
		char *str = NULL;
		if (cur->type != XML_ELEMENT_NODE)
			continue;

		str = (char*)xmlNodeGetContent(cur);
		if (!str)
			continue;

		if (!xmlStrcmp(cur->name, (const xmlChar *)"Username")) {
			auth->supported_options |= OSYNC_PLUGIN_AUTHENTICATION_USERNAME;
			osync_plugin_authentication_set_username(auth, str);
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"Password")) {
			auth->supported_options |= OSYNC_PLUGIN_AUTHENTICATION_PASSWORD;
			osync_plugin_authentication_set_password(auth, str);
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"Reference")) {
			auth->supported_options |= OSYNC_PLUGIN_AUTHENTICATION_REFERENCE;
			osync_plugin_authentication_set_reference(auth, str);
		}

		osync_xml_free(str);
	}

	osync_plugin_config_set_authentication(config, auth);
	osync_plugin_authentication_unref(auth);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool _osync_plugin_config_parse_externalplugin(OSyncPluginConfig *config, xmlNode *cur, OSyncError **error)
{
	OSyncPluginExternalPlugin *externalplugin = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, config, cur, error);

	if (cur == NULL) { // don't set externalplugin if ExternalPlugin tag is empty
		osync_trace(TRACE_EXIT, "%s", __func__);
		return TRUE;	
	}
	
	externalplugin = osync_plugin_externalplugin_new(error);
	if (!externalplugin)
		goto error;

	for (; cur != NULL; cur = cur->next) {
		char *str = NULL;
		if (cur->type != XML_ELEMENT_NODE)
			continue;

		str = (char*)xmlNodeGetContent(cur);
		if (!str)
			continue;

		if (!xmlStrcmp(cur->name, (const xmlChar *)"ExternalCommand")) {
			osync_plugin_externalplugin_set_external_command(externalplugin, str);
		}

		osync_xml_free(str);
	}

	osync_plugin_config_set_externalplugin(config, externalplugin);
	osync_plugin_externalplugin_unref(externalplugin);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool _osync_plugin_config_parse_connection_bluetooth(OSyncPluginConnection *conn, xmlNode *cur, OSyncError **error) {

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, conn, cur, error);

	for (; cur != NULL; cur = cur->next) {
		char *str = NULL;
		if (cur->type != XML_ELEMENT_NODE)
			continue;

		str = (char*)xmlNodeGetContent(cur);
		if (!str)
			continue;

		if (!xmlStrcmp(cur->name, (const xmlChar *)"MAC")) {
			conn->supported_options |= OSYNC_PLUGIN_CONNECTION_BLUETOOTH_ADDRESS;
			osync_plugin_connection_bt_set_addr(conn, str);
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"RFCommChannel")) {
			conn->supported_options |= OSYNC_PLUGIN_CONNECTION_BLUETOOTH_RFCOMM;
			osync_plugin_connection_bt_set_channel(conn, atoi(str));
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"SDPUUID")) {
			conn->supported_options |= OSYNC_PLUGIN_CONNECTION_BLUETOOTH_SDPUUID;
			osync_plugin_connection_bt_set_sdpuuid(conn, str);
		} else {
			//			osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "Unknown configuration field \"%s\"", __NULLSTR(cur->name));
			osync_xml_free(str);
			goto error;
		}

		osync_xml_free(str);
	}


	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
 error:

	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool _osync_plugin_config_parse_connection_usb(OSyncPluginConnection *conn, xmlNode *cur, OSyncError **error) {

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, conn, cur, error);

	for (; cur != NULL; cur = cur->next) {
		char *str = NULL;
		if (cur->type != XML_ELEMENT_NODE)
			continue;

		str = (char*)xmlNodeGetContent(cur);
		if (!str)
			continue;

		if (!xmlStrcmp(cur->name, (const xmlChar *)"VendorID")) {
			conn->supported_options |= OSYNC_PLUGIN_CONNECTION_USB_VENDORID;
			osync_plugin_connection_usb_set_vendorid(conn, str);
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"ProductID")) {
			conn->supported_options |= OSYNC_PLUGIN_CONNECTION_USB_PRODUCTID;
			osync_plugin_connection_usb_set_productid(conn, str);
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"Interface")) {
			conn->supported_options |= OSYNC_PLUGIN_CONNECTION_USB_INTERFACE;
			osync_plugin_connection_usb_set_interface(conn, atoi(str));
		} else {
			osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "Unknown configuration field \"%s\"", cur->name);

			osync_xml_free(str);
			goto error;
		}

		osync_xml_free(str);
	}

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
 error:

	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool _osync_plugin_config_parse_connection_irda(OSyncPluginConnection *conn, xmlNode *cur, OSyncError **error) {

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, conn, cur, error);

	for (; cur != NULL; cur = cur->next) {
		char *str = NULL;
		if (cur->type != XML_ELEMENT_NODE)
			continue;

		str = (char*)xmlNodeGetContent(cur);
		if (!str)
			continue;

		if (!xmlStrcmp(cur->name, (const xmlChar *)"Service")) {
			conn->supported_options |= OSYNC_PLUGIN_CONNECTION_IRDA_SERVICE;
			osync_plugin_connection_irda_set_service(conn, str);
		} else {
			osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "Unknown configuration field \"%s\"", cur->name);
			osync_xml_free(str);
			goto error;
		}

		osync_xml_free(str);
	}

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
 error:

	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool _osync_plugin_config_parse_connection_network(OSyncPluginConnection *conn, xmlNode *cur, OSyncError **error) {

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, conn, cur, error);

	for (; cur != NULL; cur = cur->next) {
		char *str = NULL;
		if (cur->type != XML_ELEMENT_NODE)
			continue;

		str = (char*)xmlNodeGetContent(cur);
		if (!str)
			continue;

		if (!xmlStrcmp(cur->name, (const xmlChar *)"Address")) {
			conn->supported_options |= OSYNC_PLUGIN_CONNECTION_NETWORK_ADDRESS;
			osync_plugin_connection_net_set_address(conn, str);
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"Port")) {
			conn->supported_options |= OSYNC_PLUGIN_CONNECTION_NETWORK_PORT;
			osync_plugin_connection_net_set_port(conn, atoi(str));
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"Protocol")) {
			conn->supported_options |= OSYNC_PLUGIN_CONNECTION_NETWORK_PROTOCOL;
			osync_plugin_connection_net_set_protocol(conn, str);
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"DNSSD")) {
			conn->supported_options |= OSYNC_PLUGIN_CONNECTION_NETWORK_DNSSD;
			osync_plugin_connection_net_set_dnssd(conn, str);
		} else {
			osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "Unknown configuration field \"%s\"", cur->name);
			osync_xml_free(str);
			goto error;
		}

		osync_xml_free(str);
	}

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
 error:

	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool _osync_plugin_config_parse_connection_serial(OSyncPluginConnection *conn, xmlNode *cur, OSyncError **error) {

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, conn, cur, error);

	for (; cur != NULL; cur = cur->next) {
		char *str = NULL;
		if (cur->type != XML_ELEMENT_NODE)
			continue;

		str = (char*)xmlNodeGetContent(cur);
		if (!str)
			continue;

		if (!xmlStrcmp(cur->name, (const xmlChar *)"Speed")) {
			conn->supported_options |= OSYNC_PLUGIN_CONNECTION_SERIAL_SPEED;
			osync_plugin_connection_serial_set_speed(conn, atoi(str));
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"DeviceNode")) {
			conn->supported_options |= OSYNC_PLUGIN_CONNECTION_SERIAL_DEVICENODE;
			osync_plugin_connection_serial_set_devicenode(conn, str);
		} else {
			osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "Unknown configuration field \"%s\"", cur->name);
			osync_xml_free(str);
			goto error;
		}

		osync_xml_free(str);
	}

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
 error:

	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool _osync_plugin_config_parse_active_connection(OSyncPluginConnection *conn, xmlNode *cur, OSyncError **error)
{
	/* If ActiveConnection is empty, this could be the very first
		 configuration. Just accept without noise. */
	if (!cur)
		return TRUE;

	if (!xmlStrcmp(cur->content, (const xmlChar *)"Bluetooth")) {
		osync_plugin_connection_set_type(conn, OSYNC_PLUGIN_CONNECTION_BLUETOOTH);
	} else if (!xmlStrcmp(cur->content, (const xmlChar *)"USB")) {
		osync_plugin_connection_set_type(conn, OSYNC_PLUGIN_CONNECTION_USB);
	} else if (!xmlStrcmp(cur->content, (const xmlChar *)"IrDA")) {
		osync_plugin_connection_set_type(conn, OSYNC_PLUGIN_CONNECTION_IRDA);
	} else if (!xmlStrcmp(cur->content, (const xmlChar *)"Network")) {
		osync_plugin_connection_set_type(conn, OSYNC_PLUGIN_CONNECTION_NETWORK);
	} else if (!xmlStrcmp(cur->content, (const xmlChar *)"Serial")) {
		osync_plugin_connection_set_type(conn, OSYNC_PLUGIN_CONNECTION_SERIAL);
	} else {
		osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "Unknown configuration field \"%s\"", cur->content);
		return FALSE;
	}
	
	return TRUE;
}


static osync_bool _osync_plugin_config_parse_connection(OSyncPluginConfig *config, xmlNode *cur, OSyncError **error)
{
	osync_bool ret = TRUE;
	OSyncPluginConnection *conn = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, config, cur, error);

	if (!(conn = osync_plugin_connection_new(error)))
		goto error;

	for (; cur != NULL; cur = cur->next) {

		if (cur->type != XML_ELEMENT_NODE)
			continue;
		if (!xmlStrcmp(cur->name, (const xmlChar *)"ActiveConnection")) {
			ret = _osync_plugin_config_parse_active_connection(conn, cur->xmlChildrenNode, error);
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"Bluetooth")) {
			conn->supported |= OSYNC_PLUGIN_CONNECTION_BLUETOOTH;
			ret = _osync_plugin_config_parse_connection_bluetooth(conn, cur->xmlChildrenNode, error);
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"USB")) {
			conn->supported |= OSYNC_PLUGIN_CONNECTION_USB;
			ret = _osync_plugin_config_parse_connection_usb(conn, cur->xmlChildrenNode, error);
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"IrDA")) {
			conn->supported |= OSYNC_PLUGIN_CONNECTION_IRDA;
			ret = _osync_plugin_config_parse_connection_irda(conn, cur->xmlChildrenNode, error);
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"Network")) {
			conn->supported |= OSYNC_PLUGIN_CONNECTION_NETWORK;
			ret = _osync_plugin_config_parse_connection_network(conn, cur->xmlChildrenNode, error);
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"Serial")) {
			conn->supported |= OSYNC_PLUGIN_CONNECTION_SERIAL;
			ret = _osync_plugin_config_parse_connection_serial(conn, cur->xmlChildrenNode, error);
		} else {
			osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "Unknown configuration field \"%s\"", cur->name);
			goto error;
		}
	}

	if (!ret)
		goto error_and_free;

	osync_plugin_config_set_connection(config, conn);
	osync_plugin_connection_unref(conn);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_and_free:
	osync_plugin_connection_unref(conn);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool _osync_plugin_config_parse_localization(OSyncPluginConfig *config, xmlNode *cur, OSyncError **error)
{
	OSyncPluginLocalization *local = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, config, cur, error);

	local = osync_plugin_localization_new(error);
	if (!local)
		goto error;

	for (; cur != NULL; cur = cur->next) {
		char *str = NULL;
		if (cur->type != XML_ELEMENT_NODE)
			continue;

		str = (char*)xmlNodeGetContent(cur);
		if (!str)
			continue;

		if (!xmlStrcmp(cur->name, (const xmlChar *)"Encoding")) {
			local->supported_options |= OSYNC_PLUGIN_LOCALIZATION_ENCODING;
			osync_plugin_localization_set_encoding(local, str);
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"Timezone")) {
			local->supported_options |= OSYNC_PLUGIN_LOCALIZATION_TIMEZONE;
			osync_plugin_localization_set_timezone(local, str);
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"Language")) {
			local->supported_options |= OSYNC_PLUGIN_LOCALIZATION_LANGUAGE;
			osync_plugin_localization_set_language(local, str);
		}

		osync_xml_free(str);
	}

	osync_plugin_config_set_localization(config, local);
	osync_plugin_localization_unref(local);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool _osync_plugin_config_parse_resource_format(OSyncPluginResource *res, xmlNode *cur, OSyncError **error)
{
	OSyncObjFormatSink *format_sink = NULL; 
	const char *objformat = NULL, *config = NULL;

	osync_assert(res);
	osync_assert(cur);

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, res, cur, error);

	for (; cur != NULL; cur = cur->next) {
		char *str = NULL;
		if (cur->type != XML_ELEMENT_NODE)
			continue;

		str = (char*)xmlNodeGetContent(cur);
		if (!str)
			continue;

		if (!xmlStrcmp(cur->name, (const xmlChar *)"Name"))
			objformat = str;
		else if (!xmlStrcmp(cur->name, (const xmlChar *)"Config"))
			config = str;
		else
			osync_xml_free(str);
	}

	osync_assert(objformat);

	format_sink = osync_objformat_sink_new(objformat, error);
	osync_xml_free((xmlChar *) objformat);

	if (!format_sink)
		goto error_free_config;

	if (config) {
		osync_objformat_sink_set_config(format_sink, config);
		osync_xml_free((xmlChar *) config);
	}

	osync_plugin_resource_add_objformat_sink(res, format_sink);
	osync_objformat_sink_unref(format_sink);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_free_config:
	osync_xml_free((xmlChar *) config);
	/*error:*/
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool _osync_plugin_config_parse_resource_formats(OSyncPluginResource *res, xmlNode *cur, OSyncError **error)
{
	const char *preferred_format = NULL;

	osync_assert(res);
	osync_assert(cur);

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, res, cur, error);

	for (; cur != NULL; cur = cur->next) {
		char *str = NULL;
		if (cur->type != XML_ELEMENT_NODE)
			continue;

		str = (char*)xmlNodeGetContent(cur);
		if (!str)
			continue;

		if (!xmlStrcmp(cur->name, (const xmlChar *)"Preferred"))
			preferred_format = str;
		else if (!xmlStrcmp(cur->name, (const xmlChar *)"Format")) {
			if (!_osync_plugin_config_parse_resource_format(res, cur->xmlChildrenNode, error))
				goto error;
			osync_xml_free(str);
		} else
			osync_xml_free(str);

	}

	if (preferred_format) {
		osync_plugin_resource_set_preferred_format(res, preferred_format);
		osync_xml_free((xmlChar *) preferred_format);
	}

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static OSyncPluginResource *_osync_plugin_config_parse_resource(OSyncPluginConfig *config, xmlNode *cur, OSyncError **error)
{
	char *str = NULL;
	OSyncPluginResource *res = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, config, cur, error);

	res = osync_plugin_resource_new(error);
	if (!res)
		goto error;

	for (; cur != NULL; cur = cur->next) {

		if (cur->type != XML_ELEMENT_NODE)
			continue;

		str = (char*)xmlNodeGetContent(cur);
		if (!str)
			continue;

		if (!xmlStrcmp(cur->name, (const xmlChar *)"Enabled")) {
			osync_plugin_resource_enable(res, atoi(str));
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"Formats")) {
			if (!_osync_plugin_config_parse_resource_formats(res, cur->xmlChildrenNode, error))
				goto error_free_str;
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"Name")) {
			res->supported_options |= OSYNC_PLUGIN_RESOURCE_NAME;
			osync_plugin_resource_set_name(res, str);
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"MIME")) {
			osync_plugin_resource_set_mime(res, str);
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"ObjType")) {
			osync_plugin_resource_set_objtype(res, str);
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"Path")) {
			res->supported_options |= OSYNC_PLUGIN_RESOURCE_PATH;
			osync_plugin_resource_set_path(res, str);
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"Url")) {
			res->supported_options |= OSYNC_PLUGIN_RESOURCE_URL;
			osync_plugin_resource_set_url(res, str);
		}

		osync_xml_free(str);
	}

	osync_trace(TRACE_EXIT, "%s", __func__);
	return res;

 error_free_str:
	osync_xml_free(str);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

static osync_bool _osync_plugin_config_parse_resources(OSyncPluginConfig *config, xmlNode *cur, OSyncError **error)
{
	OSyncPluginResource *res; 
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, config, cur, error);

	for (; cur != NULL; cur = cur->next) {

		if (cur->type != XML_ELEMENT_NODE)
			continue;

		if (!xmlStrcmp(cur->name, (const xmlChar *)"Resource")) {
			if (!(res = _osync_plugin_config_parse_resource(config, cur->xmlChildrenNode, error)))
				goto error;

			config->resources = osync_list_prepend(config->resources, res);
		}
	}

	config->resources = osync_list_reverse(config->resources);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool _osync_plugin_config_parse(OSyncPluginConfig *config, xmlNode *cur, OSyncError **error)
{
	osync_bool ret = TRUE;
	osync_assert(config);
	osync_assert(cur);

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, config, cur, error);

	config->supported = 0;

	for (; cur != NULL; cur = cur->next) {

		if (cur->type != XML_ELEMENT_NODE)
			continue;

		if (!xmlStrcmp(cur->name, (const xmlChar *)"AdvancedOptions")) {
			config->supported |= OPENSYNC_PLUGIN_CONFIG_ADVANCEDOPTION;
			ret = _osync_plugin_config_parse_advancedoptions(config, cur->xmlChildrenNode, error);
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"Authentication")) {
			config->supported |= OPENSYNC_PLUGIN_CONFIG_AUTHENTICATION;
			ret = _osync_plugin_config_parse_authentication(config, cur->xmlChildrenNode, error);
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"Connection")) {
			config->supported |= OPENSYNC_PLUGIN_CONFIG_CONNECTION;
			ret = _osync_plugin_config_parse_connection(config, cur->xmlChildrenNode, error);
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"Localization")) {
			config->supported |= OPENSYNC_PLUGIN_CONFIG_LOCALIZATION;
			ret = _osync_plugin_config_parse_localization(config, cur->xmlChildrenNode, error);
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"Resources")) {
			config->supported |= OPENSYNC_PLUGIN_CONFIG_RESOURCES;
			ret = _osync_plugin_config_parse_resources(config, cur->xmlChildrenNode, error);
		} else if (!xmlStrcmp(cur->name, (const xmlChar *)"ExternalPlugin")) {
			config->supported |= OPENSYNC_PLUGIN_CONFIG_EXTERNALPLUGIN;
			ret = _osync_plugin_config_parse_externalplugin(config, cur->xmlChildrenNode, error);
		} else {
			osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "Unknown configuration field \"%s\"", cur->name);
			goto error;
		}

		if (!ret)
			goto error;
	}

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
 error:

	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_plugin_config_file_load(OSyncPluginConfig *config, const char *path, OSyncError **error)
{
	xmlDocPtr doc = NULL;
	xmlNodePtr cur = NULL;
	char *schemafile = NULL;
	const char *schemapath = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %s, %p, %p)", __func__, config, __NULLSTR(path), path, error);

	schemapath = config->schemadir ? config->schemadir : OPENSYNC_SCHEMASDIR;

	if (!osync_xml_open_file(&doc, &cur, path, "config", error))
		goto error;

	/* Validate plugin configuration file */
	schemafile = osync_strdup_printf("%s%c%s", schemapath, G_DIR_SEPARATOR, OSYNC_PLUGIN_CONFING_SCHEMA);
	if (!osync_xml_validate_document(doc, schemafile)) {
		osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "Plugin configuration file is not valid! %s", schemafile);
		osync_free(schemafile);
		goto error;
	}
	osync_free(schemafile);

	if (cur && !_osync_plugin_config_parse(config, cur, error))
		goto error;

	osync_xml_free_doc(doc);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
 error:

	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

void osync_plugin_config_set_schemadir(OSyncPluginConfig *config, const char *schemadir)
{
	osync_return_if_fail(config);
	
	if (config->schemadir) {
		osync_free(config->schemadir);
		config->schemadir = NULL;
	}

	if (schemadir) {
		config->schemadir = osync_strdup(schemadir);
	}
}

static osync_bool _osync_plugin_config_assemble_authentication(xmlNode *cur, OSyncPluginAuthentication *auth, OSyncError **error)
{
	const char *username, *password, *ref;
	xmlNode *node = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, cur, auth, error);

	node = xmlNewChild(cur, NULL, (xmlChar*)"Authentication", NULL);
	if (!node) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "No memory left to assemble configuration.");
		goto error;
	}

	if ((username = osync_plugin_authentication_get_username(auth)))
		xmlNewChild(node, NULL, (xmlChar*)"Username", (xmlChar*)username);

	if ((password = osync_plugin_authentication_get_password(auth)))
		xmlNewChild(node, NULL, (xmlChar*)"Password", (xmlChar*)password);

	if ((ref = osync_plugin_authentication_get_reference(auth)))
		xmlNewChild(node, NULL, (xmlChar*)"Reference", (xmlChar*)ref);


	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool _osync_plugin_config_assemble_externalplugin(xmlNode *cur, OSyncPluginExternalPlugin *externalplugin, OSyncError **error)
{
	const char *external_command;
	xmlNode *node = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, cur, externalplugin, error);

	node = xmlNewChild(cur, NULL, (xmlChar*)"ExternalPlugin", NULL);
	if (!node) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "No memory left to assemble configuration.");
		goto error;
	}

	if ((external_command = osync_plugin_externalplugin_get_external_command(externalplugin)))
		xmlNewChild(node, NULL, (xmlChar*)"ExternalCommand", (xmlChar*)external_command);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool _osync_plugin_config_assemble_connection(xmlNode *cur, OSyncPluginConnection *conn, OSyncError **error)
{
	char *str;
	const char *mac, *sdpuuid, *address, *protocol, *dnssd, *devicenode, *service, *vendorid, *productid;
	unsigned int rfcomm_channel, interf, port, speed;
	xmlNode *typenode, *node = NULL;
	OSyncPluginConnectionType conn_type;
	const char *conn_str = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, cur, conn, error);

	node = xmlNewChild(cur, NULL, (xmlChar*)"Connection", NULL);
	if (!node)
		goto error_nomemory;

	/* Active Connection */
	conn_type = osync_plugin_connection_get_type(conn);
	conn_str = osync_plugin_connection_get_type_string(conn_type);
	typenode = xmlNewChild(node, NULL, BAD_CAST "ActiveConnection", BAD_CAST conn_str);
	if (!typenode)
		goto error_nomemory;

	/* Bluetooth */
	if (osync_plugin_connection_is_supported(conn, OSYNC_PLUGIN_CONNECTION_BLUETOOTH)) {
		typenode = xmlNewChild(node, NULL, (xmlChar*)"Bluetooth", NULL);
		if (!typenode)
			goto error_nomemory;

		mac = osync_plugin_connection_bt_get_addr(conn);
		if (mac)
			xmlNewChild(typenode, NULL, (xmlChar*)"MAC", (xmlChar*)mac);

		rfcomm_channel = osync_plugin_connection_bt_get_channel(conn);
		if (rfcomm_channel) {
			str = osync_strdup_printf("%u", rfcomm_channel);
			xmlNewChild(typenode, NULL, (xmlChar*)"RFCommChannel", (xmlChar*)str);
			osync_free(str);
		}

		sdpuuid = osync_plugin_connection_bt_get_sdpuuid(conn);
		if (sdpuuid)
			xmlNewChild(typenode, NULL, (xmlChar*)"SDPUUID", (xmlChar*)sdpuuid);
	}

	/* USB */
	if (osync_plugin_connection_is_supported(conn, OSYNC_PLUGIN_CONNECTION_USB)) {
		typenode = xmlNewChild(node, NULL, (xmlChar*)"USB", NULL);
		if (!typenode)
			goto error_nomemory;

		vendorid = osync_plugin_connection_usb_get_vendorid(conn);
		if (vendorid) {
			str = osync_strdup(vendorid);
			xmlNewChild(typenode, NULL, (xmlChar*)"VendorID", (xmlChar*)str);
			osync_free(str);
		}

		productid = osync_plugin_connection_usb_get_productid(conn);
		if (productid) {
			str = osync_strdup(productid);
			xmlNewChild(typenode, NULL, (xmlChar*)"ProductID", (xmlChar*)str);
			osync_free(str);
		}

		interf = osync_plugin_connection_usb_get_interface(conn);
		if (interf) {
			str = osync_strdup_printf("%u", interf);
			xmlNewChild(typenode, NULL, (xmlChar*)"Interface", (xmlChar*)str);
			osync_free(str);
		}
	}

	/* Network */
	if (osync_plugin_connection_is_supported(conn, OSYNC_PLUGIN_CONNECTION_NETWORK)) {
		typenode = xmlNewChild(node, NULL, (xmlChar*)"Network", NULL);
		if (!typenode)
			goto error_nomemory;

		address = osync_plugin_connection_net_get_address(conn);
		if (address)
			xmlNewChild(typenode, NULL, (xmlChar*)"Address", (xmlChar*)address);

		port = osync_plugin_connection_net_get_port(conn);
		if (port) {
			str = osync_strdup_printf("%u", port);
			xmlNewChild(typenode, NULL, (xmlChar*)"Port", (xmlChar*)str);
			osync_free(str);
		}

		protocol = osync_plugin_connection_net_get_protocol(conn);
		if (protocol)
			xmlNewChild(typenode, NULL, (xmlChar*)"Protocol", (xmlChar*)protocol);

		dnssd = osync_plugin_connection_net_get_dnssd(conn);
		if (dnssd)
			xmlNewChild(typenode, NULL, (xmlChar*)"DNSSD", (xmlChar*)dnssd);
	}

	/* Serial */
	if (osync_plugin_connection_is_supported(conn, OSYNC_PLUGIN_CONNECTION_SERIAL)) {
		typenode = xmlNewChild(node, NULL, (xmlChar*)"Serial", NULL);
		if (!typenode)
			goto error_nomemory;

		speed = osync_plugin_connection_serial_get_speed(conn);
		if (speed) {
			str = osync_strdup_printf("%u", speed);
			xmlNewChild(typenode, NULL, (xmlChar*)"Speed", (xmlChar*)str);
			osync_free(str);
		}

		devicenode = osync_plugin_connection_serial_get_devicenode(conn);
		if (devicenode)
			xmlNewChild(typenode, NULL, (xmlChar*)"DeviceNode", (xmlChar*)devicenode);
	}


	/* IrDA */
	if (osync_plugin_connection_is_supported(conn, OSYNC_PLUGIN_CONNECTION_IRDA)) {
		typenode = xmlNewChild(node, NULL, (xmlChar*)"IrDA", NULL);
		if (!typenode)
			goto error_nomemory;

		service = osync_plugin_connection_irda_get_service(conn);
		if (service)
			xmlNewChild(typenode, NULL, (xmlChar*)"Service", (xmlChar*)service);
	}

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_nomemory:	
	osync_error_set(error, OSYNC_ERROR_GENERIC, "No memory left to assemble configuration.");
	/* error: */
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;

}

static osync_bool _osync_plugin_config_assemble_localization(xmlNode *cur, OSyncPluginLocalization *local, OSyncError **error)
{
	const char *encoding, *tz, *language;
	xmlNode *node = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, cur, local, error);
	
	node = xmlNewChild(cur, NULL, (xmlChar*)"Localization", NULL);
	if (!node) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "No memory left to assemble configuration.");
		goto error;
	}

	if ((encoding = osync_plugin_localization_get_encoding(local)))
		xmlNewChild(node, NULL, (xmlChar*)"Encoding", (xmlChar*)encoding);

	if ((tz = osync_plugin_localization_get_timezone(local)))
		xmlNewChild(node, NULL, (xmlChar*)"Timezone", (xmlChar*)tz);

	if ((language = osync_plugin_localization_get_language(local)))
		xmlNewChild(node, NULL, (xmlChar*)"Language", (xmlChar*)language);


	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;

}

static osync_bool _osync_plugin_config_assemble_resource_format(xmlNode *cur, OSyncObjFormatSink *format_sink, OSyncError **error)
{
	const char *name, *config;
	xmlNode *node = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, cur, format_sink, error);

	node = xmlNewChild(cur, NULL, (xmlChar*)"Format", NULL);
	if (!node) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "No memory left to assemble configuration.");
		goto error;
	}

	if ((config = osync_objformat_sink_get_config(format_sink)))
		xmlNewChild(node, NULL, (xmlChar*)"Config", (xmlChar*)config);

	if ((name = osync_objformat_sink_get_objformat(format_sink)))
		xmlNewChild(node, NULL, (xmlChar*)"Name", (xmlChar*)name);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool _osync_plugin_config_assemble_resource(xmlNode *cur, OSyncPluginResource *res, OSyncError **error)
{
	OSyncList *o = NULL, *objformats = NULL;
	const char *preferred_format, *name, *mime, *objtype, *path, *url;
	xmlNode *next, *node = NULL;
	osync_bool res_enabled;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, cur, res, error);

	node = xmlNewChild(cur, NULL, (xmlChar*)"Resource", NULL);
	if (!node) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "No memory left to assemble configuration.");
		goto error;
	}

	res_enabled = osync_plugin_resource_is_enabled(res);
	xmlNewChild(node, NULL, (xmlChar*)"Enabled", res_enabled ? (xmlChar*) "1" : (xmlChar*) "0");

	next = xmlNewChild(node, NULL, (xmlChar*)"Formats", NULL);
	objformats = osync_plugin_resource_get_objformat_sinks(res);
	for (o = objformats; o; o = o->next) {
		OSyncObjFormatSink *format_sink = o->data;
		if (!_osync_plugin_config_assemble_resource_format(next, format_sink, error))
			goto error;
	}
	osync_list_free(objformats);
	
	if ((preferred_format = osync_plugin_resource_get_preferred_format(res)))
		xmlNewChild(next, NULL, (xmlChar*)"Preferred", (xmlChar*)preferred_format);

	if ((name = osync_plugin_resource_get_name(res)))
		xmlNewChild(node, NULL, (xmlChar*)"Name", (xmlChar*)name);

	if ((mime = osync_plugin_resource_get_mime(res)))
		xmlNewChild(node, NULL, (xmlChar*)"MIME", (xmlChar*)mime);

	objtype = osync_plugin_resource_get_objtype(res);
	osync_assert(objtype); /* ObjType for Resource MUST be set! */
	xmlNewChild(node, NULL, (xmlChar*)"ObjType", (xmlChar*) objtype);

	if ((path = osync_plugin_resource_get_path(res)))
		xmlNewChild(node, NULL, (xmlChar*)"Path", (xmlChar*)path);

	if ((url = osync_plugin_resource_get_url(res)))
		xmlNewChild(node, NULL, (xmlChar*)"Url", (xmlChar*)url);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool _osync_plugin_config_assemble_resources(xmlNode *cur, OSyncList *resources, OSyncError **error)
{
	OSyncList *res;
	xmlNode *node = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, cur, resources, error);

	node = xmlNewChild(cur, NULL, (xmlChar*)"Resources", NULL);
	if (!node) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "No memory left to assemble configuration.");
		goto error;
	}

	for (res = resources; res; res = res->next)
		if (!_osync_plugin_config_assemble_resource(node, res->data, error))
			goto error;


	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool _osync_plugin_config_assemble_advancedoption_param(xmlNode *cur, OSyncPluginAdvancedOptionParameter *param, OSyncError **error)
{
	xmlNode *node = NULL;
	OSyncList *v = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, cur, param, error);

	node = xmlNewChild(cur, NULL, (xmlChar*)"Parameter", NULL);
	if (!node) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "No memory left to assemble configuration.");
		goto error;
	}

	/* DisplayName */
	if (osync_plugin_advancedoption_param_get_displayname(param))
		xmlNewChild(node, NULL, BAD_CAST "DisplayName", 
		            BAD_CAST osync_plugin_advancedoption_param_get_displayname(param));

	/* Name */
	if (!osync_plugin_advancedoption_param_get_name(param)) {
		osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "Name for advanced option not set.");
		goto error;
	}

	xmlNewChild(node, NULL, BAD_CAST "Name", BAD_CAST osync_plugin_advancedoption_param_get_name(param));

	/* Type */
	if (!osync_plugin_advancedoption_param_get_type(param)) {
		osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "Type for advanced option not set.");
		goto error;
	}

	xmlNewChild(node, NULL, BAD_CAST "Type", BAD_CAST osync_plugin_advancedoption_param_get_type_string(param));

	/* ValEnum */
	OSyncList *valenums = osync_plugin_advancedoption_param_get_valenums(param);
	for (v = valenums; v; v = v->next) {
		char *valenum = v->data;
		xmlNewChild(node, NULL, BAD_CAST "ValEnum", BAD_CAST valenum);
	}
	osync_list_free(valenums);

	/* Value */
	if (!osync_plugin_advancedoption_param_get_value(param)) {
		osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "Value for advanced option not set.");
		goto error;
	}

	xmlNewChild(node, NULL, BAD_CAST "Value", BAD_CAST osync_plugin_advancedoption_param_get_value(param));

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;

}

static osync_bool _osync_plugin_config_assemble_advancedoption(xmlNode *cur, OSyncPluginAdvancedOption *option, OSyncError **error)
{
	xmlNode *node = NULL;
	OSyncList *p;
	OSyncList *v;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, cur, option, error);

	node = xmlNewChild(cur, NULL, (xmlChar*)"AdvancedOption", NULL);
	if (!node) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "No memory left to assemble configuration.");
		goto error;
	}

	/* DisplayName */
	if (osync_plugin_advancedoption_get_displayname(option))
		xmlNewChild(node, NULL, BAD_CAST "DisplayName", 
		            BAD_CAST osync_plugin_advancedoption_get_displayname(option));

	/* MaxOccurs */
	if (osync_plugin_advancedoption_get_maxoccurs(option)) {
		char *str = osync_strdup_printf("%u", osync_plugin_advancedoption_get_maxoccurs(option));
		xmlNewChild(node, NULL, BAD_CAST "MaxOccurs", BAD_CAST str);
		osync_free(str);
	}

	/* Max */
	if (osync_plugin_advancedoption_get_max(option)) {
		char *str = osync_strdup_printf("%u", osync_plugin_advancedoption_get_max(option));
		xmlNewChild(node, NULL, BAD_CAST "Max", BAD_CAST str);
		osync_free(str);
	}

	/* Min */
	if (osync_plugin_advancedoption_get_min(option)) {
		char *str = osync_strdup_printf("%u", osync_plugin_advancedoption_get_min(option));
		xmlNewChild(node, NULL, BAD_CAST "Min", BAD_CAST str);
		osync_free(str);
	}

	/* Name */
	if (!osync_plugin_advancedoption_get_name(option)) {
		osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "Name for advanced option not set.");
		goto error;
	}

	xmlNewChild(node, NULL, BAD_CAST "Name", BAD_CAST osync_plugin_advancedoption_get_name(option));

	/* Parameters */
	OSyncList *parameters = osync_plugin_advancedoption_get_parameters(option);
	for (p = parameters; p; p = p->next) {
		OSyncPluginAdvancedOptionParameter *param = p->data;
		if (!_osync_plugin_config_assemble_advancedoption_param(node, param, error)) {
			osync_list_free(parameters);
			goto error;
		}
	}
	osync_list_free(parameters);
	
	/* Type */
	if (!osync_plugin_advancedoption_get_type(option)) {
		osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "Type for advanced option not set.");
		goto error;
	}

	xmlNewChild(node, NULL, BAD_CAST "Type", BAD_CAST osync_plugin_advancedoption_get_type_string(option));

	/* ValEnum */
	OSyncList *valenums = osync_plugin_advancedoption_get_valenums(option);
	for (v = valenums; v; v = v->next) {
		char *valenum = v->data;
		xmlNewChild(node, NULL, BAD_CAST "ValEnum", BAD_CAST valenum);
	}
	osync_list_free(valenums);
	
	/* Value */
	if (!osync_plugin_advancedoption_get_value(option)) {
		osync_error_set(error, OSYNC_ERROR_MISCONFIGURATION, "Value for advanced option not set.");
		goto error;
	}

	xmlNewChild(node, NULL, BAD_CAST "Value", BAD_CAST osync_plugin_advancedoption_get_value(option));


	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

static osync_bool _osync_plugin_config_assemble_advancedoptions(xmlNode *cur, OSyncList *options, OSyncError **error)
{
	OSyncList *o;
	xmlNode *node = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, cur, options, error);

	node = xmlNewChild(cur, NULL, (xmlChar*)"AdvancedOptions", NULL);
	if (!node) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "No memory left to assemble configuration.");
		goto error;
	}

	for (o = options; o; o = o->next)
		if (!_osync_plugin_config_assemble_advancedoption(node, o->data, error))
			goto error;


	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

const char *osync_plugin_get_default_configdir(void)
{
	return OPENSYNC_CONFIGDIR G_DIR_SEPARATOR_S;
}

OSyncPluginConfig *osync_plugin_config_new(OSyncError **error)
{
	OSyncPluginConfig *config = osync_try_malloc0(sizeof(OSyncPluginConfig), error);
	if (!config)
		return NULL;

	config->ref_count = 1;

	return config;
}

void osync_plugin_config_unref(OSyncPluginConfig *config)
{
	osync_assert(config);

	if (g_atomic_int_dec_and_test(&(config->ref_count))) {
		if (config->connection)
			osync_plugin_connection_unref(config->connection);

		if (config->localization)
			osync_plugin_localization_unref(config->localization);
			
		if (config->authentication)
			osync_plugin_authentication_unref(config->authentication);

		while (config->advancedoptions) {
			OSyncPluginAdvancedOption *opt = config->advancedoptions->data;
			osync_plugin_advancedoption_unref(opt);
			config->advancedoptions = osync_list_remove(config->advancedoptions, opt);
		}

		while (config->resources) {
			OSyncPluginResource *res = config->resources->data;
			osync_plugin_resource_unref(res);
			config->resources = osync_list_remove(config->resources, res);
		}

		if (config->externalplugin)
			osync_plugin_externalplugin_unref(config->externalplugin);

		if (config->schemadir)
			osync_free(config->schemadir);
			
		osync_free(config);
	}
}

OSyncPluginConfig *osync_plugin_config_ref(OSyncPluginConfig *config)
{
	osync_assert(config);
	
	g_atomic_int_inc(&(config->ref_count));

	return config;
}

osync_bool osync_plugin_config_file_save(OSyncPluginConfig *config, const char *path, OSyncError **error)
{
	xmlDocPtr doc = NULL;
	OSyncPluginConnection *conn;
	OSyncPluginAuthentication *auth;
	OSyncPluginLocalization *local;
	OSyncPluginExternalPlugin *externalplugin;
	OSyncList *resources;
	OSyncList *options;
	char *version_str = NULL;

	osync_assert(config);
	osync_assert(path);

	osync_trace(TRACE_ENTRY, "%s(%p, %s)", __func__, config, __NULLSTR(path));

	doc = xmlNewDoc((xmlChar*)"1.0");
	if (!doc) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Couldn't allocate memory to assemble configuration file."); 
		goto error;
	}
	doc->children = xmlNewDocNode(doc, NULL, (xmlChar*)"config", NULL);
	if (!doc->children) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Couldn't allocate memory to assemble root node for configuration file."); 
		goto error_and_free;
	}

	/* Set version for plugin configuration */
	version_str = osync_strdup_printf("%u.%u", OSYNC_PLUGIN_MAJOR_VERSION, OSYNC_PLUGIN_MINOR_VERSION);
	xmlSetProp(doc->children, (const xmlChar*)"version", (const xmlChar *)version_str);
	osync_free(version_str);

	/** Assemble... */
	/* Advanced Options */
	options = config->advancedoptions;
	if (options)
		if (!_osync_plugin_config_assemble_advancedoptions(doc->children, options, error))
			goto error_and_free;

	/* Authentication */
	if ((auth = osync_plugin_config_get_authentication(config)))
		if (!_osync_plugin_config_assemble_authentication(doc->children, auth, error))
			goto error_and_free;

	/* Connection */
	if ((conn = osync_plugin_config_get_connection(config)))
		if (!_osync_plugin_config_assemble_connection(doc->children, conn, error))
			goto error_and_free;

	/* Localization */
	if ((local = osync_plugin_config_get_localization(config)))
		if (!_osync_plugin_config_assemble_localization(doc->children, local, error))
			goto error_and_free;

	/* Resources */
	if ((resources = osync_plugin_config_get_resources(config)))
		if (!_osync_plugin_config_assemble_resources(doc->children, resources, error))
			goto error_and_free;

	/* ExternalPlugin */
	if ((externalplugin = osync_plugin_config_get_externalplugin(config)))
		if (!_osync_plugin_config_assemble_externalplugin(doc->children, externalplugin, error))
			goto error_and_free;

	xmlSaveFormatFile(path, doc, 1);

	osync_xml_free_doc(doc);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_and_free:	
	osync_xml_free_doc(doc);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_plugin_config_is_supported(OSyncPluginConfig *config, OSyncPluginConfigSupportedFlag flag)
{
	osync_assert(config);

	if (config->supported & flag)
		return TRUE;

	return FALSE;
}

void osync_plugin_config_set_supported(OSyncPluginConfig *config, OSyncPluginConfigSupportedFlags flags)
{
	osync_assert(config);
	config->supported = flags;
}

/* Advanced Options */
OSyncList *osync_plugin_config_get_advancedoptions(OSyncPluginConfig *config)
{
	osync_assert(config);
	return osync_list_copy(config->advancedoptions);
}

const char *osync_plugin_config_get_advancedoption_value_by_name(OSyncPluginConfig *config, const char *name)
{
	OSyncList *opt;

	osync_assert(config);
	osync_assert(name);

	for (opt = config->advancedoptions; opt; opt = opt->next) {
		OSyncPluginAdvancedOption *option = opt->data; 
		const char *opt_name = osync_plugin_advancedoption_get_name(option);
		osync_assert(opt_name);

		if (!strcmp(opt_name, name))
			return osync_plugin_advancedoption_get_value(option);
	}

	return NULL;
}

void osync_plugin_config_add_advancedoption(OSyncPluginConfig *config, OSyncPluginAdvancedOption *option)
{
	osync_assert(config);
	osync_assert(option);

	if (osync_list_find(config->advancedoptions, option))
		return;

	osync_plugin_advancedoption_ref(option);
	config->advancedoptions = osync_list_prepend(config->advancedoptions, option);
}

void osync_plugin_config_remove_advancedoption(OSyncPluginConfig *config, OSyncPluginAdvancedOption *option)
{
	osync_assert(config);
	osync_assert(option);

	config->advancedoptions = osync_list_remove(config->advancedoptions, option);
	osync_plugin_advancedoption_unref(option);
}

/* Authentication */
OSyncPluginAuthentication *osync_plugin_config_get_authentication(OSyncPluginConfig *config)
{
	osync_assert(config);
	return config->authentication;
}

void osync_plugin_config_set_authentication(OSyncPluginConfig *config, OSyncPluginAuthentication *authentication)
{
	osync_assert(config);
	
	if (config->authentication) {
		osync_plugin_authentication_unref(config->authentication);
		config->authentication = NULL;
	}
	
	if (authentication) {
		config->authentication = osync_plugin_authentication_ref(authentication);
	}
}

/* Localization */
OSyncPluginLocalization *osync_plugin_config_get_localization(OSyncPluginConfig *config)
{
	osync_assert(config);
	return config->localization;
}

void osync_plugin_config_set_localization(OSyncPluginConfig *config, OSyncPluginLocalization *localization)
{
	osync_assert(config);
	
	if (config->localization) {
		osync_plugin_localization_unref(config->localization);
		config->localization = NULL;
	}
	if (localization) {
		config->localization = osync_plugin_localization_ref(localization);
	}
}

/* Resources */
OSyncList *osync_plugin_config_get_resources(OSyncPluginConfig *config)
{
	osync_assert(config);
	return config->resources;

}

void osync_plugin_config_add_resource(OSyncPluginConfig *config, OSyncPluginResource *resource)
{
	osync_assert(config);
	osync_assert(resource);

	if (osync_list_find(config->resources, resource))
		return;

	osync_plugin_resource_ref(resource);
	config->resources = osync_list_append(config->resources, resource);
}

void osync_plugin_config_remove_resource(OSyncPluginConfig *config, OSyncPluginResource *resource)
{
	osync_assert(config);
	osync_assert(resource);

	config->resources = osync_list_remove(config->resources, resource);
	osync_plugin_resource_unref(resource);
}

void osync_plugin_config_flush_resources(OSyncPluginConfig *config)
{
	osync_assert(config);
	while (config->resources) {
		OSyncPluginResource *resource = config->resources->data;
		config->resources = osync_list_remove(config->resources, resource);
		osync_plugin_resource_unref(resource);
	}

}


OSyncPluginResource *osync_plugin_config_find_active_resource(OSyncPluginConfig *config, const char *objtype)
{
	OSyncList *r;

	osync_assert(config);
	osync_assert(objtype);

	for (r = config->resources; r; r = r->next) {
		OSyncPluginResource *res = r->data;
		const char *res_objtype = NULL;

		if (!osync_plugin_resource_is_enabled(res))
			continue;

		res_objtype = osync_plugin_resource_get_objtype(res);
		if (!res_objtype)
			continue;

		if (!strcmp(res_objtype, objtype))
			return res;

	}

	return NULL;
}

/* External Plugin */
OSyncPluginExternalPlugin *osync_plugin_config_get_externalplugin(OSyncPluginConfig *config)
{
	osync_assert(config);
	return config->externalplugin;
}

void osync_plugin_config_set_externalplugin(OSyncPluginConfig *config, OSyncPluginExternalPlugin *externalplugin)
{
	osync_assert(config);
	
	if (config->externalplugin) {
		osync_plugin_externalplugin_unref(config->externalplugin);
		config->externalplugin = NULL;
	}
	
	if (externalplugin) {
		config->externalplugin = osync_plugin_externalplugin_ref(externalplugin);
	}
}

OSyncPluginConnection *osync_plugin_config_get_connection(OSyncPluginConfig *config)
{
	osync_assert(config);
	return config->connection;
}

void osync_plugin_config_set_connection(OSyncPluginConfig *config, OSyncPluginConnection *connection)
{
	osync_assert(config);

	if (config->connection) {
		osync_plugin_connection_unref(config->connection);
		config->connection = NULL;
	}
	if (connection) {
		config->connection = osync_plugin_connection_ref(connection);
	}
}

