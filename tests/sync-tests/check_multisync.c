#include "support.h"

#include "opensync/group/opensync_group_internals.h"
#include "opensync/engine/opensync_engine_internals.h"


void multisync_easy_new_inner(const char *testbed)
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
}

START_TEST (multisync_easy_new)
{
	char *testbed = setup_testbed("multisync_easy_new");
	multisync_easy_new_inner(testbed);
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
	osync_testing_system_abort("cp newdata data3/testdata");
	
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
	osync_testing_system_abort("cp newdata data1/testdata");
	osync_testing_system_abort("cp newdata data3/testdata");
	
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
	osync_testing_system_abort("cp newdata data1/testdata");
	osync_testing_system_abort("cp newdata data2/testdata");
	osync_testing_system_abort("cp newdata data3/testdata");
	
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

	osync_testing_system_abort("rm -f data2/testdata");
	
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

	osync_testing_system_abort("rm -f data1/testdata");
	osync_testing_system_abort("rm -f data3/testdata");
	
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
void multisync_triple_del_inner(const char *testbed)
{
	OSyncError *error = NULL;
	OSyncEngine *engine = setup_engine(testbed);
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);	
	
	synchronize_once(engine, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));

	osync_testing_system_abort("rm -f data1/testdata");
	osync_testing_system_abort("rm -f data2/testdata");
	osync_testing_system_abort("rm -f data3/testdata");
	
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
}

START_TEST (multisync_triple_del)
{
	char *testbed = setup_testbed("multisync_easy_new");
	multisync_triple_del_inner(testbed);
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
	/* Currently opensync propgates the matching entry from member 1 to member 3
	   so makes two writes, one to member 2 and one to member 3
	   Ideally num_change_written == 1 but as this is only an optimisation step
	   it can be left for now */
	fail_unless(num_change_written == 2, NULL);
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
	
	osync_testing_system_abort("rm -f data3/testdata");
	
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
	/* Currently opensync propgates the matching entry from member 1 to member 3
	   so makes two writes, one to member 2 and one to member 3
	   Ideally num_change_written == 1 but as this is only an optimisation step
	   it can be left for now */
	fail_unless(num_change_written == 2, NULL);
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
	
	osync_testing_system_abort("rm -f data3/testdata");
	
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
	
	osync_testing_system_abort("rm -f data1/testdata");
	osync_testing_system_abort("cp newdata data3/testdata");
	
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
	
	osync_testing_system_abort("rm -f data1/testdata");
	
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
	
	osync_testing_system_abort("rm -f data1/testdata");
	osync_testing_system_abort("cp newdata data3/testdata");
	
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
	
	osync_testing_system_abort("rm -f data1/testdata");
	osync_testing_system_abort("cp newdata data3/testdata");
	osync_testing_system_abort("cp newdata2 data2/testdata");
	
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
	
	osync_testing_system_abort("rm -f data1/testdata");
	
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
void multisync_conflict_hybrid_choose2_inner(const char *testbed)
{
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
	
	osync_testing_system_abort("rm -f data1/testdata");
	osync_testing_system_abort("cp newdata data3/testdata");
	osync_testing_system_abort("cp newdata2 data2/testdata");
	
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
}

START_TEST (multisync_conflict_hybrid_choose2)
{
	char *testbed = setup_testbed("multisync_conflict_changetype_choose");
	multisync_conflict_hybrid_choose2_inner(testbed);
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

	
	osync_testing_system_abort("rm -f data3/testdata");
	
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

	
	osync_testing_system_abort("rm -f data2/testdata-dupe");
	
	mark_point();

	synchronize_once(engine, NULL);
	destroy_engine(engine);
	
	check_empty(testbed);
	
	destroy_testbed(testbed);
}
END_TEST

/* Given a number of members and a set of uids we check
 * the number of mappings == num_uids
 * there are num_member connections in each map for all the given uids
 */
void validate_mapping_table(const char *testbed, unsigned int num_members, const char *uids[], unsigned int num_uids)
{
	unsigned int member;
	unsigned int uid;

	char *path = g_strdup_printf("%s/configs/group/archive.db", testbed);
	OSyncMappingTable *maptable = mappingtable_load(path, "mockobjtype1", num_uids);
	g_free(path);

	/* check we have num_members mapping entries for each uid */
	for (uid = 0; uid < num_uids; uid++) {
		for (member = 1; member <= num_members; member++) {
			check_mapping(maptable, member, -1, num_members, uids[uid]);
		}
	}
	osync_mapping_table_close(maptable);
	osync_mapping_table_unref(maptable);
}

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

	unsigned int num_members = 3;
	unsigned int num_uids = 3;
	const char *uids[] = {"testdata", "testdata-dupe", "testdata-dupe-dupe"};
	
	validate_mapping_table(testbed, num_members, uids, num_uids);

	hashtable_load_and_check(testbed, 1, uids, num_uids);
	hashtable_load_and_check(testbed, 2, uids, num_uids);
	hashtable_load_and_check(testbed, 3, uids, num_uids);
	
	/* force the removal of all  but test-data-dupe */
	osync_testing_system_abort("rm -f data3/testdata data3/testdata-dupe-dupe");
	mark_point();
	
	synchronize_once(engine, NULL);
	
	num_uids = 1;
	uids[0] = "testdata-dupe";

	/* load mapping table, check for 1 mappings */
	validate_mapping_table(testbed, num_members, uids, num_uids);
	
	hashtable_load_and_check(testbed, 1, uids, num_uids);
	hashtable_load_and_check(testbed, 2, uids, num_uids);
	hashtable_load_and_check(testbed, 3, uids, num_uids);
 
	/* force the removal of the remaining testdata-dupe */
	osync_testing_system_abort("rm -f data2/testdata-dupe");
	mark_point();

	synchronize_once(engine, NULL);
	destroy_engine(engine);
	
	check_empty(testbed);
	
	destroy_testbed(testbed);
}
END_TEST


