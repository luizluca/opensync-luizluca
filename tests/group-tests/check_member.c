#include <check.h>
#include <opensync/opensync.h>
#include <opensync/opensync-group.h>
#include <opensync/opensync-plugin.h>
#include <opensync/opensync_internals.h>
#include <opensync/group/opensync_member_internals.h>
#include <stdlib.h>

#include <glib.h>
#include <gmodule.h>

#include "support.h"

/** @brief Calls osync_member_new setting the schema dir and
    checking for success
 */
OSyncMember *osync_testing_member_new(const char *testbed)
{
	OSyncError *error = NULL;
	OSyncMember *member = NULL;
	member = osync_member_new(&error);
	fail_unless(member != NULL, "Member == NULL on creation");
	fail_unless(error == NULL, NULL);
	if (testbed)
		osync_member_set_schemadir(member, testbed);
	return member;
}

START_TEST (member_new)
{
	OSyncMember *member = osync_testing_member_new(NULL);
	osync_member_unref(member);
}
END_TEST

START_TEST (member_ref)
{
	OSyncMember *member = NULL;
	OSyncMember *member2 = NULL;
	member = osync_testing_member_new(NULL);

	member2 = osync_member_ref(member);
	fail_unless(member == member2, NULL);

	osync_member_unref(member);
	osync_member_unref(member);
}
END_TEST

START_TEST (member_name)
{
	OSyncMember *member = NULL;
	const char *membername = NULL;
	member = osync_testing_member_new(NULL);

	osync_member_set_name(member, "foo");
	membername = osync_member_get_name(member);
	fail_unless(membername != NULL, NULL);
	fail_unless(strcmp(membername, "foo") == 0, "Expecting member name 'foo' got '%s'\n", membername);

	/* Overwrite (leak check) */
	osync_member_set_name(member, "bar");
	membername = osync_member_get_name(member);
	fail_unless(membername != NULL, NULL);
	fail_unless(strcmp(membername, "bar") == 0, NULL);

	osync_member_unref(member);	
}
END_TEST

START_TEST (member_name_save_and_load)
{
	char *testbed = setup_testbed("filter_save_and_load");

	OSyncError *error = NULL;
	OSyncMember *member = NULL;
	const char *membername = NULL;
	member = osync_testing_member_new(testbed);

	fail_unless(osync_member_load(member, "configs/group/1", &error), NULL);
	fail_unless(error == NULL, NULL);

	osync_member_set_name(member, "foo");
	membername = osync_member_get_name(member);
	fail_unless(membername != NULL, NULL);
	fail_unless(strcmp(membername, "foo") == 0, NULL);

	fail_unless(osync_member_save(member, &error), NULL);
	fail_unless(error == NULL, NULL);

	osync_member_unref(member);
	member = osync_testing_member_new(testbed);

	/* Reload and check it worked */
	fail_unless(osync_member_load(member, "configs/group/1", &error), NULL);
	fail_unless(error == NULL, NULL);

	membername = osync_member_get_name(member);
	fail_unless(membername != NULL, NULL);
	fail_unless(strcmp(membername, "foo") == 0, "After reloading member got name '%s' expecting 'foo'\n", membername);

	osync_member_unref(member);
	destroy_testbed(testbed);
}
END_TEST

START_TEST (member_pluginname)
{
	char *testbed = setup_testbed("filter_save_and_load");

	OSyncError *error = NULL;
	OSyncMember *member = NULL;
	const char *pluginname = NULL;
	member = osync_testing_member_new(testbed);

	fail_unless(osync_member_load(member, "configs/group/1", &error), NULL);
	fail_unless(error == NULL, NULL);

	pluginname = osync_member_get_pluginname(member);
	fail_unless(pluginname != NULL, NULL);
	fail_unless(strcmp(pluginname, "file-sync") == 0, NULL);

	osync_member_set_pluginname(member, "foo");
	pluginname = osync_member_get_pluginname(member);
	fail_unless(pluginname != NULL, NULL);
	fail_unless(strcmp(pluginname, "foo") == 0, NULL);

	osync_member_set_pluginname(member, "bar");
	pluginname = osync_member_get_pluginname(member);
	fail_unless(pluginname != NULL, NULL);
	fail_unless(strcmp(pluginname, "bar") == 0, NULL);

  	fail_unless(osync_member_save(member, &error), NULL);
	fail_unless(error == NULL, NULL);

	osync_member_unref(member);
	member = osync_testing_member_new(testbed);

	/* Reload and check it worked */
	fail_unless(osync_member_load(member, "configs/group/1", &error), NULL);
	fail_unless(error == NULL, NULL);  

	pluginname = osync_member_get_pluginname(member);
	fail_unless(pluginname != NULL, NULL);
	fail_unless(strcmp(pluginname, "bar") == 0, "After reloading member got plugin name '%s' expecting 'bar'\n", pluginname);

	osync_member_unref(member);
	destroy_testbed(testbed);
}
END_TEST


