#include "support.h"

#include <opensync/opensync-group.h>
#include <opensync/opensync-data.h>
#include <opensync/opensync-format.h>
#include <opensync/opensync-client.h>
#include <opensync/opensync-engine.h>
#include <opensync/opensync-plugin.h>

#include "opensync/engine/opensync_engine_internals.h"
#include "opensync/group/opensync_group_internals.h"
#include "opensync/group/opensync_member_internals.h"
#include "opensync/client/opensync_client_internals.h"

#include "../mock-plugin/mock_sync.h"
#include "../mock-plugin/mock_format.h"

START_TEST (context_new)
{
	OSyncError *error = NULL;
	OSyncContext *context = osync_context_new(&error);
	fail_unless(context != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_context_ref(context);
	osync_context_unref(context);
	osync_context_unref(context);
}
END_TEST

START_TEST (context_uid_update)
{
	char *testbed = setup_testbed("sync");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);

	g_setenv("SYNC_DONE_REPORT_UID_UPDATE", "1", TRUE);
	g_setenv("SYNC_DONE_REPORT_UID_UPDATES", "10", TRUE);
	g_setenv("SYNC_DONE_REPORT_UID_UPDATES_OLDUIDS", "0123456789", TRUE);
	g_setenv("SYNC_DONE_REPORT_UID_UPDATES_NEWUIDS", "abcdefghij", TRUE);

	osync_testing_system_abort("cp testdata data2/0");
	osync_testing_system_abort("cp testdata data2/1");
	osync_testing_system_abort("cp testdata data2/2");
	osync_testing_system_abort("cp testdata data2/3");
	osync_testing_system_abort("cp testdata data2/4");
	osync_testing_system_abort("cp testdata data2/5");
	osync_testing_system_abort("cp testdata data2/6");
	osync_testing_system_abort("cp testdata data2/7");
	osync_testing_system_abort("cp testdata data2/8");
	osync_testing_system_abort("cp testdata data2/9");

	
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
	
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_first, GINT_TO_POINTER(1));
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
	
	char *path = g_strdup_printf("%s/configs/group/archive.db", testbed);
	OSyncMappingTable *maptable = mappingtable_load(path, "mockobjtype1", 10);
	g_free(path);

	check_mapping(maptable, 2, 1, 2, "0");
	check_mapping(maptable, 1, 1, 2, "a");

	/* ... gap ... mapping id ... 2 to 4 ... */

	check_mapping(maptable, 2, 5, 2, "4");
	check_mapping(maptable, 1, 5, 2, "e");

	/* ... gap ... mapping id ... 6 to 8 ... */

	check_mapping(maptable, 2, 9, 2, "8");
	check_mapping(maptable, 1, 9, 2, "i");

	check_mapping(maptable, 2, 10, 2, "9");
	check_mapping(maptable, 1, 10, 2, "j");

	osync_mapping_table_close(maptable);
	osync_mapping_table_unref(maptable);
    
	path = g_strdup_printf("%s/configs/group/1/hashtable.db", testbed);
	OSyncHashTable *table = hashtable_load(path, "mockobjtype1", 10);
	g_free(path);
	check_hash(table, "a"); /* This is the important check here ... hashtable needs also get updated if uid changes */
	check_hash(table, "e"); /* This is the important check here ... hashtable needs also get updated if uid changes */
	check_hash(table, "i"); /* This is the important check here ... hashtable needs also get updated if uid changes */
	check_hash(table, "j"); /* This is the important check here ... hashtable needs also get updated if uid changes */
	osync_hashtable_unref(table);

	path = g_strdup_printf("%s/configs/group/2/hashtable.db", testbed);
	table = hashtable_load(path, "mockobjtype1", 10);
	g_free(path);
	check_hash(table, "0");
	check_hash(table, "4");
	check_hash(table, "8");
	check_hash(table, "9");
	osync_hashtable_unref(table);

	g_free(formatdir);
	g_free(plugindir);

	destroy_testbed(testbed);

}
END_TEST

OSYNC_TESTCASE_START(context)

OSYNC_TESTCASE_ADD(context_new)
OSYNC_TESTCASE_ADD(context_uid_update)

OSYNC_TESTCASE_END

