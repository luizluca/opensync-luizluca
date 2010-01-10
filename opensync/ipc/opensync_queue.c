/*
 * libosengine - A synchronization engine for the opensync framework
 * Copyright (C) 2004-2005  Armin Bauer <armin.bauer@opensync.org>
 * Copyright (C) 2007  Daniel Friedrich <daniel.friedrich@opensync.org>
 * Copyright (C) 2009 Graham R. Cobb <g+opensync@cobb.uk.net>
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

#include "opensync_message_internals.h"
#include "opensync_queue.h"

#include "opensync_queue_internals.h"
#include "opensync_queue_private.h"

static gboolean _osync_queue_generate_error(OSyncQueue *queue, OSyncMessageCommand errcode, OSyncError **error)
{
	OSyncMessage *message;

	queue->connected = FALSE;
	
	/* Now we can send the hup message, and wake up the consumer thread so
	 * it can pickup the messages in the incoming queue */
	message = osync_message_new(errcode, 0, error);
	if (!message) {
		return FALSE;
	}
	osync_trace(TRACE_INTERNAL, "Generating incoming error message %p(%s), id= %lli", message, osync_message_get_commandstr(message), osync_message_get_id(message));
	
	osync_message_ref(message);

	g_async_queue_push(queue->incoming, message);
	
	if (queue->incomingContext)
		g_main_context_wakeup(queue->incomingContext);

	return TRUE;
}

static gboolean _timeout_prepare(GSource *source, gint *timeout_)
{
	/* TODO adapt *timeout_ value to shortest message timeout value...
	   GTimeVal current_time;
	   g_source_get_current_time(source, &current_time);
	*/

	*timeout_ = 1;
	return FALSE;
}

static gboolean _timeout_check(GSource *source)
{
	OSyncList *p;
	GTimeVal current_time;
	OSyncTimeoutInfo *toinfo;
	OSyncPendingMessage *pending;

	OSyncQueue *queue = *((OSyncQueue **)(source + 1));

	g_source_get_current_time(source, &current_time);

	/* Search for the pending reply. We have to lock the
	 * list since another thread might be duing the updates */
	g_mutex_lock(queue->pendingLock);

	/* First check the overall queue timer */
	if (queue->pendingCount> 0 && queue->pending_timeout.tv_sec > 0) {
		if (current_time.tv_sec > queue->pending_timeout.tv_sec
		    || (current_time.tv_sec == queue->pending_timeout.tv_sec
			&& current_time.tv_usec >= queue->pending_timeout.tv_usec) ) {
			g_mutex_unlock(queue->pendingLock);
			return TRUE;
		}
	}

	for (p = queue->pendingReplies; p; p = p->next) {
		pending = p->data;

		if (!pending->timeout_info)
			continue;

		toinfo = pending->timeout_info;

		if (current_time.tv_sec > toinfo->expiration.tv_sec 
				|| (current_time.tv_sec == toinfo->expiration.tv_sec 
						&& current_time.tv_usec >= toinfo->expiration.tv_usec)) {
			/* Unlock the pending lock since the messages might be sent during the callback */
			g_mutex_unlock(queue->pendingLock);

			return TRUE;
		}
	}
	/* Unlock the pending lock since the messages might be sent during the callback */
	g_mutex_unlock(queue->pendingLock);

	return FALSE;
}

static gboolean _timeout_dispatch(GSource *source, GSourceFunc callback, gpointer user_data)
{
	OSyncList *p;
	OSyncPendingMessage *pending;
	OSyncTimeoutInfo *toinfo;
	OSyncQueue *queue = NULL;
	GTimeVal current_time;

	osync_trace(TRACE_INTERNAL, "%s(%p)", __func__, user_data);

	queue = *((OSyncQueue **)(source + 1));

	g_source_get_current_time (source, &current_time);

	/* Search for the pending reply. We have to lock the
	 * list since another thread might be duing the updates */
	g_mutex_lock(queue->pendingLock);

	/* First check the overall queue timer */
	if (queue->pendingCount> 0 && queue->pending_timeout.tv_sec > 0) {
		if (current_time.tv_sec > queue->pending_timeout.tv_sec
		    || (current_time.tv_sec == queue->pending_timeout.tv_sec
			&& current_time.tv_usec >= queue->pending_timeout.tv_usec) ) {
			/* The queue has died.  Generate an error */
			osync_trace(TRACE_INTERNAL, "%s: Pending queue timer expired: receiver must have died", __func__);
			_osync_queue_generate_error(queue, OSYNC_MESSAGE_QUEUE_ERROR, NULL);
			queue->pending_timeout.tv_sec = 0; // Stop timer
		}
	}

	for (p = queue->pendingReplies; p; p = p->next) {
		pending = p->data;

		if (!pending->timeout_info)
			continue;

		toinfo = pending->timeout_info;

		if (current_time.tv_sec > toinfo->expiration.tv_sec ||
				(current_time.tv_sec == toinfo->expiration.tv_sec 
				 && current_time.tv_usec >= toinfo->expiration.tv_usec)) {
			OSyncError *error = NULL;
			OSyncError *timeouterr = NULL;
			OSyncMessage *errormsg = NULL;

			osync_trace(TRACE_INTERNAL, "Timeout for message with id %lli", pending->id);


			/* Call the callback of the pending message */
			osync_assert(pending->callback);
			osync_error_set(&timeouterr, OSYNC_ERROR_IO_ERROR, "Timeout.");
			errormsg = osync_message_new_errorreply(NULL, timeouterr, &error);
			osync_error_unref(&timeouterr);
			osync_message_set_id(errormsg, pending->id);

			/* Remove first the pending message!
				 To avoid that _incoming_dispatch catchs this message
				 when we're releasing the lock. If _incoming_dispatch
				 would catch this message, the pending callback
				 gets called twice! */

			queue->pendingReplies = osync_list_remove(queue->pendingReplies, pending);
			queue->pendingCount--;
			/* Unlock the pending lock since the messages might be sent during the callback */
			g_mutex_unlock(queue->pendingLock);

			pending->callback(errormsg, pending->user_data);
			if (errormsg != NULL)
				osync_message_unref(errormsg);

			// TODO: Refcounting for OSyncPendingMessage
			g_free(pending->timeout_info);
			osync_free(pending);

			/* Lock again, to keep the iteration of the pendingReplies list atomic. */
			g_mutex_lock(queue->pendingLock);

			/* The queue may have been modified while it was unlocked so don't go
			   looking for any more entries.  If there are more, they will be found
			   on the next call */
			break;
		}
	}
	
	g_mutex_unlock(queue->pendingLock);

	return TRUE;
}

