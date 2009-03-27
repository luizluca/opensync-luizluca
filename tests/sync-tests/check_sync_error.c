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

OSYNC_TESTCASE_START("sync_error")
OSYNC_TESTCASE_ADD(sync_error_single_init_error_noerror)

OSYNC_TESTCASE_END

