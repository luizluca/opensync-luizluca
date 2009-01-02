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

#include "opensync_thread_private.h"
#include "opensync_thread_internals.h"

/**
 * @defgroup OSyncThreadAPI OpenSync Thread Module 
 * @ingroup OSyncPrivate
 * @brief Some threading  functions
 * 
 */
/*@{*/

/*! @brief Stop callback function to stop thread mainloop
 *
 * @param data Pointer to passed callback data
 * @returns Always FALSE
 */
static gboolean osyncThreadStopCallback(gpointer data)
{
	OSyncThread *thread = data;
	
	g_main_loop_quit(thread->loop);
	
	return FALSE;
}

/*! @brief Start callback function to emit signal when thread's mainloop got started
 *
 * @param data Pointer to passed callback data
 * @returns Always FALSE
 */
static gboolean osyncThreadStartCallback(gpointer data)
{
	OSyncThread *thread = data;
	
	g_mutex_lock(thread->started_mutex);
	g_cond_signal(thread->started);
	g_mutex_unlock(thread->started_mutex);
	
	return FALSE;
}

/*! @brief Allocates a new thread with a g_mainloop 
 * 
 * @param context Pointer to GMainContext 
 * @param error The error which will hold the info in case of an error
 * @returns A pointer to the new allocated OSyncThread with inactive thread and mainloop 
 * 
 */
OSyncThread *osync_thread_new(GMainContext *context, OSyncError **error)
{
	OSyncThread *thread = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, context, error);
	
	thread = osync_try_malloc0(sizeof(OSyncThread), error);
	if (!thread)
		goto error;
	thread->ref_count = 1;

	if (!g_thread_supported ()) g_thread_init (NULL);
	
	thread->started_mutex = g_mutex_new();
	thread->started = g_cond_new();
	thread->context = context;
	if (thread->context)
		g_main_context_ref(thread->context);
	thread->loop = g_main_loop_new(thread->context, FALSE);
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, thread);
	return thread;
	
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

/** @brief Increases the reference count on thread object 
 * 
 * @param thread Pointer to OSyncThread
 * 
 */
OSyncThread *osync_thread_ref(OSyncThread *thread)
{
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, thread);
	osync_assert(thread);

	g_atomic_int_inc(&(thread->ref_count));

	osync_trace(TRACE_EXIT, "%s", __func__);
	return thread;
}

/** @brief Decrements reference count on thread object 
 * 
 * @param thread Pointer to OSyncThread
 * 
 */
void osync_thread_unref(OSyncThread *thread)
{
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, thread);
	osync_assert(thread);
	
	if (g_atomic_int_dec_and_test(&(thread->ref_count))) {
		if (thread->started_mutex)
			g_mutex_free(thread->started_mutex);

		if (thread->started)
			g_cond_free(thread->started);
	
		if (thread->loop)
			g_main_loop_unref(thread->loop);
	
		if (thread->context)
			g_main_context_unref(thread->context);
		
		osync_free(thread);
	}
	osync_trace(TRACE_EXIT, "%s", __func__);
}

/*static gpointer osyncThreadStartCallback(gpointer data)
	{
	OSyncThread *thread = data;
	
	g_mutex_lock(thread->started_mutex);
	g_cond_signal(thread->started);
	g_mutex_unlock(thread->started_mutex);
	
	g_main_loop_run(thread->loop);
	
	return NULL;
	}*/

/*void osync_thread_start(OSyncThread *thread)
	{
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, thread);
	osync_assert(thread);
	
	//Start the thread
	g_mutex_lock(thread->started_mutex);
	thread->thread = g_thread_create (osyncThreadStartCallback, thread, TRUE, NULL);
	g_cond_wait(thread->started, thread->started_mutex);
	g_mutex_unlock(thread->started_mutex);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	}*/


/*! @brief Start thread and it's mainloop 
 * 
 * @param func GThreadFunc Pointer
 * @param userdata Custom data poitner which get supplied to thread function
 * @param error Pointer to error struct
 * @return Newly allocate OSyncThread object with inactive mainloop
 * 
 */
OSyncThread *osync_thread_create(GThreadFunc func, void *userdata, OSyncError **error)
{
	GError *gerror;
	OSyncThread *thread;

	osync_assert(func);
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, func, userdata, error);

	thread = osync_try_malloc0(sizeof(OSyncThread), error);
	if (!thread)
		goto error;
	thread->ref_count = 1;

	if (!g_thread_supported ())
		g_thread_init (NULL);

	thread->thread = g_thread_create(func, userdata, TRUE, &gerror);

	if (!thread->thread) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "%s", gerror->message);
		g_error_free(gerror);
		goto error_free_thread;
	}

	osync_trace(TRACE_EXIT, "%s", __func__);
	return thread;

 error_free_thread:
	osync_free(thread);
 error:	
	osync_trace(TRACE_EXIT_ERROR, "%s", __func__, osync_error_print(error));
	return NULL;
}


/*! @brief Start thread and it's mainloop 
 * 
 * @param thread Thread object 
 * 
 */
void osync_thread_start(OSyncThread *thread)
{
	GSource *idle = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, thread);
	
	g_mutex_lock(thread->started_mutex);
	idle = g_idle_source_new();
	g_source_set_callback(idle, osyncThreadStartCallback, thread, NULL);
	g_source_attach(idle, thread->context);
	thread->thread = g_thread_create ((GThreadFunc)g_main_loop_run, thread->loop, TRUE, NULL);
	g_cond_wait(thread->started, thread->started_mutex);
	g_mutex_unlock(thread->started_mutex);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}	

/*! @brief Stops thread's mainloop and joins the thread
 * 
 * @param thread Thread object 
 * 
 */
void osync_thread_stop(OSyncThread *thread)
{
	GSource *source = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, thread);
	osync_assert(thread);

	source = g_idle_source_new();
	g_source_set_callback(source, osyncThreadStopCallback, thread, NULL);
	g_source_attach(source, thread->context);

	g_thread_join(thread->thread);
	thread->thread = NULL;
	
	g_source_unref(source);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

/*! @brief Exit thread 
 * 
 * @param thread Thread object 
 * @param retval Return value of thread while exiting
 * 
 */
void osync_thread_exit(OSyncThread *thread, int retval)
{
	GSource *source = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %i)", __func__, thread, retval);
	osync_assert(thread);

	source = g_idle_source_new();
	g_source_set_callback(source, osyncThreadStopCallback, thread, NULL);
	g_source_attach(source, thread->context);
	g_source_unref(source);
	thread->thread = NULL;

	g_thread_exit(GINT_TO_POINTER(retval));
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}



/*@}*/