static gboolean _incoming_prepare(GSource *source, gint *timeout_)
{
	*timeout_ = 1;
	return FALSE;
}

static gboolean _incoming_check(GSource *source)
{
	OSyncQueue *queue = *((OSyncQueue **)(source + 1));

	/* As well as checking there is something on the incoming queue, we check
	   that we are not blocked because we have too many pending commands.
	   This check is only done if the pendingLimit has been set.

	   Note, to avoid taking out the pending lock in order to count
	   the length of the pending queue we use the pendingCount counter.
	   This better not get out of line! */

	if (g_async_queue_length(queue->incoming) > 0
	    && (queue->pendingLimit == 0 
		|| queue->pendingCount < queue->pendingLimit) 
	    && !queue->disc_in_progress )
		return TRUE;
	
	return FALSE;
}

static void _osync_send_timeout_response(OSyncMessage *errormsg, void *user_data)
{
	OSyncQueue *queue = user_data;

	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, errormsg, queue);

	if (queue->reply_queue) {
		osync_queue_send_message_with_timeout(queue->reply_queue, NULL, errormsg, 0, NULL);
	} else {
		osync_message_unref(errormsg);
		/* As we can't send the timeout response, create an error and drop it in the incoming queue
		   so the higher layer will disconnect. Note: we can't just call osync_queue_disconnect 
		   as that tries to kill the thread and hits a deadlock! */
		if (!_osync_queue_generate_error(queue, OSYNC_MESSAGE_QUEUE_ERROR, NULL)) {
			osync_trace(TRACE_EXIT_ERROR, "%s: cannot even generate error on incoming queue", __func__);
			return;
		}
		osync_trace(TRACE_EXIT_ERROR, "%s: cannot find reply queue to send timeout error", __func__);
		return;
	}
	osync_trace(TRACE_EXIT, "%s", __func__);
}

/* Restart the pending queue timeout */
static void _osync_queue_restart_pending_timeout(OSyncQueue *queue)
{
	/* Note that the pending queue timeout is just to make sure that progress is being made.
	   It does not time individual commands -- it just makes sure that responses are being
	   received for outstanding commands at some rate. Individual message timeouts are
	   handled at the receiver.  The main purpose of this timer is to detect if the
	   receiver has stopped receiving for some reason.
	   
	   The timer is started when the first message is put on the pending queue and is reset
	   and restarted whenever a response is received.  The timeout value is based on the
	   largest message timeout seen to date. */

	/* Note: queue->pending_timout is protected by the pending lock, which should be held 
	   by the caller before calling this function */

	if (queue->max_timeout) {
		unsigned int timeout;
		timeout = queue->max_timeout + OSYNC_QUEUE_PENDING_QUEUE_IPC_DELAY;
		if (timeout < OSYNC_QUEUE_PENDING_QUEUE_MIN_TIMEOUT) 
			timeout = OSYNC_QUEUE_PENDING_QUEUE_MIN_TIMEOUT;
		g_source_get_current_time(queue->timeout_source, &queue->pending_timeout);
		queue->pending_timeout.tv_sec += timeout;
	}
}

/* Find and remove a pending message that this message is a reply to */
static void _osync_queue_remove_pending_reply(OSyncQueue *queue, OSyncMessage *reply, gboolean callback)
{
	OSyncPendingMessage *pending = NULL;
	OSyncList *p = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %d)", __func__, queue, reply, callback);
	osync_trace(TRACE_INTERNAL, "Searching for pending message id=%lli", osync_message_get_id(reply));

	/* Search for the pending reply. We have to lock the
	 * list since another thread might be duing the updates */
	g_mutex_lock(queue->pendingLock);
			
	for (p = queue->pendingReplies; p; p = p->next) {
		pending = p->data;
			
		if (pending->id == osync_message_get_id(reply)) {
			
			osync_trace(TRACE_INTERNAL, "Found pending message id=%lli: %p", osync_message_get_id(reply), pending);
			/* Remove first the pending message!
			   To avoid that _timeout_dispatch catchs this message
			   when we're releasing the lock. If _timeout_dispatch
			   would catch this message, the pending callback
			   gets called twice! */
			
			queue->pendingReplies = osync_list_remove(queue->pendingReplies, pending);
			if (--queue->pendingCount != 0)
				_osync_queue_restart_pending_timeout(queue);

			/* Unlock the pending lock since the messages might be sent during the callback */
			g_mutex_unlock(queue->pendingLock);
			
			if (callback) {
				/* Call the callback of the pending message */
				osync_assert(pending->callback);
				pending->callback(reply, pending->user_data);
			}

			// TODO: Refcounting for OSyncPendingMessage
			if (pending->timeout_info)
				g_free(pending->timeout_info);
			osync_free(pending);

			osync_trace(TRACE_EXIT, "%s", __func__);
			return;
		}
	}
	
	g_mutex_unlock(queue->pendingLock);
	osync_trace(TRACE_EXIT, "%s", __func__);
}

/* This function is called from the master thread. The function dispatched incoming data from
 * the remote end */
