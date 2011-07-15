#include "support.h"

#include <opensync/opensync-data.h>
#include <opensync/opensync-format.h>

#include "opensync/format/opensync_converter_internals.h"

osync_bool converter_conv(OSyncFormatConverter *converter, char *input, unsigned int inpsize, char **output, unsigned int *outpsize, osync_bool *free_input, const char *config, void *userdata, OSyncError **error)
{
	return TRUE;
}

osync_bool destroy_format(char *data, unsigned int size, void *user_data, OSyncError **error)
{
	g_free(data);

	return TRUE;
}

typedef struct testdata {
	char *string1;
} testdata;

osync_bool destroy_testdata(char *input, unsigned int size, void *user_data, OSyncError **error)
{
	testdata *data = (testdata *)input;
	if (data->string1)
		g_free(data->string1);
	g_free(data);

	return TRUE;
}

osync_bool conv_format1_to_format2_const(OSyncFormatConverter *converter, char *input, unsigned int inpsize, char **output, unsigned int *outpsize, osync_bool *free_input, const char *config, void *userdata, OSyncError **error)
{
	osync_assert(!strcmp(input, "format1"));
	osync_assert(inpsize == 8);

	input[6] = '2';

	*output = input;
	*outpsize = 8;

	*free_input = FALSE;

	return TRUE;
}

osync_bool decap_format1_to_format2_const(OSyncFormatConverter *converter, char *input, unsigned int inpsize, char **output, unsigned int *outpsize, osync_bool *free_input, const char *config, void *userdata, OSyncError **error)
{
	testdata *data = (testdata *)input;
	*output = data->string1;
	*outpsize = strlen(*output) + 1;
	data->string1 = NULL;
	*free_input = TRUE;

	return TRUE;
}

osync_bool encap_format1_to_format2_const(OSyncFormatConverter *converter, char *input, unsigned int inpsize, char **output, unsigned int *outpsize, osync_bool *free_input, const char *config, void *userdata, OSyncError **error)
{
	osync_assert(!strcmp(input, "format1"));
	osync_assert(inpsize == 8);

	testdata *data = malloc(sizeof(testdata));
	data->string1 = input;
	*output = (char *)data;
	*outpsize = sizeof(testdata);

	*free_input = FALSE;

	return TRUE;
}

osync_bool conv_format1_to_format2_dup(OSyncFormatConverter *converter, char *input, unsigned int inpsize, char **output, unsigned int *outpsize, osync_bool *free_input, const char *config, void *userdata, OSyncError **error)
{
	osync_assert(!strcmp(input, "format1"));
	osync_assert(inpsize == 8);

	*output = strdup("format2");
	*outpsize = 8;

	*free_input = TRUE;

	return TRUE;
}

osync_bool decap_format1_to_format2_dup(OSyncFormatConverter *converter, char *input, unsigned int inpsize, char **output, unsigned int *outpsize, osync_bool *free_input, const char *config, void *userdata, OSyncError **error)
{
	osync_assert(!strncmp(input, "SHELL", 5));

	*output = strdup(input + 5);
	*outpsize = strlen(*output) + 1;

	*free_input = TRUE;

	return TRUE;
}

osync_bool encap_format1_to_format2_dup(OSyncFormatConverter *converter, char *input, unsigned int inpsize, char **output, unsigned int *outpsize, osync_bool *free_input, const char *config, void *userdata, OSyncError **error)
{
	osync_assert(!strcmp(input, "format1"));
	osync_assert(inpsize == 8);

	*output = g_strdup_printf("SHELL%s", input);
	*outpsize = strlen(*output) + 1;

	*free_input = TRUE;

	return TRUE;
}

osync_bool conv_detect(OSyncFormatConverter *converter, const char *data, int size, void *userdata)
{
	if (!strcmp(data, "format2"))
		return TRUE;
	return FALSE;
}

