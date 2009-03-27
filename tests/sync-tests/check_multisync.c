#include "support.h"

#include "opensync/group/opensync_group_internals.h"
#include "opensync/engine/opensync_engine_internals.h"

START_TEST (multisync_easy_new)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
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
	
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_engine_synchronize_and_block(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_engine_finalize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_unref(engine);
	
	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	
	/* Client checks */
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_main_connected == 3, NULL);
	fail_unless(num_client_read == 3, NULL);
	fail_unless(num_client_main_read == 3, NULL);
	fail_unless(num_client_written == 3, NULL);
	fail_unless(num_client_main_written == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	fail_unless(num_client_main_disconnected == 3, NULL);
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_sync_done == 3, NULL);
	fail_unless(num_client_main_sync_done == 3, NULL);
	
	/* Engine checks */
	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_errors == 0, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_sync_done == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_engine_successful == 1, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);
	fail_unless(num_engine_prev_unclean == 0, NULL);

	/* Change checks */
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_change_written == 2, NULL);
	fail_unless(num_change_error == 0, NULL);
	
	char *path = g_strdup_printf("%s/configs/group/archive.db", testbed);
	OSyncMappingTable *maptable = mappingtable_load(path, "mockobjtype1", 1);
	g_free(path);
	check_mapping(maptable, 1, 1, 3, "testdata");
	check_mapping(maptable, 2, 1, 3, "testdata");
	check_mapping(maptable, 3, 1, 3, "testdata");
	osync_mapping_table_close(maptable);
	osync_mapping_table_unref(maptable);
 
	path = g_strdup_printf("%s/configs/group/1/hashtable.db", testbed);
	OSyncHashTable *table = hashtable_load(path, "mockobjtype1", 1);
	g_free(path);
	check_hash(table, "testdata");
	osync_hashtable_unref(table);
	
	path = g_strdup_printf("%s/configs/group/2/hashtable.db", testbed);
	table = hashtable_load(path, "mockobjtype1", 1);
	g_free(path);
	check_hash(table, "testdata");
	osync_hashtable_unref(table);
	
	path = g_strdup_printf("%s/configs/group/3/hashtable.db", testbed);
	table = hashtable_load(path, "mockobjtype1", 1);
	g_free(path);
	check_hash(table, "testdata");
	osync_hashtable_unref(table);

	g_free(formatdir);
	g_free(plugindir);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (multisync_dual_new)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);

	osync_testing_system_abort("cp data1/testdata data2/");
	
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
	
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_engine_synchronize_and_block(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	
	/* Client checks */
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_main_connected == 3, NULL);
	fail_unless(num_client_read == 3, NULL);
	fail_unless(num_client_main_read == 3, NULL);
	fail_unless(num_client_written == 3, NULL);
	fail_unless(num_client_main_written == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	fail_unless(num_client_main_disconnected == 3, NULL);
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_sync_done == 3, NULL);
	fail_unless(num_client_main_sync_done == 3, NULL);
	
	/* Engine checks */
	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_errors == 0, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_sync_done == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_engine_successful == 1, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);
	fail_unless(num_engine_prev_unclean == 0, NULL);

	/* Change checks */
	fail_unless(num_change_read == 2, NULL);
	fail_unless(num_change_written == 1, NULL);
	fail_unless(num_change_error == 0, NULL);
	
	char *path = g_strdup_printf("%s/configs/group/archive.db", testbed);
	OSyncMappingTable *maptable = mappingtable_load(path, "mockobjtype1", 1);
	g_free(path);
	check_mapping(maptable, 1, 1, 3, "testdata");
	check_mapping(maptable, 2, 1, 3, "testdata");
	check_mapping(maptable, 3, 1, 3, "testdata");
	osync_mapping_table_close(maptable);
	osync_mapping_table_unref(maptable);
 
	path = g_strdup_printf("%s/configs/group/1/hashtable.db", testbed);
	OSyncHashTable *table = hashtable_load(path, "mockobjtype1", 1);
	g_free(path);
	check_hash(table, "testdata");
	osync_hashtable_unref(table);
	
	path = g_strdup_printf("%s/configs/group/2/hashtable.db", testbed);
	table = hashtable_load(path, "mockobjtype1", 1);
	g_free(path);
	check_hash(table, "testdata");
	osync_hashtable_unref(table);
	
	path = g_strdup_printf("%s/configs/group/3/hashtable.db", testbed);
	table = hashtable_load(path, "mockobjtype1", 1);
	g_free(path);
	check_hash(table, "testdata");
	osync_hashtable_unref(table);

	osync_testing_file_remove("data2/testdata");

	fail_unless(synchronize_once(engine, &error), NULL);
	fail_unless(osync_engine_finalize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_unref(engine);

	path = g_strdup_printf("%s/configs/group/archive.db", testbed);
	maptable = mappingtable_load(path, "mockobjtype1", 0);
	g_free(path);
	osync_mapping_table_close(maptable);
	osync_mapping_table_unref(maptable);

	path = g_strdup_printf("%s/configs/group/1/hashtable.db", testbed);
	table = hashtable_load(path, "mockobjtype1", 0);
	osync_hashtable_unref(table);
	g_free(path);

	path = g_strdup_printf("%s/configs/group/2/hashtable.db", testbed);
	table = hashtable_load(path, "mockobjtype1", 0);
	osync_hashtable_unref(table);
	g_free(path);

	path = g_strdup_printf("%s/configs/group/3/hashtable.db", testbed);
	table = hashtable_load(path, "mockobjtype1", 0);
	osync_hashtable_unref(table);
	g_free(path);

	fail_unless(osync_testing_directory_is_empty("data1"), NULL);
	fail_unless(osync_testing_directory_is_empty("data2"), NULL);
	fail_unless(osync_testing_directory_is_empty("data3"), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (multisync_triple_new)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);


	osync_testing_system_abort("cp data1/testdata data2/");
	osync_testing_system_abort("cp data1/testdata data3/");
	
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
	
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_engine_synchronize_and_block(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	
	/* Client checks */
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_main_connected == 3, NULL);
	fail_unless(num_client_read == 3, NULL);
	fail_unless(num_client_main_read == 3, NULL);
	fail_unless(num_client_written == 3, NULL);
	fail_unless(num_client_main_written == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	fail_unless(num_client_main_disconnected == 3, NULL);
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_sync_done == 3, NULL);
	fail_unless(num_client_main_sync_done == 3, NULL);
	
	/* Engine checks */
	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_errors == 0, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_sync_done == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_engine_successful == 1, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);
	fail_unless(num_engine_prev_unclean == 0, NULL);

	/* Change checks */
	fail_unless(num_change_read == 3, NULL);
	fail_unless(num_change_written == 0, NULL);
	fail_unless(num_change_error == 0, NULL);
	
	char *path = g_strdup_printf("%s/configs/group/archive.db", testbed);
	OSyncMappingTable *maptable = mappingtable_load(path, "mockobjtype1", 1);
	g_free(path);
	check_mapping(maptable, 1, 1, 3, "testdata");
	check_mapping(maptable, 2, 1, 3, "testdata");
	check_mapping(maptable, 3, 1, 3, "testdata");
	osync_mapping_table_close(maptable);
	osync_mapping_table_unref(maptable);
 
	path = g_strdup_printf("%s/configs/group/1/hashtable.db", testbed);
	OSyncHashTable *table = hashtable_load(path, "mockobjtype1", 1);
	g_free(path);
	check_hash(table, "testdata");
	osync_hashtable_unref(table);
	
	path = g_strdup_printf("%s/configs/group/2/hashtable.db", testbed);
	table = hashtable_load(path, "mockobjtype1", 1);
	g_free(path);
	check_hash(table, "testdata");
	osync_hashtable_unref(table);
	
	path = g_strdup_printf("%s/configs/group/3/hashtable.db", testbed);
	table = hashtable_load(path, "mockobjtype1", 1);
	g_free(path);
	check_hash(table, "testdata");
	osync_hashtable_unref(table);

	osync_testing_file_remove("data1/testdata");

	fail_unless(synchronize_once(engine, &error), NULL);
	fail_unless(osync_engine_finalize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_unref(engine);

	path = g_strdup_printf("%s/configs/group/archive.db", testbed);
	maptable = mappingtable_load(path, "mockobjtype1", 0);
	g_free(path);
	osync_mapping_table_close(maptable);
	osync_mapping_table_unref(maptable);

	path = g_strdup_printf("%s/configs/group/1/hashtable.db", testbed);
	table = hashtable_load(path, "mockobjtype1", 0);
	osync_hashtable_unref(table);
	g_free(path);

	path = g_strdup_printf("%s/configs/group/2/hashtable.db", testbed);
	table = hashtable_load(path, "mockobjtype1", 0);
	osync_hashtable_unref(table);
	g_free(path);

	path = g_strdup_printf("%s/configs/group/3/hashtable.db", testbed);
	table = hashtable_load(path, "mockobjtype1", 0);
	osync_hashtable_unref(table);
	g_free(path);

	fail_unless(osync_testing_directory_is_empty("data1"), NULL);
	fail_unless(osync_testing_directory_is_empty("data2"), NULL);
	fail_unless(osync_testing_directory_is_empty("data3"), NULL);

	destroy_testbed(testbed);
	
}
END_TEST