static gboolean _incoming_dispatch(GSource *source, GSourceFunc callback, gpointer user_data)
{
	OSyncQueue *queue = user_data;
	OSyncMessage *message = NULL;
	OSyncError *error = NULL;
	
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, user_data);
	osync_trace(TRACE_INTERNAL, "queue->pendingCount = %d, queue->pendingLimit = %d", queue->pendingCount, queue->pendingLimit);

	while ( (queue->pendingLimit == 0 || queue->pendingCount < queue->pendingLimit)
		&& (message = g_async_queue_try_pop(queue->incoming)) ) {

		/* We check if the message is a reply to something */
		osync_trace(TRACE_INTERNAL, "Dispatching %p:%i(%s), timeout=%d, id=%lli", message, 
			    osync_message_get_cmd(message), osync_message_get_commandstr(message), 
			    osync_message_get_timeout(message), osync_message_get_id(message));
		
		if (osync_message_get_cmd(message) == OSYNC_MESSAGE_REPLY || osync_message_get_cmd(message) == OSYNC_MESSAGE_ERRORREPLY) {
			/* Remove pending reply and call callback*/
			_osync_queue_remove_pending_reply(queue, message, TRUE);
		} else {
			unsigned int timeout = osync_message_get_timeout(message);
			if (timeout) {
				/* This message has a timeout. Put message on pending list and run timeout */
				/* NOTE: We really want to put it on the pending list for the corresponding
				   outgoing queue but there may not be one at this time (if this is an Initialise,
				   for example).  So we put it on the pending list of the incoming queue and deal with
				   finding the outgoing queue if the timeout fires.  This also complicates removing
				   pending items when sending the responses. Oh well. */
				
				OSyncPendingMessage *pending = osync_try_malloc0(sizeof(OSyncPendingMessage), &error);
				if (!pending)
					goto error;

				pending->id = osync_message_get_id(message);

				OSyncTimeoutInfo *toinfo = osync_try_malloc0(sizeof(OSyncTimeoutInfo), &error);
				if (!toinfo)
					goto error;

				GTimeVal current_time;
				g_source_get_current_time(queue->timeout_source, &current_time);

				toinfo->expiration = current_time;
				toinfo->expiration.tv_sec += timeout;

				pending->timeout_info = toinfo;

				pending->callback = _osync_send_timeout_response;
				pending->user_data = queue;
		
				g_mutex_lock(queue->pendingLock);
				queue->pendingReplies = osync_list_append(queue->pendingReplies, pending);
				queue->pendingCount++;
				g_mutex_unlock(queue->pendingLock);
			}

			queue->message_handler(message, queue->user_data);
		}
		
		osync_message_unref(message);
	}
	
	osync_trace(TRACE_EXIT, "%s: Done dispatching", __func__);
	return TRUE;

 error:
	/* TODO: as we can't send the timeout response, disconnect the queue to signal to
	   the sender that they aren't going to get a response.  Note: we can't just
	   call osync_queue_disconnect as that tries to kill the thread and hits a deadlock! */

	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(&error));
	return TRUE;
}

static void _osync_queue_stop_incoming(OSyncQueue *queue)
{
	if (queue->incoming_source) {
		g_source_destroy(queue->incoming_source);
		queue->incoming_source = NULL;
	}
	
	if (queue->incomingContext) {
		g_main_context_unref(queue->incomingContext);
		queue->incomingContext = NULL;
	}
	
	if (queue->incoming_functions) {
		g_free(queue->incoming_functions);
		queue->incoming_functions = NULL;
	}
}

static gboolean _queue_prepare(GSource *source, gint *timeout_)
{
	*timeout_ = 1;
	return FALSE;
}

static gboolean _queue_check(GSource *source)
{
	OSyncQueue *queue = *((OSyncQueue **)(source + 1));
	if (g_async_queue_length(queue->outgoing) > 0)
		return TRUE;
	return FALSE;
}

static int _osync_queue_write_data(OSyncQueue *queue, const void *vptr, size_t n, OSyncError **error)
{
#ifdef _WIN32
	return FALSE;
#else //_WIN32

	ssize_t nwritten = 0;

	while (n > 0) {
		if ((nwritten = write(queue->fd, vptr, n)) <= 0) {
			if (errno == EINTR)
				nwritten = 0;  /* and call write() again */
			else {
				osync_error_set(error, OSYNC_ERROR_IO_ERROR, "Unable to write IPC data: %i: %s", errno, g_strerror(errno));
				return (-1);  /* error */
			}
		}
		
		n -= nwritten;
		vptr += nwritten;
	}
	return (nwritten);
#endif //_WIN32
}

static osync_bool _osync_queue_write_long_long_int(OSyncQueue *queue, const long long int message, OSyncError **error)
{
	if (_osync_queue_write_data(queue, &message, sizeof(long long int), error) < 0)
		return FALSE;

	return TRUE;
}

static osync_bool _osync_queue_write_int(OSyncQueue *queue, const int message, OSyncError **error)
{
	if (_osync_queue_write_data(queue, &message, sizeof(int), error) < 0)
		return FALSE;

	return TRUE;
}

/* This function sends the data to the remote side. If there is an error, it sends an error
 * message to the incoming queue */
