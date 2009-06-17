#include "support.h"

#include <opensync/opensync.h>
#include <opensync/opensync-group.h>
#include <opensync/opensync_internals.h>

#include "opensync/engine/opensync_engine_internals.h"
#include "opensync/engine/opensync_engine_private.h"

#include "opensync/group/opensync_group_internals.h"

START_TEST (lock_simple_lock)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	
	fail_unless(osync_group_lock(group, &error) == OSYNC_LOCK_OK, NULL);
	osync_group_unlock(group);
	osync_group_unref(group);

	fail_unless(!g_file_test("configs/group/lock", G_FILE_TEST_EXISTS), NULL);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (lock_simple_seq_lock)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	
	fail_unless(osync_group_lock(group, &error) == OSYNC_LOCK_OK, NULL);
	osync_group_unlock(group);
	
	fail_unless(osync_group_lock(group, &error) == OSYNC_LOCK_OK, NULL);
	osync_group_unlock(group);
	osync_group_unref(group);

	fail_unless(!g_file_test("configs/group/lock", G_FILE_TEST_EXISTS), NULL);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (lock_dual_lock)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	
	fail_unless(osync_group_lock(group, &error) == OSYNC_LOCK_OK, NULL);
	fail_unless(osync_group_lock(group, &error) == OSYNC_LOCKED, NULL);
	
	osync_group_unlock(group);
	osync_group_unref(group);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (lock_dual_lock2)
{
	char *testbed = setup_testbed("multisync_easy_new");
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	OSyncGroup *group2 = osync_group_new(NULL);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group2, "configs/group", &error);
	
	fail_unless(osync_group_lock(group, &error) == OSYNC_LOCK_OK, NULL);
	fail_unless(osync_group_lock(group2, &error) == OSYNC_LOCKED, NULL);
	
	osync_group_unlock(group);
	osync_group_unref(group);
	osync_group_unref(group2);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (lock_dual_sync_engine_lock)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats",  testbed);
	char *plugindir = g_strdup_printf("%s/plugins",  testbed);
	
	OSyncGroup *group = osync_group_new(NULL);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", NULL);
	OSyncGroup *group2 = osync_group_new(NULL);
	osync_group_set_schemadir(group2, testbed);
	osync_group_load(group2, "configs/group", NULL);
	
	OSyncError *error = NULL;

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(error == NULL, osync_error_print(&error));
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);

	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);
	osync_engine_set_schemadir(engine, testbed);

	OSyncEngine *engine2 = osync_engine_new(group2, &error);
	fail_unless(error == NULL, osync_error_print(&error));
	osync_engine_set_enginestatus_callback(engine2, engine_status, NULL);

	osync_engine_set_plugindir(engine2, plugindir);
	osync_engine_set_formatdir(engine2, formatdir);
	osync_engine_set_schemadir(engine2, testbed);

	fail_unless(osync_engine_initialize(engine, &error), osync_error_print(&error));
	fail_unless(!osync_engine_initialize(engine2, &error), osync_error_print(&error));
	fail_unless(osync_error_is_set(&error), osync_error_print(&error));
	osync_error_unref(&error);
	
	fail_unless(synchronize_once(engine, &error), NULL);
	fail_unless(num_engine_prev_unclean == 0, NULL);
	fail_unless(!synchronize_once(engine2, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	osync_error_unref(&error);
	fail_unless(num_engine_prev_unclean == 0, NULL);
	osync_engine_finalize(engine, &error);
	fail_unless(error == NULL, osync_error_print(&error));
	
	fail_unless(osync_engine_initialize(engine2, &error), NULL);
	fail_unless(synchronize_once(engine2, &error), NULL);
	fail_unless(num_engine_prev_unclean == 0, NULL);
	osync_engine_finalize(engine2, &error);
	fail_unless(error == NULL, osync_error_print(&error));
	
	osync_engine_unref(engine);
	osync_engine_unref(engine2);
	
	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));
	
	osync_group_unref(group);
	osync_group_unref(group2);
	
	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (lock_dual_sync_engine_unclean)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats",  testbed);
	char *plugindir = g_strdup_printf("%s/plugins",  testbed);
	
	OSyncGroup *group = osync_group_new(NULL);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", NULL);
	
	OSyncError *error = NULL;
	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(error == NULL, osync_error_print(&error));
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);
	osync_engine_set_schemadir(engine, testbed);

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, osync_error_print(&error));

	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);

	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	osync_group_unref(group);

	/* Ugly hack to simulate a engine crash.
	 * 
	 * osync_group_unref() cleans up the lockfile. osync_group_unref can't be skipped,
	 * since the process would keep the exclusive lock of the lockfile.
	 *
	 * So we just create a dummy lockfile without any exclusive lock:
	 */

	int lock_fd = g_open("configs/group/lock", O_CREAT | O_WRONLY, 00700);
	fail_unless(lock_fd > 0); 
	close(lock_fd);


	group = osync_group_new(NULL);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", NULL);
	engine = osync_engine_new(group, &error);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);

	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);
	osync_engine_set_schemadir(engine, testbed);
	
	num_engine_prev_unclean = 0;
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(num_engine_prev_unclean == 1, NULL);
	
	OSyncList *o;
	for (o = engine->object_engines; o; o = o->next) {
		OSyncObjEngine *objengine = o->data;
		fail_unless(osync_obj_engine_get_slowsync(objengine), "Slow Sync got NOT set for ObjEngine! But previous sync was unclean!");
	}

	fail_unless(synchronize_once(engine, &error), NULL);
	osync_engine_finalize(engine, &error);
	fail_unless(error == NULL, osync_error_print(&error));
	osync_engine_unref(engine);
	osync_group_unref(group);
	
	group = osync_group_new(NULL);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", NULL);
	engine = osync_engine_new(group, &error);
	osync_engine_set_enginestatus_callback(engine, engine_status, NULL);
	
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);
	osync_engine_set_schemadir(engine, testbed);
	
	for (o = engine->object_engines; o; o = o->next) {
		OSyncObjEngine *objengine = o->data;
		fail_unless(!osync_obj_engine_get_slowsync(objengine), "Slow Sync got set for ObjEngine! But previous sync was clean!");
	}
	fail_unless(osync_engine_initialize(engine, &error), NULL);

	for (o = engine->object_engines; o; o = o->next) {
		OSyncObjEngine *objengine = o->data;
		fail_unless(!osync_obj_engine_get_slowsync(objengine), "Slow Sync got set for ObjEngine! But previous sync was clean!");
	}

	
	fail_unless(synchronize_once(engine, &error), NULL);
	osync_engine_finalize(engine, &error);
	fail_unless(error == NULL, osync_error_print(&error));
	osync_engine_unref(engine);
	
	fail_unless(num_engine_prev_unclean == 0, NULL);
	
	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));

	osync_group_unref(group);
	
	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_TESTCASE_START("lock")
OSYNC_TESTCASE_ADD(lock_simple_lock)
OSYNC_TESTCASE_ADD(lock_simple_seq_lock)
OSYNC_TESTCASE_ADD(lock_dual_lock)
OSYNC_TESTCASE_ADD(lock_dual_lock2)
OSYNC_TESTCASE_ADD(lock_dual_sync_engine_lock)
OSYNC_TESTCASE_ADD(lock_dual_sync_engine_unclean)
OSYNC_TESTCASE_END