/* FIXME: port testcases, see ticket #981 */
OSyncEngine *setup_engine(const char *testbed)
{
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);		   
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
	g_free(formatdir);
	g_free(plugindir);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(1));
	
	return engine;
}

void hashtable_simple_load_and_check(const char *testbed, int group)
{
	char *path = g_strdup_printf("%s/configs/group/%d/hashtable.db", testbed, group);
	OSyncHashTable *table = hashtable_load(path, "mockobjtype1", 1);
	g_free(path);
	check_hash(table, "testdata");
	osync_hashtable_unref(table);
}

void hashtable_load_and_check(const char *testbed, int group, const char *uids[], uint num_uids)
{
	char *path = g_strdup_printf("%s/configs/group/%d/hashtable.db", testbed, group);
	OSyncHashTable *table = hashtable_load(path, "mockobjtype1", num_uids);
	g_free(path);
	uint i;
	for (i = 0; i < num_uids; i++) {
		check_hash(table, uids[i]);
	}
	osync_hashtable_unref(table);
}

void check_empty(const char *testbed)
{
	/* check the mapping table loads */
	char *path = g_strdup_printf("%s/configs/group/archive.db", testbed);
	OSyncMappingTable *maptable = mappingtable_load(path, "mockobjtype1", 0);
	g_free(path);
	osync_mapping_table_close(maptable);
	osync_mapping_table_unref(maptable);

	/* check the hash tables load */
	int group;
	for (group = 1; group <= 3; group++) {
		path = g_strdup_printf("%s/configs/group/%d/hashtable.db", testbed, group);
		OSyncHashTable *table = hashtable_load(path, "mockobjtype1", 0);
		g_free(path);
		osync_hashtable_unref(table);
	}

	/* make sure the data directories are empty */
 	fail_unless(!system("test \"x$(ls data1)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(ls data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(ls data3)\" = \"x\""), NULL);
}

void destroy_engine(OSyncEngine *engine)
{
	OSyncError *error = NULL;
	fail_unless(osync_engine_finalize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	osync_engine_unref(engine);
}

/* Sync the single item testdata from data 1 to data2 and data3
 * then change testdata in data3 */
START_TEST (multisync_easy_mod)
{
	char *testbed = setup_testbed("multisync_easy_new");
	OSyncError *error = NULL;
	OSyncEngine *engine = setup_engine(testbed);
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	mark_point();

	synchronize_once(engine, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	
	fail_unless(num_change_written == 2, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_engine_end_conflicts = 1, NULL);
	
	sleep(2);
	system("cp newdata data3/testdata");
	
	synchronize_once(engine, NULL);
	destroy_engine(engine);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	
	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_written == 2, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_change_error == 0, NULL);
		
	char *path = g_strdup_printf("%s/configs/group/archive.db", testbed);
	OSyncMappingTable *maptable = mappingtable_load(path, "mockobjtype1", 1);
	g_free(path);
	check_mapping(maptable, 1, 1, 3, "testdata");
	check_mapping(maptable, 2, 1, 3, "testdata");
	check_mapping(maptable, 3, 1, 3, "testdata");
	osync_mapping_table_close(maptable);
	osync_mapping_table_unref(maptable);
	
	hashtable_simple_load_and_check(testbed, 1);
	hashtable_simple_load_and_check(testbed, 2);
	hashtable_simple_load_and_check(testbed, 3);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (multisync_dual_mod)
{
	char *testbed = setup_testbed("multisync_easy_new");
	OSyncError *error = NULL;
	OSyncEngine *engine = setup_engine(testbed);
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	mark_point();

	synchronize_once(engine, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	
	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_written == 2, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);
	
	sleep(2);
	system("cp newdata data1/testdata");
	system("cp newdata data3/testdata");
	
	synchronize_once(engine, NULL);

	destroy_engine(engine);
	
	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	
	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_written == 1, NULL);
	fail_unless(num_change_read == 2, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);

	char *path = g_strdup_printf("%s/configs/group/archive.db", testbed);
	OSyncMappingTable *maptable = mappingtable_load(path, "mockobjtype1", 1);
	g_free(path);
	check_mapping(maptable, 1, 1, 3, "testdata");
	check_mapping(maptable, 2, 1, 3, "testdata");
	check_mapping(maptable, 3, 1, 3, "testdata");
	osync_mapping_table_close(maptable);
	osync_mapping_table_unref(maptable);

	hashtable_simple_load_and_check(testbed, 1);
	hashtable_simple_load_and_check(testbed, 2);
	hashtable_simple_load_and_check(testbed, 3);	
	
	destroy_testbed(testbed);
}
END_TEST




START_TEST (multisync_triple_mod)
{
	char *testbed = setup_testbed("multisync_easy_new");
	OSyncError *error = NULL;
	OSyncEngine *engine = setup_engine(testbed);
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	mark_point();

	synchronize_once(engine, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	
	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_written == 2, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);
	
	sleep(2);
	system("cp newdata data1/testdata");
	system("cp newdata data2/testdata");
	system("cp newdata data3/testdata");
	
	synchronize_once(engine, NULL);
	destroy_engine(engine);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	
	fail_unless(num_change_written == 0, NULL);
	fail_unless(num_change_read == 3, NULL);
	fail_unless(num_engine_end_conflicts = 1, NULL);
	
	char *path = g_strdup_printf("%s/configs/group/archive.db", testbed);
	OSyncMappingTable *maptable = mappingtable_load(path, "mockobjtype1", 1);
	g_free(path);
	check_mapping(maptable, 1, 1, 3, "testdata");
	check_mapping(maptable, 2, 1, 3, "testdata");
	check_mapping(maptable, 3, 1, 3, "testdata");
	osync_mapping_table_close(maptable);
	osync_mapping_table_unref(maptable);
	
	hashtable_simple_load_and_check(testbed, 1);
	hashtable_simple_load_and_check(testbed, 2);
	hashtable_simple_load_and_check(testbed, 3);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (multisync_easy_del)
{
	char *testbed = setup_testbed("multisync_easy_new");
	OSyncError *error = NULL;
	OSyncEngine *engine = setup_engine(testbed);
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);	
	
	synchronize_once(engine, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));

	system("rm -f data2/testdata");
	
	synchronize_once(engine, NULL);
	destroy_engine(engine);
	
	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_change_written == 2, NULL);
	
	check_empty(testbed);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (multisync_dual_del)
{
	char *testbed = setup_testbed("multisync_easy_new");
	OSyncError *error = NULL;
	OSyncEngine *engine = setup_engine(testbed);
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);	
	
	synchronize_once(engine, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));

	system("rm -f data1/testdata");
	system("rm -f data3/testdata");
	
	synchronize_once(engine, NULL);
	fail_unless(osync_engine_finalize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_unref(engine);
	
	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 2, NULL);
        fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_change_written == 1, NULL);
	
	check_empty(testbed);
	
	destroy_testbed(testbed);
}
END_TEST