START_TEST (converter_create)
{
	char *testbed = setup_testbed(NULL);

	OSyncError *error = NULL;
	OSyncObjFormat *format1 = osync_objformat_new("format1", "objtype", &error);
	fail_unless(format1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	OSyncObjFormat *format2 = osync_objformat_new("format2", "objtype", &error);
	fail_unless(format2 != NULL, NULL);
	fail_unless(error == NULL, NULL);

	OSyncFormatConverter *converter = osync_converter_new(OSYNC_CONVERTER_CONV, format1, format2, converter_conv, &error);
	fail_unless(converter != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_converter_ref(converter);
	osync_converter_unref(converter);

	osync_objformat_unref(format1);
	osync_objformat_unref(format2);

	osync_converter_unref(converter);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (converter_get)
{
	char *testbed = setup_testbed(NULL);

	OSyncError *error = NULL;
	OSyncObjFormat *format1 = osync_objformat_new("format1", "objtype", &error);
	fail_unless(format1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	OSyncObjFormat *format2 = osync_objformat_new("format2", "objtype", &error);
	fail_unless(format2 != NULL, NULL);
	fail_unless(error == NULL, NULL);

	OSyncFormatConverter *converter = osync_converter_new(OSYNC_CONVERTER_CONV, format1, format2, converter_conv, &error);
	fail_unless(converter != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_objformat_unref(format1);
	osync_objformat_unref(format2);

	fail_unless(osync_converter_get_sourceformat(converter) == format1, NULL);
	fail_unless(osync_converter_get_targetformat(converter) == format2, NULL);
	fail_unless(osync_converter_get_type(converter) == OSYNC_CONVERTER_CONV, NULL);

	osync_converter_unref(converter);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (converter_create_decap)
{
	char *testbed = setup_testbed(NULL);

	OSyncError *error = NULL;
	OSyncObjFormat *format1 = osync_objformat_new("format1", "objtype", &error);
	fail_unless(format1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	OSyncObjFormat *format2 = osync_objformat_new("format2", "objtype", &error);
	fail_unless(format2 != NULL, NULL);
	fail_unless(error == NULL, NULL);

	OSyncFormatConverter *converter = osync_converter_new(OSYNC_CONVERTER_DECAP, format1, format2, converter_conv, &error);
	fail_unless(converter != NULL, NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(osync_converter_get_sourceformat(converter) == format1, NULL);
	fail_unless(osync_converter_get_targetformat(converter) == format2, NULL);
	fail_unless(osync_converter_get_type(converter) == OSYNC_CONVERTER_DECAP, NULL);

	osync_objformat_unref(format1);
	osync_objformat_unref(format2);

	osync_converter_unref(converter);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (converter_create_detector)
{
	char *testbed = setup_testbed(NULL);

	OSyncError *error = NULL;
	OSyncObjFormat *format1 = osync_objformat_new("format1", "objtype", &error);
	fail_unless(format1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	OSyncObjFormat *format2 = osync_objformat_new("format2", "objtype", &error);
	fail_unless(format2 != NULL, NULL);
	fail_unless(error == NULL, NULL);

	OSyncFormatConverter *converter = osync_converter_new_detector(format1, format2, conv_detect, &error);
	fail_unless(converter != NULL, NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(osync_converter_get_sourceformat(converter) == format1, NULL);
	fail_unless(osync_converter_get_targetformat(converter) == format2, NULL);
	fail_unless(osync_converter_get_type(converter) == OSYNC_CONVERTER_DETECTOR, NULL);

	osync_objformat_unref(format1);
	osync_objformat_unref(format2);

	osync_converter_unref(converter);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (converter_matches)
{
	char *testbed = setup_testbed(NULL);

	OSyncError *error = NULL;
	OSyncObjFormat *format1 = osync_objformat_new("format1", "objtype", &error);
	fail_unless(format1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	OSyncObjFormat *format2 = osync_objformat_new("format2", "objtype", &error);
	fail_unless(format2 != NULL, NULL);
	fail_unless(error == NULL, NULL);

	OSyncFormatConverter *converter = osync_converter_new(OSYNC_CONVERTER_CONV, format1, format2, converter_conv, &error);
	fail_unless(converter != NULL, NULL);
	fail_unless(error == NULL, NULL);

	OSyncData *data = osync_data_new("test", 4, format1, &error);
	fail_unless(data != NULL, NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(osync_converter_matches(converter, data) == TRUE, NULL);

	osync_data_set_objformat(data, format2);
	fail_unless(osync_converter_matches(converter, data) == FALSE, NULL);
	osync_data_unref(data);

	osync_objformat_unref(format1);
	osync_objformat_unref(format2);

	osync_converter_unref(converter);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (converter_detect)
{
	char *testbed = setup_testbed(NULL);

	OSyncError *error = NULL;
	OSyncObjFormat *format1 = osync_objformat_new("format1", "objtype", &error);
	fail_unless(format1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	OSyncObjFormat *format2 = osync_objformat_new("format2", "objtype", &error);
	fail_unless(format2 != NULL, NULL);
	fail_unless(error == NULL, NULL);

	OSyncFormatConverter *converter = osync_converter_new_detector(format1, format2, conv_detect, &error);
	fail_unless(converter != NULL, NULL);
	fail_unless(error == NULL, NULL);

	OSyncData *data = osync_data_new("format2", 8, format1, &error);
	fail_unless(data != NULL, NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(osync_converter_detect(converter, data) == format2, NULL);

	osync_data_set_data(data, "format1", 8);
	fail_unless(osync_converter_detect(converter, data) == FALSE, NULL);

	osync_data_unref(data);

	osync_objformat_unref(format1);
	osync_objformat_unref(format2);

	osync_converter_unref(converter);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (converter_detector_invoke)
{
	char *testbed = setup_testbed(NULL);

	OSyncError *error = NULL;
	OSyncObjFormat *format1 = osync_objformat_new("format1", "objtype", &error);
	fail_unless(format1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	OSyncObjFormat *format2 = osync_objformat_new("format2", "objtype", &error);
	fail_unless(format2 != NULL, NULL);
	fail_unless(error == NULL, NULL);

	OSyncFormatConverter *converter = osync_converter_new_detector(format1, format2, conv_detect, &error);
	fail_unless(converter != NULL, NULL);
	fail_unless(error == NULL, NULL);

	OSyncData *data = osync_data_new("format2", 8, format1, &error);
	fail_unless(data != NULL, NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(osync_converter_invoke(converter, data, NULL, &error) == TRUE, NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(osync_data_get_objformat(data) == format2, NULL);

	osync_data_unref(data);

	osync_objformat_unref(format1);
	osync_objformat_unref(format2);

	osync_converter_unref(converter);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (converter_detect_non_detector)
{
	char *testbed = setup_testbed(NULL);

	OSyncError *error = NULL;
	OSyncObjFormat *format1 = osync_objformat_new("format1", "objtype", &error);
	fail_unless(format1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	OSyncObjFormat *format2 = osync_objformat_new("format2", "objtype", &error);
	fail_unless(format2 != NULL, NULL);
	fail_unless(error == NULL, NULL);

	OSyncFormatConverter *converter = osync_converter_new(OSYNC_CONVERTER_CONV, format1, format2, converter_conv, &error);
	fail_unless(converter != NULL, NULL);
	fail_unless(error == NULL, NULL);

	OSyncData *data = osync_data_new("format2", 8, format1, &error);
	fail_unless(data != NULL, NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(osync_converter_detect(converter, data) == FALSE, NULL);
	fail_unless(error == NULL, NULL);

	osync_data_unref(data);

	osync_objformat_unref(format1);
	osync_objformat_unref(format2);

	osync_converter_unref(converter);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (converter_conv_invoke)
{
	char *testbed = setup_testbed(NULL);

	OSyncError *error = NULL;
	OSyncObjFormat *format1 = osync_objformat_new("format1", "objtype", &error);
	fail_unless(format1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_objformat_set_destroy_func(format1, destroy_format);
	OSyncObjFormat *format2 = osync_objformat_new("format2", "objtype", &error);
	fail_unless(format2 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_objformat_set_destroy_func(format2, destroy_format);

	OSyncFormatConverter *converter = osync_converter_new(OSYNC_CONVERTER_CONV, format1, format2, conv_format1_to_format2_dup, &error);
	fail_unless(converter != NULL, NULL);
	fail_unless(error == NULL, NULL);

	OSyncData *data = osync_data_new(g_strdup("format1"), 8, format1, &error);
	fail_unless(data != NULL, NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(osync_converter_invoke(converter, data, "test", &error) == TRUE, NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(osync_data_get_objformat(data) == format2, NULL);
	char *string = NULL;
	unsigned int size = 0;
	osync_data_get_data(data, &string, &size);
	fail_unless(!strcmp(string, "format2"), NULL);
	fail_unless(size == 8, NULL);

	osync_data_unref(data);

	osync_objformat_unref(format1);
	osync_objformat_unref(format2);

	osync_converter_unref(converter);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (converter_decap_invoke)
{
	char *testbed = setup_testbed(NULL);

	OSyncError *error = NULL;
	OSyncObjFormat *format1 = osync_objformat_new("format1", "objtype", &error);
	fail_unless(format1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_objformat_set_destroy_func(format1, destroy_format);
	OSyncObjFormat *format2 = osync_objformat_new("format2", "objtype", &error);
	fail_unless(format2 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_objformat_set_destroy_func(format2, destroy_format);

	OSyncFormatConverter *converter = osync_converter_new(OSYNC_CONVERTER_DECAP, format1, format2, decap_format1_to_format2_dup, &error);
	fail_unless(converter != NULL, NULL);
	fail_unless(error == NULL, NULL);

	OSyncData *data = osync_data_new(g_strdup("SHELLformat2"), 8, format1, &error);
	fail_unless(data != NULL, NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(osync_converter_invoke(converter, data, "test", &error) == TRUE, NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(osync_data_get_objformat(data) == format2, NULL);
	char *string = NULL;
	unsigned int size = 0;
	osync_data_get_data(data, &string, &size);
	fail_unless(!strcmp(string, "format2"), NULL);
	fail_unless(size == 8, NULL);

	osync_data_unref(data);

	osync_objformat_unref(format1);
	osync_objformat_unref(format2);

	osync_converter_unref(converter);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (converter_encap_invoke)
{
	char *testbed = setup_testbed(NULL);

	OSyncError *error = NULL;
	OSyncObjFormat *format1 = osync_objformat_new("format1", "objtype", &error);
	fail_unless(format1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_objformat_set_destroy_func(format1, destroy_format);
	OSyncObjFormat *format2 = osync_objformat_new("format2", "objtype", &error);
	fail_unless(format2 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_objformat_set_destroy_func(format2, destroy_format);

	OSyncFormatConverter *converter = osync_converter_new(OSYNC_CONVERTER_ENCAP, format1, format2, encap_format1_to_format2_dup, &error);
	fail_unless(converter != NULL, NULL);
	fail_unless(error == NULL, NULL);

	OSyncData *data = osync_data_new(g_strdup("format1"), 8, format1, &error);
	fail_unless(data != NULL, NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(osync_converter_invoke(converter, data, "test", &error) == TRUE, NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(osync_data_get_objformat(data) == format2, NULL);
	char *string = NULL;
	unsigned int size = 0;
	osync_data_get_data(data, &string, &size);
	fail_unless(!strcmp(string, "SHELLformat1"), NULL);

	osync_data_unref(data);

	osync_objformat_unref(format1);
	osync_objformat_unref(format2);

	osync_converter_unref(converter);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (converter_conv_invoke_const)
{
	char *testbed = setup_testbed(NULL);

	OSyncError *error = NULL;
	OSyncObjFormat *format1 = osync_objformat_new("format1", "objtype", &error);
	fail_unless(format1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_objformat_set_destroy_func(format1, destroy_format);
	OSyncObjFormat *format2 = osync_objformat_new("format2", "objtype", &error);
	fail_unless(format2 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_objformat_set_destroy_func(format2, destroy_format);

	OSyncFormatConverter *converter = osync_converter_new(OSYNC_CONVERTER_CONV, format1, format2, conv_format1_to_format2_const, &error);
	fail_unless(converter != NULL, NULL);
	fail_unless(error == NULL, NULL);

	OSyncData *data = osync_data_new(g_strdup("format1"), 8, format1, &error);
	fail_unless(data != NULL, NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(osync_converter_invoke(converter, data, "test", &error) == TRUE, NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(osync_data_get_objformat(data) == format2, NULL);
	char *string = NULL;
	unsigned int size = 0;
	osync_data_get_data(data, &string, &size);
	fail_unless(!strcmp(string, "format2"), NULL);
	fail_unless(size == 8, NULL);

	osync_data_unref(data);

	osync_objformat_unref(format1);
	osync_objformat_unref(format2);

	osync_converter_unref(converter);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (converter_decap_invoke_const)
{
	char *testbed = setup_testbed(NULL);

	OSyncError *error = NULL;
	OSyncObjFormat *format1 = osync_objformat_new("format1", "objtype", &error);
	fail_unless(format1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_objformat_set_destroy_func(format1, destroy_testdata);
	OSyncObjFormat *format2 = osync_objformat_new("format2", "objtype", &error);
	fail_unless(format2 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_objformat_set_destroy_func(format2, destroy_format);

	OSyncFormatConverter *converter = osync_converter_new(OSYNC_CONVERTER_DECAP, format1, format2, decap_format1_to_format2_const, &error);
	fail_unless(converter != NULL, NULL);
	fail_unless(error == NULL, NULL);

	testdata *testdata = malloc(sizeof(testdata));
	testdata->string1 = strdup("format");
	OSyncData *data = osync_data_new((char *)testdata, sizeof(testdata), format1, &error);
	fail_unless(data != NULL, NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(osync_converter_invoke(converter, data, "test", &error) == TRUE, NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(osync_data_get_objformat(data) == format2, NULL);
	char *string = NULL;
	unsigned int size = 0;
	osync_data_get_data(data, &string, &size);
	fail_unless(!strcmp(string, "format"), NULL);
	fail_unless(size == 7, NULL);

	osync_data_unref(data);

	osync_objformat_unref(format1);
	osync_objformat_unref(format2);

	osync_converter_unref(converter);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (converter_encap_invoke_const)
{
	char *testbed = setup_testbed(NULL);

	OSyncError *error = NULL;
	OSyncObjFormat *format1 = osync_objformat_new("format1", "objtype", &error);
	fail_unless(format1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_objformat_set_destroy_func(format1, destroy_format);
	OSyncObjFormat *format2 = osync_objformat_new("format2", "objtype", &error);
	fail_unless(format2 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_objformat_set_destroy_func(format2, destroy_testdata);

	OSyncFormatConverter *converter = osync_converter_new(OSYNC_CONVERTER_ENCAP, format1, format2, encap_format1_to_format2_const, &error);
	fail_unless(converter != NULL, NULL);
	fail_unless(error == NULL, NULL);

	OSyncData *data = osync_data_new(strdup("format1"), 8, format1, &error);
	fail_unless(data != NULL, NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(osync_converter_invoke(converter, data, "test", &error) == TRUE, NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(osync_data_get_objformat(data) == format2, NULL);
	char *buffer = NULL;
	unsigned int size = 0;
	osync_data_get_data(data, &buffer, &size);

	testdata *tdata = (testdata *)buffer;
	fail_unless(!strcmp(tdata->string1, "format1"), NULL);
	fail_unless(size == sizeof(testdata), NULL);

	osync_data_unref(data);

	osync_objformat_unref(format1);
	osync_objformat_unref(format2);

	osync_converter_unref(converter);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (converter_path_create)
{
	char *testbed = setup_testbed(NULL);

	OSyncError *error = NULL;
	OSyncFormatConverterPath *path = osync_converter_path_new(&error);
	fail_unless(path != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_converter_path_ref(path);
	osync_converter_path_unref(path);
	osync_converter_path_unref(path);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (converter_path_add)
{
	char *testbed = setup_testbed(NULL);

	OSyncError *error = NULL;
	OSyncObjFormat *format1 = osync_objformat_new("format1", "objtype", &error);
	fail_unless(format1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_objformat_set_destroy_func(format1, destroy_format);
	OSyncObjFormat *format2 = osync_objformat_new("format2", "objtype", &error);
	fail_unless(format2 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_objformat_set_destroy_func(format2, destroy_testdata);

	OSyncFormatConverter *converter = osync_converter_new(OSYNC_CONVERTER_ENCAP, format1, format2, encap_format1_to_format2_const, &error);
	fail_unless(converter != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_objformat_unref(format1);
	osync_objformat_unref(format2);

	OSyncFormatConverterPath *path = osync_converter_path_new(&error);
	fail_unless(path != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_converter_path_add_edge(path, converter);
	fail_unless(osync_converter_path_num_edges(path) == 1, NULL);

	osync_converter_path_add_edge(path, converter);
	fail_unless(osync_converter_path_num_edges(path) == 2, NULL);

	fail_unless(osync_converter_path_nth_edge(path, 0) == converter, NULL);
	fail_unless(osync_converter_path_nth_edge(path, 1) == converter, NULL);
	osync_converter_unref(converter);

	osync_converter_path_unref(path);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (converter_config)
{
	char *testbed = setup_testbed(NULL);

	OSyncError *error = NULL;
	OSyncFormatConverterPath *path = osync_converter_path_new(&error);
	fail_unless(path != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_converter_path_set_config(path, "test");
	fail_unless(!strcmp(osync_converter_path_get_config(path), "test"), NULL);

	osync_converter_path_set_config(path, "test1");
	fail_unless(!strcmp(osync_converter_path_get_config(path), "test1"), NULL);

	osync_converter_path_unref(path);

	destroy_testbed(testbed);
}
END_TEST

OSYNC_TESTCASE_START("converter")
OSYNC_TESTCASE_ADD(converter_create)
OSYNC_TESTCASE_ADD(converter_get)
OSYNC_TESTCASE_ADD(converter_create_decap)
OSYNC_TESTCASE_ADD(converter_create_detector)
OSYNC_TESTCASE_ADD(converter_matches)
OSYNC_TESTCASE_ADD(converter_detect)
OSYNC_TESTCASE_ADD(converter_detector_invoke)
OSYNC_TESTCASE_ADD(converter_detect_non_detector)

OSYNC_TESTCASE_ADD(converter_conv_invoke)
OSYNC_TESTCASE_ADD(converter_decap_invoke)
OSYNC_TESTCASE_ADD(converter_encap_invoke)

OSYNC_TESTCASE_ADD(converter_conv_invoke_const)
OSYNC_TESTCASE_ADD(converter_decap_invoke_const)
OSYNC_TESTCASE_ADD(converter_encap_invoke_const)

OSYNC_TESTCASE_ADD(converter_path_create)
OSYNC_TESTCASE_ADD(converter_path_add)
OSYNC_TESTCASE_ADD(converter_config)
OSYNC_TESTCASE_END

