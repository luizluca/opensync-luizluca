#include "support.h"
#ifndef _WIN32
#include <sys/wait.h>
#endif

#include <opensync/opensync-ipc.h>
#include "opensync/ipc/opensync_message_internals.h"
#include "opensync/ipc/opensync_queue_internals.h"

START_TEST (ipc_new)
{
	char *testbed = setup_testbed(NULL);
	osync_testing_file_remove("/tmp/testpipe");
	
	OSyncError *error = NULL;
	OSyncQueue *queue1 = osync_queue_new("/tmp/testpipe", &error);
	fail_unless(queue1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_queue_unref(queue1);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (ipc_ref)
{
	char *testbed = setup_testbed(NULL);
	osync_testing_file_remove("/tmp/testpipe");
	
	OSyncError *error = NULL;
	OSyncQueue *queue1 = osync_queue_new("/tmp/testpipe", &error);
	fail_unless(queue1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	queue1 = osync_queue_ref(queue1);
	fail_unless(queue1 != NULL, NULL);

	osync_queue_unref(queue1);
	osync_queue_unref(queue1);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (ipc_create)
{
	char *testbed = setup_testbed(NULL);
	osync_testing_file_remove("/tmp/testpipe");
	
	OSyncError *error = NULL;
	OSyncQueue *queue1 = osync_queue_new("/tmp/testpipe", &error);
	fail_unless(queue1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_queue_create(queue1, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe") == TRUE, NULL);
	
	fail_unless(osync_queue_remove(queue1, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe") == FALSE, NULL);
	
	osync_queue_unref(queue1);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (ipc_connect)
{
	char *testbed = setup_testbed(NULL);
	osync_testing_file_remove("/tmp/testpipe");
	
	OSyncError *error = NULL;
	OSyncQueue *queue = osync_queue_new("/tmp/testpipe", &error);
	
	osync_queue_create(queue, &error);
	fail_unless(error == NULL, NULL);
		
	pid_t cpid = fork();
	if (cpid == 0) { //Child
		osync_assert(osync_queue_connect(queue, OSYNC_QUEUE_RECEIVER, &error));
		osync_assert(error == NULL);
		
		if (osync_queue_disconnect(queue, &error) != TRUE || error != NULL)
			exit(1);
		
		osync_queue_unref(queue);
	
		g_free(testbed);
		exit(0);
	} else {
		fail_unless(osync_queue_connect(queue, OSYNC_QUEUE_SENDER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		osync_queue_disconnect(queue, &error);
		fail_unless(error == NULL, NULL);
		
		int status = 0;
		wait(&status);
		fail_unless(WEXITSTATUS(status) == 0, NULL);
		
	}
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe") == TRUE, NULL);
		
	fail_unless(osync_queue_remove(queue, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe") == FALSE, NULL);

	osync_queue_unref(queue);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (ipc_payload)
{	
	char *testbed = setup_testbed(NULL);
	osync_testing_file_remove("/tmp/testpipe-server");
	osync_testing_file_remove("/tmp/testpipe-client");
	
	OSyncError *error = NULL;
	OSyncQueue *server_queue = osync_queue_new("/tmp/testpipe-server", &error);
	OSyncQueue *client_queue = osync_queue_new("/tmp/testpipe-client", &error);
	OSyncMessage *message = NULL;
	
	osync_queue_create(server_queue, &error);
	fail_unless(error == NULL, NULL);
	
	osync_queue_create(client_queue, &error);
	fail_unless(error == NULL, NULL);
	char *data = "this is another test string";
	
	pid_t cpid = fork();
	if (cpid == 0) { //Child
		osync_assert(osync_queue_connect(client_queue, OSYNC_QUEUE_RECEIVER, &error));
		osync_assert(error == NULL);

		osync_assert(osync_queue_connect(server_queue, OSYNC_QUEUE_SENDER, &error));
		osync_assert(error == NULL);
		
		message = osync_queue_get_message(client_queue);
		
		if (osync_message_get_command(message) != OSYNC_MESSAGE_INITIALIZE) {
			exit (1);
		}
		
		int int1;
		long long int longint1;
		char *string;
		void *databuf;
		
		osync_message_read_int(message, &int1);
		osync_message_read_const_string(message, &string);
		osync_message_read_long_long_int(message, &longint1);
		osync_message_read_const_data(message, &databuf, strlen(data) + 1);
		
		osync_assert(int1 == 4000000);
		osync_assert(!strcmp(string, "this is a test string"));
		osync_assert(longint1 == 400000000);
		osync_assert(!strcmp(databuf, "this is another test string"));
		
		OSyncMessage *reply = osync_message_new_reply(message, &error);
		
		osync_message_unref(message);
		
		if (osync_queue_disconnect(client_queue, &error) != TRUE || error != NULL)
			exit(1);
		osync_queue_unref(client_queue);
		
		osync_queue_send_message(server_queue, NULL, reply, &error);
		osync_message_unref(reply);
		
		message = osync_queue_get_message(server_queue);
		
		if (osync_message_get_command(message) != OSYNC_MESSAGE_QUEUE_HUP) {
			exit (1);
		}
		
		osync_message_unref(message);
	
		if (osync_queue_disconnect(server_queue, &error) != TRUE || error != NULL)
			exit(1);
		osync_queue_unref(server_queue);
		
		g_free(testbed);
		
		exit(0);
	} else {
		fail_unless(osync_queue_connect(client_queue, OSYNC_QUEUE_SENDER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		fail_unless(osync_queue_connect(server_queue, OSYNC_QUEUE_RECEIVER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		message = osync_message_new(OSYNC_MESSAGE_INITIALIZE, 0, &error);
		fail_unless(message != NULL, NULL);
		fail_unless(!osync_error_is_set(&error), NULL);
		
		osync_message_write_int(message, 4000000);
		osync_message_write_string(message, "this is a test string");
		osync_message_write_long_long_int(message, 400000000);
		osync_message_write_data(message, data, strlen(data) + 1);
		
		fail_unless(osync_queue_send_message(client_queue, NULL, message, &error), NULL);
		fail_unless(!osync_error_is_set(&error), NULL);
		
		osync_message_unref(message);
		
		while (!(message = osync_queue_get_message(server_queue))) {
			g_usleep(100000);
		}
		
		fail_unless(osync_message_get_command(message) == OSYNC_MESSAGE_REPLY);
		
		osync_message_unref(message);
		
		osync_queue_disconnect(server_queue, &error);
		fail_unless(error == NULL, NULL);
		
		while (!(message = osync_queue_get_message(client_queue))) {
			g_usleep(10000);
		}
		
		if (osync_message_get_command(message) != OSYNC_MESSAGE_QUEUE_HUP) {
			exit (1);
		}
		
		osync_message_unref(message);
		
		osync_queue_disconnect(client_queue, &error);
		fail_unless(error == NULL, NULL);
		
		int status = 0;
		wait(&status);
		fail_unless(WEXITSTATUS(status) == 0, NULL);
	}
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == TRUE, NULL);
	
	fail_unless(osync_queue_remove(client_queue, &error), NULL);
	fail_unless(osync_queue_remove(server_queue, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == FALSE, NULL);

	osync_queue_unref(client_queue);
	osync_queue_unref(server_queue);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (ipc_payload_wait)
{	
	char *testbed = setup_testbed(NULL);
	osync_testing_file_remove("/tmp/testpipe-server");
	osync_testing_file_remove("/tmp/testpipe-client");
	
	OSyncError *error = NULL;
	OSyncQueue *server_queue = osync_queue_new("/tmp/testpipe-server", &error);
	OSyncQueue *client_queue = osync_queue_new("/tmp/testpipe-client", &error);
	OSyncMessage *message = NULL;
	
	osync_queue_create(server_queue, &error);
	fail_unless(error == NULL, NULL);
	
	osync_queue_create(client_queue, &error);
	fail_unless(error == NULL, NULL);
	char *data = "this is another test string";
	
	pid_t cpid = fork();
	if (cpid == 0) { //Child
		g_usleep(1*G_USEC_PER_SEC);
		osync_assert(osync_queue_connect(client_queue, OSYNC_QUEUE_RECEIVER, &error));
		osync_assert(error == NULL);

		osync_assert(osync_queue_connect(server_queue, OSYNC_QUEUE_SENDER, &error));
		osync_assert(error == NULL);
		
		while (!(message = osync_queue_get_message(client_queue))) {
			g_usleep(10000);
		}
		
		if (osync_message_get_command(message) != OSYNC_MESSAGE_INITIALIZE) {
			exit (1);
		}
		
		int int1;
		long long int longint1;
		char *string;
		char databuf[strlen(data) + 1];
			
		osync_message_read_int(message, &int1);
		osync_message_read_string(message, &string);
		osync_message_read_long_long_int(message, &longint1);
		osync_message_read_data(message, databuf, strlen(data) + 1);
		
		osync_assert(int1 == 4000000);
		osync_assert(!strcmp(string, "this is a test string"));
		osync_assert(longint1 == 400000000);
		osync_assert(!strcmp(databuf, "this is another test string"));
		
		g_usleep(1*G_USEC_PER_SEC);
		
		OSyncMessage *reply = osync_message_new_reply(message, &error);
		
		osync_message_unref(message);
		
		osync_queue_send_message(server_queue, NULL, reply, &error);
		
		osync_message_unref(reply);
		
		g_usleep(1*G_USEC_PER_SEC);
		
		if (osync_queue_disconnect(client_queue, &error) != TRUE || error != NULL)
			exit(1);
		osync_queue_unref(client_queue);
	
		while (!(message = osync_queue_get_message(server_queue))) {
			g_usleep(10000);
		}
		
		if (osync_message_get_command(message) != OSYNC_MESSAGE_QUEUE_HUP) {
			exit (1);
		}
	
		osync_message_unref(message);
		g_usleep(1*G_USEC_PER_SEC);
		
		if (osync_queue_disconnect(server_queue, &error) != TRUE || error != NULL)
			exit(1);
		osync_queue_unref(server_queue);
		
		g_free(testbed);
		
		exit(0);
	} else {
		fail_unless(osync_queue_connect(client_queue, OSYNC_QUEUE_SENDER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		fail_unless(osync_queue_connect(server_queue, OSYNC_QUEUE_RECEIVER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		message = osync_message_new(OSYNC_MESSAGE_INITIALIZE, 0, &error);
		fail_unless(message != NULL, NULL);
		fail_unless(!osync_error_is_set(&error), NULL);
		
		osync_message_write_int(message, 4000000);
		osync_message_write_string(message, "this is a test string");
		osync_message_write_long_long_int(message, 400000000);
		osync_message_write_data(message, data, strlen(data) + 1);
		
		fail_unless(osync_queue_send_message(client_queue, NULL, message, &error), NULL);
		fail_unless(!osync_error_is_set(&error), NULL);
		
		osync_message_unref(message);
		
		while (!(message = osync_queue_get_message(server_queue))) {
			g_usleep(100000);
		}
		
		fail_unless(osync_message_get_command(message) == OSYNC_MESSAGE_REPLY);
		
		osync_message_unref(message);
		
		osync_queue_disconnect(server_queue, &error);
		fail_unless(error == NULL, NULL);
		
		while (!(message = osync_queue_get_message(client_queue))) {
			g_usleep(10000);
		}
		
		if (osync_message_get_command(message) != OSYNC_MESSAGE_QUEUE_HUP) {
			exit (1);
		}
		osync_message_unref(message);
		
		osync_queue_disconnect(client_queue, &error);
		fail_unless(error == NULL, NULL);
		
		int status = 0;
		wait(&status);
		fail_unless(WEXITSTATUS(status) == 0, NULL);
	}
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == TRUE, NULL);
	
	fail_unless(osync_queue_remove(client_queue, &error), NULL);
	fail_unless(osync_queue_remove(server_queue, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == FALSE, NULL);

	osync_queue_unref(client_queue);
	osync_queue_unref(server_queue);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (ipc_payload_stress)
{	
	char *testbed = setup_testbed(NULL);
	osync_testing_file_remove("/tmp/testpipe-server");
	osync_testing_file_remove("/tmp/testpipe-client");
	
	int num_mess = 1000;
	int size = 100;
	
	char *data = malloc(size);
	memset(data, 42, size);
	
	OSyncError *error = NULL;
	OSyncQueue *server_queue = osync_queue_new("/tmp/testpipe-server", &error);
	OSyncQueue *client_queue = osync_queue_new("/tmp/testpipe-client", &error);
	OSyncMessage *message = NULL;
	
	osync_queue_create(server_queue, &error);
	fail_unless(error == NULL, NULL);
	
	osync_queue_create(client_queue, &error);
	fail_unless(error == NULL, NULL);
	
	pid_t cpid = fork();
	if (cpid == 0) { //Child
		osync_assert(osync_queue_connect(client_queue, OSYNC_QUEUE_RECEIVER, &error));
		osync_assert(error == NULL);

		osync_assert(osync_queue_connect(server_queue, OSYNC_QUEUE_SENDER, &error));
		osync_assert(error == NULL);
		
		while (num_mess > 0) {
			osync_trace(TRACE_INTERNAL, "Waiting for message");
			message = osync_queue_get_message(client_queue);
			
			if (osync_message_get_command(message) != OSYNC_MESSAGE_INITIALIZE) {
				exit (1);
			}
			
			osync_trace(TRACE_INTERNAL, "Parsing message");
			char databuf[size];
				
			osync_message_read_data(message, databuf, size);
			
			osync_assert(!memcmp(databuf, data, size));
			
			osync_trace(TRACE_INTERNAL, "Creating new reply");
			OSyncMessage *reply = osync_message_new_reply(message, &error);
			
			osync_message_unref(message);
			
			osync_trace(TRACE_INTERNAL, "Sending reply");
			osync_queue_send_message(server_queue, NULL, reply, &error);
			
			osync_message_unref(reply);
			
			num_mess--;
		}
		
		if (osync_queue_disconnect(client_queue, &error) != TRUE || error != NULL)
			exit(1);
		osync_queue_unref(client_queue);
	
		message = osync_queue_get_message(server_queue);
		
		if (osync_message_get_command(message) != OSYNC_MESSAGE_QUEUE_HUP) {
			exit (1);
		}
	
		osync_message_unref(message);
			
		if (osync_queue_disconnect(server_queue, &error) != TRUE || error != NULL)
			exit(1);
		osync_queue_unref(server_queue);
		
		g_free(data);
		g_free(testbed);
		
		exit(0);
	} else {
		fail_unless(osync_queue_connect(client_queue, OSYNC_QUEUE_SENDER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		fail_unless(osync_queue_connect(server_queue, OSYNC_QUEUE_RECEIVER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		while (num_mess > 0) {
			osync_trace(TRACE_INTERNAL, "Creating new message");
			message = osync_message_new(OSYNC_MESSAGE_INITIALIZE, 0, &error);
			fail_unless(message != NULL, NULL);
			fail_unless(!osync_error_is_set(&error), NULL);
			
			osync_message_write_data(message, data, size);
			
			osync_trace(TRACE_INTERNAL, "Sending message");
			fail_unless(osync_queue_send_message(client_queue, NULL, message, &error), NULL);
			fail_unless(!osync_error_is_set(&error), NULL);
			
			osync_message_unref(message);
			
			osync_trace(TRACE_INTERNAL, "Waiting for message");
			message = osync_queue_get_message(server_queue);
			
			fail_unless(osync_message_get_command(message) == OSYNC_MESSAGE_REPLY);
			
			osync_message_unref(message);
			
			num_mess--;
		}
		
		osync_queue_disconnect(server_queue, &error);
		fail_unless(error == NULL, NULL);
		
		message = osync_queue_get_message(client_queue);
		
		if (osync_message_get_command(message) != OSYNC_MESSAGE_QUEUE_HUP) {
			exit (1);
		}
		
		osync_message_unref(message);
			
		osync_queue_disconnect(client_queue, &error);
		fail_unless(error == NULL, NULL);
		
		int status = 0;
		wait(&status);
		fail_unless(WEXITSTATUS(status) == 0, NULL);
	}
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == TRUE, NULL);
	
	fail_unless(osync_queue_remove(client_queue, &error), NULL);
	fail_unless(osync_queue_remove(server_queue, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == FALSE, NULL);

	osync_queue_unref(client_queue);
	osync_queue_unref(server_queue);
	
	g_free(data);
	destroy_testbed(testbed);
}
END_TEST

START_TEST (ipc_payload_stress2)
{	
	char *testbed = setup_testbed(NULL);
	osync_testing_file_remove("/tmp/testpipe-server");
	osync_testing_file_remove("/tmp/testpipe-client");
	int i = 0;
	
	int num_mess = 1000;
	int size = 100;
	
	char *data = malloc(size);
	memset(data, 42, size);
	
	OSyncError *error = NULL;
	OSyncQueue *server_queue = osync_queue_new("/tmp/testpipe-server", &error);
	OSyncQueue *client_queue = osync_queue_new("/tmp/testpipe-client", &error);
	OSyncMessage *message = NULL;
	
	osync_queue_create(server_queue, &error);
	fail_unless(error == NULL, NULL);
	
	osync_queue_create(client_queue, &error);
	fail_unless(error == NULL, NULL);
	
	pid_t cpid = fork();
	if (cpid == 0) { //Child
		osync_assert(osync_queue_connect(client_queue, OSYNC_QUEUE_RECEIVER, &error));
		osync_assert(error == NULL);

		osync_assert(osync_queue_connect(server_queue, OSYNC_QUEUE_SENDER, &error));
		osync_assert(error == NULL);
		
		for (i = 0; i < num_mess; i++) {
			message = osync_queue_get_message(client_queue);
			
			if (osync_message_get_command(message) != OSYNC_MESSAGE_INITIALIZE) {
				exit (1);
			}
			
			char databuf[size];
				
			osync_message_read_data(message, databuf, size);
			
			osync_assert(!memcmp(databuf, data, size));
			
			osync_message_unref(message);
		}
		
		message = osync_message_new(OSYNC_MESSAGE_INITIALIZE, 0, &error);
			
		for (i = 0; i < num_mess; i++) {
			OSyncMessage *reply = osync_message_new_reply(message, &error);
			
			osync_queue_send_message(server_queue, NULL, reply, &error);
			
			osync_message_unref(reply);
		}
		
		osync_message_unref(message);
		
		if (osync_queue_disconnect(client_queue, &error) != TRUE || error != NULL)
			exit(1);
		osync_queue_unref(client_queue);
	
		message = osync_queue_get_message(server_queue);
		
		if (osync_message_get_command(message) != OSYNC_MESSAGE_QUEUE_HUP) {
			exit (1);
		}
	
		osync_message_unref(message);
			
		if (osync_queue_disconnect(server_queue, &error) != TRUE || error != NULL)
			exit(1);
		osync_queue_unref(server_queue);
		
		g_free(data);
		g_free(testbed);
		
		exit(0);
	} else {
		fail_unless(osync_queue_connect(client_queue, OSYNC_QUEUE_SENDER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		fail_unless(osync_queue_connect(server_queue, OSYNC_QUEUE_RECEIVER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		for (i = 0; i < num_mess; i++) {
			message = osync_message_new(OSYNC_MESSAGE_INITIALIZE, 0, &error);
			fail_unless(message != NULL, NULL);
			fail_unless(!osync_error_is_set(&error), NULL);
			
			osync_message_write_data(message, data, size);
			
			fail_unless(osync_queue_send_message(client_queue, NULL, message, &error), NULL);
			fail_unless(!osync_error_is_set(&error), NULL);
			
			osync_message_unref(message);
		}
		
		for (i = 0; i < num_mess; i++) {
			message = osync_queue_get_message(server_queue);
			
			fail_unless(osync_message_get_command(message) == OSYNC_MESSAGE_REPLY);
			
			osync_message_unref(message);
		}
		
		osync_queue_disconnect(server_queue, &error);
		fail_unless(error == NULL, NULL);
		
		message = osync_queue_get_message(client_queue);
		
		if (osync_message_get_command(message) != OSYNC_MESSAGE_QUEUE_HUP) {
			exit (1);
		}
		
		osync_message_unref(message);
		
		osync_queue_disconnect(client_queue, &error);
		fail_unless(error == NULL, NULL);
		
		int status = 0;
		wait(&status);
		fail_unless(WEXITSTATUS(status) == 0, NULL);
	}
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == TRUE, NULL);
	
	fail_unless(osync_queue_remove(client_queue, &error), NULL);
	fail_unless(osync_queue_remove(server_queue, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == FALSE, NULL);

	osync_queue_unref(client_queue);
	osync_queue_unref(server_queue);
	
	g_free(data);
	destroy_testbed(testbed);
}
END_TEST

START_TEST (ipc_large_payload)
{	
	char *testbed = setup_testbed(NULL);
	osync_testing_file_remove("/tmp/testpipe-server");
	osync_testing_file_remove("/tmp/testpipe-client");
	int i = 0;
	
	int num_mess = 10;
	int size = 1024 * 1024 * 20; //20mbyte
	
	char *data = malloc(size);
	memset(data, 42, size);
	
	OSyncError *error = NULL;
	OSyncQueue *server_queue = osync_queue_new("/tmp/testpipe-server", &error);
	OSyncQueue *client_queue = osync_queue_new("/tmp/testpipe-client", &error);
	OSyncMessage *message = NULL;
	
	osync_queue_create(server_queue, &error);
	fail_unless(error == NULL, NULL);
	
	osync_queue_create(client_queue, &error);
	fail_unless(error == NULL, NULL);
	
	pid_t cpid = fork();
	if (cpid == 0) { //Child
		osync_assert(osync_queue_connect(client_queue, OSYNC_QUEUE_RECEIVER, &error));
		osync_assert(error == NULL);

		osync_assert(osync_queue_connect(server_queue, OSYNC_QUEUE_SENDER, &error));
		osync_assert(error == NULL);
		
		for (i = 0; i < num_mess; i++) {
			message = osync_queue_get_message(client_queue);
			
			if (osync_message_get_command(message) != OSYNC_MESSAGE_INITIALIZE) {
				exit (1);
			}
			
			void *databuf = NULL;
			osync_message_read_const_data(message, &databuf, size);
		
			if (memcmp(databuf, data, size))
				exit(1);
			
			OSyncMessage *reply = osync_message_new_reply(message, &error);
			
			osync_message_unref(message);
			
			osync_queue_send_message(server_queue, NULL, reply, &error);
			
			osync_message_unref(reply);
		}
		
		if (osync_queue_disconnect(client_queue, &error) != TRUE || error != NULL)
			exit(1);
		osync_queue_unref(client_queue);
	
		message = osync_queue_get_message(server_queue);
		
		if (osync_message_get_command(message) != OSYNC_MESSAGE_QUEUE_HUP) {
			exit (1);
		}
		
		osync_message_unref(message);
			
		if (osync_queue_disconnect(server_queue, &error) != TRUE || error != NULL)
			exit(1);
		osync_queue_unref(server_queue);
		
		g_free(data);
		
		g_free(testbed);
		
		exit(0);
	} else {
		fail_unless(osync_queue_connect(client_queue, OSYNC_QUEUE_SENDER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		fail_unless(osync_queue_connect(server_queue, OSYNC_QUEUE_RECEIVER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		for (i = 0; i < num_mess; i++) {
			message = osync_message_new(OSYNC_MESSAGE_INITIALIZE, 0, &error);
			fail_unless(message != NULL, NULL);
			fail_unless(!osync_error_is_set(&error), NULL);
			
			osync_message_write_data(message, data, size);
			
			fail_unless(osync_queue_send_message(client_queue, NULL, message, &error), NULL);
			fail_unless(!osync_error_is_set(&error), NULL);

			osync_message_unref(message);

			message = osync_queue_get_message(server_queue);
			
			fail_unless(osync_message_get_command(message) == OSYNC_MESSAGE_REPLY);
			
			osync_message_unref(message);
		}
		
		osync_queue_disconnect(server_queue, &error);
		fail_unless(error == NULL, NULL);
		
		message = osync_queue_get_message(client_queue);
		
		if (osync_message_get_command(message) != OSYNC_MESSAGE_QUEUE_HUP) {
			exit (1);
		}
		
		osync_message_unref(message);
			
		osync_queue_disconnect(client_queue, &error);
		fail_unless(error == NULL, NULL);
		
		int status = 0;
		wait(&status);
		fail_unless(WEXITSTATUS(status) == 0, NULL);
	}
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == TRUE, NULL);
	
	fail_unless(osync_queue_remove(client_queue, &error), NULL);
	fail_unless(osync_queue_remove(server_queue, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == FALSE, NULL);

	osync_queue_unref(client_queue);
	osync_queue_unref(server_queue);
	
	g_free(data);
	destroy_testbed(testbed);
}
END_TEST

START_TEST (ipc_error_no_pipe)
{
	char *testbed = setup_testbed(NULL);
	osync_testing_file_remove("/tmp/testpipe");
	
	OSyncError *error = NULL;
	OSyncQueue *queue1 = osync_queue_new("/tmp/testpipe", &error);
	fail_unless(queue1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!osync_queue_connect(queue1, OSYNC_QUEUE_RECEIVER, &error), NULL);
	fail_unless(error != NULL, NULL);
	osync_error_unref(&error);
	
	osync_queue_unref(queue1);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (ipc_error_perm)
{
	char *testbed = setup_testbed(NULL);
	osync_testing_file_remove("/tmp/testpipe");
	
	OSyncError *error = NULL;
	OSyncQueue *queue = osync_queue_new("/tmp/testpipe", &error);
	
	osync_queue_create(queue, &error);
	fail_unless(error == NULL, NULL);
	
	if (osync_testing_file_chmod("/tmp/testpipe", 000))
		abort();
	
	fail_unless(!osync_queue_connect(queue, OSYNC_QUEUE_RECEIVER, &error), NULL);
	fail_unless(error != NULL, NULL);
	osync_error_unref(&error);
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe") == TRUE, NULL);
		
	fail_unless(osync_queue_remove(queue, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe") == FALSE, NULL);

	osync_queue_unref(queue);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (ipc_error_rem)
{	
	char *testbed = setup_testbed(NULL);
	osync_testing_file_remove("/tmp/testpipe");
	
	OSyncError *error = NULL;
	OSyncQueue *server_queue = osync_queue_new("/tmp/testpipe", &error);
	OSyncMessage *message = NULL;
	
	osync_queue_create(server_queue, &error);
	fail_unless(error == NULL, NULL);
	
	pid_t cpid = fork();
	if (cpid == 0) { //Child
		osync_assert(osync_queue_connect(server_queue, OSYNC_QUEUE_RECEIVER, &error));
		osync_assert(error == NULL);
		
		g_free(testbed);
		exit(0);
	} else {
		fail_unless(osync_queue_connect(server_queue, OSYNC_QUEUE_SENDER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		message = osync_queue_get_message(server_queue);
		osync_assert(osync_message_get_command(message) == OSYNC_MESSAGE_QUEUE_HUP);
		osync_message_unref(message);
		
		osync_queue_disconnect(server_queue, &error);
		fail_unless(error == NULL, NULL);
		
		int status = 0;
		wait(&status);
		fail_unless(WEXITSTATUS(status) == 0, NULL);
	}
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe") == TRUE, NULL);
	
	fail_unless(osync_queue_remove(server_queue, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe") == FALSE, NULL);

	osync_queue_unref(server_queue);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (ipc_error_rem2)
{	
	char *testbed = setup_testbed(NULL);
	osync_testing_file_remove("/tmp/testpipe");
	
	OSyncError *error = NULL;
	OSyncQueue *server_queue = osync_queue_new("/tmp/testpipe", &error);
	OSyncMessage *message = NULL;
	
	osync_queue_create(server_queue, &error);
	fail_unless(error == NULL, NULL);
	
	pid_t cpid = fork();
	if (cpid == 0) { //Child
		osync_assert(osync_queue_connect(server_queue, OSYNC_QUEUE_SENDER, &error));
		osync_assert(error == NULL);
		
		message = osync_message_new(OSYNC_MESSAGE_INITIALIZE, 0, &error);
		osync_assert(message != NULL);
		osync_assert(!osync_error_is_set(&error));
		
		osync_message_write_int(message, 4000000);
		osync_message_write_string(message, "this is a test string");
		osync_message_write_long_long_int(message, 400000000);
		
		osync_assert(osync_queue_send_message(server_queue, NULL, message, &error));
		osync_assert(!osync_error_is_set(&error));
		
		osync_message_unref(message);
		
		g_usleep(2*G_USEC_PER_SEC);
		
		osync_queue_disconnect(server_queue, &error);
		osync_assert(error == NULL);
		
		osync_queue_unref(server_queue);
		
		g_free(testbed);
		exit(0);
	} else {
		fail_unless(osync_queue_connect(server_queue, OSYNC_QUEUE_RECEIVER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		message = osync_queue_get_message(server_queue);
		
		/* 2005-05-23 dgollub
		   This unit tests seems to be kind of broken! What is is supposed to test?
		   No errors appears.... change osync_assert to "OSYNC_MESSAGE_INITALIZE".
		   TODO: Armin, whats wrong with this testcase? What means ipc_error_rem2? rem? read error message?

		*/   
//		osync_assert(osync_message_get_command(message) == OSYNC_MESSAGE_QUEUE_ERROR);
		osync_assert(osync_message_get_command(message) == OSYNC_MESSAGE_INITIALIZE);

		osync_message_unref(message);
		
		osync_queue_disconnect(server_queue, &error);
		fail_unless(error == NULL, NULL);
		
		int status = 0;
		wait(&status);
		fail_unless(WEXITSTATUS(status) == 0, NULL);
	}
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe") == TRUE, NULL);
	
	fail_unless(osync_queue_remove(server_queue, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe") == FALSE, NULL);

	osync_queue_unref(server_queue);
	
	destroy_testbed(testbed);
}
END_TEST

OSyncQueue *server_queue = NULL;
OSyncQueue *client_queue = NULL;

void server_handler1(OSyncMessage *message, void *user_data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);
	OSyncError *error = NULL;
	
	osync_assert(GPOINTER_TO_INT(user_data) ==1);
	
	osync_assert(osync_message_get_command(message) == OSYNC_MESSAGE_INITIALIZE);
	
	osync_queue_disconnect(server_queue, &error);
	osync_assert(error == NULL);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

void client_handler1(OSyncMessage *message, void *user_data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);
	OSyncError *error = NULL;
	
	osync_assert(GPOINTER_TO_INT(user_data) ==1);
	osync_assert(osync_message_get_command(message) == OSYNC_MESSAGE_INITIALIZE);
	
	int int1;
	long long int longint1;
	char *string;
	void *databuf;
	
	osync_message_read_int(message, &int1);
	osync_message_read_const_string(message, &string);
	osync_message_read_long_long_int(message, &longint1);
	osync_message_read_const_data(message, &databuf, strlen("this is another test string") + 1);
	
	osync_assert(int1 == 4000000);
	osync_assert(!strcmp(string, "this is a test string"));
	osync_assert(longint1 == 400000000);
	osync_assert(!strcmp(databuf, "this is another test string"));
	
	OSyncMessage *reply = osync_message_new(OSYNC_MESSAGE_INITIALIZE, 0, &error);
	
	osync_queue_send_message(server_queue, NULL, reply, &error);
	
	osync_message_unref(reply);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

START_TEST (ipc_loop_payload)
{	
	char *testbed = setup_testbed(NULL);
	osync_testing_file_remove("/tmp/testpipe-server");
	osync_testing_file_remove("/tmp/testpipe-client");
	
	OSyncError *error = NULL;
	server_queue = osync_queue_new("/tmp/testpipe-server", &error);
	client_queue = osync_queue_new("/tmp/testpipe-client", &error);
	OSyncMessage *message = NULL;
	
	osync_queue_create(server_queue, &error);
	fail_unless(error == NULL, NULL);
	
	osync_queue_create(client_queue, &error);
	fail_unless(error == NULL, NULL);
	char *data = "this is another test string";
	
	GMainContext *context = g_main_context_new();
	OSyncThread *thread = osync_thread_new(context, &error);
	
	pid_t cpid = fork();
	if (cpid == 0) { //Child
		osync_queue_set_message_handler(client_queue, client_handler1, GINT_TO_POINTER(1));
		
		osync_queue_setup_with_gmainloop(client_queue, context);
		
		osync_thread_start(thread);
	
		osync_assert(osync_queue_connect(client_queue, OSYNC_QUEUE_RECEIVER, &error));
		osync_assert(error == NULL);
		osync_assert(osync_queue_connect(server_queue, OSYNC_QUEUE_SENDER, &error));
		osync_assert(error == NULL);

		osync_queue_cross_link(client_queue, server_queue);
		
		message = osync_queue_get_message(server_queue);
		
		if (osync_message_get_command(message) != OSYNC_MESSAGE_QUEUE_HUP) {
			exit (1);
		}
		
		osync_message_unref(message);
	
		if (osync_queue_disconnect(server_queue, &error) != TRUE || error != NULL)
			exit(1);
		osync_queue_unref(server_queue);
		
		osync_assert(osync_queue_disconnect(client_queue, &error));
		osync_assert(error == NULL);
		
		osync_thread_stop(thread);
		osync_thread_unref(thread);
		
		osync_queue_unref(client_queue);
		
		g_free(testbed);
		
		exit(0);
	} else {
		osync_queue_set_message_handler(server_queue, server_handler1, GINT_TO_POINTER(1));
		
		osync_queue_setup_with_gmainloop(server_queue, context);
		
		osync_thread_start(thread);
	
		fail_unless(osync_queue_connect(client_queue, OSYNC_QUEUE_SENDER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		fail_unless(osync_queue_connect(server_queue, OSYNC_QUEUE_RECEIVER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		message = osync_message_new(OSYNC_MESSAGE_INITIALIZE, 0, &error);
		fail_unless(message != NULL, NULL);
		fail_unless(!osync_error_is_set(&error), NULL);
		
		osync_message_write_int(message, 4000000);
		osync_message_write_string(message, "this is a test string");
		osync_message_write_long_long_int(message, 400000000);
		osync_message_write_data(message, data, strlen(data) + 1);
		
		fail_unless(osync_queue_send_message(client_queue, NULL, message, &error), NULL);
		fail_unless(!osync_error_is_set(&error), NULL);
		
		osync_message_unref(message);
		
		message = osync_queue_get_message(client_queue);
		
		fail_unless(osync_message_get_command(message) == OSYNC_MESSAGE_QUEUE_HUP);
		
		osync_message_unref(message);
		
		osync_queue_disconnect(client_queue, &error);
		fail_unless(error == NULL, NULL);
		
		osync_thread_stop(thread);
		osync_thread_unref(thread);
		
		int status = 0;
		wait(&status);
		fail_unless(WEXITSTATUS(status) == 0, NULL);
	}
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == TRUE, NULL);
	
	fail_unless(osync_queue_remove(client_queue, &error), NULL);
	fail_unless(osync_queue_remove(server_queue, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == FALSE, NULL);

	osync_queue_unref(client_queue);
	osync_queue_unref(server_queue);
	
	destroy_testbed(testbed);
}
END_TEST

int num_msgs = 0;
int req_msgs = 1000;

void server_handler2(OSyncMessage *message, void *user_data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);
	OSyncError *error = NULL;
	char *data = "this is another test string";
	
	osync_assert(GPOINTER_TO_INT(user_data) ==1);
	
	num_msgs++;
	osync_assert(osync_message_get_command(message) == OSYNC_MESSAGE_INITIALIZE);
		
	if (num_msgs >= req_msgs) {
		osync_queue_disconnect(server_queue, &error);
		osync_assert(error == NULL);
	} else {
		message = osync_message_new(OSYNC_MESSAGE_INITIALIZE, 0, &error);
		osync_assert(message != NULL);
		osync_assert(!osync_error_is_set(&error));
		
		osync_message_write_int(message, 4000000);
		osync_message_write_string(message, "this is a test string");
		osync_message_write_long_long_int(message, 400000000);
		osync_message_write_data(message, data, strlen(data) + 1);
		
		osync_assert(osync_queue_send_message(client_queue, NULL, message, &error));
		osync_assert(!osync_error_is_set(&error));
		
		osync_message_unref(message);
	}
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

void client_handler2(OSyncMessage *message, void *user_data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);
	OSyncError *error = NULL;
	
	osync_assert(GPOINTER_TO_INT(user_data) ==1);
	osync_assert(osync_message_get_command(message) == OSYNC_MESSAGE_INITIALIZE);
	
	int int1;
	long long int longint1;
	char *string;
	void *databuf;
	
	osync_message_read_int(message, &int1);
	osync_message_read_const_string(message, &string);
	osync_message_read_long_long_int(message, &longint1);
	osync_message_read_const_data(message, &databuf, strlen("this is another test string") + 1);
	
	osync_assert(int1 == 4000000);
	osync_assert(!strcmp(string, "this is a test string"));
	osync_assert(longint1 == 400000000);
	osync_assert(!strcmp(databuf, "this is another test string"));
	
	OSyncMessage *reply = osync_message_new(OSYNC_MESSAGE_INITIALIZE, 0, &error);
	
	osync_queue_send_message(server_queue, NULL, reply, &error);
	
	osync_message_unref(reply);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

START_TEST (ipc_loop_stress)
{	
	char *testbed = setup_testbed(NULL);
	osync_testing_file_remove("/tmp/testpipe-server");
	osync_testing_file_remove("/tmp/testpipe-client");
	
	OSyncError *error = NULL;
	server_queue = osync_queue_new("/tmp/testpipe-server", &error);
	client_queue = osync_queue_new("/tmp/testpipe-client", &error);
	OSyncMessage *message = NULL;
	
	osync_queue_create(server_queue, &error);
	fail_unless(error == NULL, NULL);
	
	osync_queue_create(client_queue, &error);
	fail_unless(error == NULL, NULL);
	char *data = "this is another test string";
	
	pid_t cpid = fork();
	if (cpid == 0) { //Child
		
		GMainContext *context = g_main_context_new();
		OSyncThread *thread = osync_thread_new(context, &error);
	
		osync_queue_set_message_handler(client_queue, client_handler2, GINT_TO_POINTER(1));
		
		osync_queue_setup_with_gmainloop(client_queue, context);
		
		osync_thread_start(thread);
		
		osync_assert(osync_queue_connect(client_queue, OSYNC_QUEUE_RECEIVER, &error));
		osync_assert(error == NULL);
		osync_assert(osync_queue_connect(server_queue, OSYNC_QUEUE_SENDER, &error));
		osync_assert(error == NULL);

		osync_queue_cross_link(client_queue, server_queue);
		
		message = osync_queue_get_message(server_queue);
		
		if (osync_message_get_command(message) != OSYNC_MESSAGE_QUEUE_HUP) {
			exit (1);
		}
		
		osync_message_unref(message);
	
		if (osync_queue_disconnect(server_queue, &error) != TRUE || error != NULL)
			exit(1);
		osync_queue_unref(server_queue);
		
		osync_assert(osync_queue_disconnect(client_queue, &error));
		osync_assert(error == NULL);
		
		osync_thread_stop(thread);
		osync_thread_unref(thread);
		
		osync_queue_unref(client_queue);
		
		g_free(testbed);
		
		exit(0);
	} else {
		GMainContext *context = g_main_context_new();
		OSyncThread *thread = osync_thread_new(context, &error);
		
		osync_queue_set_message_handler(server_queue, server_handler2, GINT_TO_POINTER(1));
		
		osync_queue_setup_with_gmainloop(server_queue, context);
		
		osync_thread_start(thread);
	
		fail_unless(osync_queue_connect(client_queue, OSYNC_QUEUE_SENDER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		fail_unless(osync_queue_connect(server_queue, OSYNC_QUEUE_RECEIVER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		message = osync_message_new(OSYNC_MESSAGE_INITIALIZE, 0, &error);
		fail_unless(message != NULL, NULL);
		fail_unless(!osync_error_is_set(&error), NULL);
		
		osync_message_write_int(message, 4000000);
		osync_message_write_string(message, "this is a test string");
		osync_message_write_long_long_int(message, 400000000);
		osync_message_write_data(message, data, strlen(data) + 1);
		
		fail_unless(osync_queue_send_message(client_queue, NULL, message, &error), NULL);
		fail_unless(!osync_error_is_set(&error), NULL);
		
		osync_message_unref(message);
		
		message = osync_queue_get_message(client_queue);
		
		fail_unless(osync_message_get_command(message) == OSYNC_MESSAGE_QUEUE_HUP);
		
		osync_message_unref(message);
		
		osync_queue_disconnect(client_queue, &error);
		fail_unless(error == NULL, NULL);
		
		osync_thread_stop(thread);
		osync_thread_unref(thread);
		
		int status = 0;
		wait(&status);
		fail_unless(WEXITSTATUS(status) == 0, NULL);
	}
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == TRUE, NULL);
	
	fail_unless(osync_queue_remove(client_queue, &error), NULL);
	fail_unless(osync_queue_remove(server_queue, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == FALSE, NULL);

	osync_queue_unref(client_queue);
	osync_queue_unref(server_queue);
	
	destroy_testbed(testbed);
}
END_TEST

void callback_handler_check_reply(OSyncMessage *message, void *user_data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);
	OSyncError *error = NULL;
	
	osync_assert(GPOINTER_TO_INT(user_data) == 1);
	
	num_msgs++;
	osync_assert(osync_message_get_command(message) == OSYNC_MESSAGE_REPLY);
	
	if (num_msgs >= req_msgs) {
		osync_queue_disconnect(server_queue, &error);
		osync_assert(error == NULL);
	}
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

void server_handler_abort(OSyncMessage *message, void *user_data)
{
	abort();
}

void client_handler3(OSyncMessage *message, void *user_data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);
	OSyncError *error = NULL;
	
	osync_assert(GPOINTER_TO_INT(user_data) ==1);
	osync_assert(osync_message_get_command(message) == OSYNC_MESSAGE_INITIALIZE);
	
	int int1;
	long long int longint1;
	char *string;
	void *databuf;
	
	osync_message_read_int(message, &int1);
	osync_message_read_const_string(message, &string);
	osync_message_read_long_long_int(message, &longint1);
	osync_message_read_const_data(message, &databuf, strlen("this is another test string") + 1);
	
	osync_assert(int1 == 4000000);
	osync_assert(!strcmp(string, "this is a test string"));
	osync_assert(longint1 == 400000000);
	osync_assert(!strcmp(databuf, "this is another test string"));
	
	OSyncMessage *reply = osync_message_new_reply(message, &error);
	
	osync_queue_send_message(server_queue, NULL, reply, &error);
	
	osync_message_unref(reply);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

START_TEST (ipc_loop_callback)
{	
	num_msgs = 0;
	
	char *testbed = setup_testbed(NULL);
	osync_testing_file_remove("/tmp/testpipe-server");
	osync_testing_file_remove("/tmp/testpipe-client");
	
	OSyncError *error = NULL;
	server_queue = osync_queue_new("/tmp/testpipe-server", &error);
	client_queue = osync_queue_new("/tmp/testpipe-client", &error);
	OSyncMessage *message = NULL;
	
	osync_queue_create(server_queue, &error);
	fail_unless(error == NULL, NULL);
	
	osync_queue_create(client_queue, &error);
	fail_unless(error == NULL, NULL);
	char *data = "this is another test string";
	
	pid_t cpid = fork();
	if (cpid == 0) { //Child
		
		GMainContext *context = g_main_context_new();
		OSyncThread *thread = osync_thread_new(context, &error);
	
		osync_queue_set_message_handler(client_queue, client_handler3, GINT_TO_POINTER(1));
		
		osync_queue_setup_with_gmainloop(client_queue, context);
		
		osync_thread_start(thread);
		
		osync_assert(osync_queue_connect(client_queue, OSYNC_QUEUE_RECEIVER, &error));
		osync_assert(error == NULL);

		osync_assert(osync_queue_connect(server_queue, OSYNC_QUEUE_SENDER, &error));
		osync_assert(error == NULL);

		osync_queue_cross_link(client_queue, server_queue);
		
		message = osync_queue_get_message(server_queue);
		
		if (osync_message_get_command(message) != OSYNC_MESSAGE_QUEUE_HUP) {
			exit (1);
		}
		
		osync_message_unref(message);
	
		if (osync_queue_disconnect(server_queue, &error) != TRUE || error != NULL)
			exit(1);
		osync_queue_unref(server_queue);
		
		osync_assert(osync_queue_disconnect(client_queue, &error));
		osync_assert(error == NULL);
		
		osync_thread_stop(thread);
		osync_thread_unref(thread);
		
		osync_queue_unref(client_queue);
		
		g_free(testbed);
		
		exit(0);
	} else {
		GMainContext *context = g_main_context_new();
		OSyncThread *thread = osync_thread_new(context, &error);
		
		osync_queue_set_message_handler(server_queue, server_handler_abort, GINT_TO_POINTER(1));
		
		osync_queue_setup_with_gmainloop(server_queue, context);
		
		osync_thread_start(thread);
	
		fail_unless(osync_queue_connect(client_queue, OSYNC_QUEUE_SENDER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		fail_unless(osync_queue_connect(server_queue, OSYNC_QUEUE_RECEIVER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		int i = 0;
		for (i = 0; i < req_msgs; i++) {
			message = osync_message_new(OSYNC_MESSAGE_INITIALIZE, 0, &error);
			fail_unless(message != NULL, NULL);
			fail_unless(!osync_error_is_set(&error), NULL);
			
			osync_message_write_int(message, 4000000);
			osync_message_write_string(message, "this is a test string");
			osync_message_write_long_long_int(message, 400000000);
			osync_message_write_data(message, data, strlen(data) + 1);
			
			osync_message_set_handler(message, callback_handler_check_reply, GINT_TO_POINTER(1));
			
			fail_unless(osync_queue_send_message(client_queue, server_queue, message, &error), NULL);
			fail_unless(!osync_error_is_set(&error), NULL);
			
			osync_message_unref(message);
		}
		
		message = osync_queue_get_message(client_queue);
		
		fail_unless(osync_message_get_command(message) == OSYNC_MESSAGE_QUEUE_HUP);
		
		osync_message_unref(message);
		
		osync_queue_disconnect(client_queue, &error);
		fail_unless(error == NULL, NULL);
		
		osync_thread_stop(thread);
		osync_thread_unref(thread);
		
		int status = 0;
		wait(&status);
		fail_unless(WEXITSTATUS(status) == 0, NULL);
	}
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == TRUE, NULL);
	
	fail_unless(osync_queue_remove(client_queue, &error), NULL);
	fail_unless(osync_queue_remove(server_queue, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == FALSE, NULL);

	osync_queue_unref(client_queue);
	osync_queue_unref(server_queue);
	
	destroy_testbed(testbed);
}
END_TEST

int stop_after = 500;

void callback_handler2(OSyncMessage *message, void *user_data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);
	
	osync_assert(GPOINTER_TO_INT(user_data) == 1);
	
	if (num_msgs >= stop_after) {
		osync_assert(osync_message_get_command(message) == OSYNC_MESSAGE_ERRORREPLY);
	} else {
		osync_assert(osync_message_get_command(message) == OSYNC_MESSAGE_REPLY);
	}
	
	num_msgs++;
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

int num_msgs2 = 0;

void server_handler4(OSyncMessage *message, void *user_data)
{
	osync_assert(osync_message_get_command(message) == OSYNC_MESSAGE_QUEUE_HUP || osync_message_get_command(message) == OSYNC_MESSAGE_QUEUE_ERROR);
}

void client_handler4(OSyncMessage *message, void *user_data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);
	OSyncError *error = NULL;
	
	osync_assert(GPOINTER_TO_INT(user_data) ==1);
	osync_assert(osync_message_get_command(message) == OSYNC_MESSAGE_INITIALIZE);
	
	int int1;
	long long int longint1;
	char *string;
	void *databuf;
	
	
	osync_message_read_int(message, &int1);
	osync_message_read_const_string(message, &string);
	osync_message_read_long_long_int(message, &longint1);
	osync_message_read_const_data(message, &databuf, strlen("this is another test string") + 1);
	
	osync_assert(int1 == 4000000);
	osync_assert(!strcmp(string, "this is a test string"));
	osync_assert(longint1 == 400000000);
	osync_assert(!strcmp(databuf, "this is another test string"));
	
	if (num_msgs2 >= stop_after) {
		osync_assert(osync_queue_disconnect(client_queue, &error));
		osync_assert(error == NULL);
	} else {
		OSyncMessage *reply = osync_message_new_reply(message, &error);
		
		osync_queue_send_message(server_queue, NULL, reply, &error);
		
		osync_message_unref(reply);
	}
	
	num_msgs2++;
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

START_TEST (ipc_callback_break)
{	
	num_msgs = 0;
	
	char *testbed = setup_testbed(NULL);
	osync_testing_file_remove("/tmp/testpipe-server");
	osync_testing_file_remove("/tmp/testpipe-client");
	
	OSyncError *error = NULL;
	server_queue = osync_queue_new("/tmp/testpipe-server", &error);
	client_queue = osync_queue_new("/tmp/testpipe-client", &error);
	OSyncMessage *message = NULL;
	
	osync_queue_create(server_queue, &error);
	fail_unless(error == NULL, NULL);
	
	osync_queue_create(client_queue, &error);
	fail_unless(error == NULL, NULL);
	char *data = "this is another test string";
	
	pid_t cpid = fork();
	if (cpid == 0) { //Child
		
		GMainContext *context = g_main_context_new();
		OSyncThread *thread = osync_thread_new(context, &error);
	
		osync_queue_set_message_handler(client_queue, client_handler4, GINT_TO_POINTER(1));
		
		osync_queue_setup_with_gmainloop(client_queue, context);
		
		osync_thread_start(thread);
		
		osync_assert(osync_queue_connect(client_queue, OSYNC_QUEUE_RECEIVER, &error));
		osync_assert(error == NULL);

		osync_assert(osync_queue_connect(server_queue, OSYNC_QUEUE_SENDER, &error));
		osync_assert(error == NULL);

		osync_queue_cross_link(client_queue, server_queue);
		
		while (osync_queue_is_connected(client_queue)) { g_usleep(100); }
		
		osync_assert(osync_queue_disconnect(server_queue, &error));
		osync_assert(error == NULL);
		
		osync_thread_stop(thread);
		osync_thread_unref(thread);
		
		osync_queue_unref(client_queue);
		osync_queue_unref(server_queue);
		
		g_free(testbed);
		exit(0);
	} else {
		GMainContext *context = g_main_context_new();
		OSyncThread *thread = osync_thread_new(context, &error);
		
		osync_queue_set_message_handler(server_queue, server_handler4, GINT_TO_POINTER(1));
		
		osync_queue_setup_with_gmainloop(server_queue, context);
		
		osync_thread_start(thread);
	
		fail_unless(osync_queue_connect(client_queue, OSYNC_QUEUE_SENDER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		fail_unless(osync_queue_connect(server_queue, OSYNC_QUEUE_RECEIVER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		int i = 0;
		for (i = 0; i < req_msgs; i++) {
			message = osync_message_new(OSYNC_MESSAGE_INITIALIZE, 0, &error);
			fail_unless(message != NULL, NULL);
			fail_unless(!osync_error_is_set(&error), NULL);
			
			osync_message_write_int(message, 4000000);
			osync_message_write_string(message, "this is a test string");
			osync_message_write_long_long_int(message, 400000000);
			osync_message_write_data(message, data, strlen(data) + 1);
			
			osync_message_set_handler(message, callback_handler2, GINT_TO_POINTER(1));
			
			fail_unless(osync_queue_send_message(client_queue, server_queue, message, &error), NULL);
			fail_unless(!osync_error_is_set(&error), NULL);
			
			osync_message_unref(message);
		}
		
		message = osync_queue_get_message(client_queue);
		
		fail_unless(osync_message_get_command(message) == OSYNC_MESSAGE_QUEUE_HUP);
		
		osync_message_unref(message);
		
		while (num_msgs < req_msgs) { g_usleep(100); };
		
		osync_queue_disconnect(client_queue, &error);
		fail_unless(error == NULL, NULL);
		
		osync_queue_disconnect(server_queue, &error);
		osync_assert(error == NULL);
		
		osync_thread_stop(thread);
		osync_thread_unref(thread);
		
		int status = 0;
		wait(&status);
		fail_unless(WEXITSTATUS(status) == 0, NULL);
	}
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == TRUE, NULL);
	
	fail_unless(osync_queue_remove(client_queue, &error), NULL);
	fail_unless(osync_queue_remove(server_queue, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == FALSE, NULL);

	osync_queue_unref(client_queue);
	osync_queue_unref(server_queue);
	
	destroy_testbed(testbed);
}
END_TEST


START_TEST (ipc_pipes)
{	
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncQueue *read1 = NULL;
	OSyncQueue *write1 = NULL;
	char *data = "this is another test string";
	
	osync_assert(osync_queue_new_pipes(&read1, &write1, &error));
	osync_assert(error == NULL);
		
	fail_unless(osync_queue_connect(read1, OSYNC_QUEUE_RECEIVER, &error), NULL);
	fail_unless(error == NULL, NULL);
		
	fail_unless(osync_queue_connect(write1, OSYNC_QUEUE_SENDER, &error), NULL);
	fail_unless(error == NULL, NULL);
		
	OSyncMessage *message = osync_message_new(OSYNC_MESSAGE_INITIALIZE, 0, &error);
	fail_unless(message != NULL, NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	osync_message_write_int(message, 4000000);
	osync_message_write_string(message, "this is a test string");
	osync_message_write_long_long_int(message, 400000000);
	osync_message_write_data(message, data, strlen(data) + 1);
	
	fail_unless(osync_queue_send_message(write1, NULL, message, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	osync_message_unref(message);
		
	message = osync_queue_get_message(read1);
	osync_assert(osync_message_get_command(message) == OSYNC_MESSAGE_INITIALIZE);
	
	int int1;
	long long int longint1;
	char *string;
	void *databuf;
	
	osync_message_read_int(message, &int1);
	osync_message_read_const_string(message, &string);
	osync_message_read_long_long_int(message, &longint1);
	osync_message_read_const_data(message, &databuf, strlen("this is another test string") + 1);
	
	fail_unless(int1 == 4000000, NULL);
	fail_unless(!strcmp(string, "this is a test string"), NULL);
	fail_unless(longint1 == 400000000, NULL);
	fail_unless(!strcmp(databuf, "this is another test string"), NULL);
		
	osync_message_unref(message);
		
	osync_assert(osync_queue_disconnect(read1, &error));
	osync_assert(error == NULL);
	
	message = osync_queue_get_message(write1);
	osync_assert(osync_message_get_command(message) == OSYNC_MESSAGE_QUEUE_HUP);
	osync_message_unref(message);

	osync_assert(osync_queue_disconnect(write1, &error));
	osync_assert(error == NULL);
	
	
	osync_queue_unref(read1);
	osync_queue_unref(write1);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (ipc_pipes_stress)
{	
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncQueue *read1 = NULL;
	OSyncQueue *read2 = NULL;
	OSyncQueue *write1 = NULL;
	OSyncQueue *write2 = NULL;
	
	
	// First the pipe from the parent to the child
	osync_assert(osync_queue_new_pipes(&read1, &write1, &error));
	osync_assert(error == NULL);
	
	// Then the pipe from the child to the parent
	osync_assert(osync_queue_new_pipes(&read2, &write2, &error));
	osync_assert(error == NULL);
	
	OSyncMessage *message = NULL;
	
	char *data = "this is another test string";
	
	pid_t cpid = fork();
	if (cpid == 0) { //Child
		
		osync_assert(osync_queue_disconnect(write1, &error));
		osync_queue_unref(write1);
		
		osync_assert(osync_queue_disconnect(read2, &error));
		osync_queue_unref(read2);
		
		client_queue = read1;
		server_queue = write2;
		
		GMainContext *context = g_main_context_new();
		OSyncThread *thread = osync_thread_new(context, &error);
	
		osync_queue_set_message_handler(client_queue, client_handler2, GINT_TO_POINTER(1));
		
		osync_queue_setup_with_gmainloop(client_queue, context);
		
		osync_thread_start(thread);
		
		osync_assert(osync_queue_connect(client_queue, OSYNC_QUEUE_RECEIVER, &error));
		osync_assert(error == NULL);

		osync_assert(osync_queue_connect(server_queue, OSYNC_QUEUE_SENDER, &error));
		osync_assert(error == NULL);

		osync_queue_cross_link(client_queue, server_queue);
		
		message = osync_queue_get_message(server_queue);
		if (osync_message_get_command(message) != OSYNC_MESSAGE_QUEUE_HUP) {
			exit (1);
		}
		osync_message_unref(message);
	
		
		if (osync_queue_disconnect(server_queue, &error) != TRUE || error != NULL)
			exit(1);
		osync_queue_unref(server_queue);
		
		osync_assert(osync_queue_disconnect(client_queue, &error));
		osync_assert(error == NULL);
		
		osync_thread_stop(thread);
		osync_thread_unref(thread);
		
		osync_queue_unref(client_queue);
		
		g_free(testbed);
		
		exit(0);
	} else {
		
		osync_assert(osync_queue_disconnect(write2, &error));
		osync_queue_unref(write2);
		
		osync_assert(osync_queue_disconnect(read1, &error));
		osync_queue_unref(read1);
		
		client_queue = write1;
		server_queue = read2;
		
		GMainContext *context = g_main_context_new();
		OSyncThread *thread = osync_thread_new(context, &error);
		
		osync_queue_set_message_handler(server_queue, server_handler2, GINT_TO_POINTER(1));
		
		osync_queue_setup_with_gmainloop(server_queue, context);
		
		osync_thread_start(thread);
	
		fail_unless(osync_queue_connect(client_queue, OSYNC_QUEUE_SENDER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		fail_unless(osync_queue_connect(server_queue, OSYNC_QUEUE_RECEIVER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		message = osync_message_new(OSYNC_MESSAGE_INITIALIZE, 0, &error);
		fail_unless(message != NULL, NULL);
		fail_unless(!osync_error_is_set(&error), NULL);
		
		osync_message_write_int(message, 4000000);
		osync_message_write_string(message, "this is a test string");
		osync_message_write_long_long_int(message, 400000000);
		osync_message_write_data(message, data, strlen(data) + 1);
		
		fail_unless(osync_queue_send_message(client_queue, NULL, message, &error), NULL);
		fail_unless(!osync_error_is_set(&error), NULL);
		
		osync_message_unref(message);
		
		message = osync_queue_get_message(client_queue);
		fail_unless(osync_message_get_command(message) == OSYNC_MESSAGE_QUEUE_HUP);
		
		osync_message_unref(message);
		
		osync_queue_disconnect(client_queue, &error);
		fail_unless(error == NULL, NULL);
		
		osync_thread_stop(thread);
		osync_thread_unref(thread);
		
		int status = 0;
		wait(&status);
		fail_unless(WEXITSTATUS(status) == 0, NULL);
	}
	
	osync_queue_unref(client_queue);
	osync_queue_unref(server_queue);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (ipc_callback_break_pipes)
{	
	num_msgs = 0;
	
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncQueue *read1 = NULL;
	OSyncQueue *read2 = NULL;
	OSyncQueue *write1 = NULL;
	OSyncQueue *write2 = NULL;
	OSyncMessage *message = NULL;
	
	// First the pipe from the parent to the child
	osync_assert(osync_queue_new_pipes(&read1, &write1, &error));
	osync_assert(error == NULL);
	
	// Then the pipe from the child to the parent
	osync_assert(osync_queue_new_pipes(&read2, &write2, &error));
	osync_assert(error == NULL);
	
	char *data = "this is another test string";
	
	pid_t cpid = fork();
	if (cpid == 0) { //Child
		
		osync_assert(osync_queue_disconnect(write1, &error));
		osync_queue_unref(write1);
		
		osync_assert(osync_queue_disconnect(read2, &error));
		osync_queue_unref(read2);
		
		client_queue = read1;
		server_queue = write2;
		
		GMainContext *context = g_main_context_new();
		OSyncThread *thread = osync_thread_new(context, &error);
	
		osync_queue_set_message_handler(client_queue, client_handler4, GINT_TO_POINTER(1));
		
		osync_queue_setup_with_gmainloop(client_queue, context);
		
		osync_thread_start(thread);
		
		osync_assert(osync_queue_connect(client_queue, OSYNC_QUEUE_RECEIVER, &error));
		osync_assert(error == NULL);

		osync_assert(osync_queue_connect(server_queue, OSYNC_QUEUE_SENDER, &error));
		osync_assert(error == NULL);

		osync_queue_cross_link(client_queue, server_queue);
		
		while (osync_queue_is_connected(client_queue)) { g_usleep(100); }
		
		osync_assert(osync_queue_disconnect(server_queue, &error));
		osync_assert(error == NULL);
		
		osync_thread_stop(thread);
		osync_thread_unref(thread);
		
		osync_queue_unref(client_queue);
		osync_queue_unref(server_queue);
		
		g_free(testbed);
		exit(0);
	} else {
		
		osync_assert(osync_queue_disconnect(write2, &error));
		osync_queue_unref(write2);
		
		osync_assert(osync_queue_disconnect(read1, &error));
		osync_queue_unref(read1);
		
		client_queue = write1;
		server_queue = read2;
		
		GMainContext *context = g_main_context_new();
		OSyncThread *thread = osync_thread_new(context, &error);
		
		osync_queue_set_message_handler(server_queue, server_handler4, GINT_TO_POINTER(1));
		
		osync_queue_setup_with_gmainloop(server_queue, context);
		
		osync_thread_start(thread);
	
		fail_unless(osync_queue_connect(client_queue, OSYNC_QUEUE_SENDER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		fail_unless(osync_queue_connect(server_queue, OSYNC_QUEUE_RECEIVER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		int i = 0;
		for (i = 0; i < req_msgs; i++) {
			message = osync_message_new(OSYNC_MESSAGE_INITIALIZE, 0, &error);
			fail_unless(message != NULL, NULL);
			fail_unless(!osync_error_is_set(&error), NULL);
			
			osync_message_write_int(message, 4000000);
			osync_message_write_string(message, "this is a test string");
			osync_message_write_long_long_int(message, 400000000);
			osync_message_write_data(message, data, strlen(data) + 1);
			
			osync_message_set_handler(message, callback_handler2, GINT_TO_POINTER(1));
			
			fail_unless(osync_queue_send_message(client_queue, server_queue, message, &error), NULL);
			fail_unless(!osync_error_is_set(&error), NULL);
			
			osync_message_unref(message);
		}
		
		message = osync_queue_get_message(client_queue);
		
		fail_unless(osync_message_get_command(message) == OSYNC_MESSAGE_QUEUE_HUP);
		
		osync_message_unref(message);
		
		while (num_msgs < req_msgs) { g_usleep(100); };
		
		osync_queue_disconnect(client_queue, &error);
		fail_unless(error == NULL, NULL);
		
		osync_queue_disconnect(server_queue, &error);
		osync_assert(error == NULL);
		
		osync_thread_stop(thread);
		osync_thread_unref(thread);
		
		int status = 0;
		wait(&status);
		fail_unless(WEXITSTATUS(status) == 0, NULL);
	}
	
	osync_queue_unref(client_queue);
	osync_queue_unref(server_queue);
	
	destroy_testbed(testbed);
}
END_TEST

int num_callback_timeout = 0;
int num_callback = 0;

static void _message_handler(OSyncMessage *message, void *user_data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);
	osync_trace(TRACE_INTERNAL, "%s",osync_message_get_commandstr(message));
	if (osync_message_is_error(message))
		num_callback_timeout++;
	else
		num_callback++;
	osync_trace(TRACE_EXIT, "%s", __func__);
}

char *data5 = "this is another test string";
void client_handler5(OSyncMessage *message, void *user_data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);
	
	osync_assert(GPOINTER_TO_INT(user_data) ==1);
	osync_assert(osync_message_get_command(message) == OSYNC_MESSAGE_INITIALIZE);
		
	int int1;
	long long int longint1;
	char *string;
	char databuf[strlen(data5) + 1];
	
	
	osync_message_read_int(message, &int1);
	osync_message_read_string(message, &string);
	osync_message_read_long_long_int(message, &longint1);
	osync_message_read_data(message, databuf, strlen(data5) + 1);
	
	osync_assert(int1 == 4000000);
	osync_assert(!strcmp(string, "this is a test string"));
	osync_assert(longint1 == 400000000);
	osync_assert(!strcmp(databuf, data5));
	
	/* TIMEOUT TIMEOUT TIMEOUT (no reply...) */
	
	/* Proper code would reply to this message, but for testing
	   purposes we don't reply and simulate a "timeout" situation */
		
	osync_trace(TRACE_EXIT, "%s", __func__);
}

START_TEST (ipc_timeout)
{	
	/* This testcase is inteded to test osync_queue_send_message_with_timeout().
	   Client got forked and listens for messages from Server and replies.

	   To simulate a "timeout" situation the Client doesn't reply to one of the Server messages.

	   The timeout handler will call the _message_handler() with an error.
	   JFYI, every timed out message calls the callback/message_handler with an (timeout) error.
	   */

	char *testbed = setup_testbed(NULL);
	osync_testing_file_remove("/tmp/testpipe-server");
	osync_testing_file_remove("/tmp/testpipe-client");

	num_callback_timeout = 0;
	num_callback = 0;
	
	OSyncError *error = NULL;
	server_queue = osync_queue_new("/tmp/testpipe-server", &error);
	client_queue = osync_queue_new("/tmp/testpipe-client", &error);
	OSyncMessage *message = NULL;
	
	osync_queue_create(server_queue, &error);
	fail_unless(error == NULL, NULL);
	
	osync_queue_create(client_queue, &error);
	fail_unless(error == NULL, NULL);
	
	pid_t cpid = fork();
	if (cpid == 0) { //Child
		
		GMainContext *context = g_main_context_new();
		OSyncThread *thread = osync_thread_new(context, &error);
	
		osync_queue_set_message_handler(client_queue, client_handler5, GINT_TO_POINTER(1));
		osync_queue_set_pending_limit(client_queue, OSYNC_QUEUE_PENDING_LIMIT);
		
		osync_queue_setup_with_gmainloop(client_queue, context);
		
		osync_thread_start(thread);

		osync_assert(osync_queue_connect(client_queue, OSYNC_QUEUE_RECEIVER, &error));
		osync_assert(error == NULL);

		osync_assert(osync_queue_connect(server_queue, OSYNC_QUEUE_SENDER, &error));
		osync_assert(error == NULL);

		osync_queue_cross_link(client_queue, server_queue);
		
		message = osync_queue_get_message(server_queue);
		
		if (osync_message_get_command(message) != OSYNC_MESSAGE_QUEUE_HUP) {
			exit (1);
		}
		
		osync_message_unref(message);
	
		if (osync_queue_disconnect(server_queue, &error) != TRUE || error != NULL)
			exit(1);
		osync_queue_unref(server_queue);
		
		osync_assert(osync_queue_disconnect(client_queue, &error));
		osync_assert(error == NULL);
		
		osync_thread_stop(thread);
		osync_thread_unref(thread);
		
		osync_queue_unref(client_queue);
		
		g_free(testbed);
		
		exit(0);
	} else {
		GMainContext *context = g_main_context_new();
		OSyncThread *thread = osync_thread_new(context, &error);
		
		osync_queue_set_message_handler(server_queue, server_handler4, GINT_TO_POINTER(1));
		
		osync_queue_setup_with_gmainloop(server_queue, context);
		
		osync_thread_start(thread);

		fail_unless(osync_queue_connect(client_queue, OSYNC_QUEUE_SENDER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		fail_unless(osync_queue_connect(server_queue, OSYNC_QUEUE_RECEIVER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		message = osync_message_new(OSYNC_MESSAGE_INITIALIZE, 0, &error);
		fail_unless(message != NULL, NULL);
		fail_unless(!osync_error_is_set(&error), NULL);

		osync_message_set_handler(message, _message_handler, NULL);
		
		osync_message_write_int(message, 4000000);
		osync_message_write_string(message, "this is a test string");
		osync_message_write_long_long_int(message, 400000000);
		osync_message_write_data(message, data5, strlen(data5) + 1);
		
		// Send with timeout of one second
		fail_unless(osync_queue_send_message_with_timeout(client_queue, server_queue, message, 1, &error), NULL);
		fail_unless(!osync_error_is_set(&error), NULL);

		osync_message_unref(message);

		// Block
		g_usleep(5*G_USEC_PER_SEC);
		
		osync_queue_disconnect(server_queue, &error);
		fail_unless(error == NULL, NULL);
		
		while (!(message = osync_queue_get_message(client_queue))) {
			g_usleep(10000);
		}
		
		if (osync_message_get_command(message) != OSYNC_MESSAGE_QUEUE_HUP) {
			exit (1);
		}
		osync_message_unref(message);
		
		osync_queue_disconnect(client_queue, &error);
		fail_unless(error == NULL, NULL);
		
		int status = 0;
		wait(&status);
		fail_unless(WEXITSTATUS(status) == 0, NULL);
	}
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == TRUE, NULL);
	
	fail_unless(osync_queue_remove(client_queue, &error), NULL);
	fail_unless(osync_queue_remove(server_queue, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);

	/* Check if the timeout handler replied with an error */
	fail_unless(num_callback_timeout == 1, NULL);
	fail_unless(num_callback == 0, NULL);
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == FALSE, NULL);

	osync_queue_unref(client_queue);
	osync_queue_unref(server_queue);
	
	destroy_testbed(testbed);
}
END_TEST

int ch_sleep_time = 3; // Seconds
void client_handler_sleep(OSyncMessage *message, void *user_data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);
	OSyncError *error = NULL;
	
	osync_assert(GPOINTER_TO_INT(user_data) ==1);
	osync_assert(osync_message_get_command(message) == OSYNC_MESSAGE_INITIALIZE);
		
	int int1;
	long long int longint1;
	char *string;
	char databuf[strlen(data5) + 1];
	
	
	osync_message_read_int(message, &int1);
	osync_message_read_string(message, &string);
	osync_message_read_long_long_int(message, &longint1);
	osync_message_read_data(message, databuf, strlen(data5) + 1);
	
	osync_assert(int1 == 4000000);
	osync_assert(!strcmp(string, "this is a test string"));
	osync_assert(longint1 == 400000000);
	osync_assert(!strcmp(databuf, data5));

	// Do some time consuming processing
	g_usleep(ch_sleep_time*G_USEC_PER_SEC);
	
	OSyncMessage *reply = osync_message_new_reply(message, &error);
	
	osync_queue_send_message(server_queue, NULL, reply, &error);
	
	osync_message_unref(reply);
		
	osync_trace(TRACE_EXIT, "%s", __func__);
}

START_TEST (ipc_late_reply)
{	
	/* This testcase is inteded to test osync_queue_send_message_with_timeout().
	   Client got forked and listens for messages from Server and replies.

	   To simulate a "timeout" situation the Client delays 3 seconds and then replies

	   The timeout handler will call the _message_handler() with an error and the late reply
	   will be discarded.

	   JFYI, every timed out message calls the callback/message_handler with an (timeout) error.
	   */

	char *testbed = setup_testbed(NULL);
	osync_testing_file_remove("/tmp/testpipe-server");
	osync_testing_file_remove("/tmp/testpipe-client");

	num_callback_timeout = 0;
	num_callback = 0;
	ch_sleep_time = 3;

	OSyncError *error = NULL;
	server_queue = osync_queue_new("/tmp/testpipe-server", &error);
	client_queue = osync_queue_new("/tmp/testpipe-client", &error);
	OSyncMessage *message = NULL;
	
	osync_queue_create(server_queue, &error);
	fail_unless(error == NULL, NULL);
	
	osync_queue_create(client_queue, &error);
	fail_unless(error == NULL, NULL);
	
	pid_t cpid = fork();
	if (cpid == 0) { //Child
		
		GMainContext *context = g_main_context_new();
		OSyncThread *thread = osync_thread_new(context, &error);
	
		osync_queue_set_message_handler(client_queue, client_handler_sleep, GINT_TO_POINTER(1));
		osync_queue_set_pending_limit(client_queue, OSYNC_QUEUE_PENDING_LIMIT);
		
		osync_queue_setup_with_gmainloop(client_queue, context);
		
		osync_thread_start(thread);

		osync_assert(osync_queue_connect(client_queue, OSYNC_QUEUE_RECEIVER, &error));
		osync_assert(error == NULL);

		osync_assert(osync_queue_connect(server_queue, OSYNC_QUEUE_SENDER, &error));
		osync_assert(error == NULL);

		osync_queue_cross_link(client_queue, server_queue);
		
		message = osync_queue_get_message(server_queue);
		
		if (osync_message_get_command(message) != OSYNC_MESSAGE_QUEUE_HUP) {
			exit (1);
		}
		
		osync_message_unref(message);
	
		if (osync_queue_disconnect(server_queue, &error) != TRUE || error != NULL)
			exit(1);
		osync_queue_unref(server_queue);
		
		osync_assert(osync_queue_disconnect(client_queue, &error));
		osync_assert(error == NULL);
		
		osync_thread_stop(thread);
		osync_thread_unref(thread);
		
		osync_queue_unref(client_queue);
		
		g_free(testbed);
		
		exit(0);
	} else {
		GMainContext *context = g_main_context_new();
		OSyncThread *thread = osync_thread_new(context, &error);
		
		osync_queue_set_message_handler(server_queue, server_handler4, GINT_TO_POINTER(1));
		
		osync_queue_setup_with_gmainloop(server_queue, context);
		
		osync_thread_start(thread);

		fail_unless(osync_queue_connect(client_queue, OSYNC_QUEUE_SENDER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		fail_unless(osync_queue_connect(server_queue, OSYNC_QUEUE_RECEIVER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		message = osync_message_new(OSYNC_MESSAGE_INITIALIZE, 0, &error);
		fail_unless(message != NULL, NULL);
		fail_unless(!osync_error_is_set(&error), NULL);

		osync_message_set_handler(message, _message_handler, NULL);
		
		osync_message_write_int(message, 4000000);
		osync_message_write_string(message, "this is a test string");
		osync_message_write_long_long_int(message, 400000000);
		osync_message_write_data(message, data5, strlen(data5) + 1);
		
		// Send with timeout of one second
		fail_unless(osync_queue_send_message_with_timeout(client_queue, server_queue, message, 1, &error), NULL);
		fail_unless(!osync_error_is_set(&error), NULL);

		osync_message_unref(message);

		// Block
		g_usleep(5*G_USEC_PER_SEC);
		
		osync_queue_disconnect(server_queue, &error);
		fail_unless(error == NULL, NULL);
		
		while (!(message = osync_queue_get_message(client_queue))) {
			g_usleep(10000);
		}
		
		if (osync_message_get_command(message) != OSYNC_MESSAGE_QUEUE_HUP) {
			exit (1);
		}
		osync_message_unref(message);
		
		osync_queue_disconnect(client_queue, &error);
		fail_unless(error == NULL, NULL);
		
		int status = 0;
		wait(&status);
		fail_unless(WEXITSTATUS(status) == 0, NULL);
	}
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == TRUE, NULL);
	
	fail_unless(osync_queue_remove(client_queue, &error), NULL);
	fail_unless(osync_queue_remove(server_queue, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);

	/* Check if the timeout handler replied with an error */
	fail_unless(num_callback_timeout == 1, NULL);
	fail_unless(num_callback == 0, NULL);
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == FALSE, NULL);

	osync_queue_unref(client_queue);
	osync_queue_unref(server_queue);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (ipc_loop_with_timeout)
{
	
	/* Even though each action takes 1 second, none of these messages should time out
	   as they are being sent with a timeout of 3 seconds */
	
	num_msgs = 0;
	req_msgs = 20;
	ch_sleep_time = 1; // Second
	
	char *testbed = setup_testbed(NULL);
	osync_testing_file_remove("/tmp/testpipe-server");
	osync_testing_file_remove("/tmp/testpipe-client");
	
	OSyncError *error = NULL;
	server_queue = osync_queue_new("/tmp/testpipe-server", &error);
	client_queue = osync_queue_new("/tmp/testpipe-client", &error);
	OSyncMessage *message = NULL;
	
	osync_queue_create(server_queue, &error);
	fail_unless(error == NULL, NULL);
	
	osync_queue_create(client_queue, &error);
	fail_unless(error == NULL, NULL);
	char *data = "this is another test string";
	
	pid_t cpid = fork();
	if (cpid == 0) { //Child
		
		GMainContext *context = g_main_context_new();
		OSyncThread *thread = osync_thread_new(context, &error);
	
		osync_queue_set_message_handler(client_queue, client_handler_sleep, GINT_TO_POINTER(1));
		osync_queue_set_pending_limit(client_queue, OSYNC_QUEUE_PENDING_LIMIT);
		
		osync_queue_setup_with_gmainloop(client_queue, context);
		
		osync_thread_start(thread);
		
		osync_assert(osync_queue_connect(client_queue, OSYNC_QUEUE_RECEIVER, &error));
		osync_assert(error == NULL);

		osync_assert(osync_queue_connect(server_queue, OSYNC_QUEUE_SENDER, &error));
		osync_assert(error == NULL);

		osync_queue_cross_link(client_queue, server_queue);
		
		message = osync_queue_get_message(server_queue);
		
		if (osync_message_get_command(message) != OSYNC_MESSAGE_QUEUE_HUP) {
			exit (1);
		}
		
		osync_message_unref(message);
	
		if (osync_queue_disconnect(server_queue, &error) != TRUE || error != NULL)
			exit(1);
		osync_queue_unref(server_queue);
		
		osync_assert(osync_queue_disconnect(client_queue, &error));
		osync_assert(error == NULL);
		
		osync_thread_stop(thread);
		osync_thread_unref(thread);
		
		osync_queue_unref(client_queue);
		
		g_free(testbed);
		
		exit(0);
	} else {
		GMainContext *context = g_main_context_new();
		OSyncThread *thread = osync_thread_new(context, &error);
		
		osync_queue_set_message_handler(server_queue, server_handler_abort, GINT_TO_POINTER(1));
		
		osync_queue_setup_with_gmainloop(server_queue, context);
		
		osync_thread_start(thread);
	
		fail_unless(osync_queue_connect(client_queue, OSYNC_QUEUE_SENDER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		fail_unless(osync_queue_connect(server_queue, OSYNC_QUEUE_RECEIVER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		int i = 0;
		for (i = 0; i < req_msgs; i++) {
			message = osync_message_new(OSYNC_MESSAGE_INITIALIZE, 0, &error);
			fail_unless(message != NULL, NULL);
			fail_unless(!osync_error_is_set(&error), NULL);
			
			osync_message_write_int(message, 4000000);
			osync_message_write_string(message, "this is a test string");
			osync_message_write_long_long_int(message, 400000000);
			osync_message_write_data(message, data, strlen(data) + 1);
			
			osync_message_set_handler(message, callback_handler_check_reply, GINT_TO_POINTER(1));
			
			fail_unless(osync_queue_send_message_with_timeout(client_queue, server_queue, message, 3, &error), NULL);
			fail_unless(!osync_error_is_set(&error), NULL);
			
			osync_message_unref(message);
		}
		
		message = osync_queue_get_message(client_queue);
		
		fail_unless(osync_message_get_command(message) == OSYNC_MESSAGE_QUEUE_HUP);
		
		osync_message_unref(message);
		
		osync_queue_disconnect(client_queue, &error);
		fail_unless(error == NULL, NULL);
		
		osync_thread_stop(thread);
		osync_thread_unref(thread);
		
		int status = 0;
		wait(&status);
		fail_unless(WEXITSTATUS(status) == 0, NULL);
	}
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == TRUE, NULL);
	
	fail_unless(osync_queue_remove(client_queue, &error), NULL);
	fail_unless(osync_queue_remove(server_queue, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == FALSE, NULL);

	osync_queue_unref(client_queue);
	osync_queue_unref(server_queue);
	
	destroy_testbed(testbed);
}
END_TEST

GSList *ch_pending = NULL;
void client_handler_first_part(OSyncMessage *message, void *user_data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);
	
	osync_assert(GPOINTER_TO_INT(user_data) ==1);
	osync_assert(osync_message_get_command(message) == OSYNC_MESSAGE_INITIALIZE);
		
	int int1;
	long long int longint1;
	char *string;
	char databuf[strlen(data5) + 1];
	
	
	osync_message_read_int(message, &int1);
	osync_message_read_string(message, &string);
	osync_message_read_long_long_int(message, &longint1);
	osync_message_read_data(message, databuf, strlen(data5) + 1);
	
	osync_assert(int1 == 4000000);
	osync_assert(!strcmp(string, "this is a test string"));
	osync_assert(longint1 == 400000000);
	osync_assert(!strcmp(databuf, data5));

	// Put message on pending queue and return
	osync_message_ref(message);
	ch_pending = g_slist_append(ch_pending, message);

	osync_trace(TRACE_EXIT, "%s", __func__);
}
gboolean client_handler_second_part(gpointer userdata)
{
	OSyncError *error = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p)", __func__, userdata);

	if (ch_pending) {
		
		OSyncMessage *message = ch_pending->data;

		OSyncMessage *reply = osync_message_new_reply(message, &error);
	
		osync_queue_send_message(server_queue, NULL, reply, &error);
	
		osync_message_unref(reply);

		ch_pending = g_slist_remove(ch_pending, message);
		osync_message_unref(message);

		osync_trace(TRACE_EXIT, "%s", __func__);
		return TRUE;
	}
		
	osync_trace(TRACE_EXIT, "%s: no more entries", __func__);
	return FALSE;
}

START_TEST (ipc_loop_timeout_with_idle)
{
	
	/* Same as ipc_loop_with_timeout except that the client handler doesn't sleep,
	   so the queue dispatchers can run while the operation is waiting.
	   Even though each action takes 1 second, and might be delayed by 3 seconds
	   due to the messages already processed on the pending queue, none of these 
	   messages should time out as they are being sent with a timeout of 5 seconds */
	
	num_msgs = 0;
	req_msgs = 10;
	
	char *testbed = setup_testbed(NULL);
	osync_testing_file_remove("/tmp/testpipe-server");
	osync_testing_file_remove("/tmp/testpipe-client");
	
	OSyncError *error = NULL;
	server_queue = osync_queue_new("/tmp/testpipe-server", &error);
	client_queue = osync_queue_new("/tmp/testpipe-client", &error);
	OSyncMessage *message = NULL;
	
	osync_queue_create(server_queue, &error);
	fail_unless(error == NULL, NULL);
	
	osync_queue_create(client_queue, &error);
	fail_unless(error == NULL, NULL);
	char *data = "this is another test string";
	
	pid_t cpid = fork();
	if (cpid == 0) { //Child
		
		GMainContext *context = g_main_context_new();
		OSyncThread *thread = osync_thread_new(context, &error);
	
		osync_queue_set_message_handler(client_queue, client_handler_first_part, GINT_TO_POINTER(1));
		// Set pending limit to 3 so response wil be delayed at most 3 seconds
		osync_queue_set_pending_limit(client_queue, 3);
		
		osync_queue_setup_with_gmainloop(client_queue, context);
		
		osync_thread_start(thread);
		
		osync_assert(osync_queue_connect(client_queue, OSYNC_QUEUE_RECEIVER, &error));
		osync_assert(error == NULL);

		osync_assert(osync_queue_connect(server_queue, OSYNC_QUEUE_SENDER, &error));
		osync_assert(error == NULL);

		osync_queue_cross_link(client_queue, server_queue);

		GSource *tsource = g_timeout_source_new(1000);
		osync_assert(tsource);
		g_source_set_callback(tsource, client_handler_second_part, NULL, NULL);
		osync_assert(g_source_attach(tsource, context));
		g_source_unref(tsource);
		
		message = osync_queue_get_message(server_queue);
		
		if (osync_message_get_command(message) != OSYNC_MESSAGE_QUEUE_HUP) {
			exit (1);
		}
		
		osync_message_unref(message);
	
		if (osync_queue_disconnect(server_queue, &error) != TRUE || error != NULL)
			exit(1);
		osync_queue_unref(server_queue);
		
		osync_assert(osync_queue_disconnect(client_queue, &error));
		osync_assert(error == NULL);
		
		osync_thread_stop(thread);
		osync_thread_unref(thread);
		
		osync_queue_unref(client_queue);
		
		g_free(testbed);
		
		exit(0);
	} else {
		GMainContext *context = g_main_context_new();
		OSyncThread *thread = osync_thread_new(context, &error);
		
		osync_queue_set_message_handler(server_queue, server_handler_abort, GINT_TO_POINTER(1));
		
		osync_queue_setup_with_gmainloop(server_queue, context);
		
		osync_thread_start(thread);
	
		fail_unless(osync_queue_connect(client_queue, OSYNC_QUEUE_SENDER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		fail_unless(osync_queue_connect(server_queue, OSYNC_QUEUE_RECEIVER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		int i = 0;
		for (i = 0; i < req_msgs; i++) {
			message = osync_message_new(OSYNC_MESSAGE_INITIALIZE, 0, &error);
			fail_unless(message != NULL, NULL);
			fail_unless(!osync_error_is_set(&error), NULL);
			
			osync_message_write_int(message, 4000000);
			osync_message_write_string(message, "this is a test string");
			osync_message_write_long_long_int(message, 400000000);
			osync_message_write_data(message, data, strlen(data) + 1);
			
			osync_message_set_handler(message, callback_handler_check_reply, GINT_TO_POINTER(1));
			
			// Timeout of 5 will do as pending limit is 3
			fail_unless(osync_queue_send_message_with_timeout(client_queue, server_queue, message, 5, &error), NULL);
			fail_unless(!osync_error_is_set(&error), NULL);
			
			osync_message_unref(message);
		}
		
		message = osync_queue_get_message(client_queue);
		
		fail_unless(osync_message_get_command(message) == OSYNC_MESSAGE_QUEUE_HUP);
		
		osync_message_unref(message);
		
		osync_queue_disconnect(client_queue, &error);
		fail_unless(error == NULL, NULL);
		
		osync_thread_stop(thread);
		osync_thread_unref(thread);
		
		int status = 0;
		wait(&status);
		fail_unless(WEXITSTATUS(status) == 0, NULL);
	}
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == TRUE, NULL);
	
	fail_unless(osync_queue_remove(client_queue, &error), NULL);
	fail_unless(osync_queue_remove(server_queue, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == FALSE, NULL);

	osync_queue_unref(client_queue);
	osync_queue_unref(server_queue);
	
	destroy_testbed(testbed);
}
END_TEST

void client_handler6(OSyncMessage *message, void *user_data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, message, user_data);
	
	osync_assert(GPOINTER_TO_INT(user_data) ==1);

	if (osync_message_get_command(message) == OSYNC_MESSAGE_QUEUE_ERROR) {
		osync_queue_disconnect(client_queue, NULL);
		osync_trace(TRACE_EXIT, "%s: disconnect", __func__);
		return;
	}

	osync_assert(osync_message_get_command(message) == OSYNC_MESSAGE_INITIALIZE);

	int int1;
	long long int longint1;
	char *string;
	char databuf[strlen(data5) + 1];
	
	osync_message_read_int(message, &int1);
	osync_message_read_string(message, &string);
	osync_message_read_long_long_int(message, &longint1);
	osync_message_read_data(message, databuf, strlen(data5) + 1);
	
	osync_assert(int1 == 4000000);
	osync_assert(!strcmp(string, "this is a test string"));
	osync_assert(longint1 == 400000000);
	osync_assert(!strcmp(databuf, data5));
	
	/* TIMEOUT TIMEOUT TIMEOUT (no reply...) */
	
	/* Proper code would reply to this message, but for testing
	   purposes we don't reply and simulate a "timeout" situation */
		
	osync_trace(TRACE_EXIT, "%s", __func__);
}
START_TEST (ipc_timeout_noreplyq)
{	
	/* This testcase is inteded to test timeout before the command and reply queues are cross-linked.
	   Client got forked and listens for messages from Server and replies.

	   To simulate a "timeout" situation the Client doesn't reply to one of the Server messages.

	   As there is no reply queue, an error will be sent to the **client**, who then disconnects
	   so an error (although not a timeout) ends up sent to the server.
	   */

	char *testbed = setup_testbed(NULL);
	osync_testing_file_remove("/tmp/testpipe-server");
	osync_testing_file_remove("/tmp/testpipe-client");

	num_callback_timeout = 0;
	num_callback = 0;
	
	OSyncError *error = NULL;
	server_queue = osync_queue_new("/tmp/testpipe-server", &error);
	client_queue = osync_queue_new("/tmp/testpipe-client", &error);
	OSyncMessage *message = NULL;
	
	osync_queue_create(server_queue, &error);
	fail_unless(error == NULL, NULL);
	
	osync_queue_create(client_queue, &error);
	fail_unless(error == NULL, NULL);
	
	pid_t cpid = fork();
	if (cpid == 0) { //Child
		
		GMainContext *context = g_main_context_new();
		OSyncThread *thread = osync_thread_new(context, &error);
	
		osync_queue_set_message_handler(client_queue, client_handler6, GINT_TO_POINTER(1));
		osync_queue_set_pending_limit(client_queue, OSYNC_QUEUE_PENDING_LIMIT);
		
		osync_queue_setup_with_gmainloop(client_queue, context);
		
		osync_thread_start(thread);

		osync_assert(osync_queue_connect(client_queue, OSYNC_QUEUE_RECEIVER, &error));
		osync_assert(error == NULL);

		osync_assert(osync_queue_connect(server_queue, OSYNC_QUEUE_SENDER, &error));
		osync_assert(error == NULL);

		/* Do not cross-link */
		/*osync_queue_cross_link(client_queue, server_queue);*/
		
		message = osync_queue_get_message(server_queue);
		
		osync_assert(osync_message_get_command(message) == OSYNC_MESSAGE_QUEUE_HUP);
		
		osync_message_unref(message);
	
		if (osync_queue_disconnect(server_queue, &error) != TRUE || error != NULL)
			exit(1);
		osync_queue_unref(server_queue);
		
		osync_assert(osync_queue_disconnect(client_queue, &error));
		osync_assert(error == NULL);
		
		osync_thread_stop(thread);
		osync_thread_unref(thread);
		
		osync_queue_unref(client_queue);
		
		g_free(testbed);
		
		exit(0);
	} else {
		GMainContext *context = g_main_context_new();
		OSyncThread *thread = osync_thread_new(context, &error);
		
		osync_queue_set_message_handler(server_queue, server_handler4, GINT_TO_POINTER(1));
		
		osync_queue_setup_with_gmainloop(server_queue, context);
		
		osync_thread_start(thread);

		fail_unless(osync_queue_connect(client_queue, OSYNC_QUEUE_SENDER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		fail_unless(osync_queue_connect(server_queue, OSYNC_QUEUE_RECEIVER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		message = osync_message_new(OSYNC_MESSAGE_INITIALIZE, 0, &error);
		fail_unless(message != NULL, NULL);
		fail_unless(!osync_error_is_set(&error), NULL);

		osync_message_set_handler(message, _message_handler, NULL);
		
		osync_message_write_int(message, 4000000);
		osync_message_write_string(message, "this is a test string");
		osync_message_write_long_long_int(message, 400000000);
		osync_message_write_data(message, data5, strlen(data5) + 1);
		
		// Send with timeout of one second
		fail_unless(osync_queue_send_message_with_timeout(client_queue, server_queue, message, 1, &error), NULL);
		fail_unless(!osync_error_is_set(&error), NULL);

		osync_message_unref(message);
		
		while (!(message = osync_queue_get_message(client_queue))) {
			g_usleep(10000);
		}
		
		fail_unless(osync_message_get_command(message) == OSYNC_MESSAGE_QUEUE_HUP);
		osync_message_unref(message);
		
		osync_queue_disconnect(client_queue, &error);
		fail_unless(error == NULL, NULL);
		
		osync_queue_disconnect(server_queue, &error);
		fail_unless(error == NULL, NULL);
		
		int status = 0;
		wait(&status);
		fail_unless(WEXITSTATUS(status) == 0, NULL);
	}
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == TRUE, NULL);
	
	fail_unless(osync_queue_remove(client_queue, &error), NULL);
	fail_unless(osync_queue_remove(server_queue, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);

	/* Check if the timeout handler replied with an error */
	fail_unless(num_callback_timeout == 1, NULL);
	fail_unless(num_callback == 0, NULL);
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == FALSE, NULL);

	osync_queue_unref(client_queue);
	osync_queue_unref(server_queue);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (ipc_timeout_noreceiver)
{	
	/* This testcase is intended to test the case where the receiver is not even listening,
	   and so does not run the timeout.
	*/

	char *testbed = setup_testbed(NULL);
	osync_testing_file_remove("/tmp/testpipe-server");
	osync_testing_file_remove("/tmp/testpipe-client");

	num_callback_timeout = 0;
	num_callback = 0;
	
	OSyncError *error = NULL;
	server_queue = osync_queue_new("/tmp/testpipe-server", &error);
	client_queue = osync_queue_new("/tmp/testpipe-client", &error);
	OSyncMessage *message = NULL;
	
	osync_queue_create(server_queue, &error);
	fail_unless(error == NULL, NULL);
	
	osync_queue_create(client_queue, &error);
	fail_unless(error == NULL, NULL);
	
	pid_t cpid = fork();
	if (cpid == 0) { //Child
		
		GMainContext *context = g_main_context_new();
		OSyncThread *thread = osync_thread_new(context, &error);
	
		osync_queue_set_message_handler(client_queue, client_handler1, GINT_TO_POINTER(1));
		osync_queue_set_pending_limit(client_queue, OSYNC_QUEUE_PENDING_LIMIT);
		
		/* Do not start receiver */
		/* osync_queue_setup_with_gmainloop(client_queue, context); */
		
		osync_thread_start(thread);

		osync_assert(osync_queue_connect(client_queue, OSYNC_QUEUE_RECEIVER, &error));
		osync_assert(error == NULL);

		osync_assert(osync_queue_connect(server_queue, OSYNC_QUEUE_SENDER, &error));
		osync_assert(error == NULL);

		/* Do not cross-link */
		osync_queue_cross_link(client_queue, server_queue);
		
		message = osync_queue_get_message(server_queue);
		
		osync_assert(osync_message_get_command(message) == OSYNC_MESSAGE_QUEUE_HUP);
		
		osync_message_unref(message);
	
		if (osync_queue_disconnect(server_queue, &error) != TRUE || error != NULL)
			exit(1);
		osync_queue_unref(server_queue);
		
		osync_assert(osync_queue_disconnect(client_queue, &error));
		osync_assert(error == NULL);
		
		osync_thread_stop(thread);
		osync_thread_unref(thread);
		
		osync_queue_unref(client_queue);
		
		g_free(testbed);
		
		exit(0);
	} else {
		GMainContext *context = g_main_context_new();
		OSyncThread *thread = osync_thread_new(context, &error);
		
		osync_queue_set_message_handler(server_queue, server_handler4, GINT_TO_POINTER(1));
		
		osync_queue_setup_with_gmainloop(server_queue, context);
		
		osync_thread_start(thread);

		fail_unless(osync_queue_connect(client_queue, OSYNC_QUEUE_SENDER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		fail_unless(osync_queue_connect(server_queue, OSYNC_QUEUE_RECEIVER, &error), NULL);
		fail_unless(error == NULL, NULL);
		
		message = osync_message_new(OSYNC_MESSAGE_INITIALIZE, 0, &error);
		fail_unless(message != NULL, NULL);
		fail_unless(!osync_error_is_set(&error), NULL);

		osync_message_set_handler(message, _message_handler, NULL);
		
		osync_message_write_int(message, 4000000);
		osync_message_write_string(message, "this is a test string");
		osync_message_write_long_long_int(message, 400000000);
		osync_message_write_data(message, data5, strlen(data5) + 1);
		
		// Send with timeout of one second
		fail_unless(osync_queue_send_message_with_timeout(client_queue, server_queue, message, 1, &error), NULL);
		fail_unless(!osync_error_is_set(&error), NULL);

		osync_message_unref(message);
		
		/* Note: OSYNC_QUEUE_PENDING_QUEUE_MIN_TIMEOUT is 20 */
		g_usleep(25*G_USEC_PER_SEC);

		/* Check if the timeout handler replied with an error.
		   Note: it is important we check **before** we start disconnecting
		   otherwise we are not testing the right thing */
		fail_unless(num_callback_timeout == 1, NULL);
		fail_unless(num_callback == 0, NULL);
		
		osync_queue_disconnect(client_queue, &error);
		fail_unless(error == NULL, NULL);
		
		osync_queue_disconnect(server_queue, &error);
		fail_unless(error == NULL, NULL);
		
		int status = 0;
		wait(&status);
		fail_unless(WEXITSTATUS(status) == 0, NULL);
	}
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == TRUE, NULL);
	
	fail_unless(osync_queue_remove(client_queue, &error), NULL);
	fail_unless(osync_queue_remove(server_queue, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	fail_unless(osync_testing_file_exists("/tmp/testpipe-client") == FALSE, NULL);

	osync_queue_unref(client_queue);
	osync_queue_unref(server_queue);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_TESTCASE_START("ipc")
OSYNC_TESTCASE_ADD(ipc_new)
OSYNC_TESTCASE_ADD(ipc_ref)
OSYNC_TESTCASE_ADD(ipc_create)
OSYNC_TESTCASE_ADD(ipc_connect)
OSYNC_TESTCASE_ADD(ipc_payload)
OSYNC_TESTCASE_ADD(ipc_payload_wait)
OSYNC_TESTCASE_ADD(ipc_payload_stress)
OSYNC_TESTCASE_ADD(ipc_payload_stress2)
OSYNC_TESTCASE_ADD(ipc_large_payload)

OSYNC_TESTCASE_ADD(ipc_error_no_pipe)
OSYNC_TESTCASE_ADD(ipc_error_perm)
OSYNC_TESTCASE_ADD(ipc_error_rem)
OSYNC_TESTCASE_ADD(ipc_error_rem2)

OSYNC_TESTCASE_ADD(ipc_loop_payload)
OSYNC_TESTCASE_ADD(ipc_loop_stress)
OSYNC_TESTCASE_ADD(ipc_loop_callback)
OSYNC_TESTCASE_ADD(ipc_callback_break)

OSYNC_TESTCASE_ADD(ipc_pipes)
OSYNC_TESTCASE_ADD(ipc_pipes_stress)
OSYNC_TESTCASE_ADD(ipc_callback_break_pipes)

OSYNC_TESTCASE_ADD(ipc_timeout)
OSYNC_TESTCASE_ADD(ipc_late_reply)
OSYNC_TESTCASE_ADD(ipc_loop_with_timeout)
OSYNC_TESTCASE_ADD(ipc_loop_timeout_with_idle)
OSYNC_TESTCASE_ADD(ipc_timeout_noreplyq)
OSYNC_TESTCASE_ADD(ipc_timeout_noreceiver)
OSYNC_TESTCASE_END

