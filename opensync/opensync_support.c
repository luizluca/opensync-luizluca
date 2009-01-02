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

GPrivate* current_tabs = NULL;
GPrivate* thread_id = NULL;
GPrivate* trace_disabled = NULL;
GPrivate* trace_sensitive = NULL;
GPrivate* print_stderr = NULL;
const char *trace = NULL;

#ifndef _WIN32
#include <pthread.h>
#endif

void osync_trace_reset_indent(void)
{
	g_private_set(current_tabs, GINT_TO_POINTER(0));
}

void osync_trace_disable(void)
{
	if (!trace_disabled)
		trace_disabled = g_private_new (NULL);
	
	g_private_set(trace_disabled, GINT_TO_POINTER(1));
}
void osync_trace_enable(void)
{
	if (!trace_disabled)
		trace_disabled = g_private_new (NULL);

	if (!trace)
		g_private_set(trace_disabled, GINT_TO_POINTER(1));
	else
		g_private_set(trace_disabled, GINT_TO_POINTER(0));

}

osync_bool osync_trace_is_enabled(void)
{
	if (!trace_disabled || !g_private_get(trace_disabled))
		return TRUE;

	return FALSE;
}

/*! @brief Initailize tracing 
 *
 */
static void _osync_trace_init()
{
	const char *noprivacy;
	const char *error;
	trace = g_getenv("OSYNC_TRACE");
	if (!trace)
		return;
	
	noprivacy = g_getenv("OSYNC_NOPRIVACY");
	if (!trace_sensitive)
		trace_sensitive = g_private_new(NULL);

	if (noprivacy)
		g_private_set(trace_sensitive, GINT_TO_POINTER(1));
	else
		g_private_set(trace_sensitive, GINT_TO_POINTER(0));

	error = g_getenv("OSYNC_PRINTERROR");
	if (!print_stderr)
		print_stderr = g_private_new(NULL);

	if (error)
		g_private_set(print_stderr, GINT_TO_POINTER(1));
	else
		g_private_set(print_stderr, GINT_TO_POINTER(0));
	
	if (!g_file_test(trace, G_FILE_TEST_IS_DIR)) {
		printf("OSYNC_TRACE argument is no directory\n");
		return;
	}

}
	
void osync_trace(OSyncTraceType type, const char *message, ...)
{
#ifdef OPENSYNC_TRACE
	va_list arglist;
	char *buffer = NULL;
	int tabs = 0;
	unsigned long int id = 0;
#ifdef _WIN32
	int pid = 0;
	char tmp_buf[1024];
#else
	pid_t pid = 0;
#endif
	char *logfile = NULL;
	GString *tabstr = NULL;
	int i = 0;
	GTimeVal curtime;
	char *logmessage = NULL;
	GError *error = NULL;
	GIOChannel *chan = NULL;
	gsize writen;
	const char *endline = NULL;
	
	if (!g_thread_supported ()) g_thread_init (NULL);
	
	if (!trace_disabled || !g_private_get(trace_disabled)) {
		_osync_trace_init();
		osync_trace_enable();
	}
	
	if (GPOINTER_TO_INT(g_private_get(trace_disabled)))
		return;
	
	if (!current_tabs)
		current_tabs = g_private_new (NULL);
	else
		tabs = GPOINTER_TO_INT(g_private_get(current_tabs));
	
#ifdef _WIN32
	if (!thread_id)
		thread_id = g_private_new (NULL);
	id = GPOINTER_TO_INT(thread_id);
	pid = _getpid();
	endline = "\r\n";
#else
	id = (unsigned long int)pthread_self();
	pid = getpid();
	endline = "\n";
#endif
	logfile = g_strdup_printf("%s%cThread%lu-%i.log", trace, G_DIR_SEPARATOR, id, pid);
	
	va_start(arglist, message);
	
#ifdef _WIN32
	vsnprintf(tmp_buf, 1024, message, arglist);
	buffer = g_strdup(tmp_buf);
#else
	buffer = g_strdup_vprintf(message, arglist);
#endif
	
	tabstr = g_string_new("");
	for (i = 0; i < tabs; i++) {
		tabstr = g_string_append(tabstr, "\t");
	}

	g_get_current_time(&curtime);
	switch (type) {
	case TRACE_ENTRY:
		logmessage = g_strdup_printf("[%li.%06li]\t%s>>>>>>>  %s%s", curtime.tv_sec, curtime.tv_usec, tabstr->str, buffer, endline);
		tabs++;
		break;
	case TRACE_INTERNAL:
		logmessage = g_strdup_printf("[%li.%06li]\t%s%s%s", curtime.tv_sec, curtime.tv_usec, tabstr->str, buffer, endline);
		break;
	case TRACE_SENSITIVE:
		if (GPOINTER_TO_INT(g_private_get(trace_sensitive)))
			logmessage = g_strdup_printf("[%li.%06li]\t%s[SENSITIVE] %s%s", curtime.tv_sec, curtime.tv_usec, tabstr->str, buffer, endline);
		else
			logmessage = g_strdup_printf("[%li.%06li]\t%s[SENSITIVE CONTENT HIDDEN]%s", curtime.tv_sec, curtime.tv_usec, tabstr->str, endline);
		break;
	case TRACE_EXIT:
		logmessage = g_strdup_printf("[%li.%06li]%s<<<<<<<  %s%s", curtime.tv_sec, curtime.tv_usec, tabstr->str, buffer, endline);
		tabs--;
		if (tabs < 0)
			tabs = 0;
		break;
	case TRACE_EXIT_ERROR:
		logmessage = g_strdup_printf("[%li.%06li]%s<--- ERROR --- %s%s", curtime.tv_sec, curtime.tv_usec, tabstr->str, buffer, endline);
		tabs--;
		if (tabs < 0)
			tabs = 0;

		if (print_stderr)
			fprintf(stderr, "EXIT_ERROR: %s\n", buffer);
		break;
	case TRACE_ERROR:
		logmessage = g_strdup_printf("[%li.%06li]%sERROR: %s%s", curtime.tv_sec, curtime.tv_usec, tabstr->str, buffer, endline);

		if (print_stderr)
			fprintf(stderr, "ERROR: %s\n", buffer);

		break;
	}
	g_free(buffer);
	g_private_set(current_tabs, GINT_TO_POINTER(tabs));
	va_end(arglist);
	
	g_string_free(tabstr, TRUE);
	
	chan = g_io_channel_new_file(logfile, "a", &error);
	if (!chan) {
		printf("unable to open %s for writing: %s\n", logfile, error->message);
		return;
	}
	
	g_io_channel_set_encoding(chan, NULL, NULL);
	if (g_io_channel_write_chars(chan, logmessage, strlen(logmessage), &writen, NULL) != G_IO_STATUS_NORMAL) {
		printf("unable to write trace to %s\n", logfile);
	} else
		g_io_channel_flush(chan, NULL);

	g_io_channel_shutdown(chan, TRUE, NULL);
	g_io_channel_unref(chan);
	g_free(logmessage);
	g_free(logfile);
	
#endif /* OPENSYNC_TRACE */
}

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