START_TEST (member_configdir)
{
	char *testbed = setup_testbed("filter_save_and_load");
	OSyncError *error = NULL;
	OSyncMember *member = NULL;
	const char *configdir = NULL;
	member = osync_testing_member_new(testbed);

	fail_unless(osync_member_load(member, "configs/group/1", &error), NULL);
	fail_unless(error == NULL, NULL);

	configdir = osync_member_get_configdir(member);
	fail_unless(configdir != NULL, NULL);
	fail_unless(strcmp(configdir, "configs/group/1") == 0, NULL);

	
	osync_member_set_configdir(member, "configdir_test");
	configdir = osync_member_get_configdir(member);
	fail_unless(configdir != NULL, NULL);
	fail_unless(strcmp(configdir, "configdir_test") == 0, NULL);

	fail_unless(osync_member_save(member, &error), NULL);
	fail_unless(error == NULL, NULL);

	osync_member_unref(member);

	fail_unless(osync_testing_diff("configs/group/1", "configdir_test"), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (member_configdir_deep_path)
{
	char *testbed = setup_testbed("filter_save_and_load");
	OSyncError *error = NULL;
	OSyncMember *member = NULL;
	const char *configdir = NULL;
	member = osync_testing_member_new(testbed);

	fail_unless(osync_member_load(member, "configs/group/1", &error), NULL);
	fail_unless(error == NULL, NULL);

	configdir = osync_member_get_configdir(member);
	fail_unless(configdir != NULL, NULL);
	fail_unless(strcmp(configdir, "configs/group/1") == 0, NULL);

	
	osync_member_set_configdir(member, "configdir_test/group/1");
	configdir = osync_member_get_configdir(member);
	fail_unless(configdir != NULL, NULL);
	fail_unless(strcmp(configdir, "configdir_test/group/1") == 0, NULL);

	fail_unless(osync_member_save(member, &error), NULL);
	fail_unless(error == NULL, NULL);

	osync_member_unref(member);

	fail_unless(osync_testing_diff("configs/group/1", "configdir_test/group/1"), NULL);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (member_config)
{	
	OSyncError *error = NULL;
	OSyncMember *member = NULL;
	OSyncPluginConfig *config = NULL;
	OSyncPluginConfig *config2 = NULL;
	member = osync_testing_member_new(NULL);

	fail_if(osync_member_has_config(member), NULL);

	config = osync_plugin_config_new(&error);
	fail_unless(config != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_member_set_config(member, config);
	fail_unless(osync_member_has_config(member), NULL);
	fail_unless(osync_member_get_config(member, &error) == config, NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(osync_member_get_config_or_default(member, &error) == config, NULL);
	fail_unless(error == NULL, NULL);

	osync_plugin_config_unref(config);
	config2 = osync_plugin_config_new(&error);
	fail_unless(config != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_member_set_config(member, config2);
	fail_unless(osync_member_has_config(member), NULL);
	fail_unless(osync_member_get_config(member, &error) == config2, NULL);
	fail_unless(error == NULL, NULL);

	osync_member_unref(member);
	osync_plugin_config_unref(config2);
}
END_TEST

/* Need to test 
   if config loaded then returned config is same
   if config file then that is loaded
   if no config then default is loaded
*/
START_TEST (member_get_config_or_default)
{	
	char *testbed = setup_testbed("plugin_no_config");
	OSyncError *error = NULL;
	OSyncMember *member = NULL;
	OSyncPluginConfig *config = NULL;
	OSyncPluginConfig *config2 = NULL;
	member = osync_testing_member_new(testbed);

	fail_unless(osync_member_load(member, "configs/group/1", &error), NULL);
	fail_unless(error == NULL, NULL);

	osync_member_set_default_configdir(member, "defaults");

	config = osync_member_get_config_or_default(member, &error);
	fail_unless(config != NULL, NULL);
	fail_unless(error == NULL, NULL);
	fail_unless(osync_member_has_config(member), NULL);

	fail_unless(osync_member_save(member, &error), NULL);
	fail_unless(error == NULL, NULL);

	osync_member_unref(member);
	fail_unless(osync_testing_file_exists("configs/group/1/file-sync.conf"));

	/* Reload with config in place */
	member = osync_testing_member_new(testbed);

	fail_unless(osync_member_load(member, "configs/group/1", &error), NULL);
	fail_unless(error == NULL, NULL);

	config = osync_member_get_config_or_default(member, &error);
	fail_unless(config != NULL, NULL);
	fail_unless(error == NULL, NULL);
	fail_unless(osync_member_has_config(member), NULL);

	/* Check we get the same config now it has been loaded */
	config2 = osync_member_get_config_or_default(member, &error);
	fail_unless(config2 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	fail_unless(config2 == config, NULL);

	config2 = osync_member_get_config(member, &error);
	fail_unless(config2 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	fail_unless(config2 == config, NULL);

	osync_member_unref(member);
	destroy_testbed(testbed);
}
END_TEST

START_TEST (member_id)
{
	char *testbed = setup_testbed("filter_save_and_load");
	
	OSyncError *error = NULL;
	OSyncMember *member = osync_testing_member_new(testbed);

	fail_unless(osync_member_load(member, "configs/group/2", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless((int)osync_member_get_id(member) == 2, NULL);
	osync_member_unref(member);

	member = osync_testing_member_new(testbed);

	fail_unless(osync_member_load(member, "configs/group/1", &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless((int)osync_member_get_id(member) == 1, NULL);
	osync_member_unref(member);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_TESTCASE_START("member")
OSYNC_TESTCASE_ADD(member_new)
OSYNC_TESTCASE_ADD(member_ref)
OSYNC_TESTCASE_ADD(member_name)
OSYNC_TESTCASE_ADD(member_name_save_and_load)
OSYNC_TESTCASE_ADD(member_pluginname)
OSYNC_TESTCASE_ADD(member_configdir)
OSYNC_TESTCASE_ADD(member_configdir_deep_path)
OSYNC_TESTCASE_ADD(member_config)
OSYNC_TESTCASE_ADD(member_get_config_or_default)
OSYNC_TESTCASE_ADD(member_id)
OSYNC_TESTCASE_END

