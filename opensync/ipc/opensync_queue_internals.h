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

#ifndef _OPENSYNC_QUEUE_INTERNALS_H
#define _OPENSYNC_QUEUE_INTERNALS_H

/**
 * @defgroup OSyncQueueInternalAPI OpenSync Queue Internals
 * @ingroup OSyncIPCPrivate
 * @brief OpenSync Queue Internal API
 */

/*@{*/

/** 
 * @brief Creates anonymous pipes which don't have to be created and are automatically connected.
 * @param read_queue Queue to read from
 * @param write_queue Queue to write to
 * @param error An OpenSync Error
 * 
 * Lets assume parent wants to send, child wants to receive
 * 
 * osync_queue_new_pipes()
 * fork()
 * 
 * Parent:
 * connect(write_queue)
 * disconnect(read_queue)
 * 
 * Child:
 * connect(read_queue)
 * close(write_queue)
 */
OSYNC_TEST_EXPORT osync_bool osync_queue_new_pipes(OSyncQueue **read_queue, OSyncQueue **write_queue, OSyncError **error);

/**
 * @brief Creates two queues which are connected via thread communication 
 */
OSYNC_TEST_EXPORT osync_bool osync_queue_new_threadcom(OSyncQueue **read_queue, OSyncQueue **write_queue, OSyncError **error);

/**
 * @brief Removes a Queue
 * @param queue OpenSync Queue that should be removed
 * @param error
 * @return TRUE if the queue could be removed
 */
OSYNC_TEST_EXPORT osync_bool osync_queue_remove(OSyncQueue *queue, OSyncError **error);

/**
 * @brief Tests if a queue already exists
 * @param queue Queue that should be queried
 * @return TRUE if the queue exists
 */
osync_bool osync_queue_exists(OSyncQueue *queue);

/**
 * @brief Tests if a queue is connected
 * @param queue Queue to test
 * @return TRUE if queue is connected
 */
OSYNC_TEST_EXPORT osync_bool osync_queue_is_connected(OSyncQueue *queue);

/**
 * @brief Sets the message handler for a queue
 * 
 * Sets the function that will receive all messages, except the methodcall replies
 * 
 * @param queue The queue to set the handler on
 * @param handler The message handler function
 * @param user_data The userdata that the message handler should receive
 * 
 */
OSYNC_TEST_EXPORT void osync_queue_set_message_handler(OSyncQueue *queue, OSyncMessageHandler handler, gpointer user_data);

/**
 * @brief Cross links command queue and reply queue
 * 
 * Stores the queue used for replies in the command queue object so
 * that timeout responses can be sent if necessary.
 * And stores the command queue in the reply queue object so that
 * replies can remove pending messages before they time out.
 * 
 * @param cmd_queue The command queue used to receive incoming commands
 * @param reply_queue The queue used to send replies 
 * 
 */
OSYNC_TEST_EXPORT void osync_queue_cross_link(OSyncQueue *cmd_queue, OSyncQueue *reply_queue);

/**
 * @brief Remove cross links between command queues and reply queues
 * 
 * Removes the cross-links from this queue and all queues linked
 * from it, recursively
 * 
 * @param queue The queue to unlink
 * 
 */
OSYNC_TEST_EXPORT void osync_queue_remove_cross_link(OSyncQueue *queue);

/**
 * @brief Set pending limit on queue
 * 
 * This should be used on queues used to receive incoming commands to
 * limit the number of outstanding commands waiting for replies.
 * This avoids timing out commands which are just waiting for previous
 * commands to finish.
 *
 * Note: the pending limit should not be set on other queues and
 * MUST NOT be set on command reply queues as it could cause deadlocks.
 *
 * The recommended value to use is OSYNC_QUEUE_PENDING_LIMIT.
 * 
 * @param queue The command queue used to receive incoming commands
 * @param limit The maximum number of waiting commands 
 * 
 */
OSYNC_TEST_EXPORT void osync_queue_set_pending_limit(OSyncQueue *queue, unsigned int limit);
#define OSYNC_QUEUE_PENDING_LIMIT 5

/**
 * @brief Sends a Message to a Queue
 * @param queue Pointer to the queue
 * @param replyqueue
 * @param message Message to send
 * @param error An OpenSync Error
 * @return TRUE if successful
 */
OSYNC_TEST_EXPORT osync_bool osync_queue_send_message(OSyncQueue *queue, OSyncQueue *replyqueue, OSyncMessage *message, OSyncError **error);

/**
 * @brief Sends a Message to a Queue or waits for a timeout if the message couldn't be send
 * @param queue Pointer to the queue
 * @param replyqueue
 * @param message Message to send
 * @param timeout The timeout in seconds
 * @param error An OpenSync Error
 * @return TRUE if successful
 */
OSYNC_TEST_EXPORT osync_bool osync_queue_send_message_with_timeout(OSyncQueue *queue, OSyncQueue *replyqueue, OSyncMessage *message, unsigned int timeout, OSyncError **error);

/**
 * @brief Sets the queue to use the gmainloop with the given context
 * 
 * This function will attach the OSyncQueue as a source to the given context.
 * The queue will then be check for new messages and the messages will be
 * handled.
 * 
 * @param queue The queue to set up
 * @param context The context to use. NULL for default loop
 * @param error An OpenSync Error
 * @return TRUE if successful, FALSE otherwise
 * 
 */
OSYNC_TEST_EXPORT osync_bool osync_queue_setup_with_gmainloop(OSyncQueue *queue, GMainContext *context, OSyncError **error);

/**
 * @brief Dispatches incoming data from the remote end
 * @param queue Queue to dispatch
 * @param error An OpenSync Error
 * @return Always TRUE
 */
osync_bool osync_queue_dispatch(OSyncQueue *queue, OSyncError **error);

/**
 * @brief Polls a Queue
 * @param queue Pointer to a queue
 * @return An OSyncQueueEvent
 */
OSyncQueueEvent osync_queue_poll(OSyncQueue *queue);

/**
 * @brief Waits for a message from a queue
 * @param queue A Pointer to a queue
 * @returns The message
 * 
 * NOTE: This function is blocking 
 */
OSYNC_TEST_EXPORT OSyncMessage *osync_queue_get_message(OSyncQueue *queue);

/**
 * @brief Get the path of the fifo for the Queue
 * 
 * Get the full path of the fifo for this Queue if fifos used.
 *
 * @param queue The queue to get the file descriptor
 * @return The full path of the fifo or NULL if no fifos used 
 * 
 */
const char *osync_queue_get_path(OSyncQueue *queue);

/**
 * @brief Get the pipe file descriptor of the Queue
 * 
 * Get the pipe file descriptor of this Queue if pipes are used.
 *
 * @param queue The queue to get the file descriptor
 * @returns The pipe file descriptor of the queue or -1 if no pipes used or set
 * 
 */
int osync_queue_get_fd(OSyncQueue *queue);

/**
 * @brief Queries if a queue is still alive
 * @param queue Pointer to the queue
 * @return TRUE if the queue is alive
 */
osync_bool osync_queue_is_alive(OSyncQueue *queue);

/*@}*/

#endif /* _OPENSYNC_QUEUE_INTERNALS_H */

