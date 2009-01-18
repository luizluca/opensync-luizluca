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

#include "opensync_file.h"

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

int osync_remove_directory_recursively(const char *dirname)
{
	GDir *gdir = NULL;
	GError *gerror = NULL;
	const char *gdir_entry = NULL; 
	char *gdir_entry_path = NULL;

	gdir = g_dir_open(dirname, 0, &gerror);
        if (!gdir)
		goto error;
	while ((gdir_entry = g_dir_read_name(gdir))) {
		gdir_entry_path = g_strdup_printf("%s%c%s", dirname, G_DIR_SEPARATOR, gdir_entry);
		if (g_file_test(gdir_entry_path, G_FILE_TEST_IS_DIR)) {
			if (osync_remove_directory_recursively(gdir_entry_path) < 0){
				g_set_error(&gerror, G_FILE_ERROR, g_file_error_from_errno(errno), "%s", gdir_entry_path);
				g_free(gdir_entry_path);
				goto error;
			}
		}else{
			if (g_unlink(gdir_entry_path) < 0){
				g_set_error(&gerror, G_FILE_ERROR, g_file_error_from_errno(errno), "%s", gdir_entry_path);
				g_free(gdir_entry_path);
				goto error;
			}
		}
		g_free(gdir_entry_path);
	} /* While */
	g_dir_close(gdir);
	gdir = NULL;
	if (g_rmdir(dirname) < 0){
		g_set_error(&gerror, G_FILE_ERROR, g_file_error_from_errno(errno), "%s", dirname);
		goto error;
	}

	return 0; 
		
error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, gerror->message);
	g_error_free(gerror);
	if (gdir)
		g_dir_close(gdir);
	return -1;	
}

