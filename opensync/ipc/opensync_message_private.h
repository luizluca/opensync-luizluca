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

#ifndef _OPENSYNC_MESSAGES_PRIVATE_H
#define _OPENSYNC_MESSAGES_PRIVATE_H

/**
 * @defgroup OSyncIPCPrivate OpenSync IPC Module Private
 * @ingroup OSyncPrivate
 * @defgroup OSyncMessageInternalAPI OpenSync Message Internals
 * @ingroup OSyncIPCPrivate
 * @brief A Message used by the inter thread messaging library
 * 
 */

/*@{*/
/*! @brief A OSyncMessage
 * 
 */
struct OSyncMessage {
	gint refCount;
	/** The type of this message */
	OSyncMessageCommand cmd;
	/** The name of the message*/
	osync_messageid id;
	/** Where should the reply be received? */
	OSyncMessageHandler callback;
	/** The user data */
	gpointer user_data;
	/** The timeout associated with this message */
	unsigned int timeout;
	/** If this message has already been answered */
	osync_bool is_answered;
	/** The internal OSyncMarshal object **/
	OSyncMarshal *marshal;
};

/*@}*/

#endif /* _OPENSYNC_MESSAGES_PRIVATE_H */