static gboolean _queue_dispatch(GSource *source, GSourceFunc callback, gpointer user_data)
{
	OSyncQueue *queue = user_data;
	OSyncError *error = NULL;
	
	OSyncMessage *message = NULL;
	
	while ((message = g_async_queue_try_pop(queue->outgoing))) {
		char *data = NULL;
		unsigned int length = 0;

		/* Check if the queue is connected */
		if (!queue->connected) {
			osync_error_set(&error, OSYNC_ERROR_GENERIC, "Trying to send to a queue thats not connected");
			goto error;
		}

		/* When using threadec communication, the message is directly passed to the */
		/* incoming asynch queue of the connected queue                             */
		if (queue->usethreadcom){
			g_async_queue_push(queue->connected_queue->incoming, message);
			continue;
		}
		
		/* The size of the message */
		if (!_osync_queue_write_int(queue, osync_message_get_message_size(message), &error))
			goto error;
		
		/* The command type (integer) */
		if (!_osync_queue_write_int(queue, osync_message_get_cmd(message), &error))
			goto error;

		/* The id (long long int) */
		if (!_osync_queue_write_long_long_int(queue, osync_message_get_id(message), &error))
			goto error;
		
		/* The timeout (integer) */
		if (!_osync_queue_write_int(queue, (int) osync_message_get_timeout(message), &error))
			goto error;
		
		if (!osync_message_get_buffer(message, &data, &length, &error))
			goto error;
		
		if (length) {
			unsigned int sent = 0;
			do {
				int written = _osync_queue_write_data(queue, data + sent, length - sent, &error);
				if (written < 0)
					goto error;
				
				sent += written;
			} while (sent < length);
		}
		
		osync_message_unref(message);
	}
	
	return TRUE;
	
 error:
	if (message)
		osync_message_unref(message);
	
	if (error) {
		message = osync_message_new_queue_error(error, NULL);
		if (message)
			g_async_queue_push(queue->incoming, message);
		
		osync_error_unref(&error);
	}
	return FALSE;
}

static gboolean _source_prepare(GSource *source, gint *timeout_)
{
	*timeout_ = 1;
	return FALSE;
}

static int _osync_queue_read_data(OSyncQueue *queue, void *vptr, size_t n, OSyncError **error)
{
#ifdef _WIN32
	return 0;
#else //_WIN32
	size_t nleft;
	ssize_t nread = 0;

	nleft = n;
	while (n > 0) {
		if ((nread = read(queue->fd, vptr, nleft)) < 0) {
			if (errno == EINTR)
				nread = 0;  /* and call read() again */
			else {
				osync_error_set(error, OSYNC_ERROR_IO_ERROR, "Unable to read IPC data: %i: %s", errno, g_strerror(errno));
				return (-1);
			}
		} else if (nread == 0)
			break;  /* EOF */
		
		nleft -= nread;
		vptr += nread;
	}
	return (n - nleft);  /* return >= 0 */
#endif //_WIN32
}

static osync_bool _osync_queue_read_int(OSyncQueue *queue, int *message, OSyncError **error)
{
	int read = _osync_queue_read_data(queue, message, sizeof(int), error);
	
	if (read < 0)
		return FALSE;

	if (read != sizeof(int)) {
		osync_error_set(error, OSYNC_ERROR_IO_ERROR, "Unable to read int. EOF");
		return FALSE;
	}
	
	return TRUE;
}

static osync_bool _osync_queue_read_long_long_int(OSyncQueue *queue, long long int *message, OSyncError **error)
{
	int read = _osync_queue_read_data(queue, message, sizeof(long long int), error);

	if (read < 0)
		return FALSE;
	
	if (read != sizeof(long long int)) {
		osync_error_set(error, OSYNC_ERROR_IO_ERROR, "Unable to read int. EOF");
		return FALSE;
	}

	return TRUE;
}

static gboolean _source_check(GSource *source)
{
	OSyncQueue *queue = *((OSyncQueue **)(source + 1));
	OSyncMessage *message = NULL;
	OSyncError *error = NULL;
	
	if (queue->connected == FALSE) {
		/* Ok. so we arent connected. lets check if there are pending replies. We cannot
		 * receive any data on the pipe, therefore, any pending replies will never
		 * be answered. So we return error messages for all of them. */
		if (queue->pendingReplies) {
			OSyncList *p = NULL;
			g_mutex_lock(queue->pendingLock);
			osync_error_set(&error, OSYNC_ERROR_IO_ERROR, "Broken Pipe");
			for (p = queue->pendingReplies; p; p = p->next) {
				OSyncPendingMessage *pending = p->data;
				
				message = osync_message_new_errorreply(NULL, error, NULL);
				if (message) {
					osync_message_set_id(message, pending->id);
					
					g_async_queue_push(queue->incoming, message);
				}
			}
			
			osync_error_unref(&error);
			g_mutex_unlock(queue->pendingLock);
		}
		
		return FALSE;
	}

	if (queue->usethreadcom){
		if (queue->connection_closing)
			_osync_queue_generate_error(queue, OSYNC_MESSAGE_QUEUE_HUP, &error);
		return FALSE;
	}
	
	switch (osync_queue_poll(queue)) {
	case OSYNC_QUEUE_EVENT_NONE:
		return FALSE;
	case OSYNC_QUEUE_EVENT_READ:
		return TRUE;
	case OSYNC_QUEUE_EVENT_HUP:
	case OSYNC_QUEUE_EVENT_ERROR:
		if (!_osync_queue_generate_error(queue, OSYNC_MESSAGE_QUEUE_HUP, &error))
			goto error;
		return FALSE;
	}
	
	return FALSE;

 error:
	message = osync_message_new_queue_error(error, NULL);
	if (message) 
		g_async_queue_push(queue->incoming, message);
	
	osync_error_unref(&error);
	return FALSE;
}

/* This function reads from the file descriptor and inserts incoming data into the
 * incoming queue */