/* Sync data1/testdata to all
 * - remove data1/testdata
 * - change data3/testdata to newdata
 * Expect 2 changes, 1 conflict and the newdata version of testdata to be 
 * synced to data1 and data2
 */
START_TEST (multisync_conflict_changetype_duplicate)
{
	char *testbed = setup_testbed("multisync_conflict_changetype_choose");
	OSyncError *error = NULL;
	OSyncEngine *engine = setup_engine(testbed);
	osync_engine_set_conflict_callback(engine, conflict_handler_duplicate, GINT_TO_POINTER(2));
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	synchronize_once(engine, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	
	sleep(2);
	
	osync_testing_system_abort("rm -f data1/testdata");
	osync_testing_system_abort("cp newdata data3/testdata");
	
	synchronize_once(engine, NULL);

	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 2, NULL);
	fail_unless(num_mapping_conflicts == 1, NULL);
	fail_unless(num_change_written == 2, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	fail_unless(!system("test \"x$(diff newdata data1/testdata)\" = \"x\""), NULL);

	const char *uids[] = {"testdata"};
	validate_mapping_table(testbed, 3, uids, 1);
	
	hashtable_simple_load_and_check(testbed, 1);
	hashtable_simple_load_and_check(testbed, 2);
	hashtable_simple_load_and_check(testbed, 3);
	
	osync_testing_system_abort("rm -f data1/testdata");
	
	mark_point();

	synchronize_once(engine, NULL);
	destroy_engine(engine);

	check_empty(testbed);
	
	destroy_testbed(testbed);
}
END_TEST

/* Sync data1/testdata to all
 * - remove data2/testdata
 * - remove data3/testdata
 * - change data1/testdata to newdata2
 * Expect 3 changes, 1 conflict and the newdata2 version of testdata to be 
 * synced to data2 and data3
 */
START_TEST (multisync_conflict_changetype_duplicate2)
{
	char *testbed = setup_testbed("multisync_conflict_changetype_choose");
	OSyncError *error = NULL;
	OSyncEngine *engine = setup_engine(testbed);
	osync_engine_set_conflict_callback(engine, conflict_handler_duplicate, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);	

	mark_point();

	synchronize_once(engine, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	
	sleep(2);
	
	osync_testing_system_abort("rm -f data2/testdata");
	osync_testing_system_abort("rm -f data3/testdata");
	osync_testing_system_abort("cp newdata2 data1/testdata");
	
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
	fail_unless(!system("test \"x$(diff newdata2 data1/testdata)\" = \"x\""), NULL);

	
	const char *uids[] = {"testdata"};
	validate_mapping_table(testbed, 3, uids, 1);
	
	hashtable_simple_load_and_check(testbed, 1);
	hashtable_simple_load_and_check(testbed, 2);
	hashtable_simple_load_and_check(testbed, 3);
	
	osync_testing_system_abort("rm -f data1/testdata");
	
	mark_point();

	synchronize_once(engine, NULL);
	destroy_engine(engine);
	
	check_empty(testbed);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (multisync_conflict_hybrid_duplicate)
{
	char *testbed = setup_testbed("multisync_conflict_changetype_choose");
	OSyncError *error = NULL;
	OSyncEngine *engine = setup_engine(testbed);
	osync_engine_set_conflict_callback(engine, conflict_handler_duplicate, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);	

	mark_point();

	synchronize_once(engine, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	
	sleep(2);
	
	osync_testing_system_abort("rm -f data2/testdata");
	osync_testing_system_abort("cp newdata data3/testdata");
	osync_testing_system_abort("cp newdata2 data1/testdata");
	
	synchronize_once(engine, NULL);

	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 3, NULL);
	fail_unless(num_mapping_conflicts == 1, NULL);
	fail_unless(num_change_written == 5, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	
	unsigned int num_uids = 2;
	const char *uids[] = {"testdata", "testdata-dupe"};
	validate_mapping_table(testbed, 3, uids, num_uids);
	
	hashtable_load_and_check(testbed, 1, uids, num_uids);
	hashtable_load_and_check(testbed, 2, uids, num_uids);
	hashtable_load_and_check(testbed, 3, uids, num_uids);
 

	osync_testing_system_abort("rm -f data1/testdata data2/testdata-dupe");
       	mark_point();

	synchronize_once(engine, NULL);
	destroy_engine(engine);
	
	check_empty(testbed);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (multisync_multi_conflict)
{
	char *testbed = setup_testbed("multisync_easy_new");

	OSyncError *error = NULL;
	OSyncEngine *engine = setup_engine(testbed);
	/* Cant do sanity check on conflict handler as num changes per conflict is
	   not constant later on - so pass NULL as userdata */
	osync_engine_set_conflict_callback(engine, conflict_handler_duplicate, NULL);

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);	
		
	osync_testing_system_abort("cp newdata data3/testdata1");
	osync_testing_system_abort("cp newdata1 data2/testdata2");
	
	synchronize_once(engine, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));

	unsigned int num_uids = 3;
	const char *uids[] = {"testdata", "testdata1", "testdata2"};
	validate_mapping_table(testbed, 3, uids, num_uids);
	

	hashtable_load_and_check(testbed, 1, uids, num_uids);
	hashtable_load_and_check(testbed, 2, uids, num_uids);
	hashtable_load_and_check(testbed, 3, uids, num_uids);  
		
	//Change statuses
	fail_unless(num_change_read == 3, NULL);
	/*	fail_unless(num_read_info == 0, NULL);*/
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_change_written == 6, NULL);
	fail_unless(num_change_error == 0, NULL);
	/*	fail_unless(num_recv_errors == 0, NULL);*/
	
	//Member statuses
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	/*	fail_unless(num_client_comitted_all == 3, NULL);*/
	fail_unless(num_client_errors == 0, NULL);
	/*	fail_unless(num_client_sent_changes == 3, NULL);
	fail_unless(num_client_connect_errors == 0, NULL);
	fail_unless(num_client_get_changes_errors == 0, NULL);
	fail_unless(num_client_sync_done_errors == 0, NULL);
	fail_unless(num_client_disconnect_errors == 0, NULL);
	fail_unless(num_client_comitted_all_errors == 0, NULL);*/
	
	//Engine statuses
	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_engine_errors == 0, NULL);
	fail_unless(num_engine_successful == 1, NULL);
	fail_unless(num_engine_prev_unclean == 0, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);
	

	
	sleep(2);
	
	/* Should end up with 
	   testdata(newdata) - 2 writes
	   testdata1(newdata3) - 2 writes
	   testdata1-dupe(newdata4) -3 writes
	   testdata2(newdata) - 2 writes
	   testdata2-dupe(newdata5) - 3 writes
	*/

	osync_testing_system_abort("rm -f data2/testdata");
	osync_testing_system_abort("cp newdata data3/testdata");

	osync_testing_system_abort("cp newdata3 data1/testdata1");
	osync_testing_system_abort("cp newdata4 data3/testdata1");
	
	osync_testing_system_abort("cp newdata data1/testdata2");
	osync_testing_system_abort("cp newdata5 data3/testdata2");
	osync_testing_system_abort("rm -f data2/testdata2");


	synchronize_once(engine, NULL);

	fail_unless(num_change_read == 7, NULL);
	fail_unless(num_mapping_conflicts == 3, NULL);
	fail_unless(num_change_written == 12, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));

	unsigned int num_members = 3;
	num_uids = 5;
	const char *uids2[] = {"testdata",
			       "testdata1",
			       "testdata1-dupe",
			       "testdata2",
			       "testdata2-dupe"};
	
	validate_mapping_table(testbed, num_members, uids2, num_uids);

	hashtable_load_and_check(testbed, 1, uids2, num_uids);
	hashtable_load_and_check(testbed, 2, uids2, num_uids);
	hashtable_load_and_check(testbed, 3, uids2, num_uids);
 	
	
	osync_testing_system_abort("rm -f data1/*");
	
	mark_point();

	synchronize_once(engine, NULL);
	destroy_engine(engine);
	
	check_empty(testbed);
	
	destroy_testbed(testbed);
}
END_TEST




