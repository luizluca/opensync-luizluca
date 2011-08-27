#include "support.h"

static osync_bool detect(OSyncFormatConverter *conv, const char *data, int size, void *userdata)
{
	return TRUE;
}

static osync_bool detect_false(OSyncFormatConverter *conv, const char *data, int size, void *userdata)
{
	return FALSE;
}

START_TEST (detect_smart)
{
	OSyncError *error = NULL;

	OSyncFormatEnv *env = osync_format_env_new(&error);
	fail_unless(error == NULL);

	OSyncObjFormat *format1 = osync_objformat_new("Format1", "Type1", &error);
	OSyncObjFormat *format2 = osync_objformat_new("Format2", "Type1", &error);
	fail_unless(error == NULL);

	OSyncFormatConverter *conv = osync_converter_new_detector(format2, format1, detect, &error);
	fail_unless(error == NULL);

	osync_format_env_register_converter(env, conv, &error);

	mark_point();

	OSyncData *data = osync_data_new("test", 5, format2, &error);
	fail_unless(error == NULL);

	OSyncObjFormat *result = osync_format_env_detect_objformat(env, data);
	fail_unless(result == format1);
	fail_unless(osync_data_get_objformat(data) == format2);

	osync_data_unref(data);
	osync_format_env_unref(env);
}
END_TEST

START_TEST (detect_different_objtype)
{
	OSyncError *error = NULL;

	OSyncFormatEnv *env = osync_format_env_new(&error);
	fail_unless(error == NULL);

	OSyncObjFormat *format1 = osync_objformat_new("Format1", "Type1", &error);
	// Different objtype!
	OSyncObjFormat *format2 = osync_objformat_new("Format2", "Type2", &error);
	fail_unless(error == NULL);

	OSyncFormatConverter *conv = osync_converter_new_detector(format2, format1, detect, &error);
	fail_unless(error == NULL);

	osync_format_env_register_converter(env, conv, &error);

	mark_point();

	OSyncData *data = osync_data_new("test", 5, format2, &error);
	fail_unless(error == NULL);

	OSyncObjFormat *result = osync_format_env_detect_objformat(env, data);
	fail_unless(result == format1);
	fail_unless(osync_data_get_objformat(data) == format2);

	osync_data_unref(data);
	osync_format_env_unref(env);
}
END_TEST


START_TEST (detect_smart_no)
{
	OSyncError *error = NULL;

	OSyncFormatEnv *env = osync_format_env_new(&error);
	fail_unless(error == NULL);

	OSyncObjFormat *format1 = osync_objformat_new("Format1", "Type1", &error);
	OSyncObjFormat *format2 = osync_objformat_new("Format2", "Type1", &error);
	fail_unless(error == NULL);

	OSyncFormatConverter *conv = osync_converter_new_detector(format2, format1, detect_false, &error);
	fail_unless(error == NULL);

	osync_format_env_register_converter(env, conv, &error);

	mark_point();

	OSyncData *data = osync_data_new("test", 5, format2, &error);
	fail_unless(error == NULL);

	OSyncObjFormat *result = osync_format_env_detect_objformat(env, data);
	fail_unless(!result);
	fail_unless(osync_data_get_objformat(data) == format2);

	osync_data_unref(data);
	osync_format_env_unref(env);
}
END_TEST

OSYNC_TESTCASE_START("detect")
OSYNC_TESTCASE_ADD(detect_smart)
OSYNC_TESTCASE_ADD(detect_different_objtype)
OSYNC_TESTCASE_ADD(detect_smart_no)
OSYNC_TESTCASE_END