static gboolean _source_dispatch(GSource *source, GSourceFunc callback, gpointer user_data)
{
	OSyncQueue *queue = user_data;
	OSyncMessage *message = NULL;
	OSyncError *error = NULL;
	
	do {
		int size = 0;
		int cmd = 0, timeout = 0;
		long long int id = 0;
		char *buffer = NULL;
		
		/* The size of the buffer */
		if (!_osync_queue_read_int(queue, &size, &error))
			goto error;
		
		/* The command */
		if (!_osync_queue_read_int(queue, &cmd, &error))
			goto error;
		
		/* The id */
		if (!_osync_queue_read_long_long_int(queue, &id, &error))
			goto error;
		
		/* The timeout */
		if (!_osync_queue_read_int(queue, &timeout, &error))
			goto error;
		
		message = osync_message_new(cmd, size, &error);
		if (!message)
			goto error;
	
		osync_message_set_id(message, id);
		osync_message_set_timeout(message, (unsigned int) timeout);
		
		/* We now get the buffer from the message which will already
		 * have the correct size for the read */
		if (!osync_message_get_buffer(message, &buffer, NULL, &error))
			goto error_free_message;

		if (size) {
			int read = 0;
			do {
				int inc = _osync_queue_read_data(queue, buffer + read, size - read, &error);
				
				if (inc < 0)
					goto error_free_message;
				
				if (inc == 0) {
					osync_error_set(&error, OSYNC_ERROR_IO_ERROR, "Encountered EOF while data was missing");
					goto error_free_message;
				}
				
				read += inc;
			} while (read < size);
		}

		if (!osync_message_set_message_size(message, size, &error))
			goto error_free_message;
		
		g_async_queue_push(queue->incoming, message);
		
		if (queue->incomingContext)
			g_main_context_wakeup(queue->incomingContext);
	} while (_source_check(queue->read_source));
	
	return TRUE;

 error_free_message:
	osync_message_unref(message);
 error:
	if (error) {
		message = osync_message_new_queue_error(error, NULL);
		if (message)
			g_async_queue_push(queue->incoming, message);
		
		
		osync_error_unref(&error);
	}
	
	return FALSE;
}


static void _osync_queue_flush_messages(GAsyncQueue *queue)
{
	OSyncMessage *message;
	osync_assert(queue);

	g_async_queue_lock(queue);

	while ((message = g_async_queue_try_pop_unlocked(queue)))
		osync_message_unref(message);

	g_async_queue_unlock(queue);
}

OSyncQueue *osync_queue_new(const char *name, OSyncError **error)
{
	OSyncQueue *queue = NULL;
	osync_trace(TRACE_ENTRY, "%s(%s, %p)", __func__, name, error);
	
	queue = osync_try_malloc0(sizeof(OSyncQueue), error);
	if (!queue)
		goto error;
	
	if (name)
		queue->name = osync_strdup(name);
	queue->fd = -1;
	
	if (!g_thread_supported ())
		g_thread_init (NULL);
	
	queue->pendingLock = g_mutex_new();
	
	queue->context = g_main_context_new();
	
	queue->outgoing = g_async_queue_new();
	queue->incoming = g_async_queue_new();

	queue->disconnectLock = g_mutex_new();

	queue->ref_count = 1;

	queue->usethreadcom = FALSE;
	queue->connected_queue = NULL;
	queue->connection_closing = FALSE;

	osync_trace(TRACE_EXIT, "%s: %p", __func__, queue);
	return queue;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

OSyncQueue *osync_queue_new_from_fd(int fd, OSyncError **error)
{
	OSyncQueue *queue = NULL;
	osync_trace(TRACE_ENTRY, "%s(%i, %p)", __func__, fd, error);
	
	queue = osync_queue_new(NULL, error);
	if (!queue)
		goto error;
	
	queue->fd = fd;

	osync_trace(TRACE_EXIT, "%s: %p", __func__, queue);
	return queue;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

osync_bool osync_queue_new_threadcom(OSyncQueue **read_queue, OSyncQueue **write_queue, OSyncError **error){

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, read_queue, write_queue, error);
	*read_queue = osync_queue_new(NULL, error);
	if (!*read_queue)
		goto error;
	*write_queue = osync_queue_new(NULL, error);
	if (!*write_queue)
		goto error_free_read_queue;

	(*read_queue)->usethreadcom = TRUE;
	(*write_queue)->usethreadcom = TRUE;
	(*read_queue)->connected_queue = *write_queue;
	(*write_queue)->connected_queue = *read_queue;
       	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_free_read_queue:
	osync_queue_unref(*read_queue);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;

}

osync_bool osync_queue_new_pipes(OSyncQueue **read_queue, OSyncQueue **write_queue, OSyncError **error)
{
#ifdef _WIN32
	return FALSE;
#else
	int filedes[2];
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, read_queue, write_queue, error);
	
	if (pipe(filedes) < 0) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to create pipes");
		goto error;
	}
	
	*read_queue = osync_queue_new_from_fd(filedes[0], error);
	if (!*read_queue)
		goto error_close_pipes;
	
	*write_queue = osync_queue_new_from_fd(filedes[1], error);
	if (!*write_queue)
		goto error_free_read_queue;
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_free_read_queue:
	osync_queue_unref(*read_queue);
 error_close_pipes:
	close(filedes[0]);
	close(filedes[1]);
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
#endif
}

OSyncQueue *osync_queue_ref(OSyncQueue *queue)
{
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, queue);
	osync_assert(queue);

	g_atomic_int_inc(&(queue->ref_count));

	osync_trace(TRACE_EXIT, "%s", __func__);
	return queue;
}

void osync_queue_unref(OSyncQueue *queue)
{
	OSyncPendingMessage *pending = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, queue);
	osync_assert(queue);

	if (g_atomic_int_dec_and_test(&(queue->ref_count))) {
		g_mutex_free(queue->pendingLock);

		g_mutex_free(queue->disconnectLock);

		g_main_context_unref(queue->context);

		_osync_queue_stop_incoming(queue);

		_osync_queue_flush_messages(queue->incoming);
		g_async_queue_unref(queue->incoming);

		_osync_queue_flush_messages(queue->outgoing);
		g_async_queue_unref(queue->outgoing);

		while (queue->pendingReplies) {
			pending = queue->pendingReplies->data;

			queue->pendingReplies = osync_list_remove(queue->pendingReplies, pending);
			queue->pendingCount--;

			/** @todo Refcounting for OSyncPendingMessage */
			if (pending->timeout_info)
				g_free(pending->timeout_info);

			osync_free(pending);
		}

		osync_assert(queue->pendingCount == 0);

		if (queue->name)
			osync_free(queue->name);

		if (queue->connected_queue)
			queue->connected_queue = NULL;
		
		osync_free(queue);
		queue = NULL;
	}
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