/* Sync testdata from data1 to data2 and data3.
 * Then remove all testdata, should sync with 3 changes read
 * and no changes written */
START_TEST (multisync_triple_del)
{
	char *testbed = setup_testbed("multisync_easy_new");
	OSyncError *error = NULL;
	OSyncEngine *engine = setup_engine(testbed);
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);	
	
	synchronize_once(engine, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));

	system("rm -f data1/testdata");
	system("rm -f data2/testdata");
	system("rm -f data3/testdata");
	
	synchronize_once(engine, NULL);
	fail_unless(osync_engine_finalize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_unref(engine);
	
	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 3, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_change_written == 0, NULL);
	
	check_empty(testbed);
	
	destroy_testbed(testbed);
}
END_TEST


/* testdata in data1 and data2 are different */
START_TEST (multisync_conflict_data_choose)
{
	char *testbed = setup_testbed("multisync_conflict_data_choose");
	OSyncError *error = NULL;
	OSyncEngine *engine = setup_engine(testbed);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_first, GINT_TO_POINTER(2));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);	

	mark_point();

	synchronize_once(engine, NULL);

	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_written == 1, NULL);
	fail_unless(num_change_read == 2, NULL);
	fail_unless(num_mapping_conflicts == 1, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);
	
	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	
	char *path = g_strdup_printf("%s/configs/group/archive.db", testbed);
	OSyncMappingTable *maptable = mappingtable_load(path, "mockobjtype1", 1);
	g_free(path);
	check_mapping(maptable, 1, 1, 3, "testdata");
	check_mapping(maptable, 2, 1, 3, "testdata");
	check_mapping(maptable, 3, 1, 3, "testdata");
	osync_mapping_table_close(maptable);
	osync_mapping_table_unref(maptable);
	
	hashtable_simple_load_and_check(testbed, 1);
	hashtable_simple_load_and_check(testbed, 2);
	hashtable_simple_load_and_check(testbed, 3);
	
	system("rm -f data3/testdata");
	
	mark_point();

	synchronize_once(engine, NULL);

	destroy_engine(engine);
	
	check_empty(testbed);
	
	destroy_testbed(testbed);
}
END_TEST


/* testdata in data 1 and data3 are the same. data2 is different */
START_TEST (multisync_conflict_data_choose2)
{
	char *testbed = setup_testbed("multisync_conflict_data_choose2");
	OSyncError *error = NULL;
	OSyncEngine *engine = setup_engine(testbed);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_first, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);	

	mark_point();

	synchronize_once(engine, NULL);

	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 3, NULL);
	fail_unless(num_change_written == 1, "Num change written = '%u' not 1 or I have it wrong...\n", num_change_written);
	fail_unless(num_mapping_conflicts == 1, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	
	char *path = g_strdup_printf("%s/configs/group/archive.db", testbed);
	OSyncMappingTable *maptable = mappingtable_load(path, "mockobjtype1", 1);
	g_free(path);
	check_mapping(maptable, 1, 1, 3, "testdata");
	check_mapping(maptable, 2, 1, 3, "testdata");
	check_mapping(maptable, 3, 1, 3, "testdata");
	osync_mapping_table_close(maptable);
	osync_mapping_table_unref(maptable);
	
	hashtable_simple_load_and_check(testbed, 1);
	hashtable_simple_load_and_check(testbed, 2);
	hashtable_simple_load_and_check(testbed, 3);
	
	system("rm -f data3/testdata");
	
	mark_point();

	synchronize_once(engine, NULL);

	fail_unless(osync_engine_finalize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	osync_engine_unref(engine);
	
	check_empty(testbed);
	
	destroy_testbed(testbed);
}
END_TEST


/* Sync data1/testdata to all
 * remove data1/testdata and modify data3/testdata
 * using choose_modified conflict handler so data3 should end up eveywhere
 */
START_TEST (multisync_conflict_changetype_choose)
{
	char *testbed = setup_testbed("multisync_conflict_changetype_choose");
	OSyncError *error = NULL;
	OSyncEngine *engine = setup_engine(testbed);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(2));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);	

	mark_point();

	synchronize_once(engine, NULL);	

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	
	sleep(2);
	
	system("rm -f data1/testdata");
	system("cp newdata data3/testdata");
	
	synchronize_once(engine, NULL);

	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 2, NULL);
	fail_unless(num_mapping_conflicts == 1, NULL);
	fail_unless(num_change_written == 2, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	
	char *path = g_strdup_printf("%s/configs/group/archive.db", testbed);
	OSyncMappingTable *maptable = mappingtable_load(path, "mockobjtype1", 1);
	g_free(path);
	check_mapping(maptable, 1, 1, 3, "testdata");
	check_mapping(maptable, 2, 1, 3, "testdata");
	check_mapping(maptable, 3, 1, 3, "testdata");
	osync_mapping_table_close(maptable);
	osync_mapping_table_unref(maptable);
	
	hashtable_simple_load_and_check(testbed, 1);
	hashtable_simple_load_and_check(testbed, 2);
	hashtable_simple_load_and_check(testbed, 3);
	
	system("rm -f data1/testdata");
	
	mark_point();

	synchronize_once(engine, NULL);

	fail_unless(osync_engine_finalize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	osync_engine_unref(engine);

	check_empty(testbed);
	
	destroy_testbed(testbed);
}
END_TEST


/* As above but use conflict_handler_choose_first so all data should be deleted */
START_TEST (multisync_conflict_changetype_choose2)
{
	char *testbed = setup_testbed("multisync_conflict_changetype_choose");
	OSyncError *error = NULL;
	OSyncEngine *engine = setup_engine(testbed);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_first, GINT_TO_POINTER(2));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);	

	mark_point();

	synchronize_once(engine, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	
	sleep(2);
	
	system("rm -f data1/testdata");
	system("cp newdata data3/testdata");
	
	synchronize_once(engine, NULL);

	destroy_engine(engine);

	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 2, NULL);
	fail_unless(num_mapping_conflicts == 1, NULL);
	fail_unless(num_change_written == 2, NULL);
	
	check_empty(testbed);
	
	destroy_testbed(testbed);
}
END_TEST

/* Sync from data1/testdata to all
 * remove data1 and change data2 to newdata2 and data3 to newdata
 * conflict_handler_choose_modified picks the first modified so should get newdata2
 * everywhere.
 */
START_TEST (multisync_conflict_hybrid_choose)
{
	char *testbed = setup_testbed("multisync_conflict_changetype_choose");
	OSyncError *error = NULL;
	OSyncEngine *engine = setup_engine(testbed);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);	

	mark_point();

	synchronize_once(engine, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	
	sleep(2);
	
	sleep(2);
	
	system("rm -f data1/testdata");
	system("cp newdata data3/testdata");
	system("cp newdata2 data2/testdata");
	
	synchronize_once(engine, NULL);

	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 3, NULL);
	fail_unless(num_mapping_conflicts == 1, NULL);
	fail_unless(num_change_written == 2, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	fail_unless(!system("test \"x$(diff newdata2 data2/testdata)\" = \"x\""), NULL);
	
	char *path = g_strdup_printf("%s/configs/group/archive.db", testbed);
	OSyncMappingTable *maptable = mappingtable_load(path, "mockobjtype1", 1);
	g_free(path);
	check_mapping(maptable, 1, 1, 3, "testdata");
	check_mapping(maptable, 2, 1, 3, "testdata");
	check_mapping(maptable, 3, 1, 3, "testdata");
	osync_mapping_table_close(maptable);
	osync_mapping_table_unref(maptable);
	
	hashtable_simple_load_and_check(testbed, 1);
	hashtable_simple_load_and_check(testbed, 2);
	hashtable_simple_load_and_check(testbed, 3);
	
	system("rm -f data1/testdata");
	
	mark_point();

	synchronize_once(engine, NULL);
	destroy_engine(engine);
	
	check_empty(testbed);
	
	destroy_testbed(testbed);
}
END_TEST

