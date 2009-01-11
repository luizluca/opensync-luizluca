#include "support.h"

#include <opensync/opensync-capabilities.h>
#include "opensync/capabilities/opensync-capabilities_internals.h"

START_TEST (capabilities_new)
{
	char *testbed = setup_testbed("capabilities");

	OSyncError *error = NULL;
	OSyncCapabilities *capabilities = osync_capabilities_new(&error);
	fail_unless(capabilities != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_capabilities_ref(capabilities);
	osync_capabilities_unref(capabilities);
	
	osync_capabilities_unref(capabilities);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (capability_new)
{
	char *testbed = setup_testbed("capabilities");

	OSyncError *error = NULL;
	OSyncCapabilities *capabilities = osync_capabilities_new(&error);
	fail_unless(capabilities != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncCapability *capability = osync_capability_new(capabilities, "contact", "Name", &error);
	fail_unless(capability != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_capabilities_unref(capabilities);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (capabilities_parse)
{
	char *testbed = setup_testbed("capabilities");
	
	OSyncError *error = NULL;
	char* buffer;
	unsigned int size;
	fail_unless(osync_file_read("capabilities.xml", &buffer, &size, &error), NULL);

	OSyncCapabilities *capabilities = osync_capabilities_parse(buffer, size, &error);
	fail_unless(capabilities != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_capabilities_unref(capabilities);

	g_free(buffer);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (capabilities_sort)
{
	char *testbed = setup_testbed("capabilities");
	
	OSyncError *error = NULL;
	char* buffer;
	unsigned int size;
	fail_unless(osync_file_read("capabilities.xml", &buffer, &size, &error), NULL);

	OSyncCapabilities *capabilities = osync_capabilities_parse(buffer, size, &error);
	fail_unless(capabilities != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_capabilities_sort(capabilities);
	
	osync_capabilities_unref(capabilities);

	g_free(buffer);

	destroy_testbed(testbed);
}
END_TEST

OSYNC_TESTCASE_START("capabilities")
OSYNC_TESTCASE_ADD(capabilities_new)
OSYNC_TESTCASE_ADD(capability_new)
OSYNC_TESTCASE_ADD(capabilities_parse)
OSYNC_TESTCASE_ADD(capabilities_sort)
OSYNC_TESTCASE_END

