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


#ifdef OPENSYNC_TRACE
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
#endif /* OPENSYNC_TRACE */
	
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

