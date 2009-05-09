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
#include "opensync_plugin_authentication_private.h"

OSyncPluginAuthentication *osync_plugin_authentication_new(OSyncError **error)
{
	OSyncPluginAuthentication *auth = osync_try_malloc0(sizeof(OSyncPluginAuthentication), error);
	if (!auth)
		return NULL;

	auth->ref_count = 1;

	return auth;
}

OSyncPluginAuthentication *osync_plugin_authentication_ref(OSyncPluginAuthentication *auth)
{
	osync_assert(auth);
	
	g_atomic_int_inc(&(auth->ref_count));

	return auth;
}

void osync_plugin_authentication_unref(OSyncPluginAuthentication *auth)
{
	osync_assert(auth);
	
	if (g_atomic_int_dec_and_test(&(auth->ref_count))) {
		if (auth->username)
			osync_free(auth->username);
			
		if (auth->password)
			osync_free(auth->password);

		if (auth->reference)
			osync_free(auth->reference);
			
		osync_free(auth);
	}
}

osync_bool osync_plugin_authentication_option_is_supported(OSyncPluginAuthentication *auth, OSyncPluginAuthenticationOptionSupportedFlag flag)
{
	osync_assert(auth);
	if (auth->supported_options & flag)
		return TRUE;

	return FALSE;
}

void osync_plugin_authentication_option_set_supported(OSyncPluginAuthentication *auth, OSyncPluginAuthenticationOptionSupportedFlags flags)
{
	osync_assert(auth);
	auth->supported_options = flags;
}

const char *osync_plugin_authentication_get_username(OSyncPluginAuthentication *auth)
{
	osync_assert(auth);
	return auth->username;
}

void osync_plugin_authentication_set_username(OSyncPluginAuthentication *auth, const char *username)
{
	osync_assert(auth);

	if (auth->username)
		osync_free(auth->username);

	auth->username = osync_strdup(username);
}

const char *osync_plugin_authentication_get_password(OSyncPluginAuthentication *auth)
{
	osync_assert(auth);
	return auth->password;
}

void osync_plugin_authentication_set_password(OSyncPluginAuthentication *auth, const char *password)
{
	osync_assert(auth);

	if (auth->password)
		osync_free(auth->password);

	auth->password = osync_strdup(password);
}

const char *osync_plugin_authentication_get_reference(OSyncPluginAuthentication *auth)
{
	osync_assert(auth);
	return auth->reference;
}

void osync_plugin_authentication_set_reference(OSyncPluginAuthentication *auth, const char *reference)
{
	osync_assert(auth);

	if (auth->reference)
		osync_free(auth->reference);

	auth->reference = osync_strdup(reference);
}

