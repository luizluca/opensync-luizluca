/*
 * libopensync - A synchronization engine for the opensync framework
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

#ifndef _OPENSYNC_QUEUE_H
#define _OPENSYNC_QUEUE_H

/**
 * @defgroup OSyncQueueAPI OpenSync Queue
 * @ingroup OSyncIPC
 * @brief A Queue used for asynchronous communication between thread
 */

/*@{*/


/** 
 * @brief The type of a queue event 
 */

typedef enum {
	OSYNC_QUEUE_EVENT_NONE,
	OSYNC_QUEUE_EVENT_READ,
	OSYNC_QUEUE_EVENT_ERROR,
	OSYNC_QUEUE_EVENT_HUP
} OSyncQueueEvent;


/** 
 * @brief The queue type 
 */

typedef enum {
	/** Queue Sender */
	OSYNC_QUEUE_SENDER,
	/** Queue Receiver */
	OSYNC_QUEUE_RECEIVER
} OSyncQueueType;


/** 
 * @brief Creates a new asynchronous queue
 * @param name Name of the queue
 * @param error An OpenSync Error
 * 
 * This function return the pointer to a newly created OSyncQueue
 * 
 */
OSYNC_EXPORT OSyncQueue *osync_queue_new(const char *name, OSyncError **error);

/** 
 * @brief Creates a new asynchronous queue
 * @param fd A file descriptor
 * @param error An OpenSync Error
 * 
 * This function return the pointer to a newly created OSyncQueue
 * 
 */
OSYNC_EXPORT OSyncQueue *osync_queue_new_from_fd(int fd, OSyncError **error);

/**
 * @brief Initializes and creates a new FIFO Queue
 * @param queue OpenSync Queue that should be used to create a Queue
 * @param error An OpenSync Error
 * @return TRUE if the queue could be created
 */
OSYNC_EXPORT osync_bool osync_queue_create(OSyncQueue *queue, OSyncError **error);

/** @brief Increase the reference count on an OSyncQueue
 * 
 * Use when storing a reference to the group environment.  When the
 * reference is no longer needed use osync_queue_unref
 * 
 * @param queue Pointer to the queue to reference
 * @return the passed queue
 * 
 */
OSYNC_EXPORT OSyncQueue *osync_queue_ref(OSyncQueue *queue);

/** @brief Decrements the reference count on an OSyncQueue
 * 
 * If the reference count reaches zero then the queue is freed and
 * all resources are freed or unrefed
 * 
 * @param queue Pointer to the queue to unreference
 * 
 */
OSYNC_EXPORT void osync_queue_unref(OSyncQueue *queue);

/**
 * @brief Connects a Queue
 * @param queue Queue that should be connected
 * @param type Type of the Queue (Sender or Receiver)
 * @param error An OpenSync Error
 * @return TRUE if successful
 */
OSYNC_EXPORT osync_bool osync_queue_connect(OSyncQueue *queue, OSyncQueueType type, OSyncError **error);

/**
 * @brief Disconnects a Queue
 * @param queue Queue that should be disconnected
 * @param error An OpenSync Error
 * @return TRUE if successful
 */
OSYNC_EXPORT osync_bool osync_queue_disconnect(OSyncQueue *queue, OSyncError **error);

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
OSYNC_EXPORT void osync_queue_cross_link(OSyncQueue *cmd_queue, OSyncQueue *reply_queue);

/*@}*/
#endif /* _OPENSYNC_QUEUE_H */