/* Sync data1/testdata to all
 * Remove data1/testdata and change the other 2
 * deleted conflict handler should delete all
 */
START_TEST (multisync_conflict_hybrid_choose2)
{
	char *testbed = setup_testbed("multisync_conflict_changetype_choose");
	OSyncError *error = NULL;
	OSyncEngine *engine = setup_engine(testbed);
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_deleted, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);	

	mark_point();

	synchronize_once(engine, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	
	sleep(2);
	
	system("rm -f data1/testdata");
	system("cp newdata data3/testdata");
	system("cp newdata2 data2/testdata");
	
	synchronize_once(engine, NULL);
	destroy_engine(engine);

	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 3, NULL);
	fail_unless(num_mapping_conflicts == 1, NULL);
	fail_unless(num_change_written == 2, NULL);

	check_empty(testbed);
	
	destroy_testbed(testbed);
}
END_TEST

/* data1 and data2 are different
 * using duplicate so should end up with 2 entries in all 3 places
 * 5 writes are expected as data2/testdata gets moved to data2/testdata-dupe
 */
START_TEST (multisync_conflict_data_duplicate)
{
	char *testbed = setup_testbed("multisync_conflict_data_choose");
	OSyncError *error = NULL;
	OSyncEngine *engine = setup_engine(testbed);
	osync_engine_set_conflict_callback(engine, conflict_handler_duplicate, GINT_TO_POINTER(2));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);	

	mark_point();

	synchronize_once(engine, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));

	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 2, NULL);
	fail_unless(num_mapping_conflicts == 1, NULL);
	fail_unless(num_change_written == 5, NULL);

	char *path = g_strdup_printf("%s/configs/group/archive.db", testbed);
	OSyncMappingTable *maptable = mappingtable_load(path, "mockobjtype1", 2);
	g_free(path);
	check_mapping(maptable, 1, -1, 3, "testdata");
	check_mapping(maptable, 2, -1, 3, "testdata");
	check_mapping(maptable, 3, -1, 3, "testdata");
	check_mapping(maptable, 1, -1, 3, "testdata-dupe");
	check_mapping(maptable, 2, -1, 3, "testdata-dupe");
	check_mapping(maptable, 3, -1, 3, "testdata-dupe");
	osync_mapping_table_close(maptable);
	osync_mapping_table_unref(maptable);

	unsigned int num_uids = 2;
	const char *uids[] = {"testdata", "testdata-dupe"};
	hashtable_load_and_check(testbed, 1, uids, num_uids);
	hashtable_load_and_check(testbed, 2, uids, num_uids);
	hashtable_load_and_check(testbed, 3, uids, num_uids);

	
	system("rm -f data3/testdata");
	
	synchronize_once(engine, NULL);
	
	path = g_strdup_printf("%s/configs/group/archive.db", testbed);
	maptable = mappingtable_load(path, "mockobjtype1", 1);
	g_free(path);
	check_mapping(maptable, 1, -1, 3, "testdata-dupe");
	check_mapping(maptable, 2, -1, 3, "testdata-dupe");
	check_mapping(maptable, 3, -1, 3, "testdata-dupe");
	osync_mapping_table_close(maptable);
	osync_mapping_table_unref(maptable);

	num_uids = 1;
	uids[0] = "testdata-dupe";
	hashtable_load_and_check(testbed, 1, uids, num_uids);
	hashtable_load_and_check(testbed, 2, uids, num_uids);
	hashtable_load_and_check(testbed, 3, uids, num_uids);	

	
	system("rm -f data2/testdata-dupe");
	
	mark_point();

	synchronize_once(engine, NULL);
	destroy_engine(engine);
	
	check_empty(testbed);
	
	destroy_testbed(testbed);
}
END_TEST


/* As above but with 3 conflicts, one from each data member
 * should get 3+3+2 writes
 */
START_TEST (multisync_conflict_data_duplicate2)
{
	char *testbed = setup_testbed("multisync_conflict_data_duplicate2");
	OSyncError *error = NULL;
	OSyncEngine *engine = setup_engine(testbed);
	osync_engine_set_conflict_callback(engine, conflict_handler_duplicate, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);	

	mark_point();

	synchronize_once(engine, NULL);

	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 3, NULL);
	fail_unless(num_mapping_conflicts == 1, NULL);
	fail_unless(num_change_written == 8, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));

	/* load mapping table, check for 3 mappings */
	char *path = g_strdup_printf("%s/configs/group/archive.db", testbed);
	OSyncMappingTable *maptable = mappingtable_load(path, "mockobjtype1", 3);
	g_free(path);

	unsigned int num_members = 3;
	unsigned int num_uids = 3;
	const char *uids[] = {"testdata", "testdata-dupe", "testdata-dupe-dupe"};
	unsigned int member;
	unsigned int uid;
	
	/* check we have num_members mapping entries for each uid */
	for (uid = 0; uid < num_uids; uid++) {
		for (member = 1; member <= num_members; member++) {
			check_mapping(maptable, member, -1, num_members, uids[uid]);
		}
	}
	osync_mapping_table_close(maptable);
	osync_mapping_table_unref(maptable);

	hashtable_load_and_check(testbed, 1, uids, num_uids);
	hashtable_load_and_check(testbed, 2, uids, num_uids);
	hashtable_load_and_check(testbed, 3, uids, num_uids);
	
	/* force the removal of all  but test-data-dupe */
	system("rm -f data3/testdata data3/testdata-dupe-dupe");
	mark_point();
	
	synchronize_once(engine, NULL);
	
	num_uids = 1;
	uids[0] = "testdata-dupe";

	/* load mapping table, check for 1 mappings */
	path = g_strdup_printf("%s/configs/group/archive.db", testbed);
	maptable = mappingtable_load(path, "mockobjtype1", 1);
	g_free(path);

	check_mapping(maptable, 1, -1, 3, "testdata-dupe");
	check_mapping(maptable, 2, -1, 3, "testdata-dupe");
	check_mapping(maptable, 3, -1, 3, "testdata-dupe");
	osync_mapping_table_close(maptable);
	osync_mapping_table_unref(maptable);
	
	hashtable_load_and_check(testbed, 1, uids, num_uids);
	hashtable_load_and_check(testbed, 2, uids, num_uids);
	hashtable_load_and_check(testbed, 3, uids, num_uids);
 
	/* force the removal of the remaining testdata-dupe */
	system("rm -f data2/testdata-dupe");
	mark_point();

	synchronize_once(engine, NULL);
	destroy_engine(engine);
	
	check_empty(testbed);
	
	destroy_testbed(testbed);
}
END_TEST