osync_bool osync_file_write(const char *filename, const char *data, unsigned int size, int mode, OSyncError **oserror)
{
	osync_bool ret = FALSE;
	GError *error = NULL;
	gsize writen;
	
	GIOChannel *chan = g_io_channel_new_file(filename, "w", &error);
	if (!chan) {
		osync_trace(TRACE_INTERNAL, "Unable to open file %s for writing: %s", filename, error->message);
		osync_error_set(oserror, OSYNC_ERROR_IO_ERROR, "Unable to open file %s for writing: %s", filename, error->message);
		return FALSE;
	}
	if (mode) {
		if (g_chmod(filename, mode)) {
			osync_trace(TRACE_INTERNAL, "Unable to set file permissions %i for file %s", mode, filename);
			osync_error_set(oserror, OSYNC_ERROR_IO_ERROR, "Unable to set file permissions %i for file %s", mode, filename);
			return FALSE;
		}
	}
	
	g_io_channel_set_encoding(chan, NULL, NULL);
	if (g_io_channel_write_chars(chan, data, size, &writen, &error) != G_IO_STATUS_NORMAL) {
		osync_trace(TRACE_INTERNAL, "Unable to write contents of file %s: %s", filename, error->message);
		osync_error_set(oserror, OSYNC_ERROR_IO_ERROR, "Unable to write contents of file %s: %s", filename, error->message);
	} else {
		g_io_channel_flush(chan, NULL);
		ret = TRUE;
	}
	g_io_channel_shutdown(chan, FALSE, NULL);
	g_io_channel_unref(chan);
	return ret;
}

osync_bool osync_file_read(const char *filename, char **data, unsigned int *size, OSyncError **oserror)
{
	osync_bool ret = FALSE;
	GError *error = NULL;
	gsize sz = 0;
	GIOChannel *chan = NULL;
	
	if (!filename) {
		osync_trace(TRACE_INTERNAL, "No file open specified");
		osync_error_set(oserror, OSYNC_ERROR_IO_ERROR, "No file to open specified");
		return FALSE;
	}
	
	chan = g_io_channel_new_file(filename, "r", &error);
	if (!chan) {
		osync_trace(TRACE_INTERNAL, "Unable to read file %s: %s", filename, error->message);
		osync_error_set(oserror, OSYNC_ERROR_IO_ERROR, "Unable to open file %s for reading: %s", filename, error->message);
		return FALSE;
	}
	
	g_io_channel_set_encoding(chan, NULL, NULL);
	if (g_io_channel_read_to_end(chan, data, &sz, &error) != G_IO_STATUS_NORMAL) {
		osync_trace(TRACE_INTERNAL, "Unable to read contents of file %s: %s", filename, error->message);
		osync_error_set(oserror, OSYNC_ERROR_IO_ERROR, "Unable to read contents of file %s: %s", filename, error->message);
	} else {
		ret = TRUE;
		if (size)
			*size = (int)sz;
	}
	g_io_channel_shutdown(chan, FALSE, NULL);
	g_io_channel_unref(chan);
	return ret;
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


