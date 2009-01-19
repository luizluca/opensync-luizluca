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

/**
 * @defgroup OSyncThreadInternalAPI OpenSync Thread Internals
 * @ingroup OSyncCommonPrivate
 * @brief Some threading functions
 */

/*@{*/

typedef struct OSyncThread OSyncThread;

/** @brief Allocates a new thread with a g_mainloop 
 * 
 * @param context Pointer to GMainContext 
 * @param error The error which will hold the info in case of an error
 * @returns A pointer to the new allocated OSyncThread with inactive thread and mainloop 
 * 
 */
OSYNC_TEST_EXPORT OSyncThread *osync_thread_new(GMainContext *context, OSyncError **error);

/** @brief Increases the reference count on thread object 
 * 
 * @param thread Pointer to OSyncThread
 * 
 */
OSYNC_TEST_EXPORT OSyncThread *osync_thread_ref(OSyncThread *thread);

/** @brief Decrements reference count on thread object 
 * 
 * @param thread Pointer to OSyncThread
 * 
 */
OSYNC_TEST_EXPORT void osync_thread_unref(OSyncThread *thread);

/** @brief Start thread and it's mainloop 
 * 
 * @param thread Thread object 
 * 
 */
OSYNC_TEST_EXPORT void osync_thread_start(OSyncThread *thread);

/** @brief Stops thread's mainloop and joins the thread
 * 
 * @param thread Thread object 
 * 
 */
OSYNC_TEST_EXPORT void osync_thread_stop(OSyncThread *thread);

/** @brief Exit thread 
 * 
 * @param thread Thread object 
 * @param retval Return value of thread while exiting
 * 
 */
OSYNC_TEST_EXPORT void osync_thread_exit(OSyncThread *thread, int retval);

/** @brief Start thread and it's mainloop 
 * 
 * @param func GThreadFunc Pointer
 * @param userdata Custom data poitner which get supplied to thread function
 * @param error Pointer to error struct
 * @return Newly allocate OSyncThread object with inactive mainloop
 * 
 */
OSYNC_TEST_EXPORT OSyncThread *osync_thread_create(GThreadFunc func, void *userdata, OSyncError **error);

/*@}*/

#endif /* _OPENSYNC_THREAD_INTERNALS_H */

