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

#ifndef _OPENSYNC_MESSAGES_INTERNALS_H
#define _OPENSYNC_MESSAGES_INTERNALS_H

/**
 * @defgroup OSyncIPC OpenSync IPC Module
 * @ingroup OSyncPublic
 * @defgroup OSyncMessage OpenSync Message
 * @ingroup OSyncIPC
 * @brief A Message used by the inter thread messaging library
 * 
 */

/*@{*/

/** @brief The Type of the message
 * 
 */
typedef enum {
	OSYNC_MESSAGE_NOOP,
	OSYNC_MESSAGE_CONNECT,
	OSYNC_MESSAGE_CONNECT_DONE,
	OSYNC_MESSAGE_DISCONNECT,
	OSYNC_MESSAGE_GET_CHANGES,
	OSYNC_MESSAGE_READ_CHANGE,
	OSYNC_MESSAGE_COMMIT_CHANGE,
	OSYNC_MESSAGE_COMMITTED_ALL,
	OSYNC_MESSAGE_SYNC_DONE,
	OSYNC_MESSAGE_CALL_PLUGIN,
	OSYNC_MESSAGE_NEW_CHANGE,
	OSYNC_MESSAGE_REPLY,
	OSYNC_MESSAGE_ERRORREPLY,
	OSYNC_MESSAGE_INITIALIZE,
	OSYNC_MESSAGE_FINALIZE,
	OSYNC_MESSAGE_DISCOVER,
	OSYNC_MESSAGE_SYNCHRONIZE,
	OSYNC_MESSAGE_ENGINE_CHANGED,
	OSYNC_MESSAGE_MAPPING_CHANGED,
	OSYNC_MESSAGE_MAPPINGENTRY_CHANGED,
	OSYNC_MESSAGE_ERROR,
	OSYNC_MESSAGE_QUEUE_ERROR,
	OSYNC_MESSAGE_QUEUE_HUP
} OSyncMessageCommand;

/** @brief Function which can receive messages
 * 
 * @param message The reply that is being received.
 * @param user_data The userdata which was set previously
 * 
 */
typedef void (*OSyncMessageHandler)(OSyncMessage *message, void *user_data);

/** @brief Creates a new message of the given command
 * 
 * @param cmd The message command 
 * @param size The size of the message
 * @param error Pointer to a error-struct
 * @returns Pointer to a newly allocated message
 * 
 */
OSYNC_TEST_EXPORT OSyncMessage *osync_message_new(OSyncMessageCommand cmd, unsigned int size, OSyncError **error);

/** @brief Creates a new reply
 * 
 * @param message The message to which you wish to reply
 * @param error Pointer to error-struct 
 * @returns Pointer to a newly allocated message
 * 
 */
OSYNC_TEST_EXPORT OSyncMessage *osync_message_new_reply(OSyncMessage *message, OSyncError **error);

/** @brief Creates a new error reply
 * 
 * @param message The message to which you wish to reply
 * @param error Pointer to error object for the error reply
 * @param loc_error Pointer to a error-struct for errors which appear while creating message 
 * @returns Pointer to a newly allocated error-reply message
 */
OSYNC_TEST_EXPORT OSyncMessage *osync_message_new_errorreply(OSyncMessage *message, OSyncError *error, OSyncError **loc_error);

/** @brief Creates a new error message 
 * 
 * @param error Pointer to error object to send error
 * @param loc_error Pointer to a error-struct for errors which appear while creating message 
 * @returns Pointer to a newly allocated error message
 */
OSYNC_TEST_EXPORT OSyncMessage *osync_message_new_error(OSyncError *error, OSyncError **loc_error);

/** @brief Creates a new queue error message 
 * 
 * @param error Pointer to error object to send error
 * @param loc_error Pointer to a error-struct for errors which appear while creating message 
 * @returns Pointer to a newly allocated queue error message
 */
OSYNC_TEST_EXPORT OSyncMessage *osync_message_new_queue_error(OSyncError *error, OSyncError **loc_error);

/** @brief Increase the reference count of the message 
 * 
 * @param message The message
 * @returns The referenced message pointer
 * 
 */
OSYNC_TEST_EXPORT OSyncMessage *osync_message_ref(OSyncMessage *message);