void multisync_delayed_conflict_handler_inner(const char *testbed)
{
	OSyncError *error = NULL;
	OSyncEngine *engine = setup_engine(testbed);
	/* Unable to sanity check conflict */
	osync_engine_set_conflict_callback(engine, conflict_handler_delay, NULL);
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);	

	mark_point();
	
	osync_testing_system_abort("cp newdata data3/testdata1");
	osync_testing_system_abort("cp newdata1 data2/testdata2");
	
	synchronize_once(engine, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	
	unsigned int num_uids = 3;
	const char *uids[] = {"testdata", "testdata1", "testdata2"};
	validate_mapping_table(testbed, 3, uids, num_uids);
	
	hashtable_load_and_check(testbed, 1, uids, num_uids);
	hashtable_load_and_check(testbed, 2, uids, num_uids);
	hashtable_load_and_check(testbed, 3, uids, num_uids);
	
	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 3, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_change_written == 6, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);
	
	sleep(2);
	
	osync_testing_system_abort("rm -f data2/testdata");
	osync_testing_system_abort("cp newdata data3/testdata");

	osync_testing_system_abort("cp newdata3 data1/testdata1");
	osync_testing_system_abort("rm -f data2/testdata1");
	
	osync_testing_system_abort("cp newdata data1/testdata2");
	osync_testing_system_abort("rm -f data3/testdata2");
	osync_testing_system_abort("rm -f data2/testdata2");
	
	synchronize_once(engine, NULL);

	fail_unless(num_change_read == 7, NULL);
	fail_unless(num_mapping_conflicts == 3, NULL);
	fail_unless(num_change_written == 6, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	
	validate_mapping_table(testbed, 3, uids, num_uids);

	/* force the removal of everything */
	osync_testing_system_abort("rm -f data1/*");
	
	mark_point();

	synchronize_once(engine, NULL);
	destroy_engine(engine);
	
	check_empty(testbed);
}

