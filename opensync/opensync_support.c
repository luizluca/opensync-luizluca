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

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include "opensync.h"
#include "opensync_internals.h"

#include "opensync-support.h"
#include "opensync_support_internals.h"



/*! @brief Used for printing binary data
 * 
 * Unprintable character will be printed in hex, printable are just printed
 * 
 * @param data The data to print
 * @param len The length to print
 * 
 */
char *osync_print_binary(const unsigned char *data, int len)
{
	int t;
	GString *str = g_string_new("");
	for (t = 0; t < len; t++) {
		if (data[t] >= ' ' && data[t] <= 'z')
			g_string_append_c(str, data[t]);
		else
			g_string_append_printf(str, " %02x ", data[t]);
	}
	return g_string_free(str, FALSE);
}

const char *osync_get_version(void)
{
	return OPENSYNC_VERSION;
}

void *osync_try_malloc0(unsigned int size, OSyncError **error)
{
	void *result = NULL;
	
#ifdef OPENSYNC_UNITTESTS 	
	if (!g_getenv("OSYNC_NOMEMORY"))
		result = g_try_malloc(size);
#else		
	result = g_try_malloc(size);
#endif /*OPENSYNC_UNITTESTS*/

	if (!result) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "No memory left");
		return NULL;
	}
	memset(result, 0, size);
	return result;
}

void osync_free(void *ptr)
{
	if (!ptr)
		return;

	g_free(ptr);
}

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

char *osync_rand_str(int maxlength)
{
	char *randchars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIKLMNOPQRSTUVWXYZ1234567890";
	
	int length;
	char *retchar;
	int i = 0;

	length = g_random_int_range(1, maxlength + 1);

	retchar = osync_try_malloc0(length * sizeof(char) + 1, NULL);
	osync_return_val_if_fail(retchar, NULL);

	retchar[0] = 0;

	for (i = 0; i < length; i++) {
		retchar[i] = randchars[g_random_int_range(0, strlen(randchars))];
		retchar[i + 1] = 0;
	}

	return retchar;
}


/*! @brief Bit counting
 * 
 * MIT HAKMEM Count, Bit counting in constant time and memory. 
 * 
 * @param u unsigned integer value to count bits
 * @returns The bit counting result 
 * 
 */
int osync_bitcount(unsigned int u)
{
	unsigned int uCount = u - ((u >> 1) & 033333333333) - ((u >> 2) & 011111111111);
	return ((uCount + (uCount >> 3)) & 030707070707) % 63;
}