/** @brief Decrease the reference count of the message 
 * 
 * @param message The message 
 * 
 */
OSYNC_TEST_EXPORT void osync_message_unref(OSyncMessage *message);

/** @brief Set new message command for the message object
 * 
 * @param message The message to modify 
 * @param cmd The new message command
 * 
 */
OSYNC_TEST_EXPORT void osync_message_set_cmd(OSyncMessage *message, OSyncMessageCommand cmd);

/** @brief Get message command for the message object
 * 
 * @param message The message
 * 
 */
OSYNC_TEST_EXPORT OSyncMessageCommand osync_message_get_cmd(OSyncMessage *message);

/** @brief Set an ID for the message 
 * 
 * @param message The message
 * @param id The ID which get set for supplied message object 
 * 
 */
OSYNC_TEST_EXPORT void osync_message_set_id(OSyncMessage *message, long long int id);

/** @brief Get message ID of supplied message object
 * 
 * @param message The message
 * @returns The message ID of supplied message
 * 
 */
OSYNC_TEST_EXPORT long long int osync_message_get_id(OSyncMessage *message);

/** @brief Get marshal object of supplied message object
 * 
 * @param message The message
 * @returns Pointer of the marshal object, NULL if no marshal object is
 *          associated with the message 
 *
 */
OSYNC_TEST_EXPORT OSyncMarshal *osync_message_get_marshal(OSyncMessage *message);

/** @brief Get message size of supplied message object
 * 
 * @param message The message
 * @returns The message size of supplied message
 * 
 */
OSYNC_TEST_EXPORT unsigned int osync_message_get_message_size(OSyncMessage *message);

/** @brief Set message size for supplied message object
 * 
 * @param message The message
 * @param size The size of the message to set
 * 
 */
OSYNC_TEST_EXPORT void osync_message_set_message_size(OSyncMessage *message, unsigned int size);

/** @brief Get the buffer/content of the message object
 * 
 * @param message The message
 * @param data Pointer to data 
 * @param size Size of the data
 * 
 */
OSYNC_TEST_EXPORT void osync_message_get_buffer(OSyncMessage *message, char **data, unsigned int *size);

/** @brief Sets the handler that will receive the reply
 * 
 * @param message The message to work on
 * @param handler Which handler should be called when the reply is received
 * @param user_data Which user data should be passed to the handler
 * 
 */
OSYNC_TEST_EXPORT void osync_message_set_handler(OSyncMessage *message, OSyncMessageHandler handler, void *user_data);

/** @brief Get the message handler of the message
 * 
 * @param message The message to work on
 * @returns The message handler of the message
 * 
 */
OSYNC_TEST_EXPORT OSyncMessageHandler osync_message_get_handler(OSyncMessage *message);

/** @brief Get the data which gets passed to the handler function 
 * 
 * @param message The message to work on
 * @returns Pointer of the supplied handler data
 * 
 */
OSYNC_TEST_EXPORT void *osync_message_get_handler_data(OSyncMessage *message);

/** @brief Checks if the message is a error
 * 
 * @param message The message to check
 * @return TRUE if the message is a error, FLASE otherwise
 * 
 */
OSYNC_TEST_EXPORT osync_bool osync_message_is_error(OSyncMessage *message);

/** @brief Gets the command from a message
 * 
 * This function will return the command of a message
 * 
 * @param message The message
 * @return the command
 */
OSYNC_TEST_EXPORT OSyncMessageCommand osync_message_get_command(OSyncMessage *message);

/**
 * @brief Convertes the command of a message to a readable string
 * @param message The message
 * @return The command as a string
 */
OSYNC_TEST_EXPORT char* osync_message_get_commandstr(OSyncMessage *message);

/** @brief Checks if the message got answered 
 * 
 * @param message The message to check
 * @return TRUE if the message got answered, FLASE otherwise
 * 
 */
OSYNC_TEST_EXPORT osync_bool osync_message_is_answered(OSyncMessage *message);

/** @brief Set message as answered 
 * 
 * @param message The message to work on 
 * 
 */
OSYNC_TEST_EXPORT void osync_message_set_answered(OSyncMessage *message);