#if 0
START_TEST (multisync_conflict_changetype_duplicate)
{
	char *testbed = setup_testbed("multisync_conflict_changetype_choose");
	OSyncEnv *osync = init_env();
	OSyncGroup *group = osync_group_load(osync, "configs/group", NULL);
	
	OSyncEngine *engine = init_engine(group);
	osengine_set_conflict_callback(engine, conflict_handler_duplication, GINT_TO_POINTER(3));
	
	synchronize_once(engine, NULL);

	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" = \"x\""), NULL);
	
	sleep(2);
	
	system("rm -f data1/testdata");
	system("cp newdata data3/testdata");
	
	synchronize_once(engine, NULL);

	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 2, NULL);
	fail_unless(num_mapping_conflicts == 1, NULL);
	fail_unless(num_change_written == 2, NULL);

	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" = \"x\""), NULL);
	
	OSyncMappingTable *maptable = mappingtable_load(group, 1, 0);
	check_mapping(maptable, 1, 0, 3, "testdata");
	check_mapping(maptable, 2, 0, 3, "testdata");
	check_mapping(maptable, 3, 0, 3, "testdata");
    mappingtable_close(maptable);
	
	OSyncHashTable *table = hashtable_load(group, 1, 1);
    check_hash(table, "testdata");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 2, 1);
    check_hash(table, "testdata");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 3, 1);
    check_hash(table, "testdata");
	osync_hashtable_close(table);
	
	system("rm -f data1/testdata");
	
	mark_point();
	num_mapping_conflicts = 0;
	synchronize_once(engine, NULL);
	osengine_finalize(engine);
	
	mappingtable_load(group, 0, 0);
    mappingtable_close(maptable);
	
	hashtable_load(group, 1, 0);
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 2, 0);
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 3, 0);
	osync_hashtable_close(table);
	
	fail_unless(!system("test \"x$(ls data1)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(ls data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(ls data3)\" = \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (multisync_conflict_changetype_duplicate2)
{
	char *testbed = setup_testbed("multisync_conflict_changetype_choose");
	OSyncEnv *osync = init_env();
	OSyncGroup *group = osync_group_load(osync, "configs/group", NULL);
	
	OSyncEngine *engine = init_engine(group);
	osengine_set_conflict_callback(engine, conflict_handler_duplication, GINT_TO_POINTER(3));
	
	synchronize_once(engine, NULL);

	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" = \"x\""), NULL);
	
	sleep(2);
	
	system("rm -f data2/testdata");
	system("rm -f data3/testdata");
	system("cp newdata2 data1/testdata");
	
	synchronize_once(engine, NULL);

	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 3, NULL);
	fail_unless(num_mapping_conflicts == 1, NULL);
	fail_unless(num_change_written == 2, NULL);

	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" = \"x\""), NULL);
	
	OSyncMappingTable *maptable = mappingtable_load(group, 1, 0);
	check_mapping(maptable, 1, 0, 3, "testdata");
	check_mapping(maptable, 2, 0, 3, "testdata");
	check_mapping(maptable, 3, 0, 3, "testdata");
    mappingtable_close(maptable);
	
	OSyncHashTable *table = hashtable_load(group, 1, 1);
    check_hash(table, "testdata");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 2, 1);
    check_hash(table, "testdata");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 3, 1);
    check_hash(table, "testdata");
	osync_hashtable_close(table);
	
	system("rm -f data1/testdata");
	
	mark_point();
	num_mapping_conflicts = 0;
	synchronize_once(engine, NULL);
	osengine_finalize(engine);
	
	mappingtable_load(group, 0, 0);
    mappingtable_close(maptable);
	
	hashtable_load(group, 1, 0);
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 2, 0);
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 3, 0);
	osync_hashtable_close(table);
	
	fail_unless(!system("test \"x$(ls data1)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(ls data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(ls data3)\" = \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (multisync_conflict_hybrid_duplicate)
{
	char *testbed = setup_testbed("multisync_conflict_changetype_choose");
	OSyncEnv *osync = init_env();
	OSyncGroup *group = osync_group_load(osync, "configs/group", NULL);
	
	OSyncEngine *engine = init_engine(group);
	osengine_set_conflict_callback(engine, conflict_handler_duplication, GINT_TO_POINTER(3));
	
	synchronize_once(engine, NULL);

	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" = \"x\""), NULL);
	
	sleep(2);
	
	system("rm -f data2/testdata");
	system("cp newdata data3/testdata");
	system("cp newdata2 data1/testdata");
	
	synchronize_once(engine, NULL);

	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 3, NULL);
	fail_unless(num_mapping_conflicts == 1, NULL);
	fail_unless(num_change_written == 5, NULL);

	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" = \"x\""), NULL);
	
	OSyncMappingTable *maptable = mappingtable_load(group, 2, 0);
	check_mapping(maptable, 1, -1, 3, "testdata");
	check_mapping(maptable, 2, -1, 3, "testdata");
	check_mapping(maptable, 3, -1, 3, "testdata");
	check_mapping(maptable, 1, -1, 3, "testdata-dupe");
	check_mapping(maptable, 2, -1, 3, "testdata-dupe");
	check_mapping(maptable, 3, -1, 3, "testdata-dupe");
    mappingtable_close(maptable);
	
	OSyncHashTable *table = hashtable_load(group, 1, 2);
    check_hash(table, "testdata");
    check_hash(table, "testdata-dupe");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 2, 2);
    check_hash(table, "testdata");
    check_hash(table, "testdata-dupe");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 3, 2);
    check_hash(table, "testdata");
    check_hash(table, "testdata-dupe");
	osync_hashtable_close(table);
	
	system("rm -f data1/testdata data2/testdata-dupe");
	
	mark_point();
	num_mapping_conflicts = 0;
	synchronize_once(engine, NULL);
	osengine_finalize(engine);
	
	mappingtable_load(group, 0, 0);
    mappingtable_close(maptable);
	
	hashtable_load(group, 1, 0);
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 2, 0);
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 3, 0);
	osync_hashtable_close(table);
	
	fail_unless(!system("test \"x$(ls data1)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(ls data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(ls data3)\" = \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (multisync_multi_conflict)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	OSyncEnv *osync = init_env();
	OSyncGroup *group = osync_group_load(osync, "configs/group", NULL);
	
	OSyncEngine *engine = init_engine(group);
	osengine_set_conflict_callback(engine, conflict_handler_duplication, GINT_TO_POINTER(3));
	
	system("cp newdata data3/testdata1");
	system("cp newdata1 data2/testdata2");
	
	synchronize_once(engine, NULL);

	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" = \"x\""), NULL);
	
	OSyncMappingTable *maptable = mappingtable_load(group, 3, 0);
	check_mapping(maptable, 1, -1, 3, "testdata");
	check_mapping(maptable, 2, -1, 3, "testdata");
	check_mapping(maptable, 3, -1, 3, "testdata");
	check_mapping(maptable, 1, -1, 3, "testdata1");
	check_mapping(maptable, 2, -1, 3, "testdata1");
	check_mapping(maptable, 3, -1, 3, "testdata1");
	check_mapping(maptable, 1, -1, 3, "testdata2");
	check_mapping(maptable, 2, -1, 3, "testdata2");
	check_mapping(maptable, 3, -1, 3, "testdata2");
    mappingtable_close(maptable);
	
	OSyncHashTable *table = hashtable_load(group, 1, 3);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
    check_hash(table, "testdata2");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 2, 3);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
    check_hash(table, "testdata2");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 3, 3);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
    check_hash(table, "testdata2");
	osync_hashtable_close(table);
		
	//Change statuses
	fail_unless(num_change_read == 3, NULL);
	fail_unless(num_read_info == 0, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_change_written == 6, NULL);
	fail_unless(num_written_errors == 0, NULL);
	fail_unless(num_recv_errors == 0, NULL);
	
	//Member statuses
	fail_unless(num_connected == 3, NULL);
	fail_unless(num_disconnected == 3, NULL);
	fail_unless(num_member_comitted_all == 3, NULL);
	fail_unless(num_member_sent_changes == 3, NULL);
	fail_unless(num_member_connect_errors == 0, NULL);
	fail_unless(num_member_get_changes_errors == 0, NULL);
	fail_unless(num_member_sync_done_errors == 0, NULL);
	fail_unless(num_member_disconnect_errors == 0, NULL);
	fail_unless(num_member_comitted_all_errors == 0, NULL);
	
	//Engine statuses
	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_engine_errors == 0, NULL);
	fail_unless(num_engine_successfull == 1, NULL);
	fail_unless(num_engine_prev_unclean == 0, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);
	

	
	sleep(2);
	
	system("rm -f data2/testdata");
	system("cp newdata data3/testdata");

	system("cp newdata3 data1/testdata1");
	system("cp newdata4 data3/testdata1");
	
	system("cp newdata data1/testdata2");
	system("cp newdata5 data3/testdata2");
	system("rm -f data2/testdata2");
	
	synchronize_once(engine, NULL);

	fail_unless(num_change_read == 7, NULL);
	fail_unless(num_mapping_conflicts == 3, NULL);
	fail_unless(num_change_written == 12, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);

	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" = \"x\""), NULL);
	
	maptable = mappingtable_load(group, 5, 0);
	check_mapping(maptable, 1, -1, 3, "testdata");
	check_mapping(maptable, 2, -1, 3, "testdata");
	check_mapping(maptable, 3, -1, 3, "testdata");
	check_mapping(maptable, 1, -1, 3, "testdata1");
	check_mapping(maptable, 2, -1, 3, "testdata1");
	check_mapping(maptable, 3, -1, 3, "testdata1");
	check_mapping(maptable, 1, -1, 3, "testdata1-dupe");
	check_mapping(maptable, 2, -1, 3, "testdata1-dupe");
	check_mapping(maptable, 3, -1, 3, "testdata1-dupe");
	check_mapping(maptable, 1, -1, 3, "testdata2");
	check_mapping(maptable, 2, -1, 3, "testdata2");
	check_mapping(maptable, 3, -1, 3, "testdata2");
	check_mapping(maptable, 1, -1, 3, "testdata2-dupe");
	check_mapping(maptable, 2, -1, 3, "testdata2-dupe");
	check_mapping(maptable, 3, -1, 3, "testdata2-dupe");
    mappingtable_close(maptable);
	
	table = hashtable_load(group, 1, 5);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
    check_hash(table, "testdata2");
    check_hash(table, "testdata1-dupe");
    check_hash(table, "testdata2-dupe");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 2, 5);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
    check_hash(table, "testdata2");
    check_hash(table, "testdata1-dupe");
    check_hash(table, "testdata2-dupe");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 3, 5);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
    check_hash(table, "testdata2");
    check_hash(table, "testdata1-dupe");
    check_hash(table, "testdata2-dupe");
	osync_hashtable_close(table);
	
	system("rm -f data1/*");
	
	mark_point();
	num_mapping_conflicts = 0;
	synchronize_once(engine, NULL);
	osengine_finalize(engine);
	
	maptable = mappingtable_load(group, 0, 0);
    mappingtable_close(maptable);
	
	table = hashtable_load(group, 1, 0);
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 2, 0);
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 3, 0);
	osync_hashtable_close(table);
	
	fail_unless(!system("test \"x$(ls data1)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(ls data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(ls data3)\" = \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (multisync_delayed_conflict_handler)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	OSyncEnv *osync = init_env();
	OSyncGroup *group = osync_group_load(osync, "configs/group", NULL);
	
	OSyncEngine *engine = init_engine(group);
	osengine_set_conflict_callback(engine, conflict_handler_delay, GINT_TO_POINTER(3));
	
	system("cp newdata data3/testdata1");
	system("cp newdata1 data2/testdata2");
	
	synchronize_once(engine, NULL);

	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" = \"x\""), NULL);
	
	OSyncMappingTable *maptable = mappingtable_load(group, 3, 0);
	check_mapping(maptable, 1, -1, 3, "testdata");
	check_mapping(maptable, 2, -1, 3, "testdata");
	check_mapping(maptable, 3, -1, 3, "testdata");
	check_mapping(maptable, 1, -1, 3, "testdata1");
	check_mapping(maptable, 2, -1, 3, "testdata1");
	check_mapping(maptable, 3, -1, 3, "testdata1");
	check_mapping(maptable, 1, -1, 3, "testdata2");
	check_mapping(maptable, 2, -1, 3, "testdata2");
	check_mapping(maptable, 3, -1, 3, "testdata2");
    mappingtable_close(maptable);
	
	OSyncHashTable *table = hashtable_load(group, 1, 3);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
    check_hash(table, "testdata2");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 2, 3);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
    check_hash(table, "testdata2");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 3, 3);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
    check_hash(table, "testdata2");
	osync_hashtable_close(table);
	
	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 3, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_change_written == 6, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);
	
	sleep(2);
	
	system("rm -f data2/testdata");
	system("cp newdata data3/testdata");

	system("cp newdata3 data1/testdata1");
	system("rm -f data2/testdata1");
	
	system("cp newdata data1/testdata2");
	system("rm -f data3/testdata2");
	system("rm -f data2/testdata2");
	
	synchronize_once(engine, NULL);

	fail_unless(num_change_read == 7, NULL);
	fail_unless(num_mapping_conflicts == 3, NULL);
	fail_unless(num_change_written == 6, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);

	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" = \"x\""), NULL);
	
	maptable = mappingtable_load(group, 3, 0);
	check_mapping(maptable, 1, -1, 3, "testdata");
	check_mapping(maptable, 2, -1, 3, "testdata");
	check_mapping(maptable, 3, -1, 3, "testdata");
	check_mapping(maptable, 1, -1, 3, "testdata1");
	check_mapping(maptable, 2, -1, 3, "testdata1");
	check_mapping(maptable, 3, -1, 3, "testdata1");
	check_mapping(maptable, 1, -1, 3, "testdata2");
	check_mapping(maptable, 2, -1, 3, "testdata2");
	check_mapping(maptable, 3, -1, 3, "testdata2");
    mappingtable_close(maptable);
	
	table = hashtable_load(group, 1, 3);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
    check_hash(table, "testdata2");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 2, 3);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
    check_hash(table, "testdata2");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 3, 3);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
    check_hash(table, "testdata2");
	osync_hashtable_close(table);
	
	system("rm -f data1/*");
	
	mark_point();
	num_mapping_conflicts = 0;
	synchronize_once(engine, NULL);
	osengine_finalize(engine);
	
	maptable = mappingtable_load(group, 0, 0);
    mappingtable_close(maptable);
	
	table = hashtable_load(group, 1, 0);
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 2, 0);
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 3, 0);
	osync_hashtable_close(table);
	
	fail_unless(!system("test \"x$(ls data1)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(ls data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(ls data3)\" = \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (multisync_delayed_slow)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	OSyncEnv *osync = init_env();
	OSyncGroup *group = osync_group_load(osync, "configs/group", NULL);
	
	OSyncEngine *engine = init_engine(group);
	osengine_set_conflict_callback(engine, conflict_handler_delay, GINT_TO_POINTER(3));
	
	system("cp newdata data3/testdata1");
	setenv("SLOW_REPORT", "2", TRUE);
	
	synchronize_once(engine, NULL);

	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" = \"x\""), NULL);
	
	OSyncMappingTable *maptable = mappingtable_load(group, 2, 0);
	check_mapping(maptable, 1, -1, 3, "testdata");
	check_mapping(maptable, 2, -1, 3, "testdata");
	check_mapping(maptable, 3, -1, 3, "testdata");
	check_mapping(maptable, 1, -1, 3, "testdata1");
	check_mapping(maptable, 2, -1, 3, "testdata1");
	check_mapping(maptable, 3, -1, 3, "testdata1");
    mappingtable_close(maptable);
	
	OSyncHashTable *table = hashtable_load(group, 1, 2);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 2, 2);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 3, 2);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
	osync_hashtable_close(table);
	
	fail_unless(num_change_read == 2, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_change_written == 4, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);
	
	sleep(2);
	
	system("cp newdata data3/testdata");

	system("cp newdata3 data1/testdata1");
	system("rm -f data2/testdata1");
	
	synchronize_once(engine, NULL);

	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 3, NULL);
	fail_unless(num_mapping_conflicts == 1, NULL);
	fail_unless(num_change_written == 4, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);

	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" = \"x\""), NULL);
	
	maptable = mappingtable_load(group, 2, 0);
	check_mapping(maptable, 1, -1, 3, "testdata");
	check_mapping(maptable, 2, -1, 3, "testdata");
	check_mapping(maptable, 3, -1, 3, "testdata");
	check_mapping(maptable, 1, -1, 3, "testdata1");
	check_mapping(maptable, 2, -1, 3, "testdata1");
	check_mapping(maptable, 3, -1, 3, "testdata1");
    mappingtable_close(maptable);
	
	table = hashtable_load(group, 1, 2);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 2, 2);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 3, 2);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
	osync_hashtable_close(table);
	
	system("rm -f data1/*");
	
	mark_point();
	num_mapping_conflicts = 0;
	synchronize_once(engine, NULL);
	osengine_finalize(engine);
	
	maptable = mappingtable_load(group, 0, 0);
    mappingtable_close(maptable);
	
	table = hashtable_load(group, 1, 0);
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 2, 0);
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 3, 0);
	osync_hashtable_close(table);
	
	fail_unless(!system("test \"x$(ls data1)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(ls data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(ls data3)\" = \"x\""), NULL);
	
	unsetenv("SLOW_REPORT");
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (multisync_conflict_ignore)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	OSyncEnv *osync = init_env();
	OSyncGroup *group = osync_group_load(osync, "configs/group", NULL);
	
	OSyncEngine *engine = init_engine(group);
	osengine_set_conflict_callback(engine, conflict_handler_ignore, GINT_TO_POINTER(3));
	
	system("cp newdata data3/testdata1");
	
	synchronize_once(engine, NULL);

	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" = \"x\""), NULL);
	
	OSyncMappingTable *maptable = mappingtable_load(group, 2, 0);
	check_mapping(maptable, 1, -1, 3, "testdata");
	check_mapping(maptable, 2, -1, 3, "testdata");
	check_mapping(maptable, 3, -1, 3, "testdata");
	check_mapping(maptable, 1, -1, 3, "testdata1");
	check_mapping(maptable, 2, -1, 3, "testdata1");
	check_mapping(maptable, 3, -1, 3, "testdata1");
    mappingtable_close(maptable);
	
	OSyncHashTable *table = hashtable_load(group, 1, 2);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 2, 2);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 3, 2);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
	osync_hashtable_close(table);
	
	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 2, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_change_written == 4, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" = \"x\""), NULL);
	
	sleep(2);
	
	system("rm -f data2/testdata");
	system("cp newdata data3/testdata");

	system("cp newdata3 data1/testdata1");
	system("cp newdata2 data2/testdata1");
	system("cp newdata1 data3/testdata1");
	
	synchronize_once(engine, NULL);

	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 5, NULL);
	fail_unless(num_mapping_conflicts == 2, NULL);
	fail_unless(num_change_written == 0, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);

	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" != \"x\""), NULL);
	
	maptable = mappingtable_load(group, 2, 0);
	check_mapping(maptable, 1, -1, 3, "testdata");
	check_mapping(maptable, 2, -1, 3, "testdata");
	check_mapping(maptable, 3, -1, 3, "testdata");
	check_mapping(maptable, 1, -1, 3, "testdata1");
	check_mapping(maptable, 2, -1, 3, "testdata1");
	check_mapping(maptable, 3, -1, 3, "testdata1");
    mappingtable_close(maptable);
	
	table = hashtable_load(group, 1, 2);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 2, 1);
    check_hash(table, "testdata1");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 3, 2);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
	osync_hashtable_close(table);
	
	synchronize_once(engine, NULL);

	fail_unless(num_change_read == 5, NULL);
	fail_unless(num_mapping_conflicts == 2, NULL);
	fail_unless(num_change_written == 0, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);

	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" != \"x\""), NULL);
	
	maptable = mappingtable_load(group, 2, 0);
	check_mapping(maptable, 1, -1, 3, "testdata");
	check_mapping(maptable, 2, -1, 3, "testdata");
	check_mapping(maptable, 3, -1, 3, "testdata");
	check_mapping(maptable, 1, -1, 3, "testdata1");
	check_mapping(maptable, 2, -1, 3, "testdata1");
	check_mapping(maptable, 3, -1, 3, "testdata1");
    mappingtable_close(maptable);
	
	table = hashtable_load(group, 1, 2);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 2, 1);
    check_hash(table, "testdata1");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 3, 2);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
	osync_hashtable_close(table);
	
	
	osengine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));
	synchronize_once(engine, NULL);

	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 5, NULL);
	fail_unless(num_mapping_conflicts == 2, NULL);
	fail_unless(num_change_written == 4, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);

	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" = \"x\""), NULL);
	
	maptable = mappingtable_load(group, 2, 0);
	check_mapping(maptable, 1, -1, 3, "testdata");
	check_mapping(maptable, 2, -1, 3, "testdata");
	check_mapping(maptable, 3, -1, 3, "testdata");
	check_mapping(maptable, 1, -1, 3, "testdata1");
	check_mapping(maptable, 2, -1, 3, "testdata1");
	check_mapping(maptable, 3, -1, 3, "testdata1");
    mappingtable_close(maptable);
	
	table = hashtable_load(group, 1, 2);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 2, 2);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 3, 2);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
	osync_hashtable_close(table);
	
	system("rm -f data1/*");

	synchronize_once(engine, NULL);
	osengine_finalize(engine);
	
	maptable = mappingtable_load(group, 0, 0);
    mappingtable_close(maptable);
	
	table = hashtable_load(group, 1, 0);
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 2, 0);
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 3, 0);
	osync_hashtable_close(table);
	
	fail_unless(!system("test \"x$(ls data1)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(ls data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(ls data3)\" = \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (multisync_conflict_ignore2)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	OSyncEnv *osync = init_env();
	OSyncGroup *group = osync_group_load(osync, "configs/group", NULL);
	
	OSyncEngine *engine = init_engine(group);
	osengine_set_conflict_callback(engine, conflict_handler_ignore, NULL);
	
	system("cp newdata data3/testdata1");
	system("cp newdata1 data3/testdata");
	
	synchronize_once(engine, NULL);
	
	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 3, NULL);
	fail_unless(num_mapping_conflicts == 1, NULL);
	fail_unless(num_change_written == 2, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);

	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" != \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" != \"x\""), NULL);
	
	OSyncMappingTable *maptable = mappingtable_load(group, 2, 0);
	check_mapping(maptable, 1, -1, 2, "testdata");
	check_mapping(maptable, 3, -1, 2, "testdata");
	check_mapping(maptable, 1, -1, 3, "testdata1");
	check_mapping(maptable, 2, -1, 3, "testdata1");
	check_mapping(maptable, 3, -1, 3, "testdata1");
    mappingtable_close(maptable);
	
	OSyncHashTable *table = hashtable_load(group, 1, 2);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 2, 1);
    check_hash(table, "testdata1");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 3, 2);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
	osync_hashtable_close(table);
	
	system("cp newdata2 data2/testdata");
	
	osengine_set_conflict_callback(engine, conflict_handler_choose_first, GINT_TO_POINTER(3));
	synchronize_once(engine, NULL);
	
	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 3, NULL);
	fail_unless(num_mapping_conflicts == 1, NULL);
	fail_unless(num_change_written == 2, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);
	
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" = \"x\""), NULL);
	
	maptable = mappingtable_load(group, 2, 0);
	check_mapping(maptable, 1, -1, 3, "testdata");
	check_mapping(maptable, 2, -1, 3, "testdata");
	check_mapping(maptable, 3, -1, 3, "testdata");
	check_mapping(maptable, 1, -1, 3, "testdata1");
	check_mapping(maptable, 2, -1, 3, "testdata1");
	check_mapping(maptable, 3, -1, 3, "testdata1");
    mappingtable_close(maptable);
	
	table = hashtable_load(group, 1, 2);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 2, 2);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 3, 2);
    check_hash(table, "testdata");
    check_hash(table, "testdata1");
	osync_hashtable_close(table);
	
	system("rm -f data1/*");

	synchronize_once(engine, NULL);
	osengine_finalize(engine);
	
	maptable = mappingtable_load(group, 0, 0);
    mappingtable_close(maptable);
	
	table = hashtable_load(group, 1, 0);
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 2, 0);
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 3, 0);
	osync_hashtable_close(table);
	
	fail_unless(!system("test \"x$(ls data1)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(ls data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(ls data3)\" = \"x\""), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST(multisync_easy_new_b)
{
	setenv("BATCH_COMMIT", "7", TRUE);
	multisync_easy_new();
	unsetenv("BATCH_COMMIT");
}
END_TEST

START_TEST(multisync_triple_del_b)
{
	setenv("BATCH_COMMIT", "7", TRUE);
	multisync_triple_del();
	unsetenv("BATCH_COMMIT");
}
END_TEST

START_TEST(multisync_conflict_hybrid_choose2_b)
{
	setenv("BATCH_COMMIT", "7", TRUE);
	multisync_conflict_hybrid_choose2();
	unsetenv("BATCH_COMMIT");
}
END_TEST

START_TEST(multisync_delayed_conflict_handler_b)
{
	setenv("BATCH_COMMIT", "7", TRUE);
	multisync_delayed_conflict_handler();
	unsetenv("BATCH_COMMIT");
}
END_TEST

START_TEST(multisync_delayed_slow_b)
{
	setenv("BATCH_COMMIT", "7", TRUE);
	multisync_delayed_slow();
	unsetenv("BATCH_COMMIT");
}
END_TEST

START_TEST(multisync_conflict_ignore_b)
{
	setenv("BATCH_COMMIT", "7", TRUE);
	multisync_conflict_ignore();
	unsetenv("BATCH_COMMIT");
}
END_TEST

START_TEST(multisync_conflict_ignore2_b)
{
	setenv("BATCH_COMMIT", "7", TRUE);
	setenv("NO_TIMEOUTS", "7", TRUE);
	multisync_conflict_ignore2();
	unsetenv("BATCH_COMMIT");
	unsetenv("NO_TIMEOUTS");
}
END_TEST

START_TEST(multisync_conflict_hybrid_duplicate_b)
{
	setenv("BATCH_COMMIT", "7", TRUE);
	multisync_conflict_hybrid_duplicate();
	unsetenv("BATCH_COMMIT");
}
END_TEST

START_TEST(multisync_multi_conflict_b)
{
	setenv("BATCH_COMMIT", "7", TRUE);
	multisync_multi_conflict();
	unsetenv("BATCH_COMMIT");
}
END_TEST

START_TEST(multisync_zero_changes_b)
{
	setenv("BATCH_COMMIT", "7", TRUE);
	char *testbed = setup_testbed("multisync_easy_new");
	
	OSyncEnv *osync = init_env();
	OSyncGroup *group = osync_group_load(osync, "configs/group", NULL);
	
	OSyncEngine *engine = init_engine(group);
	osengine_set_conflict_callback(engine, conflict_handler_duplication, GINT_TO_POINTER(3));
	
	synchronize_once(engine, NULL);

	fail_unless(!system("test \"x$(diff -x \".*\" data1 data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(diff -x \".*\" data1 data3)\" = \"x\""), NULL);
	
	OSyncMappingTable *maptable = mappingtable_load(group, 1, 0);
	check_mapping(maptable, 1, -1, 3, "testdata");
	check_mapping(maptable, 2, -1, 3, "testdata");
	check_mapping(maptable, 3, -1, 3, "testdata");
	mappingtable_close(maptable);
	
	OSyncHashTable *table = hashtable_load(group, 1, 1);
    check_hash(table, "testdata");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 2, 1);
    check_hash(table, "testdata");
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 3, 1);
    check_hash(table, "testdata");
	osync_hashtable_close(table);
	
	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_change_written == 2, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);

	setenv("NUM_BATCH_COMMITS", "0", TRUE);
	
	synchronize_once(engine, NULL);

	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 0, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_change_written == 0, NULL);
	fail_unless(num_engine_end_conflicts == 0, NULL);

	system("rm -f data1/*");
		
	unsetenv("NUM_BATCH_COMMITS");
	
	synchronize_once(engine, NULL);
	osengine_finalize(engine);
	
	maptable = mappingtable_load(group, 0, 0);
    mappingtable_close(maptable);
	
	table = hashtable_load(group, 1, 0);
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 2, 0);
	osync_hashtable_close(table);
	
	table = hashtable_load(group, 3, 0);
	osync_hashtable_close(table);
	
	fail_unless(!system("test \"x$(ls data1)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(ls data2)\" = \"x\""), NULL);
	fail_unless(!system("test \"x$(ls data3)\" = \"x\""), NULL);
	
	destroy_testbed(testbed);
	unsetenv("BATCH_COMMIT");
}
END_TEST