START_TEST (multisync_delayed_conflict_handler)
{
	char *testbed = setup_testbed("multisync_easy_new");
	multisync_delayed_conflict_handler_inner(testbed);
	destroy_testbed(testbed);
}
END_TEST

/* Seems to have threading issues */
void multisync_delayed_slow_inner(const char *testbed)
{
	OSyncError *error = NULL;
	OSyncEngine *engine = setup_engine(testbed);
	/* Unable to sanity check conflict as different number of changes per mapping */
	osync_engine_set_conflict_callback(engine, conflict_handler_delay, NULL);
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	osync_testing_system_abort("cp newdata data3/testdata1");
	setenv("SLOW_REPORT", "2", TRUE);
	
	synchronize_once(engine, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));

	unsigned int num_uids = 2;
	const char *uids[] = {"testdata", "testdata1"};
	
	validate_mapping_table(testbed, 3, uids, num_uids);
	
	hashtable_load_and_check(testbed, 1, uids, num_uids);
	hashtable_load_and_check(testbed, 2, uids, num_uids);
	hashtable_load_and_check(testbed, 3, uids, num_uids);
	
	fail_unless(num_change_read == 2, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_change_written == 4, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);
	
	sleep(2);
	
	osync_testing_system_abort("cp newdata data3/testdata");

	osync_testing_system_abort("cp newdata3 data1/testdata1");
	osync_testing_system_abort("rm -f data2/testdata1");

	synchronize_once(engine, NULL);

	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 3, NULL);
	fail_unless(num_mapping_conflicts == 1, NULL);
	fail_unless(num_change_written == 4, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	
	validate_mapping_table(testbed, 3, uids, num_uids);
	
	hashtable_load_and_check(testbed, 1, uids, num_uids);
	hashtable_load_and_check(testbed, 2, uids, num_uids);
	hashtable_load_and_check(testbed, 3, uids, num_uids);
	
	osync_testing_system_abort("rm -f data1/*");
	
	mark_point();

	synchronize_once(engine, NULL);
	destroy_engine(engine);
	
	check_empty(testbed);
	
	unsetenv("SLOW_REPORT");
}

