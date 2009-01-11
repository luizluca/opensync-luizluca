#include "support.h"

#include <opensync/opensync-version.h>
#include <opensync/version/opensync-version_internals.h>

START_TEST (version_new)
{
	char *testbed = setup_testbed(NULL);

	OSyncError *error = NULL;
	OSyncVersion *version = osync_version_new(&error);
	fail_unless(version != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_version_ref(version);
	osync_version_unref(version);
	
	osync_version_unref(version);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (version_matches)
{
	char *testbed = setup_testbed(NULL);

	OSyncError *error = NULL;
	OSyncVersion *version = osync_version_new(&error);
	fail_unless(version != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_version_set_plugin(version, "SyncML");
	osync_version_set_vendor(version, "Nokia");
	osync_version_set_modelversion(version, "7650");
	osync_version_set_firmwareversion(version, "*");
	osync_version_set_softwareversion(version, "*");
	osync_version_set_hardwareversion(version, "*");

	OSyncVersion *pattern = osync_version_new(&error);
	fail_unless(pattern != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_version_set_plugin(pattern, "Sync[A-Z]");
	osync_version_set_priority(pattern, "100");
	osync_version_set_vendor(pattern, "Nokia");
	osync_version_set_modelversion(pattern, "[0-9]");
	osync_version_set_firmwareversion(pattern, "");
	osync_version_set_softwareversion(pattern, "");
	osync_version_set_hardwareversion(pattern, "");

	fail_unless(osync_version_matches(pattern, version, &error) > 0, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_version_set_firmwareversion(pattern, "[0-9]");

	fail_unless(osync_version_matches(pattern, version, &error) == 0, NULL);
	fail_unless(error == NULL, NULL);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (version_load_from_descriptions)
{
	char *testbed = setup_testbed(NULL);

	OSyncError *error = NULL;
	OSyncList *versions = osync_version_load_from_descriptions(&error, testbed, testbed);
	//fail_unless(versions != NULL, NULL);
	fail_unless(error == NULL, NULL);

	OSyncList *cur = osync_list_first(versions);
	while(cur) {
		osync_version_unref(cur->data);
		cur = cur->next;
	}
	osync_list_free(versions);

	destroy_testbed(testbed);
}
END_TEST

OSYNC_TESTCASE_START("version")
OSYNC_TESTCASE_ADD(version_new)
OSYNC_TESTCASE_ADD(version_matches)
OSYNC_TESTCASE_ADD(version_load_from_descriptions)
OSYNC_TESTCASE_END