START_TEST(multisync_conflict_hybrid_choose2_b2)
{
	setenv("BATCH_COMMIT", "2", TRUE);
	multisync_conflict_hybrid_choose2();
	unsetenv("BATCH_COMMIT");
}
END_TEST

START_TEST(multisync_delayed_conflict_handler_b2)
{
	setenv("BATCH_COMMIT", "2", TRUE);
	multisync_delayed_conflict_handler();
	unsetenv("BATCH_COMMIT");
}
END_TEST

START_TEST(multisync_conflict_ignore_b2)
{
	setenv("BATCH_COMMIT", "2", TRUE);
	multisync_conflict_ignore();
	unsetenv("BATCH_COMMIT");
}
END_TEST

START_TEST(multisync_multi_conflict_b2)
{
	setenv("BATCH_COMMIT", "2", TRUE);
	multisync_multi_conflict();
	unsetenv("BATCH_COMMIT");
}
END_TEST
#endif

OSYNC_TESTCASE_START("multisync")
OSYNC_TESTCASE_ADD(multisync_easy_new)
OSYNC_TESTCASE_ADD(multisync_dual_new)
OSYNC_TESTCASE_ADD(multisync_triple_new)

OSYNC_TESTCASE_ADD(multisync_easy_mod)
OSYNC_TESTCASE_ADD(multisync_dual_mod)
OSYNC_TESTCASE_ADD(multisync_triple_mod)