START_TEST (multisync_delayed_slow)
{
	char *testbed = setup_testbed("multisync_easy_new");
	multisync_delayed_slow_inner(testbed);
	destroy_testbed(testbed);
}
END_TEST

START_TEST (multisync_conflict_ignore)
{
	char *testbed = setup_testbed("multisync_easy_new");

	OSyncError *error = NULL;	
	OSyncEngine *engine = setup_engine(testbed);
	osync_engine_set_conflict_callback(engine, conflict_handler_ignore, NULL);	
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	osync_testing_system_abort("cp newdata data3/testdata1");
	
	synchronize_once(engine, NULL);


	unsigned int num_uids = 2;
	const char *uids[] = {"testdata", "testdata1"};
	
	validate_mapping_table(testbed, 3, uids, num_uids);
	
	hashtable_load_and_check(testbed, 1, uids, num_uids);
	hashtable_load_and_check(testbed, 2, uids, num_uids);
	hashtable_load_and_check(testbed, 3, uids, num_uids);
	
	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 2, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_change_written == 4, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);
	
	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	
	sleep(2);
	
	/* create some conflicts and ignore them twice */

	osync_testing_system_abort("rm -f data2/testdata");
	osync_testing_system_abort("cp newdata data3/testdata");

	osync_testing_system_abort("cp newdata3 data1/testdata1");
	osync_testing_system_abort("cp newdata2 data2/testdata1");
	osync_testing_system_abort("cp newdata1 data3/testdata1");

	int i;
	for (i = 0; i < 2; i++ ) {
		synchronize_once(engine, NULL);

		fail_unless(num_engine_connected == 1, NULL);
		fail_unless(num_engine_read == 1, NULL);
		fail_unless(num_engine_written == 1, NULL);
		fail_unless(num_engine_disconnected == 1, NULL);
		fail_unless(num_change_read == 5, NULL);
		fail_unless(num_mapping_conflicts == 2, NULL);
		fail_unless(num_change_written == 0, NULL);
		fail_unless(num_engine_end_conflicts == 1, NULL);
		
		/* check the members are different */
		fail_unless(!osync_testing_diff("data1", "data2"));
		fail_unless(!osync_testing_diff("data1", "data3"));
		

		char *path = g_strdup_printf("%s/configs/group/archive.db", testbed);
		OSyncMappingTable *maptable = mappingtable_load(path, "mockobjtype1", 2);
		g_free(path);
		check_mapping(maptable, 1, -1, 2, "testdata");
		check_mapping(maptable, 3, -1, 2, "testdata");
		check_mapping(maptable, 1, -1, 3, "testdata1");
		check_mapping(maptable, 2, -1, 3, "testdata1");
		check_mapping(maptable, 3, -1, 3, "testdata1");
		osync_mapping_table_close(maptable);
		osync_mapping_table_unref(maptable);	
	
		const char *member2_uids[] = {"testdata1"};
		hashtable_load_and_check(testbed, 1, uids, num_uids);
		hashtable_load_and_check(testbed, 2, member2_uids, 1);
		hashtable_load_and_check(testbed, 3, uids, num_uids);
	}
	
	/* Now should pick
	   data3/testdata(newdata)
	   data1/testdata1(newdata3)
	*/
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, NULL);
	synchronize_once(engine, NULL);

	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 5, NULL);
	fail_unless(num_mapping_conflicts == 2, NULL);
	fail_unless(num_change_written == 4, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	
	validate_mapping_table(testbed, 3, uids, num_uids);

	hashtable_load_and_check(testbed, 1, uids, num_uids);
	hashtable_load_and_check(testbed, 2, uids, num_uids);
	hashtable_load_and_check(testbed, 3, uids, num_uids);
	
	osync_testing_system_abort("rm -f data1/*");

	synchronize_once(engine, NULL);
	destroy_engine(engine);
	
	check_empty(testbed);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (multisync_conflict_ignore2)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	OSyncError *error = NULL;
	OSyncEngine *engine = setup_engine(testbed);
	osync_engine_set_conflict_callback(engine, conflict_handler_ignore, NULL);
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	osync_testing_system_abort("cp newdata data3/testdata1");
	osync_testing_system_abort("cp newdata1 data3/testdata");
	
	synchronize_once(engine, NULL);
	
	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 3, NULL);
	fail_unless(num_mapping_conflicts == 1, NULL);
	fail_unless(num_change_written == 2, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);

	/* check the members are different */
	fail_unless(!osync_testing_diff("data1", "data2"));
	fail_unless(!osync_testing_diff("data1", "data3"));

	char *path = g_strdup_printf("%s/configs/group/archive.db", testbed);
	OSyncMappingTable *maptable = mappingtable_load(path, "mockobjtype1", 2);
	g_free(path);
	check_mapping(maptable, 1, -1, 2, "testdata");
	check_mapping(maptable, 3, -1, 2, "testdata");
	check_mapping(maptable, 1, -1, 3, "testdata1");
	check_mapping(maptable, 2, -1, 3, "testdata1");
	check_mapping(maptable, 3, -1, 3, "testdata1");
	osync_mapping_table_close(maptable);
	osync_mapping_table_unref(maptable);

	unsigned int num_uids = 2;
	const char *uids[] = {"testdata", "testdata1"};
	unsigned int num_member2_uids = 1;
	const char *member2_uids[] = {"testdata1"};
	hashtable_load_and_check(testbed, 1, uids, num_uids);
	hashtable_load_and_check(testbed, 2, member2_uids, num_member2_uids);
	hashtable_load_and_check(testbed, 3, uids, num_uids);

	mark_point();
	
	osync_testing_system_abort("cp newdata2 data2/testdata");
	
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_first, GINT_TO_POINTER(3));
	synchronize_once(engine, NULL);
	fail_unless(num_engine_connected == 1, NULL);
	fail_unless(num_engine_read == 1, NULL);
	fail_unless(num_engine_written == 1, NULL);
	fail_unless(num_engine_disconnected == 1, NULL);
	fail_unless(num_change_read == 3, NULL);
	fail_unless(num_mapping_conflicts == 1, NULL);
	fail_unless(num_change_written == 2, NULL);
	fail_unless(num_engine_end_conflicts == 1, NULL);
	
	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	
	validate_mapping_table(testbed, 3, uids, num_uids);

	hashtable_load_and_check(testbed, 1, uids, num_uids);
	hashtable_load_and_check(testbed, 2, uids, num_uids);
	hashtable_load_and_check(testbed, 3, uids, num_uids);

	
	osync_testing_system_abort("rm -f data1/*");

	synchronize_once(engine, NULL);
	destroy_engine(engine);

	check_empty(testbed);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST(multisync_easy_new_b)
{
	char *testbed = setup_testbed("multisync_easy_new");
	setenv("BATCH_COMMIT", "7", TRUE);
	multisync_easy_new_inner(testbed);
	destroy_testbed(testbed);
}
END_TEST

