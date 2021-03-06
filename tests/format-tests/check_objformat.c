#include "support.h"

#include <opensync/opensync-format.h>
#include <opensync/opensync-serializer.h>
#include "opensync/format/opensync_objformat_internals.h"

static OSyncConvCmpResult compare_format(const char *leftdata, unsigned int leftsize, const char *rightdata, unsigned int rightsize, void *user_data, OSyncError **error)
{
	if (rightsize == leftsize && !strcmp(leftdata, rightdata))
		return OSYNC_CONV_DATA_SAME;
	
	return OSYNC_CONV_DATA_MISMATCH;
}

osync_bool destroy_format(char *data, unsigned int size, void *user_data, OSyncError **error)
{
	g_free(data);
	return TRUE;
}

osync_bool copy_format(const char *indata, unsigned int insize, char **outdata, unsigned int *outsize, void *user_data, OSyncError **error)
{
	*outdata = strdup(indata);
	*outsize = insize;
	return TRUE;
}

static osync_bool duplicate_format(const char *uid, const char *input, unsigned int insize, char **newuid, char **output, unsigned int *outsize, osync_bool *dirty, void *user_data, OSyncError **error)
{
	fail_unless(!strcmp(uid, "uid"), NULL);
	*newuid = strdup("newuid");
	return TRUE;
}

osync_bool create_format(char **data, unsigned int *size, void *user_data, OSyncError **error)
{
	*data = strdup("data");
	*size = 5;
	return TRUE;
}

char *print_format(const char *data, unsigned int size, void *user_data, OSyncError **error)
{
	return strdup(data);
}

time_t revision_format(const char *data, unsigned int size, void *user_data, OSyncError **error)
{
	return atoi(data);
}

osync_bool marshal_format(const char *input, unsigned int inpsize, OSyncMarshal *marshal, void *user_data, OSyncError **error)
{
	return osync_marshal_write_buffer(marshal, input, inpsize, error);
}

osync_bool demarshal_format(OSyncMarshal *marshal, char **output, unsigned int *outsize, void *user_data, OSyncError **error)
{
	return osync_marshal_read_buffer(marshal, (void *)output, outsize, error);
}