OSYNC_TESTCASE_ADD(multisync_easy_del)
OSYNC_TESTCASE_ADD(multisync_dual_del)
OSYNC_TESTCASE_ADD(multisync_triple_del)

OSYNC_TESTCASE_ADD(multisync_conflict_data_choose)
OSYNC_TESTCASE_ADD(multisync_conflict_data_choose2)

OSYNC_TESTCASE_ADD(multisync_conflict_changetype_choose)
OSYNC_TESTCASE_ADD(multisync_conflict_changetype_choose2)

OSYNC_TESTCASE_ADD(multisync_conflict_hybrid_choose)
OSYNC_TESTCASE_ADD(multisync_conflict_hybrid_choose2)

OSYNC_TESTCASE_ADD(multisync_conflict_data_duplicate)
OSYNC_TESTCASE_ADD(multisync_conflict_data_duplicate2)

/* FIXME: port testcases, see ticket #981 */
#if 0
OSYNC_TESTCASE_ADD(multisync_conflict_changetype_duplicate)
OSYNC_TESTCASE_ADD(multisync_conflict_changetype_duplicate2)
OSYNC_TESTCASE_ADD(multisync_conflict_hybrid_duplicate)
OSYNC_TESTCASE_ADD(multisync_multi_conflict)

OSYNC_TESTCASE_ADD(multisync_delayed_conflict_handler)
OSYNC_TESTCASE_ADD(multisync_delayed_slow)

