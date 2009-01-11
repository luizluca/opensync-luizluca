#include "support.h"

#include "opensync/group/opensync_group_internals.h"
#include "opensync/engine/opensync_engine_internals.h"

/* Regression test for Ticket #988 */
START_TEST (sync_error_single_init_error_noerror)
{
	char *testbed = setup_testbed("sync");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("INIT_NULL_NOERROR", "2", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_group_unref(group);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_unref(engine);
	g_free(formatdir);
	g_free(plugindir);
       
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_single_init_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("INIT_NULL", "2", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	/* this is a bug - please see sml_fail_unless for details */
	fail_unless(osync_engine_initialize(engine, &error), osync_error_print(&error));
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_dual_connect_error)
{
	char *testbed = setup_testbed("sync_easy_new");
	
	g_setenv("CONNECT_ERROR", "3", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	fail_unless(num_client_errors == 2, NULL);
	fail_unless(num_client_connected == 0, NULL);
	fail_unless(num_client_disconnected == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_one_of_two_connect_error)
{
	char *testbed = setup_testbed("sync_easy_new");
	
	g_setenv("CONNECT_ERROR", "1", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	fail_unless(num_client_errors == 1, NULL);
	fail_unless(num_client_connected == 1, NULL);
	fail_unless(num_client_disconnected == 1, NULL);
	//fail_unless(num_member_sent_changes == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_two_of_three_connect_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("CONNECT_ERROR", "5", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	fail_unless(num_client_errors == 2, NULL);
	fail_unless(num_client_connected == 1, NULL);
	fail_unless(num_client_disconnected == 1, NULL);
	//fail_unless(num_member_sent_changes == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_two_of_three_connect_error2)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("CONNECT_ERROR", "6", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	fail_unless(num_client_errors == 2, NULL);
	fail_unless(num_client_connected == 1, NULL);
	fail_unless(num_client_disconnected == 1, NULL);
	//fail_unless(num_member_sent_changes == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_three_of_three_connect_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("CONNECT_ERROR", "7", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	fail_unless(num_client_errors == 3, NULL);
	fail_unless(num_client_connected == 0, NULL);
	fail_unless(num_client_disconnected == 0, NULL);
	//fail_unless(num_member_sent_changes == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_one_of_three_connect_error)
{
	char *testbed = setup_testbed("multisync_easy_new");

	g_setenv("CONNECT_ERROR", "2", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	fail_unless(num_client_errors == 1, NULL);
	fail_unless(num_client_connected == 2, NULL);
	fail_unless(num_client_disconnected == 2, NULL);
	//fail_unless(num_member_sent_changes == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_no_connect_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("CONNECT_ERROR", "0", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(synchronize_once(engine, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	//fail_unless(num_member_sent_changes == 3, NULL);
	fail_unless(num_engine_errors == 0, NULL);
	fail_unless(num_engine_successful == 1, NULL);
	
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" == \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" == \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_single_connect_timeout)
{
	char *testbed = setup_testbed("sync_easy_new");
	
	g_setenv("CONNECT_TIMEOUT", "2", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	fail_unless(num_client_errors == 1, NULL);
	fail_unless(num_client_connected == 1, NULL);
	fail_unless(num_client_disconnected == 1, NULL);
	//fail_unless(num_member_sent_changes == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_dual_connect_timeout)
{
	char *testbed = setup_testbed("sync_easy_new");
	
	g_setenv("CONNECT_TIMEOUT", "3", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	fail_unless(num_client_errors == 2, NULL);
	fail_unless(num_client_connected == 0, NULL);
	fail_unless(num_client_disconnected == 0, NULL);
	//fail_unless(num_member_sent_changes == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_one_of_three_timeout)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("CONNECT_TIMEOUT", "2", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	fail_unless(num_client_errors == 1, NULL);
	fail_unless(num_client_connected == 2, NULL);
	fail_unless(num_client_disconnected == 2, NULL);
	//fail_unless(num_member_sent_changes == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_timeout_and_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("CONNECT_TIMEOUT", "2", TRUE);
	g_setenv("CONNECT_ERROR", "4", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	fail_unless(num_client_errors == 2, NULL);
	fail_unless(num_client_connected == 1, NULL);
	fail_unless(num_client_disconnected == 1, NULL);
	//fail_unless(num_member_sent_changes == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_single_get_changes_error)
{
	char *testbed = setup_testbed("sync_easy_conflict");
	
	g_setenv("GET_CHANGES_ERROR", "2", TRUE);
	g_setenv("NO_COMMITTED_ALL_CHECK", "1", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 2, NULL);
	fail_unless(num_client_disconnected == 2, NULL);
	//fail_unless(num_member_get_changes_errors == 1, NULL);
	fail_unless(num_client_written == 0, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_dual_get_changes_error)
{
	char *testbed = setup_testbed("sync_easy_conflict");
	
	g_setenv("GET_CHANGES_ERROR", "3", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 2, NULL);
	fail_unless(num_client_disconnected == 2, NULL);
	//fail_unless(num_member_sent_changes == 0, NULL);
	fail_unless(num_client_read == 0, NULL);
	fail_unless(num_client_written == 0, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_two_of_three_get_changes_error)
{
	char *testbed = setup_testbed("multisync_conflict_data_choose2");
	
	g_setenv("GET_CHANGES_ERROR", "5", TRUE);
	g_setenv("NO_COMMITTED_ALL_CHECK", "1", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	fail_unless(num_client_written == 0, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_one_of_three_get_changes_error)
{
	char *testbed = setup_testbed("multisync_conflict_data_choose2");
	
	g_setenv("GET_CHANGES_ERROR", "1", TRUE);
	g_setenv("NO_COMMITTED_ALL_CHECK", "1", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	//fail_unless(num_member_get_changes_errors == 1, NULL);
	fail_unless(num_client_written == 0, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_one_of_three_get_changes_timeout)
{
	char *testbed = setup_testbed("multisync_conflict_data_choose2");
	
	g_setenv("GET_CHANGES_TIMEOUT", "1", TRUE);
	g_setenv("NO_COMMITTED_ALL_CHECK", "1", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	//fail_unless(num_member_get_changes_errors == 1, NULL);
	//fail_unless(num_member_sent_changes == 2, NULL);
	fail_unless(num_client_read == 2, NULL);
	fail_unless(num_client_written == 0, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_get_changes_timeout_and_error)
{
	char *testbed = setup_testbed("multisync_conflict_data_choose2");
	
	g_setenv("GET_CHANGES_TIMEOUT", "3", TRUE);
	g_setenv("GET_CHANGES_ERROR", "4", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	//fail_unless(num_member_sent_changes == 0, NULL);
	fail_unless(num_client_read == 0, NULL);
	fail_unless(num_client_written == 0, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_get_changes_timeout_sleep)
{
	char *testbed = setup_testbed("multisync_conflict_data_choose2");
	
	g_setenv("GET_CHANGES_TIMEOUT2", "7", TRUE);
	g_setenv("NO_COMMITTED_ALL_CHECK", "1", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	mark_point();
	osync_error_unref(&error);
	mark_point();
	osync_engine_finalize(engine, &error);
	mark_point();
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	//fail_unless(num_member_sent_changes == 0, NULL);
	fail_unless(num_client_read == 0, NULL);
	fail_unless(num_client_written == 0, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_single_commit_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("COMMIT_ERROR", "4", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_mappingstatus_callback(engine, mapping_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	mark_point();
	osync_error_unref(&error);
	mark_point();
	osync_engine_finalize(engine, &error);
	mark_point();
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	//fail_unless(num_member_sent_changes == 3, NULL);
	fail_unless(num_client_read == 1, NULL);
	fail_unless(num_client_written == 1, NULL);
	//fail_unless(num_written_errors == 1, NULL);
	fail_unless(num_mapping_errors == 1, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" == \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" != \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_dual_commit_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("COMMIT_ERROR", "6", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_mappingstatus_callback(engine, mapping_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	mark_point();
	osync_error_unref(&error);
	mark_point();
	osync_engine_finalize(engine, &error);
	mark_point();
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	//fail_unless(num_member_sent_changes == 3, NULL);
	fail_unless(num_client_read == 1, NULL);
	fail_unless(num_client_written == 0, NULL);
	//fail_unless(num_written_errors == 2, NULL);
	fail_unless(num_mapping_errors == 2, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" != \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_single_commit_timeout)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("COMMIT_TIMEOUT", "4", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_mappingstatus_callback(engine, mapping_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	mark_point();
	osync_error_unref(&error);
	mark_point();
	osync_engine_finalize(engine, &error);
	mark_point();
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	//fail_unless(num_member_sent_changes == 3, NULL);
	fail_unless(num_client_read == 1, NULL);
	fail_unless(num_client_written == 1, NULL);
	//fail_unless(num_written_errors == 1, NULL);
	fail_unless(num_mapping_errors == 1, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" == \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" != \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_dual_commit_timeout)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("COMMIT_TIMEOUT", "6", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_mappingstatus_callback(engine, mapping_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	mark_point();
	osync_error_unref(&error);
	mark_point();
	osync_engine_finalize(engine, &error);
	mark_point();
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	//fail_unless(num_member_sent_changes == 3, NULL);
	fail_unless(num_client_read == 1, NULL);
	fail_unless(num_client_written == 0, NULL);
	//fail_unless(num_written_errors == 2, NULL);
	fail_unless(num_mapping_errors == 2, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" != \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_commit_timeout_and_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("COMMIT_TIMEOUT", "4", TRUE);
	g_setenv("COMMIT_ERROR", "2", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_mappingstatus_callback(engine, mapping_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	mark_point();
	osync_error_unref(&error);
	mark_point();
	osync_engine_finalize(engine, &error);
	mark_point();
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	//fail_unless(num_member_sent_changes == 3, NULL);
	fail_unless(num_client_read == 1, NULL);
	fail_unless(num_client_written == 0, NULL);
	//fail_unless(num_written_errors == 2, NULL);
	fail_unless(num_mapping_errors == 2, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" != \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_commit_timeout_and_error2)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("COMMIT_TIMEOUT", "2", TRUE);
	g_setenv("COMMIT_ERROR", "4", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_mappingstatus_callback(engine, mapping_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	mark_point();
	osync_error_unref(&error);
	mark_point();
	osync_engine_finalize(engine, &error);
	mark_point();
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	//fail_unless(num_member_sent_changes == 3, NULL);
	fail_unless(num_client_read == 1, NULL);
	fail_unless(num_client_written == 0, NULL);
	//fail_unless(num_written_errors == 2, NULL);
	fail_unless(num_mapping_errors == 2, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" != \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_commit_error_modify)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_mappingstatus_callback(engine, mapping_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(synchronize_once(engine, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	g_setenv("COMMIT_TIMEOUT", "2", TRUE);
	g_setenv("COMMIT_ERROR", "4", TRUE);
	
	g_usleep(2*G_USEC_PER_SEC);
	
	osync_testing_system_abort("cp newdata2 data1/testdata");
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	mark_point();
	osync_error_unref(&error);
	mark_point();
	osync_engine_finalize(engine, &error);
	mark_point();
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	//fail_unless(num_member_sent_changes == 3, NULL);
	fail_unless(num_client_read == 1, NULL);
	fail_unless(num_client_written == 0, NULL);
	//fail_unless(num_written_errors == 2, NULL);
	fail_unless(num_mapping_errors == 2, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" != \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data2 data3)\" == \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_commit_error_delete)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_mappingstatus_callback(engine, mapping_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(synchronize_once(engine, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	g_setenv("COMMIT_TIMEOUT", "2", TRUE);
	g_setenv("COMMIT_ERROR", "4", TRUE);
	
	g_usleep(2*G_USEC_PER_SEC);
	
	osync_testing_system_abort("rm -f data1/testdata");
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	mark_point();
	osync_error_unref(&error);
	mark_point();
	osync_engine_finalize(engine, &error);
	mark_point();
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	//fail_unless(num_member_sent_changes == 3, NULL);
	fail_unless(num_client_read == 1, NULL);
	fail_unless(num_client_written == 0, NULL);
	//fail_unless(num_written_errors == 2, NULL);
	fail_unless(num_mapping_errors == 2, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" != \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data2 data3)\" == \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_committed_all_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("COMMITTED_ALL_ERROR", "3", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_mappingstatus_callback(engine, mapping_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	mark_point();
	osync_error_unref(&error);
	mark_point();
	osync_engine_finalize(engine, &error);
	mark_point();
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	//fail_unless(num_member_sent_changes == 3, NULL);
	fail_unless(num_client_read == 1, NULL);
	fail_unless(num_client_written == 2, NULL);
	//fail_unless(num_written_errors == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" = \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_committed_all_batch_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("BATCH_COMMIT", "7", TRUE);
	g_setenv("COMMITTED_ALL_ERROR", "3", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_mappingstatus_callback(engine, mapping_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	mark_point();
	osync_error_unref(&error);
	mark_point();
	osync_engine_finalize(engine, &error);
	mark_point();
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	//fail_unless(num_member_sent_changes == 3, NULL);
	fail_unless(num_client_read == 1, NULL);
	fail_unless(num_client_written == 2, NULL);
	//fail_unless(num_written_errors == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" = \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_single_sync_done_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("SYNC_DONE_ERROR", "4", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_mappingstatus_callback(engine, mapping_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	mark_point();
	osync_error_unref(&error);
	mark_point();
	osync_engine_finalize(engine, &error);
	mark_point();
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	//fail_unless(num_member_sent_changes == 3, NULL);
	fail_unless(num_client_read == 1, NULL);
	fail_unless(num_client_written == 2, NULL);
	//fail_unless(num_written_errors == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	//fail_unless(num_member_sync_done_errors == 1, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" == \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" == \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_dual_sync_done_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("SYNC_DONE_ERROR", "6", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_mappingstatus_callback(engine, mapping_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	mark_point();
	osync_error_unref(&error);
	mark_point();
	osync_engine_finalize(engine, &error);
	mark_point();
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	//fail_unless(num_member_sent_changes == 3, NULL);
	fail_unless(num_client_read == 1, NULL);
	fail_unless(num_client_written == 2, NULL);
	//fail_unless(num_written_errors == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	//fail_unless(num_member_sync_done_errors == 2, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" == \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" == \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_triple_sync_done_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("SYNC_DONE_ERROR", "7", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_mappingstatus_callback(engine, mapping_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	mark_point();
	osync_error_unref(&error);
	mark_point();
	osync_engine_finalize(engine, &error);
	mark_point();
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	//fail_unless(num_member_sent_changes == 3, NULL);
	fail_unless(num_client_read == 1, NULL);
	fail_unless(num_client_written == 2, NULL);
	//fail_unless(num_written_errors == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	//fail_unless(num_member_sync_done_errors == 3, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" == \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" == \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_single_sync_done_timeout)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("SYNC_DONE_TIMEOUT", "4", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_mappingstatus_callback(engine, mapping_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	mark_point();
	osync_error_unref(&error);
	mark_point();
	osync_engine_finalize(engine, &error);
	mark_point();
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	//fail_unless(num_member_sent_changes == 3, NULL);
	fail_unless(num_client_read == 1, NULL);
	fail_unless(num_client_written == 2, NULL);
	//fail_unless(num_written_errors == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	//fail_unless(num_member_sync_done_errors == 1, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" == \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" == \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_dual_sync_done_timeout)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("SYNC_DONE_TIMEOUT", "6", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_mappingstatus_callback(engine, mapping_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	mark_point();
	osync_error_unref(&error);
	mark_point();
	osync_engine_finalize(engine, &error);
	mark_point();
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	//fail_unless(num_member_sent_changes == 3, NULL);
	fail_unless(num_client_read == 1, NULL);
	fail_unless(num_client_written == 2, NULL);
	//fail_unless(num_written_errors == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	//fail_unless(num_member_sync_done_errors == 2, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" == \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" == \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_sync_done_timeout_and_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("SYNC_DONE_TIMEOUT", "5", TRUE);
	g_setenv("SYNC_DONE_ERROR", "2", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_mappingstatus_callback(engine, mapping_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	mark_point();
	osync_error_unref(&error);
	mark_point();
	osync_engine_finalize(engine, &error);
	mark_point();
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	//fail_unless(num_member_sent_changes == 3, NULL);
	fail_unless(num_client_read == 1, NULL);
	fail_unless(num_client_written == 2, NULL);
	//fail_unless(num_written_errors == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	//fail_unless(num_member_sync_done_errors == 3, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" == \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" == \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_single_disconnect_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("DISCONNECT_ERROR", "4", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_mappingstatus_callback(engine, mapping_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(synchronize_once(engine, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	mark_point();
	osync_error_unref(&error);
	mark_point();
	osync_engine_finalize(engine, &error);
	mark_point();
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 1, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 2, NULL);
	//fail_unless(num_member_sent_changes == 3, NULL);
	fail_unless(num_client_read == 1, NULL);
	fail_unless(num_client_written == 2, NULL);
	//fail_unless(num_written_errors == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	//fail_unless(num_member_sync_done_errors == 0, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 0, NULL);
	fail_unless(num_engine_successful == 1, NULL);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" == \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" == \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_dual_disconnect_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("DISCONNECT_ERROR", "6", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_mappingstatus_callback(engine, mapping_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(synchronize_once(engine, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	mark_point();
	osync_error_unref(&error);
	mark_point();
	osync_engine_finalize(engine, &error);
	mark_point();
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 2, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 1, NULL);
	//fail_unless(num_member_sent_changes == 3, NULL);
	fail_unless(num_client_read == 1, NULL);
	fail_unless(num_client_written == 2, NULL);
	//fail_unless(num_written_errors == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	//fail_unless(num_member_sync_done_errors == 0, NULL);
	fail_unless(num_engine_errors == 0, NULL);
	fail_unless(num_engine_successful == 1, NULL);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" == \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" == \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_triple_disconnect_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("DISCONNECT_ERROR", "7", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_mappingstatus_callback(engine, mapping_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(synchronize_once(engine, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	mark_point();
	osync_error_unref(&error);
	mark_point();
	osync_engine_finalize(engine, &error);
	mark_point();
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 3, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 0, NULL);
	//fail_unless(num_member_sent_changes == 3, NULL);
	fail_unless(num_client_read == 1, NULL);
	fail_unless(num_client_written == 2, NULL);
	//fail_unless(num_written_errors == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	//fail_unless(num_member_sync_done_errors == 0, NULL);
	fail_unless(num_engine_errors == 0, NULL);
	fail_unless(num_engine_successful == 1, NULL);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" == \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" == \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_single_disconnect_timeout)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("DISCONNECT_TIMEOUT", "4", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_mappingstatus_callback(engine, mapping_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(synchronize_once(engine, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	mark_point();
	osync_error_unref(&error);
	mark_point();
	osync_engine_finalize(engine, &error);
	mark_point();
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 1, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 2, NULL);
	//fail_unless(num_member_sent_changes == 3, NULL);
	fail_unless(num_client_read == 1, NULL);
	fail_unless(num_client_written == 2, NULL);
	//fail_unless(num_written_errors == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	//fail_unless(num_member_sync_done_errors == 0, NULL);
	fail_unless(num_engine_errors == 0, NULL);
	fail_unless(num_engine_successful == 1, NULL);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" == \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" == \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_dual_disconnect_timeout)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("DISCONNECT_TIMEOUT", "6", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_mappingstatus_callback(engine, mapping_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(synchronize_once(engine, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	mark_point();
	osync_error_unref(&error);
	mark_point();
	osync_engine_finalize(engine, &error);
	mark_point();
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 2, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 1, NULL);
	//fail_unless(num_member_sent_changes == 3, NULL);
	fail_unless(num_client_read == 1, NULL);
	fail_unless(num_client_written == 2, NULL);
	//fail_unless(num_written_errors == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	//fail_unless(num_member_sync_done_errors == 0, NULL);
	fail_unless(num_engine_errors == 0, NULL);
	fail_unless(num_engine_successful == 1, NULL);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" == \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" == \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_disconnect_timeout_and_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("DISCONNECT_TIMEOUT", "5", TRUE);
	g_setenv("DISCONNECT_ERROR", "2", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_mappingstatus_callback(engine, mapping_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(synchronize_once(engine, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	mark_point();
	osync_error_unref(&error);
	mark_point();
	osync_engine_finalize(engine, &error);
	mark_point();
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 3, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 0, NULL);
	//fail_unless(num_member_sent_changes == 3, NULL);
	fail_unless(num_client_read == 1, NULL);
	fail_unless(num_client_written == 2, NULL);
	//fail_unless(num_written_errors == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	//fail_unless(num_member_sync_done_errors == 0, NULL);
	fail_unless(num_engine_errors == 0, NULL);
	fail_unless(num_engine_successful == 1, NULL);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" == \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" == \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_UNUSED START_TEST (sync_error_get_changes_disconnect_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	g_setenv("DISCONNECT_TIMEOUT", "1", TRUE);
	g_setenv("DISCONNECT_ERROR", "2", TRUE);
	g_setenv("GET_CHANGES_TIMEOUT", "6", TRUE);
	g_setenv("NO_COMMITTED_ALL_CHECK", "1", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_schemadir(group, testbed);
	fail_unless(osync_group_load(group, "configs/group", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	osync_engine_set_memberstatus_callback(engine, member_status, NULL);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	osync_engine_set_changestatus_callback(engine, entry_status, NULL);
	osync_engine_set_mappingstatus_callback(engine, mapping_status, NULL);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	fail_unless(num_client_errors == 2, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 1, NULL);
	//fail_unless(num_member_sent_changes == 1, NULL);
	fail_unless(num_client_read == 1, NULL);
	fail_unless(num_client_written == 0, NULL);
	//fail_unless(num_written_errors == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	//fail_unless(num_conflicts == 0, NULL);
	//fail_unless(num_member_sync_done_errors == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	mark_point();
	osync_error_unref(&error);
	mark_point();
	osync_engine_finalize(engine, &error);
	mark_point();
	osync_engine_unref(engine);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" != \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_TESTCASE_START("sync_error")
OSYNC_TESTCASE_ADD(sync_error_single_init_error_noerror)

#if 0
/* Disabled as not ported see http://opensync.org/ticket/980 */
OSYNC_TESTCASE_ADD(sync_error_single_init_error)
OSYNC_TESTCASE_ADD(sync_error_dual_connect_error)
OSYNC_TESTCASE_ADD(sync_error_one_of_two_connect_error)
OSYNC_TESTCASE_ADD(sync_error_two_of_three_connect_error)
OSYNC_TESTCASE_ADD(sync_error_two_of_three_connect_error2)
OSYNC_TESTCASE_ADD(sync_error_three_of_three_connect_error)
OSYNC_TESTCASE_ADD(sync_error_one_of_three_connect_error)
OSYNC_TESTCASE_ADD(sync_error_no_connect_error)
OSYNC_TESTCASE_ADD(sync_error_single_connect_timeout)
OSYNC_TESTCASE_ADD(sync_error_dual_connect_timeout)
OSYNC_TESTCASE_ADD(sync_error_one_of_three_timeout)
OSYNC_TESTCASE_ADD(sync_error_timeout_and_error)
OSYNC_TESTCASE_ADD(sync_error_single_get_changes_error)
OSYNC_TESTCASE_ADD(sync_error_dual_get_changes_error)
OSYNC_TESTCASE_ADD(sync_error_two_of_three_get_changes_error)
OSYNC_TESTCASE_ADD(sync_error_one_of_three_get_changes_error)
OSYNC_TESTCASE_ADD(sync_error_one_of_three_get_changes_timeout)
OSYNC_TESTCASE_ADD(sync_error_get_changes_timeout_and_error)
OSYNC_TESTCASE_ADD(sync_error_get_changes_timeout_sleep)
OSYNC_TESTCASE_ADD(sync_error_single_commit_error)
OSYNC_TESTCASE_ADD(sync_error_dual_commit_error)
OSYNC_TESTCASE_ADD(sync_error_single_commit_timeout)
OSYNC_TESTCASE_ADD(sync_error_dual_commit_timeout)
OSYNC_TESTCASE_ADD(sync_error_commit_timeout_and_error)
OSYNC_TESTCASE_ADD(sync_error_commit_timeout_and_error2)
OSYNC_TESTCASE_ADD(sync_error_commit_error_modify)
OSYNC_TESTCASE_ADD(sync_error_commit_error_delete)
OSYNC_TESTCASE_ADD(sync_error_committed_all_error)
OSYNC_TESTCASE_ADD(sync_error_committed_all_batch_error)
OSYNC_TESTCASE_ADD(sync_error_single_sync_done_error)
OSYNC_TESTCASE_ADD(sync_error_dual_sync_done_error)
OSYNC_TESTCASE_ADD(sync_error_triple_sync_done_error)
OSYNC_TESTCASE_ADD(sync_error_single_sync_done_timeout)
OSYNC_TESTCASE_ADD(sync_error_dual_sync_done_timeout)
OSYNC_TESTCASE_ADD(sync_error_sync_done_timeout_and_error)
OSYNC_TESTCASE_ADD(sync_error_single_disconnect_error)
OSYNC_TESTCASE_ADD(sync_error_dual_disconnect_error)
OSYNC_TESTCASE_ADD(sync_error_triple_disconnect_error)
OSYNC_TESTCASE_ADD(sync_error_single_disconnect_timeout)
OSYNC_TESTCASE_ADD(sync_error_dual_disconnect_timeout)
OSYNC_TESTCASE_ADD(sync_error_disconnect_timeout_and_error)
OSYNC_TESTCASE_ADD(sync_error_get_changes_disconnect_error)
#endif

OSYNC_TESTCASE_END

