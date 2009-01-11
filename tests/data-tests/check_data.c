#include "support.h"

#include <opensync/opensync-data.h>
#include <opensync/opensync-format.h>

START_TEST (data_new)
{
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncObjFormat *format = osync_objformat_new("test", "test", &error);
	fail_unless(format != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncData *data = osync_data_new(NULL, 0, format, &error);
	fail_unless(data != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_data_ref(data);
	osync_data_unref(data);
	
	osync_data_unref(data);
	osync_objformat_unref(format);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (data_new_with_data)
{
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncObjFormat *format = osync_objformat_new("test", "test", &error);
	fail_unless(format != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncData *data = osync_data_new("test", 4, format, &error);
	fail_unless(data != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_objformat_unref(format);
	
	char *buffer = NULL;
	unsigned int size = 0;
	osync_data_get_data(data, &buffer, &size);
	fail_unless(!strncmp(buffer, "test", 4), NULL);
	fail_unless(size == 4, NULL);
	
	osync_data_unref(data);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (data_set_data)
{
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncObjFormat *format = osync_objformat_new("test", "test", &error);
	fail_unless(format != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncData *data = osync_data_new(NULL, 0, format, &error);
	fail_unless(data != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_objformat_unref(format);
	
	osync_data_set_data(data, "test", 4);
	
	char *buffer = NULL;
	unsigned int size = 0;
	osync_data_get_data(data, &buffer, &size);
	fail_unless(!strncmp(buffer, "test", 4), NULL);
	fail_unless(size == 4, NULL);
	
	osync_data_unref(data);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (data_set_data2)
{
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncObjFormat *format = osync_objformat_new("test", "test", &error);
	fail_unless(format != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncData *data = osync_data_new(NULL, 0, format, &error);
	fail_unless(data != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_objformat_unref(format);
	
	fail_unless(osync_data_has_data(data) == FALSE, NULL);
	
	osync_data_set_data(data, "test", 4);
	
	fail_unless(osync_data_has_data(data) == TRUE, NULL);
	
	char *buffer = NULL;
	unsigned int size = 0;
	osync_data_get_data(data, &buffer, &size);
	fail_unless(!strncmp(buffer, "test", 4), NULL);
	fail_unless(size == 4, NULL);
	
	osync_data_set_data(data, "test2", 5);
	
	osync_data_get_data(data, &buffer, &size);
	fail_unless(!strncmp(buffer, "test2", 5), NULL);
	fail_unless(size == 5, NULL);
	
	osync_data_unref(data);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (data_objformat)
{
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncObjFormat *format = osync_objformat_new("test", "test", &error);
	fail_unless(format != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncData *data = osync_data_new(NULL, 0, format, &error);
	fail_unless(data != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_data_get_objformat(data) == format, NULL);
	
	osync_data_set_objformat(data, format);
	osync_data_set_objformat(data, format);
	osync_data_set_objformat(data, format);
	
	osync_data_unref(data);
	osync_objformat_unref(format);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (data_objtype)
{
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncObjFormat *format = osync_objformat_new("test", "objtype", &error);
	fail_unless(format != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	OSyncData *data = osync_data_new(NULL, 0, format, &error);
	fail_unless(data != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	/* Quote from osync_data.c ////////////////////////////
	 *  
	 * If no object type is explicitly set, we will just
	 * return the default objtype for this format

	OSyncObjFormat *format = data->objformat;
	if (format)
		return osync_objformat_get_objtype(format);
         
         * ////// End of Quote ////////////////////////////////
		
 	// Obsolate!
	// fail_unless(osync_data_get_objtype(data) == NULL, NULL);
	*/
	
	osync_data_set_objtype(data, "objtype");
	fail_unless(!strcmp(osync_data_get_objtype(data), "objtype"), NULL);

	osync_data_set_objtype(data, "objtype");
	osync_data_set_objtype(data, "objtype2");
	fail_unless(!strcmp(osync_data_get_objtype(data), "objtype2"), NULL);
	
	osync_data_unref(data);
	osync_objformat_unref(format);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_TESTCASE_START("data")
OSYNC_TESTCASE_ADD(data_new)
OSYNC_TESTCASE_ADD(data_new_with_data)
OSYNC_TESTCASE_ADD(data_set_data)
OSYNC_TESTCASE_ADD(data_set_data2)
OSYNC_TESTCASE_ADD(data_objformat)
OSYNC_TESTCASE_ADD(data_objtype)
OSYNC_TESTCASE_END

