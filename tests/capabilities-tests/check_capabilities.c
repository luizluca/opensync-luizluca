#include "support.h"

#include <opensync/opensync-capabilities.h>
#include "opensync/capabilities/opensync-capabilities_internals.h"

START_TEST (capabilities_new)
{
	char *testbed = setup_testbed("capabilities");

	OSyncError *error = NULL;
	OSyncCapabilities *capabilities = osync_capabilities_new("testformat", &error);
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

	OSyncCapability *capability = osync_capability_new(&error);
	fail_unless(capability != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_capability_set_name(capability, "Name");
	
	osync_capability_unref(capability);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (capabilities_parse)
{
	char *testbed = setup_testbed("capabilities");

	char *dummy_caps_file = osync_strdup_printf("%s%cdummy.caps", testbed, G_DIR_SEPARATOR);
	
	OSyncCapability *capability;
	OSyncError *error = NULL;

	OSyncCapabilities *capabilities = osync_capabilities_new("testformat", &error);
	fail_unless(capabilities != NULL, NULL);
	fail_unless(error == NULL, NULL);


	OSyncCapabilitiesObjType *capsobjtype = osync_capabilities_objtype_new(capabilities, "contact", &error);
	fail_unless(capsobjtype != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	/** capmock1 */
	capability = osync_capability_new(&error);
	fail_unless(capability != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_capability_set_name(capability, "capmock1");
	osync_capabilities_objtype_add_capability(capsobjtype, capability);
	osync_capability_unref(capability);

	/** capmock2 */
	capability = osync_capability_new(&error);
	fail_unless(capability != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_capability_set_name(capability, "capmock2");
	osync_capabilities_objtype_add_capability(capsobjtype, capability);
	osync_capability_unref(capability);

	/** capmock3 */
	capability = osync_capability_new(&error);
	fail_unless(capability != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_capability_set_name(capability, "capmock3");
	osync_capabilities_objtype_add_capability(capsobjtype, capability);
	osync_capability_unref(capability);

	/** capmock4 as child */
	capability = osync_capability_new_child(capability, &error);
	fail_unless(capability != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_capability_set_name(capability, "capmock4");

	fail_unless(osync_capabilities_save(capabilities, dummy_caps_file, &error) != FALSE, NULL);
	fail_unless(error == NULL, NULL);

	osync_capabilities_unref(capabilities);

	/** Parse */
	capabilities = osync_capabilities_load(dummy_caps_file, &error);
	fail_unless(capabilities != NULL, NULL);
	fail_unless(error == NULL, NULL);

	capsobjtype = osync_capabilities_get_objtype(capabilities, "contact");
	fail_unless(capsobjtype != NULL, NULL);

	OSyncList *c = osync_capabilities_objtype_get_caps(capsobjtype);
	fail_unless(osync_list_length(c) == 3, NULL);

	/** capmock1 */
	OSyncCapability *cap = osync_list_nth_data(c, 0);
	fail_unless(cap != NULL, NULL);
	fail_unless(!strcmp(osync_capability_get_name(cap), "capmock1"), NULL);

	/** capmock2 */
	cap = osync_list_nth_data(c, 1);
	fail_unless(cap != NULL, NULL);
	fail_unless(!strcmp(osync_capability_get_name(cap), "capmock2"), NULL);

	/** capmock3 */
	cap = osync_list_nth_data(c, 2);
	fail_unless(cap != NULL, NULL);
	fail_unless(!strcmp(osync_capability_get_name(cap), "capmock3"), NULL);

	/** capmock4, as child */
	cap = osync_list_nth_data(osync_capability_get_childs(cap), 0);
	fail_unless(cap != NULL, NULL);
	fail_unless(!strcmp(osync_capability_get_name(cap), "capmock4"), NULL);

	
	osync_capabilities_unref(capabilities);

	osync_free(dummy_caps_file);

	destroy_testbed(testbed);
}
END_TEST

OSYNC_TESTCASE_START("capabilities")
OSYNC_TESTCASE_ADD(capabilities_new)
OSYNC_TESTCASE_ADD(capability_new)
OSYNC_TESTCASE_ADD(capabilities_parse)
OSYNC_TESTCASE_END

