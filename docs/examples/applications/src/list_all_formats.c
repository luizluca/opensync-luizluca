#include <opensync/opensync.h>
#include <opensync/opensync-format.h>

int main(int argc, char *argv[]) {
	OSyncFormatEnv *env;
	OSyncObjFormat *format;
	
	osync_bool couldloadformats;
	
	OSyncList *f, *formats;
	int i = 0;
	
	env = osync_format_env_new (NULL);
	/* load formats from default dir */
	couldloadformats= osync_format_env_load_plugins(env, NULL, NULL);
	if (!couldloadformats) {
		/* print error */
		printf("Could not load formats.");
		return -1;
	}
	
	formats = osync_format_env_get_objformats(env);
	printf("found %i formats\n", osync_list_length(formats));
	
	for (f = formats; f; f = f->next) {
		format = (OSyncObjFormat *) f->data;
		printf("format nr. %i is %s\n", i+1, osync_objformat_get_name(format));
	}

	/* Free the list */
	osync_list_free(formats);
	
	osync_format_env_unref(env);
	
	return 0;
}
