/*
 * libopensync - A synchronization framework
 * Copyright (C) 2004-2005  Armin Bauer <armin.bauer@opensync.org>
 * Copyright (C) 2009       Daniel Gollub <dgollub@suse.de>
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
 
#ifndef _OPENSYNC_THREAD_INTERNALS_H
#define _OPENSYNC_THREAD_INTERNALS_H

typedef struct OSyncThread OSyncThread;

OSYNC_TEST_EXPORT OSyncThread *osync_thread_new(GMainContext *context, OSyncError **error);
OSYNC_TEST_EXPORT OSyncThread *osync_thread_ref(OSyncThread *thread);
OSYNC_TEST_EXPORT void osync_thread_unref(OSyncThread *thread);
OSYNC_TEST_EXPORT void osync_thread_start(OSyncThread *thread);
OSYNC_TEST_EXPORT void osync_thread_stop(OSyncThread *thread);
OSYNC_TEST_EXPORT void osync_thread_exit(OSyncThread *thread, int retval);
OSYNC_TEST_EXPORT OSyncThread *osync_thread_create(GThreadFunc func, void *userdata, OSyncError **error);

#endif /* _OPENSYNC_THREAD_INTERNALS_H */