START_TEST(multisync_triple_del_b)
{
	char *testbed = setup_testbed("multisync_easy_new");
	setenv("BATCH_COMMIT", "7", TRUE);
	multisync_triple_del_inner(testbed);
	destroy_testbed(testbed);
}
END_TEST

START_TEST(multisync_conflict_hybrid_choose2_b)
{
	char *testbed = setup_testbed("multisync_conflict_changetype_choose");
	setenv("BATCH_COMMIT", "7", TRUE);
	multisync_conflict_hybrid_choose2_inner(testbed);
	destroy_testbed(testbed);
}
END_TEST

START_TEST(multisync_delayed_conflict_handler_b)
{
	char *testbed = setup_testbed("multisync_easy_new");
	setenv("BATCH_COMMIT", "7", TRUE);
	multisync_delayed_conflict_handler_inner(testbed);
	destroy_testbed(testbed);
}
END_TEST

START_TEST(multisync_delayed_slow_b)
{
	char *testbed = setup_testbed("multisync_easy_new");
	setenv("BATCH_COMMIT", "7", TRUE);
	multisync_delayed_slow_inner(testbed);
	destroy_testbed(testbed);
}
END_TEST
#if 0
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

	osync_testing_system_abort("rm -f data1/*");
		
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
#if 0
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