osync_bool osync_queue_exists(OSyncQueue *queue)
{
	osync_assert(queue);

	if (!queue->name)
		return FALSE;

	return g_file_test(queue->name, G_FILE_TEST_EXISTS) ? TRUE : FALSE;
}

osync_bool osync_queue_create(OSyncQueue *queue, OSyncError **error)
{
#ifdef _WIN32
	return FALSE;
#else //_WIN32
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, queue, error);
	
	if (mkfifo(queue->name, 0600) != 0) {
		if (errno != EEXIST) {
			osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to create fifo");
			osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
			return FALSE;
		}
	}
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
#endif //_WIN32
}

osync_bool osync_queue_remove(OSyncQueue *queue, OSyncError **error)
{
#ifdef _WIN32
	return FALSE;
#else //_WIN32
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, queue, error);

	if (queue->name && unlink(queue->name) != 0) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to remove queue");
		osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
		return FALSE;
	}
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
#endif
}

osync_bool osync_queue_connect(OSyncQueue *queue, OSyncQueueType type, OSyncError **error)
{
	OSyncQueue **queueptr = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %i, %p)", __func__, queue, type, error);
	osync_assert(queue);
	osync_assert(queue->connected == FALSE);
	
	queue->type = type;

	if (!queue->usethreadcom){
#ifdef _WIN32	
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Windows does not support pipes");
		goto error;
#else
		if (queue->fd == -1) {
			/* First, open the queue with the flags provided by the user */
			int fd = open(queue->name, type == OSYNC_QUEUE_SENDER ? O_WRONLY : O_RDONLY);
			if (fd == -1) {
				osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to open fifo");
				goto error;
			}
			queue->fd = fd;
		}

		int oldflags = fcntl(queue->fd, F_GETFD);
		if (oldflags == -1) {
			osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to get fifo flags");
			goto error_close;
		}
		if (fcntl(queue->fd, F_SETFD, oldflags|FD_CLOEXEC) == -1) {
			osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to set fifo flags");
			goto error_close;
		}
#endif /*_WIN32*/
	}
	queue->connected = TRUE;
	queue->connection_closing = FALSE;
#ifndef _WIN32
	signal(SIGPIPE, SIG_IGN);
#endif
	
	/* now we start a thread which handles reading/writing of the queue */
	queue->thread = osync_thread_new(queue->context, error);

	if (!queue->thread)
		goto error_close;
	
	queue->write_functions = osync_try_malloc0(sizeof(GSourceFuncs), error);
	if (!queue->write_functions)
		goto error_close;
	queue->write_functions->prepare = _queue_prepare;
	queue->write_functions->check = _queue_check;
	queue->write_functions->dispatch = _queue_dispatch;
	queue->write_functions->finalize = NULL;

	queue->write_source = g_source_new(queue->write_functions, sizeof(GSource) + sizeof(OSyncQueue *));
	queueptr = (OSyncQueue **)(queue->write_source + 1);
	*queueptr = queue;
	g_source_set_callback(queue->write_source, NULL, queue, NULL);
	g_source_attach(queue->write_source, queue->context);
	if (queue->context)
		g_main_context_ref(queue->context);

	queue->read_functions = osync_try_malloc0(sizeof(GSourceFuncs), error);
	if (!queue->read_functions)
		goto error_close;
	queue->read_functions->prepare = _source_prepare;
	queue->read_functions->check = _source_check;
	queue->read_functions->dispatch = _source_dispatch;
	queue->read_functions->finalize = NULL;

	queue->read_source = g_source_new(queue->read_functions, sizeof(GSource) + sizeof(OSyncQueue *));
	queueptr = (OSyncQueue **)(queue->read_source + 1);
	*queueptr = queue;
	g_source_set_callback(queue->read_source, NULL, queue, NULL);
	g_source_attach(queue->read_source, queue->context);
	if (queue->context)
		g_main_context_ref(queue->context);

	queue->timeout_functions = osync_try_malloc0(sizeof(GSourceFuncs), error);
	if (!queue->timeout_functions)
		goto error_close;
	queue->timeout_functions->prepare = _timeout_prepare;
	queue->timeout_functions->check = _timeout_check;
	queue->timeout_functions->dispatch = _timeout_dispatch;
	queue->timeout_functions->finalize = NULL;

	queue->timeout_source = g_source_new(queue->timeout_functions, sizeof(GSource) + sizeof(OSyncQueue *));
	queueptr = (OSyncQueue **)(queue->timeout_source + 1);
	*queueptr = queue;
	g_source_set_callback(queue->timeout_source, NULL, queue, NULL);
	g_source_attach(queue->timeout_source, queue->context);
	if (queue->context)
		g_main_context_ref(queue->context);
	
	osync_thread_start(queue->thread);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error_close:
#ifndef _WIN32
	if (!queue->usethreadcom)
		close(queue->fd);
