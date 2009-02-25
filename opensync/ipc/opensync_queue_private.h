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

#ifndef _OPENSYNC_QUEUE_PRIVATE_H
#define _OPENSYNC_QUEUE_PRIVATE_H

#include <fcntl.h>
#ifndef _WIN32
#include <sys/poll.h>
#include <sys/time.h>
#endif //_WIN32

#include <signal.h>

/**
 * @defgroup OSyncQueuePrivateAPI OpenSync Queue Private
 * @ingroup OSyncIPCPrivate
 * @brief A Queue used for asynchronous communication between thread
 */

/*@{*/

/*! @brief Represents a Queue which can be used to receive messages
 */
struct OSyncQueue {
	/** The queue type **/
	OSyncQueueType type;

	/** The real asynchronous queue from glib **/
	int fd;

	/** The path name of this queue **/
	char *name;

	/** The message handler for this queue **/
	OSyncMessageHandler message_handler;

	/** The user_data associated with this queue **/
	gpointer user_data;

	/** The source associated with this queue */
	GSourceFuncs *incoming_functions;
	GSource *incoming_source;

	/** The context in which the IO of the queue is dispatched */
	GMainContext *context;
	GMainContext *incomingContext;
	
	OSyncThread *thread;
	
	GAsyncQueue *incoming;
	GAsyncQueue *outgoing;
	
	/** List of pending replies */
	OSyncList *pendingReplies;
	GMutex *pendingLock;
	unsigned int pendingCount, pendingLimit;
	
	GSourceFuncs *write_functions;
	GSource *write_source;
	
	GSourceFuncs *read_functions;
	GSource *read_source;

	/** Timeout Source **/
	GSourceFuncs *timeout_functions;
	GSource *timeout_source;
	
	GMutex *disconnectLock;

	/** Connection status **/
	osync_bool connected;

	/** Reference count **/
	int ref_count;

	/* Queue for replies to incoming messages */
	OSyncQueue *reply_queue;

	/* Queue for receiving commands we are replying to */
	OSyncQueue *cmd_queue;

	/* Disconnect in progress */
	gboolean disc_in_progress;

	/* Largest timeout value seen so far */
	unsigned int max_timeout;

	/* Expiration time of pending queue timeout */
	GTimeVal pending_timeout;

	/* Threaded communication */
	gboolean usethreadcom;
	/* Connected queue */
	OSyncQueue *connected_queue;

	/* Indicate that the connection is closing for threaded com */
	gboolean connection_closing;
};

/** @brief Pending queue timeout addition for ipc delay
 */
#define OSYNC_QUEUE_PENDING_QUEUE_IPC_DELAY 1

/** @brief Pending queue minimum timeout
 */
#define OSYNC_QUEUE_PENDING_QUEUE_MIN_TIMEOUT 20

/** @brief Timeout object 
 */
typedef struct OSyncTimeoutInfo {
	/** Expiration date */
	GTimeVal expiration; 
} OSyncTimeoutInfo;

/** @brief Pending Message object 
 */
typedef struct OSyncPendingMessage {
	/** ID of the expected Message */
	long long int id;
	/** Where should the reply be received? */
	OSyncMessageHandler callback;
	/** The user data */
	gpointer user_data;
	/** Message Timeout */
	OSyncTimeoutInfo *timeout_info;
} OSyncPendingMessage;

/**
 * @brief Generate IDs
 * Ids are generated as follows:
 * 
 * the upper 6 bytes are the time in seconds and microseconds
 * the lower 2 bytes are a random number
 * 
 * */
static long long int opensync_queue_gen_id(const GTimeVal *tv);

/*@}*/

#endif /* _OPENSYNC_QUEUE_PRIVATE_H */