/** @brief Appends an integer value to serialized message buffer
 * 
 * @param message The message
 * @param value The integer value to append
 */
OSYNC_TEST_EXPORT void osync_message_write_int(OSyncMessage *message, int value);

/** @brief Appends an unsigned integer value to serialized message buffer
 * 
 * @param message The message
 * @param value The integer value to append
 */
OSYNC_TEST_EXPORT void osync_message_write_uint(OSyncMessage *message, unsigned int value);

/** @brief Appends a long long integer value to serialized message buffer
 * 
 * @param message The message
 * @param value The long long integer value to append
 */
OSYNC_TEST_EXPORT void osync_message_write_long_long_int(OSyncMessage *message, long long int value);

/** @brief Appends a string to serialized message buffer
 * 
 * @param message The message
 * @param value The string to append
 */
OSYNC_TEST_EXPORT void osync_message_write_string(OSyncMessage *message, const char *value);

/** @brief Appends data with a specific length to the serialized message buffer
 *
 * This data should be completely serialized. This is only for internal use,
 * since this function doesn't append the size/end of the appended data.
 * 
 * @param message The message
 * @param value The data to append
 * @param size Size of corresponding data parameter
 */
OSYNC_TEST_EXPORT void osync_message_write_data(OSyncMessage *message, const void *value, int size);

/** @brief Appends data with a specific length to the serialized message buffer,
 * plus the length of the data to determine the end.
 *
 * @param message The message
 * @param value The data to append
 * @param size Size of corresponding data parameter
 */
OSYNC_TEST_EXPORT void osync_message_write_buffer(OSyncMessage *message, const void *value, int size);

/** @brief Read serialized integer from message buffer. This increments the read
 * position of the message buffer.
 *
 * @param message The message
 * @param value Reference to store the integer value 
 */
OSYNC_TEST_EXPORT void osync_message_read_int(OSyncMessage *message, int *value);

/** @brief Read serialized unsigned integer from message buffer. This increments the read
 * position of the message buffer.
 *
 * @param message The message
 * @param value Reference to store the integer value 
 */
OSYNC_TEST_EXPORT void osync_message_read_uint(OSyncMessage *message, unsigned int *value);

/** @brief Read serialized long long integer from message buffer. This increments the read
 * position of the message buffer.
 *
 * @param message The message
 * @param value Reference to store the long long integer value 
 */
OSYNC_TEST_EXPORT void osync_message_read_long_long_int(OSyncMessage *message, long long int *value);

/** @brief Read serialized string from message buffer. This increments the read
 * position of the message buffer. Caller is responsible for freeing the duplicated
 * string.
 *
 * @param message The message
 * @param value Reference to store the pointer to the newly allocated string 
 */
OSYNC_TEST_EXPORT void osync_message_read_string(OSyncMessage *message, char **value);

/** @brief Read specific size of serialized data from message buffer. This increments 
 * the read position of the message buffer. Caller is responsible for freeing the 
 * duplicate data.
 *
 * @param message The message
 * @param value Reference to store the pointer to the newly allocated data 
 * @param size Size of data
 */
OSYNC_TEST_EXPORT void osync_message_read_data(OSyncMessage *message, void *value, int size);

/** @brief Read serialized const data from message buffer. This increments the read
 * position of the message buffer.
 *
 * @param message The message
 * @param value Reference to store the data pointer 
 * @param size The size of data
 */
OSYNC_TEST_EXPORT void osync_message_read_const_data(OSyncMessage *message, void **value, int size);

/** @brief Read serialized const string from message buffer. This increments the read
 * position of the message buffer.
 *
 * @param message The message
 * @param value Reference to store the string pointer 
 */
OSYNC_TEST_EXPORT void osync_message_read_const_string(OSyncMessage *message, char **value);

/** @brief Read serialized data from message buffer. This increments the read
 * position of the message buffer. Caller is responsible for freeing the duplicated
 * data.
 *
 * @param message The message
 * @param value Reference to store the pointer to the newly allocated data 
 * @param size Size of data
 */
OSYNC_TEST_EXPORT void osync_message_read_buffer(OSyncMessage *message, void **value, int *size);

/*@}*/

#endif /* _OPENSYNC_MESSAGES_INTERNALS_H */

