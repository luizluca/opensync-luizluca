/*
 * Here you can specify a format plugin if you like.
 * You should use a format plugin, if the format conversion
 * is rather difficult or if other plugins might be able to reuse
 * your conversion. A format plugin can create new formats in get_format_info
 * and provide converters between different formats in get_conversion_info
 *
 */
#include <opensync/opensync.h>
#include <opensync/opensync-format.h>

typedef struct converter_data {
	char *data;
} converter_data;


static OSyncConvCmpResult compare_format1(const char *leftdata, unsigned int leftsize, const char *rightdata, unsigned int rightsize, void *data, OSyncError **error)
{
	/*
	 * This function can be used to compare two types of your formats.
	 * This is optional. For example, you can only provide a conversion
	 * from and to the xml format and let all the comparison be done there
	 *
	 */

	/*
	 * Compare your objects here. You might need to cast the data of the change
	 * objects to you own defined types.
	 *
	 * The possible result of the comparison are:
	 *
	 * return OSYNC_CONV_DATA_SAME;
	 * The objects are exactly the same. (they might differ in some uid or
	 * timestamp though, but ALL the "real" information is the same)
	 *
	 * return OSYNC_CONV_DATA_SIMILAR;
	 * The objects are not _exactly_ the same, but they look similar. This is used
	 * to detect conflicts. It is up to you to decide what "similar" means for your
	 * object
	 *
	 * return OSYNC_CONV_DATA_MISMATCH;
	 * This means the objects are not the same and not similar.
	 *
	 * return OSYNC_CONV_DATA_UNKNOWN;
	 * On any error, and set parameter error with a human readable error
	 * message.
	 *
	 */
	return OSYNC_CONV_DATA_MISMATCH;
}

static osync_bool conv_format1_to_format2(char *input, unsigned int inpsize, char **output, unsigned int *outpsize, osync_bool *free_input, const char *config, void *userdata, OSyncError **error)
{
	/*
	 * This function can be used to convert your format to another format.
	 * Return TRUE if the conversion was successful or return FALSE and set
	 * the error if something bad has happend.
	 *
	 */

	/* The arguments mean:
	 *
	 * - conv_data:
	 * Pointer to the data you returned in your init function (if any)
	 *
	 * - input:
	 * The data you need to convert
	 * - inpsize
	 * The size of the input data
	 *
	 * - output:
	 * After converting you need to set this
	 * to your result
	 * - outpsize:
	 * The size of the output
	 *
	 * - free_input:
	 * You need to set this to TRUE if opensync
	 * can free the input after the conversion (so you dont
	 * use any reference from or to the input). A example where
	 * *free_input = FALSE; needs to be done would be a encapsulator
	 * that stores the input reference somewhere in its struct
	 *
	 * - error:
	 * if something bad happens and you cannot convert, set the error!
	 *
	 */

	return TRUE;
}

static osync_bool conv_format2_to_format1(char *input, unsigned int inpsize, char **output, unsigned int *outpsize, osync_bool *free_input, const char *config, void *userdata, OSyncError **error)
{
	/*
	 * This function can be used to convert another format to your format.
	 * Return TRUE if the conversion was successful or return FALSE and set
	 * the error if something bad has happend.
	 *
	 */

	return TRUE;
}

static osync_bool destroy_format1(char *input, unsigned int size, void *data, OSyncError **error)
{
	/*
	 * Here you have to free the data allocated by your format
	 *
	 */

	return TRUE;
}

static osync_bool duplicate_format1(const char *uid, const char *input, unsigned int insize, char **newuid, char **output, unsigned int *outsize, osync_bool *dirty, void *data, OSyncError **error)
{
	/*
	 * This function can be used to duplicate your format.
	 * Duplication does not mean to make 2 objects out of one,
	 * but to change to uid of the object in such a way that it
	 * differes from the original uid.
	 *
	 * Most format will never need this.
	 *
	 */

	return TRUE;
}

static char *print_format1(const char *data, unsigned int size, void *user_data, OSyncError **error)
{
	/*
	 * If your format is not in a human printable format already
	 * you have to return a human readable string here describing the object
	 * as closely as possible. This information will be used by the user to decide
	 * which object to pick in a conflict.
	 *
	 */
	 return NULL;
}

void *init_format1(OSyncError **error) {
	/*
	 * If your format needs specific data e.g. a XML Schema file
	 * for validation it is possible to load the data in this 
	 * function. If the data should be passes to all format 
	 * functions it has to be returned as a pointer.
	 */
	char *format_specific_data = osync_try_malloc0(0, error);
	return (void *)format_specific_data;
}

osync_bool finialize_format1(void *data, OSyncError **error) {
	/*
	 * Release all format data
	 */
	osync_free(data);

	return TRUE;
}

osync_bool get_format_info(OSyncFormatEnv *env, OSyncError **error)
{
	/*
	 * this function is called to register a new format
	 */
	OSyncObjFormat *format = osync_objformat_new("<your format name>", "<some object type>", error);
	if (!format)
		goto error;

	osync_objformat_set_compare_func(format, compare_format1);
	osync_objformat_set_destroy_func(format, destroy_format1);
	osync_objformat_set_duplicate_func(format, duplicate_format1);
	osync_objformat_set_print_func(format, print_format1);

	osync_objformat_set_initialize_func(format, init_format1);
	osync_objformat_set_finalize_func(format, finialize_format1);

	if (!osync_format_env_register_objformat(env, format, error))
		goto error;

	osync_objformat_unref(format);

	return TRUE;

error:
	return FALSE;
}

void *initialize_converter(const char* config, OSyncError **error)
{
	/*
	 * Here you can create converter specific data.
	 * If you return the converter specific data, it is passed
	 * to the conversion and detector function as void *userdata.
	 * Converter specific data could be e.g. a XSLT file
	 */
	converter_data *userdata = osync_try_malloc0(sizeof(converter_data), error);
	return (void*)userdata;
}

void finalize_converter(void *userdata)
{
	/*
	 * Here you can free all your converter specific data.
	 */
	converter_data *converterdata = (converter_data*)userdata;
	osync_free(converterdata->data);
	osync_free(converterdata);
}

osync_bool get_conversion_info(OSyncFormatEnv *env, OSyncError **error)
{
	/*
	 * Here you have to give opensync some information about your format
	 * This function will be called directly after the plugin has been loaded
	 * to get converters that convert between different formats
	 */

	OSyncObjFormat *format1 = osync_format_env_find_objformat(env, "<your format name>");
	if (!format1) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to find <your format name> format");
		return FALSE;
	}

	OSyncObjFormat *format2 = osync_format_env_find_objformat(env, "xmlformat-contact");
	if (!format2) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to find xmlformat-contact format");
		return FALSE;
	}

	OSyncFormatConverter *conv = osync_converter_new(OSYNC_CONVERTER_CONV, format1, format2, conv_format1_to_format2, error);
	if (!conv)
		return FALSE;
	/* set init and finalize functions */
	osync_converter_set_initialize_func(conv, initialize_converter);
	osync_converter_set_finalize_func(conv, finalize_converter);
	/* register converter */
	osync_format_env_register_converter(env, conv, error);
	osync_converter_unref(conv);

	conv = osync_converter_new(OSYNC_CONVERTER_CONV, format2, format1, conv_format2_to_format1, error);
	if (!conv)
		return FALSE;
	/* e.g. this converter doesn't need init and finalize functions therefore don't set them */
	/* register converter */
	osync_format_env_register_converter(env, conv, error);
	osync_converter_unref(conv);
	return TRUE;
}

int get_version(void)
{
	/* always return 1 */
	return 1;
}