#endif
 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_queue_disconnect(OSyncQueue *queue, OSyncError **error)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, queue, error);
	osync_assert(queue);

	/* Before doing the real disconnect, we empty the pending queue by creating HUP
	   errors for anything on it. In order to make sure it empties, the disconnect 
	   is marked as in progress so that no more entries get put on it (e.g. if a 
	   callback tries to send another message). */

	g_mutex_lock(queue->pendingLock);

	queue->disc_in_progress = TRUE;

	while (queue->pendingCount > 0) {
		OSyncPendingMessage *pending = queue->pendingReplies->data;
		OSyncError *error = NULL;
		OSyncError *huperr = NULL;
		OSyncMessage *errormsg = NULL;
		
		queue->pendingReplies = osync_list_remove(queue->pendingReplies, pending);
		queue->pendingCount--;

		/* Call the callback of the pending message */
		if (pending->callback) {
			osync_error_set(&huperr, OSYNC_ERROR_IO_ERROR, "Disconnect.");
			errormsg = osync_message_new_errorreply(NULL, huperr, &error);
			osync_error_unref(&huperr);
			osync_message_set_id(errormsg, pending->id);
			
			/* Unlock the pending lock during the callback */
			g_mutex_unlock(queue->pendingLock);
			
			osync_trace(TRACE_INTERNAL, "%s: Reporting disconnect error for message %lli", __func__, pending->id);

			pending->callback(errormsg, pending->user_data);
			if (errormsg != NULL)
				osync_message_unref(errormsg);
		
			/* Lock again */
			g_mutex_lock(queue->pendingLock);
		}
		
		// TODO: Refcounting for OSyncPendingMessage
		if (pending->timeout_info)
			g_free(pending->timeout_info);

		osync_free(pending);
	}
	g_mutex_unlock(queue->pendingLock);

	osync_queue_remove_cross_link(queue);

	g_mutex_lock(queue->disconnectLock);
	if (queue->thread) {
		osync_thread_stop(queue->thread);
		osync_thread_unref(queue->thread);
		queue->thread = NULL;
	}
	
	//g_source_unref(queue->write_source);
	
	if (queue->write_functions) {
		osync_free(queue->write_functions);
		queue->write_functions = NULL;
	}
		
	//g_source_unref(queue->read_source);
	
	if (queue->timeout_functions) {
		osync_free(queue->timeout_functions);
		queue->timeout_functions = NULL;
	}
		
	_osync_queue_stop_incoming(queue);
	
	/* We have to empty the incoming queue if we disconnect the queue. Otherwise, the
	 * consumer threads might try to pick up messages even after we are done. */
	_osync_queue_flush_messages(queue->incoming);

	if (queue->usethreadcom){
		queue->connected_queue->connection_closing = TRUE;
	}else{
		if (queue->fd != -1 && close(queue->fd) != 0) {
			osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to close queue");
			osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
			return FALSE;
		}
	}
	
	queue->fd = -1;
	queue->connected = FALSE;
	g_mutex_unlock(queue->disconnectLock);

	g_mutex_lock(queue->pendingLock);
	queue->disc_in_progress = FALSE;
	g_mutex_unlock(queue->pendingLock);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
}

osync_bool osync_queue_is_connected(OSyncQueue *queue)
{
	osync_assert(queue);
	return queue->connected;
}

void osync_queue_set_message_handler(OSyncQueue *queue, OSyncMessageHandler handler, gpointer user_data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, queue, handler, user_data);
	
	queue->message_handler = handler;
	queue->user_data = user_data;
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

void osync_queue_cross_link(OSyncQueue *cmd_queue, OSyncQueue *reply_queue)
{
	/* Cross-linking is needed to make timeouts work when commands are
	   being received on one queue and replies sent on another.
	   Note that cross-linking is not needed when commands are being sent. */
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, cmd_queue, reply_queue);

	osync_assert(cmd_queue->type == OSYNC_QUEUE_RECEIVER);
	osync_assert(reply_queue->type == OSYNC_QUEUE_SENDER);

	cmd_queue->reply_queue = reply_queue;
	osync_queue_ref(reply_queue);
	reply_queue->cmd_queue = cmd_queue;
	osync_queue_ref(cmd_queue);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

void osync_queue_remove_cross_link(OSyncQueue *queue)
{
	/* Remove the cross links from this queue and all queues linked
	   from it, recursively */
	if (queue->cmd_queue) {
		OSyncQueue *linked_queue = queue->cmd_queue;
		queue->cmd_queue = NULL;
		osync_queue_remove_cross_link(linked_queue);
		osync_queue_unref(linked_queue);
	}
	if (queue->reply_queue) {
		OSyncQueue *linked_queue = queue->reply_queue;
		queue->reply_queue = NULL;
		osync_queue_remove_cross_link(linked_queue);
		osync_queue_unref(linked_queue);
	}
}

void osync_queue_set_pending_limit(OSyncQueue *queue, unsigned int limit)
{
	/* The pending limit is used on queues which receive commands and run timeouts.
	   It is used to limit the number of pending transactions so timeouts don't occur
	   just because there are a lot of commands waiting to complete.
	   It is not necessary on other queues and, in fact, MUST NOT be set on
	   queues which receive command replies as it could cause deadlocks. */
	osync_trace(TRACE_ENTRY, "%s(%p, %u)", __func__, queue, limit);

	queue->pendingLimit = limit;
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

osync_bool osync_queue_setup_with_gmainloop(OSyncQueue *queue, GMainContext *context, OSyncError **error)
{
	OSyncQueue **queueptr = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, queue, context);
	
	queue->incoming_functions = osync_try_malloc0(sizeof(GSourceFuncs), error);
	if (!queue->incoming_functions)
		goto error;

	queue->incoming_functions->prepare = _incoming_prepare;
	queue->incoming_functions->check = _incoming_check;
	queue->incoming_functions->dispatch = _incoming_dispatch;
	queue->incoming_functions->finalize = NULL;

	queue->incoming_source = g_source_new(queue->incoming_functions, sizeof(GSource) + sizeof(OSyncQueue *));
	queueptr = (OSyncQueue **)(queue->incoming_source + 1);
	*queueptr = queue;
	g_source_set_callback(queue->incoming_source, NULL, queue, NULL);
	g_source_attach(queue->incoming_source, context);
	queue->incomingContext = context;
	// For the source
	if (context)
		g_main_context_ref(context);
	
	//To unref it later
	if (context)
		g_main_context_ref(context);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__,  osync_error_print(error));
	return FALSE;
}

osync_bool osync_queue_dispatch(OSyncQueue *queue, OSyncError **error)
{
	_incoming_dispatch(NULL, NULL, queue);
	return TRUE;
}

