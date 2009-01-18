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
 
#ifndef _OPENSYNC_THREAD_PRIVATE_H
#define _OPENSYNC_THREAD_PRIVATE_H

/**
 * @defgroup OSyncThreadPrivateAPI OpenSync Thread Private
 * @ingroup OSyncCommonPrivate
 */

/*@{*/

/**
 * @brief Represents a Thread
 */
struct OSyncThread {
	GThread *thread;
	GCond *started;
	GMutex *started_mutex;
	GMainContext *context;
	GMainLoop *loop;
	/** reference counter */
	int ref_count;
};

/** @brief Start callback function to emit signal when thread's mainloop got started
 *
 * @param data Pointer to passed callback data
 * @returns Always FALSE
 */
static gboolean osyncThreadStartCallback(gpointer data);

/** @brief Stop callback function to stop thread mainloop
 *
 * @param data Pointer to passed callback data
 * @returns Always FALSE
 */
static gboolean osyncThreadStopCallback(gpointer data);

/*@}*/

#endif /* _OPENSYNC_THREAD_PRIVATE_H */