START_TEST (objformat_new)
{
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncObjFormat *format = osync_objformat_new("format1", "objtype", &error);
	fail_unless(format != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_objformat_ref(format);
	osync_objformat_unref(format);
	osync_objformat_unref(format);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (objformat_get)
{
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncObjFormat *format = osync_objformat_new("format", "objtype", &error);
	fail_unless(format != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!strcmp(osync_objformat_get_name(format), "format"), NULL);
	fail_unless(!strcmp(osync_objformat_get_objtype(format), "objtype"), NULL);
	
	osync_objformat_unref(format);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (objformat_equal)
{
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncObjFormat *format1 = osync_objformat_new("format", "objtype", &error);
	fail_unless(format1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	OSyncObjFormat *format2 = osync_objformat_new("format", "objtype", &error);
	fail_unless(format2 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	OSyncObjFormat *format3 = osync_objformat_new("format2", "objtype", &error);
	fail_unless(format3 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_objformat_is_equal(format1, format2), NULL);
	fail_unless(!osync_objformat_is_equal(format1, format3), NULL);
	
	osync_objformat_unref(format1);
	osync_objformat_unref(format2);
	osync_objformat_unref(format3);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (objformat_compare)
{
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncObjFormat *format = osync_objformat_new("format", "objtype", &error);
	fail_unless(format != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_objformat_set_compare_func(format, compare_format);
	
	fail_unless(osync_objformat_compare(format, "test", 5, "test", 5, &error) == OSYNC_CONV_DATA_SAME, NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(osync_objformat_compare(format, "test", 5, "tesd", 5, &error) == OSYNC_CONV_DATA_MISMATCH, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_objformat_unref(format);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (objformat_destroy)
{
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncObjFormat *format = osync_objformat_new("format", "objtype", &error);
	fail_unless(format != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_objformat_set_destroy_func(format, destroy_format);
	
	osync_objformat_destroy(format, strdup("test"), 5, &error);
	fail_unless(error == NULL, NULL);
	
	osync_objformat_unref(format);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (objformat_copy)
{
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncObjFormat *format = osync_objformat_new("format", "objtype", &error);
	fail_unless(format != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_objformat_set_copy_func(format, copy_format);
	osync_objformat_set_destroy_func(format, destroy_format);
	
	char *outdata = NULL;
	unsigned int outsize = 0;
	fail_unless(osync_objformat_copy(format, "test", 5, &outdata, &outsize, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!strcmp(outdata, "test"), NULL);
	fail_unless(outsize == 5, NULL);
	
	osync_objformat_destroy(format, outdata, 5, &error);
	fail_unless(error == NULL, NULL);
	
	osync_objformat_unref(format);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (objformat_duplicate)
{
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncObjFormat *format = osync_objformat_new("format", "objtype", &error);
	fail_unless(format != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_objformat_set_duplicate_func(format, duplicate_format);
	
	char *newuid = NULL;
	char *output = NULL;
	unsigned int outsize = 0;
	osync_bool dirty = FALSE;
	fail_unless(osync_objformat_duplicate(format, "uid", "test", 5, &newuid, &output, &outsize, &dirty, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!strcmp(newuid, "newuid"), NULL);
	g_free(newuid);
	
	osync_objformat_unref(format);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (objformat_create)
{
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncObjFormat *format = osync_objformat_new("format", "objtype", &error);
	fail_unless(format != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_objformat_set_create_func(format, create_format);
	osync_objformat_set_destroy_func(format, destroy_format);
	
	char *outdata = NULL;
	unsigned int outsize = 0;
	osync_objformat_create(format, &outdata, &outsize, &error);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!strcmp(outdata, "data"), NULL);
	fail_unless(outsize == 5, NULL);
	
	osync_objformat_destroy(format, outdata, 5, &error);
	fail_unless(error == NULL, NULL);
	
	osync_objformat_unref(format);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (objformat_print)
{
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncObjFormat *format = osync_objformat_new("format", "objtype", &error);
	fail_unless(format != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_objformat_set_print_func(format, print_format);
	
	char *print = osync_objformat_print(format, "test", 5, &error);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!strcmp(print, "test"), NULL);
	g_free(print);
	
	osync_objformat_unref(format);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (objformat_revision)
{
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncObjFormat *format = osync_objformat_new("format", "objtype", &error);
	fail_unless(format != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_objformat_set_revision_func(format, revision_format);
	
	time_t curtime = osync_objformat_get_revision(format, "5", 2, &error);
	fail_unless(error == NULL, NULL);
	
	fail_unless(curtime == 5, NULL);
	
	osync_objformat_unref(format);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (objformat_marshal)
{
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncObjFormat *format = osync_objformat_new("format", "objtype", &error);
	fail_unless(format != NULL, NULL);
	fail_unless(error == NULL, NULL);
	fail_unless(osync_objformat_must_marshal(format) == FALSE, NULL);
	
	osync_objformat_set_marshal_func(format, marshal_format);
	osync_objformat_set_destroy_func(format, destroy_format);
	
	fail_unless(osync_objformat_must_marshal(format) == TRUE, NULL);

	OSyncMarshal *marshal = osync_marshal_new(&error);
	fail_unless(marshal != NULL, NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(osync_objformat_marshal(format, "test", 5, marshal, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	osync_marshal_unref(marshal);
	
	osync_objformat_unref(format);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (objformat_demarshal)
{
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncObjFormat *format = osync_objformat_new("format", "objtype", &error);
	fail_unless(format != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_objformat_set_demarshal_func(format, demarshal_format);
	osync_objformat_set_marshal_func(format, marshal_format);
	osync_objformat_set_destroy_func(format, destroy_format);
	
	OSyncMarshal *marshal = osync_marshal_new(&error);
	fail_unless(marshal != NULL, NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(osync_objformat_marshal(format, "test", 5, marshal, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	char *outdata = NULL;
	unsigned int outsize = 0;
	fail_unless(osync_objformat_demarshal(format, marshal, &outdata, &outsize, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!strcmp(outdata, "test"), NULL);
	fail_unless(outsize == 5, NULL);
	g_free(outdata);
	
	osync_objformat_unref(format);
	osync_marshal_unref(marshal);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_TESTCASE_START("objformat")
OSYNC_TESTCASE_ADD(objformat_new)
OSYNC_TESTCASE_ADD(objformat_get)
OSYNC_TESTCASE_ADD(objformat_equal)
OSYNC_TESTCASE_ADD(objformat_compare)
OSYNC_TESTCASE_ADD(objformat_destroy)
OSYNC_TESTCASE_ADD(objformat_copy)
OSYNC_TESTCASE_ADD(objformat_duplicate)
OSYNC_TESTCASE_ADD(objformat_create)
OSYNC_TESTCASE_ADD(objformat_print)
OSYNC_TESTCASE_ADD(objformat_revision)
OSYNC_TESTCASE_ADD(objformat_marshal)
OSYNC_TESTCASE_ADD(objformat_demarshal)
OSYNC_TESTCASE_END