OSYNC_TESTCASE_ADD(multisync_conflict_ignore)
OSYNC_TESTCASE_ADD(multisync_conflict_ignore2)

OSYNC_TESTCASE_ADD(multisync_easy_new_b)
OSYNC_TESTCASE_ADD(multisync_triple_del_b)
OSYNC_TESTCASE_ADD(multisync_conflict_hybrid_choose2_b)
OSYNC_TESTCASE_ADD(multisync_delayed_conflict_handler_b)
OSYNC_TESTCASE_ADD(multisync_delayed_slow_b)
OSYNC_TESTCASE_ADD(multisync_conflict_ignore_b)
OSYNC_TESTCASE_ADD(multisync_conflict_ignore2_b)
OSYNC_TESTCASE_ADD(multisync_conflict_hybrid_duplicate_b)
OSYNC_TESTCASE_ADD(multisync_multi_conflict_b)
OSYNC_TESTCASE_ADD(multisync_zero_changes_b)

OSYNC_TESTCASE_ADD(multisync_conflict_hybrid_choose2_b2)
OSYNC_TESTCASE_ADD(multisync_delayed_conflict_handler_b2)
OSYNC_TESTCASE_ADD(multisync_conflict_ignore_b2)
OSYNC_TESTCASE_ADD(multisync_multi_conflict_b2)
#endif

OSYNC_TESTCASE_END