OSyncQueueEvent osync_queue_poll(OSyncQueue *queue)
{
#ifdef _WIN32
	return OSYNC_QUEUE_EVENT_ERROR;
#else //_WIN32
	struct pollfd pfd;
	pfd.fd = queue->fd;
	pfd.events = POLLIN;
	
	/* Here we poll on the queue. If we read on the queue, we either receive a 
	 * POLLIN or POLLHUP. Since we cannot write to the queue, we can block pretty long here.
	 * 
	 * If we are sending, we can only receive a POLLERR which means that the remote side has
	 * disconnected. Since we mainly dispatch the write IO, we dont want to block here. */
	int ret = poll(&pfd, 1, queue->type == OSYNC_QUEUE_SENDER ? 0 : 100);

	if (ret == 0) 
		return OSYNC_QUEUE_EVENT_NONE;	

	/* Ignore interrupts. */
	if	(ret < 0 && errno == EINTR) 
		return OSYNC_QUEUE_EVENT_NONE;

	if (ret < 0 )
		osync_trace(TRACE_ERROR, "queue poll failed - system error :%i %s", errno, strerror(errno));


	if (pfd.revents & POLLERR)
		return OSYNC_QUEUE_EVENT_ERROR;
	else if (pfd.revents & POLLHUP)
		return OSYNC_QUEUE_EVENT_HUP;
	else if (pfd.revents & POLLIN)
		return OSYNC_QUEUE_EVENT_READ;
		
	return OSYNC_QUEUE_EVENT_ERROR;
#endif //_WIN32
}

OSyncMessage *osync_queue_get_message(OSyncQueue *queue)
{
	return g_async_queue_pop(queue->incoming);
}

static long long int opensync_queue_gen_id(const GTimeVal *tv)
{
	long long int now = (tv->tv_sec * 1000000 + tv->tv_usec) << 16;
	long long int rnd = ((long long int)g_random_int()) & 0xFFFF;
		
	return now | rnd;
}

osync_bool osync_queue_send_message(OSyncQueue *queue, OSyncQueue *replyqueue, OSyncMessage *message, OSyncError **error)
{
	return osync_queue_send_message_with_timeout(queue, replyqueue, message, 0, error);
}

osync_bool osync_queue_send_message_with_timeout(OSyncQueue *queue, OSyncQueue *replyqueue, OSyncMessage *message, unsigned int timeout, OSyncError **error)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %u, %p)", __func__, queue, replyqueue, message, timeout, error);

	if (queue->cmd_queue) {
		/* This queue is a reply queue for some command receiving queue */

		/* If this is actually a reply message, we check to see if there is
		   message to remove from the command queue's pending list */

		if (osync_message_get_cmd(message) == OSYNC_MESSAGE_REPLY || osync_message_get_cmd(message) == OSYNC_MESSAGE_ERRORREPLY) {
			/* Remove pending reply but do not call callback
			   (real callback will be called by the receiver of this reply later) */
			_osync_queue_remove_pending_reply(queue->cmd_queue, message, FALSE);
		}
	}

	if (osync_message_get_handler(message)) {
		OSyncPendingMessage *pending = NULL;
		GTimeVal current_time;
		long long int id = 0;
		osync_assert(replyqueue);

		g_mutex_lock(replyqueue->pendingLock);
		if (replyqueue->disc_in_progress) {
			osync_error_set(error, OSYNC_ERROR_IO_ERROR, "Disconnect in progress.");
			goto error;
		}

		pending = osync_try_malloc0(sizeof(OSyncPendingMessage), error);
		if (!pending)
			goto error;

		/* g_sourcet_get_current_time used cached time ... hopefully faster then g_get_.._time() */
		g_source_get_current_time(queue->timeout_source, &current_time);

		id = opensync_queue_gen_id(&current_time);
		osync_message_set_id(message, id);
		pending->id = id;
		osync_trace(TRACE_INTERNAL, "Setting id %lli for pending reply", id);

		if (timeout) {
			/* Send timeout info to other end to handle */
			osync_message_set_timeout(message, timeout);
			/* Note largest timeout seen */
			if (timeout > replyqueue->max_timeout)
				replyqueue->max_timeout = timeout;
		} else {
			osync_trace(TRACE_INTERNAL, "handler message got sent without timeout!: %s", osync_message_get_commandstr(message));
		}
		
		pending->callback = osync_message_get_handler(message);
		pending->user_data = osync_message_get_handler_data(message);
		
		replyqueue->pendingReplies = osync_list_append(replyqueue->pendingReplies, pending);
		if (replyqueue->pendingCount++ == 0) {
			/* Start queue timeout */
			_osync_queue_restart_pending_timeout(replyqueue);
		}
		g_mutex_unlock(replyqueue->pendingLock);
	}
	
	osync_message_ref(message);
	g_async_queue_push(queue->outgoing, message);

	g_main_context_wakeup(queue->context);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;

 error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return FALSE;
}

osync_bool osync_queue_is_alive(OSyncQueue *queue)
{
	OSyncMessage *message = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, queue);
	
	// FIXME
	/*if (!osync_queue_connect(queue, O_WRONLY | O_NONBLOCK, NULL)) {
		osync_trace(TRACE_EXIT_ERROR, "%s: Unable to connect", __func__);
		return FALSE;
		}*/
	
	message = osync_message_new(OSYNC_MESSAGE_NOOP, 0, NULL);
	if (!message) {
		osync_trace(TRACE_EXIT_ERROR, "%s: Unable to create new message", __func__);
		return FALSE;
	}
	
	if (!osync_queue_send_message(queue, NULL, message, NULL)) {
		osync_trace(TRACE_EXIT, "%s: Not alive", __func__);
		return FALSE;
	}
	
	osync_queue_disconnect(queue, NULL);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
}

const char *osync_queue_get_path(OSyncQueue *queue)
{
	osync_assert(queue);
	return queue->name;

}

int osync_queue_get_fd(OSyncQueue *queue)
{
	osync_assert(queue);
	return queue->fd;
}

