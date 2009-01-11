#include "support.h"

#include <opensync/opensync-module.h>
#include "opensync/module/opensync_module_internals.h"

START_TEST (module_create)
{
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncModule *module = osync_module_new(&error);
	fail_unless(module != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_module_ref(module);
	osync_module_unref(module);
	osync_module_unref(module);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (module_load)
{
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncModule *module = osync_module_new(&error);
	fail_unless(module != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	char *curdir = g_get_current_dir();
	char *path = g_strdup_printf("%s/formats/mock-format.%s", curdir, G_MODULE_SUFFIX);
	fail_unless(osync_module_load(module, path, &error), NULL);
	fail_unless(error == NULL, NULL);
	g_free(path);
	g_free(curdir);
	
	osync_module_unload(module);
	
	osync_module_unref(module);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (module_load_false)
{
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncModule *module = osync_module_new(&error);
	fail_unless(module != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	char *curdir = g_get_current_dir();
	char *path = g_strdup_printf("%s/does-not-exist.%s", curdir, G_MODULE_SUFFIX);
	fail_unless(!osync_module_load(module, path, &error), NULL);
	fail_unless(error != NULL, NULL);
	g_free(path);
	g_free(curdir);
	osync_error_unref(&error);
	
	osync_module_unref(module);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (module_function)
{
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncModule *module = osync_module_new(&error);
	fail_unless(module != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	char *curdir = g_get_current_dir();
	char *path = g_strdup_printf("%s/formats/mock-format.%s", curdir, G_MODULE_SUFFIX);
	fail_unless(osync_module_load(module, path, &error), NULL);
	fail_unless(error == NULL, NULL);
	g_free(path);
	g_free(curdir);
	
	void *func = osync_module_get_function(module, "get_version", &error);
	fail_unless(func != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_module_unload(module);
	
	osync_module_unref(module);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (module_function_false)
{
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncModule *module = osync_module_new(&error);
	fail_unless(module != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	char *curdir = g_get_current_dir();
	char *path = g_strdup_printf("%s/formats/mock-format.%s", curdir, G_MODULE_SUFFIX);
	fail_unless(osync_module_load(module, path, &error), NULL);
	fail_unless(error == NULL, NULL);
	g_free(path);
	g_free(curdir);
	
	void *func = osync_module_get_function(module, "get_version1", &error);
	fail_unless(func == NULL, NULL);
	fail_unless(error != NULL, NULL);
	osync_error_unref(&error);
	
	osync_module_unload(module);
	
	osync_module_unref(module);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (module_version)
{
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncModule *module = osync_module_new(&error);
	fail_unless(module != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	char *curdir = g_get_current_dir();
	char *path = g_strdup_printf("%s/formats/mock-format.%s", curdir, G_MODULE_SUFFIX);
	fail_unless(osync_module_load(module, path, &error), NULL);
	fail_unless(error == NULL, NULL);
	g_free(path);
	g_free(curdir);
	
	int version = osync_module_get_version(module);
	fail_unless(version == 1, NULL);
	
	osync_module_unload(module);
	
	osync_module_unref(module);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (module_check)
{
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncModule *module = osync_module_new(&error);
	fail_unless(module != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	char *curdir = g_get_current_dir();
	char *path = g_strdup_printf("%s/formats/mock-format.%s", curdir, G_MODULE_SUFFIX);
	fail_unless(osync_module_load(module, path, &error), NULL);
	fail_unless(error == NULL, NULL);
	g_free(path);
	g_free(curdir);
	
	fail_unless(osync_module_check(module, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	osync_module_unload(module);
	
	osync_module_unref(module);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_TESTCASE_START("module")
OSYNC_TESTCASE_ADD(module_create)
OSYNC_TESTCASE_ADD(module_load)
OSYNC_TESTCASE_ADD(module_load_false)
OSYNC_TESTCASE_ADD(module_function)
OSYNC_TESTCASE_ADD(module_function_false)
OSYNC_TESTCASE_ADD(module_version)
OSYNC_TESTCASE_ADD(module_check)
OSYNC_TESTCASE_END

