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

#include "config.h"

#include "opensync.h"
#include "opensync_internals.h"

char *osync_strdup(const char *str)
{
	return g_strdup(str);
}

char *osync_strdup_printf(const char *format, ...)
{
	va_list ap;
	char *str;

	va_start(ap, format);
	str = g_strdup_vprintf(format, ap); 
	va_end(ap);

	return str;
}

char *osync_strreplace(const char *input, const char *delimiter, const char *replacement)
{
	gchar **array;
	gchar *ret;
	osync_return_val_if_fail(input != NULL, NULL);
	osync_return_val_if_fail(delimiter != NULL, NULL);
	osync_return_val_if_fail(replacement != NULL, NULL);

	array = g_strsplit(input, delimiter, 0);
	ret = g_strjoinv(replacement, array);
	g_strfreev(array);

	return ret;
}

char *osync_rand_str(int maxlength, OSyncError **error)
{
	char *randchars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIKLMNOPQRSTUVWXYZ1234567890";
	
	int length;
	char *retchar;
	int i = 0;

	length = g_random_int_range(1, maxlength + 1);

	retchar = osync_try_malloc0(length * sizeof(char) + 1, error);
	if (!retchar)
		goto error;

	retchar[0] = 0;

	for (i = 0; i < length; i++) {
		retchar[i] = randchars[g_random_int_range(0, strlen(randchars))];
		retchar[i + 1] = 0;
	}

	return retchar;

error:
	return NULL;
}

