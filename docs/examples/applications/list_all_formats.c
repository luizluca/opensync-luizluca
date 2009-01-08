#include <opensync/opensync.h>
#include <opensync/opensync-format.h>

int main(int argc, char *argv[]) {
	OSyncFormatEnv *env;
	OSyncObjFormat *format;
	
	osync_bool couldloadformats;
	
	int numformats = 0;
	int i = 0;
	
	env = osync_format_env_new (NULL);
	/* load formats from default dir */
	couldloadformats= osync_format_env_load_plugins(env, NULL, NULL);
	if (!couldloadformats) {
		/* print error */
		printf("Could not load formats.");
		return -1;
	}
	
	numformats = osync_format_env_num_objformats(env);
	printf("found %i formats\n", numformats);
	
	for( i = 0; i < numformats; i++ ) {
		format = osync_format_env_nth_objformat(env, i);
		printf("format nr. %i is %s\n", i+1, osync_objformat_get_name(format));
	}
	
	osync_format_env_unref(env);
	
	return 0;
}
